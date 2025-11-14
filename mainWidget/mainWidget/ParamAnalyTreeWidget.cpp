#pragma execution_character_set("utf-8")
#include "ParamAnalyTreeWidget.h"
#include "ParamAnalyWidget.h"
#include "ProgressDialog.h"
#include "ParamAnalyCalculateWorker.h"
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
#include <iostream>
#include <algorithm>
#include <QThread>

// vector转String
QString vectorToString(const vector<double>& vec, int precision) {
	QStringList parts;

	for (double value : vec) {
		parts.append(QString::number(value, 'f', precision)); // 'f'表示固定小数格式，4位小数
	}

	return "[" + parts.join(", ") + "]";
}

vector<double> getMembership(const double& data, const double& one, const double& two, const double& three) {
	vector<double> result;
	if (data <= one)
	{
		result.push_back(1);
		result.push_back(0);
		result.push_back(0);
		result.push_back(0);
	}
	else if (data > one && data <= two)
	{
		result.push_back(0);
		QString valueStr = QString::number((data - one) / (two - one), 'f', 2);
		double temp = valueStr.toDouble();
		result.push_back(temp);
		result.push_back(1- temp);
		result.push_back(0);
	}
	else if (data > two && data <= three)
	{
		result.push_back(0);
		QString valueStr = QString::number((data - two) / (three - two), 'f', 2);
		double temp = valueStr.toDouble();
		result.push_back(temp);
		result.push_back(1 - temp);
		result.push_back(0);
	}
	else 
	{
		result.push_back(0);
		result.push_back(0);
		result.push_back(0);
		result.push_back(1);
	}
	return result;
}
// 根据隶属度向量计算分数
double getPoint(vector<double> data)
{
	vector<double> scoreVector = { 95,80,65,30 };
	double score = 0.0;
	for (size_t i = 0; i < scoreVector.size(); ++i) {
		score = score + scoreVector[i] * data[i];
	}
	return score;
}

ParamAnalyTreeWidget::ParamAnalyTreeWidget(QWidget* parent)
	:QWidget(parent)
{
	QIcon error_icon(":/src/Error.svg");
	QIcon checked_icon(":/src/Checked.svg");
	QIcon icon(":/src/data_calculation.svg");

	treeWidget = new GFTreeWidget(this);
	treeWidget->setColumnCount(1);
	treeWidget->setHeaderLabels({ "发动机AHP安全性分析与评估" });
	treeWidget->setHeaderHidden(true);
	treeWidget->setStyleSheet("QTreeWidget { color: black; }");
	treeWidget->setColumnCount(1);

	// 创建根节点
	QTreeWidgetItem* rootItem = new QTreeWidgetItem(treeWidget);
	rootItem->setText(0, "发动机AHP安全性分析与评估");
	rootItem->setData(0, Qt::UserRole, "ParamAnaly");
	rootItem->setExpanded(true);
	rootItem->setIcon(0, icon);

	QTreeWidgetItem* evaluationSystemNode = new QTreeWidgetItem(rootItem);
	evaluationSystemNode->setText(0, "三级评价体系设计");
	evaluationSystemNode->setData(0, Qt::UserRole, "EvaluationSystem");
	evaluationSystemNode->setIcon(0, icon);

	QTreeWidgetItem* weightNode = new QTreeWidgetItem(rootItem);
	weightNode->setText(0, "层级指标权重设计");
	weightNode->setData(0, Qt::UserRole, "Weight");
	weightNode->setIcon(0, icon);

	QTreeWidgetItem* u1Weight = new QTreeWidgetItem();
	u1Weight->setText(0, "U1指标权重设计");
	u1Weight->setData(0, Qt::UserRole, "U1Weight");
	u1Weight->setIcon(0, icon);

	QTreeWidgetItem* u2Weight = new QTreeWidgetItem();
	u2Weight->setText(0, "U2指标权重设计");
	u2Weight->setData(0, Qt::UserRole, "U2Weight");
	u2Weight->setIcon(0, icon);

	weightNode->addChild(u1Weight);
	weightNode->addChild(u2Weight);


	QTreeWidgetItem* calculationNode = new QTreeWidgetItem(rootItem);
	calculationNode->setText(0, "权重向量与一致性计算");
	calculationNode->setData(0, Qt::UserRole, "Calculation");
	calculationNode->setIcon(0, icon);

	QTreeWidgetItem* u1Calculation = new QTreeWidgetItem();
	u1Calculation->setText(0, "U1权重向量A1与一致性Rc1计算");
	u1Calculation->setData(0, Qt::UserRole, "U1Calculation");
	u1Calculation->setIcon(0, icon);

	QTreeWidgetItem* u2Calculation = new QTreeWidgetItem();
	u2Calculation->setText(0, "U2权重向量A2与一致性Rc2计算");
	u2Calculation->setData(0, Qt::UserRole, "U2Calculation");
	u2Calculation->setIcon(0, icon);

	calculationNode->addChild(u1Calculation);
	calculationNode->addChild(u2Calculation);


	QTreeWidgetItem* gradeDefinitionNode = new QTreeWidgetItem(rootItem);
	gradeDefinitionNode->setText(0, "评语集与等级定义");
	gradeDefinitionNode->setData(0, Qt::UserRole, "GradeDefinition");
	gradeDefinitionNode->setIcon(0, icon);

	QTreeWidgetItem* u1GradeDefinition = new QTreeWidgetItem();
	u1GradeDefinition->setText(0, "U1等级定量标准");
	u1GradeDefinition->setData(0, Qt::UserRole, "u1GradeDefinition");
	u1GradeDefinition->setIcon(0, icon);

	QTreeWidgetItem* u2GradeDefinition = new QTreeWidgetItem();
	u2GradeDefinition->setText(0, "U2等级定量标准");
	u2GradeDefinition->setData(0, Qt::UserRole, "u2GradeDefinition");
	u2GradeDefinition->setIcon(0, icon);

	gradeDefinitionNode->addChild(u1GradeDefinition);
	gradeDefinitionNode->addChild(u2GradeDefinition);



	QTreeWidgetItem* evaluationMatrixNode = new QTreeWidgetItem(rootItem);
	evaluationMatrixNode->setText(0, "模糊评判矩阵");
	evaluationMatrixNode->setData(0, Qt::UserRole, "EvaluationMatrix");
	evaluationMatrixNode->setIcon(0, icon);

	QTreeWidgetItem* u1EvaluationMatrix = new QTreeWidgetItem();
	u1EvaluationMatrix->setText(0, "U1权重向量A1与一致性Rc1计算");
	u1EvaluationMatrix->setData(0, Qt::UserRole, "U1EvaluationMatrix");
	u1EvaluationMatrix->setIcon(0, icon);

	QTreeWidgetItem* u2EvaluationMatrix = new QTreeWidgetItem();
	u2EvaluationMatrix->setText(0, "U2权重向量A2与一致性Rc2计算");
	u2EvaluationMatrix->setData(0, Qt::UserRole, "U2EvaluationMatrix");
	u2EvaluationMatrix->setIcon(0, icon);

	evaluationMatrixNode->addChild(u1EvaluationMatrix);
	evaluationMatrixNode->addChild(u2EvaluationMatrix);


	QTreeWidgetItem* matrixOperationNode = new QTreeWidgetItem(rootItem);
	matrixOperationNode->setText(0, "模糊矩阵运算");
	matrixOperationNode->setData(0, Qt::UserRole, "MatrixOperation");
	matrixOperationNode->setIcon(0, icon);

	QTreeWidgetItem* criterion = new QTreeWidgetItem();
	criterion->setText(0, "准则层隶属度");
	criterion->setData(0, Qt::UserRole, "Criterion");
	criterion->setIcon(0, icon);

	QTreeWidgetItem* target = new QTreeWidgetItem();
	target->setText(0, "目标层隶属度");
	target->setData(0, Qt::UserRole, "Target");
	target->setIcon(0, icon);

	matrixOperationNode->addChild(criterion);
	matrixOperationNode->addChild(target);

	QTreeWidgetItem* levelReportNode = new QTreeWidgetItem(rootItem);
	levelReportNode->setText(0, "安全等级判定与报告");
	levelReportNode->setData(0, Qt::UserRole, "LevelReport");
	levelReportNode->setIcon(0, icon);

	QTreeWidgetItem* level = new QTreeWidgetItem();
	level->setText(0, "等级判定");
	level->setData(0, Qt::UserRole, "Level");
	level->setIcon(0, icon);

	QTreeWidgetItem* score = new QTreeWidgetItem();
	score->setText(0, "分数评定");
	score->setData(0, Qt::UserRole, "Score");
	score->setIcon(0, icon);

	QTreeWidgetItem* report = new QTreeWidgetItem();
	report->setText(0, "安全性分析与评价报告");
	report->setData(0, Qt::UserRole, "Report");
	report->setIcon(0, icon);

	levelReportNode->addChild(level);
	levelReportNode->addChild(score);
	levelReportNode->addChild(report);



	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(treeWidget);
	layout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(layout);

	// 连接信号槽
	connect(treeWidget, &QTreeWidget::itemClicked, this, &ParamAnalyTreeWidget::onTreeItemClicked);
}

ParamAnalyTreeWidget::~ParamAnalyTreeWidget()
{
}

void ParamAnalyTreeWidget::onTreeItemClicked(QTreeWidgetItem* item, int column)
{
	QString itemData = item->data(0, Qt::UserRole).toString();
	emit itemClicked(itemData);
}



void ParamAnalyTreeWidget::contextMenuEvent(QContextMenuEvent* event)
{
	QTreeWidgetItem* item = treeWidget->itemAt(event->pos());
	if (!item) {
		return;
	}
	QString text = item->text(0);
	if (text == "发动机AHP安全性分析与评估")
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
			calculate();
		});
		contextMenu->addAction(calAction); // 将动作添加到菜单中
		contextMenu->exec(event->globalPos()); // 在鼠标位置显示菜单
	}
}

void ParamAnalyTreeWidget::calculate()
{
	QWidget* parent = parentWidget();
	while (parent)
	{
		ParamAnalyWidget* paParent = dynamic_cast<ParamAnalyWidget*>(parent);
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

					// U1权重向量A1与一致性Rc1计算
					auto u1WeightTtableWidget = paParent->getU1WeightTtableWidget();
					// table数据转Vector
					auto u1WeightVector = convertTableToVector(u1WeightTtableWidget);
					// 计算权重向量
					auto u1Weight = calculateWeights(u1WeightVector);
					// 一致性
					auto u1Consistency = consistencyCheck(u1WeightVector, u1Weight);

					auto u1CalculationTableWidget = paParent->getU1CalculationTableWidget();
					QTableWidgetItem* A1Item = new QTableWidgetItem("A1=" + vectorToString(u1Weight, 4));
					A1Item->setFlags(A1Item->flags() & ~Qt::ItemIsEditable); // 不可编辑
					u1CalculationTableWidget->setItem(0, 2, A1Item);

					QTableWidgetItem* u1ConsistencyItem = new QTableWidgetItem(u1Consistency <= 0.1 ? "通过，" + QString::number(u1Consistency) : "不通过，" + QString::number(u1Consistency));
					u1ConsistencyItem->setFlags(u1ConsistencyItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
					u1CalculationTableWidget->setItem(1, 2, u1ConsistencyItem);
					u1CalculationTableWidget->resizeColumnsToContents();


					// U2权重向量A2与一致性Rc2计算
					auto u2WeightTtableWidget = paParent->getU2WeightTtableWidget();
					// table数据转Vector
					auto u2WeightVector = convertTableToVector(u2WeightTtableWidget);
					// 计算权重向量
					auto u2Weight = calculateWeights(u2WeightVector);
					// 一致性
					auto u2Consistency = consistencyCheck(u2WeightVector, u2Weight);

					auto u2CalculationTableWidget = paParent->getU2CalculationTableWidget();
					QTableWidgetItem* A2Item = new QTableWidgetItem("A2=" + vectorToString(u2Weight, 4));
					A2Item->setFlags(A2Item->flags() & ~Qt::ItemIsEditable); // 不可编辑
					u2CalculationTableWidget->setItem(0, 2, A2Item);

					QTableWidgetItem* u2ConsistencyItem = new QTableWidgetItem(u2Consistency <= 0.1 ? "通过，" + QString::number(u2Consistency) : "不通过，" + QString::number(u2Consistency));
					u2ConsistencyItem->setFlags(u2ConsistencyItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
					u2CalculationTableWidget->setItem(1, 2, u2ConsistencyItem);
					u2CalculationTableWidget->resizeColumnsToContents();


					auto ins = ModelDataManager::GetInstance();
					// 跌落计算结果
					StressResult m_FallStressResult = ins->GetFallStressResult();
					StrainResult m_FallStrainResult = ins->GetFallStrainResult();
					TemperatureResult m_FallTemperatureResult = ins->GetFallTemperatureResult();
					OverpressureResult m_FallOverpressureResult = ins->GetFallOverpressureResult();
					// 快烤计算结果
					TemperatureResult m_FastCombustionTemperatureResult = ins->GetFastCombustionTemperatureResult();
					// 慢烤计算结果
					TemperatureResult m_SlowCombustionTemperatureResult = ins->GetSlowCombustionTemperatureResult();
					// 枪击计算结果
					StressResult m_ShootStressResult = ins->GetShootStressResult();
					StrainResult m_ShootStrainResult = ins->GetShootStrainResult();
					TemperatureResult m_ShootTemperatureResult = ins->GetShootTemperatureResult();
					OverpressureResult m_ShootOverpressureResult = ins->GetShootOverpressureResult();
					// 射流冲击计算结果
					StressResult m_JetImpactStressResult = ins->GetJetImpactStressResult();
					StrainResult m_JetImpactStrainResult = ins->GetJetImpactStrainResult();
					TemperatureResult m_JetImpactTemperatureResult = ins->GetJetImpactTemperatureResult();
					OverpressureResult m_JetImpactOverpressureResult = ins->GetJetImpactOverpressureResult();
					// 破片撞击计算结果
					StressResult m_FragmentationImpactStressResult = ins->GetFragmentationImpactStressResult();
					StrainResult m_FragmentationImpactStrainResult = ins->GetFragmentationImpactStrainResult();
					TemperatureResult m_FragmentationImpactTemperatureResult = ins->GetFragmentationImpactTemperatureResult();
					OverpressureResult m_FragmentationImpactOverpressureResult = ins->GetFragmentationImpactOverpressureResult();
					// 爆炸冲击波计算结果
					StressResult m_ExplosiveBlastStressResult = ins->GetExplosiveBlastStressResult();
					StrainResult m_ExplosiveBlastStrainResult = ins->GetExplosiveBlastStrainResult();
					TemperatureResult m_ExplosiveBlastTemperatureResult = ins->GetExplosiveBlastTemperatureResult();
					OverpressureResult m_ExplosiveBlastOverpressureResult = ins->GetExplosiveBlastOverpressureResult();
					// 殉爆计算结果
					StressResult m_SacrificeExplosionStressResult = ins->GetSacrificeExplosionStressResult();
					StrainResult m_SacrificeExplosionStrainResult = ins->GetSacrificeExplosionStrainResult();
					TemperatureResult m_SacrificeExplosionTemperatureResult = ins->GetSacrificeExplosionTemperatureResult();
					OverpressureResult m_SacrificeExplosionOverpressureResult = ins->GetSacrificeExplosionOverpressureResult();
					// U1权重向量A1与一致性Rc1计算
					auto u1EvaluationMatrixTableWidget = paParent->getU1EvaluationMatrixTableWidget();
					// 推进剂超压
					double one_verpressure = ins->GetPropellantPropertyInfo().fireOverpressure * 0.7;
					double two_verpressure = ins->GetPropellantPropertyInfo().fireOverpressure * 0.8;
					double three_verpressure = ins->GetPropellantPropertyInfo().fireOverpressure * 0.9;

					// 推进剂温度
					double one_ignitionTemperature = ins->GetPropellantPropertyInfo().ignitionTemperature - 50;
					double two_ignitionTemperature = ins->GetPropellantPropertyInfo().ignitionTemperature - 30;
					double three_ignitionTemperature = ins->GetPropellantPropertyInfo().ignitionTemperature - 10;

					u1EvaluationMatrixTableWidget->item(1, 1)->setText(QString::number(m_FallOverpressureResult.propellantsMaxOverpressure) + "MPa");
					vector<double> fallOverpressureMembership = getMembership(m_FallOverpressureResult.propellantsMaxOverpressure, one_verpressure, two_verpressure, three_verpressure);
					u1EvaluationMatrixTableWidget->item(1, 2)->setText(vectorToString(fallOverpressureMembership, 2));

					u1EvaluationMatrixTableWidget->item(2, 1)->setText(QString::number(m_FallTemperatureResult.propellantsMaxTemperature) + "℃");
					vector<double> fallTemperatureMembership = getMembership(m_FallTemperatureResult.propellantsMaxTemperature, one_ignitionTemperature, two_ignitionTemperature, three_ignitionTemperature);
					u1EvaluationMatrixTableWidget->item(2, 2)->setText(vectorToString(fallTemperatureMembership, 2));

					u1EvaluationMatrixTableWidget->item(3, 1)->setText(QString::number(m_FastCombustionTemperatureResult.propellantsMaxTemperature) + "℃");
					vector<double> fastCombustionTemperatureMembership = getMembership(m_FastCombustionTemperatureResult.propellantsMaxTemperature, one_ignitionTemperature, two_ignitionTemperature, three_ignitionTemperature);
					u1EvaluationMatrixTableWidget->item(3, 2)->setText(vectorToString(fastCombustionTemperatureMembership, 2));

					u1EvaluationMatrixTableWidget->item(4, 1)->setText(QString::number(m_SlowCombustionTemperatureResult.propellantsMaxTemperature) + "℃");
					vector<double> slowCombustionTemperatureMembership = getMembership(m_SlowCombustionTemperatureResult.propellantsMaxTemperature, one_ignitionTemperature, two_ignitionTemperature, three_ignitionTemperature);
					u1EvaluationMatrixTableWidget->item(4, 2)->setText(vectorToString(slowCombustionTemperatureMembership, 2));

					u1EvaluationMatrixTableWidget->item(5, 1)->setText(QString::number(m_ShootOverpressureResult.propellantsMaxOverpressure) + "MPa");
					vector<double> shootOverpressureMembership = getMembership(m_ShootOverpressureResult.propellantsMaxOverpressure, one_verpressure, two_verpressure, three_verpressure);
					u1EvaluationMatrixTableWidget->item(5, 2)->setText(vectorToString(shootOverpressureMembership, 2));

					u1EvaluationMatrixTableWidget->item(6, 1)->setText(QString::number(m_ShootTemperatureResult.propellantsMaxTemperature) + "℃");
					vector<double> shootTemperatureMembership = getMembership(m_ShootTemperatureResult.propellantsMaxTemperature, one_ignitionTemperature, two_ignitionTemperature, three_ignitionTemperature);
					u1EvaluationMatrixTableWidget->item(6, 2)->setText(vectorToString(shootTemperatureMembership, 2));

					u1EvaluationMatrixTableWidget->item(7, 1)->setText(QString::number(m_FragmentationImpactOverpressureResult.propellantsMaxOverpressure) + "MPa");
					vector<double> fragmentationImpactOverpressureMembership = getMembership(m_FragmentationImpactOverpressureResult.propellantsMaxOverpressure, one_verpressure, two_verpressure, three_verpressure);
					u1EvaluationMatrixTableWidget->item(7, 2)->setText(vectorToString(fragmentationImpactOverpressureMembership, 2));

					u1EvaluationMatrixTableWidget->item(8, 1)->setText(QString::number(m_FragmentationImpactTemperatureResult.propellantsMaxTemperature) + "℃");
					vector<double> fragmentationImpactTemperatureMembership = getMembership(m_FragmentationImpactTemperatureResult.propellantsMaxTemperature, one_ignitionTemperature, two_ignitionTemperature, three_ignitionTemperature);
					u1EvaluationMatrixTableWidget->item(8, 2)->setText(vectorToString(fragmentationImpactTemperatureMembership, 2));

					u1EvaluationMatrixTableWidget->item(9, 1)->setText(QString::number(m_SacrificeExplosionOverpressureResult.propellantsMaxOverpressure) + "MPa");
					vector<double> sacrificeExplosionOverpressureMembership = getMembership(m_SacrificeExplosionOverpressureResult.propellantsMaxOverpressure, one_verpressure, two_verpressure, three_verpressure);
					u1EvaluationMatrixTableWidget->item(9, 2)->setText(vectorToString(sacrificeExplosionOverpressureMembership, 2));

					u1EvaluationMatrixTableWidget->item(10, 1)->setText(QString::number(m_SacrificeExplosionTemperatureResult.propellantsMaxTemperature) + "℃");
					vector<double> sacrificeExplosionTemperatureMembership = getMembership(m_SacrificeExplosionTemperatureResult.propellantsMaxTemperature, one_ignitionTemperature, two_ignitionTemperature, three_ignitionTemperature);
					u1EvaluationMatrixTableWidget->item(10, 2)->setText(vectorToString(sacrificeExplosionTemperatureMembership, 2));

					u1EvaluationMatrixTableWidget->item(11, 1)->setText(QString::number(m_JetImpactOverpressureResult.propellantsMaxOverpressure) + "MPa");
					vector<double> jetImpactOverpressureMembership = getMembership(m_JetImpactOverpressureResult.propellantsMaxOverpressure, one_verpressure, two_verpressure, three_verpressure);
					u1EvaluationMatrixTableWidget->item(11, 2)->setText(vectorToString(jetImpactOverpressureMembership, 2));

					u1EvaluationMatrixTableWidget->item(12, 1)->setText(QString::number(m_JetImpactTemperatureResult.propellantsMaxTemperature) + "℃");
					vector<double> jetImpactTemperatureMembership = getMembership(m_JetImpactTemperatureResult.propellantsMaxTemperature, one_ignitionTemperature, two_ignitionTemperature, three_ignitionTemperature);
					u1EvaluationMatrixTableWidget->item(12, 2)->setText(vectorToString(jetImpactTemperatureMembership, 2));

					u1EvaluationMatrixTableWidget->item(13, 1)->setText(QString::number(m_ExplosiveBlastOverpressureResult.propellantsMaxOverpressure) + "MPa");
					vector<double> explosiveBlastOverpressureMembership = getMembership(m_ExplosiveBlastOverpressureResult.propellantsMaxOverpressure, one_verpressure, two_verpressure, three_verpressure);
					u1EvaluationMatrixTableWidget->item(13, 2)->setText(vectorToString(explosiveBlastOverpressureMembership, 2));

					u1EvaluationMatrixTableWidget->item(14, 1)->setText(QString::number(m_ExplosiveBlastTemperatureResult.propellantsMaxTemperature) + "℃");
					vector<double> explosiveBlastTemperatureMembership = getMembership(m_ExplosiveBlastTemperatureResult.propellantsMaxTemperature, one_ignitionTemperature, two_ignitionTemperature, three_ignitionTemperature);
					u1EvaluationMatrixTableWidget->item(14, 2)->setText(vectorToString(explosiveBlastTemperatureMembership, 2));

					u1EvaluationMatrixTableWidget->resizeColumnsToContents();
					// U2权重向量A1与一致性Rc1计算

					// 壳体应力
					double one_yieldStrengthe = ins->GetSteelPropertyInfo().yieldStrength * 0.7;
					double two_yieldStrength = ins->GetSteelPropertyInfo().yieldStrength * 0.8;
					double three_yieldStrength = ins->GetSteelPropertyInfo().yieldStrength * 0.9;
					auto u2EvaluationMatrixTableWidget = paParent->getU2EvaluationMatrixTableWidget();
					u2EvaluationMatrixTableWidget->item(1, 1)->setText(QString::number(m_FallStressResult.propellantsMaxStress) + "MPa");
					vector<double> fallStressMembership = getMembership(m_FallStressResult.propellantsMaxStress, one_yieldStrengthe, two_yieldStrength, three_yieldStrength);
					u2EvaluationMatrixTableWidget->item(1, 2)->setText(vectorToString(fallStressMembership, 2));

					u2EvaluationMatrixTableWidget->item(2, 1)->setText(QString::number(m_ShootStressResult.propellantsMaxStress) + "MPa");
					vector<double> shootStressMembership = getMembership(m_ShootStressResult.propellantsMaxStress, one_yieldStrengthe, two_yieldStrength, three_yieldStrength);
					u2EvaluationMatrixTableWidget->item(2, 2)->setText(vectorToString(shootStressMembership, 2));

					u2EvaluationMatrixTableWidget->item(3, 1)->setText(QString::number(m_FragmentationImpactStressResult.propellantsMaxStress) + "MPa");
					vector<double> fragmentationImpactStressMembership = getMembership(m_FragmentationImpactStressResult.propellantsMaxStress, one_yieldStrengthe, two_yieldStrength, three_yieldStrength);
					u2EvaluationMatrixTableWidget->item(3, 2)->setText(vectorToString(fragmentationImpactStressMembership, 2));

					u2EvaluationMatrixTableWidget->item(4, 1)->setText(QString::number(m_SacrificeExplosionStressResult.propellantsMaxStress) + "MPa");
					vector<double> sacrificeExplosionStressMembership = getMembership(m_SacrificeExplosionStressResult.propellantsMaxStress, one_yieldStrengthe, two_yieldStrength, three_yieldStrength);
					u2EvaluationMatrixTableWidget->item(4, 2)->setText(vectorToString(sacrificeExplosionStressMembership, 2));

					u2EvaluationMatrixTableWidget->item(5, 1)->setText(QString::number(m_JetImpactStressResult.propellantsMaxStress) + "MPa");
					vector<double> jetImpactStressMembership = getMembership(m_JetImpactStressResult.propellantsMaxStress, one_yieldStrengthe, two_yieldStrength, three_yieldStrength);
					u2EvaluationMatrixTableWidget->item(5, 2)->setText(vectorToString(jetImpactStressMembership, 2));

					u2EvaluationMatrixTableWidget->item(6, 1)->setText(QString::number(m_ExplosiveBlastStressResult.propellantsMaxStress) + "MPa");
					vector<double> explosiveBlastStressMembership = getMembership(m_ExplosiveBlastStressResult.propellantsMaxStress, one_yieldStrengthe, two_yieldStrength, three_yieldStrength);
					u2EvaluationMatrixTableWidget->item(6, 2)->setText(vectorToString(explosiveBlastStressMembership, 2));

					u2EvaluationMatrixTableWidget->resizeColumnsToContents();



					double yieldStrength = ins->GetSteelPropertyInfo().yieldStrength; // 壳体应力
					double ignitionTemperature = ins->GetPropellantPropertyInfo().ignitionTemperature; //推进剂温度
					double fireOverpressure = ins->GetPropellantPropertyInfo().fireOverpressure; //推进剂超压

					// U1等级定量标准
					auto u1GradeDefinitionTableWidget = paParent->getU1GradeDefinitionTableWidget();
					u1GradeDefinitionTableWidget->item(0, 1)->setText("跌落场景推进剂超压（P₁=" + QString::number(fireOverpressure) + "MPa）");
					u1GradeDefinitionTableWidget->item(0, 2)->setText("跌落场景推进剂温度（T₁=" + QString::number(ignitionTemperature) + "℃）");
					u1GradeDefinitionTableWidget->item(0, 3)->setText("快速烤燃场景推进剂温度（T₂=" + QString::number(ignitionTemperature) + "℃）");
					u1GradeDefinitionTableWidget->item(0, 4)->setText("慢速烤燃场景推进剂温度（T₃=" + QString::number(ignitionTemperature) + "℃）");
					u1GradeDefinitionTableWidget->item(0, 5)->setText("子弹撞击场景推进剂超压（P₄=" + QString::number(fireOverpressure) + "MPa）");
					u1GradeDefinitionTableWidget->item(0, 6)->setText("子弹撞击场景推进剂温度（T₄=" + QString::number(ignitionTemperature) + "℃）");
					u1GradeDefinitionTableWidget->item(0, 7)->setText("破片撞击场景推进剂超压（P₅=" + QString::number(fireOverpressure) + "MPa）");
					u1GradeDefinitionTableWidget->item(0, 8)->setText("破片撞击场景推进剂温度（T₅=" + QString::number(ignitionTemperature) + "℃）");
					u1GradeDefinitionTableWidget->item(0, 9)->setText("殉爆场景推进剂超压（P₆=" + QString::number(fireOverpressure) + "MPa）");
					u1GradeDefinitionTableWidget->item(0, 10)->setText("殉爆场景推进剂温度（T₆=" + QString::number(ignitionTemperature) + "℃）");
					u1GradeDefinitionTableWidget->item(0, 11)->setText("射流场景推进剂超压（P₇=" + QString::number(fireOverpressure) + "MPa）");
					u1GradeDefinitionTableWidget->item(0, 12)->setText("射流场景推进剂温度（T₇=" + QString::number(ignitionTemperature) + "℃）");
					u1GradeDefinitionTableWidget->item(0, 13)->setText("爆炸冲击场景推进剂超压（P₈=" + QString::number(fireOverpressure) + "MPa）");
					u1GradeDefinitionTableWidget->item(0, 14)->setText("爆炸冲击场景推进剂温度（T₈=" + QString::number(ignitionTemperature) + "℃）");

					u1GradeDefinitionTableWidget->item(1, 1)->setText("≤ " + QString::number(fireOverpressure * 0.7) + "MPa（0.7P₁）");
					u1GradeDefinitionTableWidget->item(1, 2)->setText("≤ " + QString::number(ignitionTemperature - 50) + "℃（≤T₁-50）");
					u1GradeDefinitionTableWidget->item(1, 3)->setText("≤ " + QString::number(ignitionTemperature - 50) + "℃（≤T₂-50）");
					u1GradeDefinitionTableWidget->item(1, 4)->setText("≤ " + QString::number(ignitionTemperature - 50) + "℃（≤T₃-50）");
					u1GradeDefinitionTableWidget->item(1, 5)->setText("≤ " + QString::number(fireOverpressure * 0.7) + "MPa（0.7P₂）");
					u1GradeDefinitionTableWidget->item(1, 6)->setText("≤ " + QString::number(ignitionTemperature - 50) + "℃（≤T₄-50）");
					u1GradeDefinitionTableWidget->item(1, 7)->setText("≤ " + QString::number(fireOverpressure * 0.7) + "MPa（0.7P₃）");
					u1GradeDefinitionTableWidget->item(1, 8)->setText("≤ " + QString::number(ignitionTemperature - 50) + "℃（≤T₅-50）");
					u1GradeDefinitionTableWidget->item(1, 9)->setText("≤ " + QString::number(fireOverpressure * 0.7) + "MPa（0.7P₄）");
					u1GradeDefinitionTableWidget->item(1, 10)->setText("≤ " + QString::number(ignitionTemperature - 50) + "℃（≤T₆-50）");
					u1GradeDefinitionTableWidget->item(1, 11)->setText("≤ " + QString::number(fireOverpressure * 0.7) + "MPa（0.7P₅）");
					u1GradeDefinitionTableWidget->item(1, 12)->setText("≤ " + QString::number(ignitionTemperature - 50) + "℃（≤T₇-50）");
					u1GradeDefinitionTableWidget->item(1, 13)->setText("≤ " + QString::number(fireOverpressure * 0.7) + "MPa（0.7P₆）");
					u1GradeDefinitionTableWidget->item(1, 14)->setText("≤ " + QString::number(ignitionTemperature - 50) + "℃（≤T₈-50）");

					u1GradeDefinitionTableWidget->item(2, 1)->setText(QString::number(fireOverpressure * 0.7) + "~" + QString::number(fireOverpressure * 0.8) + "MPa（0.7~0.8P₁）");
					u1GradeDefinitionTableWidget->item(2, 2)->setText(QString::number(ignitionTemperature - 50) + "~" + QString::number(ignitionTemperature - 30) + "℃（≤T₁-50~30）");
					u1GradeDefinitionTableWidget->item(2, 3)->setText(QString::number(ignitionTemperature - 50) + "~" + QString::number(ignitionTemperature - 30) + "℃（≤T₂-50~30）");
					u1GradeDefinitionTableWidget->item(2, 4)->setText(QString::number(ignitionTemperature - 50) + "~" + QString::number(ignitionTemperature - 30) + "℃（≤T₃-50~30）");
					u1GradeDefinitionTableWidget->item(2, 5)->setText(QString::number(fireOverpressure * 0.7) + "~" + QString::number(fireOverpressure * 0.8) + "MPa（0.7~0.8P₂）");
					u1GradeDefinitionTableWidget->item(2, 6)->setText(QString::number(ignitionTemperature - 50) + "~" + QString::number(ignitionTemperature - 30) + "℃（≤T₄-50~30）");
					u1GradeDefinitionTableWidget->item(2, 7)->setText(QString::number(fireOverpressure * 0.7) + "~" + QString::number(fireOverpressure * 0.8) + "MPa（0.7~0.8P₃）");
					u1GradeDefinitionTableWidget->item(2, 8)->setText(QString::number(ignitionTemperature - 50) + "~" + QString::number(ignitionTemperature - 30) + "℃（≤T₅-50~30）");
					u1GradeDefinitionTableWidget->item(2, 9)->setText(QString::number(fireOverpressure * 0.7) + "~" + QString::number(fireOverpressure * 0.8) + "MPa（0.7~0.8P₄）");
					u1GradeDefinitionTableWidget->item(2, 10)->setText(QString::number(ignitionTemperature - 50) + "~" + QString::number(ignitionTemperature - 30) + "℃（≤T₆-50~30）");
					u1GradeDefinitionTableWidget->item(2, 11)->setText(QString::number(fireOverpressure * 0.7) + "~" + QString::number(fireOverpressure * 0.8) + "MPa（0.7~0.8P₅）");
					u1GradeDefinitionTableWidget->item(2, 12)->setText(QString::number(ignitionTemperature - 50) + "~" + QString::number(ignitionTemperature - 30) + "℃（≤T₇-50~30）");
					u1GradeDefinitionTableWidget->item(2, 13)->setText(QString::number(fireOverpressure * 0.7) + "~" + QString::number(fireOverpressure * 0.8) + "MPa（0.7~0.8P₆）");
					u1GradeDefinitionTableWidget->item(2, 14)->setText(QString::number(ignitionTemperature - 50) + "~" + QString::number(ignitionTemperature - 30) + "℃（≤T₈-50~30）");

					u1GradeDefinitionTableWidget->item(3, 1)->setText(QString::number(fireOverpressure * 0.8) + "~" + QString::number(fireOverpressure * 0.9) + "MPa（0.8~0.9P₁）");
					u1GradeDefinitionTableWidget->item(3, 2)->setText(QString::number(ignitionTemperature - 30) + "~" + QString::number(ignitionTemperature - 10) + "℃（≤T₁-30~10）");
					u1GradeDefinitionTableWidget->item(3, 3)->setText(QString::number(ignitionTemperature - 30) + "~" + QString::number(ignitionTemperature - 10) + "℃（≤T₂-30~10）");
					u1GradeDefinitionTableWidget->item(3, 4)->setText(QString::number(ignitionTemperature - 30) + "~" + QString::number(ignitionTemperature - 10) + "℃（≤T₃-30~10）");
					u1GradeDefinitionTableWidget->item(3, 5)->setText(QString::number(fireOverpressure * 0.8) + "~" + QString::number(fireOverpressure * 0.9) + "MPa（0.8~0.9P₂）");
					u1GradeDefinitionTableWidget->item(3, 6)->setText(QString::number(ignitionTemperature - 30) + "~" + QString::number(ignitionTemperature - 10) + "℃（≤T₄-30~10）");
					u1GradeDefinitionTableWidget->item(3, 7)->setText(QString::number(fireOverpressure * 0.8) + "~" + QString::number(fireOverpressure * 0.9) + "MPa（0.8~0.9P₃）");
					u1GradeDefinitionTableWidget->item(3, 8)->setText(QString::number(ignitionTemperature - 30) + "~" + QString::number(ignitionTemperature - 10) + "℃（≤T₅-30~10）");
					u1GradeDefinitionTableWidget->item(3, 9)->setText(QString::number(fireOverpressure * 0.8) + "~" + QString::number(fireOverpressure * 0.9) + "MPa（0.8~0.9P₄）");
					u1GradeDefinitionTableWidget->item(3, 10)->setText(QString::number(ignitionTemperature - 30) + "~" + QString::number(ignitionTemperature - 10) + "℃（≤T₆-30~10）");
					u1GradeDefinitionTableWidget->item(3, 11)->setText(QString::number(fireOverpressure * 0.8) + "~" + QString::number(fireOverpressure * 0.9) + "MPa（0.8~0.9P₅）");
					u1GradeDefinitionTableWidget->item(3, 12)->setText(QString::number(ignitionTemperature - 30) + "~" + QString::number(ignitionTemperature - 10) + "℃（≤T₇-30~10）");
					u1GradeDefinitionTableWidget->item(3, 13)->setText(QString::number(fireOverpressure * 0.8) + "~" + QString::number(fireOverpressure * 0.9) + "MPa（0.8~0.9P₆）");
					u1GradeDefinitionTableWidget->item(3, 14)->setText(QString::number(ignitionTemperature - 30) + "~" + QString::number(ignitionTemperature - 10) + "℃（≤T₈-30~10）");

					u1GradeDefinitionTableWidget->item(4, 1)->setText("> " + QString::number(fireOverpressure * 0.9) + "MPa（0.9P₁）");
					u1GradeDefinitionTableWidget->item(4, 2)->setText("> " + QString::number(ignitionTemperature - 10) + "℃（≤T₁-10）");
					u1GradeDefinitionTableWidget->item(4, 3)->setText("> " + QString::number(ignitionTemperature - 10) + "℃（≤T₂-10）");
					u1GradeDefinitionTableWidget->item(4, 4)->setText("> " + QString::number(ignitionTemperature - 10) + "℃（≤T₃-10）");
					u1GradeDefinitionTableWidget->item(4, 5)->setText("> " + QString::number(fireOverpressure * 0.9) + "MPa（0.9P₂）");
					u1GradeDefinitionTableWidget->item(4, 6)->setText("> " + QString::number(ignitionTemperature - 10) + "℃（≤T₄-10）");
					u1GradeDefinitionTableWidget->item(4, 7)->setText("> " + QString::number(fireOverpressure * 0.9) + "MPa（0.9P₃）");
					u1GradeDefinitionTableWidget->item(4, 8)->setText("> " + QString::number(ignitionTemperature - 10) + "℃（≤T₅-10）");
					u1GradeDefinitionTableWidget->item(4, 9)->setText("> " + QString::number(fireOverpressure * 0.9) + "MPa（0.9P₄）");
					u1GradeDefinitionTableWidget->item(4, 10)->setText("> " + QString::number(ignitionTemperature - 10) + "℃（≤T₆-10）");
					u1GradeDefinitionTableWidget->item(4, 11)->setText("> " + QString::number(fireOverpressure * 0.9) + "MPa（0.9P₅）");
					u1GradeDefinitionTableWidget->item(4, 12)->setText("> " + QString::number(ignitionTemperature - 10) + "℃（≤T₇-10）");
					u1GradeDefinitionTableWidget->item(4, 13)->setText("> " + QString::number(fireOverpressure * 0.9) + "MPa（0.9P₆）");
					u1GradeDefinitionTableWidget->item(4, 14)->setText("> " + QString::number(ignitionTemperature - 10) + "℃（≤T₈-10）");

					u1GradeDefinitionTableWidget->resizeColumnsToContents();
					// U2等级定量标准
					auto u2GradeDefinitionTableWidget = paParent->getU2GradeDefinitionTableWidget();
					u2GradeDefinitionTableWidget->item(0, 1)->setText("跌落场景壳体应力（σ₁=" + QString::number(yieldStrength) + "MPa）");
					u2GradeDefinitionTableWidget->item(0, 2)->setText("子弹撞击场景壳体应力（σ₂=" + QString::number(yieldStrength) + "MPa）");
					u2GradeDefinitionTableWidget->item(0, 3)->setText("破片撞击场景壳体应力（σ₃=" + QString::number(yieldStrength) + "MPa）");
					u2GradeDefinitionTableWidget->item(0, 4)->setText("殉爆场景壳体应力（σ₄=" + QString::number(yieldStrength) + "MPa）");
					u2GradeDefinitionTableWidget->item(0, 5)->setText("射流场景壳体应力（σ₅=" + QString::number(yieldStrength) + "MPa）");
					u2GradeDefinitionTableWidget->item(0, 6)->setText("爆炸冲击场景壳体应力（σ₆=" + QString::number(yieldStrength) + "MPa）");

					u2GradeDefinitionTableWidget->item(1, 1)->setText("≤ " + QString::number(yieldStrength * 0.7) + "MPa（0.7σ₁）");
					u2GradeDefinitionTableWidget->item(1, 2)->setText("≤ " + QString::number(yieldStrength * 0.7) + "MPa（0.7σ₂）");
					u2GradeDefinitionTableWidget->item(1, 3)->setText("≤ " + QString::number(yieldStrength * 0.7) + "MPa（0.7σ₃）");
					u2GradeDefinitionTableWidget->item(1, 4)->setText("≤ " + QString::number(yieldStrength * 0.7) + "MPa（0.7σ₄）");
					u2GradeDefinitionTableWidget->item(1, 5)->setText("≤ " + QString::number(yieldStrength * 0.7) + "MPa（0.7σ₅）");
					u2GradeDefinitionTableWidget->item(1, 6)->setText("≤ " + QString::number(yieldStrength * 0.7) + "MPa（0.7σ₆）");

					u2GradeDefinitionTableWidget->item(2, 1)->setText(QString::number(yieldStrength * 0.7) + "~" + QString::number(yieldStrength * 0.8) + "MPa（0.7~0.8σ₁）");
					u2GradeDefinitionTableWidget->item(2, 2)->setText(QString::number(yieldStrength * 0.7) + "~" + QString::number(yieldStrength * 0.8) + "MPa（0.7~0.8σ₂）");
					u2GradeDefinitionTableWidget->item(2, 3)->setText(QString::number(yieldStrength * 0.7) + "~" + QString::number(yieldStrength * 0.8) + "MPa（0.7~0.8σ₃）");
					u2GradeDefinitionTableWidget->item(2, 4)->setText(QString::number(yieldStrength * 0.7) + "~" + QString::number(yieldStrength * 0.8) + "MPa（0.7~0.8σ₄）");
					u2GradeDefinitionTableWidget->item(2, 5)->setText(QString::number(yieldStrength * 0.7) + "~" + QString::number(yieldStrength * 0.8) + "MPa（0.7~0.8σ₅）");
					u2GradeDefinitionTableWidget->item(2, 6)->setText(QString::number(yieldStrength * 0.7) + "~" + QString::number(yieldStrength * 0.8) + "MPa（0.7~0.8σ₆）");

					u2GradeDefinitionTableWidget->item(3, 1)->setText(QString::number(yieldStrength * 0.8) + "~" + QString::number(yieldStrength * 0.9) + "MPa（0.8~0.9σ₁）");
					u2GradeDefinitionTableWidget->item(3, 2)->setText(QString::number(yieldStrength * 0.8) + "~" + QString::number(yieldStrength * 0.9) + "MPa（0.8~0.9σ₂）");
					u2GradeDefinitionTableWidget->item(3, 3)->setText(QString::number(yieldStrength * 0.8) + "~" + QString::number(yieldStrength * 0.9) + "MPa（0.8~0.9σ₃）");
					u2GradeDefinitionTableWidget->item(3, 4)->setText(QString::number(yieldStrength * 0.8) + "~" + QString::number(yieldStrength * 0.9) + "MPa（0.8~0.9σ₄）");
					u2GradeDefinitionTableWidget->item(3, 5)->setText(QString::number(yieldStrength * 0.8) + "~" + QString::number(yieldStrength * 0.9) + "MPa（0.8~0.9σ₅）");
					u2GradeDefinitionTableWidget->item(3, 6)->setText(QString::number(yieldStrength * 0.8) + "~" + QString::number(yieldStrength * 0.9) + "MPa（0.8~0.9σ₆）");

					u2GradeDefinitionTableWidget->item(4, 1)->setText("> " + QString::number(yieldStrength * 0.9) + "MPa（0.9σ₁）");
					u2GradeDefinitionTableWidget->item(4, 2)->setText("> " + QString::number(yieldStrength * 0.9) + "MPa（0.9σ₂）");
					u2GradeDefinitionTableWidget->item(4, 3)->setText("> " + QString::number(yieldStrength * 0.9) + "MPa（0.9σ₃）");
					u2GradeDefinitionTableWidget->item(4, 4)->setText("> " + QString::number(yieldStrength * 0.9) + "MPa（0.9σ₄）");
					u2GradeDefinitionTableWidget->item(4, 5)->setText("> " + QString::number(yieldStrength * 0.9) + "MPa（0.9σ₅）");
					u2GradeDefinitionTableWidget->item(4, 6)->setText("> " + QString::number(yieldStrength * 0.9) + "MPa（0.9σ₆）");

					u2GradeDefinitionTableWidget->resizeColumnsToContents();
					// 准则层隶属度
					vector<vector<double>> A1;
					A1.push_back(u1Weight);
					vector<vector<double>> A2;
					A2.push_back(u2Weight);
					vector<vector<double>> R1;
					R1.push_back(fallOverpressureMembership);
					R1.push_back(fallTemperatureMembership);
					R1.push_back(fastCombustionTemperatureMembership);
					R1.push_back(slowCombustionTemperatureMembership);
					R1.push_back(shootOverpressureMembership);
					R1.push_back(shootTemperatureMembership);
					R1.push_back(fragmentationImpactOverpressureMembership);
					R1.push_back(fragmentationImpactTemperatureMembership);
					R1.push_back(sacrificeExplosionOverpressureMembership);
					R1.push_back(sacrificeExplosionTemperatureMembership);
					R1.push_back(jetImpactOverpressureMembership);
					R1.push_back(jetImpactTemperatureMembership);
					R1.push_back(explosiveBlastOverpressureMembership);
					R1.push_back(explosiveBlastTemperatureMembership);


					vector<vector<double>> R2;
					R2.push_back(fallStressMembership);
					R2.push_back(shootStressMembership);
					R2.push_back(fragmentationImpactStressMembership);
					R2.push_back(sacrificeExplosionStressMembership);
					R2.push_back(jetImpactStressMembership);
					R2.push_back(explosiveBlastStressMembership);

					auto B1 = matrixMultiply(A1, R1);
					auto B2 = matrixMultiply(A2, R2);

					auto criterionTableWidget = paParent->getCriterionTableWidget();
					criterionTableWidget->item(1, 2)->setText(vectorToString(B1[0], 2));
					criterionTableWidget->item(2, 2)->setText(vectorToString(B2[0], 2));
					criterionTableWidget->resizeColumnsToContents();

					auto weightTtableWidget = paParent->getWeightTtableWidget();
					// table数据转Vector
					auto weightVector = convertTableToVector(weightTtableWidget);
					// 计算权重向量
					auto weight = calculateWeights(weightVector);

					vector<vector<double>> A;
					A.push_back(weight);
					vector<vector<double>> B;
					B.push_back(B1[0]);
					B.push_back(B2[0]);

					// 目标层隶属度
					auto targetTableWidget = paParent->getTargetTableWidget();

					auto result = matrixMultiply(A, B)[0];

					targetTableWidget->item(1, 2)->setText(vectorToString(result, 2));
					targetTableWidget->resizeColumnsToContents();

					// 等级判定
					auto levelTableWidget = paParent->getLevelTableWidget();
					auto max_it = std::max_element(result.begin(), result.end());
					size_t max_index = std::distance(result.begin(), max_it);
					double max_value = *max_it;

					vector<QString> level = { "V1（优，安全）","V2（良，基本安全）" ,"V3（中，需管控）" ,"V4（差，不安全）" };

					levelTableWidget->item(1, 1)->setText(QString::number(max_value, 'f', 2));
					levelTableWidget->item(1, 2)->setText(level[max_index]);
					levelTableWidget->resizeColumnsToContents();

					// 分数评定
					auto sourceTableWidget = paParent->getScoreTableWidget();
					vector<double> scoreVector = { 95,80,65,30 };
					double score = 0;
					for (size_t i = 0; i < scoreVector.size(); ++i) {
						score = score + scoreVector[i] * result[i];
					}
					QString scoreText = "";
					if (score >= 85)
					{
						scoreText = "V₁（优）";
					}
					else if (score >= 70 && score < 85)
					{
						scoreText = "V₂（良）";
					}
					else if (score >= 50 && score < 70)
					{
						scoreText = "V₃（中）";
					}
					else
					{
						scoreText = "V₄（差）";
					}
					sourceTableWidget->item(1, 1)->setText(QString::number(score, 'f', 2));
					sourceTableWidget->item(1, 2)->setText(scoreText);
					sourceTableWidget->resizeColumnsToContents();



					// 计算安全性指标评分结果数据
					double fallOverpressurePoint = getPoint(fallOverpressureMembership);
					double fallTemperaturePoint = getPoint(fallTemperatureMembership);
					double fallStressPoint = getPoint(fallStressMembership);
					double fallPoint = (fallOverpressurePoint + fallTemperaturePoint + fallStressPoint) / 3.0;

					double fastCombustionPoint = getPoint(fastCombustionTemperatureMembership);

					double slowCombustionPoint = getPoint(slowCombustionTemperatureMembership);

					double shootOverpressurePoint = getPoint(shootOverpressureMembership);
					double shootTemperaturePoint = getPoint(shootTemperatureMembership);
					double shootStressPoint = getPoint(shootStressMembership);
					double shootPoint = (shootOverpressurePoint + shootTemperaturePoint + shootStressPoint) / 3.0;

					double fragmentationImpactOverpressurePoint = getPoint(fragmentationImpactOverpressureMembership);
					double fragmentationImpactTemperaturePoint = getPoint(fragmentationImpactTemperatureMembership);
					double fragmentationImpactStressPoint = getPoint(fragmentationImpactStressMembership);
					double fragmentationImpactPoint = (fragmentationImpactOverpressurePoint + fragmentationImpactTemperaturePoint + fragmentationImpactStressPoint) / 3.0;

					double sacrificeExplosionOverpressurePoint = getPoint(sacrificeExplosionOverpressureMembership);
					double sacrificeExplosionTemperaturePoint = getPoint(sacrificeExplosionTemperatureMembership);
					double sacrificeExplosionStressPoint = getPoint(sacrificeExplosionStressMembership);
					double sacrificeExplosionPoint = (sacrificeExplosionOverpressurePoint + sacrificeExplosionTemperaturePoint + sacrificeExplosionStressPoint) / 3.0;

					double jetImpactOverpressurePoint = getPoint(jetImpactOverpressureMembership);
					double jetImpactTemperaturePoint = getPoint(jetImpactTemperatureMembership);
					double jetImpactStressPoint = getPoint(jetImpactStressMembership);
					double jetImpactPoint = (jetImpactOverpressurePoint + jetImpactTemperaturePoint + jetImpactStressPoint) / 3.0;

					double explosiveBlastOverpressurePoint = getPoint(explosiveBlastOverpressureMembership);
					double explosiveBlastTemperaturePoint = getPoint(explosiveBlastTemperatureMembership);
					double explosiveBlastStressPoint = getPoint(explosiveBlastStressMembership);
					double explosiveBlastPoint = (explosiveBlastOverpressurePoint + explosiveBlastTemperaturePoint + explosiveBlastStressPoint) / 3.0;

					auto pointResult = ins->GetPointResult();
					pointResult.fallPoint = fallPoint;
					pointResult.fastCombustionPoint = fastCombustionPoint;
					pointResult.slowCombustionPoint = slowCombustionPoint;
					pointResult.shootPoint = shootPoint;
					pointResult.jetImpactPoint = jetImpactPoint;
					pointResult.fragmentationImpactPoint = fragmentationImpactPoint;
					pointResult.explosiveBlastPoint = explosiveBlastPoint;
					pointResult.sacrificeExplosionPoint = sacrificeExplosionPoint;
					ins->SetPointResult(pointResult);

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

}

// 解析字符串为数字（支持分数和小数）
double ParamAnalyTreeWidget::parseNumber(const QString& fractionStr)
{
	QString str = fractionStr.trimmed();

	// 如果是普通数字，直接转换
	bool ok;
	double value = str.toDouble(&ok);
	if (ok) {
		return value;
	}
	// 处理分数格式：a/b
	QRegularExpression fractionRegex(R"(^\s*(\d+)\s*/\s*(\d+)\s*$)");
	QRegularExpressionMatch match = fractionRegex.match(str);

	if (match.hasMatch()) {
		double numerator = match.captured(1).toDouble();
		double denominator = match.captured(2).toDouble();

		if (fabs(denominator) < 1e-10) {
			throw runtime_error("分母不能为0");
		}

		return numerator / denominator;
	}

	// 处理小数形式
	QRegularExpression decimalRegex(R"(^\s*[-+]?\d*\.?\d+([eE][-+]?\d+)?\s*$)");
	if (decimalRegex.match(str).hasMatch()) {
		value = str.toDouble(&ok);
		if (ok) {
			return value;
		}
	}

	throw runtime_error("数据异常: " + str.toStdString());
}
// 表格转二维数组
vector<vector<double>> ParamAnalyTreeWidget::convertTableToVector(QTableWidget* tableWidget)
{
	int startRow = 1;
	int startCol = 1;
	int rowCount = tableWidget->rowCount();
	int colCount = tableWidget->columnCount();
	// 检查是否有有效数据
	if (rowCount <= startRow || colCount <= startCol) {
		throw runtime_error("缺少数据，无法计算");
	}

	vector<vector<double>> result;
	result.reserve(rowCount - startRow);
	// 遍历行（跳过第一行）
	for (int row = startRow; row < rowCount; ++row) {
		vector<double> rowData;
		rowData.reserve(colCount - startCol);

		// 遍历列（跳过第一列）
		for (int col = startCol; col < colCount; ++col) {
			QTableWidgetItem* item = tableWidget->item(row, col);
			double value = 0.0;

			if (item && !item->text().isEmpty()) {
				QString text = item->text().trimmed();
				try {
					value = parseNumber(text);
				}
				catch (const exception& e) {
					throw runtime_error(
						QString("数据异常， (%1,%2): '%3' - %4").arg(row).arg(col).arg(text).arg(e.what()).toStdString()
					);
				}
			}
			else {
				// 空单元格默认为0
				value = 0.0;
			}
			rowData.push_back(value);
		}
		result.push_back(rowData);
	}
	return result;
}


// 矩阵乘法
vector<vector<double>> ParamAnalyTreeWidget::matrixMultiply(const vector<vector<double>>& a, const vector<vector<double>>& b)
{
	int rowsA = a.size();
	int colsA = a[0].size();
	int colsB = b[0].size();

	vector<vector<double>> result(rowsA, vector<double>(colsB, 0));

	for (int i = 0; i < rowsA; ++i) {
		for (int j = 0; j < colsB; ++j) {
			for (int k = 0; k < colsA; ++k) {
				result[i][j] += a[i][k] * b[k][j];
			}
		}
	}

	return result;
}


// 列归一化计算
vector<vector<double>> ParamAnalyTreeWidget::columnNormalize(const vector<vector<double>>& matrix)
{
	int n = matrix.size();
	vector<vector<double>> normalized(n, vector<double>(n, 0.0));

	// 计算每列的和
	vector<double> colSums(n, 0.0);
	for (int j = 0; j < n; ++j) {
		for (int i = 0; i < n; ++i) {
			colSums[j] += matrix[i][j];
		}
	}

	// 列归一化：每个元素除以对应列的和
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			normalized[i][j] = matrix[i][j] / colSums[j];
		}
	}

	return normalized;
}

// 计算权重向量
vector<double> ParamAnalyTreeWidget::calculateWeights(const vector<vector<double>>& matrix)
{
	vector<double> weights;
	int n = matrix.size();

	// 列归一化
	vector<vector<double>> normalized = columnNormalize(matrix);
	// 行求和
	vector<double> rowSums(n, 0.0);
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			rowSums[i] += normalized[i][j];
		}
	}
	double totalRowSum = 0.0;
	for (int i = 0; i < n; ++i) {
		totalRowSum += rowSums[i];
	}
	// 归一化得到权重向量
	weights.resize(n);
	for (int i = 0; i < n; ++i) {
		weights[i] = rowSums[i] / totalRowSum;
	}
	return weights;
}

// 一致性校验
double ParamAnalyTreeWidget::consistencyCheck(const vector<vector<double>>& matrix, vector<double> weights)
{
	int n = matrix.size();

	// 计算最大特征值
	double lambdaMax = 0.0;
	for (int i = 0; i < n; ++i) {
		double rowSum = 0.0;
		for (int j = 0; j < n; ++j) {
			rowSum += matrix[i][j] * weights[j];
		}
		lambdaMax += rowSum / weights[i];
	}
	lambdaMax /= n;

	// 计算一致性指标
	double CI = (lambdaMax - n) / (n - 1);

	// 随机一致性指标（RI值）
	vector<double> RI = { 0, 0, 0, 0.58, 0.90, 1.12, 1.24, 1.32, 1.41, 1.45, 1.49, 1.51, 1.48, 1.56, 1.57, 1.59, 1.60, 1.61, 1.62, 1.63, 1.64 };
	/*double CR = (n < RI.size()) ? CI / RI[n] : CI;*/
	double CR = 0.0;
	if (RI[n] == 0)
	{
		CR = 0;

	}
	else
	{
		CR = (n < RI.size()) ? CI / RI[n] : CI;

	}
	return CR;
}