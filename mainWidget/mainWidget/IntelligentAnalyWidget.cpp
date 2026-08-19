#pragma execution_character_set("utf-8")
#include "IntelligentAnalyWidget.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QSplitter>
#include <QHeaderView>

// 自绘线条指示器：用于自定义图例中区分实线/虚线
class LineIndicator : public QWidget
{
public:
	LineIndicator(Qt::PenStyle style, const QColor& color, QWidget* parent = nullptr)
		: QWidget(parent), m_style(style), m_color(color)
	{
		setFixedSize(36, 14);
	}
protected:
	void paintEvent(QPaintEvent*) override
	{
		QPainter p(this);
		p.setRenderHint(QPainter::Antialiasing);
		QPen pen(m_color);
		pen.setWidth(2);
		pen.setStyle(m_style);
		if (m_style == Qt::DashLine)
			pen.setDashPattern(QVector<qreal>{5, 3});
		p.setPen(pen);
		p.drawLine(2, height() / 2, width() - 2, height() / 2);
	}
private:
	Qt::PenStyle m_style;
	QColor m_color;
};


QLegendMarker* getSeriesMarker(QChart* chart, QAbstractSeries* series)
{
	for (auto marker : chart->legend()->markers())
	{
		if (marker->series() == series)
			return marker;
	}
	return nullptr;
}


IntelligentAnalyWidget::IntelligentAnalyWidget(QWidget* parent)
	: QWidget(parent)
{
	m_treeModelWidget = new IntelligentAnalyTreeWidget();
	m_propertyStackWidget = new QStackedWidget();
	m_tableStackWidget = new QStackedWidget();

	// 设置m_PropertyStackWidget的背景为白色
	m_propertyStackWidget->setStyleSheet("background-color: white;");

	m_intelligentPropertyWidget = new IntelligentPropertyWidget();
	m_fallPropertyWidget = new IntelligentFallPropertyWidget();
	m_fastCombustionPropertyWidget = new IntelligentFastCombustionPropertyWidget();
	m_slowCombustionPropertyWidget = new IntelligentSlowCombustionPropertyWidget();
	m_shootPropertyWidget = new IntelligentShootPropertyWidget();
	m_jetImpactPropertyWidget = new IntelligentJetImpactPropertyWidget();
	m_fragmentationImpactPropertyWidget = new IntelligentFragmentationImpactPropertyWidget();
	m_explosiveBlastPropertyWidget = new IntelligentExplosiveBlastPropertyWidget();
	m_sacrificeExplosionPropertyWidget = new IntelligentSacrificeExplosionPropertyWidget();


	// 将所有的 PropertyWidget 添加到 QStackedWidget 中
	m_propertyStackWidget->addWidget(m_intelligentPropertyWidget);
	m_propertyStackWidget->addWidget(m_fallPropertyWidget);
	m_propertyStackWidget->addWidget(m_fastCombustionPropertyWidget);
	m_propertyStackWidget->addWidget(m_slowCombustionPropertyWidget);
	m_propertyStackWidget->addWidget(m_shootPropertyWidget);
	m_propertyStackWidget->addWidget(m_jetImpactPropertyWidget);
	m_propertyStackWidget->addWidget(m_fragmentationImpactPropertyWidget);
	m_propertyStackWidget->addWidget(m_explosiveBlastPropertyWidget);
	m_propertyStackWidget->addWidget(m_sacrificeExplosionPropertyWidget);



	m_fallTableWidget = new QTableWidget();
	m_fallTableWidget->setRowCount(26); // 行数
	m_fallTableWidget->setColumnCount(7); // 列数
	m_fallTableWidget->verticalHeader()->setVisible(false);
	m_fallTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_fallData = {
		{ "设计点", "壳体厚度[mm]", "跌落高度[m]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "10", " ", " ", " ", " " },
		{ "2", "1.5", "10", " ", " ", " ", " " },
		{ "3", "2", "10", " ", " ", " ", " " },
		{ "4", "2.5", "10", " ", " ", " ", " " },
		{ "5", "3", "10", " ", " ", " ", " " },
		{ "6", "1", "30", " ", " ", " ", " " },
		{ "7", "1.5", "30", " ", " ", " ", " " },
		{ "8", "2", "30", " ", " ", " ", " " },
		{ "9", "2.5", "30", " ", " ", " ", " " },
		{ "10", "3", "30", " ", " ", " ", " " },
		{ "11", "1", "50", " ", " ", " ", " " },
		{ "12", "1.5", "50", " ", " ", " ", " " },
		{ "13", "2", "50", " ", " ", " ", " " },
		{ "14", "2.5", "50", " ", " ", " ", " " },
		{ "15", "3", "50", " ", " ", " ", " " },
		{ "16", "1", "70", " ", " ", " ", " " },
		{ "17", "1.5", "70", " ", " ", " ", " " },
		{ "18", "2", "70", " ", " ", " ", " " },
		{ "19", "2.5", "70", " ", " ", " ", " " },
		{ "20", "3", "70", " ", " ", " ", " " },
		{ "21", "1", "90", " ", " ", " ", " " },
		{ "22", "1.5", "90", " ", " ", " ", " " },
		{ "23", "2", "90", " ", " ", " ", " " },
		{ "24", "2.5", "90", " ", " ", " ", " " },
		{ "25", "3", "90", " ", " ", " ", " " },

	};
	for (int i = 0; i < m_fallData.size(); ++i) {
		for (int j = 0; j < m_fallData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_fallData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_fallTableWidget->setItem(i, j, item);
		}
	}
	m_fallTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_fastCombustionTableWidget = new QTableWidget();
	m_fastCombustionTableWidget->setRowCount(26); // 行数
	m_fastCombustionTableWidget->setColumnCount(7); // 列数
	m_fastCombustionTableWidget->verticalHeader()->setVisible(false);
	m_fastCombustionTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_fastCombustionData = {
		{ "设计点", "壳体厚度[mm]", "快烤平均温度[℃]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "600", " ", " ", " ", " " },
		{ "2", "1.5", "600", " ", " ", " ", " " },
		{ "3", "2", "600", " ", " ", " ", " " },
		{ "4", "2.5", "600", " ", " ", " ", " " },
		{ "5", "3", "600", " ", " ", " ", " " },
		{ "6", "1", "650", " ", " ", " ", " " },
		{ "7", "1.5", "650", " ", " ", " ", " " },
		{ "8", "2", "650", " ", " ", " ", " " },
		{ "9", "2.5", "650", " ", " ", " ", " " },
		{ "10", "3", "650", " ", " ", " ", " " },
		{ "11", "1", "700", " ", " ", " ", " " },
		{ "12", "1.5", "700", " ", " ", " ", " " },
		{ "13", "2", "700", " ", " ", " ", " " },
		{ "14", "2.5", "700", " ", " ", " ", " " },
		{ "15", "3", "700", " ", " ", " ", " " },
		{ "16", "1", "750", " ", " ", " ", " " },
		{ "17", "1.5", "750", " ", " ", " ", " " },
		{ "18", "2", "750", " ", " ", " ", " " },
		{ "19", "2.5", "750", " ", " ", " ", " " },
		{ "20", "3", "750", " ", " ", " ", " " },
		{ "21", "1", "800", " ", " ", " ", " " },
		{ "22", "1.5", "800", " ", " ", " ", " " },
		{ "23", "2", "800", " ", " ", " ", " " },
		{ "24", "2.5", "800", " ", " ", " ", " " },
		{ "25", "3", "800", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_fastCombustionData.size(); ++i) {
		for (int j = 0; j < m_fastCombustionData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_fastCombustionData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_fastCombustionTableWidget->setItem(i, j, item);
		}
	}
	m_fastCombustionTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_slowCombustionTableWidget = new QTableWidget();
	m_slowCombustionTableWidget->setRowCount(26); // 行数
	m_slowCombustionTableWidget->setColumnCount(7); // 列数
	m_slowCombustionTableWidget->verticalHeader()->setVisible(false);
	m_slowCombustionTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_slowCombustionData = {
		{ "设计点", "壳体厚度[mm]", "烘箱终止温度[℃]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "315", " ", " ", " ", " " },
		{ "2", "1.5", "315", " ", " ", " ", " " },
		{ "3", "2", "315", " ", " ", " ", " " },
		{ "4", "2.5", "315", " ", " ", " ", " " },
		{ "5", "3", "315", " ", " ", " ", " " },
		{ "6", "1", "320", " ", " ", " ", " " },
		{ "7", "1.5", "320", " ", " ", " ", " " },
		{ "8", "2", "320", " ", " ", " ", " " },
		{ "9", "2.5", "320", " ", " ", " ", " " },
		{ "10", "3", "320", " ", " ", " ", " " },
		{ "11", "1", "325", " ", " ", " ", " " },
		{ "12", "1.5", "325", " ", " ", " ", " " },
		{ "13", "2", "325", " ", " ", " ", " " },
		{ "14", "2.5", "325", " ", " ", " ", " " },
		{ "15", "3", "325", " ", " ", " ", " " },
		{ "16", "1", "330", " ", " ", " ", " " },
		{ "17", "1.5", "330", " ", " ", " ", " " },
		{ "18", "2", "330", " ", " ", " ", " " },
		{ "19", "2.5", "330", " ", " ", " ", " " },
		{ "20", "3", "330", " ", " ", " ", " " },
		{ "21", "1", "345", " ", " ", " ", " " },
		{ "22", "1.5", "345", " ", " ", " ", " " },
		{ "23", "2", "345", " ", " ", " ", " " },
		{ "24", "2.5", "345", " ", " ", " ", " " },
		{ "25", "3", "345", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_slowCombustionData.size(); ++i) {
		for (int j = 0; j < m_slowCombustionData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_slowCombustionData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_slowCombustionTableWidget->setItem(i, j, item);
		}
	}
	m_slowCombustionTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_shootTableWidget = new QTableWidget();
	m_shootTableWidget->setRowCount(26); // 行数
	m_shootTableWidget->setColumnCount(7); // 列数
	m_shootTableWidget->verticalHeader()->setVisible(false);
	m_shootTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_shootData = {
		{ "设计点", "壳体厚度[mm]", "撞击速度[m/s]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "620", " ", " ", " ", " " },
		{ "2", "1.5", "620", " ", " ", " ", " " },
		{ "3", "2", "620", " ", " ", " ", " " },
		{ "4", "2.5", "620", " ", " ", " ", " " },
		{ "5", "3", "620", " ", " ", " ", " " },
		{ "6", "1", "670", " ", " ", " ", " " },
		{ "7", "1.5", "670", " ", " ", " ", " " },
		{ "8", "2", "670", " ", " ", " ", " " },
		{ "9", "2.5", "670", " ", " ", " ", " " },
		{ "10", "3", "670", " ", " ", " ", " " },
		{ "11", "1", "720", " ", " ", " ", " " },
		{ "12", "1.5", "720", " ", " ", " ", " " },
		{ "13", "2", "720", " ", " ", " ", " " },
		{ "14", "2.5", "720", " ", " ", " ", " " },
		{ "15", "3", "720", " ", " ", " ", " " },
		{ "16", "1", "770", " ", " ", " ", " " },
		{ "17", "1.5", "770", " ", " ", " ", " " },
		{ "18", "2", "770", " ", " ", " ", " " },
		{ "19", "2.5", "770", " ", " ", " ", " " },
		{ "20", "3", "770", " ", " ", " ", " " },
		{ "21", "1", "820", " ", " ", " ", " " },
		{ "22", "1.5", "820", " ", " ", " ", " " },
		{ "23", "2", "820", " ", " ", " ", " " },
		{ "24", "2.5", "820", " ", " ", " ", " " },
		{ "25", "3", "820", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_shootData.size(); ++i) {
		for (int j = 0; j < m_shootData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_shootData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_shootTableWidget->setItem(i, j, item);
		}
	}
	m_shootTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_jetImpactTableWidget = new QTableWidget();
	m_jetImpactTableWidget->setRowCount(26); // 行数
	m_jetImpactTableWidget->setColumnCount(7); // 列数
	m_jetImpactTableWidget->verticalHeader()->setVisible(false);
	m_jetImpactTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_jetImpactData = {
		{ "设计点", "壳体厚度[mm]", "聚能装药口径[mm]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "30", " ", " ", " ", " " },
		{ "2", "1.5", "30", " ", " ", " ", " " },
		{ "3", "2", "30", " ", " ", " ", " " },
		{ "4", "2.5", "30", " ", " ", " ", " " },
		{ "5", "3", "30", " ", " ", " ", " " },
		{ "6", "1", "35", " ", " ", " ", " " },
		{ "7", "1.5", "35", " ", " ", " ", " " },
		{ "8", "2", "35", " ", " ", " ", " " },
		{ "9", "2.5", "35", " ", " ", " ", " " },
		{ "10", "3", "35", " ", " ", " ", " " },
		{ "11", "1", "40", " ", " ", " ", " " },
		{ "12", "1.5", "40", " ", " ", " ", " " },
		{ "13", "2", "40", " ", " ", " ", " " },
		{ "14", "2.5", "40", " ", " ", " ", " " },
		{ "15", "3", "40", " ", " ", " ", " " },
		{ "16", "1", "45", " ", " ", " ", " " },
		{ "17", "1.5", "45", " ", " ", " ", " " },
		{ "18", "2", "45", " ", " ", " ", " " },
		{ "19", "2.5", "45", " ", " ", " ", " " },
		{ "20", "3", "45", " ", " ", " ", " " },
		{ "21", "1", "50", " ", " ", " ", " " },
		{ "22", "1.5", "50", " ", " ", " ", " " },
		{ "23", "2", "50", " ", " ", " ", " " },
		{ "24", "2.5", "50", " ", " ", " ", " " },
		{ "25", "3", "50", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_jetImpactData.size(); ++i) {
		for (int j = 0; j < m_jetImpactData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_jetImpactData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_jetImpactTableWidget->setItem(i, j, item);
		}
	}
	m_jetImpactTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_fragmentationImpactTableWidget = new QTableWidget();
	m_fragmentationImpactTableWidget->setRowCount(26); // 行数
	m_fragmentationImpactTableWidget->setColumnCount(7); // 列数
	m_fragmentationImpactTableWidget->verticalHeader()->setVisible(false);
	m_fragmentationImpactTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_fragmentationImpactData = {
		{ "设计点", "壳体厚度[mm]", "撞击速度[m/s]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "1630", " ", " ", " ", " " },
		{ "2", "1.5", "1630", " ", " ", " ", " " },
		{ "3", "2", "1630", " ", " ", " ", " " },
		{ "4", "2.5", "1630", " ", " ", " ", " " },
		{ "5", "3", "1630", " ", " ", " ", " " },
		{ "6", "1", "1680", " ", " ", " ", " " },
		{ "7", "1.5", "1680", " ", " ", " ", " " },
		{ "8", "2", "1680", " ", " ", " ", " " },
		{ "9", "2.5", "1680", " ", " ", " ", " " },
		{ "10", "3", "1680", " ", " ", " ", " " },
		{ "11", "1", "1730", " ", " ", " ", " " },
		{ "12", "1.5", "1730", " ", " ", " ", " " },
		{ "13", "2", "1730", " ", " ", " ", " " },
		{ "14", "2.5", "1730", " ", " ", " ", " " },
		{ "15", "3", "1730", " ", " ", " ", " " },
		{ "16", "1", "1780", " ", " ", " ", " " },
		{ "17", "1.5", "1780", " ", " ", " ", " " },
		{ "18", "2", "1780", " ", " ", " ", " " },
		{ "19", "2.5", "1780", " ", " ", " ", " " },
		{ "20", "3", "1780", " ", " ", " ", " " },
		{ "21", "1", "1830", " ", " ", " ", " " },
		{ "22", "1.5", "1830", " ", " ", " ", " " },
		{ "23", "2", "1830", " ", " ", " ", " " },
		{ "24", "2.5", "1830", " ", " ", " ", " " },
		{ "25", "3", "1830", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_fragmentationImpactData.size(); ++i) {
		for (int j = 0; j < m_fragmentationImpactData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_fragmentationImpactData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_fragmentationImpactTableWidget->setItem(i, j, item);
		}
	}
	m_fragmentationImpactTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_explosiveBlastTableWidget = new QTableWidget();
	m_explosiveBlastTableWidget->setRowCount(26); // 行数
	m_explosiveBlastTableWidget->setColumnCount(7); // 列数
	m_explosiveBlastTableWidget->verticalHeader()->setVisible(false);
	m_explosiveBlastTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_explosiveBlastData = {
		{ "设计点", "壳体厚度[mm]", "TNT当量[g]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "30", " ", " ", " ", " " },
		{ "2", "1.5", "30", " ", " ", " ", " " },
		{ "3", "2", "30", " ", " ", " ", " " },
		{ "4", "2.5", "30", " ", " ", " ", " " },
		{ "5", "3", "30", " ", " ", " ", " " },
		{ "6", "1", "35", " ", " ", " ", " " },
		{ "7", "1.5", "35", " ", " ", " ", " " },
		{ "8", "2", "35", " ", " ", " ", " " },
		{ "9", "2.5", "35", " ", " ", " ", " " },
		{ "10", "3", "35", " ", " ", " ", " " },
		{ "11", "1", "40", " ", " ", " ", " " },
		{ "12", "1.5", "40", " ", " ", " ", " " },
		{ "13", "2", "40", " ", " ", " ", " " },
		{ "14", "2.5", "40", " ", " ", " ", " " },
		{ "15", "3", "40", " ", " ", " ", " " },
		{ "16", "1", "45", " ", " ", " ", " " },
		{ "17", "1.5", "45", " ", " ", " ", " " },
		{ "18", "2", "45", " ", " ", " ", " " },
		{ "19", "2.5", "45", " ", " ", " ", " " },
		{ "20", "3", "45", " ", " ", " ", " " },
		{ "21", "1", "50", " ", " ", " ", " " },
		{ "22", "1.5", "50", " ", " ", " ", " " },
		{ "23", "2", "50", " ", " ", " ", " " },
		{ "24", "2.5", "50", " ", " ", " ", " " },
		{ "25", "3", "50", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_explosiveBlastData.size(); ++i) {
		for (int j = 0; j < m_explosiveBlastData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_explosiveBlastData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_explosiveBlastTableWidget->setItem(i, j, item);
		}
	}
	m_explosiveBlastTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_sacrificeExplosionTableWidget = new QTableWidget();
	m_sacrificeExplosionTableWidget->setRowCount(26); // 行数
	m_sacrificeExplosionTableWidget->setColumnCount(7); // 列数
	m_sacrificeExplosionTableWidget->verticalHeader()->setVisible(false);
	m_sacrificeExplosionTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_sacrificeExplosionData = {
		{ "设计点", "壳体厚度[mm]", "殉爆距离[mm]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "80", " ", " ", " ", " " },
		{ "2", "1.5", "80", " ", " ", " ", " " },
		{ "3", "2", "80", " ", " ", " ", " " },
		{ "4", "2.5", "80", " ", " ", " ", " " },
		{ "5", "3", "80", " ", " ", " ", " " },
		{ "6", "1", "85", " ", " ", " ", " " },
		{ "7", "1.5", "85", " ", " ", " ", " " },
		{ "8", "2", "85", " ", " ", " ", " " },
		{ "9", "2.5", "85", " ", " ", " ", " " },
		{ "10", "3", "85", " ", " ", " ", " " },
		{ "11", "1", "90", " ", " ", " ", " " },
		{ "12", "1.5", "90", " ", " ", " ", " " },
		{ "13", "2", "90", " ", " ", " ", " " },
		{ "14", "2.5", "90", " ", " ", " ", " " },
		{ "15", "3", "90", " ", " ", " ", " " },
		{ "16", "1", "95", " ", " ", " ", " " },
		{ "17", "1.5", "95", " ", " ", " ", " " },
		{ "18", "2", "95", " ", " ", " ", " " },
		{ "19", "2.5", "95", " ", " ", " ", " " },
		{ "20", "3", "95", " ", " ", " ", " " },
		{ "21", "1", "100", " ", " ", " ", " " },
		{ "22", "1.5", "100", " ", " ", " ", " " },
		{ "23", "2", "100", " ", " ", " ", " " },
		{ "24", "2.5", "100", " ", " ", " ", " " },
		{ "25", "3", "100", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_sacrificeExplosionData.size(); ++i) {
		for (int j = 0; j < m_sacrificeExplosionData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_sacrificeExplosionData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_sacrificeExplosionTableWidget->setItem(i, j, item);
		}
	}
	m_sacrificeExplosionTableWidget->resizeColumnsToContents(); // 根据内容调整列宽


	m_tableStackWidget->addWidget(m_fallTableWidget);
	m_tableStackWidget->addWidget(m_fastCombustionTableWidget);
	m_tableStackWidget->addWidget(m_slowCombustionTableWidget);
	m_tableStackWidget->addWidget(m_shootTableWidget);
	m_tableStackWidget->addWidget(m_jetImpactTableWidget);
	m_tableStackWidget->addWidget(m_fragmentationImpactTableWidget);
	m_tableStackWidget->addWidget(m_explosiveBlastTableWidget);
	m_tableStackWidget->addWidget(m_sacrificeExplosionTableWidget);
	m_tableWidget = m_fallTableWidget;

	m_dataMap["壳体厚度"] = QStringList() << "1" << "1.5" << "2" << "2.5" << "3";
	m_dataMap["跌落高度"] = QStringList() << "10" << "30" << "50" << "70" << "90";
	m_dataMap["快烤平均温度"] = QStringList() << "600" << "650" << "700" << "750" << "800";
	m_dataMap["慢烤平均温度"] = QStringList() << "315" << "320" << "325" << "330" << "345";
	m_dataMap["枪击速度"] = QStringList() << "620" << "670" << "720" << "770" << "820";
	m_dataMap["破片撞击速度"] = QStringList() << "1630" << "1680" << "1730" << "1780" << "1830";
	m_dataMap["聚能装药口径"] = QStringList() << "30" << "35" << "40" << "45" << "50";
	m_dataMap["TNT当量"] = QStringList() << "30" << "35" << "40" << "45" << "50";
	m_dataMap["殉爆距离"] = QStringList() << "80" << "85" << "90" << "95" << "100";

	graphicWid = new QWidget();
	graphicLayout = new QHBoxLayout();
	QLabel* x_label = new QLabel("X轴：");
	x_comboBox = new QComboBox();
	x_comboBox->addItems({ "壳体厚度", "跌落高度" });
	x_comboBox->setFixedWidth(180);
	QLabel* y_label = new QLabel("Y轴：");
	y_comboBox = new QComboBox();
	y_comboBox->addItems({ "壳体最大应力 ", "推进剂最大应力", "壳体最高温度", "推进剂最高温度" });

	// 连接信号槽
	connect(x_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &IntelligentAnalyWidget::onComboBoxIndexChanged);

	connect(y_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &IntelligentAnalyWidget::onComboBoxIndexChanged);



	m_chart = new QChart();
	m_chart->setTitle("正交试验");
	m_chart->setMargins(QMargins(15, 15, 15, 0));
	createChartDataGroup(m_lineSeries1, m_scatter1, m_quadSeries1, "壳体厚度:1mm", Qt::red);
	createChartDataGroup(m_lineSeries2, m_scatter2, m_quadSeries2, "壳体厚度:2mm", Qt::blue);
	createChartDataGroup(m_lineSeries3, m_scatter3, m_quadSeries3, "壳体厚度:3mm", Qt::green);

	m_chart->addSeries(m_lineSeries1);
	m_chart->addSeries(m_scatter1);
	m_chart->addSeries(m_lineSeries2);
	m_chart->addSeries(m_scatter2);
	m_chart->addSeries(m_lineSeries3);
	m_chart->addSeries(m_scatter3);
	m_chart->addSeries(m_quadSeries1);
	m_chart->addSeries(m_quadSeries2);
	m_chart->addSeries(m_quadSeries3);
	if (auto marker = getSeriesMarker(m_chart, m_scatter1))
		marker->setVisible(false);
	if (auto marker = getSeriesMarker(m_chart, m_scatter2))
		marker->setVisible(false);
	if (auto marker = getSeriesMarker(m_chart, m_scatter3))
		marker->setVisible(false);

	m_chart->legend()->hide();
	/*m_chart->legend()->setVisible(true);
	m_chart->legend()->setAlignment(Qt::AlignRight);
	m_chart->legend()->setFont(QFont("Microsoft YaHei", 8));
	m_chart->legend()->setMaximumWidth(720);
	m_chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);*/

	// 4. 创建X轴和Y轴（初始范围适配空数据）
	m_axisX = new QValueAxis();
	m_axisY = new QValueAxis();
	m_axisX->setTitleText("壳体厚度");
	m_axisY->setTitleText("壳体最大应力");
	m_axisX->setRange(0, 5);
	m_axisY->setRange(0, 10);
	m_axisX->setTickCount(4);
	m_axisY->setTickCount(4);

	// 5. 所有系列绑定到同一组轴（确保曲线和点对齐）
	QList<QAbstractSeries*> allSeries = {
		m_lineSeries1, m_scatter1, m_lineSeries2, m_scatter2, m_lineSeries3, m_scatter3, m_quadSeries1, m_quadSeries2, m_quadSeries3
	};
	for (QAbstractSeries* series : allSeries) {
		m_chart->setAxisX(m_axisX, series);
		m_chart->setAxisY(m_axisY, series);
	}

	m_chartView = new QChartView(m_chart);
	m_chartView->setBackgroundBrush(QBrush(Qt::white));
	m_chartView->setRenderHint(QPainter::Antialiasing);  // 抗锯齿
	m_chartView->setMinimumHeight(400);


	// 构建布局
	QVBoxLayout* m_leftLayout = new QVBoxLayout();
	m_leftLayout->setSpacing(0);
	m_leftLayout->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout* labelLayou = new QHBoxLayout();
	labelLayou->addWidget(x_label);
	labelLayou->addWidget(x_comboBox);
	labelLayou->addWidget(y_label);
	labelLayou->addWidget(y_comboBox);
	labelLayou->setContentsMargins(0, 0, 0, 0);
	labelLayou->addStretch(200);

	m_leftLayout->addLayout(labelLayou);
	m_leftLayout->addWidget(m_chartView);

	// ===== 自定义图例：2行布局，第一行实线(一次拟合)，第二行虚线(二次拟合) =====
	m_customLegend = new QWidget();
	m_customLegend->setStyleSheet("background-color: white;");
	QGridLayout* legendLayout = new QGridLayout(m_customLegend);
	legendLayout->setContentsMargins(8, 0, 8, 4);
	legendLayout->setHorizontalSpacing(16);
	legendLayout->setVerticalSpacing(2);
	m_linLabel1 = new QLabel("壳体厚度:1mm");
	m_linLabel2 = new QLabel("壳体厚度:2mm");
	m_linLabel3 = new QLabel("壳体厚度:3mm");
	m_quadLabel1 = new QLabel("壳体厚度:1mm(二次)");
	m_quadLabel2 = new QLabel("壳体厚度:2mm(二次)");
	m_quadLabel3 = new QLabel("壳体厚度:3mm(二次)");
	legendLayout->addWidget(createLegendItem(Qt::SolidLine, Qt::red, m_linLabel1), 0, 0);
	legendLayout->addWidget(createLegendItem(Qt::DashLine, Qt::red, m_quadLabel1), 0, 1);
	legendLayout->addWidget(createLegendItem(Qt::SolidLine, Qt::blue, m_linLabel2), 1, 0);
	legendLayout->addWidget(createLegendItem(Qt::DashLine, Qt::blue, m_quadLabel2), 1, 1);
	legendLayout->addWidget(createLegendItem(Qt::SolidLine, Qt::green, m_linLabel3), 2, 0);
	legendLayout->addWidget(createLegendItem(Qt::DashLine, Qt::green, m_quadLabel3), 2, 1);
	m_leftLayout->addWidget(m_customLegend);


	// 三维图形
	QLabel* grapgicLabel = new QLabel("结果集：");
	m_grapgicComboBox = new QComboBox();
	m_grapgicComboBox->addItems({ "壳体最大应力 ", "推进剂最大应力", "壳体最高温度", "推进剂最高温度" });
	connect(m_grapgicComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &IntelligentAnalyWidget::onComboBoxIndexGraphicChanged);

	QHBoxLayout* choseLayou = new QHBoxLayout();
	choseLayou->addWidget(grapgicLabel);
	choseLayou->addWidget(m_grapgicComboBox);
	choseLayou->setContentsMargins(0, 0, 0, 0);
	choseLayou->addStretch(200);


	m_3dGraphicWid = new GraphicWidget();

	// 构建布局
	QVBoxLayout* m_rightLayout = new QVBoxLayout();
	m_rightLayout->addLayout(choseLayou);
	m_rightLayout->addWidget(m_3dGraphicWid);

	graphicLayout->addLayout(m_leftLayout, 1);
	graphicLayout->addLayout(m_rightLayout, 1);
	graphicWid->setLayout(graphicLayout);


	// ------ 左侧垂直分割器（树结构与属性表） ------
	auto leftSplitter = new QSplitter(Qt::Vertical);
	leftSplitter->addWidget(m_treeModelWidget);
	leftSplitter->addWidget(m_propertyStackWidget);
	leftSplitter->setStretchFactor(0, 3);
	leftSplitter->setStretchFactor(1, 1);
	leftSplitter->setContentsMargins(0, 0, 0, 0);
	// 设置分割器的Handle宽度为0（消除视觉间隙）
	leftSplitter->setHandleWidth(1);



	// ------ 右侧垂直分割器（树结构与属性表） ------
	auto rightSplitter = new QSplitter(Qt::Vertical);
	rightSplitter->addWidget(m_tableStackWidget);
	rightSplitter->addWidget(graphicWid);
	rightSplitter->setStretchFactor(0, 3);
	rightSplitter->setStretchFactor(1, 1);
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

	mainSplitter->setStretchFactor(0, 1);
	mainSplitter->setStretchFactor(1, 1);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(mainSplitter);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	// 连接信号和槽
	connect(m_treeModelWidget, &IntelligentAnalyTreeWidget::itemClicked, this, &IntelligentAnalyWidget::onTreeItemClicked);
}

IntelligentAnalyWidget::~IntelligentAnalyWidget()
{}


void IntelligentAnalyWidget::onTreeItemClicked(const QString& itemData)
{
	QString yName = "跌落高度";
	if (itemData == "IntelligentAnaly") {
		m_propertyStackWidget->setCurrentWidget(m_intelligentPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_fallTableWidget);
		m_tableWidget = m_fallTableWidget;
		x_comboBox->setItemText(1, "跌落高度");
		yName = "跌落高度";
	}
	else if (itemData == "FallIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_fallPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_fallTableWidget);
		m_tableWidget = m_fallTableWidget;
		x_comboBox->setItemText(1, "跌落高度");
		yName = "跌落高度";
	}

	else if (itemData == "FastCombustionIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_fastCombustionPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_fastCombustionTableWidget);
		m_tableWidget = m_fastCombustionTableWidget;
		x_comboBox->setItemText(1, "快烤平均温度");
		yName = "快烤平均温度";
	}
	else if (itemData == "SlowCombustionIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_slowCombustionPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_slowCombustionTableWidget);
		m_tableWidget = m_slowCombustionTableWidget;
		x_comboBox->setItemText(1, "慢烤平均温度");
		yName = "慢烤平均温度";
	}
	else if (itemData == "ShootIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_shootPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_shootTableWidget);
		m_tableWidget = m_shootTableWidget;
		x_comboBox->setItemText(1, "枪击速度");
		yName = "枪击速度";
	}
	else if (itemData == "JetImpactIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_jetImpactPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_jetImpactTableWidget);
		m_tableWidget = m_jetImpactTableWidget;
		x_comboBox->setItemText(1, "聚能装药口径");
		yName = "聚能装药口径";
	}
	else if (itemData == "FragmentationImpactIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_fragmentationImpactPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_fragmentationImpactTableWidget);
		m_tableWidget = m_fragmentationImpactTableWidget;
		x_comboBox->setItemText(1, "破片撞击速度");
		yName = "破片撞击速度";
	}
	else if (itemData == "ExplosiveBlastIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_explosiveBlastPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_explosiveBlastTableWidget);
		m_tableWidget = m_explosiveBlastTableWidget;
		x_comboBox->setItemText(1, "TNT当量");
		yName = "TNT当量";
	}
	else if (itemData == "SacrificeExplosionIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_sacrificeExplosionPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_sacrificeExplosionTableWidget);
		m_tableWidget = m_sacrificeExplosionTableWidget;
		x_comboBox->setItemText(1, "殉爆距离");
		yName = "殉爆距离";
	}

	onComboBoxIndexChanged(0);

	// 更新三维模型数据
	QVector<QVector<double>> newData;
	QVector<double> data1;
	data1.append(m_tableWidget->item(1, 3)->text().toDouble());
	data1.append(m_tableWidget->item(6, 3)->text().toDouble());
	data1.append(m_tableWidget->item(11, 3)->text().toDouble());
	data1.append(m_tableWidget->item(16, 3)->text().toDouble());
	data1.append(m_tableWidget->item(21, 3)->text().toDouble());
	newData.append(data1);
	QVector<double> data2;
	data2.append(m_tableWidget->item(2, 3)->text().toDouble());
	data2.append(m_tableWidget->item(7, 3)->text().toDouble());
	data2.append(m_tableWidget->item(12, 3)->text().toDouble());
	data2.append(m_tableWidget->item(17, 3)->text().toDouble());
	data2.append(m_tableWidget->item(22, 3)->text().toDouble());
	newData.append(data2);
	QVector<double> data3;
	data3.append(m_tableWidget->item(3, 3)->text().toDouble());
	data3.append(m_tableWidget->item(8, 3)->text().toDouble());
	data3.append(m_tableWidget->item(13, 3)->text().toDouble());
	data3.append(m_tableWidget->item(18, 3)->text().toDouble());
	data3.append(m_tableWidget->item(23, 3)->text().toDouble());
	newData.append(data3);
	QVector<double> data4;
	data4.append(m_tableWidget->item(4, 3)->text().toDouble());
	data4.append(m_tableWidget->item(9, 3)->text().toDouble());
	data4.append(m_tableWidget->item(14, 3)->text().toDouble());
	data4.append(m_tableWidget->item(19, 3)->text().toDouble());
	data4.append(m_tableWidget->item(24, 3)->text().toDouble());
	newData.append(data4);
	QVector<double> data5;
	data5.append(m_tableWidget->item(5, 3)->text().toDouble());
	data5.append(m_tableWidget->item(10, 3)->text().toDouble());
	data5.append(m_tableWidget->item(15, 3)->text().toDouble());
	data5.append(m_tableWidget->item(20, 3)->text().toDouble());
	data5.append(m_tableWidget->item(25, 3)->text().toDouble());
	newData.append(data5);

	QVector<double> xCoords;
	xCoords.append(m_tableWidget->item(1, 1)->text().toDouble());
	xCoords.append(m_tableWidget->item(2, 1)->text().toDouble());
	xCoords.append(m_tableWidget->item(3, 1)->text().toDouble());
	xCoords.append(m_tableWidget->item(4, 1)->text().toDouble());
	xCoords.append(m_tableWidget->item(5, 1)->text().toDouble());

	QVector<double> yCoords;
	yCoords.append(m_tableWidget->item(1, 2)->text().toDouble());
	yCoords.append(m_tableWidget->item(6, 2)->text().toDouble());
	yCoords.append(m_tableWidget->item(11, 2)->text().toDouble());
	yCoords.append(m_tableWidget->item(16, 2)->text().toDouble());
	yCoords.append(m_tableWidget->item(21, 2)->text().toDouble());

	updateGraphicData("壳体厚度", yName, "壳体最大应力", xCoords, yCoords, newData,
		m_tableWidget->item(1, 1)->text().toDouble(), m_tableWidget->item(5, 1)->text().toDouble(),
		m_tableWidget->item(1, 2)->text().toDouble(), m_tableWidget->item(25, 2)->text().toDouble());

}

void IntelligentAnalyWidget::onComboBoxIndexChanged(int index)
{
	x_comboBox->currentIndex();
	dataChange(0);
}

void IntelligentAnalyWidget::onComboBoxIndexGraphicChanged(int index)
{
	int z = 3;
	QString zName = "壳体最大应力";
	if (index == 0)
	{
		z = 3;
		zName = "壳体最大应力";
	}
	else if (index == 1)
	{
		z = 4;
		zName = "推进剂最大应力";
	}
	else if (index == 2)
	{
		z = 5;
		zName = "壳体最高温度";
	}
	else if (index == 3)
	{
		z = 6;
		zName = "推进剂最高温度";
	}
	// 更新三维模型数据
	QVector<QVector<double>> newData;
	QVector<double> data1;
	data1.append(m_tableWidget->item(1, z)->text().toDouble());
	data1.append(m_tableWidget->item(6, z)->text().toDouble());
	data1.append(m_tableWidget->item(11, z)->text().toDouble());
	data1.append(m_tableWidget->item(16, z)->text().toDouble());
	data1.append(m_tableWidget->item(21, z)->text().toDouble());
	newData.append(data1);
	QVector<double> data2;
	data2.append(m_tableWidget->item(2, z)->text().toDouble());
	data2.append(m_tableWidget->item(7, z)->text().toDouble());
	data2.append(m_tableWidget->item(12, z)->text().toDouble());
	data2.append(m_tableWidget->item(17, z)->text().toDouble());
	data2.append(m_tableWidget->item(22, z)->text().toDouble());
	newData.append(data2);
	QVector<double> data3;
	data3.append(m_tableWidget->item(3, z)->text().toDouble());
	data3.append(m_tableWidget->item(8, z)->text().toDouble());
	data3.append(m_tableWidget->item(13, z)->text().toDouble());
	data3.append(m_tableWidget->item(18, z)->text().toDouble());
	data3.append(m_tableWidget->item(23, z)->text().toDouble());
	newData.append(data3);
	QVector<double> data4;
	data4.append(m_tableWidget->item(4, z)->text().toDouble());
	data4.append(m_tableWidget->item(9, z)->text().toDouble());
	data4.append(m_tableWidget->item(14, z)->text().toDouble());
	data4.append(m_tableWidget->item(19, z)->text().toDouble());
	data4.append(m_tableWidget->item(24, z)->text().toDouble());
	newData.append(data4);
	QVector<double> data5;
	data5.append(m_tableWidget->item(5, z)->text().toDouble());
	data5.append(m_tableWidget->item(10, z)->text().toDouble());
	data5.append(m_tableWidget->item(15, z)->text().toDouble());
	data5.append(m_tableWidget->item(20, z)->text().toDouble());
	data5.append(m_tableWidget->item(25, z)->text().toDouble());
	newData.append(data5);



	QVector<double> xCoords;
	xCoords.append(m_tableWidget->item(1, 1)->text().toDouble());
	xCoords.append(m_tableWidget->item(2, 1)->text().toDouble());
	xCoords.append(m_tableWidget->item(3, 1)->text().toDouble());
	xCoords.append(m_tableWidget->item(4, 1)->text().toDouble());
	xCoords.append(m_tableWidget->item(5, 1)->text().toDouble());

	QVector<double> yCoords;
	yCoords.append(m_tableWidget->item(1, 2)->text().toDouble());
	yCoords.append(m_tableWidget->item(6, 2)->text().toDouble());
	yCoords.append(m_tableWidget->item(11, 2)->text().toDouble());
	yCoords.append(m_tableWidget->item(16, 2)->text().toDouble());
	yCoords.append(m_tableWidget->item(21, 2)->text().toDouble());


	updateGraphicData("壳体厚度", "", zName, xCoords, yCoords, newData,
		m_tableWidget->item(1, 1)->text().toDouble(), m_tableWidget->item(5, 1)->text().toDouble(),
		m_tableWidget->item(1, 2)->text().toDouble(), m_tableWidget->item(25, 2)->text().toDouble());
}

void IntelligentAnalyWidget::dataChange(int index)
{
	QList<int> rowList1;
	QList<int> rowList2;
	QList<int> rowList3;
	int x_col = 3;
	int y_col = 1;

	int x_index = x_comboBox->currentIndex();
	int y_index = y_comboBox->currentIndex();
	if (x_index == 0)
	{
		rowList1 = { 1,2,3,4,5 };
		rowList2 = { 11,12,13,14,15 };
		rowList3 = { 21,22,23,24,25 };
		x_col = 1;
	}
	else
	{
		rowList1 = { 1,6,11,16,21 };
		rowList2 = { 3,8,13,18,23 };
		rowList3 = { 5,10,15,20,25 };
		x_col = 2;
	}


	if (y_index == 0)
	{
		y_col = 3;
	}
	else if (y_index == 1)
	{
		y_col = 4;
	}
	else if (y_index == 2)
	{
		y_col = 5;
	}
	else if (y_index == 3)
	{
		y_col = 6;
	}

	QVector<QPointF> data1;
	foreach(int row, rowList1) {
		if (m_tableWidget->item(row, x_col) && m_tableWidget->item(row, y_col))
		{
			data1.append(QPointF(m_tableWidget->item(row, x_col)->text().toDouble(), m_tableWidget->item(row, y_col)->text().toDouble()));
		}
	}
	QVector<QPointF> data2;
	foreach(int row, rowList2) {
		if (m_tableWidget->item(row, x_col) && m_tableWidget->item(row, y_col))
		{
			data2.append(QPointF(m_tableWidget->item(row, x_col)->text().toDouble(), m_tableWidget->item(row, y_col)->text().toDouble()));
		}
	}
	QVector<QPointF> data3;
	foreach(int row, rowList3) {
		if (m_tableWidget->item(row, x_col) && m_tableWidget->item(row, y_col))
		{
			data3.append(QPointF(m_tableWidget->item(row, x_col)->text().toDouble(), m_tableWidget->item(row, y_col)->text().toDouble()));
		}
	}

	if (x_index == 0)
	{
		QString name1, name2, name3;
		QString text = m_tableWidget->item(0, 2)->text();
		int startIndex = text.indexOf('[');
		int endIndex = text.indexOf(']');
		if (startIndex != -1 && endIndex != -1 && endIndex > startIndex) {
			QString titleText = text.mid(0, startIndex) + ":";
			QString extractedText = text.mid(startIndex + 1, endIndex - startIndex - 1);
			QString leg1 = m_tableWidget->item(1, 2)->text();
			QString leg2 = m_tableWidget->item(11, 2)->text();
			QString leg3 = m_tableWidget->item(21, 2)->text();
			name1 = titleText + leg1 + extractedText;
			name2 = titleText + leg2 + extractedText;
			name3 = titleText + leg3 + extractedText;
		}
		else {
			name1 = "曲线1";
			name2 = "曲线2";
			name3 = "曲线3";
		}
		m_lineSeries1->setName(name1);
		m_lineSeries2->setName(name2);
		m_lineSeries3->setName(name3);
		m_quadSeries1->setName(name1 + "(二次)");
		m_quadSeries2->setName(name2 + "(二次)");
		m_quadSeries3->setName(name3 + "(二次)");
		m_baseName1 = name1;
		m_baseName2 = name2;
		m_baseName3 = name3;
		m_linLabel1->setText(name1);
		m_linLabel2->setText(name2);
		m_linLabel3->setText(name3);
		m_quadLabel1->setText(name1 + "(二次)");
		m_quadLabel2->setText(name2 + "(二次)");
		m_quadLabel3->setText(name3 + "(二次)");
	}
	else
	{
		m_lineSeries1->setName("壳体厚度:1mm");
		m_lineSeries2->setName("壳体厚度:2mm");
		m_lineSeries3->setName("壳体厚度:3mm");
		m_quadSeries1->setName("壳体厚度:1mm(二次)");
		m_quadSeries2->setName("壳体厚度:2mm(二次)");
		m_quadSeries3->setName("壳体厚度:3mm(二次)");
		m_baseName1 = "壳体厚度:1mm";
		m_baseName2 = "壳体厚度:2mm";
		m_baseName3 = "壳体厚度:3mm";
		m_linLabel1->setText("壳体厚度:1mm");
		m_linLabel2->setText("壳体厚度:2mm");
		m_linLabel3->setText("壳体厚度:3mm");
		m_quadLabel1->setText("壳体厚度:1mm(二次)");
		m_quadLabel2->setText("壳体厚度:2mm(二次)");
		m_quadLabel3->setText("壳体厚度:3mm(二次)");
	}

	updateChartData(data1, data2, data3, x_comboBox->currentText(), y_comboBox->currentText());
}


void IntelligentAnalyWidget::updateChartData(QVector<QPointF> data1, QVector<QPointF> data2, QVector<QPointF> data3, QString xAxisTitle, QString yAxisTitle)
{
	if (!m_chartView || !m_chart || !m_scatter1 || !m_scatter2 || !m_scatter3) return;
	if (!m_quadSeries1 || !m_quadSeries2 || !m_quadSeries3) return;

	// 对每组数据执行：一次拟合 + 二次拟合 + 散点保留
	auto fitGroup = [](const QVector<QPointF>& data, QSplineSeries* lineSeries,
		QSplineSeries* quadSeries, QScatterSeries* scatter,
		QVector<QPointF>& allPoints,
		QLabel* linLabel, QLabel* quadLabel, const QString& baseName)
	{
		// 散点：原始数据点
		/*scatter->clear();
		for (const QPointF& p : data) {
			scatter->append(p);
			allPoints.append(p);
		}*/
		if (data.isEmpty()) {
			lineSeries->clear();
			quadSeries->clear();
			return;
		}
		// 判断y值是否全为0，全0则不拟合不显示
		bool allYZero = true;
		for (const QPointF& p : data) {
			if (qAbs(p.y()) > 1e-12) {
				allYZero = false;
				break;
			}
		}
		
		// 计算x范围
		double xMin = data.first().x(), xMax = data.first().x();
		for (const QPointF& p : data) {
			xMin = qMin(xMin, p.x());
			xMax = qMax(xMax, p.x());
		}
		// 一次拟合 → 生成平滑曲线
		FitResult linFit = APIPolynomialFitter::fitLinear(data);
		QVector<QPointF> linCurve = APIPolynomialFitter::generateLinearCurve(linFit, xMin, xMax, 50);
		lineSeries->clear();
		for (const QPointF& p : linCurve) {
			lineSeries->append(p);
			allPoints.append(p);
		}
		// 二次拟合 → 生成平滑曲线
		FitResult quadFit = APIPolynomialFitter::fitQuadratic(data);
		QVector<QPointF> quadCurve = APIPolynomialFitter::generateQuadraticCurve(quadFit, xMin, xMax, 50);
		quadSeries->clear();
		for (const QPointF& p : quadCurve) {
			quadSeries->append(p);
			allPoints.append(p);
		}

		if (!allYZero) {
			// 更新图例label，附加拟合函数方程
			if (linLabel && !baseName.isEmpty()) {
				QString eq = QString("y=%1x+%2")
					.arg(linFit.a, 0, 'f', 2).arg(linFit.b, 0, 'f', 2);
				linLabel->setText(baseName + "  " + eq);
			}
			if (quadLabel && !baseName.isEmpty()) {
				QString eq = QString("y=%1x²+%2x+%3")
					.arg(quadFit.a, 0, 'f', 2).arg(quadFit.b, 0, 'f', 2).arg(quadFit.c, 0, 'f', 2);
				quadLabel->setText(baseName + "  " + eq);
			}
		}
	};

	QVector<QPointF> allPoints;
	fitGroup(data1, m_lineSeries1, m_quadSeries1, m_scatter1, allPoints, m_linLabel1, m_quadLabel1, m_baseName1);
	fitGroup(data2, m_lineSeries2, m_quadSeries2, m_scatter2, allPoints, m_linLabel2, m_quadLabel2, m_baseName2);
	fitGroup(data3, m_lineSeries3, m_quadSeries3, m_scatter3, allPoints, m_linLabel3, m_quadLabel3, m_baseName3);

	// 自动调整轴范围（包含所有拟合曲线和原始数据点）
	m_chart->createDefaultAxes();
	m_axisX = qobject_cast<QValueAxis*>(m_chart->axisX());
	m_axisY = qobject_cast<QValueAxis*>(m_chart->axisY());
	if (m_axisX) {
		m_axisX->setTitleText(xAxisTitle);
	}
	if (m_axisY) {
		m_axisY->setTitleText(yAxisTitle);
	}
	qreal maxX = calculateMaxValue(allPoints, true);
	qreal maxY = calculateMaxValue(allPoints, false);
	qreal minX = calculateMinValue(allPoints, true);
	qreal minY = calculateMinValue(allPoints, false);
	m_axisX->setRange(minX * 0.99, maxX * 1.01);
	m_axisY->setRange(minY * 0.99, maxY * 1.01);
	m_axisX->setTickCount(8);
	m_axisY->setTickCount(8);
	// 全局过滤：隐藏所有散点图例
	auto legend = m_chart->legend();
	legend->setMaximumWidth(720);
	for (auto marker : legend->markers())
	{
		if (qobject_cast<QScatterSeries*>(marker->series()))
			marker->setVisible(false);
	}
	m_chartView->update();
}

// 创建自定义图例条目（线条指示器 + 名称标签）
QWidget* IntelligentAnalyWidget::createLegendItem(Qt::PenStyle style, const QColor& color, QLabel* label)
{
	QWidget* item = new QWidget();
	QHBoxLayout* lay = new QHBoxLayout(item);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(6);
	LineIndicator* ind = new LineIndicator(style, color);
	lay->addWidget(ind);
	lay->addWidget(label);
	lay->addStretch();
	return item;
}

// 计算数据最大值（确保不小于0）
qreal IntelligentAnalyWidget::calculateMaxValue(const QVector<QPointF>& series, bool isX)
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
qreal IntelligentAnalyWidget::calculateMinValue(const QVector<QPointF>& series, bool isX)
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

//创建一组数据（曲线 + 圆点），统一配置样式
void IntelligentAnalyWidget::createChartDataGroup(QSplineSeries*& lineSeries, QScatterSeries*& scatterSeries, QSplineSeries*& quadSeries,
	const QString& name, const QColor& color)
{
	// 1. 创建曲线系列（只负责线条）
	lineSeries = new QSplineSeries();
	lineSeries->setName(name);  // 系列名称（与散点共享，图例只显示1次）
	QPen linePen(color);
	linePen.setWidth(2);        // 曲线宽度
	lineSeries->setPen(linePen);

	// 2. 创建散点系列（只负责圆点）
	scatterSeries = new QScatterSeries();
	scatterSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);  // 圆点（原生支持）
	scatterSeries->setMarkerSize(8);                                  // 圆点大小（8px）
	scatterSeries->setBrush(QBrush(color));                            // 圆点填充色
	scatterSeries->setPen(QPen(Qt::black, 1));                         // 圆点边框（黑色，1px）

	quadSeries = new QSplineSeries();
	quadSeries->setName(name + "(二次)");
	QPen quadPen(color);
	quadPen.setWidth(2);
	quadPen.setStyle(Qt::DashLine);
	quadPen.setDashPattern(QVector<qreal>{6, 3});
	quadSeries->setPen(quadPen);
}

void IntelligentAnalyWidget::updateGraphicData(QString xName, QString yName, QString zName,
	const QVector<double>& xCoords,
	const QVector<double>& yCoords,
	const QVector<QVector<double>>& newData,
	double xMin,
	double xMax,
	double yMin,
	double yMax)
{
	if (zName == "壳体最大应力")
	{
		m_grapgicComboBox->setCurrentIndex(0);
	}
	else if (zName == "推进剂最大应力")
	{
		m_grapgicComboBox->setCurrentIndex(1);
	}
	else if (zName == "壳体最高温度")
	{
		m_grapgicComboBox->setCurrentIndex(2);
	}
	else if (zName == "推进剂最高温度")
	{
		m_grapgicComboBox->setCurrentIndex(3);
	}
	QMap<QString, QString> unitmMap;
	unitmMap.insert("壳体厚度", "壳体厚度[mm*10]");
	unitmMap.insert("跌落高度", "跌落高度[m]");
	unitmMap.insert("快烤平均温度", "快烤平均温度[℃]");
	unitmMap.insert("慢烤平均温度", "烘箱终止温度[℃]");
	unitmMap.insert("聚能装药口径", "聚能装药口径[mm]");
	unitmMap.insert("枪击速度", "撞击速度[m/s]");
	unitmMap.insert("破片撞击速度", "撞击速度[m/s]");
	unitmMap.insert("TNT当量", "TNT当量[g]");
	unitmMap.insert("殉爆距离", "殉爆距离[mm]");
	unitmMap.insert("壳体最大应力", "壳体最大应力[MPa]");
	unitmMap.insert("推进剂最大应力", "推进剂最大应力[MPa]");
	unitmMap.insert("壳体最高温度", "壳体最高温度[℃]");
	unitmMap.insert("推进剂最高温度", "推进剂最高温度[℃]");
	m_3dGraphicWid->axisTitleChange(unitmMap.value(xName), unitmMap.value(yName), unitmMap.value(zName));
	m_3dGraphicWid->dataUpdate(xCoords, yCoords, newData, newData.size(), newData.at(0).size(), xMin, xMax, yMin, yMax);
}

QVector<QPointF> IntelligentAnalyWidget::bezierSmooth(const QVector<QPointF>& src, int subdiv)
{
	QVector<QPointF> res;
	if (src.size() <= 1)
		return src;

	res << src[0];
	int n = src.size();

	for (int i = 0; i < n - 1; i++)
	{
		QPointF p0 = src[i];
		QPointF p3 = src[i + 1];

		// 计算控制点，实现平滑过渡
		QPointF p1, p2;
		if (i == 0)
		{
			// 第一段
			p1 = p0 + (p3 - p0) * 0.33;
		}
		else
		{
			QPointF prev = src[i - 1];
			p1 = p0 + (p3 - prev) * 0.2;
		}

		if (i == n - 2)
		{
			// 最后一段
			p2 = p3 - (p3 - p0) * 0.33;
		}
		else
		{
			QPointF next = src[i + 2];
			p2 = p3 - (next - p0) * 0.2;
		}

		// 细分采样贝塞尔曲线
		for (int s = 1; s <= subdiv; s++)
		{
			qreal t = qreal(s) / subdiv;
			qreal mt = 1 - t;

			QPointF pt = mt * mt * mt * p0
				+ 3 * mt * mt * t * p1
				+ 3 * mt * t * t * p2
				+ t * t * t * p3;
			res << pt;
		}
	}
	return res;
}