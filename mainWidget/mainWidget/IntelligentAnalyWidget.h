#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QStackedWidget>
#include <QtCharts>
#include <QChartView>
#include <QComboBox>
#include <QLabel>
#include <QScatterSeries>



#include "IntelligentAnalyTreeWidget.h"
#include "IntelligentPropertyWidget.h"
#include "IntelligentFallPropertyWidget.h"
#include "IntelligentFastCombustionPropertyWidget.h"
#include "IntelligentSlowCombustionPropertyWidget.h"
#include "IntelligentShootPropertyWidget.h"
#include "IntelligentJetImpactPropertyWidget.h"
#include "IntelligentFragmentationImpactPropertyWidget.h"
#include "IntelligentExplosiveBlastPropertyWidget.h"
#include "IntelligentSacrificeExplosionPropertyWidget.h"

#include "GraphicWidget.h"


class IntelligentAnalyWidget : public QWidget
{
	Q_OBJECT

public:
	IntelligentAnalyWidget(QWidget* parent = nullptr);
	~IntelligentAnalyWidget();

	QComboBox* getXComboBox() { return x_comboBox; }
	QComboBox* getYComboBox() { return y_comboBox; }


	QTableWidget* getFallTableWidget() { return m_fallTableWidget; }
	QTableWidget* getFastCombustionTableWidget() { return m_fastCombustionTableWidget; }
	QTableWidget* getSlowCombustionTableWidget() { return m_slowCombustionTableWidget; }
	QTableWidget* getShootTableWidget() { return m_shootTableWidget; }
	QTableWidget* getJetImpactTableWidget() { return m_jetImpactTableWidget; }
	QTableWidget* getFragmentationImpactTableWidget() { return m_fragmentationImpactTableWidget; }
	QTableWidget* getExplosiveBlastTableWidget() { return m_explosiveBlastTableWidget; }
	QTableWidget* getSacrificeExplosionTableWidget() { return m_sacrificeExplosionTableWidget; }

	QStackedWidget* getStackedWidget() { return  m_tableStackWidget; } 

	QChartView* getChartView() { return m_chartView; }
	QChart* getChart() { return m_chart; }
	QValueAxis* getAxisX() { return m_axisX; }
	QValueAxis* getAxisY() { return m_axisY; }
	QLineSeries* getLineSeries1() { return m_lineSeries1; }
	QLineSeries* getLineSeries2() { return m_lineSeries2; }
	QLineSeries* getLineSeries3() { return m_lineSeries3; }
	QScatterSeries* getScatter1() { return m_scatter1; }
	QScatterSeries* getScatter2() { return m_scatter2; }
	QScatterSeries* getScatter3() { return m_scatter3; }

	// 更新三维坐标数据
	void updateGraphicData(QString xName, QString yName, QString zName, 
		const QVector<double>& xCoords,
		const QVector<double>& yCoords, 
		const QVector<QVector<double>>& newData,
		double xMin,
		double xMax,
		double yMin,
		double yMax);


private slots:
	void onTreeItemClicked(const QString& itemData);

	void onComboBoxIndexChanged(int index);

	void onComboBoxIndexGraphicChanged(int index);

	void dataChange(int index);

	void hspinChange(int val);

	void vspinChange(int val);

	

private:

	QStackedWidget* m_propertyStackWidget = nullptr;
	QStackedWidget* m_tableStackWidget = nullptr;
	IntelligentAnalyTreeWidget* m_treeModelWidget = nullptr;
	QTableWidget* m_tableWidget = nullptr;

	IntelligentPropertyWidget* m_intelligentPropertyWidget = nullptr;
	IntelligentFallPropertyWidget* m_fallPropertyWidget = nullptr;
	IntelligentFastCombustionPropertyWidget* m_fastCombustionPropertyWidget = nullptr;
	IntelligentSlowCombustionPropertyWidget* m_slowCombustionPropertyWidget = nullptr;
	IntelligentShootPropertyWidget* m_shootPropertyWidget = nullptr;
	IntelligentJetImpactPropertyWidget* m_jetImpactPropertyWidget = nullptr;
	IntelligentFragmentationImpactPropertyWidget* m_fragmentationImpactPropertyWidget = nullptr;
	IntelligentExplosiveBlastPropertyWidget* m_explosiveBlastPropertyWidget = nullptr;
	IntelligentSacrificeExplosionPropertyWidget* m_sacrificeExplosionPropertyWidget = nullptr;

	QTableWidget* m_fallTableWidget = nullptr;
	QTableWidget* m_fastCombustionTableWidget = nullptr;
	QTableWidget* m_slowCombustionTableWidget = nullptr;
	QTableWidget* m_shootTableWidget = nullptr;
	QTableWidget* m_jetImpactTableWidget = nullptr;
	QTableWidget* m_fragmentationImpactTableWidget = nullptr;
	QTableWidget* m_explosiveBlastTableWidget = nullptr;
	QTableWidget* m_sacrificeExplosionTableWidget = nullptr;
	




	QWidget* graphicWid = nullptr;

	QComboBox* x_comboBox;
	QComboBox* y_comboBox;
	

	QChartView* m_chartView;
	QChart* m_chart;              // 图表核心
	// 每组数据：1条曲线 + 1个散点集（关键修改）
	QLineSeries* m_lineSeries1;   // 第一组曲线
	QScatterSeries* m_scatter1;   // 第一组圆点
	QLineSeries* m_lineSeries2;   // 第二组曲线
	QScatterSeries* m_scatter2;   // 第二组圆点
	QLineSeries* m_lineSeries3;   // 第三组曲线
	QScatterSeries* m_scatter3;   // 第三组圆点
	QValueAxis* m_axisX;          // X轴
	QValueAxis* m_axisY;


	QChartView* chartView2;

	QComboBox* m_grapgicComboBox;

	GraphicWidget* m_3dGraphicWid;

	QHBoxLayout* graphicLayout;
	QVBoxLayout* m_leftLayout;

	QMap<QString, QStringList> m_dataMap;

	// 更新图表数据
	void updateChartData(QVector<QPointF> data1, QVector<QPointF> data2, QVector<QPointF> data3, QString xAxisTitle, QString yAxisTitle);

	qreal calculateMaxValue(const QVector<QPointF>& series, bool isX);

	qreal calculateMinValue(const QVector<QPointF>& series, bool isX);

	void createChartDataGroup(QLineSeries*& lineSeries, QScatterSeries*& scatterSeries,
		const QString& name, const QColor& color);


};
