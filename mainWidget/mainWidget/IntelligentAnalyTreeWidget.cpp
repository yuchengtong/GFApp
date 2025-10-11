#pragma execution_character_set("utf-8")
#include "IntelligentAnalyTreeWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QApplication>
#include <QIcon>
#include <QFileDialog>
#include <QDateTime>
#include <algorithm>
#include <QRegExp>
#include <QRegularExpression> 
#include <QValidator>



IntelligentAnalyTreeWidget::IntelligentAnalyTreeWidget(QWidget* parent)
	:QWidget(parent)
{
	QIcon error_icon(":/src/Error.svg");
	QIcon checked_icon(":/src/Checked.svg");
	QIcon icon(":/src/data_calculation.svg");

	treeWidget = new GFTreeWidget(this);
	treeWidget->setColumnCount(1);
	treeWidget->setHeaderLabels({ "数据智能分析" });
	treeWidget->setHeaderHidden(true);
	treeWidget->setStyleSheet("QTreeWidget { color: black; }");
	treeWidget->setColumnCount(1);

	// 创建根节点
	QTreeWidgetItem* rootItem = new QTreeWidgetItem(treeWidget);
	rootItem->setText(0, "数据智能分析");
	rootItem->setData(0, Qt::UserRole, "IntelligentAnaly");
	rootItem->setExpanded(true);
	rootItem->setCheckState(0, Qt::Unchecked);
	rootItem->setIcon(0, icon);

	QTreeWidgetItem* fallNode = new QTreeWidgetItem(rootItem);
	fallNode->setText(0, "跌落试验智能分析");
	fallNode->setData(0, Qt::UserRole, "FallIntelligentAnaly");
	fallNode->setCheckState(0, Qt::Unchecked);
	fallNode->setIcon(0, icon);

	QTreeWidgetItem* fastCombustionNode = new QTreeWidgetItem(rootItem);
	fastCombustionNode->setText(0, "快速烤燃试验试验智能分析");
	fastCombustionNode->setData(0, Qt::UserRole, "FastCombustionIntelligentAnaly");
	fastCombustionNode->setCheckState(0, Qt::Unchecked);
	fastCombustionNode->setIcon(0, icon);

	QTreeWidgetItem* slowCombustionNode = new QTreeWidgetItem(rootItem);
	slowCombustionNode->setText(0, "慢速烤燃试验智能分析");
	slowCombustionNode->setData(0, Qt::UserRole, "SlowCombustionIntelligentAnaly");
	slowCombustionNode->setCheckState(0, Qt::Unchecked);
	slowCombustionNode->setIcon(0, icon);

	QTreeWidgetItem* shootNode = new QTreeWidgetItem(rootItem);
	shootNode->setText(0, "枪击试验智能分析");
	shootNode->setData(0, Qt::UserRole, "ShootIntelligentAnaly");
	shootNode->setCheckState(0, Qt::Unchecked);
	shootNode->setIcon(0, icon);

	QTreeWidgetItem* jetImpactNode = new QTreeWidgetItem(rootItem);
	jetImpactNode->setText(0, "射流冲击试验智能分析");
	jetImpactNode->setData(0, Qt::UserRole, "JetImpactIntelligentAnaly");
	jetImpactNode->setCheckState(0, Qt::Unchecked);
	jetImpactNode->setIcon(0, icon);

	QTreeWidgetItem* fragmentationImpactNode = new QTreeWidgetItem(rootItem);
	fragmentationImpactNode->setText(0, "破片撞击试验智能分析");
	fragmentationImpactNode->setData(0, Qt::UserRole, "FragmentationImpactIntelligentAnaly");
	fragmentationImpactNode->setCheckState(0, Qt::Unchecked);
	fragmentationImpactNode->setIcon(0, icon);

	QTreeWidgetItem* explosiveBlastNode = new QTreeWidgetItem(rootItem);
	explosiveBlastNode->setText(0, "爆炸冲击波试验智能分析");
	explosiveBlastNode->setData(0, Qt::UserRole, "ExplosiveBlastIntelligentAnaly");
	explosiveBlastNode->setCheckState(0, Qt::Unchecked);
	explosiveBlastNode->setIcon(0, icon);

	QTreeWidgetItem* sacrificeExplosionNode = new QTreeWidgetItem(rootItem);
	sacrificeExplosionNode->setText(0, "殉爆试验智能分析");
	sacrificeExplosionNode->setData(0, Qt::UserRole, "SacrificeExplosionIntelligentAnaly");
	sacrificeExplosionNode->setCheckState(0, Qt::Unchecked);
	sacrificeExplosionNode->setIcon(0, icon);


	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(treeWidget);
	layout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(layout);

	// 连接信号槽以处理复选框点击事件
	connect(treeWidget, &QTreeWidget::itemChanged, [](QTreeWidgetItem* item, int column) {
		if (item->childCount() > 0) {
			// 如果父节点被选中，则选中所有子节点
			if (item->checkState(0) == Qt::Checked) {
				for (int i = 0; i < item->childCount(); ++i) {
					item->child(i)->setCheckState(0, Qt::Checked);
				}
			}
			// 如果父节点被取消选中，则取消选中所有子节点
			else if (item->checkState(0) == Qt::Unchecked) {
				for (int i = 0; i < item->childCount(); ++i) {
					item->child(i)->setCheckState(0, Qt::Unchecked);
				}
			}
		}
		});

	// 连接信号槽
	connect(treeWidget, &QTreeWidget::itemClicked, this, &IntelligentAnalyTreeWidget::onTreeItemClicked);
}

IntelligentAnalyTreeWidget::~IntelligentAnalyTreeWidget()
{
}

void IntelligentAnalyTreeWidget::onTreeItemClicked(QTreeWidgetItem* item, int column)
{
	QString itemData = item->data(0, Qt::UserRole).toString();
	emit itemClicked(itemData);
}

void IntelligentAnalyTreeWidget::updataIcon()
{

}



void IntelligentAnalyTreeWidget::contextMenuEvent(QContextMenuEvent* event)
{
	QTreeWidgetItem* item = treeWidget->itemAt(event->pos());
	if (!item) {
		return;
	}
	QString text = item->text(0);
	if (text == "数据智能分析")
	{
		contextMenu = new QMenu(this);
		QAction* calAction = new QAction("计算", this);

		int childCount = item->childCount();
		QList<QTreeWidgetItem*> checkedChildItems;
		for (int i = 0; i < childCount; ++i) {
			QTreeWidgetItem* childItem = item->child(i);
			if (childItem->checkState(0) == Qt::Checked) {
				checkedChildItems.append(childItem);
			}
		}
		connect(calAction, &QAction::triggered, this, [item, this]() {
			QWidget* parent = parentWidget();
			while (parent)
			{
				QMessageBox::information(this, "提示", QString("开始计算"));
				break;
			}
		});
		contextMenu->addAction(calAction); // 将动作添加到菜单中
		contextMenu->exec(event->globalPos()); // 在鼠标位置显示菜单
	}
}

