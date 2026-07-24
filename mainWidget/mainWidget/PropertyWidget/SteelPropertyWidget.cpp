#pragma once
#pragma execution_character_set("utf-8")
#include "SteelPropertyWidget.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QDir>
#include <QPushButton>
#include <QDialog>
#include "ModelDataManager.h"
#include "../GFTreeModelWidget.h"
#include "../GFImportModelWidget.h"
#include "xlsxdocument.h"


#include <QDateTime>
#include <QApplication>

SteelPropertyWidget::SteelPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
	m_tableWidget->setStyleSheet(
		"QTableWidget {"
		"	background-color: #ffffff;"
		"   border: 2px solid #999999;"
		"   border-radius: 12px;"
		"}"

		"QPushButton {"
		"   background-color: #f0f0f0;"
		"   border: 1px solid #ccc;"
		"   border-radius: 8px;"
		"   padding: 4px 8px;"
		"   min-width: 60px;"
		"}"
		"QPushButton:hover {"
		"   background-color: #e0e0e0;"
		"}"
		"QPushButton:pressed {"
		"   background-color: #d0d0d0;"
		"}"
	);
}

void SteelPropertyWidget::initWidget()
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

	QStringList labels = { "材料属性","材料牌号", "密度", "热膨胀系数", "弹性模量","切线模量","泊松比","屈服强度","抗拉强度","热导率","比热容" };
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
	QTableWidgetItem* colimnItem = m_tableWidget->item(3, 1);
	int itemWidth = QFontMetrics(m_tableWidget->font()).width(colimnItem->text());
	m_tableWidget->setColumnWidth(1, itemWidth + m_tableWidget->verticalHeader()->width());

	// 单位列
	QStringList unitLabels = { " "," ", "kg/m^3", "/℃", "MPa","MPa"," ","MPa","MPa","W/(m∙℃)","J/(kg∙℃)" };
	for (int row = 0; row < unitLabels.size(); ++row) {
		if (row != 0)
		{
			QTableWidgetItem* labelItem = new QTableWidgetItem(unitLabels[row]);
			labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			m_tableWidget->setItem(row, 3, labelItem);
		}

	}

	QTableWidgetItem* unitColimnItem = m_tableWidget->item(10, 3);
	int unitItemWidth = QFontMetrics(m_tableWidget->font()).width(unitColimnItem->text());
	m_tableWidget->setColumnWidth(3, unitItemWidth + m_tableWidget->verticalHeader()->width());

	// 将第0行0列的单元格文本字体加粗
	QTableWidgetItem* headerItem = m_tableWidget->item(0, 0);
	if (headerItem) {
		QFont font = headerItem->font();
		font.setBold(true);
		headerItem->setFont(font);
	}

	// 导入按钮
	QPushButton* importButton = new QPushButton("导入");
	importButton->setIcon(QIcon(":/tree/Tree/import.svg"));
	const int btnSize = 20;
	QSize iconSize(btnSize, btnSize);
	importButton->setIconSize(iconSize);
	m_tableWidget->setCellWidget(0, 2, importButton);

	connect(importButton, &QPushButton::clicked, this, &SteelPropertyWidget::showTableDialog);
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

void SteelPropertyWidget::showTableDialog()
{
    QDialog* dialog = new QDialog();
    dialog->setWindowIcon(QIcon(":/src/engine.svg"));
    dialog->setWindowTitle("壳体材料库");
    dialog->resize(1000, 550);
    QVBoxLayout* dialogLayout = new QVBoxLayout(dialog); // 父对象修正为dialog

    QTabWidget* tabWidget = new QTabWidget(dialog);

    // ========== Tab1：金属材料页面 ==========
    QWidget* metalTabPage = new QWidget();
    QVBoxLayout* metalLayout = new QVBoxLayout(metalTabPage);
    QTableWidget* metalTable = new QTableWidget();
    metalTable->verticalHeader()->setVisible(false);
    metalTable->horizontalHeader()->setVisible(false);
    metalTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    metalTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    metalTable->setSelectionMode(QAbstractItemView::SingleSelection);
    metalLayout->addWidget(metalTable);
    tabWidget->addTab(metalTabPage, "金属材料");

    // ========== Tab2：非金属材料页面 ==========
    QWidget* nonMetalTabPage = new QWidget();
    QVBoxLayout* nonMetalLayout = new QVBoxLayout(nonMetalTabPage);
    QTableWidget* nonMetalTable = new QTableWidget();
    nonMetalTable->verticalHeader()->setVisible(false);
    nonMetalTable->horizontalHeader()->setVisible(false);
    nonMetalTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    nonMetalTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    nonMetalTable->setSelectionMode(QAbstractItemView::SingleSelection);
    nonMetalLayout->addWidget(nonMetalTable);
    tabWidget->addTab(nonMetalTabPage, "非金属材料");

    dialogLayout->addWidget(tabWidget);

    auto ins = ModelDataManager::GetInstance();
    DatabaseInfo databaseInfo = ins->GetDatabaseInfo();
    UserInfo info = ins->GetUserInfo();
    QDir dir;
    QString workDir = info.workdir;

    // ===================== 填充金属材料表格 =====================
    const QVector<QVector<QString>>& metalData = databaseInfo.m_metalData;
    int metalBaseRowCnt = metalData.size();
    if (!metalData.isEmpty())
    {
        int colCnt = metalData.first().size();
        metalTable->setRowCount(metalBaseRowCnt);
        metalTable->setColumnCount(colCnt);
        for (int row = 0; row < metalBaseRowCnt; ++row)
        {
            for (int col = 0; col < colCnt; ++col)
            {
                QTableWidgetItem* item = new QTableWidgetItem(metalData[row][col]);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                metalTable->setItem(row, col, item);
            }
        }
    }

    // 追加私有金属Excel
    QString metalXlsxPath = dir.absoluteFilePath(workDir + "/database/壳体金属材料.xlsx");
    QFile metalXlsxFile(metalXlsxPath);
    if (!metalXlsxPath.isEmpty() && metalXlsxFile.exists())
    {
        QXlsx::Document xlsx(metalXlsxPath);
        auto dim = xlsx.dimension();
        int xlsxRowCnt = dim.lastRow();
        int xlsxColCnt = dim.lastColumn();
        metalTable->setRowCount(metalBaseRowCnt + xlsxRowCnt - 1);
        int insertRow = metalBaseRowCnt;
        for (int row = 2; row <= xlsxRowCnt; ++row)
        {
            for (int col = 1; col <= xlsxColCnt; ++col)
            {
                QTableWidgetItem* item = new QTableWidgetItem(xlsx.read(row, col).toString());
                metalTable->setItem(insertRow, col - 1, item);
            }
            insertRow++;
        }
    }

    // ===================== 填充非金属材料表格 =====================
    const QVector<QVector<QString>>& nonMetalData = databaseInfo.m_nonmetallicData;
    int nonMetalBaseRowCnt = nonMetalData.size();
    if (!nonMetalData.isEmpty())
    {
        int colCnt = nonMetalData.first().size();
        nonMetalTable->setRowCount(nonMetalBaseRowCnt);
        nonMetalTable->setColumnCount(colCnt);
        for (int row = 0; row < nonMetalBaseRowCnt; ++row)
        {
            for (int col = 0; col < colCnt; ++col)
            {
                QTableWidgetItem* item = new QTableWidgetItem(nonMetalData[row][col]);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                nonMetalTable->setItem(row, col, item);
            }
        }
    }

    // 追加私有非金属Excel
    QString nonMetalXlsxPath = dir.absoluteFilePath(workDir + "/database/壳体非金属材料.xlsx");
    QFile nonMetalXlsxFile(nonMetalXlsxPath);
    if (!nonMetalXlsxPath.isEmpty() && nonMetalXlsxFile.exists())
    {
        QXlsx::Document xlsx(nonMetalXlsxPath);
        auto dim = xlsx.dimension();
        int xlsxRowCnt = dim.lastRow();
        int xlsxColCnt = dim.lastColumn();
        nonMetalTable->setRowCount(nonMetalBaseRowCnt + xlsxRowCnt - 1);
        int insertRow = nonMetalBaseRowCnt;
        for (int row = 2; row <= xlsxRowCnt; ++row)
        {
            for (int col = 1; col <= xlsxColCnt; ++col)
            {
                QTableWidgetItem* item = new QTableWidgetItem(xlsx.read(row, col).toString());
                nonMetalTable->setItem(insertRow, col - 1, item);
            }
            insertRow++;
        }
    }

    // ===================== 双击槽函数封装（金属） =====================
    auto slotCellDoubleClick = [this, dialog](QTableWidget* table, bool isMetal)
    {
        connect(table, &QTableWidget::cellDoubleClicked, this, [this, dialog, table, isMetal](int row, int column)
            {
                if (row == 0) return; // 跳过表头

                int colcount = table->columnCount();
                QString materialName;
                // 先回填当前窗口表格
                if (isMetal)
                {
                    // 金属
                    for (int col = 1; col < colcount; ++col)
                    {
                        QTableWidgetItem* srcItem = table->item(row, col);
                        if (!srcItem) continue;
                        QString content = srcItem->text();
                        if (col == 1)
                            materialName = content;

                        QTableWidgetItem* valueItem = new QTableWidgetItem(content);
                        valueItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
                        valueItem->setBackground(QBrush(QColor(230, 230, 230)));
                        m_tableWidget->setItem(col, 2, valueItem);
                    }
                }
                else
                {
                    int tabCol = 1;
                    // 非金属
                    for (int col = 1; col < colcount; ++col)
                    {
                        QTableWidgetItem* srcItem = table->item(row, col);
                        if (!srcItem) continue;
                        QString content = srcItem->text();
                        if (col == 1)
                            materialName = content;

                        QTableWidgetItem* valueItem = new QTableWidgetItem(content);
                        valueItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
                        valueItem->setBackground(QBrush(QColor(230, 230, 230)));
                        m_tableWidget->setItem(tabCol, 2, valueItem);
                        if ((col >= 5 && col <= 6) || (col >= 8 && col <= 9) || (col >= 11 && col <= 12))
                        {
                            continue;
                        }
                        else
                        {
                            tabCol = tabCol + 1;
                        }
                    }
                }

                

                auto ins = ModelDataManager::GetInstance();
                SteelPropertyInfo info;
                info.materialGrade = m_tableWidget->item(1, 2)->text();
                info.density = m_tableWidget->item(2, 2)->text().toDouble();
                info.thermalExpansion = m_tableWidget->item(3, 2)->text().toDouble();
                info.modulus = m_tableWidget->item(4, 2)->text().toDouble();
                info.tangentModulus = m_tableWidget->item(5, 2)->text().toDouble();
                info.poisonby = m_tableWidget->item(6, 2)->text().toDouble();
                info.yieldStrength = m_tableWidget->item(7, 2)->text().toDouble();
                info.tensileStrength = m_tableWidget->item(8, 2)->text().toDouble();
                info.thermalConductivity = m_tableWidget->item(9, 2)->text().toDouble();
                info.specificHeatCapacity = m_tableWidget->item(10, 2)->text().toDouble();
                info.isChecked = true;
                ins->SetSteelPropertyInfo(info);

                // 向上遍历父窗口，更新树、日志、材料属性表格（原有逻辑不变）
                QWidget* parent = parentWidget();
                while (parent)
                {
                    GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
                    if (gfParent)
                    {
                        gfParent->GetGFTreeModelWidget()->updataIcon();
                        QDateTime currentTime = QDateTime::currentDateTime();
                        QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
                        auto logWidget = gfParent->GetLogWidget();
                        auto textEdit = logWidget->GetTextEdit();
                        QString typeStr = isMetal ? "金属" : "非金属";
                        QString text = timeStr + QString("[信息]>开始导入壳体%1材料数据").arg(typeStr);
                        textEdit->appendPlainText(text);
                        logWidget->update();
                        QApplication::processEvents();

                        MaterialPropertyWidget* m_materialPropertyWidget = gfParent->GetMaterialPropertyWidget();
                        QTableWidget* materialTableWid = m_materialPropertyWidget->GetQTableWidget();
                        QTableWidgetItem* valueItem = new QTableWidgetItem(materialName);
                        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
                        valueItem->setBackground(QBrush(QColor(230, 230, 230)));
                        materialTableWid->setItem(1, 2, valueItem);
                        break;
                    }
                    parent = parent->parentWidget();
                }
                dialog->close();
            });
    };

    // 绑定两个表格双击信号
    slotCellDoubleClick(metalTable, true);
    slotCellDoubleClick(nonMetalTable, false);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}