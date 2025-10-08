#pragma once
#pragma execution_character_set("utf-8")
#include "GeomPropertyWidget.h"
#include <QTableWidget>
#include <QHeaderView>

GeomPropertyWidget::GeomPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void GeomPropertyWidget::UpdataPropertyInfo()
{
	auto modelInfo=ModelDataManager::GetInstance()->GetModelGeometryInfo();
	QTableWidgetItem* path_item = m_tableWidget->item(2, 2);
	QTableWidgetItem* length_item = m_tableWidget->item(3, 2);
	QTableWidgetItem* width_item = m_tableWidget->item(4, 2);
	QTableWidgetItem* height_item = m_tableWidget->item(5, 2);
	if (path_item&&length_item&&width_item&&height_item)
	{
		path_item->setText(modelInfo.path);
		length_item->setText(QString::number(modelInfo.length, 'f', 3));
		width_item->setText(QString::number(modelInfo.width ,'f', 3));
		height_item->setText(QString::number(modelInfo.height ,'f', 3));
	}
}

void GeomPropertyWidget::initWidget()
{
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);

	m_tableWidget = new QTableWidget(this);
	// 设置行列数，这里固定 5 行 4 列
	m_tableWidget->setRowCount(6);
	m_tableWidget->setColumnCount(4);
	// 隐藏表头（如果不需要显示表头文字，可根据需求决定是否隐藏）
	m_tableWidget->horizontalHeader()->setVisible(false);
	m_tableWidget->verticalHeader()->setVisible(false);
	// 让表格充满布局，自动调整行列大小


	// 设置第一列固定宽度（例如100像素）
	m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->resizeSection(0, 5);
	m_tableWidget->horizontalHeader()->resizeSection(2, 60);
	m_tableWidget->horizontalHeader()->resizeSection(4, 5);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
	// 让表格充满布局，自动调整行列大小
	m_tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_tableWidget->setColumnWidth(0, 5);
	// 合并第一行的第一和第二列
	m_tableWidget->setSpan(0, 0, 1, 4);

	vlayout->addWidget(m_tableWidget);
	setLayout(vlayout);

	QStringList labels = { "几何模型","发动机型号","来源","长","宽", "高度" };
	for (int row = 0; row < labels.size(); ++row) {
		QTableWidgetItem* serialItem = new QTableWidgetItem(QString::number(row));
		if (row == 0) {
			serialItem = new QTableWidgetItem("几何模型");
		}
		serialItem->setFlags(serialItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 0, serialItem);

		QTableWidgetItem* labelItem = new QTableWidgetItem(labels[row]);
		labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 1, labelItem);
	}

	//第2列用空Label
	QStringList emptyLabels = { " "," ","", "", "", "" };
	for (int row = 0; row < emptyLabels.size(); ++row) {
		if (row == 1)
		{
			QTableWidgetItem* labelItem = new QTableWidgetItem(emptyLabels[row]);
			m_tableWidget->setItem(row, 2, labelItem);
		}
		else if (row != 0)
		{
			QTableWidgetItem* labelItem = new QTableWidgetItem(emptyLabels[row]);
			labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			m_tableWidget->setItem(row, 2, labelItem);
		}
	}


	QStringList unitLabels = { " "," ","", "mm", "mm", "mm" };
	for (int row = 0; row < unitLabels.size(); ++row) {
		if (row != 0)
		{
			QTableWidgetItem* labelItem = new QTableWidgetItem(unitLabels[row]);
			labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			m_tableWidget->setItem(row, 3, labelItem);
		}
	}

	// 设置列宽度
	QTableWidgetItem *colimnItem = m_tableWidget->item(1, 1);
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

	//// 单元格背景设置为浅灰色
	//for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
	//	m_tableWidget->setRowHeight(row, 10);
	//	QTableWidgetItem *item = m_tableWidget->item(row, 2);
	//	if (item && !(item->flags() & Qt::ItemIsEditable))
	//	{
	//		item->setBackground(QBrush(QColor(230, 230, 230)));
	//	}
	//	if (row != 0)
	//	{
	//		QTableWidgetItem *unitItem = m_tableWidget->item(row, 3);
	//		unitItem->setBackground(QBrush(QColor(230, 230, 230)));
	//	}
	//}

}
