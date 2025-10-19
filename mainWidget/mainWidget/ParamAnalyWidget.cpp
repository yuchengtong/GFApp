#pragma execution_character_set("utf-8")
#include "ParamAnalyWidget.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QSplitter>
#include <QHeaderView>


ParamAnalyWidget::ParamAnalyWidget(QWidget* parent)
	: QWidget(parent)
{

	m_treeModelWidget = new ParamAnalyTreeWidget();
	m_tableWidget = new QTableWidget();
	m_tableWidget->setRowCount(10); // 行数
	m_tableWidget->setColumnCount(7); // 列数
	// 隐藏行号
	m_tableWidget->verticalHeader()->setVisible(false);
	// 隐藏列号
	m_tableWidget->horizontalHeader()->setVisible(false);

	
	evaluationSystemTableWidget = new QTableWidget();
	evaluationSystemTableWidget->setRowCount(22); // 行数
	evaluationSystemTableWidget->setColumnCount(7); // 列数
	evaluationSystemTableWidget->verticalHeader()->setVisible(false);
	evaluationSystemTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> evaluationSystemData = {
		{"层级", "名称", "场景分类", "指标编号", "指标名称", "指标含义", "指标类型"},
		{"目标层（U）", "固体导弹发动机安全性", "——", "——", "——", "综合安全等级判定", "评价目标"},
		{"准则层（U1）", "推进剂燃爆安全", "跌落场景", "U11", "推进剂超压", "跌落冲击下燃烧室最大超压", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "跌落场景", "U12", "推进剂温度", "跌落冲击下推进剂最高温度", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "快速烤燃场景", "U13", "推进剂温度", "快速升温下推进剂最高温度", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "慢速烤燃场景", "U14", "推进剂温度", "慢速升温下推进剂最高温度", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "子弹撞击场景", "U15", "推进剂超压", "子弹撞击下燃烧室最大超压", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "子弹撞击场景", "U16", "推进剂温度", "子弹撞击下推进剂最高温度", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "破片撞击场景", "U17", "推进剂超压", "破片撞击下燃烧室最大超压", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "破片撞击场景", "U18", "推进剂温度", "破片撞击下推进剂最高温度", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "殉爆场景", "U19", "推进剂超压", "殉爆冲击下燃烧室最大超压", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "殉爆场景", "U110", "推进剂温度", "殉爆冲击下推进剂最高温度", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "射流场景", "U111", "推进剂超压", "射流冲击下燃烧室最大超压", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "射流场景", "U112", "推进剂温度", "射流冲击下推进剂最高温度", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "爆炸冲击场景", "U113", "推进剂超压", "爆炸冲击下燃烧室最大超压", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "爆炸冲击场景", "U114", "推进剂温度", "爆炸冲击下推进剂最高温度", "定量"},
		{"准则层（U2）", "结构破损安全", "跌落场景", "U21", "壳体应力", "跌落冲击下燃烧室最大超压", "定量"},
		{"准则层（U2）", "结构破损安全", "子弹撞击场景", "U22", "壳体应力", "跌落冲击下燃烧室最大超压", "定量"},
		{"准则层（U2）", "结构破损安全", "破片撞击场景", "U23", "壳体应力", "跌落冲击下燃烧室最大超压", "定量"},
		{"准则层（U2）", "结构破损安全", "殉爆场景", "U24", "壳体应力", "跌落冲击下燃烧室最大超压", "定量"},
		{"准则层（U2）", "结构破损安全", "射流场景", "U25", "壳体应力", "跌落冲击下燃烧室最大超压", "定量"},
		{"准则层（U2）", "结构破损安全", "爆炸冲击场景", "U26", "壳体应力", "跌落冲击下燃烧室最大超压", "定量"},
		
	};
	
	// 遍历二维数组并填充到QTableWidget中
	for (int i = 0; i < evaluationSystemData.size(); ++i) {
		for (int j = 0; j < evaluationSystemData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(evaluationSystemData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			evaluationSystemTableWidget->setItem(i, j, item);
		}
	}
	evaluationSystemTableWidget->resizeColumnsToContents(); // 根据内容调整列宽
	evaluationSystemTableWidget->setSpan(2, 0, 14, 1);
	evaluationSystemTableWidget->setSpan(2, 1, 14, 1);
	evaluationSystemTableWidget->item(0, 2)->setTextAlignment(Qt::AlignCenter);
	evaluationSystemTableWidget->item(1, 2)->setTextAlignment(Qt::AlignCenter);
	evaluationSystemTableWidget->setSpan(16, 0, 8, 1);
	evaluationSystemTableWidget->setSpan(16, 1, 8, 1);
	evaluationSystemTableWidget->item(16, 0)->setTextAlignment(Qt::AlignCenter);
	evaluationSystemTableWidget->item(16, 1)->setTextAlignment(Qt::AlignCenter);

	m_tableWidget = evaluationSystemTableWidget;

	weightTtableWidget = new QTableWidget();
	weightTtableWidget = new QTableWidget();
	weightTtableWidget->setRowCount(3); // 行数
	weightTtableWidget->setColumnCount(3); // 列数
	weightTtableWidget->verticalHeader()->setVisible(false);
	weightTtableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> weightData = {
		{"U","U₁（推进剂燃爆）", "U₂（结构破损）"},
		{"U₁","1", "3"},
		{"U₂","1/3", "1"},
	};
	// 遍历二维数组并填充到QTableWidget中
	for (int i = 0; i < weightData.size(); ++i) {
		for (int j = 0; j < weightData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(weightData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			weightTtableWidget->setItem(i, j, item);
		}
	}
	weightTtableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	u1WeightTtableWidget = new QTableWidget();
	u1WeightTtableWidget->setRowCount(15); // 行数
	u1WeightTtableWidget->setColumnCount(15); // 列数
	u1WeightTtableWidget->verticalHeader()->setVisible(false);
	u1WeightTtableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> u1WeightData = {
		{"U1","U11", "U12", "U13",  "U14", "U15", "U16", "U17", "U18" ,"U19", "U110", "U111", "U112", "U113" ,"U114" },
		{"U₁₁（跌落超压）","1", "3", "7", "9", "1/5", "1/7", "1/7", "1/9" ,"1/9", "1/9", "1/7", "1/9", "1/9" ,"1/9" },
		{"U₁₂（跌落温度）","1/3", "1",  "5", "7", "7", "1/7","1/9", "1/9", "1/9" ,"1/9", "1/9", "1/9", "1/9", "1/9"  },
		{"U₁₃（快速烤燃温度）","1/7", "1/3", "3", "3", "1/9", "1/9", "1/9", "1/9" ,"1/9", "1/9", "1/9", "1/9", "1/9" ,"1/9" },
		{"U₁₄（慢速烤燃温度）","1/9", "1/5", "1/3", "1", "1/9", "1/9", "1/9", "1/9" ,"1/9", "1/9", "1/9", "1/9", "1/9" ,"1/9" },
		{"U₁₅（子弹撞击超压）","5", "9", "9", "9", "1", "3", "3", "1/5" ,"1/7", "1/7", "1/3", "1/5", "1/7" ,"1/7" },
		{"U₁₆（子弹撞击温度）","7", "9", "9", "9", "1/3", "1", "3", "1/7" ,"1/9", "1/9", "1/5", "1/7", "1/9" ,"1/9" },
		{"U₁₇（破片撞击超压）","7", "9", "9", "9", "1/3", "1/3", "1", "1/3" ,"1/5", "1/5", "1/3", "1/3", "1/5" ,"1/5" },
		{"U₁₈（破片撞击温度）","9",  "9", "9", "9", "5", "7", "3", "1" ,"3", "3", "1", "1/3", "1/5" ,"1/5" },
		{"U₁₉（殉爆超压）","9", "9", "9", "9", "7", "9", "5", "1/3" ,"1", "3", "1/3", "1/3", "1/3" ,"1/3" },
		{"U₁₁₀（殉爆温度）","9", "9", "9", "9", "7", "9", "5", "1/3" ,"1/3", "1", "1/3", "1/3", "1/3" ,"1/3" },
		{"U₁₁₁（射流超压）","7", "9",   "9", "9", "3", "5", "3", "1" ,"3", "3", "1", "3", "1/3" ,"1/3" },
		{"U₁₁₂（射流温度）","9", "9",  "9", "9", "5", "7", "3", "3" ,"3", "3", "1/3", "1", "1/5" ,"1/5" },
		{"U₁₁₃（爆炸冲击超压）","9", "9", "9", "9", "7", "9", "5", "5" ,"3", "3", "3", "5", "1" ,"3" },
		{"U₁₁₄（爆炸冲击温度）","9", "9", "9", "9", "7", "9", "5", "5" ,"3", "3", "3", "5", "1/3" ,"1" },
	};
	// 遍历二维数组并填充到QTableWidget中
	for (int i = 0; i < u1WeightData.size(); ++i) {
		for (int j = 0; j < u1WeightData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(u1WeightData[i][j]);
			if (i == 0 || j == 0)
			{
				item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			}
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			u1WeightTtableWidget->setItem(i, j, item);
		}
	}
	u1WeightTtableWidget->resizeColumnsToContents(); // 根据内容调整列宽


	u2WeightTableWidget = new QTableWidget();
	u2WeightTableWidget->setRowCount(7); // 行数
	u2WeightTableWidget->setColumnCount(7); // 列数
	u2WeightTableWidget->verticalHeader()->setVisible(false);
	u2WeightTableWidget->horizontalHeader()->setVisible(false);

	std::vector<std::vector<QString>> u2WeightData = {
		{ "U2", "U₂₁（跌落）", "U₂₂（子弹撞击)", "U₂₃（破片撞击）", "U₂₄（殉爆）", "U₂₅（射流）", "U₂₆（爆炸冲击）" },
		{ "U₂₁", "1", "1/5", "1/7", "1/9", "1/7", "1/9" },
		{ "U₂₂", "5", "1", "1/3", "1/5", "1/3", "1/7" },
		{ "U₂₃", "7", "3", "1", "1/3", "1/3", "1/5" },
		{ "U₂₄", "9", "5", "3", "1", "3", "1/3" },
		{ "U₂₅", "7", "3", "3", "1/3", "1", "1/5" },
		{ "U₂₆", "9", "7", "5", "3", "5", "1" },
		
	};
	for (int i = 0; i < u2WeightData.size(); ++i) {
		for (int j = 0; j < u2WeightData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(u2WeightData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0 || j == 0)
			{
				item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			}
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			u2WeightTableWidget->setItem(i, j, item);
		}
	}
	u2WeightTableWidget->resizeColumnsToContents(); // 根据内容调整列宽


	u1CalculationTableWidget = new QTableWidget();
	u1CalculationTableWidget->setRowCount(2); // 行数
	u1CalculationTableWidget->setColumnCount(3); // 列数
	u1CalculationTableWidget->verticalHeader()->setVisible(false);
	u1CalculationTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> u1CalculationData = {
		{"U1权重计算与一致性检验", "U1权重向量", " "},
		{" ", "U1一致性检验", " "},
	};
	for (int i = 0; i < u1CalculationData.size(); ++i) {
		for (int j = 0; j < u1CalculationData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(u1CalculationData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			u1CalculationTableWidget->setItem(i, j, item);
		}
	}
	u1CalculationTableWidget->resizeColumnsToContents(); // 根据内容调整列宽


	u2CalculationTableWidget = new QTableWidget();
	u2CalculationTableWidget->setRowCount(2); // 行数
	u2CalculationTableWidget->setColumnCount(3); // 列数
	u2CalculationTableWidget->verticalHeader()->setVisible(false);
	u2CalculationTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> u2CalculationData = {
		{"U2权重计算与一致性检验", "U2权重向量", " "},
		{" ", "U2一致性检验", " "},
	};
	for (int i = 0; i < u2CalculationData.size(); ++i) {
		for (int j = 0; j < u2CalculationData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(u2CalculationData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			u2CalculationTableWidget->setItem(i, j, item);
		}
	}
	u2CalculationTableWidget->resizeColumnsToContents(); // 根据内容调整列宽


	u1GradeDefinitionTableWidget = new QTableWidget();
	u1GradeDefinitionTableWidget->setRowCount(5); // 行数
	u1GradeDefinitionTableWidget->setColumnCount(16); // 列数
	u1GradeDefinitionTableWidget->verticalHeader()->setVisible(false);
	u1GradeDefinitionTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> u1GradeDefinitionData = {
		{"评语集", "跌落场景推进剂超压（P₁）", "跌落场景推进剂温度（T₁）", "快速烤燃场景推进剂温度（T₂）", "慢速烤燃场景推进剂温度（T₃）", "子弹撞击场景推进剂超压（P₄）", "子弹撞击场景推进剂温度（T₄）", "破片撞击场景推进剂超压（P₅）", "破片撞击场景推进剂温度（T₅）", "殉爆场景推进剂超压（P₆）", "殉爆场景推进剂温度（T₆）", "射流场景推进剂超压（P₇）", "射流场景推进剂温度（T₇）","爆炸冲击场景推进剂超压（P₈=）", "爆炸冲击场景推进剂温度（T₈）", "含义"},
		{"V₁（优）", "≤ 0.7P₁", "≤T₁-50", "≤T₂-50", "≤T₃-50","≤ 0.7P₄", "≤T₄-50","≤ 0.7P₅", "≤T₅-50","≤ 0.7P₆", "≤T₆-50","≤ 0.7P₇", "≤T₇-50","≤ 0.7P₈", "≤T₈-50", "无风险"},
		{"V₂（良）", "0.7-0.8P₁", "T₁-50~-30", "T₂-50~-30",  "T₃-50~-30", "0.7-0.8P₄", "T₄-50~-30", "0.7-0.8P₅", "T₅-50~-30", "0.7-0.8P₆", "T₆-50~-30", "0.7-0.8P₇", "T₇-50~-30", "0.7-0.8P₈", "T₈-50~-30", "风险可控"},
		{"V₃（中）", "0.8-0.9P₁", "T₁-30~-10", "T₂-30~-10", "T₃-30~-10","0.8-0.9P₄", "T₄-30~-10","0.8-0.9P₅", "T₅-30~-10","0.8-0.9P₆", "T₆-30~-10","0.8-0.9P₇", "T₇-30~-10","0.8-0.9P₈", "T₈-30~-10", "需专项管控"},
		{"V₄（差）", "＞0.9P₁", "＞T₁-10", "＞T₂-10", "＞T₃-10","＞0.9P₄", "＞T₄-10","＞0.9P₅", "＞T₅-10","＞0.9P₆", "＞T₆-10","＞0.9P₇", "＞T₇-10","＞0.9P₈", "＞T₈-10", "易引发事故"},

	};
	for (int i = 0; i < u1GradeDefinitionData.size(); ++i) {
		for (int j = 0; j < u1GradeDefinitionData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(u1GradeDefinitionData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			u1GradeDefinitionTableWidget->setItem(i, j, item);
		}
	}
	u1GradeDefinitionTableWidget->resizeColumnsToContents(); // 根据内容调整列宽


	u2GradeDefinitionTableWidget = new QTableWidget();
	u2GradeDefinitionTableWidget->setRowCount(5); // 行数
	u2GradeDefinitionTableWidget->setColumnCount(8); // 列数
	u2GradeDefinitionTableWidget->verticalHeader()->setVisible(false);
	u2GradeDefinitionTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> u2GradeDefinitionData = {
		{"评语集", "跌落场景壳体应力（σ₁）", "子弹撞击场景壳体应力（σ₂）", "破片撞击场景壳体应力（σ₃）", "殉爆场景壳体应力（σ₄）", "射流场景壳体应力（σ₅）", "爆炸冲击场景壳体应力（σ₆）", "含义"},
		{"V₁（优）", "≤0.7σ₁","≤0.7σ₂","≤0.7σ₃","≤0.7σ₄","≤0.7σ₅","≤0.7σ₆", "无风险"},
		{"V₂（良）", "0.7-0.8σ₁","0.7-0.8σ₂","0.7-0.8σ₃","0.7-0.8σ₄","0.7-0.8σ₅","0.7-0.8σ₆", "风险可控"},
		{"V₃（中）", "0.8-0.9σ₁","0.8-0.9σ₂","0.8-0.9σ₃","0.8-0.9σ₄","0.8-0.9σ₅","0.8-0.9σ₆", "需专项管控"},
		{"V₄（差）", ">0.9σ₁",">0.9σ₂",">0.9σ₃",">0.9σ₄",">0.9σ₅",">0.9σ₆", "易引发事故"}, };
	for (int i = 0; i < u2GradeDefinitionData.size(); ++i) {
		for (int j = 0; j < u2GradeDefinitionData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(u2GradeDefinitionData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			u2GradeDefinitionTableWidget->setItem(i, j, item);
		}
	}
	u2GradeDefinitionTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	

	u1EvaluationMatrixTableWidget = new QTableWidget();
	u1EvaluationMatrixTableWidget->setRowCount(15); // 行数
	u1EvaluationMatrixTableWidget->setColumnCount(3); // 列数
	u1EvaluationMatrixTableWidget->verticalHeader()->setVisible(false);
	u1EvaluationMatrixTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> u1EvaluationMatrixData = {
		{"指标", "仿真值   ", "隶属度向量（V₁,V₂,V₃,V₄）"},
		{"跌落推进剂超压", " ", " "},
		{"跌落推进剂温度", " ", " "},
		{"快速烤燃推进剂温度", " ", " "},
		{"慢速烤燃推进剂剂温度", " ", " "},
		{"子弹撞击推进剂超压", " ", " "},
		{"子弹撞击推进剂温度", " ", " "},
		{"破片撞击推进剂超压", " ", " "},
		{"破片撞击推进剂温度", " ", " "},
		{"殉爆推进剂超压", " ", " "},
		{"殉爆推进剂温度", " ", " "},
		{"射流推进剂超压", " ", " "},
		{"射流推进剂温度", " ", " "},
		{"爆炸冲击推进剂超压", " ", " "},
		{"爆炸冲击推进剂温度", " ", " "},
	};
	for (int i = 0; i < u1EvaluationMatrixData.size(); ++i) {
		for (int j = 0; j < u1EvaluationMatrixData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(u1EvaluationMatrixData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			u1EvaluationMatrixTableWidget->setItem(i, j, item);
		}
	}
	u1EvaluationMatrixTableWidget->resizeColumnsToContents(); // 根据内容调整列宽


	u2EvaluationMatrixTableWidget = new QTableWidget();
	u2EvaluationMatrixTableWidget->setRowCount(7); // 行数
	u2EvaluationMatrixTableWidget->setColumnCount(3); // 列数
	u2EvaluationMatrixTableWidget->verticalHeader()->setVisible(false);
	u2EvaluationMatrixTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> u2EvaluationMatrixData = {
		{"指标", "仿真值   ", "隶属度向量（V₁,V₂,V₃,V₄）"},
		{"跌落壳体应力", " ", " "},
		{"子弹撞击壳体应力", " ", " "},
		{"破片撞击壳体应力", " ", " "},
		{"殉爆壳体应力", " ", " "},
		{"射流壳体应力", " ", " "},
		{"爆炸冲击壳体应力", " ", " "},
	};
	for (int i = 0; i < u2EvaluationMatrixData.size(); ++i) {
		for (int j = 0; j < u2EvaluationMatrixData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(u2EvaluationMatrixData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			u2EvaluationMatrixTableWidget->setItem(i, j, item);
		}
	}
	u2EvaluationMatrixTableWidget->resizeColumnsToContents(); // 根据内容调整列宽



	criterionTableWidget = new QTableWidget();
	criterionTableWidget->setRowCount(3); // 行数
	criterionTableWidget->setColumnCount(3); // 列数
	criterionTableWidget->verticalHeader()->setVisible(false);
	criterionTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> criterionableData = {
		{"准则层隶属度", "公式", "数值"},
		{"U₁综合隶属度 B₁ ", "A₁×R₁", "  "},
		{"U₂综合隶属度 B₂ ", "A₂×R₂", "  "},
	};
	for (int i = 0; i < criterionableData.size(); ++i) {
		for (int j = 0; j < criterionableData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(criterionableData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			criterionTableWidget->setItem(i, j, item);
		}
	}
	criterionTableWidget->resizeColumnsToContents(); // 根据内容调整列宽



	targetTableWidget = new QTableWidget();
	targetTableWidget->setRowCount(2); // 行数
	targetTableWidget->setColumnCount(3); // 列数
	targetTableWidget->verticalHeader()->setVisible(false);
	targetTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> targetData = {
		{"目标层隶属度", "公式", "数值"},
		{"U₁综合隶属度 B₁ ", "A×[B₁; B₂] ", " "},
	};
	for (int i = 0; i < targetData.size(); ++i) {
		for (int j = 0; j < targetData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(targetData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			targetTableWidget->setItem(i, j, item);
		}
	}
	targetTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	levelReportTableWidget = new QTableWidget();
	levelReportTableWidget->setRowCount(13); // 行数
	levelReportTableWidget->setColumnCount(5); // 列数
	levelReportTableWidget->verticalHeader()->setVisible(false);
	levelReportTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> levelReportData = {
		{"等级定量标准", " ", " ", " ", " "},
		{"指标类型", "V₁（优）", "V₂（良）", "V₃（中）", "V₄（差）"},
		{"壳体应力", "σ ≤ 0.7 [σ]）", "0.7[σ] < σ ≤ 0.8[σ]", "0.8[σ] < σ ≤ 0.9[σ]", "σ > 0.9[σ]"},
		{"推进剂超压", "P ≤ 0.7P₀", "0.7P₀ < P ≤ 0.8P₀", "0.8P₀ < P ≤ 0.9P₀", "P > 0.9P₀"},
		{"推进剂温度", "T ≤ T₀-50℃", "T₀-50℃ < T ≤ T₀-30℃", "T₀-30℃ < T ≤ T₀-10℃", "T > T₀-10℃"},
		{"等级含义", "无安全风险，所有参数远低于阈值", "风险可控，参数在安全冗余内", "存在潜在风险，需专项管控", "风险不可控，易引发事故"},
		{" ", " ", " ", " ", " "},
		{"基础分值标准", " ", " ", " ", " "},
		{"评语等级", "安全含义", "分值（满分 100）", "决策建议", " "},
		{"V₁（优）", "无安全风险，所有参数远低于阈值", "85~100 分", "无需额外管控，可正常投入使用", " "},
		{"V₂（良）", "低风险，参数在安全冗余内", "70~84 分", "常规巡检，关注重点指标", " "},
		{"V₃（中）", "中风险，参数接近阈值需管控", "50~69 分", "停机专项排查，优化设计", " "},
		{"V₄（差）", "高风险，参数超阈值易引发事故", "0~49 分", "立即停用，全面整改后重新评估", " "},
	};
	for (int i = 0; i < levelReportData.size(); ++i) {
		for (int j = 0; j < levelReportData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(levelReportData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0 || i== 7)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			levelReportTableWidget->setItem(i, j, item);
		}
	}
	levelReportTableWidget->setSpan(8 , 3, 1, 2);
	levelReportTableWidget->setSpan(9 , 3, 1, 2);
	levelReportTableWidget->setSpan(10, 3, 1, 2);
	levelReportTableWidget->setSpan(11, 3, 1, 2);
	levelReportTableWidget->resizeColumnsToContents(); // 根据内容调整列宽


	levelTableWidget = new QTableWidget();
	levelTableWidget->setRowCount(2); // 行数
	levelTableWidget->setColumnCount(3); // 列数
	levelTableWidget->verticalHeader()->setVisible(false);
	levelTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> levelData = {
		{"等级判定", "数值", "等级"},
		{"最大隶属度", "  ", "  "},
	};
	for (int i = 0; i < levelData.size(); ++i) {
		for (int j = 0; j < levelData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(levelData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			levelTableWidget->setItem(i, j, item);
		}
	}
	levelTableWidget->resizeColumnsToContents(); // 根据内容调整列宽


	scoreTableWidget = new QTableWidget();
	scoreTableWidget->setRowCount(2); // 行数
	scoreTableWidget->setColumnCount(3); // 列数
	scoreTableWidget->verticalHeader()->setVisible(false);
	scoreTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> scoreData = {
		{"评分", "数值", "等级"},
		{"分数评定", "  ", "  "},
	};
	for (int i = 0; i < scoreData.size(); ++i) {
		for (int j = 0; j < scoreData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(scoreData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			scoreTableWidget->setItem(i, j, item);
		}
	}
	scoreTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_tableStackWidget = new QStackedWidget();
	m_tableStackWidget->addWidget(evaluationSystemTableWidget);
	m_tableStackWidget->addWidget(weightTtableWidget);
	m_tableStackWidget->addWidget(u1WeightTtableWidget);
	m_tableStackWidget->addWidget(u2WeightTableWidget);
	m_tableStackWidget->addWidget(u1CalculationTableWidget);
	m_tableStackWidget->addWidget(u2CalculationTableWidget);
	m_tableStackWidget->addWidget(u1GradeDefinitionTableWidget);
	m_tableStackWidget->addWidget(u2GradeDefinitionTableWidget);
	m_tableStackWidget->addWidget(u1EvaluationMatrixTableWidget);
	m_tableStackWidget->addWidget(u2EvaluationMatrixTableWidget);
	m_tableStackWidget->addWidget(criterionTableWidget);
	m_tableStackWidget->addWidget(targetTableWidget);
	m_tableStackWidget->addWidget(levelReportTableWidget);
	m_tableStackWidget->addWidget(levelTableWidget);
	m_tableStackWidget->addWidget(scoreTableWidget);

	// ------ 左侧垂直分割器（树结构与属性表） ------
	auto leftSplitter = new QSplitter(Qt::Vertical);
	leftSplitter->addWidget(m_treeModelWidget);
	leftSplitter->setContentsMargins(0, 0, 0, 0);
	// 设置分割器的Handle宽度为0（消除视觉间隙）
	leftSplitter->setHandleWidth(1);



	// ------ 右侧垂直分割器（树结构与属性表） ------
	auto rightSplitter = new QSplitter(Qt::Vertical);
	rightSplitter->addWidget(m_tableStackWidget);
	rightSplitter->setContentsMargins(0, 0, 0, 0);
	// 设置分割器的Handle宽度为0（消除视觉间隙）
	rightSplitter->setHandleWidth(1);


	// ------ 主水平分割器（左侧与右侧） ------
	auto mainSplitter = new QSplitter(Qt::Horizontal);
	mainSplitter->addWidget(leftSplitter);
	mainSplitter->addWidget(rightSplitter);
	mainSplitter->setContentsMargins(0, 0, 0, 0);
	// 设置分割器的Handle宽度为0（消除视觉间隙）
	mainSplitter->setHandleWidth(1);

	mainSplitter->setStretchFactor(0, 2);
	mainSplitter->setStretchFactor(1, 8);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(mainSplitter);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	// 连接信号和槽
	connect(m_treeModelWidget, &ParamAnalyTreeWidget::itemClicked, this, &ParamAnalyWidget::onTreeItemClicked);
}

ParamAnalyWidget::~ParamAnalyWidget()
{}


void ParamAnalyWidget::onTreeItemClicked(const QString& itemData)
{
	if (itemData == "EvaluationSystem" || itemData == "ParamAnaly") {
		m_tableStackWidget->setCurrentWidget(evaluationSystemTableWidget);
	}
	else if (itemData == "Weight")
	{
		m_tableStackWidget->setCurrentWidget(weightTtableWidget);
	}
	else if (itemData == "Weight" || itemData == "U1Weight" )
	{
		m_tableStackWidget->setCurrentWidget(u1WeightTtableWidget);
	}
	else if (itemData == "U2Weight")
	{
		m_tableStackWidget->setCurrentWidget(u2WeightTableWidget);
	}
	else if (itemData == "Calculation" || itemData == "U1Calculation")
	{
		m_tableStackWidget->setCurrentWidget(u1CalculationTableWidget);
	}
	else if (itemData == "U2Calculation")
	{
		m_tableStackWidget->setCurrentWidget(u2CalculationTableWidget);
	}
	else if (itemData == "GradeDefinition" || itemData == "u1GradeDefinition" )
	{
		m_tableStackWidget->setCurrentWidget(u1GradeDefinitionTableWidget);
	}
	else if (itemData == "u2GradeDefinition")
	{
		m_tableStackWidget->setCurrentWidget(u2GradeDefinitionTableWidget);
	}
	else if (itemData == "EvaluationMatrix" || itemData == "U1EvaluationMatrix")
	{
		m_tableStackWidget->setCurrentWidget(u1EvaluationMatrixTableWidget);
	}
	else if (itemData == "U2EvaluationMatrix")
	{
		m_tableStackWidget->setCurrentWidget(u2EvaluationMatrixTableWidget);
	}
	else if (itemData == "MatrixOperation" || itemData == "Criterion")
	{
		m_tableStackWidget->setCurrentWidget(criterionTableWidget);
	}
	else if (itemData == "Target")
	{
		m_tableStackWidget->setCurrentWidget(targetTableWidget);
	}
	else if (itemData == "LevelReport")
	{
		m_tableStackWidget->setCurrentWidget(levelReportTableWidget);
	}
	else if (itemData == "Level")
	{
		m_tableStackWidget->setCurrentWidget(levelTableWidget);
	}
	else if (itemData == "Score")
	{
		m_tableStackWidget->setCurrentWidget(scoreTableWidget);
	}
}
