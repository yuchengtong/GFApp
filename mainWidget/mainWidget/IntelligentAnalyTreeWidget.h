#pragma once
#include <QTreeWidget>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QMessageBox>

#include "GFTreeWidget.h"

class IntelligentAnalyTreeWidget :public QWidget
{
	Q_OBJECT
public:
	IntelligentAnalyTreeWidget(QWidget* parent = nullptr);
	~IntelligentAnalyTreeWidget();

public:
	void updataIcon();


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

