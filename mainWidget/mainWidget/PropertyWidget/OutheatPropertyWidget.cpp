#pragma once
#pragma execution_character_set("utf-8")
#include "OutheatPropertyWidget.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QDir>
#include <QPushButton>
#include <QDialog>
#include <QDateTime>
#include <QApplication>
#include "xlsxdocument.h"

#include "ModelDataManager.h"
#include "GFTreeModelWidget.h"
#include "GFImportModelWidget.h"

OutheatPropertyWidget::OutheatPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void OutheatPropertyWidget::initWidget()
{
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);

	m_tableWidget = new QTableWidget(this);

	m_tableWidget->setRowCount(10);
	m_tableWidget->setColumnCount(4);
	// 隐藏表头（如果不需要显示表头文字，可根据需求决定是否隐藏）
	m_tableWidget->horizontalHeader()->setVisible(false);
	m_tableWidget->verticalHeader()->setVisible(false);

	// 设置第一列固定宽度（例如100像素）
	m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->resizeSection(0, 5);
	m_tableWidget->horizontalHeader()->resizeSection(1, 120);
	m_tableWidget->horizontalHeader()->resizeSection(3, 80);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
	// 让表格充满布局，自动调整行列大小
	m_tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_tableWidget->setColumnWidth(0, 5);
	// 合并第一行的第一和第二列
	m_tableWidget->setSpan(0, 0, 1, 2);

	vlayout->addWidget(m_tableWidget);
	setLayout(vlayout);

	QStringList labels = { "材料属性","材料牌号", "密度", "热膨胀系数", "杨氏模量","泊松比","屈服强度","抗拉强度","热导率","比热容" };
	for (int row = 0; row < labels.size(); ++row) {
		QTableWidgetItem* serialItem = new QTableWidgetItem(QString::number(row));
		if (row == 0) {
			serialItem = new QTableWidgetItem("材料属性");
		}
		serialItem->setFlags(serialItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 0, serialItem);

		QTableWidgetItem* labelItem = new QTableWidgetItem(labels[row]);
		labelItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 1, labelItem);

		if (row != 0)
		{
			QTableWidgetItem* valueItem = new QTableWidgetItem("");
			valueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
			valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			m_tableWidget->setItem(row, 2, valueItem);
		}

	}

	// 设置列宽度
	QTableWidgetItem *colimnItem = m_tableWidget->item(3, 1);
	int itemWidth = QFontMetrics(m_tableWidget->font()).width(colimnItem->text());
	m_tableWidget->setColumnWidth(1, itemWidth + m_tableWidget->verticalHeader()->width());

	// 单位列
	QStringList unitLabels = { " "," ", "kg/m^3", "/K", "Pa"," ","MPa","MPa","W/m K","J/kg K" };
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

	// 导入按钮
	QWidget *importWidget = new QWidget();
	QPushButton *importButton = new QPushButton("导入");
	importButton->setMinimumHeight(30);
	importButton->setStyleSheet("QPushButton {"
		"background-color:  rgba(0, 0, 0, 0);"
		"}"
		"QPushButton:hover {"
		"background-color: white;"
		"}");
	QVBoxLayout *importLayout = new QVBoxLayout(importWidget);
	importLayout->addWidget(importButton);
	importLayout->setAlignment(Qt::AlignCenter); // 按钮居中显示
	importLayout->setMargin(0);
	importWidget->setLayout(importLayout);
	m_tableWidget->setCellWidget(0, 2, importWidget);

	connect(importButton, &QPushButton::clicked, this, &OutheatPropertyWidget::showTableDialog);
	// 合并第一行的第三和第四列
	m_tableWidget->setSpan(0, 2, 1, 2);

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

}

void OutheatPropertyWidget::showTableDialog() 
{
	QDialog *dialog = new QDialog();
	dialog->setWindowTitle("外防热材料");
	dialog->resize(1000, 500);
	QVBoxLayout *layout = new QVBoxLayout(this);

	QTableWidget *diaTableWidget = new QTableWidget();
	// 隐藏行号
	diaTableWidget->verticalHeader()->setVisible(false);
	// 隐藏列号
	diaTableWidget->horizontalHeader()->setVisible(false);
	QDir dir;
	QString filepath = dir.absoluteFilePath("src/database/外防热材料.xlsx");
	int m_rowCount = 0;

	if (!filepath.isEmpty()) {
		QXlsx::Document xlsx(filepath);
		int rowcount = xlsx.dimension().lastRow(); // 获取总行数
		int colcount = xlsx.dimension().lastColumn(); // 获取总列数
		m_rowCount = rowcount;

		diaTableWidget->setRowCount(rowcount);
		diaTableWidget->setColumnCount(colcount);

		for (int row = 1; row <= rowcount; ++row) {
			for (int col = 1; col <= colcount; ++col) {
				QTableWidgetItem *item = new QTableWidgetItem(xlsx.read(row, col).toString());
				item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
				diaTableWidget->setItem(row - 1, col - 1, item);
			}
		}
	}

	// 私有库
	auto ins = ModelDataManager::GetInstance();
	UserInfo info = ins->GetUserInfo();
	QString privateFilePath = dir.absoluteFilePath("src/database/" + info.username + "/外防热材料.xlsx");


	QFile file(privateFilePath);

	if (!privateFilePath.isEmpty() && file.exists()) {
		QXlsx::Document xlsx(privateFilePath);
		int rowcount = xlsx.dimension().lastRow(); // 获取总行数
		int colcount = xlsx.dimension().lastColumn(); // 获取总列数

		diaTableWidget->setRowCount(m_rowCount + rowcount - 1);

		int xlsxrow = m_rowCount;
		for (int row = 2; row <= rowcount; ++row) {
			for (int col = 1; col <= colcount; ++col) {
				QTableWidgetItem *item = new QTableWidgetItem(xlsx.read(row, col).toString());
				diaTableWidget->setItem(xlsxrow, col - 1, item);
			}
			xlsxrow++;
		}
	}

	//设置点击事件，双击单元格
	connect(diaTableWidget, &QTableWidget::cellDoubleClicked, this, [this, dialog, diaTableWidget](int row, int column) {
		if (row != 0)
		{
			int colcount = diaTableWidget->columnCount();
			QString value = "";
			for (int col = 1; col < colcount; ++col) {

				QString content = diaTableWidget->item(row, col)->text();
				if (col == 1)
				{
					value = content;
				}
				QTableWidgetItem* valueItem = new QTableWidgetItem(content);
				valueItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
				valueItem->setBackground(QBrush(QColor(230, 230, 230)));
				m_tableWidget->setItem(col, 2, valueItem);
			}
			auto ins = ModelDataManager::GetInstance();

			OutheatPropertyInfo info;
			info.materialGrade = m_tableWidget->item(1, 2)->text();
			info.density = m_tableWidget->item(2, 2)->text().toDouble();
			info.thermalExpansion = m_tableWidget->item(3, 2)->text().toDouble();
			info.modulus = m_tableWidget->item(4, 2)->text().toDouble();
			info.poisonby = m_tableWidget->item(5, 2)->text().toDouble();
			info.yieldStrength = m_tableWidget->item(6, 2)->text().toDouble();
			info.tensileStrength = m_tableWidget->item(7, 2)->text().toDouble();
			info.thermalConductivity = m_tableWidget->item(8, 2)->text().toDouble();
			info.specificHeatCapacity = m_tableWidget->item(9, 2)->text().toDouble();
			info.isChecked = true;
			ins->SetOutheatPropertyInfo(info);

			// 更新icon
			QWidget* parent = parentWidget();
			while (parent) {
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{
					gfParent->GetGFTreeModelWidget()->updataIcon();
					// 写入日志
					QDateTime currentTime = QDateTime::currentDateTime();
					QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
					auto logWidget = gfParent->GetLogWidget();
					auto textEdit = logWidget->GetTextEdit();
					QString text = timeStr + "[信息]>开始导入外防热材料数据";
					textEdit->appendPlainText(text);
					logWidget->update();

					// 关键：强制刷新UI，确保日志立即显示
					QApplication::processEvents();
					// 写入数据库模块
					MaterialPropertyWidget* m_materialPropertyWidget = gfParent->GetMaterialPropertyWidget();
					QTableWidget* materialTableWid = m_materialPropertyWidget->GetQTableWidget();
					QTableWidgetItem* valueItem = new QTableWidgetItem(value);
					valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
					valueItem->setBackground(QBrush(QColor(230, 230, 230)));
					materialTableWid->setItem(3, 2, valueItem);
					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
		}
		dialog->close();

	});
	//双击单元格选中一行
	 //设置选中整行
	diaTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	diaTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	layout->addWidget(diaTableWidget);
	dialog->setLayout(layout);
	dialog->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动删除
	dialog->exec();
}
