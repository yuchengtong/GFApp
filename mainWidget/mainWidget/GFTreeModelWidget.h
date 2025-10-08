#pragma once
#include <QTreeWidget>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QMessageBox>

#include <TopoDS_Shape.hxx>
#include "GFTreeWidget.h"

class GFTreeModelWidget :public QWidget
{
	Q_OBJECT
public:
	GFTreeModelWidget(QWidget*parent = nullptr);
	~GFTreeModelWidget();

public:
	void updataIcon();

	//void updateCheckBoxFunc(QModelIndex index);

protected:
	void contextMenuEvent(QContextMenuEvent *event) override;

signals:
	void itemClicked(const QString& itemData);

private slots:
	void onTreeItemClicked(QTreeWidgetItem* item, int column);

	void exportWord(const QString& text);

	bool exportFromTemplate(const QString &templatePath, const QString &outputPath, const QMap<QString, QString> &data);


private:
	QMenu *contextMenu; // ÓÒ¼ü²Ëµ¥¶ÔÏó

	GFTreeWidget* treeWidget = nullptr;
	TopoDS_Shape m_aShape;
};

