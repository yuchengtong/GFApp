#pragma once
#include <QTreeWidget>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QtCharts>
#include <QChartView>

#include "GFTreeWidget.h"
#include "APIPolynomialFitter.h"

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
	//void updateChartData(QChartView* chartView, QChart* chart, QLineSeries* lineSeries1, QLineSeries* lineSeries2, QLineSeries* lineSeries3, QScatterSeries* scatter1, QScatterSeries* scatter2, QScatterSeries* scatter3, QVector<QPointF> data1, QVector<QPointF> data2, QVector<QPointF> data3, QString xAxisTitle, QString yAxisTitle);
	void updateChartData(QChartView* chartView, QChart* chart,
		QLineSeries* lineSeries1, QLineSeries* lineSeries2, QLineSeries* lineSeries3,
		QLineSeries* quadSeries1, QLineSeries* quadSeries2, QLineSeries* quadSeries3,
		QScatterSeries* scatter1, QScatterSeries* scatter2, QScatterSeries* scatter3,
		QVector<QPointF> data1, QVector<QPointF> data2, QVector<QPointF> data3,
		QString xAxisTitle, QString yAxisTitle);

	qreal calculateMaxValue(const QVector<QPointF>& series, bool isX);

	qreal calculateMinValue(const QVector<QPointF>& series, bool isX);

	void exportWord(const QString& directory,
		const QMap<QString, QVariant>& textData,
		const QMap<QString, QString>& imageWidgets,
		const QMap<QString, QVector<QVector<QVariant>>>& tableData);



private:
	QMenu* contextMenu; // 右键菜单对象

	GFTreeWidget* treeWidget = nullptr;

	//WordExporter* wordExporter = nullptr;
};

