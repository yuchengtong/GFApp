#pragma once
#pragma execution_character_set("utf-8")

#include "DatabasePropertyWidget.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QDir>

DatabasePropertyWidget::DatabasePropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void DatabasePropertyWidget::initWidget()
{
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);

	m_tableWidget = new QTableWidget(this);
	// 设置行列数，这里固定 3 行 2 列（对应示例里的 3 条属性），也可动态调整
	m_tableWidget->setRowCount(3);
	m_tableWidget->setColumnCount(3);
	// 隐藏表头（如果不需要显示表头文字，可根据需求决定是否隐藏）
	m_tableWidget->horizontalHeader()->setVisible(false);
	m_tableWidget->verticalHeader()->setVisible(false);


	// 设置第一列固定宽度（例如100像素）
	m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->resizeSection(0, 5);
	m_tableWidget->horizontalHeader()->resizeSection(1, 150);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	// 让表格充满布局，自动调整行列大小
	m_tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_tableWidget->setColumnWidth(0, 5);
	// 合并第一行的第一和第二列
	m_tableWidget->setSpan(0, 0, 1, 3);

	vlayout->addWidget(m_tableWidget);
	setLayout(vlayout);


	QStringList labels = { "数据库", "评判标准数据库","计算模型数据库" };
	for (int row = 0; row < labels.size(); ++row) {
		QTableWidgetItem* serialItem = new QTableWidgetItem(QString::number(row));
		if (row == 0) {
			serialItem = new QTableWidgetItem("数据库");
		}
		serialItem->setFlags(serialItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 0, serialItem);

		QTableWidgetItem* labelItem = new QTableWidgetItem(labels[row]);
		labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 1, labelItem);

		QTableWidgetItem* valueItem = new QTableWidgetItem("");
		valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		valueItem->setBackground(QBrush(QColor(230, 230, 230)));
		m_tableWidget->setItem(row, 2, valueItem);
	}

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

}
