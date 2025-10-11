#pragma execution_character_set("utf-8")
#include "AuxiliaryAnalysisWidget.h"
#include <QtCharts>
#include <QLineSeries>
#include <QBarSeries>
#include <QtCharts\qchartview.h>
#include <QLinearGradient>
#include <QPen>
#include <QBrush>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QLegend>


AuxiliaryAnalysisWidget::AuxiliaryAnalysisWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	// 图形
	ui.gridLayout->setContentsMargins(0, 0, 0, 0);

	// 创建一个渐变对象
	QLinearGradient gradient(0, 0, 1, 1); // 参数为起点和终点坐标，(0,0)是左下角，(1,1)是右上角
	gradient.setColorAt(0, QColor(45, 86, 126)); // 设置起点颜色为蓝色
	gradient.setColorAt(1, QColor(159, 160, 162)); // 设置终点颜色为红色

	const qreal angularMin = 0;
	const qreal angularMax = 360;
	const qreal radialMin = 0;
	const qreal radialMax = 360;


	qreal ad6 = (angularMax - angularMin) / 8;

	// 设置标签
	QStringList labels = {
		"发动机壳体最大应力", "固体推进剂最大应力", "隔绝热最大应力", "外防热最大应力",
		"发动机壳体最小应力", "固体推进剂最小应力", "固体推进剂最小应力", "隔绝热最小应力"
	};

	// 设置区域颜色和透明度
	QColor stressColor(255, 0, 0, 50); // 红色
	QColor tempColor(0, 255, 0, 50);   // 绿色
	QColor pressureColor(0, 0, 255, 50); // 蓝色
	QColor strainColor(255, 255, 0, 50); // 黄色

	QVector<QVector<double>> datasets(4, QVector<double>(8, 0.0));
	datasets[0] = { 70, 80, 65, 90, 55, 60, 75, 85 };
	datasets[1] = { 40, 42, 38, 45, 36, 39, 41, 43 };
	datasets[2] = { 60, 65, 62, 68, 59, 61, 64, 63 };
	datasets[3] = { 88, 82, 90, 78, 70, 60, 79, 69 };

	// 四组标签
	QVector<QStringList> labelGroups(4);

	QStringList stressLabels;
	stressLabels << "发动机壳体最大应力" << "固体推进剂最大应力" << "隔绝热最大应力" << "外防热最大应力"
		<< "发动机壳体最小应力" << "固体推进剂最小应力" << "隔绝热最小应力" << "外防热最小应力";

	QStringList tempLabels;
	tempLabels << "发动机壳体最大温度" << "固体推进剂最大温度" << "隔绝热最大温度" << "外防热最大温度"
		<< "发动机壳体最小温度" << "固体推进剂最小温度" << "隔绝热最小温度" << "外防热最小温度";

	QStringList pressureLabels;
	pressureLabels << "发动机壳体最大超压" << "固体推进剂最大超压" << "隔绝热最大超压" << "外防热最大超压"
		<< "发动机壳体最小超压" << "固体推进剂最小超压" << "隔绝热最小超压" << "外防热最小超压";

	QStringList strainLabels;
	strainLabels << "发动机壳体最大应变" << "固体推进剂最大应变" << "隔绝热最大应变" << "外防热最大应变"
		<< "发动机壳体最小应变" << "固体推进剂最小应变" << "隔绝热最小应变" << "外防热最小应变";

	labelGroups[0] = stressLabels;
	labelGroups[1] = tempLabels;
	labelGroups[2] = pressureLabels;
	labelGroups[3] = strainLabels;

	m_polarChart1 = new CustomPolarChart(datasets, labelGroups);
	m_polarChart1->setTitle("跌落试验");
	m_polarChart1->setAnimationOptions(QChart::SeriesAnimations); // 添加动画效果
	QChartView *chartView_1 = new QChartView(ui.graphicsView_1);
	QBrush transparentBrush(Qt::transparent);
	chartView_1->setBackgroundBrush(transparentBrush);
	chartView_1->setAutoFillBackground(true);
	chartView_1->setStyleSheet("background-image: url(:/src/chart_view.png);background-repeat: no-repeat; background-position: center; background-size: cover;");
	chartView_1->setChart(m_polarChart1);
	chartView_1->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_1 = new QHBoxLayout();
	hLayout_1->addWidget(chartView_1);
	hLayout_1->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_1->setLayout(hLayout_1);

	
	m_polarChart2 = new CustomPolarChart(datasets, labelGroups);
	m_polarChart2->setTitle("快速烤燃试验");
	m_polarChart2->setAnimationOptions(QChart::SeriesAnimations); // 添加动画效果
	m_polarChart2->setActiveDatasetVisible(1);
	QChartView *chartView_2 = new QChartView(ui.graphicsView_2);
	chartView_2->setBackgroundBrush(transparentBrush);
	chartView_2->setAutoFillBackground(true);
	chartView_2->setStyleSheet("background-image: url(:/src/chart_view.png);background-repeat: no-repeat; background-position: center; background-size: cover;");
	chartView_2->setChart(m_polarChart2);
	chartView_2->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_2 = new QHBoxLayout();
	hLayout_2->addWidget(chartView_2);
	hLayout_2->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_2->setLayout(hLayout_2);


	m_polarChart3 = new CustomPolarChart(datasets, labelGroups);
	m_polarChart3->setTitle("慢速烤燃试验");
	m_polarChart3->setAnimationOptions(QChart::SeriesAnimations); // 添加动画效果
	m_polarChart3->setActiveDatasetVisible(1);
	QChartView *chartView_3 = new QChartView(ui.graphicsView_3);
	chartView_3->setBackgroundBrush(transparentBrush);
	chartView_3->setAutoFillBackground(true);
	chartView_3->setStyleSheet("background-image: url(:/src/chart_view.png);background-repeat: no-repeat; background-position: center; background-size: cover;");
	chartView_3->setChart(m_polarChart3);
	chartView_3->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_3 = new QHBoxLayout();
	hLayout_3->addWidget(chartView_3);
	hLayout_3->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_3->setLayout(hLayout_3);


	// 创建雷达图
	m_polarChart4 = new CustomPolarChart(datasets, labelGroups);
	m_polarChart4->setTitle("枪击试验");
	m_polarChart4->setAnimationOptions(QChart::SeriesAnimations); // 添加动画效果
	QChartView *chartView_4 = new QChartView(ui.graphicsView_4);
	chartView_4->setBackgroundBrush(transparentBrush);
	chartView_4->setAutoFillBackground(true);
	chartView_4->setStyleSheet("background-image: url(:/src/chart_view.png);background-repeat: no-repeat; background-position: center; background-size: cover;");
	chartView_4->setChart(m_polarChart4);
	chartView_4->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_4 = new QHBoxLayout();
	hLayout_4->addWidget(chartView_4);
	hLayout_4->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_4->setLayout(hLayout_4);

	// 创建雷达图
	m_polarChart5 = new CustomPolarChart(datasets, labelGroups);
	m_polarChart5->setTitle("射流冲击试验");
	m_polarChart5->setAnimationOptions(QChart::SeriesAnimations); // 添加动画效果
	QChartView *chartView_5 = new QChartView(ui.graphicsView_5);
	chartView_5->setBackgroundBrush(transparentBrush);
	chartView_5->setAutoFillBackground(true);
	chartView_5->setStyleSheet("background-image: url(:/src/chart_view.png);background-repeat: no-repeat; background-position: center; background-size: cover;");
	chartView_5->setChart(m_polarChart5);
	chartView_5->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_5 = new QHBoxLayout();
	hLayout_5->addWidget(chartView_5);
	hLayout_5->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_5->setLayout(hLayout_5);

	
	// 创建雷达图
	m_polarChart6 = new CustomPolarChart(datasets, labelGroups);
	m_polarChart6->setTitle("破片撞击试验");
	m_polarChart6->setAnimationOptions(QChart::SeriesAnimations); // 添加动画效果
	QChartView *chartView_6 = new QChartView(ui.graphicsView_6);
	chartView_6->setBackgroundBrush(transparentBrush);
	chartView_6->setAutoFillBackground(true);
	chartView_6->setStyleSheet("background-image: url(:/src/chart_view.png);background-repeat: no-repeat; background-position: center; background-size: cover;");
	chartView_6->setChart(m_polarChart6);
	chartView_6->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_6 = new QHBoxLayout();
	hLayout_6->addWidget(chartView_6);
	hLayout_6->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_6->setLayout(hLayout_6);

	
	// 创建雷达图
	m_polarChart7 = new CustomPolarChart(datasets, labelGroups);
	m_polarChart7->setTitle("爆炸冲击波试验");
	m_polarChart7->setAnimationOptions(QChart::SeriesAnimations); // 添加动画效果
	QChartView *chartView_7 = new QChartView(ui.graphicsView_7);
	chartView_7->setBackgroundBrush(transparentBrush);
	chartView_7->setAutoFillBackground(true);
	chartView_7->setStyleSheet("background-image: url(:/src/chart_view.png);background-repeat: no-repeat; background-position: center; background-size: cover;");
	chartView_7->setChart(m_polarChart7);
	chartView_7->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_7 = new QHBoxLayout();
	hLayout_7->addWidget(chartView_7);
	hLayout_7->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_7->setLayout(hLayout_7);

	// 创建雷达图
	m_polarChart8 = new CustomPolarChart(datasets, labelGroups);
	m_polarChart8->setTitle("殉爆试验");
	m_polarChart8->setAnimationOptions(QChart::SeriesAnimations); // 添加动画效果
	QChartView *chartView_8 = new QChartView(ui.graphicsView_8);
	chartView_8->setBackgroundBrush(transparentBrush);
	chartView_8->setAutoFillBackground(true);
	chartView_8->setStyleSheet("background-image: url(:/src/chart_view.png);background-repeat: no-repeat; background-position: center; background-size: cover;");
	chartView_8->setChart(m_polarChart8);
	chartView_8->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_8 = new QHBoxLayout();
	hLayout_8->addWidget(chartView_8);
	hLayout_8->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_8->setLayout(hLayout_8);


	/////////////////////// 中间比例图
	

	QStringList resultLabels;
	resultLabels << "89" << "97" << "70" << "83" << "93" << "92" << "85" << "88";
	labelGroups[0] = resultLabels;
	CustomPolarChart* chart = new CustomPolarChart(datasets, labelGroups);
	chart->setTitle("评分结果");
	chart->setAnimationOptions(QChart::SeriesAnimations); // 添加动画效果
	chart->renameLegend(0, "评分结果");
	chart->setActiveDatasetVisible(0);
	// 将图表小部件添加到布局中
	QChartView *chartView_0 = new QChartView(ui.graphicsView_0);
	chartView_0->setChart(chart);
	chartView_0->setRenderHint(QPainter::Antialiasing);
	chartView_0->setBackgroundBrush(transparentBrush);
	chartView_0->setAutoFillBackground(true);
	chartView_0->setStyleSheet("background-image: url(:/src/chart_view.png);background-repeat: no-repeat; background-position: center; background-size: cover;");

	QHBoxLayout*hLayout_0 = new QHBoxLayout();
	hLayout_0->addWidget(chartView_0);
	hLayout_0->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_0->setLayout(hLayout_0);

}

AuxiliaryAnalysisWidget::~AuxiliaryAnalysisWidget()
{}

QMap<int, QWidget*> AuxiliaryAnalysisWidget::getChartWidget()
{
	chartWidMap[0] = ui.widget_0;
	chartWidMap[1] = ui.widget_1;
	chartWidMap[2] = ui.widget_2;
	chartWidMap[3] = ui.widget_3;
	chartWidMap[4] = ui.widget_4;
	chartWidMap[5] = ui.widget_5;
	chartWidMap[6] = ui.widget_6;
	chartWidMap[7] = ui.widget_7;
	chartWidMap[8] = ui.widget_8;


	return chartWidMap;
}
