#pragma once
#include <QTreeWidget>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QMessageBox>

#include "GFTreeWidget.h"

class ParamAnalyTreeWidget :public QWidget
{
	Q_OBJECT
public:
	ParamAnalyTreeWidget(QWidget* parent = nullptr);
	~ParamAnalyTreeWidget();


protected:
	void contextMenuEvent(QContextMenuEvent* event) override;

signals:
	void itemClicked(const QString& itemData);

private slots:
	void onTreeItemClicked(QTreeWidgetItem* item, int column);




private:
	QMenu* contextMenu; // ÓÒ¼ü²Ëµ¥¶ÔÏó

	GFTreeWidget* treeWidget = nullptr;
};

