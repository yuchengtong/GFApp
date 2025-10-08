#pragma once
#include <QTreeWidget>
#include "GFTreeWidgetItemDelegate.h"
class GFTreeWidget :public QTreeWidget
{
	Q_OBJECT
public:
	GFTreeWidget(QWidget* parent = nullptr);
	~GFTreeWidget();

	void updateCheckBoxFunc(QModelIndex index);

private:
	GFTreeWidgetItemDelegate* m_TreeWidgetItemDelegate = nullptr;
private:
	friend GFTreeWidgetItemDelegate;
};

