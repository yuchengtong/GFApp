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
	m_polarChart2->legend()->setVisible(false);
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
	m_polarChart3->legend()->setVisible(false);
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
	
	auto ins = ModelDataManager::GetInstance();
	auto pointResult = ins->GetPointResult();
	QStringList resultLabels;

	resultLabels << QString::number(pointResult.fallPoint);
	resultLabels << QString::number(pointResult.fastCombustionPoint);
	resultLabels << QString::number(pointResult.slowCombustionPoint);
	resultLabels << QString::number(pointResult.shootPoint);
	resultLabels << QString::number(pointResult.jetImpactPoint);
	resultLabels << QString::number(pointResult.fragmentationImpactPoint);
	resultLabels << QString::number(pointResult.explosiveBlastPoint);
	resultLabels << QString::number(pointResult.sacrificeExplosionPoint);

	QVector<QVector<double>> resultdatasets(4, QVector<double>(8, 0.0));
	resultdatasets[0] = { pointResult.fallPoint, pointResult.fastCombustionPoint, pointResult.slowCombustionPoint, pointResult.shootPoint,
		pointResult.jetImpactPoint, pointResult.fragmentationImpactPoint, pointResult.explosiveBlastPoint, pointResult.sacrificeExplosionPoint };
	resultdatasets[1] = resultdatasets[0];
	resultdatasets[2] = resultdatasets[0];
	resultdatasets[3] = resultdatasets[0];

	
	labelGroups[0] = resultLabels;
	m_resultchart = new CustomPolarChart(resultdatasets, labelGroups);
	m_resultchart->setTitle("评分结果");
	m_resultchart->setAnimationOptions(QChart::SeriesAnimations); // 添加动画效果
	m_resultchart->renameLegend(0, "评分结果");
	m_resultchart->setActiveDatasetVisible(0);
	m_resultchart->legend()->setVisible(false);
	// 将图表小部件添加到布局中
	QChartView *chartView_0 = new QChartView(ui.graphicsView_0);
	chartView_0->setChart(m_resultchart);
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


void AuxiliaryAnalysisWidget::updateAllData()
{
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

	auto ins = ModelDataManager::GetInstance();
	// 跌落计算结果
	StressResult m_FallStressResult = ins->GetFallStressResult();
	StrainResult m_FallStrainResult = ins->GetFallStrainResult();
	TemperatureResult m_FallTemperatureResult = ins->GetFallTemperatureResult();
	OverpressureResult m_FallOverpressureResult = ins->GetFallOverpressureResult();
	QVector<QVector<double>> datasets1(4, QVector<double>(8, 0.0));
	datasets1[0] = { m_FallStressResult.metalsMaxStress, m_FallStressResult.propellantsMaxStress, m_FallStressResult.outheatMaxStress, m_FallStressResult.insulatingheatMaxStress, 
		m_FallStressResult.metalsMinStress, m_FallStressResult.propellantsMinStress, m_FallStressResult.outheatMinStress, m_FallStressResult.insulatingheatMinStress, };
	datasets1[1] = { m_FallTemperatureResult.metalsMaxTemperature, m_FallTemperatureResult.propellantsMaxTemperature, m_FallTemperatureResult.outheatMaxTemperature, m_FallTemperatureResult.insulatingheatMaxTemperature,
	 m_FallTemperatureResult.metalsMinTemperature,  m_FallTemperatureResult.propellantsMinTemperature,  m_FallTemperatureResult.outheatMinTemperature,  m_FallTemperatureResult.insulatingheatMinTemperature, };
	datasets1[2] = { m_FallOverpressureResult.metalsMaxOverpressure, m_FallOverpressureResult.propellantsMaxOverpressure, m_FallOverpressureResult.outheatMaxOverpressure, m_FallOverpressureResult.insulatingheatMaxOverpressure,
	 m_FallOverpressureResult.metalsMinOverpressure,  m_FallOverpressureResult.propellantsMinOverpressure,  m_FallOverpressureResult.outheatMinOverpressure,  m_FallOverpressureResult.insulatingheatMinOverpressure, };
	datasets1[3] = { m_FallStrainResult.metalsMaxStrain, m_FallStrainResult.propellantsMaxStrain, m_FallStrainResult.outheatMaxStrain, m_FallStrainResult.insulatingheatMaxStrain,
	 m_FallStrainResult.metalsMinStrain,  m_FallStrainResult.propellantsMinStrain,  m_FallStrainResult.outheatMinStrain,  m_FallStrainResult.insulatingheatMinStrain, };
	// 放大对应的值，以免数值太小显示不明显
	for (double& val : datasets1[1]) {
		val = val * 8;
	}
	for (double& val : datasets1[3]) {
		if (val < 10)
		{
			val = val * 1000000;
		}
		else if (val > 1000)
		{
			val = val / 100000000000 * 0.8;
		}
		
	}
	m_polarChart1->updateDatasets(datasets1, labelGroups);

	

	// 快烤计算结果
	TemperatureResult m_FastCombustionTemperatureResult = ins->GetFastCombustionTemperatureResult();

	QVector<QVector<double>> datasets2(4, QVector<double>(8, 0.0));
	datasets2[0] = { m_FastCombustionTemperatureResult.metalsMaxTemperature, m_FastCombustionTemperatureResult.propellantsMaxTemperature, m_FastCombustionTemperatureResult.outheatMaxTemperature, m_FastCombustionTemperatureResult.insulatingheatMaxTemperature,
	m_FastCombustionTemperatureResult.metalsMinTemperature,  m_FastCombustionTemperatureResult.propellantsMinTemperature,  m_FastCombustionTemperatureResult.outheatMinTemperature,  m_FastCombustionTemperatureResult.insulatingheatMinTemperature, };
	datasets2[1] = datasets2[0];
	datasets2[2] = datasets2[0];
	datasets2[3] = datasets2[0];
	m_polarChart2->updateDatasets(datasets2, labelGroups);

	// 慢烤计算结果
	TemperatureResult m_SlowCombustionTemperatureResult = ins->GetSlowCombustionTemperatureResult();

	QVector<QVector<double>> datasets3(4, QVector<double>(8, 0.0));
	datasets3[0] = { m_SlowCombustionTemperatureResult.metalsMaxTemperature, m_SlowCombustionTemperatureResult.propellantsMaxTemperature, m_SlowCombustionTemperatureResult.outheatMaxTemperature, m_SlowCombustionTemperatureResult.insulatingheatMaxTemperature,
	m_SlowCombustionTemperatureResult.metalsMinTemperature,  m_SlowCombustionTemperatureResult.propellantsMinTemperature,  m_SlowCombustionTemperatureResult.outheatMinTemperature,  m_SlowCombustionTemperatureResult.insulatingheatMinTemperature, };
	datasets3[1] = datasets3[0];
	datasets3[2] = datasets3[0];
	datasets3[3] = datasets3[0];
	m_polarChart3->updateDatasets(datasets3, labelGroups);

	// 枪击计算结果
	StressResult m_ShootStressResult = ins->GetShootStressResult();
	StrainResult m_ShootStrainResult = ins->GetShootStrainResult();
	TemperatureResult m_ShootTemperatureResult = ins->GetShootTemperatureResult();
	OverpressureResult m_ShootOverpressureResult = ins->GetShootOverpressureResult();
	
	QVector<QVector<double>> datasets4(4, QVector<double>(8, 0.0));
	datasets4[0] = { m_ShootStressResult.metalsMaxStress, m_ShootStressResult.propellantsMaxStress, m_ShootStressResult.outheatMaxStress, m_ShootStressResult.insulatingheatMaxStress,
		m_ShootStressResult.metalsMinStress, m_ShootStressResult.propellantsMinStress, m_ShootStressResult.outheatMinStress, m_ShootStressResult.insulatingheatMinStress, };
	datasets4[1] = { m_ShootTemperatureResult.metalsMaxTemperature, m_ShootTemperatureResult.propellantsMaxTemperature, m_ShootTemperatureResult.outheatMaxTemperature, m_ShootTemperatureResult.insulatingheatMaxTemperature,
	 m_ShootTemperatureResult.metalsMinTemperature,  m_ShootTemperatureResult.propellantsMinTemperature,  m_ShootTemperatureResult.outheatMinTemperature,  m_ShootTemperatureResult.insulatingheatMinTemperature, };
	datasets4[2] = { m_ShootOverpressureResult.metalsMaxOverpressure, m_ShootOverpressureResult.propellantsMaxOverpressure, m_ShootOverpressureResult.outheatMaxOverpressure, m_ShootOverpressureResult.insulatingheatMaxOverpressure,
	 m_ShootOverpressureResult.metalsMinOverpressure,  m_ShootOverpressureResult.propellantsMinOverpressure,  m_ShootOverpressureResult.outheatMinOverpressure,  m_ShootOverpressureResult.insulatingheatMinOverpressure, };
	datasets4[3] = { m_ShootStrainResult.metalsMaxStrain, m_ShootStrainResult.propellantsMaxStrain, m_ShootStrainResult.outheatMaxStrain, m_ShootStrainResult.insulatingheatMaxStrain,
	 m_ShootStrainResult.metalsMinStrain,  m_ShootStrainResult.propellantsMinStrain,  m_ShootStrainResult.outheatMinStrain,  m_ShootStrainResult.insulatingheatMinStrain, };
	for (double& val : datasets4[1]) {
		val = val * 8;
	}
	for (double& val : datasets4[3]) {
		val = val * 1000000;
	}
	m_polarChart4->updateDatasets(datasets4, labelGroups);

	// 射流冲击计算结果
	StressResult m_JetImpactStressResult = ins->GetJetImpactStressResult();
	StrainResult m_JetImpactStrainResult = ins->GetJetImpactStrainResult();
	TemperatureResult m_JetImpactTemperatureResult = ins->GetJetImpactTemperatureResult();
	OverpressureResult m_JetImpactOverpressureResult = ins->GetJetImpactOverpressureResult();

	QVector<QVector<double>> datasets5(4, QVector<double>(8, 0.0));
	datasets5[0] = { m_JetImpactStressResult.metalsMaxStress, m_JetImpactStressResult.propellantsMaxStress, m_JetImpactStressResult.outheatMaxStress, m_JetImpactStressResult.insulatingheatMaxStress,
		m_JetImpactStressResult.metalsMinStress, m_JetImpactStressResult.propellantsMinStress, m_JetImpactStressResult.outheatMinStress, m_JetImpactStressResult.insulatingheatMinStress, };
	datasets5[1] = { m_JetImpactTemperatureResult.metalsMaxTemperature, m_JetImpactTemperatureResult.propellantsMaxTemperature, m_JetImpactTemperatureResult.outheatMaxTemperature, m_JetImpactTemperatureResult.insulatingheatMaxTemperature,
	 m_JetImpactTemperatureResult.metalsMinTemperature,  m_JetImpactTemperatureResult.propellantsMinTemperature,  m_JetImpactTemperatureResult.outheatMinTemperature,  m_JetImpactTemperatureResult.insulatingheatMinTemperature, };
	datasets5[2] = { m_JetImpactOverpressureResult.metalsMaxOverpressure, m_JetImpactOverpressureResult.propellantsMaxOverpressure, m_JetImpactOverpressureResult.outheatMaxOverpressure, m_JetImpactOverpressureResult.insulatingheatMaxOverpressure,
	 m_JetImpactOverpressureResult.metalsMinOverpressure,  m_JetImpactOverpressureResult.propellantsMinOverpressure,  m_JetImpactOverpressureResult.outheatMinOverpressure,  m_JetImpactOverpressureResult.insulatingheatMinOverpressure, };
	datasets5[3] = { m_JetImpactStrainResult.metalsMaxStrain, m_JetImpactStrainResult.propellantsMaxStrain, m_JetImpactStrainResult.outheatMaxStrain, m_JetImpactStrainResult.insulatingheatMaxStrain,
	 m_JetImpactStrainResult.metalsMinStrain,  m_JetImpactStrainResult.propellantsMinStrain,  m_JetImpactStrainResult.outheatMinStrain,  m_JetImpactStrainResult.insulatingheatMinStrain, };
	for (double& val : datasets5[1]) {
		val = val * 8;
	}
	for (double& val : datasets5[3]) {
		val = val * 1000000;
	}
	m_polarChart5->updateDatasets(datasets5, labelGroups);

	// 破片撞击计算结果
	StressResult m_FragmentationImpactStressResult = ins->GetFragmentationImpactStressResult();
	StrainResult m_FragmentationImpactStrainResult = ins->GetFragmentationImpactStrainResult();
	TemperatureResult m_FragmentationImpactTemperatureResult = ins->GetFragmentationImpactTemperatureResult();
	OverpressureResult m_FragmentationImpactOverpressureResult = ins->GetFragmentationImpactOverpressureResult();

	QVector<QVector<double>> datasets6(4, QVector<double>(8, 0.0));
	datasets6[0] = { m_FragmentationImpactStressResult.metalsMaxStress, m_FragmentationImpactStressResult.propellantsMaxStress, m_FragmentationImpactStressResult.outheatMaxStress, m_FragmentationImpactStressResult.insulatingheatMaxStress,
		m_FragmentationImpactStressResult.metalsMinStress, m_FragmentationImpactStressResult.propellantsMinStress, m_FragmentationImpactStressResult.outheatMinStress, m_FragmentationImpactStressResult.insulatingheatMinStress, };
	datasets6[1] = { m_FragmentationImpactTemperatureResult.metalsMaxTemperature, m_FragmentationImpactTemperatureResult.propellantsMaxTemperature, m_FragmentationImpactTemperatureResult.outheatMaxTemperature, m_FragmentationImpactTemperatureResult.insulatingheatMaxTemperature,
	 m_FragmentationImpactTemperatureResult.metalsMinTemperature,  m_FragmentationImpactTemperatureResult.propellantsMinTemperature,  m_FragmentationImpactTemperatureResult.outheatMinTemperature,  m_FragmentationImpactTemperatureResult.insulatingheatMinTemperature, };
	datasets6[2] = { m_FragmentationImpactOverpressureResult.metalsMaxOverpressure, m_FragmentationImpactOverpressureResult.propellantsMaxOverpressure, m_FragmentationImpactOverpressureResult.outheatMaxOverpressure, m_FragmentationImpactOverpressureResult.insulatingheatMaxOverpressure,
	 m_FragmentationImpactOverpressureResult.metalsMinOverpressure,  m_FragmentationImpactOverpressureResult.propellantsMinOverpressure,  m_FragmentationImpactOverpressureResult.outheatMinOverpressure,  m_FragmentationImpactOverpressureResult.insulatingheatMinOverpressure, };
	datasets6[3] = { m_FragmentationImpactStrainResult.metalsMaxStrain, m_FragmentationImpactStrainResult.propellantsMaxStrain, m_FragmentationImpactStrainResult.outheatMaxStrain, m_FragmentationImpactStrainResult.insulatingheatMaxStrain,
	 m_FragmentationImpactStrainResult.metalsMinStrain,  m_FragmentationImpactStrainResult.propellantsMinStrain,  m_FragmentationImpactStrainResult.outheatMinStrain,  m_FragmentationImpactStrainResult.insulatingheatMinStrain, };
	for (double& val : datasets6[1]) {
		val = val * 8;
	}
	for (double& val : datasets6[3]) {
		val = val * 1000000;
	}
	m_polarChart6->updateDatasets(datasets6, labelGroups);

	// 爆炸冲击波计算结果
	StressResult m_ExplosiveBlastStressResult = ins->GetExplosiveBlastStressResult();
	StrainResult m_ExplosiveBlastStrainResult = ins->GetExplosiveBlastStrainResult();
	TemperatureResult m_ExplosiveBlastTemperatureResult = ins->GetExplosiveBlastTemperatureResult();
	OverpressureResult m_ExplosiveBlastOverpressureResult = ins->GetExplosiveBlastOverpressureResult();

	QVector<QVector<double>> datasets7(4, QVector<double>(8, 0.0));
	datasets7[0] = { m_ExplosiveBlastStressResult.metalsMaxStress, m_ExplosiveBlastStressResult.propellantsMaxStress, m_ExplosiveBlastStressResult.outheatMaxStress, m_ExplosiveBlastStressResult.insulatingheatMaxStress,
		m_ExplosiveBlastStressResult.metalsMinStress, m_ExplosiveBlastStressResult.propellantsMinStress, m_ExplosiveBlastStressResult.outheatMinStress, m_ExplosiveBlastStressResult.insulatingheatMinStress, };
	datasets7[1] = { m_ExplosiveBlastTemperatureResult.metalsMaxTemperature, m_ExplosiveBlastTemperatureResult.propellantsMaxTemperature, m_ExplosiveBlastTemperatureResult.outheatMaxTemperature, m_ExplosiveBlastTemperatureResult.insulatingheatMaxTemperature,
	 m_ExplosiveBlastTemperatureResult.metalsMinTemperature,  m_ExplosiveBlastTemperatureResult.propellantsMinTemperature,  m_ExplosiveBlastTemperatureResult.outheatMinTemperature,  m_ExplosiveBlastTemperatureResult.insulatingheatMinTemperature, };
	datasets7[2] = { m_ExplosiveBlastOverpressureResult.metalsMaxOverpressure, m_ExplosiveBlastOverpressureResult.propellantsMaxOverpressure, m_ExplosiveBlastOverpressureResult.outheatMaxOverpressure, m_ExplosiveBlastOverpressureResult.insulatingheatMaxOverpressure,
	 m_ExplosiveBlastOverpressureResult.metalsMinOverpressure,  m_ExplosiveBlastOverpressureResult.propellantsMinOverpressure,  m_ExplosiveBlastOverpressureResult.outheatMinOverpressure,  m_ExplosiveBlastOverpressureResult.insulatingheatMinOverpressure, };
	datasets7[3] = { m_ExplosiveBlastStrainResult.metalsMaxStrain, m_ExplosiveBlastStrainResult.propellantsMaxStrain, m_ExplosiveBlastStrainResult.outheatMaxStrain, m_ExplosiveBlastStrainResult.insulatingheatMaxStrain,
	 m_ExplosiveBlastStrainResult.metalsMinStrain,  m_ExplosiveBlastStrainResult.propellantsMinStrain,  m_ExplosiveBlastStrainResult.outheatMinStrain,  m_ExplosiveBlastStrainResult.insulatingheatMinStrain, };
	for (double& val : datasets7[1]) {
		val = val * 8;
	}
	for (double& val : datasets7[3]) {
		val = val * 1000000;
	}
	m_polarChart7->updateDatasets(datasets7, labelGroups);

	// 殉爆计算结果
	StressResult m_SacrificeExplosionStressResult = ins->GetSacrificeExplosionStressResult();
	StrainResult m_SacrificeExplosionStrainResult = ins->GetSacrificeExplosionStrainResult();
	TemperatureResult m_SacrificeExplosionTemperatureResult = ins->GetSacrificeExplosionTemperatureResult();
	OverpressureResult m_SacrificeExplosionOverpressureResult = ins->GetSacrificeExplosionOverpressureResult();

	QVector<QVector<double>> datasets8(4, QVector<double>(8, 0.0));
	datasets8[0] = { m_SacrificeExplosionStressResult.metalsMaxStress, m_SacrificeExplosionStressResult.propellantsMaxStress, m_SacrificeExplosionStressResult.outheatMaxStress, m_SacrificeExplosionStressResult.insulatingheatMaxStress,
		m_SacrificeExplosionStressResult.metalsMinStress, m_SacrificeExplosionStressResult.propellantsMinStress, m_SacrificeExplosionStressResult.outheatMinStress, m_SacrificeExplosionStressResult.insulatingheatMinStress, };
	datasets8[1] = { m_SacrificeExplosionTemperatureResult.metalsMaxTemperature, m_SacrificeExplosionTemperatureResult.propellantsMaxTemperature, m_SacrificeExplosionTemperatureResult.outheatMaxTemperature, m_SacrificeExplosionTemperatureResult.insulatingheatMaxTemperature,
	 m_SacrificeExplosionTemperatureResult.metalsMinTemperature,  m_SacrificeExplosionTemperatureResult.propellantsMinTemperature,  m_SacrificeExplosionTemperatureResult.outheatMinTemperature,  m_SacrificeExplosionTemperatureResult.insulatingheatMinTemperature, };
	datasets8[2] = { m_SacrificeExplosionOverpressureResult.metalsMaxOverpressure, m_SacrificeExplosionOverpressureResult.propellantsMaxOverpressure, m_SacrificeExplosionOverpressureResult.outheatMaxOverpressure, m_SacrificeExplosionOverpressureResult.insulatingheatMaxOverpressure,
	 m_SacrificeExplosionOverpressureResult.metalsMinOverpressure,  m_SacrificeExplosionOverpressureResult.propellantsMinOverpressure,  m_SacrificeExplosionOverpressureResult.outheatMinOverpressure,  m_SacrificeExplosionOverpressureResult.insulatingheatMinOverpressure, };
	datasets8[3] = { m_SacrificeExplosionStrainResult.metalsMaxStrain, m_SacrificeExplosionStrainResult.propellantsMaxStrain, m_SacrificeExplosionStrainResult.outheatMaxStrain, m_SacrificeExplosionStrainResult.insulatingheatMaxStrain,
	 m_SacrificeExplosionStrainResult.metalsMinStrain,  m_SacrificeExplosionStrainResult.propellantsMinStrain,  m_SacrificeExplosionStrainResult.outheatMinStrain,  m_SacrificeExplosionStrainResult.insulatingheatMinStrain, };
	for (double& val : datasets8[1]) {
		val = val * 8;
	}
	for (double& val : datasets8[3]) {
		val = val * 1000000;
	}
	m_polarChart8->updateDatasets(datasets8, labelGroups);


	// 评分结果
	auto pointResult = ins->GetPointResult();
	QStringList resultLabels;

	resultLabels << " " + QString::number(pointResult.fallPoint);
	resultLabels << "  " + QString::number(pointResult.fastCombustionPoint);
	resultLabels << "   " + QString::number(pointResult.slowCombustionPoint);
	resultLabels << "    " + QString::number(pointResult.shootPoint);
	resultLabels << QString::number(pointResult.jetImpactPoint) + " ";
	resultLabels << QString::number(pointResult.fragmentationImpactPoint) + "  ";
	resultLabels << QString::number(pointResult.explosiveBlastPoint) + "   ";
	resultLabels << QString::number(pointResult.sacrificeExplosionPoint) + "    ";

	QVector<QStringList> resultLabelGroups;
	resultLabelGroups.append(resultLabels);

	QVector<QVector<double>> resultdatasets(4, QVector<double>(8, 0.0));
	resultdatasets[0] = { pointResult.fallPoint, pointResult.fastCombustionPoint, pointResult.slowCombustionPoint, pointResult.shootPoint,
		pointResult.jetImpactPoint, pointResult.fragmentationImpactPoint, pointResult.explosiveBlastPoint, pointResult.sacrificeExplosionPoint };
	resultdatasets[1] = resultdatasets[0];
	resultdatasets[2] = resultdatasets[0];
	resultdatasets[3] = resultdatasets[0];

	m_resultchart->updateDatasets(resultdatasets, resultLabelGroups);



	
}
