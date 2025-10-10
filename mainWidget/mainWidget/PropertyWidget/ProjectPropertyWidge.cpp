#pragma once
#pragma execution_character_set("utf-8")
#include "ProjectPropertyWidge.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <Windows.h>

QString getComputerName() {
	char computerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = sizeof(computerName);
	if (GetComputerNameA(computerName, &size)) {
		return QString::fromUtf8(computerName);
	}
	return QString();
}

ProjectPropertyWidge::ProjectPropertyWidge(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void ProjectPropertyWidge::initWidget()
{
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);

	m_tableWidget = new QTableWidget(this);
	// 设置行列数，这里固定 4 行 2 列（对应示例里的 4 条属性），也可动态调整
	m_tableWidget->setRowCount(5);
	m_tableWidget->setColumnCount(3);
	// 隐藏表头（如果不需要显示表头文字，可根据需求决定是否隐藏）
	m_tableWidget->horizontalHeader()->setVisible(false);
	m_tableWidget->verticalHeader()->setVisible(false);
	// 让表格充满布局，自动调整行列大小


	// 设置第一列固定宽度（例如100像素）
	m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->resizeSection(0, 5);
	m_tableWidget->horizontalHeader()->resizeSection(1, 100);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	// 让表格充满布局，自动调整行列大小
	m_tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_tableWidget->setColumnWidth(0, 5);
	// 合并第一行的第一和第二列
	m_tableWidget->setSpan(0, 0, 1, 3);

	vlayout->addWidget(m_tableWidget);
	setLayout(vlayout);


	QStringList labels = { "项目","工程名称","工程地点","工程时间","测试设备" };
	for (int row = 0; row < labels.size(); ++row) {
		QTableWidgetItem* serialItem = new QTableWidgetItem(QString::number(row));
		if (row == 0) {
			serialItem = new QTableWidgetItem("项目");
		}
		serialItem->setFlags(serialItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 0, serialItem);

		QTableWidgetItem* labelItem = new QTableWidgetItem(labels[row]);
		labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 1, labelItem);
	}

	QString currentDate = QDate::currentDate().toString("yyyy-MM-dd");
	QStringList values = { "项目","测试项目","乌鲁木齐",currentDate,"测试设备" };
	for (int row = 0; row < values.size(); ++row) {
		QTableWidgetItem* labelItem = new QTableWidgetItem(values[row]);
		m_tableWidget->setItem(row, 2, labelItem);
	}
	QTableWidgetItem* labelItem = new QTableWidgetItem(getComputerName());
	m_tableWidget->setItem(4, 2, labelItem);

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
	}

}
