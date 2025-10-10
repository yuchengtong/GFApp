#pragma once
#pragma execution_character_set("utf-8")

#include "JetImpactPropertyWidget.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>

JetImpactPropertyWidget::JetImpactPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void JetImpactPropertyWidget::initWidget()
{

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);

	m_tableWidget = new QTableWidget(this);

	m_tableWidget->setRowCount(8);
	m_tableWidget->setColumnCount(4);
	// 隐藏表头（如果不需要显示表头文字，可根据需求决定是否隐藏）
	m_tableWidget->horizontalHeader()->setVisible(false);
	m_tableWidget->verticalHeader()->setVisible(false);


	// 设置第一列固定宽度（例如100像素）
	m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->resizeSection(0, 5);
	m_tableWidget->horizontalHeader()->resizeSection(1, 260);
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

	QStringList labels = { "属性","测试项目","聚能装药口径", "炸高", "冲击点角度", "温度传感器数量","超压传感器数量","风速" };
	for (int row = 0; row < labels.size(); ++row) {
		QTableWidgetItem* serialItem = new QTableWidgetItem(QString::number(row));
		if (row == 0) {
			serialItem = new QTableWidgetItem("属性");
		}
		serialItem->setFlags(serialItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 0, serialItem);

		QTableWidgetItem* labelItem = new QTableWidgetItem(labels[row]);
		labelItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 1, labelItem);
	}
	// 设置列宽度
	QTableWidgetItem* colimnItem = m_tableWidget->item(5, 1);
	int itemWidth = QFontMetrics(m_tableWidget->font()).width(colimnItem->text());
	m_tableWidget->setColumnWidth(1, itemWidth + m_tableWidget->verticalHeader()->width());

	// 单位
	QStringList unitLabels = { " ", " ", "mm", "mm", "°", "个", "个", "m/s" };
	for (int row = 0; row < unitLabels.size(); ++row) {
		if (row != 0)
		{
			QTableWidgetItem* labelItem = new QTableWidgetItem(unitLabels[row]);
			labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			m_tableWidget->setItem(row, 3, labelItem);
		}

	}

	// 将第0行0列的单元格文本字体加粗
	QTableWidgetItem* headerItem = m_tableWidget->item(0, 0);
	if (headerItem) {
		QFont font = headerItem->font();
		font.setBold(true);
		headerItem->setFont(font);
	}



	QTableWidgetItem* titeItem = new QTableWidgetItem("射流冲击试验");
	titeItem->setFlags(titeItem->flags() & ~Qt::ItemIsEditable); // 不可编辑

	QTableWidgetItem* caliberValueItem = new QTableWidgetItem("50");
	caliberValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中

	QTableWidgetItem* blowHighValueItem = new QTableWidgetItem("105");
	blowHighValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	blowHighValueItem->setFlags(blowHighValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑

	QTableWidgetItem* impactPointAngleValueItem = new QTableWidgetItem("90");
	impactPointAngleValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	impactPointAngleValueItem->setFlags(impactPointAngleValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑

	QTableWidgetItem* temperatureNumValueItem = new QTableWidgetItem("40");
	temperatureNumValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	temperatureNumValueItem->setFlags(temperatureNumValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑

	QTableWidgetItem* overpressureNumValueItem = new QTableWidgetItem("40");
	overpressureNumValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	overpressureNumValueItem->setFlags(overpressureNumValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑

	QTableWidgetItem* windSpeedValueItem = new QTableWidgetItem("0");
	windSpeedValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	windSpeedValueItem->setFlags(windSpeedValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑



	m_tableWidget->setItem(1, 2, titeItem);
	m_tableWidget->setItem(2, 2, caliberValueItem);
	m_tableWidget->setItem(3, 2, blowHighValueItem);
	m_tableWidget->setItem(4, 2, impactPointAngleValueItem);
	m_tableWidget->setItem(5, 2, temperatureNumValueItem);
	m_tableWidget->setItem(6, 2, overpressureNumValueItem);
	m_tableWidget->setItem(7, 2, windSpeedValueItem);

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
