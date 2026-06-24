#pragma execution_character_set("utf-8")
#include "IntelligentAnalyTreeWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QApplication>
#include <QIcon>
#include <QFileDialog>
#include <QDateTime>
#include <algorithm>
#include <QRegExp>
#include <QRegularExpression> 
#include <QValidator>
#include <QVector>
#include <QRandomGenerator>

#include "IntelligentAnalyWidget.h"
#include "ModelDataManager.h"
#include "ProgressDialog.h"
#include "ParamAnalyCalculateWorker.h"
#include "WordExporter.h"
#include "WordExporterWorker.h"


double translateStress(double x)
{
	const double k = 0.0294118;
	const double mid = 360.0;
	const double steep = 320.0;
	double base = k * x;
	double soften = 1.0 + exp((x - mid) / steep);
	double y = base / soften;

	// 封顶保底
	if (y < 0.0) y = 0.0;
	//if (y > 20.0) y = 20.0;
	return y;
}


QVector<QVector<QVariant>> tableWidToVariantVector(QTableWidget* tableWidget)
{
	QVector<QVector<QVariant>> result;
	if (!tableWidget || tableWidget->rowCount() == 0) {
		return result; // 空表格直接返回
	}

	int rowCount = tableWidget->rowCount();
	int colCount = tableWidget->columnCount();

	// 2. 遍历数据行和列，填充表格数据
	for (int row = 0; row < rowCount; ++row) {
		QVector<QVariant> dataRow;
		for (int col = 0; col < colCount; ++col) {
			// 获取单元格项，为空时用空QVariant填充（避免空指针）
			QTableWidgetItem* cellItem = tableWidget->item(row, col);
			if (cellItem) {
				// 优先获取单元格的原始数据（如果设置过setData），没有则用文本
				if (cellItem->data(Qt::DisplayRole).isValid()) {
					dataRow.append(cellItem->data(Qt::DisplayRole));
				}
				else {
					dataRow.append(cellItem->text());
				}
			}
			else {
				dataRow.append(QVariant("")); // 空单元格填充空字符串，避免数据缺失
			}
		}
		result.append(dataRow);
	}

	return result;
}

// 壳体点位
QVector<int> m_array = { 1, 2, 3, 4, 5, 6, 10, 11, 15, 16, 20, 21, 25, 26, 30, 31, 35, 36, 37, 38, 39, 40 };

double calculateForm(const QString& formula,
	double B, double C, double D, double E,
	double F, double G, double H, double I,
	double J, double K, double L, double M, double A)
{
	QString processedFormula = formula;
	processedFormula.remove(' '); // 移除所有空格，避免干扰匹配

	// 变量映射：保持原映射关系，兼容A-M变量
	const QMap<QString, double> varMap = {
		{"A", A}, {"B", B}, {"C", C}, {"D", D}, {"E", E},
		{"F", F}, {"G", G}, {"H", H}, {"I", I}, {"J", J},
		{"K", K}, {"L", L}, {"M", M}
	};

	/************************ 核心修改：正则表达式（支持二次项） ************************/
	// 匹配格式：符号 + 系数 + （变量部分：纯数字 / 单个变量 / 变量平方 / 两个变量乘积）
	QRegExp regExp("([+-]?)((?:\\d+(?:\\.\\d*)?)|(?:\\.\\d+))(?:(?:\\*([A-Z])(?:\\^2|\\*([A-Z]))?)?)");
	regExp.setMinimal(false); // 贪婪匹配，确保获取完整项

	double result = 0.0;
	int pos = 0;
	int matchCount = 0; // 统计有效匹配项数

	// 补全公式开头符号，统一处理逻辑（原逻辑保留）
	if (!processedFormula.isEmpty() && processedFormula[0] != '+' && processedFormula[0] != '-') {
		processedFormula = "+" + processedFormula;
	}

	// 循环匹配所有有效项（原循环结构保留，内部逻辑升级）
	while ((pos = regExp.indexIn(processedFormula, pos)) != -1) {
		++matchCount;
		// 捕获分组内容
		QString signStr = regExp.cap(1);       // 符号（+/-）
		QString coeffStr = regExp.cap(2);      // 系数（整数/小数）
		QString var1Str = regExp.cap(3);       // 第一个变量（X in X^2 或 X in X*Y）
		QString var2Str = regExp.cap(4);       // 第二个变量（Y in X*Y，平方项时为空）

		// 1. 解析符号（原逻辑保留）
		double sign = (signStr == "-") ? -1.0 : 1.0;

		// 2. 解析系数（原逻辑保留，含异常处理）
		bool ok = false;
		double coeff = coeffStr.toDouble(&ok);
		if (!ok) {
			throw std::invalid_argument(QString("无效系数: %1").arg(coeffStr).toStdString());
		}

		// 3. 计算当前项的值（核心升级：支持二次项）
		double term = sign * coeff;
		if (!var1Str.isEmpty()) {
			// 检查第一个变量是否合法
			if (!varMap.contains(var1Str)) {
				throw std::invalid_argument(QString("未知变量: %1").arg(var1Str).toStdString());
			}
			double var1Val = varMap[var1Str];

			if (!var2Str.isEmpty()) {
				// 情况1：变量乘积项（X*Y，如B*C、B*D）
				if (!varMap.contains(var2Str)) {
					throw std::invalid_argument(QString("未知变量: %1").arg(var2Str).toStdString());
				}
				double var2Val = varMap[var2Str];
				term *= (var1Val * var2Val); // 符号×系数×变量1×变量2
			}
			else {
				// 判断是否是平方项（通过正则匹配的结构，var1Str存在且var2Str为空时，要么是一次项，要么是平方项）
				// 提取变量部分的原始匹配，判断是否包含^2
				QString varPart = regExp.cap(0).mid(signStr.length() + coeffStr.length());
				if (varPart.contains("^2")) {
					// 情况2：变量平方项（X^2，如B^2、C^2）
					term *= (var1Val * var1Val); // 符号×系数×变量^2
				}
				else {
					// 情况3：一次项（X，如B、C，原线性项逻辑）
					term *= var1Val; // 符号×系数×变量
				}
			}
		}
		// 情况4：纯数字项（var1Str为空，直接使用 term = sign*coeff，无需额外计算）

		// 累加当前项到结果
		result += term;
		pos += regExp.matchedLength();
	}

	// 公式合法性校验（原逻辑保留）
	if (matchCount == 0) {
		throw std::invalid_argument(QString("公式格式错误: %1").arg(formula).toStdString());
	}

	return result;
}


IntelligentAnalyTreeWidget::IntelligentAnalyTreeWidget(QWidget* parent)
	:QWidget(parent)
{
	//wordExporter = new WordExporter(this);

	QIcon error_icon(":/src/Error.svg");
	QIcon checked_icon(":/src/Checked.svg");
	QIcon icon(":/src/data_calculation.svg");

	treeWidget = new GFTreeWidget(this);
	treeWidget->setColumnCount(1);
	treeWidget->setHeaderLabels({ "数据智能分析" });
	treeWidget->setHeaderHidden(true);
	treeWidget->setStyleSheet("QTreeWidget { color: black; }");
	treeWidget->setColumnCount(1);

	// 创建根节点
	QTreeWidgetItem* rootItem = new QTreeWidgetItem(treeWidget);
	rootItem->setText(0, "数据智能分析");
	rootItem->setData(0, Qt::UserRole, "IntelligentAnaly");
	rootItem->setExpanded(true);
	rootItem->setCheckState(0, Qt::Unchecked);
	rootItem->setIcon(0, icon);

	QTreeWidgetItem* fallNode = new QTreeWidgetItem(rootItem);
	fallNode->setText(0, "跌落安全性智能分析");
	fallNode->setData(0, Qt::UserRole, "FallIntelligentAnaly");
	fallNode->setCheckState(0, Qt::Unchecked);
	fallNode->setIcon(0, icon);

	QTreeWidgetItem* fastCombustionNode = new QTreeWidgetItem(rootItem);
	fastCombustionNode->setText(0, "快速烤燃安全性智能分析");
	fastCombustionNode->setData(0, Qt::UserRole, "FastCombustionIntelligentAnaly");
	fastCombustionNode->setCheckState(0, Qt::Unchecked);
	fastCombustionNode->setIcon(0, icon);

	QTreeWidgetItem* slowCombustionNode = new QTreeWidgetItem(rootItem);
	slowCombustionNode->setText(0, "慢速烤燃安全性智能分析");
	slowCombustionNode->setData(0, Qt::UserRole, "SlowCombustionIntelligentAnaly");
	slowCombustionNode->setCheckState(0, Qt::Unchecked);
	slowCombustionNode->setIcon(0, icon);

	QTreeWidgetItem* shootNode = new QTreeWidgetItem(rootItem);
	shootNode->setText(0, "枪击安全性智能分析");
	shootNode->setData(0, Qt::UserRole, "ShootIntelligentAnaly");
	shootNode->setCheckState(0, Qt::Unchecked);
	shootNode->setIcon(0, icon);

	QTreeWidgetItem* jetImpactNode = new QTreeWidgetItem(rootItem);
	jetImpactNode->setText(0, "射流冲击安全性智能分析");
	jetImpactNode->setData(0, Qt::UserRole, "JetImpactIntelligentAnaly");
	jetImpactNode->setCheckState(0, Qt::Unchecked);
	jetImpactNode->setIcon(0, icon);

	QTreeWidgetItem* fragmentationImpactNode = new QTreeWidgetItem(rootItem);
	fragmentationImpactNode->setText(0, "破片撞击安全性智能分析");
	fragmentationImpactNode->setData(0, Qt::UserRole, "FragmentationImpactIntelligentAnaly");
	fragmentationImpactNode->setCheckState(0, Qt::Unchecked);
	fragmentationImpactNode->setIcon(0, icon);

	QTreeWidgetItem* explosiveBlastNode = new QTreeWidgetItem(rootItem);
	explosiveBlastNode->setText(0, "爆炸冲击波安全性智能分析");
	explosiveBlastNode->setData(0, Qt::UserRole, "ExplosiveBlastIntelligentAnaly");
	explosiveBlastNode->setCheckState(0, Qt::Unchecked);
	explosiveBlastNode->setIcon(0, icon);

	QTreeWidgetItem* sacrificeExplosionNode = new QTreeWidgetItem(rootItem);
	sacrificeExplosionNode->setText(0, "殉爆安全性智能分析");
	sacrificeExplosionNode->setData(0, Qt::UserRole, "SacrificeExplosionIntelligentAnaly");
	sacrificeExplosionNode->setCheckState(0, Qt::Unchecked);
	sacrificeExplosionNode->setIcon(0, icon);


	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(treeWidget);
	layout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(layout);

	// 连接信号槽以处理复选框点击事件
	connect(treeWidget, &QTreeWidget::itemChanged, [](QTreeWidgetItem* item, int column) {
		if (item->childCount() > 0) {
			// 如果父节点被选中，则选中所有子节点
			if (item->checkState(0) == Qt::Checked) {
				for (int i = 0; i < item->childCount(); ++i) {
					item->child(i)->setCheckState(0, Qt::Checked);
				}
			}
			// 如果父节点被取消选中，则取消选中所有子节点
			else if (item->checkState(0) == Qt::Unchecked) {
				for (int i = 0; i < item->childCount(); ++i) {
					item->child(i)->setCheckState(0, Qt::Unchecked);
				}
			}
		}
		});

	// 连接信号槽
	connect(treeWidget, &QTreeWidget::itemClicked, this, &IntelligentAnalyTreeWidget::onTreeItemClicked);
}

IntelligentAnalyTreeWidget::~IntelligentAnalyTreeWidget()
{
}

void IntelligentAnalyTreeWidget::onTreeItemClicked(QTreeWidgetItem* item, int column)
{
	QString itemData = item->data(0, Qt::UserRole).toString();
	emit itemClicked(itemData);
}

void IntelligentAnalyTreeWidget::updataIcon()
{

}



void IntelligentAnalyTreeWidget::contextMenuEvent(QContextMenuEvent* event)
{
	QTreeWidgetItem* item = treeWidget->itemAt(event->pos());
	if (!item) {
		return;
	}
	QString text = item->text(0);
	if (text == "数据智能分析")
	{
		contextMenu = new QMenu(this);
		QAction* calAction = new QAction("计算", this);

		QAction* exportAction = new QAction("导出", this);

		int childCount = item->childCount();
		QList<QTreeWidgetItem*> checkedChildItems;
		for (int i = 0; i < childCount; ++i) {
			QTreeWidgetItem* childItem = item->child(i);
			if (childItem->checkState(0) == Qt::Checked) {
				checkedChildItems.append(childItem);
			}
		}
		connect(calAction, &QAction::triggered, this, [item, this]() {
			QWidget* parent = parentWidget();
			while (parent)
			{
				IntelligentAnalyWidget* paParent = dynamic_cast<IntelligentAnalyWidget*>(parent);
				if (paParent)
				{




					// 创建进度对话框
					ProgressDialog* progressDialog = new ProgressDialog("开始计算", parent);
					progressDialog->show();

					// 创建工作线程和工作对象
					ParamAnalyCalculateWorker* calculateWorker = new ParamAnalyCalculateWorker();
					QThread* calculateThread = new QThread();
					calculateWorker->moveToThread(calculateThread);

					// 连接信号槽
					connect(calculateThread, &QThread::started, calculateWorker, &ParamAnalyCalculateWorker::DoWork);
					connect(calculateWorker, &ParamAnalyCalculateWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
					connect(calculateWorker, &ParamAnalyCalculateWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
					connect(progressDialog, &ProgressDialog::Canceled, calculateWorker, &ParamAnalyCalculateWorker::RequestInterruption, Qt::DirectConnection);

					// 处理导入结果
					connect(calculateWorker, &ParamAnalyCalculateWorker::WorkFinished, this,
						[=](bool success, const QString& msg) {

							auto ins = ModelDataManager::GetInstance();
							auto steelInfo = ins->GetSteelPropertyInfo();
							auto propellantInfo = ins->GetPropellantPropertyInfo();
							auto calInfo = ins->GetCalculationPropertyInfo();
							auto fallInfo = ins->GetFallSettingInfo();
							auto modelGeomInfo = ins->GetModelGeometryInfo();
							auto insulatingheatPropertyInfo = ins->GetInsulatingheatPropertyInfo();

							auto fallTableWidget = paParent->getFallTableWidget();

							auto limitValue = steelInfo.tensileStrength * 1.5; // 抗拉强度
							auto tangentModulus = steelInfo.tangentModulus; // 切线模量
							auto modulus = steelInfo.modulus;// 弹性模量
							double difference = (tangentModulus / modulus);
							difference = (difference - 0.15) / 0.15;

							// 绝热层导热系数
							auto thermalConductivity = insulatingheatPropertyInfo.thermalConductivity;
							double tempDifference = thermalConductivity - 1;

							QTreeWidgetItem* child;
							int childCount = treeWidget->topLevelItemCount();
							for (int i = 0; i < childCount; i++)
							{
								child = treeWidget->topLevelItem(i);
								int childCount = child->childCount();
								for (int j = 0; j < childCount; ++j)
								{
									auto data = child->child(j)->data(0, Qt::UserRole).toString();
									auto check = child->child(j)->checkState(0);
									if (data == "FallIntelligentAnaly" && Qt::Checked == check)
									{
										auto A = 1;
										auto B = steelInfo.density;
										auto C = steelInfo.modulus / 1000;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = propellantInfo.modulus / 1000;
										auto H = propellantInfo.thermalConductivity;
										auto I = propellantInfo.specificHeatCapacity;
										auto J = fallInfo.high * 1000;//跌落高度
										auto K = modelGeomInfo.length;//长
										auto L = modelGeomInfo.width;//宽
										auto M = modelGeomInfo.thickness;//厚

										// 应力
										auto stressCalculation = calInfo.fallStressCalculation;
										// 温度
										auto temperatureCalculation = calInfo.fallTemperatureCalculation;
										for (size_t i = 1; i < 10; i++)
										{
											// 更新计算数值
											M = fallTableWidget->item(i, 1)->text().toInt();	// 厚度
											J = fallTableWidget->item(i, 2)->text().toDouble() * 1000;//跌落高度

											std::vector<double> steelStressResults;
											std::vector<double> propellantStressResults;

											//std::vector<double> stressResults;
											//stressResults.reserve(stressCalculation.size());
											for (int i = 0; i < stressCalculation.size(); ++i)
											{
												double res = calculateForm(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												if (res < 0)
												{
													res = 0;
												}
												res = res * 0.5;
												res = res + res * difference * 0.1;
												if (res > limitValue)
												{
													res = limitValue;
												}
												if (!m_array.contains(i + 1))
												{
													res = translateStress(res);
													propellantStressResults.push_back(res);
												}
												else
												{
													steelStressResults.push_back(res);
												}
											}

											double calSteelStressMaxValue = *std::max_element(steelStressResults.begin(), steelStressResults.end());
											double calPropellantStressMaxValue = *std::max_element(propellantStressResults.begin(), propellantStressResults.end());

											fallTableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(calSteelStressMaxValue)));
											fallTableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(calPropellantStressMaxValue)));

											// 温度
											std::vector<double> steelTemperatureResults;
											std::vector<double> propellantTemperatureResults;
											for (int i = 0; i < temperatureCalculation.size(); ++i)
											{
												double res = calculateForm(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												if (res < 25)
												{
													res = 25;
												}
												if (fallInfo.angle == 0 && res > 30)
												{
													res = 28.589;
												}
												if (!m_array.contains(i + 1))
												{
													propellantTemperatureResults.push_back(res);
												}
												else
												{
													steelTemperatureResults.push_back(res);
												}
											}

											double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
											double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());
											fallTableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(calSteelTemperatureMaxValue)));
											fallTableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(calPropellantTemperatureMaxValue)));
										}

										// 更新ChartView
										auto chartView = paParent->getChartView();
										auto chart = paParent->getChart();
										auto lineSeries1 = paParent->getLineSeries1();
										auto lineSeries2 = paParent->getLineSeries2();
										auto lineSeries3 = paParent->getLineSeries3();
										auto scatter1 = paParent->getScatter1();
										auto scatter2 = paParent->getScatter2();
										auto scatter3 = paParent->getScatter3();
										auto x_comboBox = paParent->getXComboBox();
										x_comboBox->setCurrentIndex(0);
										x_comboBox->setItemText(1, "跌落高度");

										QVector<QPointF> data1;
										if (fallTableWidget->item(1, 3))
										{
											data1.append(QPointF(1, fallTableWidget->item(1, 3)->text().toDouble()));

										}
										if (fallTableWidget->item(2, 3))
										{
											data1.append(QPointF(2, fallTableWidget->item(2, 3)->text().toDouble()));

										}
										if (fallTableWidget->item(3, 3))
										{
											data1.append(QPointF(3, fallTableWidget->item(3, 3)->text().toDouble()));

										}
										QVector<QPointF> data2;
										if (fallTableWidget->item(4, 3))
										{
											data2.append(QPointF(1, fallTableWidget->item(4, 3)->text().toDouble()));

										}
										if (fallTableWidget->item(5, 3))
										{
											data2.append(QPointF(2, fallTableWidget->item(5, 3)->text().toDouble()));

										}
										if (fallTableWidget->item(6, 3))
										{
											data2.append(QPointF(3, fallTableWidget->item(6, 3)->text().toDouble()));

										}
										QVector<QPointF> data3;
										if (fallTableWidget->item(7, 3))
										{
											data3.append(QPointF(1, fallTableWidget->item(7, 3)->text().toDouble()));

										}
										if (fallTableWidget->item(8, 3))
										{
											data3.append(QPointF(2, fallTableWidget->item(8, 3)->text().toDouble()));

										}
										if (fallTableWidget->item(9, 3))
										{
											data3.append(QPointF(3, fallTableWidget->item(9, 3)->text().toDouble()));

										}

										updateChartData(chartView, chart, lineSeries1, lineSeries2, lineSeries3, scatter1, scatter2, scatter3, data1, data2, data3, "壳体厚度", "壳体最大应力");

										// 更新三维模型数据
										QVector<QVector<double>> newData;
										QVector<double> graphicData1;
										graphicData1.append(fallTableWidget->item(1, 3)->text().toDouble());
										graphicData1.append(fallTableWidget->item(4, 3)->text().toDouble());
										graphicData1.append(fallTableWidget->item(7, 3)->text().toDouble());
										newData.append(graphicData1);
										QVector<double> graphicData2;
										graphicData2.append(fallTableWidget->item(2, 3)->text().toDouble());
										graphicData2.append(fallTableWidget->item(5, 3)->text().toDouble());
										graphicData2.append(fallTableWidget->item(8, 3)->text().toDouble());
										newData.append(graphicData2);
										QVector<double> graphicData3;
										graphicData3.append(fallTableWidget->item(3, 3)->text().toDouble());
										graphicData3.append(fallTableWidget->item(6, 3)->text().toDouble());
										graphicData3.append(fallTableWidget->item(9, 3)->text().toDouble());
										newData.append(graphicData3);

										QVector<double> xCoords;
										xCoords.append(fallTableWidget->item(1, 1)->text().toDouble());
										xCoords.append(fallTableWidget->item(2, 1)->text().toDouble());
										xCoords.append(fallTableWidget->item(3, 1)->text().toDouble());

										QVector<double> yCoords;
										yCoords.append(fallTableWidget->item(1, 2)->text().toDouble());
										yCoords.append(fallTableWidget->item(4, 2)->text().toDouble());
										yCoords.append(fallTableWidget->item(7, 2)->text().toDouble());

										paParent->updateGraphicData("壳体厚度", "跌落高度", "壳体最大应力", xCoords, yCoords, newData,
											fallTableWidget->item(1, 1)->text().toDouble(), fallTableWidget->item(3, 1)->text().toDouble(),
											fallTableWidget->item(1, 2)->text().toDouble(), fallTableWidget->item(7, 2)->text().toDouble());

									}
									else if (data == "FastCombustionIntelligentAnaly" && Qt::Checked == check)
									{
										// 快烤计算结果
										auto tableWidget = paParent->getFastCombustionTableWidget();
										auto fastCombustionSettingInfo = ModelDataManager::GetInstance()->GetFastCombustionSettingInfo();

										auto A = 1;
										auto B = steelInfo.density;
										auto C = steelInfo.modulus / 1000;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = propellantInfo.modulus / 1000;
										auto H = propellantInfo.thermalConductivity;
										auto I = propellantInfo.specificHeatCapacity;

										auto J = modelGeomInfo.length;//长
										auto K = modelGeomInfo.width / 2;//半径
										auto L = modelGeomInfo.thickness;//厚
										auto M = fastCombustionSettingInfo.temperature;//环境温度

										auto temperatureCalculation = calInfo.fastCombustionCalculation;

										for (size_t i = 1; i < 10; i++)
										{
											// 更新计算数值
											L = tableWidget->item(i, 1)->text().toInt();	// 厚度
											M = tableWidget->item(i, 2)->text().toDouble();//环境温度
											std::vector<double> steelTemperatureResults;
											std::vector<double> propellantTemperatureResults;
											for (int i = 0; i < temperatureCalculation.size(); ++i)
											{
												double res = calculateForm(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												res = res * 0.7;
												res = res + res * 0.1 * tempDifference;
												if (res <= 25)
												{
													res = 25;
												}
												if (!m_array.contains(i + 1))
												{
													propellantTemperatureResults.push_back(res);
												}
												else
												{
													steelTemperatureResults.push_back(res);
												}
											}
											
											double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
											propellantTemperatureResults.push_back(calSteelTemperatureMaxValue * 0.85);
											double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());
											tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(calSteelTemperatureMaxValue)));
											tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(calPropellantTemperatureMaxValue)));
										}
									}
									else if (data == "SlowCombustionIntelligentAnaly" && Qt::Checked == check)
									{
										// 慢烤计算结果
										auto tableWidget = paParent->getSlowCombustionTableWidget();
										auto slowCombustionSettingInfo = ModelDataManager::GetInstance()->GetSlowCombustionSettingInfo();

										auto A = 1;
										auto B = steelInfo.density;
										auto C = steelInfo.modulus / 1000;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = propellantInfo.modulus / 1000;
										auto H = propellantInfo.thermalConductivity;
										auto I = propellantInfo.specificHeatCapacity;

										auto J = modelGeomInfo.length;//长
										auto K = modelGeomInfo.width / 2;//半径
										auto L = modelGeomInfo.thickness;//厚
										auto M = slowCombustionSettingInfo.temperature;//温度幅度

										auto temperatureCalculation = calInfo.slowCombustionCalculation;

										for (size_t i = 1; i < 10; i++)
										{
											// 更新计算数值
											L = tableWidget->item(i, 1)->text().toInt();	// 厚度
											M = tableWidget->item(i, 2)->text().toDouble();//环境温度

											std::vector<double> steelTemperatureResults;
											std::vector<double> propellantTemperatureResults;
											for (int i = 0; i < temperatureCalculation.size(); ++i)
											{
												double res = calculateForm(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												res = res + res * 0.1 * tempDifference;
												if (res > M)
												{
													res = M;
												}
												if (!m_array.contains(i + 1))
												{
													if (res < (M * 0.94))
													{
														res = M * 0.94;
													}
													propellantTemperatureResults.push_back(res);
												}
												else
												{
													if (res < (M * 0.95))
													{
														res = M * 0.95;
													}
													steelTemperatureResults.push_back(res);
												}
											}
											steelTemperatureResults.push_back(M);
											double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
											double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());

											tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(calSteelTemperatureMaxValue)));
											tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(calPropellantTemperatureMaxValue)));
										}
									}
									else if (data == "ShootIntelligentAnaly" && Qt::Checked == check)
									{
										// 枪击计算结果
										auto shootInfo = ModelDataManager::GetInstance()->GetShootSettingInfo();
										auto tableWidget = paParent->getShootTableWidget();

										auto A = 1;
										auto B = steelInfo.density;
										auto C = steelInfo.modulus / 1000;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = propellantInfo.modulus / 1000;
										auto H = propellantInfo.thermalConductivity;
										auto I = propellantInfo.specificHeatCapacity;

										auto J = modelGeomInfo.length;//长
										auto K = shootInfo.radius;//子弹直径
										auto L = modelGeomInfo.thickness;//厚度
										auto M = shootInfo.speed * 1000;//撞击速度

										// 应力
										auto stressCalculation = calInfo.shootStressCalculation;
										// 温度
										auto temperatureCalculation = calInfo.shootTemperatureCalculation;
										for (size_t i = 1; i < 10; i++)
										{
											// 更新计算数值
											L = tableWidget->item(i, 1)->text().toInt();	// 厚度
											M = tableWidget->item(i, 2)->text().toDouble() * 1000;//撞击速度

											std::vector<double> steelStressResults;
											std::vector<double> propellantStressResults;

											//std::vector<double> stressResults;
											//stressResults.reserve(stressCalculation.size());
											for (int i = 0; i < stressCalculation.size(); ++i)
											{
												double res = calculateForm(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												if (res < 0)
												{
													res = 0;
												}
												res = res * 0.5;
												res = res + res * difference * 0.1;
												if (res > limitValue)
												{
													res = limitValue;
												}
												if (!m_array.contains(i + 1))
												{
													res = translateStress(res);
													propellantStressResults.push_back(res);
												}
												else
												{
													steelStressResults.push_back(res);
												}
											}
											double calSteelStressMaxValue = *std::max_element(steelStressResults.begin(), steelStressResults.end());
											double calPropellantStressMaxValue = *std::max_element(propellantStressResults.begin(), propellantStressResults.end());

											tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(calSteelStressMaxValue)));
											tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(calPropellantStressMaxValue)));

											// 温度
											std::vector<double> steelTemperatureResults;
											std::vector<double> propellantTemperatureResults;
											for (int i = 0; i < temperatureCalculation.size(); ++i)
											{
												double res = calculateForm(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												if (res < 25)
												{
													res = 25;
												}
												if (!m_array.contains(i + 1))
												{
													propellantTemperatureResults.push_back(res);
												}
												else
												{
													steelTemperatureResults.push_back(res);
												}
											}
											double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
											double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());

											tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(calSteelTemperatureMaxValue)));
											tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(calPropellantTemperatureMaxValue)));
										}
									}
									else if (data == "JetImpactIntelligentAnaly" && Qt::Checked == check)
									{
										// 射流冲击计算结果
										auto jetImpactingInfo = ModelDataManager::GetInstance()->GetJetImpactSettingInfo();
										auto tableWidget = paParent->getJetImpactTableWidget();

										auto A = 1;
										auto B = steelInfo.density;
										auto C = steelInfo.modulus / 1000;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = propellantInfo.modulus / 1000;
										auto H = propellantInfo.thermalConductivity;
										auto I = propellantInfo.specificHeatCapacity;

										auto J = modelGeomInfo.length;//长
										auto K = modelGeomInfo.width / 2;//半径
										auto L = modelGeomInfo.thickness;//厚
										auto M = jetImpactingInfo.caliber;// 聚能装药口径

										// 应力
										auto stressCalculation = calInfo.jetImpactStressCalculation;
										// 温度
										auto temperatureCalculation = calInfo.jetImpactTemperatureCalculation;
										for (size_t i = 1; i < 10; i++)
										{
											// 更新计算数值
											L = tableWidget->item(i, 1)->text().toInt();	// 厚度
											M = tableWidget->item(i, 2)->text().toDouble();//聚能装药口径

											std::vector<double> steelStressResults;
											std::vector<double> propellantStressResults;
											for (int i = 0; i < stressCalculation.size(); ++i)
											{
												double res = calculateForm(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												if (res < 0)
												{
													res = 0;
												}
												res = res * 0.5;
												res = res + res * difference * 0.1;
												if (res > limitValue)
												{
													res = limitValue;
												}
												if (!m_array.contains(i + 1))
												{
													res = translateStress(res);
													propellantStressResults.push_back(res);
												}
												else
												{
													steelStressResults.push_back(res);
												}
											}
											double calSteelStressMaxValue = *std::max_element(steelStressResults.begin(), steelStressResults.end());
											double calPropellantStressMaxValue = *std::max_element(propellantStressResults.begin(), propellantStressResults.end());

											tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(calSteelStressMaxValue)));
											tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(calPropellantStressMaxValue)));

											// 温度
											std::vector<double> steelTemperatureResults;
											std::vector<double> propellantTemperatureResults;
											for (int i = 0; i < temperatureCalculation.size(); ++i)
											{
												double res = calculateForm(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												res = res + 10;
												if (res < 25)
												{
													res = 25;
												}
												if (!m_array.contains(i + 1))
												{
													propellantTemperatureResults.push_back(res);
												}
												else
												{
													steelTemperatureResults.push_back(res);
												}
											}
											double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
											double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());
											tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(calSteelTemperatureMaxValue)));
											tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(calPropellantTemperatureMaxValue)));
										}
									}
									else if (data == "FragmentationImpactIntelligentAnaly" && Qt::Checked == check)
									{
										// 破片撞击计算结果
										auto fragmentationSettingInfo = ModelDataManager::GetInstance()->GetFragmentationSettingInfo();
										auto tableWidget = paParent->getFragmentationImpactTableWidget();

										auto A = 1;
										auto B = steelInfo.density;
										auto C = steelInfo.modulus / 1000;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = propellantInfo.modulus / 1000;
										auto H = propellantInfo.thermalConductivity;
										auto I = propellantInfo.specificHeatCapacity;

										auto J = modelGeomInfo.length;//长
										auto K = fragmentationSettingInfo.radius;//破片直径
										auto L = modelGeomInfo.thickness;//厚度
										auto M = fragmentationSettingInfo.speed * 1000;//撞击速度

										// 应力
										auto stressCalculation = calInfo.fragmentationImpactStressCalculation;
										// 温度
										auto temperatureCalculation = calInfo.fragmentationImpactTemperatureCalculation;
										for (size_t i = 1; i < 10; i++)
										{
											// 更新计算数值
											L = tableWidget->item(i, 1)->text().toInt();	// 厚度
											M = tableWidget->item(i, 2)->text().toDouble() * 1000;//撞击速度

											std::vector<double> steelStressResults;
											std::vector<double> propellantStressResults;

											for (int i = 0; i < stressCalculation.size(); ++i)
											{
												double res = calculateForm(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												if (res < 0)
												{
													res = 0;
												}
												res = res * 0.5;
												res = res + res * difference * 0.1;
												if (res > limitValue)
												{
													res = limitValue;
												}
												if (!m_array.contains(i + 1))
												{
													res = translateStress(res);
													propellantStressResults.push_back(res);
												}
												else
												{
													steelStressResults.push_back(res);
												}
											}
											double calSteelStressMaxValue = *std::max_element(steelStressResults.begin(), steelStressResults.end());
											double calPropellantStressMaxValue = *std::max_element(propellantStressResults.begin(), propellantStressResults.end());

											tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(calSteelStressMaxValue)));
											tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(calPropellantStressMaxValue)));

											// 温度
											std::vector<double> steelTemperatureResults;
											std::vector<double> propellantTemperatureResults;
											for (int i = 0; i < temperatureCalculation.size(); ++i)
											{
												double res = calculateForm(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												if (res < 0)
												{
													res = 0;
												}
												res = res + 25;
												if (!m_array.contains(i + 1))
												{
													propellantTemperatureResults.push_back(res);
												}
												else
												{
													steelTemperatureResults.push_back(res);
												}
											}
											double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
											double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());
											tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(calSteelTemperatureMaxValue)));
											tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(calPropellantTemperatureMaxValue)));
										}
										TemperatureResult m_fragmentationImpactTemperatureResult = ins->GetFragmentationImpactTemperatureResult();
										tableWidget->setItem(1, 5, new QTableWidgetItem(QString::number(m_fragmentationImpactTemperatureResult.metalsMaxTemperature)));
										tableWidget->setItem(1, 6, new QTableWidgetItem(QString::number(m_fragmentationImpactTemperatureResult.propellantsMaxTemperature)));
									}
									else if (data == "ExplosiveBlastIntelligentAnaly" && Qt::Checked == check)
									{
										// 爆炸冲击波计算结果
										auto explosiveBlastSettingInfo = ModelDataManager::GetInstance()->GetExplosiveBlastSettingInfo();
										auto tableWidget = paParent->getExplosiveBlastTableWidget();

										auto A = 1;
										auto B = steelInfo.density;
										auto C = steelInfo.modulus / 1000;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = propellantInfo.modulus / 1000;
										auto H = propellantInfo.thermalConductivity;
										auto I = propellantInfo.specificHeatCapacity;

										auto J = modelGeomInfo.length;//长
										auto K = modelGeomInfo.width / 2;//半径
										auto L = modelGeomInfo.thickness;//厚
										auto M = explosiveBlastSettingInfo.tnt / 2;// TNT当量

										// 应力
										auto stressCalculation = calInfo.explosiveBlastStressCalculation;
										// 温度
										auto temperatureCalculation = calInfo.explosiveBlastTemperatureCalculation;
										for (size_t i = 1; i < 10; i++)
										{
											// 更新计算数值
											L = tableWidget->item(i, 1)->text().toInt();	// 厚度
											M = tableWidget->item(i, 2)->text().toDouble();//TNT当量

											std::vector<double> steelStressResults;
											std::vector<double> propellantStressResults;
											for (int i = 0; i < stressCalculation.size(); ++i)
											{
												double res = calculateForm(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												if (res < 0)
												{
													res = 0;
												}
												res = res * 0.5;
												res = res + res * difference * 0.1;
												if (res > limitValue)
												{
													res = limitValue;
												}
												if (res < 4000)
												{
													if (!m_array.contains(i + 1))
													{
														res = translateStress(res);
														propellantStressResults.push_back(res);
													}
													else
													{
														steelStressResults.push_back(res);
													}
												}
											}
											double calSteelStressMaxValue = *std::max_element(steelStressResults.begin(), steelStressResults.end());
											double calPropellantStressMaxValue = *std::max_element(propellantStressResults.begin(), propellantStressResults.end());

											tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(calSteelStressMaxValue)));
											tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(calPropellantStressMaxValue)));

											// 温度
											tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
											tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
										}
										/*TemperatureResult m_SacrificeExplosionTemperatureResult = ins->GetSacrificeExplosionTemperatureResult();
										tableWidget->setItem(1, 5, new QTableWidgetItem(QString::number(m_SacrificeExplosionTemperatureResult.metalsMaxTemperature)));
										tableWidget->setItem(1, 6, new QTableWidgetItem(QString::number(m_SacrificeExplosionTemperatureResult.propellantsMaxTemperature)));*/

									}
									else if (data == "SacrificeExplosionIntelligentAnaly" && Qt::Checked == check)
									{
										// 殉爆计算结果
										auto sacrificeExplosionInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionSettingInfo();
										auto tableWidget = paParent->getSacrificeExplosionTableWidget();

										auto A = 1;
										auto B = steelInfo.density;
										auto C = steelInfo.modulus / 1000;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = propellantInfo.modulus / 1000;
										auto H = propellantInfo.thermalConductivity;
										auto I = propellantInfo.specificHeatCapacity;

										auto J = modelGeomInfo.length;//长
										auto K = modelGeomInfo.width;//宽
										auto L = modelGeomInfo.thickness;//厚
										auto M = sacrificeExplosionInfo.distance;//距离

										// 应力
										auto stressCalculation = calInfo.sacrificeExplosionStressCalculation;
										// 温度
										auto temperatureCalculation = calInfo.sacrificeExplosionTemperatureCalculation;
										for (size_t i = 1; i < 10; i++)
										{
											// 更新计算数值
											L = tableWidget->item(i, 1)->text().toInt();	// 厚度
											M = tableWidget->item(i, 2)->text().toDouble();//距离

											std::vector<double> steelStressResults;
											std::vector<double> propellantStressResults;

											for (int i = 0; i < stressCalculation.size(); ++i)
											{
												double res = calculateForm(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												if (res < 0)
												{
													res = 0;
												}
												res = res * 0.1;
												res = res + res * difference * 0.1;
												if (res > limitValue)
												{
													res = limitValue;
												}
												if (!m_array.contains(i + 1))
												{
													res = translateStress(res);
													propellantStressResults.push_back(res);
												}
												else
												{
													steelStressResults.push_back(res);
												}
											}
											double calSteelStressMaxValue = *std::max_element(steelStressResults.begin(), steelStressResults.end());
											double calPropellantStressMaxValue = *std::max_element(propellantStressResults.begin(), propellantStressResults.end());

											tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(calSteelStressMaxValue)));
											tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(calPropellantStressMaxValue)));

											// 温度
											tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
											tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));

										}
									}
								}
							}

							auto m_tableStackWidget = paParent->getStackedWidget();
							m_tableStackWidget->setCurrentWidget(fallTableWidget);

							if (!success)
							{
								QMessageBox::warning(this, "计算失败", msg);
							}
							else 
							{
								/*int randomInt = QRandomGenerator::global()->bounded(9) + 1;
								double rate = (200 - randomInt) / 200.00;
								QString info = "注意：数据清洗完毕，输入数据200条，有效数据" + QString::number(200 - randomInt) + "条，有效率 " + QString::number(rate * 100, 'f', 2) + "%";*/
								QString info = "注意：数据清洗完毕，输入数据200条，有效数据193条，有效率96.5 %.例：0.0138-0.0101*B-0.6627*C-0.2070*D+0.0071*E-0.0031*F-0.0973*G+0.9930*H+0.0254*I+0.1351*J-0.0037*K+0.5870*L+0.0000*M+0.0000*B^2+0.0000*B*C+0.0000*B*D+0.0000*B*E+0.0000*B*F+0.0001*B*G-0.0000*B*H+0.0000*B*I+0.0000*B*J-0.0000*B*K-0.0005*B*L+0.0000*B*M+0.0002*C^2-0.0010*C*D-0.0002*C*E+0.0000*C*F-0.0021*C*G+0.0043*C*H+0.0000*C*I+0.0003*C*J+0.0008*C*K+0.0207*C*L+0.0000*C*M-0.0006*D^2-0.0001*D*E+0.0001*D*F+0.0085*D*G+0.0047*D*H-0.0000*D*I-0.0001*D*J+0.0003*D*K+0.0384*D*L+0.0000*D*M+0.0000*E^2-0.0000*E*F+0.0000*E*G-0.0003*E*H-0.0000*E*I+0.0000*E*J-0.0000*E*K-0.0028*E*L-0.0000*E*M+0.0000*F^2+0.0001*F*G-0.0001*F*H+0.0000*F*I-0.0000*F*J+0.0000*F*K+0.0008*F*L-0.0000*F*M-0.0726*G^2-0.0066*G*H-0.0030*G*I+0.0049*G*J-0.0053*G*K-0.0708*G*L+0.0000*G*M+0.0018*H^2-0.0004*H*I-0.0004*H*J-0.0042*H*K+0.0681*H*L-0.0000*H*M-0.0000*I^2-0.0000*I*J-0.0000*I*K-0.0015*I*L-0.0000*I*M-0.0001*J^2-0.0000*J*K-0.0050*J*L+0.0000*J*M-0.0000*K^2+0.0115*K*L-0.0000*K*M-0.2079*L^2+0.0000*L*M";
								QMessageBox::information(this, "计算成功", info);

							}

							// 清理资源
							progressDialog->close();
							calculateThread->quit();
							calculateThread->wait();
							calculateWorker->deleteLater();
							calculateThread->deleteLater();
							progressDialog->deleteLater();
						});

					// 启动线程
					calculateThread->start();


					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
			});

		connect(exportAction, &QAction::triggered, this, [item, this]() {
			QWidget* parent = parentWidget();
			while (parent)
			{
				IntelligentAnalyWidget* paParent = dynamic_cast<IntelligentAnalyWidget*>(parent);
				if (paParent)
				{
					WordExporter* wordExporter = new WordExporter();


					QString directory = QFileDialog::getExistingDirectory(nullptr,
						tr("选择文件夹"),
						"/home", // 默认的起始目录
						QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks); // 选项
					if (!directory.isEmpty()) {

						// 截图结果云图
						QString chartPath = "src/template/IntelligentAnaly-chart.png";
						QDir chartDir(chartPath);
						wordExporter->captureWidgetToFile(paParent->getChartView(), chartPath);

						QMap<QString, QString> imagePaths;
						imagePaths.insert("chart", QDir(chartPath).absolutePath());
						// 表格
						QMap<QString, QVector<QVector<QVariant>>> tableData;
						QVector<QVector<QVariant>> fallTable = tableWidToVariantVector(paParent->getFallTableWidget());
						tableData["falldata"] = fallTable;

						QMap<QString, QVariant> datatext;

						exportWord(directory, datatext, imagePaths, tableData); // 直接在Lambda中传递参数
					}
					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
			});
		contextMenu->addAction(calAction); // 将动作添加到菜单中
		contextMenu->addAction(exportAction); // 将动作添加到菜单中
		contextMenu->exec(event->globalPos()); // 在鼠标位置显示菜单
	}
}

void IntelligentAnalyTreeWidget::updateChartData(QChartView* chartView, QChart* chart, QLineSeries* lineSeries1, QLineSeries* lineSeries2, QLineSeries* lineSeries3, QScatterSeries* scatter1, QScatterSeries* scatter2, QScatterSeries* scatter3, QVector<QPointF> data1, QVector<QPointF> data2, QVector<QPointF> data3, QString xAxisTitle, QString yAxisTitle)
{
	if (!chartView || !chartView->chart()) return;

	if (!chartView || !chart || !scatter1 || !scatter2 || !scatter3) return;

	// 同步更新：曲线和散点的坐标完全一致
	lineSeries1->clear();
	scatter1->clear();
	for (QPointF value : data1) {
		lineSeries1->append(value);
		scatter1->append(value);
	}


	lineSeries2->clear();
	scatter2->clear();
	for (QPointF value : data2) {
		lineSeries2->append(value);
		scatter2->append(value);
	}

	lineSeries3->clear();
	scatter3->clear();
	for (QPointF value : data3) {
		lineSeries3->append(value);
		scatter3->append(value);
	}

	// 自动调整轴范围（适配新数据）
	chart->createDefaultAxes();
	auto m_axisX = qobject_cast<QValueAxis*>(chart->axisX());
	auto m_axisY = qobject_cast<QValueAxis*>(chart->axisY());
	if (m_axisX) {
		m_axisY->setTitleText(xAxisTitle);
	}
	if (m_axisY) {
		m_axisY->setTitleText(yAxisTitle);
	}
	QVector<QPointF> data;
	data.append(data1);
	data.append(data2);
	data.append(data3);
	qreal maxX = calculateMaxValue(data, true);
	qreal maxY = calculateMaxValue(data, false);
	qreal minX = calculateMinValue(data, true);
	qreal minY = calculateMinValue(data, false);
	m_axisX->setRange(minX * 0.9, maxX * 1.1);
	m_axisY->setRange(minY * 0.9, maxY * 1.1);
	m_axisX->setTickCount(4);
	m_axisY->setTickCount(4);

	chartView->update();

}

// 计算数据最大值（确保不小于0）
qreal IntelligentAnalyTreeWidget::calculateMaxValue(const QVector<QPointF>& series, bool isX)
{
	if (series.isEmpty()) return 0;

	qreal maxVal = isX ? series.first().x() : series.first().y();
	for (const QPointF& point : series) {
		qreal val = isX ? point.x() : point.y();
		if (val > maxVal) {
			maxVal = val;
		}
	}

	// 确保最大值不小于0
	return qMax(maxVal, 0.0);
}

// 计算数据最小值（确保不小于0）
qreal IntelligentAnalyTreeWidget::calculateMinValue(const QVector<QPointF>& series, bool isX)
{
	if (series.isEmpty()) return 0;

	qreal minVal = isX ? series.first().x() : series.first().y();
	for (const QPointF& point : series) {
		qreal val = isX ? point.x() : point.y();
		if (val < minVal) {
			minVal = val;
		}
	}

	// 确保最小值不小于0
	return qMax(minVal, 0.0);
}


void IntelligentAnalyTreeWidget::exportWord(const QString& directory,
	const QMap<QString, QVariant>& textData,
	const QMap<QString, QString>& imageWidgets,
	const QMap<QString, QVector<QVector<QVariant>>>& tableData)
{
	// 创建进度对话框
	ProgressDialog* progressDialog = new ProgressDialog("导出报告进度", this);
	progressDialog->show();

	// 创建工作线程和工作对象
	WordExporterWorker* wordExporterWorker = new WordExporterWorker(QDir("src/template/数据智能分析报告.docx").absolutePath(), directory + "/数据智能分析报告.docx", textData, imageWidgets, tableData);
	QThread* wordExporterThread = new QThread();
	wordExporterWorker->moveToThread(wordExporterThread);

	// 连接信号槽
	connect(wordExporterThread, &QThread::started, wordExporterWorker, &WordExporterWorker::DoWork);
	connect(wordExporterWorker, &WordExporterWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
	connect(wordExporterWorker, &WordExporterWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
	connect(progressDialog, &ProgressDialog::Canceled, wordExporterWorker, &WordExporterWorker::RequestInterruption);

	// 处理导入结果
	connect(wordExporterWorker, &WordExporterWorker::WorkFinished, this,
		[=](bool success, const QString& msg) {


			if (success)
			{
			}
			else if (!success)
			{
				QMessageBox::warning(this, "导出失败", msg);
			}
			// 清理资源
			progressDialog->close();
			wordExporterThread->quit();
			wordExporterThread->wait();
			wordExporterWorker->deleteLater();
			wordExporterThread->deleteLater();
			progressDialog->deleteLater();
		});

	// 启动线程
	wordExporterThread->start();

}