#pragma once
#include <QTreeWidget>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QtCharts>
#include <QChartView>

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

	// 更新图表数据
	void updateChartData(QChartView* chartView, QVector<QPointF> data, QString xAxisTitle, QString yAxisTitle);

	qreal calculateMaxValue(const QVector<QPointF>& series, bool isX);


private:
	QMenu* contextMenu; // 右键菜单对象

	GFTreeWidget* treeWidget = nullptr;
};

