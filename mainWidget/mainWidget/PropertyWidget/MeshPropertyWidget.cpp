#pragma once
#pragma execution_character_set("utf-8")
#include "MeshPropertyWidget.h"
#include <QTableWidget>
#include <QHeaderView>
#include "ModelDataManager.h"

MeshPropertyWidget::MeshPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void MeshPropertyWidget::UpdataPropertyInfo()
{
	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	if (meshInfo.isChecked)
	{
		auto nodeCount = meshInfo.triangleStructure.GetAllNodes().Extent();
		auto elemCount = meshInfo.triangleStructure.GetAllElements().Extent();

		QTableWidgetItem* node_item = m_tableWidget->item(1, 2);
		QTableWidgetItem* elem_item = m_tableWidget->item(2, 2);

		if (node_item && elem_item)
		{
			node_item->setText(QString::number(nodeCount));
			elem_item->setText(QString::number(elemCount));
		}
	}

}

void MeshPropertyWidget::initWidget()
{
	// 创建垂直布局
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);

	// 初始化表格控件（3行3列）
	m_tableWidget = new QTableWidget(this);
	m_tableWidget->setRowCount(3);
	m_tableWidget->setColumnCount(3);
	// 隐藏表头
	m_tableWidget->horizontalHeader()->setVisible(false);
	m_tableWidget->verticalHeader()->setVisible(false);

	// 设置列宽模式
	m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->resizeSection(0, 5);       // 第一列固定窄列
	m_tableWidget->horizontalHeader()->resizeSection(1, 80);      // 第二列标签列
	m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch); // 第三列内容列
	// 设置行高固定
	m_tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_tableWidget->setColumnWidth(0, 5);
	// 合并第一行的所有列作为标题行
	m_tableWidget->setSpan(0, 0, 1, 3);

	// 添加表格到布局
	vlayout->addWidget(m_tableWidget);
	setLayout(vlayout);

	// 网格信息标签列表
	QStringList labels = { "网格信息","网格总数","节点总数" };
	for (int row = 0; row < labels.size(); ++row) {
		// 第一列（序号列）
		QTableWidgetItem* serialItem = new QTableWidgetItem(QString::number(row));
		if (row == 0) {
			// 第一行显示主标题
			serialItem = new QTableWidgetItem("网格信息");
		}
		serialItem->setFlags(serialItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 0, serialItem);

		// 第二列（标签列）
		QTableWidgetItem* labelItem = new QTableWidgetItem(labels[row]);
		labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 1, labelItem);
	}

	// 设置列宽度
	QTableWidgetItem* colimnItem = m_tableWidget->item(1, 1);
	int itemWidth = QFontMetrics(m_tableWidget->font()).width(colimnItem->text());
	m_tableWidget->setColumnWidth(1, itemWidth + m_tableWidget->verticalHeader()->width());

	// 第三列（内容列，初始为空）
	QStringList emptyLabels = { " ","", "" };
	for (int row = 0; row < emptyLabels.size(); ++row) {
		if (row != 0) // 跳过标题行
		{
			QTableWidgetItem* labelItem = new QTableWidgetItem(emptyLabels[row]);
			labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable);
			m_tableWidget->setItem(row, 2, labelItem);
		}
	}

	// 设置标题行文字加粗
	QTableWidgetItem* headerItem = m_tableWidget->item(0, 0);
	if (headerItem) {
		QFont font = headerItem->font();
		font.setBold(true);
		headerItem->setFont(font);
	}

	// 设置所有单元格文本左对齐
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
