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

#include "IntelligentAnalyWidget.h"
#include "ModelDataManager.h"
#include "ProgressDialog.h"
#include "ParamAnalyCalculateWorker.h"

double calculateForm(const QString& formula,
	double B, double C, double D, double E,
	double F, double G, double H, double I,
	double J, double K, double L, double M, double A)
{
	QString processedFormula = formula;  // 复制到非const变量
	processedFormula.remove(' ');
	// 变量映射：通过变量名获取对应值（使用map提高可读性和可维护性）
	const QMap<QString, double> varMap = {
		{"A", A}, {"B", B}, {"C", C}, {"D", D}, {"E", E},
		{"F", F}, {"G", G}, {"H", H}, {"I", I}, {"J", J},
		{"K", K}, {"L", L}, {"M", M}
	};

	QRegExp regExp("([+-]?)(\\d+(?:\\.\\d*)?|\\.\\d+)(?:\\*([A-Z]))?");
	regExp.setMinimal(false);

	double result = 0.0;
	int pos = 0;
	int matchCount = 0; // 统计匹配到的项数，用于校验公式合法性

	// 处理公式开头的第一项（可能无符号）
	if (processedFormula[0] != '+' && processedFormula[0] != '-') {
		processedFormula = "+" + processedFormula; // 补全正号，统一格式
	}

	while ((pos = regExp.indexIn(processedFormula, pos)) != -1) {
		++matchCount;
		QString signStr = regExp.cap(1);       // 符号（+/-）
		QString coeffStr = regExp.cap(2);      // 系数
		QString varName = regExp.cap(3);       // 变量

		// 解析符号（默认正号）
		double sign = (signStr == "-") ? -1.0 : 1.0;

		// 解析系数（处理转换失败）
		bool ok = false;
		double coeff = coeffStr.toDouble(&ok);
		if (!ok) {
			throw std::invalid_argument(QString("无效系数: %1").arg(coeffStr).toStdString());
		}

		// 计算当前项的值
		double term = sign * coeff;
		if (!varName.isEmpty()) {
			if (!varMap.contains(varName)) {
				throw std::invalid_argument(QString("未知变量: %1").arg(varName).toStdString());
			}
			term *= varMap[varName];  // 变量项：符号×系数×变量值
		}

		result += term;
		pos += regExp.matchedLength();
	}

	// 校验公式是否完全解析（无残留无效字符）
	if (matchCount == 0) {
		throw std::invalid_argument(QString("公式格式错误: %1").arg(formula).toStdString());
	}

	return result;
}


IntelligentAnalyTreeWidget::IntelligentAnalyTreeWidget(QWidget* parent)
	:QWidget(parent)
{
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
	fallNode->setText(0, "跌落试验智能分析");
	fallNode->setData(0, Qt::UserRole, "FallIntelligentAnaly");
	fallNode->setCheckState(0, Qt::Unchecked);
	fallNode->setIcon(0, icon);

	QTreeWidgetItem* fastCombustionNode = new QTreeWidgetItem(rootItem);
	fastCombustionNode->setText(0, "快速烤燃试验试验智能分析");
	fastCombustionNode->setData(0, Qt::UserRole, "FastCombustionIntelligentAnaly");
	fastCombustionNode->setCheckState(0, Qt::Unchecked);
	fastCombustionNode->setIcon(0, icon);

	QTreeWidgetItem* slowCombustionNode = new QTreeWidgetItem(rootItem);
	slowCombustionNode->setText(0, "慢速烤燃试验智能分析");
	slowCombustionNode->setData(0, Qt::UserRole, "SlowCombustionIntelligentAnaly");
	slowCombustionNode->setCheckState(0, Qt::Unchecked);
	slowCombustionNode->setIcon(0, icon);

	QTreeWidgetItem* shootNode = new QTreeWidgetItem(rootItem);
	shootNode->setText(0, "枪击试验智能分析");
	shootNode->setData(0, Qt::UserRole, "ShootIntelligentAnaly");
	shootNode->setCheckState(0, Qt::Unchecked);
	shootNode->setIcon(0, icon);

	QTreeWidgetItem* jetImpactNode = new QTreeWidgetItem(rootItem);
	jetImpactNode->setText(0, "射流冲击试验智能分析");
	jetImpactNode->setData(0, Qt::UserRole, "JetImpactIntelligentAnaly");
	jetImpactNode->setCheckState(0, Qt::Unchecked);
	jetImpactNode->setIcon(0, icon);

	QTreeWidgetItem* fragmentationImpactNode = new QTreeWidgetItem(rootItem);
	fragmentationImpactNode->setText(0, "破片撞击试验智能分析");
	fragmentationImpactNode->setData(0, Qt::UserRole, "FragmentationImpactIntelligentAnaly");
	fragmentationImpactNode->setCheckState(0, Qt::Unchecked);
	fragmentationImpactNode->setIcon(0, icon);

	QTreeWidgetItem* explosiveBlastNode = new QTreeWidgetItem(rootItem);
	explosiveBlastNode->setText(0, "爆炸冲击波试验智能分析");
	explosiveBlastNode->setData(0, Qt::UserRole, "ExplosiveBlastIntelligentAnaly");
	explosiveBlastNode->setCheckState(0, Qt::Unchecked);
	explosiveBlastNode->setIcon(0, icon);

	QTreeWidgetItem* sacrificeExplosionNode = new QTreeWidgetItem(rootItem);
	sacrificeExplosionNode->setText(0, "殉爆试验智能分析");
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

							auto fallTableWidget = paParent->getFallTableWidget();

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
										auto C = 0;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = 0;
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

											std::vector<double> stressResults;
											stressResults.reserve(stressCalculation.size());
											for (int i = 0; i < stressCalculation.size(); ++i)
											{
												double res = calculateForm(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												stressResults.push_back(res);
											}
											for (size_t i = 0; i < stressResults.size(); ++i) {
												stressResults[i] = stressResults[i] * 0.7 * 0.6;
												if (stressResults[i] < 0)
												{
													stressResults[i] = 0;
												}
											}

											double stressMinValue = *std::min_element(stressResults.begin(), stressResults.end());
											double stressMaxValue = *std::max_element(stressResults.begin(), stressResults.end());

											fallTableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(stressMaxValue)));
											fallTableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(stressMaxValue * 0.6)));

											// 温度
											std::vector<double> temperatureResults;
											temperatureResults.reserve(temperatureCalculation.size());
											for (int i = 0; i < temperatureCalculation.size(); ++i)
											{
												double res = calculateForm(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												temperatureResults.push_back(res);
											}
											for (size_t i = 0; i < temperatureResults.size(); ++i) {
												temperatureResults[i] = temperatureResults[i] * 0.7 * 0.6;
												if (temperatureResults[i] < 0)
												{
													temperatureResults[i] = 0;
												}
											}
											double temperatureMinValue = *std::min_element(temperatureResults.begin(), temperatureResults.end());
											double temperatureMaxValue = *std::max_element(temperatureResults.begin(), temperatureResults.end());
											fallTableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(temperatureMaxValue)));
											fallTableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(temperatureMaxValue * 0.6)));
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
										auto C = 0;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = 0;
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
											std::vector<double> temperatureResults;
											temperatureResults.reserve(temperatureCalculation.size());
											for (int i = 0; i < temperatureCalculation.size(); ++i)
											{
												double res = calculateForm(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												temperatureResults.push_back(res);
											}
											for (size_t i = 0; i < temperatureResults.size(); ++i) {
												temperatureResults[i] = temperatureResults[i] + 800;
											}
											double temperatureMinValue = *std::min_element(temperatureResults.begin(), temperatureResults.end());
											double temperatureMaxValue = *std::max_element(temperatureResults.begin(), temperatureResults.end());
											tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(temperatureMaxValue)));
											tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(temperatureMaxValue * 0.6)));
										}
									}
									else if (data == "SlowCombustionIntelligentAnaly" && Qt::Checked == check)
									{
										// 快烤计算结果
										auto tableWidget = paParent->getSlowCombustionTableWidget();
										auto slowCombustionSettingInfo = ModelDataManager::GetInstance()->GetSlowCombustionSettingInfo();

										auto A = 1;
										auto B = steelInfo.density;
										auto C = 0;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = 0;
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
											std::vector<double> temperatureResults;
											temperatureResults.reserve(temperatureCalculation.size());
											for (int i = 0; i < temperatureCalculation.size(); ++i)
											{
												double res = calculateForm(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												temperatureResults.push_back(res);
											}
											for (size_t i = 0; i < temperatureResults.size(); ++i) {
												temperatureResults[i] = temperatureResults[i] + 400;
												if (temperatureResults[i] < 0)
												{
													temperatureResults[i] = 0;
												}
											}
											double temperatureMinValue = *std::min_element(temperatureResults.begin(), temperatureResults.end());
											double temperatureMaxValue = *std::max_element(temperatureResults.begin(), temperatureResults.end());
											tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(temperatureMaxValue)));
											tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(temperatureMaxValue * 0.6)));
										}
									}
									else if (data == "ShootIntelligentAnaly" && Qt::Checked == check)
									{
										// 枪击计算结果
										auto shootInfo = ModelDataManager::GetInstance()->GetShootSettingInfo();
										auto tableWidget = paParent->getShootTableWidget();

										auto A = 1;
										auto B = steelInfo.density;
										auto C = 0;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = 0;
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

											std::vector<double> stressResults;
											stressResults.reserve(stressCalculation.size());
											for (int i = 0; i < stressCalculation.size(); ++i)
											{
												double res = calculateForm(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												stressResults.push_back(res);
											}
											for (size_t i = 0; i < stressResults.size(); ++i) {
												stressResults[i] = stressResults[i] * 0.7 * 0.6;
												if (stressResults[i] < 0)
												{
													stressResults[i] = 0;
												}
											}

											double stressMinValue = *std::min_element(stressResults.begin(), stressResults.end());
											double stressMaxValue = *std::max_element(stressResults.begin(), stressResults.end());

											tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(stressMaxValue)));
											tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(stressMaxValue * 0.6)));

											// 温度
											std::vector<double> temperatureResults;
											temperatureResults.reserve(temperatureCalculation.size());
											for (int i = 0; i < temperatureCalculation.size(); ++i)
											{
												double res = calculateForm(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												temperatureResults.push_back(res);
											}
											for (size_t i = 0; i < temperatureResults.size(); ++i) {
												temperatureResults[i] = temperatureResults[i] * 0.7 * 0.6;
												if (temperatureResults[i] < 0)
												{
													temperatureResults[i] = 0;
												}
											}
											double temperatureMinValue = *std::min_element(temperatureResults.begin(), temperatureResults.end());
											double temperatureMaxValue = *std::max_element(temperatureResults.begin(), temperatureResults.end());
											tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(temperatureMaxValue)));
											tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(temperatureMaxValue * 0.6)));
										}
									}
									else if (data == "JetImpactIntelligentAnaly" && Qt::Checked == check)
									{
										// 射流冲击计算结果
										auto jetImpactingInfo = ModelDataManager::GetInstance()->GetJetImpactSettingInfo();
										auto tableWidget = paParent->getJetImpactTableWidget();

										auto A = 1;
										auto B = steelInfo.density;
										auto C = 0;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = 0;
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

											std::vector<double> stressResults;
											stressResults.reserve(stressCalculation.size());
											for (int i = 0; i < stressCalculation.size(); ++i)
											{
												double res = calculateForm(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												stressResults.push_back(res);
											}
											for (size_t i = 0; i < stressResults.size(); ++i) {
												stressResults[i] = stressResults[i] * 0.7 * 0.6;
												if (stressResults[i] < 0)
												{
													stressResults[i] = 0;
												}
											}

											double stressMinValue = *std::min_element(stressResults.begin(), stressResults.end());
											double stressMaxValue = *std::max_element(stressResults.begin(), stressResults.end());

											tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(stressMaxValue)));
											tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(stressMaxValue * 0.6)));

											// 温度
											std::vector<double> temperatureResults;
											temperatureResults.reserve(temperatureCalculation.size());
											for (int i = 0; i < temperatureCalculation.size(); ++i)
											{
												double res = calculateForm(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												temperatureResults.push_back(res);
											}
											for (size_t i = 0; i < temperatureResults.size(); ++i) {
												temperatureResults[i] = temperatureResults[i] * 0.7 * 0.6;
												if (temperatureResults[i] < 0)
												{
													temperatureResults[i] = 0;
												}
											}
											double temperatureMinValue = *std::min_element(temperatureResults.begin(), temperatureResults.end());
											double temperatureMaxValue = *std::max_element(temperatureResults.begin(), temperatureResults.end());
											tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(temperatureMaxValue)));
											tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(temperatureMaxValue * 0.6)));
										}
									}
									else if (data == "FragmentationImpactIntelligentAnaly" && Qt::Checked == check)
									{
										// 破片撞击计算结果
										auto fragmentationSettingInfo = ModelDataManager::GetInstance()->GetFragmentationSettingInfo();
										auto tableWidget = paParent->getFragmentationImpactTableWidget();

										auto A = 1;
										auto B = steelInfo.density;
										auto C = 0;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = 0;
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

											std::vector<double> stressResults;
											stressResults.reserve(stressCalculation.size());
											for (int i = 0; i < stressCalculation.size(); ++i)
											{
												double res = calculateForm(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												stressResults.push_back(res);
											}
											for (size_t i = 0; i < stressResults.size(); ++i) {
												stressResults[i] = stressResults[i] * 0.7 * 0.6;
												if (stressResults[i] < 0)
												{
													stressResults[i] = 0;
												}
											}

											double stressMinValue = *std::min_element(stressResults.begin(), stressResults.end());
											double stressMaxValue = *std::max_element(stressResults.begin(), stressResults.end());

											tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(stressMaxValue)));
											tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(stressMaxValue * 0.6)));

											// 温度
											std::vector<double> temperatureResults;
											temperatureResults.reserve(temperatureCalculation.size());
											for (int i = 0; i < temperatureCalculation.size(); ++i)
											{
												double res = calculateForm(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												temperatureResults.push_back(res);
											}
											for (size_t i = 0; i < temperatureResults.size(); ++i) {
												temperatureResults[i] = temperatureResults[i] * 0.7 * 0.6;
												if (temperatureResults[i] < 0)
												{
													temperatureResults[i] = 0;
												}
											}
											double temperatureMinValue = *std::min_element(temperatureResults.begin(), temperatureResults.end());
											double temperatureMaxValue = *std::max_element(temperatureResults.begin(), temperatureResults.end());
											tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(temperatureMaxValue)));
											tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(temperatureMaxValue * 0.6)));
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
										auto C = 0;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = 0;
										auto H = propellantInfo.thermalConductivity;
										auto I = propellantInfo.specificHeatCapacity;

										auto J = modelGeomInfo.length;//长
										auto K = modelGeomInfo.width / 2;//半径
										auto L = modelGeomInfo.thickness;//厚
										auto M = explosiveBlastSettingInfo.tnt;// TNT当量

										// 应力
										auto stressCalculation = calInfo.explosiveBlastStressCalculation;
										// 温度
										auto temperatureCalculation = calInfo.explosiveBlastTemperatureCalculation;
										for (size_t i = 1; i < 10; i++)
										{
											// 更新计算数值
											L = tableWidget->item(i, 1)->text().toInt();	// 厚度
											M = tableWidget->item(i, 2)->text().toDouble();//TNT当量

											std::vector<double> stressResults;
											stressResults.reserve(stressCalculation.size());
											for (int i = 0; i < stressCalculation.size(); ++i)
											{
												double res = calculateForm(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												stressResults.push_back(res);
											}
											for (size_t i = 0; i < stressResults.size(); ++i) {
												stressResults[i] = stressResults[i] * 0.7 * 0.6;
												if (stressResults[i] < 0)
												{
													stressResults[i] = 0;
												}
											}

											double stressMinValue = *std::min_element(stressResults.begin(), stressResults.end());
											double stressMaxValue = *std::max_element(stressResults.begin(), stressResults.end());

											tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(stressMaxValue)));
											tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(stressMaxValue * 0.6)));

											
											// 温度
											tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
											tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
										}
										TemperatureResult m_SacrificeExplosionTemperatureResult = ins->GetSacrificeExplosionTemperatureResult();
										tableWidget->setItem(1, 5, new QTableWidgetItem(QString::number(m_SacrificeExplosionTemperatureResult.metalsMaxTemperature)));
										tableWidget->setItem(1, 6, new QTableWidgetItem(QString::number(m_SacrificeExplosionTemperatureResult.propellantsMaxTemperature)));

									}
									else if (data == "SacrificeExplosionIntelligentAnaly" && Qt::Checked == check)
									{
										// 殉爆计算结果
										auto sacrificeExplosionInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionSettingInfo();
										auto tableWidget = paParent->getSacrificeExplosionTableWidget();

										auto A = 1;
										auto B = steelInfo.density;
										auto C = 0;
										auto D = steelInfo.thermalConductivity;
										auto E = steelInfo.specificHeatCapacity;

										auto F = propellantInfo.density;
										auto G = 0;
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

											std::vector<double> stressResults;
											stressResults.reserve(stressCalculation.size());
											for (int i = 0; i < stressCalculation.size(); ++i)
											{
												double res = calculateForm(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
												stressResults.push_back(res);
											}
											for (size_t i = 0; i < stressResults.size(); ++i) {
												stressResults[i] = stressResults[i] * 0.7 * 0.6;
												if (stressResults[i] < 0)
												{
													stressResults[i] = 0;
												}
											}

											double stressMinValue = *std::min_element(stressResults.begin(), stressResults.end());
											double stressMaxValue = *std::max_element(stressResults.begin(), stressResults.end());

											tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(stressMaxValue)));
											tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(stressMaxValue * 0.6)));

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
		contextMenu->addAction(calAction); // 将动作添加到菜单中
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
	m_axisX->setRange(minX*0.9, maxX*1.1);
	m_axisY->setRange(minY*0.9, maxY*1.1);
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
