#pragma execution_character_set("utf-8")
#include "GFTreeParamAnalyWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QApplication>




GFTreeParamAnalyWidget::GFTreeParamAnalyWidget(QWidget*parent)
	:QWidget(parent)
{
	setMaximumWidth(240);
	QVBoxLayout*layout = new QVBoxLayout(this);
	QTreeWidget*treeWidget = new QTreeWidget(this);
	treeWidget->setColumnCount(1);
	treeWidget->setHeaderLabels({ "安全性特性参数分析" });
	QTreeWidgetItem*rootItem = new QTreeWidgetItem(treeWidget);
	rootItem->setText(0, "实体");
	QTreeWidgetItem*childItem1 = new QTreeWidgetItem(rootItem);
	childItem1->setText(0, "面");
	QTreeWidgetItem*childItem2 = new QTreeWidgetItem(rootItem);
	childItem2->setText(0, "线");

	treeWidget->addTopLevelItem(rootItem);

	treeWidget->setIndentation(20);
	treeWidget->setAnimated(true);

	layout->addWidget(treeWidget);
	layout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(layout);

}

GFTreeParamAnalyWidget::~GFTreeParamAnalyWidget()
{
}
