#pragma once

#include <QWidget>
#include "ui_DatabaseWidget.h"
#include <QResizeEvent>

class DatabaseWidget : public QWidget
{
	Q_OBJECT

public:
	DatabaseWidget(QWidget *parent = nullptr);
	~DatabaseWidget();

	QTableWidget* getTableWid();

	QTreeWidget* getQTreeWid();

private:
	Ui::DatabaseWidgetClass ui;

	// 当前选中的数据库类型
	QString currentDataaseType;

	// 窗口内所有的控件
	QMap<QWidget*, QRect> allWidgetMap;
	// 窗口内所有的布局
	QMap<QLayout*, QRect> allLayoutMap;
	// 窗口原始尺寸
	QSize windowOriginalSize;
	/** 查找窗口内所有的布局和控件 **/
	void findAllLayoutAndWidget(QObject *object);

	// TreeWidge点击事件
	void onTreeItemClicked(QTreeWidgetItem *item);


protected:
	/** 窗口尺寸改变时自动调用 **/
	void resizeEvent(QResizeEvent* event) override;

public slots:

	void importData();

	void exportData();

	void showTooltip(QTableWidgetItem *item);

	

private:
	int m_rowCount = 0;
	int m_columnCount = 0;
	QString m_privateDirPath = "";

	QMenu *contextMenu; // 右键菜单对象
};
