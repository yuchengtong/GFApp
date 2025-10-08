#pragma once
#pragma execution_character_set("utf-8")
#include "FallPropertyWidget.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include "ModelDataManager.h"

FallPropertyWidget::FallPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void FallPropertyWidget::initWidget()
{
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);

	m_tableWidget = new QTableWidget(this);
	// 设置行列数，这里固定 3 行 2 列（对应示例里的 3 条属性），也可动态调整
	m_tableWidget->setRowCount(7);
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

	QStringList labels = { "属性","跌落高度", "跌落姿态", "跌落钢板硬度", "温度传感器数量","冲击波超压传感器数量","风速" };
	for (int row = 0; row < labels.size(); ++row) {
		QTableWidgetItem* serialItem = new QTableWidgetItem(QString::number(row));
		if (row == 0) {
			serialItem = new QTableWidgetItem("属性");
		}
		serialItem->setFlags(serialItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 0, serialItem);

		QTableWidgetItem* labelItem = new QTableWidgetItem(labels[row]);
		labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 1, labelItem);
	}

	// 设置列宽度
	QTableWidgetItem *colimnItem = m_tableWidget->item(5, 1);
	int itemWidth = QFontMetrics(m_tableWidget->font()).width(colimnItem->text());
	m_tableWidget->setColumnWidth(1, itemWidth + m_tableWidget->verticalHeader()->width());

	QStringList unitLabels = { " ","m", "°", " ", "个","个","m/s" };
	for (int row = 0; row < unitLabels.size(); ++row) {
		if (row != 0)
		{
			QTableWidgetItem* labelItem = new QTableWidgetItem(unitLabels[row]);
			labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			m_tableWidget->setItem(row, 3, labelItem);
		}
		
	}

	// 将第0行0列的单元格文本字体加粗
	QTableWidgetItem *headerItem = m_tableWidget->item(0, 0);
	if (headerItem) {
		QFont font = headerItem->font();
		font.setBold(true);
		headerItem->setFont(font);
	}


	QTableWidgetItem* heightValueItem = new QTableWidgetItem("0");
	QTableWidgetItem* hardnessValueItem = new QTableWidgetItem("刚体");
	hardnessValueItem->setFlags(hardnessValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	QTableWidgetItem* temperatureSensorValueItem = new QTableWidgetItem("40");
	temperatureSensorValueItem->setFlags(temperatureSensorValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	QTableWidgetItem* overpressureSensorValueItem = new QTableWidgetItem("40");
	overpressureSensorValueItem->setFlags(overpressureSensorValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	QTableWidgetItem* airVelocityValueItem = new QTableWidgetItem("0");
	airVelocityValueItem->setFlags(airVelocityValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑

	m_tableWidget->setItem(1, 2, heightValueItem);

	QComboBox* postureComboBox = new QComboBox();
	postureComboBox->addItems({ "0", "45", "90" });
	m_tableWidget->setCellWidget(2, 2, postureComboBox);

	m_tableWidget->setItem(3, 2, hardnessValueItem);
	m_tableWidget->setItem(4, 2, temperatureSensorValueItem);
	m_tableWidget->setItem(5, 2, overpressureSensorValueItem);
	m_tableWidget->setItem(6, 2, airVelocityValueItem);

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
		QTableWidgetItem *item = m_tableWidget->item(row, 2);
		if (item && !(item->flags() & Qt::ItemIsEditable))
		{
			item->setBackground(QBrush(QColor(230, 230, 230)));
		}
		if (row != 0)
		{
			QTableWidgetItem *unitItem = m_tableWidget->item(row, 3);
			unitItem->setBackground(QBrush(QColor(230, 230, 230)));
		}
		
	}


	connect(m_tableWidget, &QTableWidget::itemChanged, this, [this](QTableWidgetItem* item) {
		if (item->row() == 1 && item->column() == 2) {
			QString value = item->text();
			double numericValue = value.toDouble();

			FallSettingInfo info;
			info.high = numericValue;
			ModelDataManager::GetInstance()->SetFallSettingInfo(info);
		}
	});

}
