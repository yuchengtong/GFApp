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
	evaluationSystemTableWidget->setRowCount(26); // 行数
	evaluationSystemTableWidget->setColumnCount(7); // 列数
	evaluationSystemTableWidget->verticalHeader()->setVisible(false);
	evaluationSystemTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> evaluationSystemData = {
		{"层级", "名称", "场景分类", "指标编号", "指标名称", "指标含义", "指标类型"},
		{"目标层（U）", "固体导弹发动机安全性", "——", "——", "——", "综合安全等级判定", "评价目标"},
		{"准则层（U1）", "推进剂燃爆安全", "跌落场景", "U11", "推进剂超压", "跌落冲击下燃烧室最大超压", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "跌落场景", "U12", "推进剂温度", "跌落冲击下推进剂最高温度", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "快速烤燃场景", "U13", "推进剂超压", "快速升温下燃烧室最大超压", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "快速烤燃场景", "U14", "推进剂温度", "快速升温下推进剂最高温度", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "慢速烤燃场景", "U15", "推进剂超压", "慢速升温下燃烧室最大超压", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "慢速烤燃场景", "U16", "推进剂温度", "慢速升温下推进剂最高温度", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "子弹撞击场景", "U17", "推进剂超压", "子弹撞击下燃烧室最大超压", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "子弹撞击场景", "U18", "推进剂温度", "子弹撞击下推进剂最高温度", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "破片撞击场景", "U19", "推进剂超压", "破片撞击下燃烧室最大超压", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "破片撞击场景", "U110", "推进剂温度", "破片撞击下推进剂最高温度", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "殉爆场景", "U111", "推进剂超压", "殉爆冲击下燃烧室最大超压", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "殉爆场景", "U112", "推进剂温度", "殉爆冲击下推进剂最高温度", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "射流场景", "U113", "推进剂超压", "射流冲击下燃烧室最大超压", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "射流场景", "U114", "推进剂温度", "射流冲击下推进剂最高温度", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "爆炸冲击场景", "U115", "推进剂超压", "爆炸冲击下燃烧室最大超压", "定量"},
		{"准则层（U1）", "推进剂燃爆安全", "爆炸冲击场景", "U116", "推进剂温度", "爆炸冲击下推进剂最高温度", "定量"},
		{"准则层（U2）", "结构破损安全", "跌落场景", "U21", "壳体应力", "跌落冲击下燃烧室最大超压", "定量"},
		{"准则层（U2）", "结构破损安全", "快速烤燃场景", "U22", "壳体应力", "跌落冲击下燃烧室最大超压", "定量"},
		{"准则层（U2）", "结构破损安全", "慢速烤燃场景", "U23", "壳体应力", "跌落冲击下燃烧室最大超压", "定量"},
		{"准则层（U2）", "结构破损安全", "子弹撞击场景", "U24", "壳体应力", "跌落冲击下燃烧室最大超压", "定量"},
		{"准则层（U2）", "结构破损安全", "破片撞击场景", "U25", "壳体应力", "跌落冲击下燃烧室最大超压", "定量"},
		{"准则层（U2）", "结构破损安全", "殉爆场景", "U26", "壳体应力", "跌落冲击下燃烧室最大超压", "定量"},
		{"准则层（U2）", "结构破损安全", "射流场景", "U27", "壳体应力", "跌落冲击下燃烧室最大超压", "定量"},
		{"准则层（U2）", "结构破损安全", "爆炸冲击场景", "U28", "壳体应力", "跌落冲击下燃烧室最大超压", "定量"},
		
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
	evaluationSystemTableWidget->setSpan(2, 0, 16, 1);
	evaluationSystemTableWidget->setSpan(2, 1, 16, 1);
	evaluationSystemTableWidget->item(0, 2)->setTextAlignment(Qt::AlignCenter);
	evaluationSystemTableWidget->item(1, 2)->setTextAlignment(Qt::AlignCenter);
	evaluationSystemTableWidget->setSpan(18, 0, 8, 1);
	evaluationSystemTableWidget->setSpan(18, 1, 8, 1);
	evaluationSystemTableWidget->item(18, 0)->setTextAlignment(Qt::AlignCenter);
	evaluationSystemTableWidget->item(18, 1)->setTextAlignment(Qt::AlignCenter);

	m_tableWidget = evaluationSystemTableWidget;


	u1WeightTtableWidget = new QTableWidget();
	u1WeightTtableWidget->setRowCount(17); // 行数
	u1WeightTtableWidget->setColumnCount(17); // 列数
	u1WeightTtableWidget->verticalHeader()->setVisible(false);
	u1WeightTtableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> u1WeightData = {
		{"U1","U11", "U12", "U13", "U14", "U15", "U16", "U17", "U18", "U19", "U110" ,"U111", "U112", "U113", "U114", "U115" ,"U116" },
		{"U₁₁（跌落超压）","1", "3", "5", "7", "9", "9", "1/5", "1/7", "1/7", "1/9" ,"1/9", "1/9", "1/7", "1/9", "1/9" ,"1/9" },
		{"U₁₂（跌落温度）","1/3", "1", "3", "5", "7", "7", "1/7", "1/9", "1/9", "1/9" ,"1/9", "1/9", "1/9", "1/9", "1/9" ,"1/9" },
		{"U₁₃（快速烤燃超压）","1/5", "1/3", "1", "3", "5", "5", "1/9", "1/9", "1/9", "1/9" ,"1/9", "1/9", "1/9", "1/9", "1/9" ,"1/9" },
		{"U₁₄（快速烤燃温度）","1/7", "1/5", "1/3", "1", "3", "3", "1/9", "1/9", "1/9", "1/9" ,"1/9", "1/9", "1/9", "1/9", "1/9" ,"1/9" },
		{"U₁₅（慢速烤燃超压）","1/9", "1/7", "1/5", "1/3", "1", "3", "1/9", "1/9", "1/9", "1/9" ,"1/9", "1/9", "1/9", "1/9", "1/9" ,"1/9" },
		{"U₁₆（慢速烤燃温度）","1/9", "1/7", "1/5", "1/3", "1/3", "1", "1/9", "1/9", "1/9", "1/9" ,"1/9", "1/9", "1/9", "1/9", "1/9" ,"1/9" },
		{"U₁₇（子弹撞击超压）","5", "7", "9", "9", "9", "9", "1", "3", "3", "1/5" ,"1/7", "1/7", "1/3", "1/5", "1/7" ,"1/7" },
		{"U₁₈（子弹撞击温度）","7", "9", "9", "9", "9", "9", "1/3", "1", "3", "1/7" ,"1/9", "1/9", "1/5", "1/7", "1/9" ,"1/9" },
		{"U₁₉（破片撞击超压）","7", "9", "9", "9", "9", "9", "1/3", "1/3", "1", "1/3" ,"1/5", "1/5", "1/3", "1/3", "1/5" ,"1/5" },
		{"U₁₁₀（破片撞击温度）","9", "9", "9", "9", "9", "9", "5", "7", "3", "1" ,"3", "3", "1", "1/3", "1/5" ,"1/5" },
		{"U₁₁₁（殉爆超压）","9", "9", "9", "9", "9", "9", "7", "9", "5", "1/3" ,"1", "3", "1/3", "1/3", "1/3" ,"1/3" },
		{"U₁₁₂（殉爆温度）","9", "9", "9", "9", "9", "9", "7", "9", "5", "1/3" ,"1/3", "1", "1/3", "1/3", "1/3" ,"1/3" },
		{"U₁₁₃（射流超压）","7", "9", "9", "9", "9", "9", "3", "5", "3", "1" ,"3", "3", "1", "3", "1/3" ,"1/3" },
		{"U₁₁₄（射流温度）","9", "9", "9", "9", "9", "9", "5", "7", "3", "3" ,"3", "3", "1/3", "1", "1/5" ,"1/5" },
		{"U₁₁₅（爆炸冲击超压）","9", "9", "9", "9", "9", "9", "7", "9", "5", "5" ,"3", "3", "3", "5", "1" ,"3" },
		{"U₁₁₆（爆炸冲击温度）","9", "9", "9", "9", "9", "9", "7", "9", "5", "5" ,"3", "3", "3", "5", "1/3" ,"1" },
	};
	// 遍历二维数组并填充到QTableWidget中
	for (int i = 0; i < u1WeightData.size(); ++i) {
		for (int j = 0; j < u1WeightData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(u1WeightData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			u1WeightTtableWidget->setItem(i, j, item);
		}
	}
	u1WeightTtableWidget->resizeColumnsToContents(); // 根据内容调整列宽


	u2WeightTableWidget = new QTableWidget();
	u2WeightTableWidget->setRowCount(9); // 行数
	u2WeightTableWidget->setColumnCount(9); // 列数
	u2WeightTableWidget->verticalHeader()->setVisible(false);
	u2WeightTableWidget->horizontalHeader()->setVisible(false);

	std::vector<std::vector<QString>> u2WeightData = {
		{ "U2", "U₂₁（跌落）", "U₂₂（快速烤燃)", "U₂₃（慢速烤燃）", "U₂₄（子弹撞击）", "U₂₅（破片撞击）", "U₂₆（殉爆）", "U₂₇（射流）", "U₂₈（爆炸冲击）" },
		{ "U₂₁", "1", "3", "5", "1/5", "1/7", "1/9", "1/7", "1/9" },
		{ "U₂₂", "1/3", "1", "3", "1/7", "1/9", "1/9", "1/9", "1/9" },
		{ "U₂₃", "1/5", "1/3", "1", "1/9", "1/9", "1/9", "1/9", "1/9" },
		{ "U₂₄", "5", "7", "9", "1", "1/3", "1/5", "1/3", "1/7" },
		{ "U₂₅", "7", "9", "9", "3", "1", "1/3", "1/3", "1/5" },
		{ "U₂₆", "9", "9", "9", "5", "3", "1", "3", "1/3" },
		{ "U₂₇", "7", "9", "9", "3", "3", "1/3", "1", "1/5" },
		{ "U₂₈", "9", "9", "9", "7", "5", "3", "5", "1" },
		
	};
	for (int i = 0; i < u2WeightData.size(); ++i) {
		for (int j = 0; j < u2WeightData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(u2WeightData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
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
		{"U1权重计算与一致性检验", "U1权重向量", "A1= [0.025, 0.018, 0.012, 0.008, 0.005, 0.003, 0.065, 0.048, 0.082, 0.125, 0.156, 0.132, 0.098, 0.075, 0.168, 0.134] "},
		{" ", "U1一致性检验", "通过"},
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
		{"U2权重计算与一致性检验", "U2权重向量", "A2 = [0.042, 0.028, 0.015, 0.095, 0.132, 0.215, 0.158, 0.315] "},
		{" ", "U2一致性检验", "通过"},
	};
	for (int i = 0; i < u2CalculationData.size(); ++i) {
		for (int j = 0; j < u2CalculationData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(u2CalculationData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			u2CalculationTableWidget->setItem(i, j, item);
		}
	}
	u2CalculationTableWidget->resizeColumnsToContents(); // 根据内容调整列宽


	gradeDefinitionTableWidget = new QTableWidget();
	gradeDefinitionTableWidget->setRowCount(5); // 行数
	gradeDefinitionTableWidget->setColumnCount(5); // 列数
	gradeDefinitionTableWidget->verticalHeader()->setVisible(false);
	gradeDefinitionTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> gradeDefinitionData = {
		{"评语集", "壳体应力（[σ]=1200MPa）", "推进剂超压（P₀=25MPa）", "推进剂温度（T₀=300℃）", "含义"},
		{"V₁（优）", "≤840MPa（0.7[σ]）", "≤17.5MPa（0.7P₀）", "≤250℃（T₀-50）", "无风险"},
		{"V₂（良）", "841~960MPa（0.7-0.8[σ]）", "17.6~20MPa（0.7-0.8P₀）", "251~270℃（T₀-50~-30）", "风险可控"},
		{"V₃（中）", "961~1080MPa（0.8-0.9[σ]）", "20.1~22.5MPa（0.8-0.9P₀）", "271~290℃（T₀-30~-10）", "需专项管控"},
		{"V₄（差）", "＞1080MPa（＞0.9[σ]）", "＞22.5MPa（＞0.9P₀）", "＞290℃（＞T₀-10）", "易引发事故"},
	};
	for (int i = 0; i < gradeDefinitionData.size(); ++i) {
		for (int j = 0; j < gradeDefinitionData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(gradeDefinitionData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			gradeDefinitionTableWidget->setItem(i, j, item);
		}
	}
	gradeDefinitionTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	

	u1EvaluationMatrixTableWidget = new QTableWidget();
	u1EvaluationMatrixTableWidget->setRowCount(4); // 行数
	u1EvaluationMatrixTableWidget->setColumnCount(3); // 列数
	u1EvaluationMatrixTableWidget->verticalHeader()->setVisible(false);
	u1EvaluationMatrixTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> u1EvaluationMatrixData = {
		{"指标（示例）", "仿真值", "隶属度向量（V₁,V₂,V₃,V₄）"},
		{"跌落壳体应力", "900MPa", "（0,1,0,0）"},
		{"爆炸冲击推进剂超压", "21MPa", "（0,0.5,0.5,0）"},
		{"快速烤燃推进剂温度", "240℃", "（1,0,0,0）"},
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
	u2EvaluationMatrixTableWidget->setRowCount(4); // 行数
	u2EvaluationMatrixTableWidget->setColumnCount(3); // 列数
	u2EvaluationMatrixTableWidget->verticalHeader()->setVisible(false);
	u2EvaluationMatrixTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> u2EvaluationMatrixData = {
		{"指标（示例）", "仿真值", "隶属度向量（V₁,V₂,V₃,V₄）"},
		{"跌落壳体应力", "800MPa", "（0,1,0,0）"},
		{"爆炸冲击推进剂超压", "31MPa", "（0,0.5,0.5,0）"},
		{"快速烤燃推进剂温度", "250℃", "（1,0,0,0）"},
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



	criterionableWidget = new QTableWidget();
	criterionableWidget->setRowCount(3); // 行数
	criterionableWidget->setColumnCount(3); // 列数
	criterionableWidget->verticalHeader()->setVisible(false);
	criterionableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> criterionableData = {
		{"准则层隶属度", "公式", "数值"},
		{"U₁综合隶属度 B₁ ", "A₁×R₁", " [0.2, 0.4, 0.3, 0.1]"},
		{"U₂综合隶属度 B₂ ", "A₂×R₂", " [0.3, 0.3, 0.3, 0.1]"},
	};
	for (int i = 0; i < criterionableData.size(); ++i) {
		for (int j = 0; j < criterionableData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(criterionableData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			criterionableWidget->setItem(i, j, item);
		}
	}
	criterionableWidget->resizeColumnsToContents(); // 根据内容调整列宽



	targetTableWidget = new QTableWidget();
	targetTableWidget->setRowCount(2); // 行数
	targetTableWidget->setColumnCount(3); // 列数
	targetTableWidget->verticalHeader()->setVisible(false);
	targetTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> targetData = {
		{"目标层隶属度", "公式", "数值"},
		{"U₁综合隶属度 B₁ ", "A×[B₁; B₂] ", " [0.23, 0.37, 0.3, 0.1]"},
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


	levelTableWidget = new QTableWidget();
	levelTableWidget->setRowCount(2); // 行数
	levelTableWidget->setColumnCount(3); // 列数
	levelTableWidget->verticalHeader()->setVisible(false);
	levelTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> levelData = {
		{"等级判定", "数值", "等级"},
		{"最大隶属度", " 0.37  ", "  V₂（良，基本安全）"},
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


	m_tableStackWidget = new QStackedWidget();
	m_tableStackWidget->addWidget(evaluationSystemTableWidget);
	m_tableStackWidget->addWidget(u1WeightTtableWidget);
	m_tableStackWidget->addWidget(u2WeightTableWidget);
	m_tableStackWidget->addWidget(u1CalculationTableWidget);
	m_tableStackWidget->addWidget(u2CalculationTableWidget);
	m_tableStackWidget->addWidget(gradeDefinitionTableWidget);
	m_tableStackWidget->addWidget(u1EvaluationMatrixTableWidget);
	m_tableStackWidget->addWidget(u2EvaluationMatrixTableWidget);
	m_tableStackWidget->addWidget(criterionableWidget);
	m_tableStackWidget->addWidget(targetTableWidget);
	m_tableStackWidget->addWidget(levelTableWidget);

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
	else if (itemData == "GradeDefinition")
	{
		m_tableStackWidget->setCurrentWidget(gradeDefinitionTableWidget);
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
		m_tableStackWidget->setCurrentWidget(criterionableWidget);
	}
	else if (itemData == "Target")
	{
		m_tableStackWidget->setCurrentWidget(targetTableWidget);
	}
	
	else if (itemData == "LevelReport" || itemData == "Level")
	{
		m_tableStackWidget->setCurrentWidget(levelTableWidget);
	}
	
	
}
