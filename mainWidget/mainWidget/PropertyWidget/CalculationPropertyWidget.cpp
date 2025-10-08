#pragma once
#pragma execution_character_set("utf-8")
#include "CalculationPropertyWidget.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
//#include <xlsxdocument.h>
#include <QDir>
#include <QPushButton>
#include <QDialog>
#include "ModelDataManager.h"

CalculationPropertyWidget::CalculationPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void CalculationPropertyWidget::initWidget()
{
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);

	m_tableWidget = new QTableWidget(this);

	m_tableWidget->setRowCount(9);
	m_tableWidget->setColumnCount(3);
	// 隐藏表头（如果不需要显示表头文字，可根据需求决定是否隐藏）
	m_tableWidget->horizontalHeader()->setVisible(false);
	m_tableWidget->verticalHeader()->setVisible(false);

	// 设置第一列固定宽度（例如100像素）
	m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->resizeSection(0, 5);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	// 让表格充满布局，自动调整行列大小
	m_tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_tableWidget->setColumnWidth(0, 5);

	// 合并第一行的第一和第二列
	m_tableWidget->setSpan(0, 0, 1, 2);

	vlayout->addWidget(m_tableWidget);
	setLayout(vlayout);

	QStringList labels = { "计算模型数据库", "跌落试验", "快速烤燃试验", "慢速烤燃试验", "枪击试验","射流冲击试验","破片撞击试验","爆炸冲击波试验","殉爆试验" };
	for (int row = 0; row < labels.size(); ++row) {

		QTableWidgetItem* serialItem = new QTableWidgetItem(QString::number(row));
		if (row == 0) {
			serialItem = new QTableWidgetItem("计算模型");
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

	}

	// 设置列宽度
	QTableWidgetItem *colimnItem = m_tableWidget->item(4, 1);
	int itemWidth = QFontMetrics(m_tableWidget->font()).width(colimnItem->text());
	m_tableWidget->setColumnWidth(1, itemWidth + m_tableWidget->verticalHeader()->width());


	// 将第0行0列的单元格文本字体加粗
	QTableWidgetItem *headerItem = m_tableWidget->item(0, 0);
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
				item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			}
		}
	}

	// 遍历第2列（索引为1），将不可编辑单元格背景设置为浅灰色
	for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
		// 遍历行，设置行高
		m_tableWidget->setRowHeight(row, 10);
		if (row != 0)
		{
			QTableWidgetItem *unitItem = m_tableWidget->item(row, 2);
			unitItem->setBackground(QBrush(QColor(230, 230, 230)));
		}
	}
}