#pragma once

#include <QWidget>
#include "ui_AuxiliaryAnalysisWidget.h"
#include <QtCharts>
#include <QLineSeries>
#include <QBarSeries>
#include <QtCharts\qchartview.h>
#include <QLinearGradient>
#include <QPen>
#include <QBrush>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QVector>
#include <QGraphicsLineItem>
#include <QList>
#include <QGraphicsSimpleTextItem>

class AuxiliaryAnalysisWidget : public QWidget
{
	Q_OBJECT

public:
	AuxiliaryAnalysisWidget(QWidget *parent = nullptr);
	~AuxiliaryAnalysisWidget();

	QMap<int, QWidget*> getChartWidget();

private:
	Ui::AuxiliaryAnalysisWidgetClass ui;

	QMap<int, QWidget*> chartWidMap;

public slots:
	//void handleLegendMarkerClicked();


private:
	// 数据标签
	QPolarChart *m_polarChart1;

	QCategoryAxis *angularAxis1;

	QLineSeries *m_stressSeries1;    // 应力
	QLineSeries *m_temperatureSeries1; // 温度
	QLineSeries *m_overpressureSeries1; // 超压
	QLineSeries *m_strainSeries1;    // 应变

	QAreaSeries *m_stressArea1;
	QAreaSeries *m_temperatureArea1;
	QAreaSeries *m_overpressureArea1;
	QAreaSeries *m_strainArea1;


	// 数据标签
	QPolarChart *m_polarChart2;

	QCategoryAxis *angularAxis2;

	QLineSeries *m_stressSeries2;    // 应力
	QLineSeries *m_temperatureSeries2; // 温度
	QLineSeries *m_overpressureSeries2; // 超压
	QLineSeries *m_strainSeries2;    // 应变

	QAreaSeries *m_stressArea2;
	QAreaSeries *m_temperatureArea2;
	QAreaSeries *m_overpressureArea2;
	QAreaSeries *m_strainArea2;

	// 数据标签
	QPolarChart *m_polarChart3;

	QCategoryAxis *angularAxis3;

	QLineSeries *m_stressSeries3;    // 应力
	QLineSeries *m_temperatureSeries3; // 温度
	QLineSeries *m_overpressureSeries3; // 超压
	QLineSeries *m_strainSeries3;    // 应变

	QAreaSeries *m_stressArea3;
	QAreaSeries *m_temperatureArea3;
	QAreaSeries *m_overpressureArea3;
	QAreaSeries *m_strainArea3;


	// 数据标签
	QPolarChart *m_polarChart4;

	QCategoryAxis *angularAxis4;

	QLineSeries *m_stressSeries4;    // 应力
	QLineSeries *m_temperatureSeries4; // 温度
	QLineSeries *m_overpressureSeries4; // 超压
	QLineSeries *m_strainSeries4;    // 应变

	QAreaSeries *m_stressArea4;
	QAreaSeries *m_temperatureArea4;
	QAreaSeries *m_overpressureArea4;
	QAreaSeries *m_strainArea4;


	// 数据标签
	QPolarChart *m_polarChart5;

	QCategoryAxis *angularAxis5;

	QLineSeries *m_stressSeries5;    // 应力
	QLineSeries *m_temperatureSeries5; // 温度
	QLineSeries *m_overpressureSeries5; // 超压
	QLineSeries *m_strainSeries5;    // 应变

	QAreaSeries *m_stressArea5;
	QAreaSeries *m_temperatureArea5;
	QAreaSeries *m_overpressureArea5;
	QAreaSeries *m_strainArea5;

	// 数据标签
	QPolarChart *m_polarChart6;

	QCategoryAxis *angularAxis6;

	QLineSeries *m_stressSeries6;    // 应力
	QLineSeries *m_temperatureSeries6; // 温度
	QLineSeries *m_overpressureSeries6; // 超压
	QLineSeries *m_strainSeries6;    // 应变

	QAreaSeries *m_stressArea6;
	QAreaSeries *m_temperatureArea6;
	QAreaSeries *m_overpressureArea6;
	QAreaSeries *m_strainArea6;

	// 数据标签
	QPolarChart *m_polarChart7;

	QCategoryAxis *angularAxis7;

	QLineSeries *m_stressSeries7;    // 应力
	QLineSeries *m_temperatureSeries7; // 温度
	QLineSeries *m_overpressureSeries7; // 超压
	QLineSeries *m_strainSeries7;    // 应变

	QAreaSeries *m_stressArea7;
	QAreaSeries *m_temperatureArea7;
	QAreaSeries *m_overpressureArea7;
	QAreaSeries *m_strainArea7;

	// 数据标签
	QPolarChart *m_polarChart8;

	QCategoryAxis *angularAxis8;

	QLineSeries *m_stressSeries8;    // 应力
	QLineSeries *m_temperatureSeries8; // 温度
	QLineSeries *m_overpressureSeries8; // 超压
	QLineSeries *m_strainSeries8;    // 应变

	QAreaSeries *m_stressArea8;
	QAreaSeries *m_temperatureArea8;
	QAreaSeries *m_overpressureArea8;
	QAreaSeries *m_strainArea8;

	QVector<QPointF> generateData(int seed);
	void handleLegendClick1();
	void handleLegendClick2();
	void handleLegendClick3();
	void handleLegendClick4();
	void handleLegendClick5();
	void handleLegendClick6();
	void handleLegendClick7();
	void handleLegendClick8();
};
