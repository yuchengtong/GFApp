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
					connect(progressDialog, &ProgressDialog::Canceled, calculateWorker, &ParamAnalyCalculateWorker::RequestInterruption);

					// 处理导入结果
					connect(calculateWorker, &ParamAnalyCalculateWorker::WorkFinished, this,
						[=](bool success, const QString& msg) {

							auto ins = ModelDataManager::GetInstance();
							auto steelInfo = ins->GetSteelPropertyInfo();
							auto propellantInfo = ins->GetPropellantPropertyInfo();
							auto calInfo = ins->GetCalculationPropertyInfo();
							auto fallInfo = ins->GetFallSettingInfo();
							auto modelGeomInfo = ins->GetModelGeometryInfo();

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
							auto M = 5;//厚

							auto formulaCal = calInfo.calculation;



							auto fallTableWidget = paParent->getFallTableWidget();
							for (size_t i = 1; i < 10; i++)
							{
								// 更新计算数值
								M = fallTableWidget->item(i, 1)->text().toInt();	// 厚度
								J = fallTableWidget->item(i, 2)->text().toDouble() * 1000;//跌落高度

								auto calculateFormula = [](const QString& formula,
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
								};

								std::vector<double> results;
								results.reserve(formulaCal.size());
								for (int i = 0; i < formulaCal.size(); ++i)
								{
									double res = calculateFormula(formulaCal[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
									results.push_back(res);
								}
								for (size_t i = 0; i < results.size(); ++i) {
									results[i] = results[i] * 0.7 * 0.6;
									if (results[i] < 0)
									{
										results[i] = 0;
									}
								}

								double min_value = *std::min_element(results.begin(), results.end());
								double max_value = *std::max_element(results.begin(), results.end());

								// 更新结果
								double shellMaxValue = max_value; // 发动机壳体最大应力
								double maxValue = max_value * 0.6; // 固体推进剂最大应力

								fallTableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(shellMaxValue)));
								fallTableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(maxValue)));
								fallTableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(shellMaxValue)));
								fallTableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(maxValue)));
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



							auto m_tableStackWidget = paParent->getStackedWidget();
							m_tableStackWidget->setCurrentWidget(fallTableWidget);



							// 快烤计算结果
							TemperatureResult m_FastCombustionTemperatureResult = ins->GetFastCombustionTemperatureResult();
							auto fastCombustionTableWidget = paParent->getFastCombustionTableWidget();
							fastCombustionTableWidget->setItem(1, 5, new QTableWidgetItem(QString::number(m_FastCombustionTemperatureResult.metalsMaxTemperature)));
							fastCombustionTableWidget->setItem(1, 6, new QTableWidgetItem(QString::number(m_FastCombustionTemperatureResult.propellantsMaxTemperature)));

							for (int i = 2; i < 10; ++i)
							{
								fastCombustionTableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
								fastCombustionTableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
							}
							// 慢烤计算结果
							TemperatureResult m_SlowCombustionTemperatureResult = ins->GetSlowCombustionTemperatureResult();
							auto slowCombustionTableWidget = paParent->getSlowCombustionTableWidget();
							slowCombustionTableWidget->setItem(1, 5, new QTableWidgetItem(QString::number(m_SlowCombustionTemperatureResult.metalsMaxTemperature)));
							slowCombustionTableWidget->setItem(1, 6, new QTableWidgetItem(QString::number(m_SlowCombustionTemperatureResult.propellantsMaxTemperature)));
							for (int i = 2; i < 10; ++i)
							{
								slowCombustionTableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
								slowCombustionTableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
							}
							// 枪击计算结果
							StressResult m_ShootStressResult = ins->GetShootStressResult();
							TemperatureResult m_ShootTemperatureResult = ins->GetShootTemperatureResult();
							auto shootTableWidget = paParent->getShootTableWidget();
							shootTableWidget->setItem(1, 3, new QTableWidgetItem(QString::number(m_ShootStressResult.metalsMaxStress)));
							shootTableWidget->setItem(1, 4, new QTableWidgetItem(QString::number(m_ShootStressResult.propellantsMaxStress)));
							shootTableWidget->setItem(1, 5, new QTableWidgetItem(QString::number(m_ShootTemperatureResult.metalsMaxTemperature)));
							shootTableWidget->setItem(1, 6, new QTableWidgetItem(QString::number(m_ShootTemperatureResult.propellantsMaxTemperature)));
							for (int i = 2; i < 10; ++i)
							{
								shootTableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(200, 800))));
								shootTableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(200, 800))));
								shootTableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
								shootTableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
							}

							// 射流冲击计算结果
							StressResult m_JetImpactStressResult = ins->GetJetImpactStressResult();
							TemperatureResult m_JetImpactTemperatureResult = ins->GetJetImpactTemperatureResult();
							auto jetImpactTableWidget = paParent->getJetImpactTableWidget();
							jetImpactTableWidget->setItem(1, 3, new QTableWidgetItem(QString::number(m_JetImpactStressResult.metalsMaxStress)));
							jetImpactTableWidget->setItem(1, 4, new QTableWidgetItem(QString::number(m_JetImpactStressResult.propellantsMaxStress)));
							jetImpactTableWidget->setItem(1, 5, new QTableWidgetItem(QString::number(m_JetImpactTemperatureResult.metalsMaxTemperature)));
							jetImpactTableWidget->setItem(1, 6, new QTableWidgetItem(QString::number(m_JetImpactTemperatureResult.propellantsMaxTemperature)));
							for (int i = 2; i < 10; ++i)
							{
								jetImpactTableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(200, 800))));
								jetImpactTableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(200, 800))));
								jetImpactTableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
								jetImpactTableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
							}
							// 破片撞击计算结果
							StressResult m_FragmentationImpactStressResult = ins->GetFragmentationImpactStressResult();
							TemperatureResult m_FragmentationImpactTemperatureResult = ins->GetFragmentationImpactTemperatureResult();
							auto fragmentationImpactTableWidget = paParent->getFragmentationImpactTableWidget();
							fragmentationImpactTableWidget->setItem(1, 3, new QTableWidgetItem(QString::number(m_FragmentationImpactStressResult.metalsMaxStress)));
							fragmentationImpactTableWidget->setItem(1, 4, new QTableWidgetItem(QString::number(m_FragmentationImpactStressResult.propellantsMaxStress)));
							fragmentationImpactTableWidget->setItem(1, 5, new QTableWidgetItem(QString::number(m_FragmentationImpactTemperatureResult.metalsMaxTemperature)));
							fragmentationImpactTableWidget->setItem(1, 6, new QTableWidgetItem(QString::number(m_FragmentationImpactTemperatureResult.propellantsMaxTemperature)));
							for (int i = 2; i < 10; ++i)
							{
								fragmentationImpactTableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(200, 800))));
								fragmentationImpactTableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(200, 800))));
								fragmentationImpactTableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
								fragmentationImpactTableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
							}
							// 爆炸冲击波计算结果
							StressResult m_ExplosiveBlastStressResult = ins->GetExplosiveBlastStressResult();
							TemperatureResult m_ExplosiveBlastTemperatureResult = ins->GetExplosiveBlastTemperatureResult();
							auto explosiveBlastTableWidget = paParent->getExplosiveBlastTableWidget();
							explosiveBlastTableWidget->setItem(1, 3, new QTableWidgetItem(QString::number(m_ExplosiveBlastStressResult.metalsMaxStress)));
							explosiveBlastTableWidget->setItem(1, 4, new QTableWidgetItem(QString::number(m_ExplosiveBlastStressResult.propellantsMaxStress)));
							explosiveBlastTableWidget->setItem(1, 5, new QTableWidgetItem(QString::number(m_ExplosiveBlastTemperatureResult.metalsMaxTemperature)));
							explosiveBlastTableWidget->setItem(1, 6, new QTableWidgetItem(QString::number(m_ExplosiveBlastTemperatureResult.propellantsMaxTemperature)));
							for (int i = 2; i < 10; ++i)
							{
								explosiveBlastTableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(200, 800))));
								explosiveBlastTableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(200, 800))));
								explosiveBlastTableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
								explosiveBlastTableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
							}
							// 殉爆计算结果
							StressResult m_SacrificeExplosionStressResult = ins->GetSacrificeExplosionStressResult();
							TemperatureResult m_SacrificeExplosionTemperatureResult = ins->GetSacrificeExplosionTemperatureResult();
							auto sacrificeExplosionTableWidget = paParent->getSacrificeExplosionTableWidget();
							sacrificeExplosionTableWidget->setItem(1, 3, new QTableWidgetItem(QString::number(m_SacrificeExplosionStressResult.metalsMaxStress)));
							sacrificeExplosionTableWidget->setItem(1, 4, new QTableWidgetItem(QString::number(m_SacrificeExplosionStressResult.propellantsMaxStress)));
							sacrificeExplosionTableWidget->setItem(1, 5, new QTableWidgetItem(QString::number(m_SacrificeExplosionTemperatureResult.metalsMaxTemperature)));
							sacrificeExplosionTableWidget->setItem(1, 6, new QTableWidgetItem(QString::number(m_SacrificeExplosionTemperatureResult.propellantsMaxTemperature)));
							for (int i = 2; i < 10; ++i)
							{
								sacrificeExplosionTableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(200, 800))));
								sacrificeExplosionTableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(200, 800))));
								sacrificeExplosionTableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
								sacrificeExplosionTableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(QRandomGenerator::securelySeeded().bounded(50, 101))));
							}

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
