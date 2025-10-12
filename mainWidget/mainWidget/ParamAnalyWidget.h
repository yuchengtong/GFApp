#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QStackedWidget>

#include "ParamAnalyTreeWidget.h"


class ParamAnalyWidget : public QWidget
{
	Q_OBJECT

public:
	ParamAnalyWidget(QWidget* parent = nullptr);
	~ParamAnalyWidget();

private slots:
	void onTreeItemClicked(const QString& itemData);

private:

	ParamAnalyTreeWidget* m_treeModelWidget = nullptr;
	QTableWidget* m_tableWidget = nullptr;
	QStackedWidget* m_tableStackWidget = nullptr;

	// 三级评价体系设计
	QTableWidget* evaluationSystemTableWidget = nullptr;

	// U1指标权重设计
	QTableWidget* u1WeightTtableWidget = nullptr;
	// U2指标权重设计
	QTableWidget* u2WeightTableWidget = nullptr;

	
	// U1权重向量A1与一致性Rc1计算
	QTableWidget* u1CalculationTableWidget = nullptr;
	// U2权重向量A2与一致性Rc2计算
	QTableWidget* u2CalculationTableWidget = nullptr;

	// 评语集与等级定义
	QTableWidget* gradeDefinitionTableWidget = nullptr;

	// U1权重向量A1与一致性Rc1计算
	QTableWidget* u1EvaluationMatrixTableWidget = nullptr;
	// U2权重向量A1与一致性Rc1计算
	QTableWidget* u2EvaluationMatrixTableWidget = nullptr;

	
	// 准则层隶属度
	QTableWidget* criterionableWidget = nullptr;
	// 目标层隶属度
	QTableWidget* targetTableWidget = nullptr;

	// 等级判定
	QTableWidget* levelTableWidget = nullptr;
	

	

};
