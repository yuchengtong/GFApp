#pragma once
#pragma execution_character_set("utf-8")
#include "JudgmentPropertyWidget.h"
#include "xlsxdocument.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QDir>
#include <QPushButton>
#include <QDialog>
#include "ModelDataManager.h"
#include "../GFTreeModelWidget.h"
#include "../GFImportModelWidget.h"

#include <QDateTime>
#include <QApplication>

JudgmentPropertyWidget::JudgmentPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void JudgmentPropertyWidget::initWidget()
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

	QStringList labels = { "评判标准数据库","标准号","标准名称", "跌落试验", "快速烤燃试验", "慢速烤燃试验","枪击试验","射流冲击试验","破片撞击试验","爆炸冲击波试验","殉爆试验" };
	for (int row = 0; row < labels.size(); ++row) {
		QTableWidgetItem* serialItem = new QTableWidgetItem(QString::number(row));
		if (row == 0) {
			serialItem = new QTableWidgetItem("评判标准数据库");
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
	QTableWidgetItem* colimnItem = m_tableWidget->item(8, 1);
	int itemWidth = QFontMetrics(m_tableWidget->font()).width(colimnItem->text());
	m_tableWidget->setColumnWidth(1, itemWidth + m_tableWidget->verticalHeader()->width());

	QStringList unitLabels = { " ", " "," ","m", "℃", "℃", "m/s","mm","m/s","kg","mm" };
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

	// 导入按钮
	QWidget* importWidget = new QWidget();
	QPushButton* importButton = new QPushButton("导入");
	importButton->setFixedSize(100, 50);
	importButton->setMinimumHeight(30);
	importButton->setStyleSheet("QPushButton {"
		"background-color:  rgba(0, 0, 0, 0);"
		"border: 2px solid #C1B1B1; "
		"border-radius: 10px; "
		"color: black; "
		"font-weight: bold; "
		"padding: 5px;"
		"outline: none;"
		"}"
		"QPushButton:hover {"
		"background-color: rgba(230, 230, 230, 100);"
		"}");
	QVBoxLayout* importLayout = new QVBoxLayout(importWidget);
	importLayout->addWidget(importButton);
	importLayout->setAlignment(Qt::AlignCenter); // 按钮居中显示
	importLayout->setMargin(0);
	importWidget->setLayout(importLayout);
	m_tableWidget->setCellWidget(0, 2, importWidget);

	connect(importButton, &QPushButton::clicked, this, &JudgmentPropertyWidget::showTableDialog);
	// 合并第一行的第三和第四列
	m_tableWidget->setSpan(0, 2, 1, 2);

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
		QTableWidgetItem* item = m_tableWidget->item(row, 3);
		if (item && !(item->flags() & Qt::ItemIsEditable))
		{
			item->setBackground(QBrush(QColor(230, 230, 230)));
		}
		if (row != 0)
		{
			QTableWidgetItem* unitItem = m_tableWidget->item(row, 2);
			unitItem->setBackground(QBrush(QColor(230, 230, 230)));
		}
	}

}

void JudgmentPropertyWidget::showTableDialog() {


	QDialog* dialog = new QDialog();
	dialog->setWindowTitle("评判标准");
	dialog->resize(1000, 500);
	QVBoxLayout* layout = new QVBoxLayout(this);

	QTableWidget* diaTableWidget = new QTableWidget();
	// 隐藏行号
	diaTableWidget->verticalHeader()->setVisible(false);
	// 隐藏列号
	diaTableWidget->horizontalHeader()->setVisible(false);
	QDir dir;
	QString filepath = dir.absoluteFilePath("src/database/标准数据库.xlsx");
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
				QTableWidgetItem* item = new QTableWidgetItem(xlsx.read(row, col).toString());
				item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
				diaTableWidget->setItem(row - 1, col - 1, item);
			}
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
				if (col == 2)
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

			JudgementPropertyInfo info;
			info.name = m_tableWidget->item(1, 1)->text();
			info.isChecked = true;
			ins->SetJudgementPropertyInfo(info);

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
					QString text = timeStr + "[信息]>选择评判标准数据";
					textEdit->appendPlainText(text);
					logWidget->update();

					// 关键：强制刷新UI，确保日志立即显示
					QApplication::processEvents();

					// 写入数据库模块
					DatabasePropertyWidget* m_databasePropertyWidget = gfParent->GetDatabasePropertyWidget();
					QTableWidget* databaseTableWid = m_databasePropertyWidget->GetQTableWidget();
					QTableWidgetItem* valueItem = new QTableWidgetItem(value);
					valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
					valueItem->setBackground(QBrush(QColor(230, 230, 230)));
					databaseTableWid->setItem(1, 2, valueItem);

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
