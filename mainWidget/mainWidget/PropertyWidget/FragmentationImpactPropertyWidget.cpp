#pragma once
#pragma execution_character_set("utf-8")

#include "FragmentationImpactPropertyWidget.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>

FragmentationImpactPropertyWidget::FragmentationImpactPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void FragmentationImpactPropertyWidget::initWidget()
{

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);

	m_tableWidget = new QTableWidget(this);

	m_tableWidget->setRowCount(11);
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

	QStringList labels = { "属性","测试项目","撞击速度", "撞击角度", "破片形状", "破片直径", "破片质量","破片硬度","温度传感器数量","超压传感器数量","风速" };
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
	QTableWidgetItem* colimnItem = m_tableWidget->item(9, 1);
	int itemWidth = QFontMetrics(m_tableWidget->font()).width(colimnItem->text());
	m_tableWidget->setColumnWidth(1, itemWidth + m_tableWidget->verticalHeader()->width());

	// 单位
	QStringList unitLabels = { " ", " ", "m/s", "°"," ", "mm","g","个"," ", "个", "m/s" };
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



	QTableWidgetItem* titeItem = new QTableWidgetItem("破片撞击试验");
	titeItem->setFlags(titeItem->flags() & ~Qt::ItemIsEditable); // 不可编辑

	QTableWidgetItem* impactVelocityValueItem = new QTableWidgetItem("1830");
	impactVelocityValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中

	QTableWidgetItem* impactAngleValueItem = new QTableWidgetItem("90");
	impactAngleValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	impactAngleValueItem->setFlags(impactAngleValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑

	QTableWidgetItem* fragmentaryShapeValueItem = new QTableWidgetItem("圆柱形");
	fragmentaryShapeValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	fragmentaryShapeValueItem->setFlags(fragmentaryShapeValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑

	QTableWidgetItem* fragmentaryDiameterValueItem = new QTableWidgetItem("14.3");
	fragmentaryDiameterValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	fragmentaryDiameterValueItem->setFlags(fragmentaryDiameterValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑

	QTableWidgetItem* fragmentaryQualityValueItem = new QTableWidgetItem("22");
	fragmentaryQualityValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	fragmentaryQualityValueItem->setFlags(fragmentaryQualityValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑

	QTableWidgetItem* fragmentaryHardnessValueItem = new QTableWidgetItem("刚体");
	fragmentaryHardnessValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	fragmentaryHardnessValueItem->setFlags(fragmentaryHardnessValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑

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
	m_tableWidget->setItem(2, 2, impactVelocityValueItem);
	m_tableWidget->setItem(3, 2, impactAngleValueItem);
	m_tableWidget->setItem(4, 2, fragmentaryShapeValueItem);
	m_tableWidget->setItem(5, 2, fragmentaryDiameterValueItem);
	m_tableWidget->setItem(6, 2, fragmentaryQualityValueItem);
	m_tableWidget->setItem(7, 2, fragmentaryHardnessValueItem);
	m_tableWidget->setItem(8, 2, temperatureNumValueItem);
	m_tableWidget->setItem(9, 2, overpressureNumValueItem);
	m_tableWidget->setItem(10, 2, windSpeedValueItem);

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
