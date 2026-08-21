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

	GraphicWidget* get3dGraphicWid() { return m_3dGraphicWid; }



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
	QSplineSeries* getQuadSeries1() { return m_quadSeries1; }
	QSplineSeries* getQuadSeries2() { return m_quadSeries2; }
	QSplineSeries* getQuadSeries3() { return m_quadSeries3; }

	// 更新三维图形数据
	void updateGraphicData(QString xName, QString yName, QString zName,
		const QVector<double>& xCoords,
		const QVector<double>& yCoords,
		const QVector<QVector<double>>& newData,
		double xMin,
		double xMax,
		double yMin,
		double yMax);

	// 更新图表数据
	void updateChartData(QVector<QPointF> data1, QVector<QPointF> data2, QVector<QPointF> data3, QString xAxisTitle, QString yAxisTitle);

	qreal calculateMaxValue(const QVector<QPointF>& series, bool isX);

	qreal calculateMinValue(const QVector<QPointF>& series, bool isX);

	void createChartDataGroup(QSplineSeries*& lineSeries, QScatterSeries*& scatterSeries, QSplineSeries*& quadSeries,
		const QString& name, const QColor& color);

	// 贝塞尔平滑
	QVector<QPointF> bezierSmooth(const QVector<QPointF>& src, int subdiv = 8);


private slots:
	void onTreeItemClicked(const QString& itemData);

	void onComboBoxIndexChanged(int index);

	void onComboBoxIndexGraphicChanged(int index);

	void dataChange(int index);

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

	QComboBox* x_comboBox = nullptr;
	QComboBox* y_comboBox = nullptr;


	QChartView* m_chartView = nullptr;
	QChart* m_chart = nullptr;              // 图表对象
	// 每组数据：1条曲线 + 1组圆点（需同步修改）
	QSplineSeries* m_lineSeries1 = nullptr;   // 第一条曲线
	QScatterSeries* m_scatter1 = nullptr;   // 第一组圆点
	QSplineSeries* m_quadSeries1 = nullptr;    // 第一组二次拟合曲线
	QSplineSeries* m_lineSeries2 = nullptr;   // 第二条曲线
	QScatterSeries* m_scatter2 = nullptr;   // 第二组圆点
	QSplineSeries* m_quadSeries2 = nullptr;    // 第二组二次拟合曲线
	QSplineSeries* m_lineSeries3 = nullptr;   // 第三条曲线
	QScatterSeries* m_scatter3 = nullptr;   // 第三组圆点
	QSplineSeries* m_quadSeries3 = nullptr;    // 第三组二次拟合曲线
	QValueAxis* m_axisX = nullptr;          // X轴
	QValueAxis* m_axisY = nullptr;          // Y轴


	QChartView* chartView2 = nullptr;

	QComboBox* m_grapgicComboBox = nullptr;
	QLineEdit* m_r = nullptr;

	GraphicWidget* m_3dGraphicWid = nullptr;

	QHBoxLayout* graphicLayout = nullptr;
	QVBoxLayout* m_leftLayout = nullptr;
	// 自定义图例（2行：实线一次拟合 / 虚线二次拟合）
	QWidget* m_customLegend = nullptr;
	QLabel* m_linLabel1 = nullptr;
	QLabel* m_linLabel2 = nullptr;
	QLabel* m_linLabel3 = nullptr;
	QLabel* m_quadLabel1 = nullptr;
	QLabel* m_quadLabel2 = nullptr;
	QLabel* m_quadLabel3 = nullptr;
	QString m_baseName1; // 图例名称
	QString m_baseName2;
	QString m_baseName3;
	QWidget* createLegendItem(Qt::PenStyle style, const QColor& color, QLabel* label);

	QMap<QString, QStringList> m_dataMap;

};