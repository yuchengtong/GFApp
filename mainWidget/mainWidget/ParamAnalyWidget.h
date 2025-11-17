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

	QTableWidget* getEvaluationSystemTableWidget()	{		return evaluationSystemTableWidget;	}

	QTableWidget* getWeightTtableWidget() { return weightTtableWidget; }
	
	QTableWidget* getU1WeightTtableWidget()	{		return u1WeightTtableWidget;	}

	QTableWidget* getU2WeightTtableWidget()	{		return u2WeightTableWidget;	}

	QTableWidget* getU1CalculationTableWidget()	{		return u1CalculationTableWidget;	}

	QTableWidget* getU2CalculationTableWidget()	{		return u2CalculationTableWidget;	}

	QTableWidget* getU1GradeDefinitionTableWidget()	{		return u1GradeDefinitionTableWidget;	}

	QTableWidget* getU2GradeDefinitionTableWidget() { return u2GradeDefinitionTableWidget; }

	QTableWidget* getU1EvaluationMatrixTableWidget()	{		return u1EvaluationMatrixTableWidget;	}

	QTableWidget* getU2EvaluationMatrixTableWidget()	{		return u2EvaluationMatrixTableWidget;	}

	QTableWidget* getCriterionTableWidget()	{		return criterionTableWidget;	}

	QTableWidget* getTargetTableWidget()	{		return targetTableWidget;	}

	QTableWidget* getLevelTableWidget()	{		return levelTableWidget;	}

	QTableWidget* getScoreTableWidget() { return scoreTableWidget; }

	void exportWord(const QString& directory, const QString& text);

private slots:
	void onTreeItemClicked(const QString& itemData);

private:

	ParamAnalyTreeWidget* m_treeModelWidget = nullptr;
	QTableWidget* m_tableWidget = nullptr;
	QStackedWidget* m_tableStackWidget = nullptr;

	// 三级评价体系设计
	QTableWidget* evaluationSystemTableWidget = nullptr;


	// 层级指标权重设计
	QTableWidget* weightTtableWidget = nullptr;
	// U1指标权重设计
	QTableWidget* u1WeightTtableWidget = nullptr;
	// U2指标权重设计
	QTableWidget* u2WeightTableWidget = nullptr;

	
	// U1权重向量A1与一致性Rc1计算
	QTableWidget* u1CalculationTableWidget = nullptr;
	// U2权重向量A2与一致性Rc2计算
	QTableWidget* u2CalculationTableWidget = nullptr;

	// U1等级定量标准
	QTableWidget* u1GradeDefinitionTableWidget = nullptr;
	// U2等级定量标准
	QTableWidget* u2GradeDefinitionTableWidget = nullptr;

	// U1权重向量A1与一致性Rc1计算
	QTableWidget* u1EvaluationMatrixTableWidget = nullptr;
	// U2权重向量A1与一致性Rc1计算
	QTableWidget* u2EvaluationMatrixTableWidget = nullptr;

	
	// 准则层隶属度
	QTableWidget* criterionTableWidget = nullptr;
	// 目标层隶属度
	QTableWidget* targetTableWidget = nullptr;

	// 安全登记判定报告
	QTableWidget* levelReportTableWidget = nullptr;
	// 等级判定
	QTableWidget* levelTableWidget = nullptr;
	// 分数评定
	QTableWidget* scoreTableWidget = nullptr;
	

	

};
