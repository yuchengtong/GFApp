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

	// tableWidge随窗口大小变化
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	// 树默认展开
	ui.m_AuxiliaryAnalysisTtreeWid->expandAll();



	// 连接信号槽以处理复选框点击事件
	connect(ui.m_AuxiliaryAnalysisTtreeWid, &QTreeWidget::itemChanged, [](QTreeWidgetItem* item, int column) {
		if (item->childCount() > 0) {
			// 如果父节点被选中，则选中所有子节点
			if (item->checkState(0) == Qt::Checked) {
				for (int i = 0; i < item->childCount(); ++i) {
					item->child(i)->setCheckState(0, Qt::Checked);
				}
			}
			// 如果父节点被取消选中，则取消选中所有子节点
			else if (item->checkState(0) == Qt::Unchecked) {
				for (int i = 0; i < item->childCount(); ++i) {
					item->child(i)->setCheckState(0, Qt::Unchecked);
				}
			}
		}
	});


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

	// 创建雷达图
	m_polarChart1 = new QPolarChart();
	m_polarChart1->setTitle("跌落实验");
	m_polarChart1->setTitleBrush(Qt::white);

	// 创建四个系列
	m_stressSeries1 = new QLineSeries();
	m_temperatureSeries1 = new QLineSeries();
	m_overpressureSeries1 = new QLineSeries();
	m_strainSeries1 = new QLineSeries();

	// 设置系列名称
	m_stressSeries1->setName("应力");
	m_temperatureSeries1->setName("温度");
	m_overpressureSeries1->setName("超压");
	m_strainSeries1->setName("应变");

	// 生成数据
	QVector<QPointF> stressData1 = generateData(1);
	QVector<QPointF> tempData1 = generateData(2);
	QVector<QPointF> pressureData1 = generateData(3);
	QVector<QPointF> strainData1 = generateData(4);

	// 为系列添加数据（不使用<<运算符）
	for (const auto &point : stressData1) {
		m_stressSeries1->append(point);
	}
	for (const auto &point : tempData1) {
		m_temperatureSeries1->append(point);
	}
	for (const auto &point : pressureData1) {
		m_overpressureSeries1->append(point);
	}
	for (const auto &point : strainData1) {
		m_strainSeries1->append(point);
	}

	// 创建角度轴（8个数据点）
	angularAxis1 = new QCategoryAxis();
	angularAxis1->setMin(0);
	angularAxis1->setMax(360);
	angularAxis1->setLabelsColor(Qt::white);

	// 设置标签
	QStringList labels = {
		"发动机壳体最大应力", "固体推进剂最大应力", "隔绝热最大应力", "外防热最大应力",
		"发动机壳体最小应力", "固体推进剂最小应力", "固体推进剂最小应力", "隔绝热最小应力"
	};

	for (int i = 0; i < 8; ++i) {
		// 平均分布在360度上
		qreal angle = 360.0 / 8 * i;
		angularAxis1->append(labels[i], angle);
	}

	// 创建径向轴
	QValueAxis *radialAxis1 = new QValueAxis();
	radialAxis1->setMin(0);
	radialAxis1->setMax(100);
	radialAxis1->setTickCount(8);
	radialAxis1->setLabelFormat("%d");
	radialAxis1->setLabelsColor(Qt::white);

	// 添加轴到图表
	m_polarChart1->addAxis(angularAxis1, QPolarChart::PolarOrientationAngular);
	m_polarChart1->addAxis(radialAxis1, QPolarChart::PolarOrientationRadial);

	// 创建区域系列用于显示阴影
	m_stressArea1 = new QAreaSeries(m_stressSeries1);
	m_temperatureArea1 = new QAreaSeries(m_temperatureSeries1);
	m_overpressureArea1 = new QAreaSeries(m_overpressureSeries1);
	m_strainArea1 = new QAreaSeries(m_strainSeries1);

	m_stressArea1->setName("应力");
	m_temperatureArea1->setName("温度");
	m_overpressureArea1->setName("超压");
	m_strainArea1->setName("应变");

	// 设置区域颜色和透明度
	QColor stressColor(255, 0, 0, 50); // 红色
	QColor tempColor(0, 255, 0, 50);   // 绿色
	QColor pressureColor(0, 0, 255, 50); // 蓝色
	QColor strainColor(255, 255, 0, 50); // 黄色

	m_stressArea1->setBrush(stressColor);
	m_stressArea1->setPen(QPen(stressColor, 2));

	m_temperatureArea1->setBrush(tempColor);
	m_temperatureArea1->setPen(QPen(tempColor, 2));

	m_overpressureArea1->setBrush(pressureColor);
	m_overpressureArea1->setPen(QPen(pressureColor, 2));

	m_strainArea1->setBrush(strainColor);
	m_strainArea1->setPen(QPen(strainColor, 2));

	// 添加所有区域到图表
	m_polarChart1->addSeries(m_stressArea1);
	m_polarChart1->addSeries(m_temperatureArea1);
	m_polarChart1->addSeries(m_overpressureArea1);
	m_polarChart1->addSeries(m_strainArea1);

	// 关联系列与轴
	m_stressArea1->attachAxis(angularAxis1);
	m_stressArea1->attachAxis(radialAxis1);

	m_temperatureArea1->attachAxis(angularAxis1);
	m_temperatureArea1->attachAxis(radialAxis1);

	m_overpressureArea1->attachAxis(angularAxis1);
	m_overpressureArea1->attachAxis(radialAxis1);

	m_strainArea1->attachAxis(angularAxis1);
	m_strainArea1->attachAxis(radialAxis1);

	// 配置图例
	m_polarChart1->legend()->setVisible(true);
	m_polarChart1->legend()->setAlignment(Qt::AlignRight);
	m_polarChart1->legend()->setLabelColor(Qt::white);



	// 允许图例点击
	foreach(QLegendMarker* marker, m_polarChart1->legend()->markers()) {
		marker->setVisible(true);
		connect(marker, &QLegendMarker::clicked, this, &AuxiliaryAnalysisWidget::handleLegendClick1);
	}

	// 默认显示应力数据，其他透明
	m_stressArea1->setOpacity(1.0);
	m_temperatureArea1->setOpacity(0.0);
	m_overpressureArea1->setOpacity(0.0);
	m_strainArea1->setOpacity(0.0);

	m_polarChart1->setBackgroundBrush(QBrush(gradient));
	// 隐藏极坐标图表（如雷达图）中的径向轴线
	QList<QAbstractAxis*> radialAxes = m_polarChart1->axes(QPolarChart::PolarOrientationRadial);
	foreach(QAbstractAxis* axis, radialAxes) {
		axis->setVisible(false);
	}




	QChartView *chartView_1 = new QChartView(ui.graphicsView_1);
	chartView_1->setChart(m_polarChart1);
	chartView_1->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_1 = new QHBoxLayout();
	hLayout_1->addWidget(chartView_1);
	hLayout_1->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_1->setLayout(hLayout_1);




	
	// 创建雷达图
	m_polarChart2 = new QPolarChart();
	m_polarChart2->setTitle("快速烤燃试验");
	m_polarChart2->setTitleBrush(Qt::white);

	// 创建四个系列
	m_temperatureSeries2 = new QLineSeries();
	
	// 设置系列名称
	m_temperatureSeries2->setName("温度");
	
	// 生成数据
	QVector<QPointF> tempData2 = generateData(2);
	
	for (const auto &point : tempData2) {
		m_temperatureSeries2->append(point);
	}
	
	// 创建角度轴（8个数据点）
	angularAxis2 = new QCategoryAxis();
	angularAxis2->setMin(0);
	angularAxis2->setMax(360);
	angularAxis2->setLabelsColor(Qt::white);


	for (int i = 0; i < 8; ++i) {
		// 平均分布在360度上
		qreal angle = 360.0 / 8 * i;
		angularAxis2->append(labels[i], angle);
	}

	// 创建径向轴
	QValueAxis *radialAxis2 = new QValueAxis();
	radialAxis2->setMin(0);
	radialAxis2->setMax(100);
	radialAxis2->setTickCount(8);
	radialAxis2->setLabelFormat("%d");
	radialAxis2->setLabelsColor(Qt::white);

	// 添加轴到图表
	m_polarChart2->addAxis(angularAxis2, QPolarChart::PolarOrientationAngular);
	m_polarChart2->addAxis(radialAxis2, QPolarChart::PolarOrientationRadial);

	// 创建区域系列用于显示阴影
	m_temperatureArea2 = new QAreaSeries(m_temperatureSeries2);
	m_temperatureArea2->setName("温度");

	m_temperatureArea2->setBrush(tempColor);
	m_temperatureArea2->setPen(QPen(tempColor, 2));

	// 添加所有区域到图表
	m_polarChart2->addSeries(m_temperatureArea2);

	// 关联系列与轴
	m_temperatureArea2->attachAxis(angularAxis2);
	m_temperatureArea2->attachAxis(radialAxis2);


	// 配置图例
	m_polarChart2->legend()->setVisible(true);
	m_polarChart2->legend()->setAlignment(Qt::AlignRight);
	m_polarChart2->legend()->setLabelColor(Qt::white);

	// 默认显示应力数据，其他透明
	m_temperatureArea2->setOpacity(1.0);

	m_polarChart2->setBackgroundBrush(QBrush(gradient));
	// 隐藏极坐标图表（如雷达图）中的径向轴线
	foreach(QAbstractAxis* axis, m_polarChart2->axes(QPolarChart::PolarOrientationRadial)) {
		axis->setVisible(false);
	}


	QChartView *chartView_2 = new QChartView(ui.graphicsView_2);
	chartView_2->setChart(m_polarChart2);
	chartView_2->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_2 = new QHBoxLayout();
	hLayout_2->addWidget(chartView_2);
	hLayout_2->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_2->setLayout(hLayout_2);


	


	
	// 创建雷达图
	m_polarChart3 = new QPolarChart();
	m_polarChart3->setTitle("慢速烤燃试验");
	m_polarChart3->setTitleBrush(Qt::white);

	// 创建四个系列
	m_temperatureSeries3 = new QLineSeries();

	// 设置系列名称
	m_temperatureSeries3->setName("温度");

	// 生成数据
	QVector<QPointF> tempData3 = generateData(2);

	// 为系列添加数据（不使用<<运算符）
	for (const auto &point : tempData3) {
		m_temperatureSeries3->append(point);
	}
	

	// 创建角度轴（8个数据点）
	angularAxis3 = new QCategoryAxis();
	angularAxis3->setMin(0);
	angularAxis3->setMax(360);
	angularAxis3->setLabelsColor(Qt::white);



	for (int i = 0; i < 8; ++i) {
		// 平均分布在360度上
		qreal angle = 360.0 / 8 * i;
		angularAxis3->append(labels[i], angle);
	}

	// 创建径向轴
	QValueAxis *radialAxis3 = new QValueAxis();
	radialAxis3->setMin(0);
	radialAxis3->setMax(100);
	radialAxis3->setTickCount(8);
	radialAxis3->setLabelFormat("%d");
	radialAxis3->setLabelsColor(Qt::white);

	// 添加轴到图表
	m_polarChart3->addAxis(angularAxis3, QPolarChart::PolarOrientationAngular);
	m_polarChart3->addAxis(radialAxis3, QPolarChart::PolarOrientationRadial);

	// 创建区域系列用于显示阴影
	m_temperatureArea3 = new QAreaSeries(m_temperatureSeries3);

	m_temperatureArea3->setName("温度");

	m_temperatureArea3->setBrush(tempColor);
	m_temperatureArea3->setPen(QPen(tempColor, 2));

	// 添加所有区域到图表
	m_polarChart3->addSeries(m_temperatureArea3);

	// 关联系列与轴
	m_temperatureArea3->attachAxis(angularAxis3);
	m_temperatureArea3->attachAxis(radialAxis3);

	// 配置图例
	m_polarChart3->legend()->setVisible(true);
	m_polarChart3->legend()->setAlignment(Qt::AlignRight);
	m_polarChart3->legend()->setLabelColor(Qt::white);


	// 默认显示应力数据，其他透明
	m_temperatureArea3->setOpacity(1.0);

	m_polarChart3->setBackgroundBrush(QBrush(gradient));
	// 隐藏极坐标图表（如雷达图）中的径向轴线
	foreach(QAbstractAxis* axis, m_polarChart3->axes(QPolarChart::PolarOrientationRadial)) {
		axis->setVisible(false);
	}


	QChartView *chartView_3 = new QChartView(ui.graphicsView_3);
	chartView_3->setChart(m_polarChart3);
	chartView_3->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_3 = new QHBoxLayout();
	hLayout_3->addWidget(chartView_3);
	hLayout_3->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_3->setLayout(hLayout_3);


	// 创建雷达图
	m_polarChart4 = new QPolarChart();
	m_polarChart4->setTitle("枪击试验");
	m_polarChart4->setTitleBrush(Qt::white);

	// 创建四个系列
	m_stressSeries4 = new QLineSeries();
	m_temperatureSeries4 = new QLineSeries();
	m_overpressureSeries4 = new QLineSeries();
	m_strainSeries4 = new QLineSeries();

	// 设置系列名称
	m_stressSeries4->setName("应力");
	m_temperatureSeries4->setName("温度");
	m_overpressureSeries4->setName("超压");
	m_strainSeries4->setName("应变");

	// 生成数据
	QVector<QPointF> stressData4 = generateData(1);
	QVector<QPointF> tempData4 = generateData(2);
	QVector<QPointF> pressureData4 = generateData(3);
	QVector<QPointF> strainData4 = generateData(4);

	// 为系列添加数据（不使用<<运算符）
	for (const auto &point : stressData4) {
		m_stressSeries4->append(point);
	}
	for (const auto &point : tempData4) {
		m_temperatureSeries4->append(point);
	}
	for (const auto &point : pressureData4) {
		m_overpressureSeries4->append(point);
	}
	for (const auto &point : strainData4) {
		m_strainSeries4->append(point);
	}

	// 创建角度轴（8个数据点）
	angularAxis4 = new QCategoryAxis();
	angularAxis4->setMin(0);
	angularAxis4->setMax(360);
	angularAxis4->setLabelsColor(Qt::white);



	for (int i = 0; i < 8; ++i) {
		// 平均分布在360度上
		qreal angle = 360.0 / 8 * i;
		angularAxis4->append(labels[i], angle);
	}

	// 创建径向轴
	QValueAxis *radialAxis4 = new QValueAxis();
	radialAxis4->setMin(0);
	radialAxis4->setMax(100);
	radialAxis4->setTickCount(8);
	radialAxis4->setLabelFormat("%d");
	radialAxis4->setLabelsColor(Qt::white);

	// 添加轴到图表
	m_polarChart4->addAxis(angularAxis4, QPolarChart::PolarOrientationAngular);
	m_polarChart4->addAxis(radialAxis4, QPolarChart::PolarOrientationRadial);

	// 创建区域系列用于显示阴影
	m_stressArea4 = new QAreaSeries(m_stressSeries4);
	m_temperatureArea4 = new QAreaSeries(m_temperatureSeries4);
	m_overpressureArea4 = new QAreaSeries(m_overpressureSeries4);
	m_strainArea4 = new QAreaSeries(m_strainSeries4);

	m_stressArea4->setName("应力");
	m_temperatureArea4->setName("温度");
	m_overpressureArea4->setName("超压");
	m_strainArea4->setName("应变");


	m_stressArea4->setBrush(stressColor);
	m_stressArea4->setPen(QPen(stressColor, 2));

	m_temperatureArea4->setBrush(tempColor);
	m_temperatureArea4->setPen(QPen(tempColor, 2));

	m_overpressureArea4->setBrush(pressureColor);
	m_overpressureArea4->setPen(QPen(pressureColor, 2));

	m_strainArea4->setBrush(strainColor);
	m_strainArea4->setPen(QPen(strainColor, 2));

	// 添加所有区域到图表
	m_polarChart4->addSeries(m_stressArea4);
	m_polarChart4->addSeries(m_temperatureArea4);
	m_polarChart4->addSeries(m_overpressureArea4);
	m_polarChart4->addSeries(m_strainArea4);

	// 关联系列与轴
	m_stressArea4->attachAxis(angularAxis4);
	m_stressArea4->attachAxis(radialAxis4);

	m_temperatureArea4->attachAxis(angularAxis4);
	m_temperatureArea4->attachAxis(radialAxis4);

	m_overpressureArea4->attachAxis(angularAxis4);
	m_overpressureArea4->attachAxis(radialAxis4);

	m_strainArea4->attachAxis(angularAxis4);
	m_strainArea4->attachAxis(radialAxis4);

	// 配置图例
	m_polarChart4->legend()->setVisible(true);
	m_polarChart4->legend()->setAlignment(Qt::AlignRight);
	m_polarChart4->legend()->setLabelColor(Qt::white);



	// 允许图例点击
	foreach(QLegendMarker* marker, m_polarChart4->legend()->markers()) {
		marker->setVisible(true);
		connect(marker, &QLegendMarker::clicked, this, &AuxiliaryAnalysisWidget::handleLegendClick4);
	}

	// 默认显示应力数据，其他透明
	m_stressArea4->setOpacity(1.0);
	m_temperatureArea4->setOpacity(0.0);
	m_overpressureArea4->setOpacity(0.0);
	m_strainArea4->setOpacity(0.0);

	m_polarChart4->setBackgroundBrush(QBrush(gradient));
	// 隐藏极坐标图表（如雷达图）中的径向轴线
	foreach(QAbstractAxis* axis, m_polarChart4->axes(QPolarChart::PolarOrientationRadial)) {
		axis->setVisible(false);
	}


	QChartView *chartView_4 = new QChartView(ui.graphicsView_4);
	chartView_4->setChart(m_polarChart4);
	chartView_4->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_4 = new QHBoxLayout();
	hLayout_4->addWidget(chartView_4);
	hLayout_4->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_4->setLayout(hLayout_4);

	// 创建雷达图
	m_polarChart5 = new QPolarChart();
	m_polarChart5->setTitle("射流冲击试验");

	// 创建四个系列
	m_stressSeries5 = new QLineSeries();
	m_temperatureSeries5 = new QLineSeries();
	m_overpressureSeries5 = new QLineSeries();
	m_strainSeries5 = new QLineSeries();

	// 设置系列名称
	m_stressSeries5->setName("应力");
	m_temperatureSeries5->setName("温度");
	m_overpressureSeries5->setName("超压");
	m_strainSeries5->setName("应变");

	// 生成数据
	QVector<QPointF> stressData5 = generateData(1);
	QVector<QPointF> tempData5 = generateData(2);
	QVector<QPointF> pressureData5 = generateData(3);
	QVector<QPointF> strainData5 = generateData(4);

	// 为系列添加数据（不使用<<运算符）
	for (const auto &point : stressData5) {
		m_stressSeries5->append(point);
	}
	for (const auto &point : tempData5) {
		m_temperatureSeries5->append(point);
	}
	for (const auto &point : pressureData5) {
		m_overpressureSeries5->append(point);
	}
	for (const auto &point : strainData5) {
		m_strainSeries5->append(point);
	}

	// 创建角度轴（8个数据点）
	angularAxis5 = new QCategoryAxis();
	angularAxis5->setMin(0);
	angularAxis5->setMax(360);

	for (int i = 0; i < 8; ++i) {
		// 平均分布在360度上
		qreal angle = 360.0 / 8 * i;
		angularAxis5->append(labels[i], angle);
	}

	// 创建径向轴
	QValueAxis *radialAxis5 = new QValueAxis();
	radialAxis5->setMin(0);
	radialAxis5->setMax(100);
	radialAxis5->setTickCount(8);
	radialAxis5->setLabelFormat("%d");

	// 添加轴到图表
	m_polarChart5->addAxis(angularAxis5, QPolarChart::PolarOrientationAngular);
	m_polarChart5->addAxis(radialAxis5, QPolarChart::PolarOrientationRadial);

	// 创建区域系列用于显示阴影
	m_stressArea5 = new QAreaSeries(m_stressSeries5);
	m_temperatureArea5 = new QAreaSeries(m_temperatureSeries5);
	m_overpressureArea5 = new QAreaSeries(m_overpressureSeries5);
	m_strainArea5 = new QAreaSeries(m_strainSeries5);

	m_stressArea5->setName("应力");
	m_temperatureArea5->setName("温度");
	m_overpressureArea5->setName("超压");
	m_strainArea5->setName("应变");


	m_stressArea5->setBrush(stressColor);
	m_stressArea5->setPen(QPen(stressColor, 2));

	m_temperatureArea5->setBrush(tempColor);
	m_temperatureArea5->setPen(QPen(tempColor, 2));

	m_overpressureArea5->setBrush(pressureColor);
	m_overpressureArea5->setPen(QPen(pressureColor, 2));

	m_strainArea5->setBrush(strainColor);
	m_strainArea5->setPen(QPen(strainColor, 2));

	// 添加所有区域到图表
	m_polarChart5->addSeries(m_stressArea5);
	m_polarChart5->addSeries(m_temperatureArea5);
	m_polarChart5->addSeries(m_overpressureArea5);
	m_polarChart5->addSeries(m_strainArea5);

	// 关联系列与轴
	m_stressArea5->attachAxis(angularAxis5);
	m_stressArea5->attachAxis(radialAxis5);

	m_temperatureArea5->attachAxis(angularAxis5);
	m_temperatureArea5->attachAxis(radialAxis5);

	m_overpressureArea5->attachAxis(angularAxis5);
	m_overpressureArea5->attachAxis(radialAxis5);

	m_strainArea5->attachAxis(angularAxis5);
	m_strainArea5->attachAxis(radialAxis5);

	// 配置图例
	m_polarChart5->legend()->setVisible(true);
	m_polarChart5->legend()->setAlignment(Qt::AlignRight);

	// 允许图例点击
	foreach(QLegendMarker* marker, m_polarChart5->legend()->markers()) {
		marker->setVisible(true);
		connect(marker, &QLegendMarker::clicked, this, &AuxiliaryAnalysisWidget::handleLegendClick5);
	}

	// 默认显示应力数据，其他透明
	m_stressArea5->setOpacity(1.0);
	m_temperatureArea5->setOpacity(0.0);
	m_overpressureArea5->setOpacity(0.0);
	m_strainArea5->setOpacity(0.0);

	m_polarChart5->setBackgroundBrush(QBrush(gradient));
	// 隐藏极坐标图表（如雷达图）中的径向轴线
	foreach(QAbstractAxis* axis, m_polarChart5->axes(QPolarChart::PolarOrientationRadial)) {
		axis->setVisible(false);
	}


	QChartView *chartView_5 = new QChartView(ui.graphicsView_5);
	chartView_5->setChart(m_polarChart5);
	chartView_5->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_5 = new QHBoxLayout();
	hLayout_5->addWidget(chartView_5);
	hLayout_5->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_5->setLayout(hLayout_5);

	
	// 创建雷达图
	m_polarChart6 = new QPolarChart();
	m_polarChart6->setTitle("破片撞击试验");

	// 创建四个系列
	m_stressSeries6 = new QLineSeries();
	m_temperatureSeries6 = new QLineSeries();
	m_overpressureSeries6 = new QLineSeries();
	m_strainSeries6 = new QLineSeries();

	// 设置系列名称
	m_stressSeries6->setName("应力");
	m_temperatureSeries6->setName("温度");
	m_overpressureSeries6->setName("超压");
	m_strainSeries6->setName("应变");

	// 生成数据
	QVector<QPointF> stressData6 = generateData(1);
	QVector<QPointF> tempData6 = generateData(2);
	QVector<QPointF> pressureData6 = generateData(3);
	QVector<QPointF> strainData6 = generateData(4);

	// 为系列添加数据（不使用<<运算符）
	for (const auto &point : stressData6) {
		m_stressSeries6->append(point);
	}
	for (const auto &point : tempData6) {
		m_temperatureSeries6->append(point);
	}
	for (const auto &point : pressureData6) {
		m_overpressureSeries6->append(point);
	}
	for (const auto &point : strainData6) {
		m_strainSeries6->append(point);
	}

	// 创建角度轴（8个数据点）
	angularAxis6 = new QCategoryAxis();
	angularAxis6->setMin(0);
	angularAxis6->setMax(360);

	for (int i = 0; i < 8; ++i) {
		// 平均分布在360度上
		qreal angle = 360.0 / 8 * i;
		angularAxis6->append(labels[i], angle);
	}

	// 创建径向轴
	QValueAxis *radialAxis6 = new QValueAxis();
	radialAxis6->setMin(0);
	radialAxis6->setMax(100);
	radialAxis6->setTickCount(8);
	radialAxis6->setLabelFormat("%d");

	// 添加轴到图表
	m_polarChart6->addAxis(angularAxis6, QPolarChart::PolarOrientationAngular);
	m_polarChart6->addAxis(radialAxis6, QPolarChart::PolarOrientationRadial);

	// 创建区域系列用于显示阴影
	m_stressArea6 = new QAreaSeries(m_stressSeries6);
	m_temperatureArea6 = new QAreaSeries(m_temperatureSeries6);
	m_overpressureArea6 = new QAreaSeries(m_overpressureSeries6);
	m_strainArea6 = new QAreaSeries(m_strainSeries6);

	m_stressArea6->setName("应力");
	m_temperatureArea6->setName("温度");
	m_overpressureArea6->setName("超压");
	m_strainArea6->setName("应变");


	m_stressArea6->setBrush(stressColor);
	m_stressArea6->setPen(QPen(stressColor, 2));

	m_temperatureArea6->setBrush(tempColor);
	m_temperatureArea6->setPen(QPen(tempColor, 2));

	m_overpressureArea6->setBrush(pressureColor);
	m_overpressureArea6->setPen(QPen(pressureColor, 2));

	m_strainArea6->setBrush(strainColor);
	m_strainArea6->setPen(QPen(strainColor, 2));

	// 添加所有区域到图表
	m_polarChart6->addSeries(m_stressArea6);
	m_polarChart6->addSeries(m_temperatureArea6);
	m_polarChart6->addSeries(m_overpressureArea6);
	m_polarChart6->addSeries(m_strainArea6);

	// 关联系列与轴
	m_stressArea6->attachAxis(angularAxis6);
	m_stressArea6->attachAxis(radialAxis6);

	m_temperatureArea6->attachAxis(angularAxis6);
	m_temperatureArea6->attachAxis(radialAxis6);

	m_overpressureArea6->attachAxis(angularAxis6);
	m_overpressureArea6->attachAxis(radialAxis6);

	m_strainArea6->attachAxis(angularAxis6);
	m_strainArea6->attachAxis(radialAxis6);

	// 配置图例
	m_polarChart6->legend()->setVisible(true);
	m_polarChart6->legend()->setAlignment(Qt::AlignRight);

	// 允许图例点击
	foreach(QLegendMarker* marker, m_polarChart6->legend()->markers()) {
		marker->setVisible(true);
		connect(marker, &QLegendMarker::clicked, this, &AuxiliaryAnalysisWidget::handleLegendClick6);
	}

	// 默认显示应力数据，其他透明
	m_stressArea6->setOpacity(1.0);
	m_temperatureArea6->setOpacity(0.0);
	m_overpressureArea6->setOpacity(0.0);
	m_strainArea6->setOpacity(0.0);

	m_polarChart6->setBackgroundBrush(QBrush(gradient));
	// 隐藏极坐标图表（如雷达图）中的径向轴线
	foreach(QAbstractAxis* axis, m_polarChart6->axes(QPolarChart::PolarOrientationRadial)) {
		axis->setVisible(false);
	}


	QChartView *chartView_6 = new QChartView(ui.graphicsView_6);
	chartView_6->setChart(m_polarChart6);
	chartView_6->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_6 = new QHBoxLayout();
	hLayout_6->addWidget(chartView_6);
	hLayout_6->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_6->setLayout(hLayout_6);

	
	// 创建雷达图
	m_polarChart7 = new QPolarChart();
	m_polarChart7->setTitle("爆炸冲击波试验");

	// 创建四个系列
	m_stressSeries7 = new QLineSeries();
	m_temperatureSeries7 = new QLineSeries();
	m_overpressureSeries7 = new QLineSeries();
	m_strainSeries7 = new QLineSeries();

	// 设置系列名称
	m_stressSeries7->setName("应力");
	m_temperatureSeries7->setName("温度");
	m_overpressureSeries7->setName("超压");
	m_strainSeries7->setName("应变");

	// 生成数据
	QVector<QPointF> stressData7 = generateData(1);
	QVector<QPointF> tempData7 = generateData(2);
	QVector<QPointF> pressureData7 = generateData(3);
	QVector<QPointF> strainData7 = generateData(4);

	// 为系列添加数据（不使用<<运算符）
	for (const auto &point : stressData7) {
		m_stressSeries7->append(point);
	}
	for (const auto &point : tempData7) {
		m_temperatureSeries7->append(point);
	}
	for (const auto &point : pressureData7) {
		m_overpressureSeries7->append(point);
	}
	for (const auto &point : strainData7) {
		m_strainSeries7->append(point);
	}

	// 创建角度轴（8个数据点）
	angularAxis7 = new QCategoryAxis();
	angularAxis7->setMin(0);
	angularAxis7->setMax(360);

	for (int i = 0; i < 8; ++i) {
		// 平均分布在360度上
		qreal angle = 360.0 / 8 * i;
		angularAxis7->append(labels[i], angle);
	}

	// 创建径向轴
	QValueAxis *radialAxis7 = new QValueAxis();
	radialAxis7->setMin(0);
	radialAxis7->setMax(100);
	radialAxis7->setTickCount(8);
	radialAxis7->setLabelFormat("%d");

	// 添加轴到图表
	m_polarChart7->addAxis(angularAxis7, QPolarChart::PolarOrientationAngular);
	m_polarChart7->addAxis(radialAxis7, QPolarChart::PolarOrientationRadial);

	// 创建区域系列用于显示阴影
	m_stressArea7 = new QAreaSeries(m_stressSeries7);
	m_temperatureArea7 = new QAreaSeries(m_temperatureSeries7);
	m_overpressureArea7 = new QAreaSeries(m_overpressureSeries7);
	m_strainArea7 = new QAreaSeries(m_strainSeries7);

	m_stressArea7->setName("应力");
	m_temperatureArea7->setName("温度");
	m_overpressureArea7->setName("超压");
	m_strainArea7->setName("应变");


	m_stressArea7->setBrush(stressColor);
	m_stressArea7->setPen(QPen(stressColor, 2));

	m_temperatureArea7->setBrush(tempColor);
	m_temperatureArea7->setPen(QPen(tempColor, 2));

	m_overpressureArea7->setBrush(pressureColor);
	m_overpressureArea7->setPen(QPen(pressureColor, 2));

	m_strainArea7->setBrush(strainColor);
	m_strainArea7->setPen(QPen(strainColor, 2));

	// 添加所有区域到图表
	m_polarChart7->addSeries(m_stressArea7);
	m_polarChart7->addSeries(m_temperatureArea7);
	m_polarChart7->addSeries(m_overpressureArea7);
	m_polarChart7->addSeries(m_strainArea7);

	// 关联系列与轴
	m_stressArea7->attachAxis(angularAxis7);
	m_stressArea7->attachAxis(radialAxis7);

	m_temperatureArea7->attachAxis(angularAxis7);
	m_temperatureArea7->attachAxis(radialAxis7);

	m_overpressureArea7->attachAxis(angularAxis7);
	m_overpressureArea7->attachAxis(radialAxis7);

	m_strainArea7->attachAxis(angularAxis7);
	m_strainArea7->attachAxis(radialAxis7);

	// 配置图例
	m_polarChart7->legend()->setVisible(true);
	m_polarChart7->legend()->setAlignment(Qt::AlignRight);

	// 允许图例点击
	foreach(QLegendMarker* marker, m_polarChart7->legend()->markers()) {
		marker->setVisible(true);
		connect(marker, &QLegendMarker::clicked, this, &AuxiliaryAnalysisWidget::handleLegendClick7);
	}

	// 默认显示应力数据，其他透明
	m_stressArea7->setOpacity(1.0);
	m_temperatureArea7->setOpacity(0.0);
	m_overpressureArea7->setOpacity(0.0);
	m_strainArea7->setOpacity(0.0);

	m_polarChart7->setBackgroundBrush(QBrush(gradient));
	// 隐藏极坐标图表（如雷达图）中的径向轴线
	foreach(QAbstractAxis* axis, m_polarChart7->axes(QPolarChart::PolarOrientationRadial)) {
		axis->setVisible(false);
	}


	QChartView *chartView_7 = new QChartView(ui.graphicsView_7);
	chartView_7->setChart(m_polarChart7);
	chartView_7->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_7 = new QHBoxLayout();
	hLayout_7->addWidget(chartView_7);
	hLayout_7->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_7->setLayout(hLayout_7);

	// 创建雷达图
	m_polarChart8 = new QPolarChart();
	m_polarChart8->setTitle("殉爆试验");

	// 创建四个系列
	m_stressSeries8 = new QLineSeries();
	m_temperatureSeries8 = new QLineSeries();
	m_overpressureSeries8 = new QLineSeries();
	m_strainSeries8 = new QLineSeries();

	// 设置系列名称
	m_stressSeries8->setName("应力");
	m_temperatureSeries8->setName("温度");
	m_overpressureSeries8->setName("超压");
	m_strainSeries8->setName("应变");

	// 生成数据
	QVector<QPointF> stressData8 = generateData(1);
	QVector<QPointF> tempData8 = generateData(2);
	QVector<QPointF> pressureData8 = generateData(3);
	QVector<QPointF> strainData8 = generateData(4);

	// 为系列添加数据（不使用<<运算符）
	for (const auto &point : stressData8) {
		m_stressSeries8->append(point);
	}
	for (const auto &point : tempData8) {
		m_temperatureSeries8->append(point);
	}
	for (const auto &point : pressureData8) {
		m_overpressureSeries8->append(point);
	}
	for (const auto &point : strainData8) {
		m_strainSeries8->append(point);
	}

	// 创建角度轴（8个数据点）
	angularAxis8 = new QCategoryAxis();
	angularAxis8->setMin(0);
	angularAxis8->setMax(360);

	for (int i = 0; i < 8; ++i) {
		// 平均分布在360度上
		qreal angle = 360.0 / 8 * i;
		angularAxis8->append(labels[i], angle);
	}

	// 创建径向轴
	QValueAxis *radialAxis8 = new QValueAxis();
	radialAxis8->setMin(0);
	radialAxis8->setMax(100);
	radialAxis8->setTickCount(8);
	radialAxis8->setLabelFormat("%d");

	// 添加轴到图表
	m_polarChart8->addAxis(angularAxis8, QPolarChart::PolarOrientationAngular);
	m_polarChart8->addAxis(radialAxis8, QPolarChart::PolarOrientationRadial);

	// 创建区域系列用于显示阴影
	m_stressArea8 = new QAreaSeries(m_stressSeries8);
	m_temperatureArea8 = new QAreaSeries(m_temperatureSeries8);
	m_overpressureArea8 = new QAreaSeries(m_overpressureSeries8);
	m_strainArea8 = new QAreaSeries(m_strainSeries8);

	m_stressArea8->setName("应力");
	m_temperatureArea8->setName("温度");
	m_overpressureArea8->setName("超压");
	m_strainArea8->setName("应变");


	m_stressArea8->setBrush(stressColor);
	m_stressArea8->setPen(QPen(stressColor, 2));

	m_temperatureArea8->setBrush(tempColor);
	m_temperatureArea8->setPen(QPen(tempColor, 2));

	m_overpressureArea8->setBrush(pressureColor);
	m_overpressureArea8->setPen(QPen(pressureColor, 2));

	m_strainArea8->setBrush(strainColor);
	m_strainArea8->setPen(QPen(strainColor, 2));

	// 添加所有区域到图表
	m_polarChart8->addSeries(m_stressArea8);
	m_polarChart8->addSeries(m_temperatureArea8);
	m_polarChart8->addSeries(m_overpressureArea8);
	m_polarChart8->addSeries(m_strainArea8);

	// 关联系列与轴
	m_stressArea8->attachAxis(angularAxis8);
	m_stressArea8->attachAxis(radialAxis8);

	m_temperatureArea8->attachAxis(angularAxis8);
	m_temperatureArea8->attachAxis(radialAxis8);

	m_overpressureArea8->attachAxis(angularAxis8);
	m_overpressureArea8->attachAxis(radialAxis8);

	m_strainArea8->attachAxis(angularAxis8);
	m_strainArea8->attachAxis(radialAxis8);

	// 配置图例
	m_polarChart8->legend()->setVisible(true);
	m_polarChart8->legend()->setAlignment(Qt::AlignRight);

	// 允许图例点击
	foreach(QLegendMarker* marker, m_polarChart8->legend()->markers()) {
		marker->setVisible(true);
		connect(marker, &QLegendMarker::clicked, this, &AuxiliaryAnalysisWidget::handleLegendClick8);
	}

	// 默认显示应力数据，其他透明
	m_stressArea8->setOpacity(1.0);
	m_temperatureArea8->setOpacity(0.0);
	m_overpressureArea8->setOpacity(0.0);
	m_strainArea8->setOpacity(0.0);

	m_polarChart8->setBackgroundBrush(QBrush(gradient));
	// 隐藏极坐标图表（如雷达图）中的径向轴线
	foreach(QAbstractAxis* axis, m_polarChart8->axes(QPolarChart::PolarOrientationRadial)) {
		axis->setVisible(false);
	}


	QChartView *chartView_8 = new QChartView(ui.graphicsView_8);
	chartView_8->setChart(m_polarChart8);
	chartView_8->setRenderHint(QPainter::Antialiasing);
	QHBoxLayout*hLayout_8 = new QHBoxLayout();
	hLayout_8->addWidget(chartView_8);
	hLayout_8->setContentsMargins(0, 0, 0, 0);
	ui.graphicsView_8->setLayout(hLayout_8);


	/////////////////////// 中间比例图
	

	QLineSeries *seriesdata = new QLineSeries();
	seriesdata->append(angularMin, 143);
	seriesdata->append(angularMin + ad6 * 1, 330);
	seriesdata->append(angularMin + ad6 * 2, 290);
	seriesdata->append(angularMin + ad6 * 3, 369);
	seriesdata->append(angularMin + ad6 * 4, 146);
	seriesdata->append(angularMin + ad6 * 5, 262);
	seriesdata->append(angularMin + ad6 * 6, 143);
	seriesdata->append(angularMin + ad6 * 7, 243);
	seriesdata->append(angularMin + ad6 * 8, 143);
	{
		//设置线上的标签可见
		seriesdata->setPointLabelsFormat("@yPoint");
		seriesdata->setPointLabelsClipping(false);
		seriesdata->setPointLabelsVisible(true);
		seriesdata->setPointLabelsColor(Qt::white);
	}



	QAreaSeries *seriesArea = new QAreaSeries();
	seriesArea->setUpperSeries(seriesdata);
	seriesArea->setOpacity(0.5);

	QPolarChart *chart = new QPolarChart();
	chart->setPlotArea(QRectF(0, 2500, 0, 0)); // 上移50个单位


	chart->addSeries(seriesdata);
	chart->addSeries(seriesArea);


	QCategoryAxis *angularAxis = new QCategoryAxis();
	angularAxis->append("A", 0);
	angularAxis->append("B", 45);
	angularAxis->append("C", 90);
	angularAxis->append("D", 134);
	angularAxis->append("E", 180);
	angularAxis->append("F", 225);
	angularAxis->append("G", 270);
	angularAxis->append("H", 315);
	angularAxis->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
	angularAxis->setLabelsColor(Qt::white);

	angularAxis->setShadesBrush(QBrush(QColor(249, 249, 255)));
	chart->addAxis(angularAxis, QPolarChart::PolarOrientationAngular);
	QValueAxis *radialAxis0 = new QValueAxis();

	radialAxis0->setLabelFormat("");
	chart->addAxis(radialAxis0, QPolarChart::PolarOrientationRadial);

	angularAxis->setLineVisible(false);
	angularAxis->setLabelsColor(Qt::white);
	chart->axes(QPolarChart::PolarOrientationRadial).at(0)->setVisible(false);
	seriesArea->attachAxis(radialAxis0);
	seriesArea->attachAxis(angularAxis);
	for (int i = 0; i <= 2; ++i)
	{
		QLineSeries *seriesLineTemp = new QLineSeries();
		chart->addSeries(seriesLineTemp);
		seriesLineTemp->attachAxis(radialAxis0);
		seriesLineTemp->attachAxis(angularAxis);
		seriesLineTemp->setColor(QColor(214, 214, 214));
		int interval = 45;
		seriesLineTemp->append(angularMin, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 1, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 2, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 3, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 4, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 5, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 6, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 7, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 8, radialMax - 10 - interval * i);
	}

	chart->legend()->markers(seriesdata).at(0)->setVisible(false);

	foreach(QLegendMarker* marker, chart->legend()->markers())
	{
		if (marker->type() != QLegendMarker::LegendMarkerTypeArea)
		{
			marker->setVisible(false);
		}
	}


	radialAxis0->setRange(radialMin, radialMax);
	angularAxis->setRange(angularMin, angularMax);

	chart->setBackgroundBrush(QBrush(gradient));
	chart->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis *xAxis_0 = chart->axisX();
	QAbstractAxis *yAxis_0 = chart->axisY();
	xAxis_0->setLabelsColor(Qt::white);
	yAxis_0->setLabelsColor(Qt::white);



	// 将图表小部件添加到布局中
	QChartView *chartView_0 = new QChartView(ui.graphicsView_0);
	chartView_0->setChart(chart);
	chartView_0->setRenderHint(QPainter::Antialiasing);
	//chartView_0->move(chartView_0->x(), chartView_0->y() - 100);

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


QVector<QPointF> AuxiliaryAnalysisWidget::generateData(int seed)
{
	// 生成8个数据点，角度均匀分布
	QVector<QPointF> data;
	qsrand(seed); // 使用不同种子确保数据不同

	for (int i = 0; i <= 8; ++i) {
		qreal angle = 360.0 / 8 * i;
		qreal value = 60 + qrand() % 60; // 随机值在30-90之间
		data.append(QPointF(angle, value));
	}

	// 闭合曲线
	//data.append(data[0]);

	return data;
}

void AuxiliaryAnalysisWidget::handleLegendClick1()
{
	// 获取点击的图例标记
	QLegendMarker* marker = qobject_cast<QLegendMarker*>(sender());
	if (!marker) return;

	// 获取被点击的系列名称
	QString seriesName = marker->series()->name();

	// 重置所有透明度
	m_stressArea1->setOpacity(0.0);
	m_temperatureArea1->setOpacity(0.0);
	m_overpressureArea1->setOpacity(0.0);
	m_strainArea1->setOpacity(0.0);

	angularAxis1 = new QCategoryAxis();
	angularAxis1->setMin(0);
	angularAxis1->setMax(360);
	angularAxis1->setLabelsColor(Qt::white);

	// 只显示被点击的系列
	if (seriesName == "应力") {
		m_stressArea1->setOpacity(1.0);
		// 设置标签

		QStringList labels = {
		"发动机壳体最大应力", "固体推进剂最大应力", "隔绝热最大应力", "外防热最大应力",
		"发动机壳体最小应力", "固体推进剂最小应力", "隔绝热最小应力", "外防热最小应力"
		};

		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis1->append(labels[i], angle);
		}
	}
	else if (seriesName == "温度") {
		m_temperatureArea1->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大温度", "固体推进剂最大温度", "隔绝热最大温度", "外防热最大温度",
		"发动机壳体最小温度", "固体推进剂最小温度", "隔绝热最小温度", "外防热最小温度"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis1->append(labels[i], angle);
		}
	}
	else if (seriesName == "超压") {
		m_overpressureArea1->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大超压", "固体推进剂最大超压", "隔绝热最大超压", "外防热最大超压",
		"发动机壳体最小超压", "固体推进剂最小超压", "隔绝热最小超压", "外防热最小超压"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis1->append(labels[i], angle);
		}
	}
	else if (seriesName == "应变") {
		m_strainArea1->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大应变", "固体推进剂最大应变", "隔绝热最大应变", "外防热最大应变",
		"发动机壳体最小应变", "固体推进剂最小应变", "隔绝热最小应变", "外防热最小应变"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis1->append(labels[i], angle);
		}
	}

	// 删除原有的轴
	QList<QAbstractAxis*> angularAxes = m_polarChart1->axes(QPolarChart::PolarOrientationAngular);
	for (QAbstractAxis* axis : angularAxes) {
		m_polarChart1->removeAxis(axis);
	}
	// 添加轴到图表
	m_polarChart1->addAxis(angularAxis1, QPolarChart::PolarOrientationAngular);
}

void AuxiliaryAnalysisWidget::handleLegendClick2()
{
	// 获取点击的图例标记
	QLegendMarker* marker = qobject_cast<QLegendMarker*>(sender());
	if (!marker) return;

	// 获取被点击的系列名称
	QString seriesName = marker->series()->name();

	// 重置所有透明度
	m_stressArea2->setOpacity(0.0);
	m_temperatureArea2->setOpacity(0.0);
	m_overpressureArea2->setOpacity(0.0);
	m_strainArea2->setOpacity(0.0);

	angularAxis2 = new QCategoryAxis();
	angularAxis2->setMin(0);
	angularAxis2->setMax(360);
	angularAxis2->setLabelsColor(Qt::white);

	// 只显示被点击的系列
	if (seriesName == "应力") {
		m_stressArea2->setOpacity(1.0);
		// 设置标签

		QStringList labels = {
		"发动机壳体最大应力", "固体推进剂最大应力", "隔绝热最大应力", "外防热最大应力",
		"发动机壳体最小应力", "固体推进剂最小应力", "隔绝热最小应力", "外防热最小应力"
		};

		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis2->append(labels[i], angle);
		}
	}
	else if (seriesName == "温度") {
		m_temperatureArea2->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大温度", "固体推进剂最大温度", "隔绝热最大温度", "外防热最大温度",
		"发动机壳体最小温度", "固体推进剂最小温度", "隔绝热最小温度", "外防热最小温度"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis2->append(labels[i], angle);
		}
	}
	else if (seriesName == "超压") {
		m_overpressureArea2->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大超压", "固体推进剂最大超压", "隔绝热最大超压", "外防热最大超压",
		"发动机壳体最小超压", "固体推进剂最小超压", "隔绝热最小超压", "外防热最小超压"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis2->append(labels[i], angle);
		}
	}
	else if (seriesName == "应变") {
		m_strainArea2->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大应变", "固体推进剂最大应变", "隔绝热最大应变", "外防热最大应变",
		"发动机壳体最小应变", "固体推进剂最小应变", "隔绝热最小应变", "外防热最小应变"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis2->append(labels[i], angle);
		}
	}

	// 删除原有的轴
	QList<QAbstractAxis*> angularAxes = m_polarChart2->axes(QPolarChart::PolarOrientationAngular);
	for (QAbstractAxis* axis : angularAxes) {
		m_polarChart2->removeAxis(axis);
	}
	// 添加轴到图表
	m_polarChart2->addAxis(angularAxis2, QPolarChart::PolarOrientationAngular);
}



void AuxiliaryAnalysisWidget::handleLegendClick3()
{
	// 获取点击的图例标记
	QLegendMarker* marker = qobject_cast<QLegendMarker*>(sender());
	if (!marker) return;

	// 获取被点击的系列名称
	QString seriesName = marker->series()->name();

	// 重置所有透明度
	m_stressArea3->setOpacity(0.0);
	m_temperatureArea3->setOpacity(0.0);
	m_overpressureArea3->setOpacity(0.0);
	m_strainArea3->setOpacity(0.0);

	angularAxis3 = new QCategoryAxis();
	angularAxis3->setMin(0);
	angularAxis3->setMax(360);
	angularAxis3->setLabelsColor(Qt::white);

	// 只显示被点击的系列
	if (seriesName == "应力") {
		m_stressArea3->setOpacity(1.0);
		// 设置标签

		QStringList labels = {
		"发动机壳体最大应力", "固体推进剂最大应力", "隔绝热最大应力", "外防热最大应力",
		"发动机壳体最小应力", "固体推进剂最小应力", "隔绝热最小应力", "外防热最小应力"
		};

		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis3->append(labels[i], angle);
		}
	}
	else if (seriesName == "温度") {
		m_temperatureArea3->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大温度", "固体推进剂最大温度", "隔绝热最大温度", "外防热最大温度",
		"发动机壳体最小温度", "固体推进剂最小温度", "隔绝热最小温度", "外防热最小温度"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis3->append(labels[i], angle);
		}
	}
	else if (seriesName == "超压") {
		m_overpressureArea3->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大超压", "固体推进剂最大超压", "隔绝热最大超压", "外防热最大超压",
		"发动机壳体最小超压", "固体推进剂最小超压", "隔绝热最小超压", "外防热最小超压"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis3->append(labels[i], angle);
		}
	}
	else if (seriesName == "应变") {
		m_strainArea3->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大应变", "固体推进剂最大应变", "隔绝热最大应变", "外防热最大应变",
		"发动机壳体最小应变", "固体推进剂最小应变", "隔绝热最小应变", "外防热最小应变"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis3->append(labels[i], angle);
		}
	}

	// 删除原有的轴
	QList<QAbstractAxis*> angularAxes = m_polarChart3->axes(QPolarChart::PolarOrientationAngular);
	for (QAbstractAxis* axis : angularAxes) {
		m_polarChart3->removeAxis(axis);
	}
	// 添加轴到图表
	m_polarChart3->addAxis(angularAxis3, QPolarChart::PolarOrientationAngular);
}


void AuxiliaryAnalysisWidget::handleLegendClick4()
{
	// 获取点击的图例标记
	QLegendMarker* marker = qobject_cast<QLegendMarker*>(sender());
	if (!marker) return;

	// 获取被点击的系列名称
	QString seriesName = marker->series()->name();

	// 重置所有透明度
	m_stressArea4->setOpacity(0.0);
	m_temperatureArea4->setOpacity(0.0);
	m_overpressureArea4->setOpacity(0.0);
	m_strainArea4->setOpacity(0.0);

	angularAxis4 = new QCategoryAxis();
	angularAxis4->setMin(0);
	angularAxis4->setMax(360);
	angularAxis4->setLabelsColor(Qt::white);

	// 只显示被点击的系列
	if (seriesName == "应力") {
		m_stressArea4->setOpacity(1.0);
		// 设置标签

		QStringList labels = {
		"发动机壳体最大应力", "固体推进剂最大应力", "隔绝热最大应力", "外防热最大应力",
		"发动机壳体最小应力", "固体推进剂最小应力", "隔绝热最小应力", "外防热最小应力"
		};

		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis4->append(labels[i], angle);
		}
	}
	else if (seriesName == "温度") {
		m_temperatureArea4->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大温度", "固体推进剂最大温度", "隔绝热最大温度", "外防热最大温度",
		"发动机壳体最小温度", "固体推进剂最小温度", "隔绝热最小温度", "外防热最小温度"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis4->append(labels[i], angle);
		}
	}
	else if (seriesName == "超压") {
		m_overpressureArea4->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大超压", "固体推进剂最大超压", "隔绝热最大超压", "外防热最大超压",
		"发动机壳体最小超压", "固体推进剂最小超压", "隔绝热最小超压", "外防热最小超压"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis4->append(labels[i], angle);
		}
	}
	else if (seriesName == "应变") {
		m_strainArea4->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大应变", "固体推进剂最大应变", "隔绝热最大应变", "外防热最大应变",
		"发动机壳体最小应变", "固体推进剂最小应变", "隔绝热最小应变", "外防热最小应变"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis4->append(labels[i], angle);
		}
	}

	// 删除原有的轴
	QList<QAbstractAxis*> angularAxes = m_polarChart4->axes(QPolarChart::PolarOrientationAngular);
	for (QAbstractAxis* axis : angularAxes) {
		m_polarChart4->removeAxis(axis);
	}
	// 添加轴到图表
	m_polarChart4->addAxis(angularAxis4, QPolarChart::PolarOrientationAngular);
}


void AuxiliaryAnalysisWidget::handleLegendClick5()
{
	// 获取点击的图例标记
	QLegendMarker* marker = qobject_cast<QLegendMarker*>(sender());
	if (!marker) return;

	// 获取被点击的系列名称
	QString seriesName = marker->series()->name();

	// 重置所有透明度
	m_stressArea5->setOpacity(0.0);
	m_temperatureArea5->setOpacity(0.0);
	m_overpressureArea5->setOpacity(0.0);
	m_strainArea5->setOpacity(0.0);

	angularAxis5 = new QCategoryAxis();
	angularAxis5->setMin(0);
	angularAxis5->setMax(360);
	angularAxis5->setLabelsColor(Qt::white);

	// 只显示被点击的系列
	if (seriesName == "应力") {
		m_stressArea5->setOpacity(1.0);
		// 设置标签

		QStringList labels = {
		"发动机壳体最大应力", "固体推进剂最大应力", "隔绝热最大应力", "外防热最大应力",
		"发动机壳体最小应力", "固体推进剂最小应力", "隔绝热最小应力", "外防热最小应力"
		};

		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis5->append(labels[i], angle);
		}
	}
	else if (seriesName == "温度") {
		m_temperatureArea5->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大温度", "固体推进剂最大温度", "隔绝热最大温度", "外防热最大温度",
		"发动机壳体最小温度", "固体推进剂最小温度", "隔绝热最小温度", "外防热最小温度"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis5->append(labels[i], angle);
		}
	}
	else if (seriesName == "超压") {
		m_overpressureArea5->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大超压", "固体推进剂最大超压", "隔绝热最大超压", "外防热最大超压",
		"发动机壳体最小超压", "固体推进剂最小超压", "隔绝热最小超压", "外防热最小超压"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis5->append(labels[i], angle);
		}
	}
	else if (seriesName == "应变") {
		m_strainArea5->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大应变", "固体推进剂最大应变", "隔绝热最大应变", "外防热最大应变",
		"发动机壳体最小应变", "固体推进剂最小应变", "隔绝热最小应变", "外防热最小应变"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis5->append(labels[i], angle);
		}
	}

	// 删除原有的轴
	QList<QAbstractAxis*> angularAxes = m_polarChart5->axes(QPolarChart::PolarOrientationAngular);
	for (QAbstractAxis* axis : angularAxes) {
		m_polarChart5->removeAxis(axis);
	}
	// 添加轴到图表
	m_polarChart5->addAxis(angularAxis5, QPolarChart::PolarOrientationAngular);
}


void AuxiliaryAnalysisWidget::handleLegendClick6()
{
	// 获取点击的图例标记
	QLegendMarker* marker = qobject_cast<QLegendMarker*>(sender());
	if (!marker) return;

	// 获取被点击的系列名称
	QString seriesName = marker->series()->name();

	// 重置所有透明度
	m_stressArea6->setOpacity(0.0);
	m_temperatureArea6->setOpacity(0.0);
	m_overpressureArea6->setOpacity(0.0);
	m_strainArea6->setOpacity(0.0);

	angularAxis6 = new QCategoryAxis();
	angularAxis6->setMin(0);
	angularAxis6->setMax(360);
	angularAxis6->setLabelsColor(Qt::white);

	// 只显示被点击的系列
	if (seriesName == "应力") {
		m_stressArea6->setOpacity(1.0);
		// 设置标签

		QStringList labels = {
		"发动机壳体最大应力", "固体推进剂最大应力", "隔绝热最大应力", "外防热最大应力",
		"发动机壳体最小应力", "固体推进剂最小应力", "隔绝热最小应力", "外防热最小应力"
		};

		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis6->append(labels[i], angle);
		}
	}
	else if (seriesName == "温度") {
		m_temperatureArea6->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大温度", "固体推进剂最大温度", "隔绝热最大温度", "外防热最大温度",
		"发动机壳体最小温度", "固体推进剂最小温度", "隔绝热最小温度", "外防热最小温度"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis6->append(labels[i], angle);
		}
	}
	else if (seriesName == "超压") {
		m_overpressureArea6->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大超压", "固体推进剂最大超压", "隔绝热最大超压", "外防热最大超压",
		"发动机壳体最小超压", "固体推进剂最小超压", "隔绝热最小超压", "外防热最小超压"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis6->append(labels[i], angle);
		}
	}
	else if (seriesName == "应变") {
		m_strainArea6->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大应变", "固体推进剂最大应变", "隔绝热最大应变", "外防热最大应变",
		"发动机壳体最小应变", "固体推进剂最小应变", "隔绝热最小应变", "外防热最小应变"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis6->append(labels[i], angle);
		}
	}

	// 删除原有的轴
	QList<QAbstractAxis*> angularAxes = m_polarChart6->axes(QPolarChart::PolarOrientationAngular);
	for (QAbstractAxis* axis : angularAxes) {
		m_polarChart6->removeAxis(axis);
	}
	// 添加轴到图表
	m_polarChart6->addAxis(angularAxis6, QPolarChart::PolarOrientationAngular);
}


void AuxiliaryAnalysisWidget::handleLegendClick7()
{
	// 获取点击的图例标记
	QLegendMarker* marker = qobject_cast<QLegendMarker*>(sender());
	if (!marker) return;

	// 获取被点击的系列名称
	QString seriesName = marker->series()->name();

	// 重置所有透明度
	m_stressArea7->setOpacity(0.0);
	m_temperatureArea7->setOpacity(0.0);
	m_overpressureArea7->setOpacity(0.0);
	m_strainArea7->setOpacity(0.0);

	angularAxis7 = new QCategoryAxis();
	angularAxis7->setMin(0);
	angularAxis7->setMax(360);
	angularAxis7->setLabelsColor(Qt::white);

	// 只显示被点击的系列
	if (seriesName == "应力") {
		m_stressArea7->setOpacity(1.0);
		// 设置标签

		QStringList labels = {
		"发动机壳体最大应力", "固体推进剂最大应力", "隔绝热最大应力", "外防热最大应力",
		"发动机壳体最小应力", "固体推进剂最小应力", "隔绝热最小应力", "外防热最小应力"
		};

		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis7->append(labels[i], angle);
		}
	}
	else if (seriesName == "温度") {
		m_temperatureArea7->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大温度", "固体推进剂最大温度", "隔绝热最大温度", "外防热最大温度",
		"发动机壳体最小温度", "固体推进剂最小温度", "隔绝热最小温度", "外防热最小温度"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis7->append(labels[i], angle);
		}
	}
	else if (seriesName == "超压") {
		m_overpressureArea7->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大超压", "固体推进剂最大超压", "隔绝热最大超压", "外防热最大超压",
		"发动机壳体最小超压", "固体推进剂最小超压", "隔绝热最小超压", "外防热最小超压"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis7->append(labels[i], angle);
		}
	}
	else if (seriesName == "应变") {
		m_strainArea7->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大应变", "固体推进剂最大应变", "隔绝热最大应变", "外防热最大应变",
		"发动机壳体最小应变", "固体推进剂最小应变", "隔绝热最小应变", "外防热最小应变"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis7->append(labels[i], angle);
		}
	}

	// 删除原有的轴
	QList<QAbstractAxis*> angularAxes = m_polarChart7->axes(QPolarChart::PolarOrientationAngular);
	for (QAbstractAxis* axis : angularAxes) {
		m_polarChart7->removeAxis(axis);
	}
	// 添加轴到图表
	m_polarChart7->addAxis(angularAxis7, QPolarChart::PolarOrientationAngular);
}


void AuxiliaryAnalysisWidget::handleLegendClick8()
{
	// 获取点击的图例标记
	QLegendMarker* marker = qobject_cast<QLegendMarker*>(sender());
	if (!marker) return;

	// 获取被点击的系列名称
	QString seriesName = marker->series()->name();

	// 重置所有透明度
	m_stressArea8->setOpacity(0.0);
	m_temperatureArea8->setOpacity(0.0);
	m_overpressureArea8->setOpacity(0.0);
	m_strainArea8->setOpacity(0.0);

	angularAxis8 = new QCategoryAxis();
	angularAxis8->setMin(0);
	angularAxis8->setMax(360);
	angularAxis8->setLabelsColor(Qt::white);

	// 只显示被点击的系列
	if (seriesName == "应力") {
		m_stressArea8->setOpacity(1.0);
		// 设置标签

		QStringList labels = {
		"发动机壳体最大应力", "固体推进剂最大应力", "隔绝热最大应力", "外防热最大应力",
		"发动机壳体最小应力", "固体推进剂最小应力", "隔绝热最小应力", "外防热最小应力"
		};

		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis8->append(labels[i], angle);
		}
	}
	else if (seriesName == "温度") {
		m_temperatureArea8->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大温度", "固体推进剂最大温度", "隔绝热最大温度", "外防热最大温度",
		"发动机壳体最小温度", "固体推进剂最小温度", "隔绝热最小温度", "外防热最小温度"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis8->append(labels[i], angle);
		}
	}
	else if (seriesName == "超压") {
		m_overpressureArea8->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大超压", "固体推进剂最大超压", "隔绝热最大超压", "外防热最大超压",
		"发动机壳体最小超压", "固体推进剂最小超压", "隔绝热最小超压", "外防热最小超压"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis8->append(labels[i], angle);
		}
	}
	else if (seriesName == "应变") {
		m_strainArea8->setOpacity(1.0);
		// 设置标签
		QStringList labels = {
		"发动机壳体最大应变", "固体推进剂最大应变", "隔绝热最大应变", "外防热最大应变",
		"发动机壳体最小应变", "固体推进剂最小应变", "隔绝热最小应变", "外防热最小应变"
		};
		for (int i = 0; i < 8; ++i) {
			// 平均分布在360度上
			qreal angle = 360.0 / 8 * i;
			angularAxis8->append(labels[i], angle);
		}
	}

	// 删除原有的轴
	QList<QAbstractAxis*> angularAxes = m_polarChart8->axes(QPolarChart::PolarOrientationAngular);
	for (QAbstractAxis* axis : angularAxes) {
		m_polarChart8->removeAxis(axis);
	}
	// 添加轴到图表
	m_polarChart8->addAxis(angularAxis8, QPolarChart::PolarOrientationAngular);
}