#pragma once
#pragma execution_character_set("utf-8")
#include "StrainResultWidget.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>

StrainResultWidget::StrainResultWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void StrainResultWidget::initWidget()
{

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);

	m_tableWidget = new QTableWidget(this);

	m_tableWidget->setRowCount(17);
	m_tableWidget->setColumnCount(4);
	// 隐藏表头（如果不需要显示表头文字，可根据需求决定是否隐藏）
	m_tableWidget->horizontalHeader()->setVisible(false);
	m_tableWidget->verticalHeader()->setVisible(false);

	// 设置第一列固定宽度（例如100像素）
	m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->resizeSection(0, 5);
	m_tableWidget->horizontalHeader()->resizeSection(1, 200);
	m_tableWidget->horizontalHeader()->resizeSection(3, 5);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
	// 让表格充满布局，自动调整行列大小
	m_tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_tableWidget->setColumnWidth(0, 5);
	// 合并第一行的第一和第二列
	m_tableWidget->setSpan(0, 0, 1, 4);

	vlayout->addWidget(m_tableWidget);
	setLayout(vlayout);

	QStringList labels = { "计算结果", "发动机壳体最大应变", "发动机壳体最小应变", "发动机壳体平均应变", "发动机壳体应变标准差","固体推进剂最大应变","固体推进剂最小应变","固体推进剂平均应变","固体推进剂应变标准差" ,
									  "隔绝热最大应变", "隔绝热最小应变", "隔绝热平均应变", "隔绝热应变标准差","外防热最大应变","外防热最小应变","外防热平均应变","外防热应变标准差" };
	for (int row = 0; row < labels.size(); ++row) {

		QTableWidgetItem* serialItem = new QTableWidgetItem(QString::number(row));
		if (row == 0) {
			serialItem = new QTableWidgetItem("计算结果");
		}
		serialItem->setFlags(serialItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 0, serialItem);

		QTableWidgetItem* labelItem = new QTableWidgetItem(labels[row]);
		labelItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 1, labelItem);

		QTableWidgetItem* valueItem = new QTableWidgetItem("");
		valueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 2, valueItem);

		QTableWidgetItem* unitItem = new QTableWidgetItem("mm/mm");
		unitItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		unitItem->setFlags(unitItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 3, unitItem);
	}

	// 设置列宽度
	QTableWidgetItem* colimnItem = m_tableWidget->item(4, 1);
	int itemWidth = QFontMetrics(m_tableWidget->font()).width(colimnItem->text());
	m_tableWidget->setColumnWidth(1, itemWidth + m_tableWidget->verticalHeader()->width());


	// 将第0行0列的单元格文本字体加粗
	QTableWidgetItem* headerItem = m_tableWidget->item(0, 0);
	if (headerItem) {
		QFont font = headerItem->font();
		font.setBold(true);
		headerItem->setFont(font);
	}

	//文本左对齐
	for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
		for (int col = 0; col < m_tableWidget->columnCount(); ++col) {
			QTableWidgetItem* item = m_tableWidget->item(row, col);
			if (item)
			{
				if (col == 0 && row != 0)
				{
					item->setTextAlignment(Qt::AlignCenter);
				}
				else
				{
					item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				}
			}
		}
	}

	// 遍历第2列（索引为1），将不可编辑单元格背景设置为浅灰色
	for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
		// 遍历行，设置行高
		m_tableWidget->setRowHeight(row, 10);
		QTableWidgetItem* item = m_tableWidget->item(row, 2);
		if (item && !(item->flags() & Qt::ItemIsEditable))
		{
			item->setBackground(QBrush(QColor(230, 230, 230)));
		}
		if (row != 0)
		{
			QTableWidgetItem* unitItem = m_tableWidget->item(row, 3);
			unitItem->setBackground(QBrush(QColor(230, 230, 230)));
		}
	}


}

void StrainResultWidget::updateData(double shellMaxValue, double shellMinValue, double shellAvgValue, double shellStandardValue, double maxValue, double minValue, double avgValue, double standardValue)
{
	QTableWidgetItem* shellMaxValueItem = new QTableWidgetItem(QString::number(shellMaxValue));
	shellMaxValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	shellMaxValueItem->setBackground(QBrush(QColor(230, 230, 230)));
	shellMaxValueItem->setFlags(shellMaxValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	m_tableWidget->setItem(1, 2, shellMaxValueItem);

	QTableWidgetItem* shellMinValueItem = new QTableWidgetItem(QString::number(shellMinValue));
	shellMinValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	shellMinValueItem->setBackground(QBrush(QColor(230, 230, 230)));
	shellMinValueItem->setFlags(shellMinValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	m_tableWidget->setItem(2, 2, shellMinValueItem);

	QTableWidgetItem* shellAvgValueItem = new QTableWidgetItem(QString::number(shellAvgValue));
	shellAvgValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	shellAvgValueItem->setBackground(QBrush(QColor(230, 230, 230)));
	shellAvgValueItem->setFlags(shellAvgValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	m_tableWidget->setItem(3, 2, shellAvgValueItem);

	QTableWidgetItem* shellStandardValueItem = new QTableWidgetItem(QString::number(shellStandardValue));
	shellStandardValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	shellStandardValueItem->setBackground(QBrush(QColor(230, 230, 230)));
	shellStandardValueItem->setFlags(shellStandardValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	m_tableWidget->setItem(4, 2, shellStandardValueItem);

	QTableWidgetItem* maxValueItem = new QTableWidgetItem(QString::number(maxValue));
	maxValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	maxValueItem->setBackground(QBrush(QColor(230, 230, 230)));
	maxValueItem->setFlags(maxValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	m_tableWidget->setItem(5, 2, maxValueItem);

	QTableWidgetItem* minValueItem = new QTableWidgetItem(QString::number(minValue));
	minValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	minValueItem->setBackground(QBrush(QColor(230, 230, 230)));
	minValueItem->setFlags(minValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	m_tableWidget->setItem(6, 2, minValueItem);

	QTableWidgetItem* avgValueItem = new QTableWidgetItem(QString::number(avgValue));
	avgValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	avgValueItem->setBackground(QBrush(QColor(230, 230, 230)));
	avgValueItem->setFlags(avgValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	m_tableWidget->setItem(7, 2, avgValueItem);

	QTableWidgetItem* standardValueItem = new QTableWidgetItem(QString::number(standardValue));
	standardValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	standardValueItem->setBackground(QBrush(QColor(230, 230, 230)));
	standardValueItem->setFlags(standardValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	m_tableWidget->setItem(8, 2, standardValueItem);

}
