#pragma execution_character_set("utf-8")
#include "ParamAnalyTreeWidget.h"
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

	QTreeWidgetItem* report = new QTreeWidgetItem();
	report->setText(0, "安全性分析与评价报告");
	report->setData(0, Qt::UserRole, "Report");
	report->setIcon(0, icon);

	levelReportNode->addChild(level);
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

}

