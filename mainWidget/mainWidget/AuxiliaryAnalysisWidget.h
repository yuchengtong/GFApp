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

#include "CustomPolarChart.h"

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
	// µøÂäÊÔÑé
	CustomPolarChart*m_polarChart1;
	// ¿ìËÙ¿¾È¼ÊÔÑé
	CustomPolarChart*m_polarChart2;
	// ÂıËÙ¿¾È¼ÊÔÑé
	CustomPolarChart*m_polarChart3;
	// Ç¹»÷ÊÔÑé
	CustomPolarChart*m_polarChart4;
	// ÉäÁ÷³å»÷ÊÔÑé
	CustomPolarChart*m_polarChart5;
	// ÆÆÆ¬×²»÷ÊÔÑé
	CustomPolarChart*m_polarChart6;
	// ±¬Õ¨³å»÷²¨ÊÔÑé
	CustomPolarChart*m_polarChart7;
	// Ñ³±¬ÊÔÑé
	CustomPolarChart*m_polarChart8;

};
