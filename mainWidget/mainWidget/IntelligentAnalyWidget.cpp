#pragma execution_character_set("utf-8")
#include "IntelligentAnalyWidget.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QSplitter>
#include <QHeaderView>


IntelligentAnalyWidget::IntelligentAnalyWidget(QWidget* parent)
	: QWidget(parent)
{

	m_treeModelWidget = new IntelligentAnalyTreeWidget();
	m_propertyStackWidget = new QStackedWidget();
	m_tableStackWidget = new QStackedWidget();

	


	// 设置m_PropertyStackWidget的背景为白色
	m_propertyStackWidget->setStyleSheet("background-color: white;");

	m_intelligentPropertyWidget = new IntelligentPropertyWidget();
	m_fallPropertyWidget = new IntelligentFallPropertyWidget();
	m_fastCombustionPropertyWidget = new IntelligentFastCombustionPropertyWidget();
	m_slowCombustionPropertyWidget = new IntelligentSlowCombustionPropertyWidget();
	m_shootPropertyWidget = new IntelligentShootPropertyWidget();
	m_jetImpactPropertyWidget = new IntelligentJetImpactPropertyWidget();
	m_fragmentationImpactPropertyWidget = new IntelligentFragmentationImpactPropertyWidget();
	m_explosiveBlastPropertyWidget = new IntelligentExplosiveBlastPropertyWidget();
	m_sacrificeExplosionPropertyWidget = new IntelligentSacrificeExplosionPropertyWidget();


	// 将所有的 PropertyWidget 添加到 QStackedWidget 中
	m_propertyStackWidget->addWidget(m_intelligentPropertyWidget);
	m_propertyStackWidget->addWidget(m_fallPropertyWidget);
	m_propertyStackWidget->addWidget(m_fastCombustionPropertyWidget);
	m_propertyStackWidget->addWidget(m_slowCombustionPropertyWidget);
	m_propertyStackWidget->addWidget(m_shootPropertyWidget);
	m_propertyStackWidget->addWidget(m_jetImpactPropertyWidget);
	m_propertyStackWidget->addWidget(m_fragmentationImpactPropertyWidget);
	m_propertyStackWidget->addWidget(m_explosiveBlastPropertyWidget);
	m_propertyStackWidget->addWidget(m_sacrificeExplosionPropertyWidget);



	m_fallTableWidget = new QTableWidget();
	m_fallTableWidget->setRowCount(10); // 行数
	m_fallTableWidget->setColumnCount(7); // 列数
	m_fallTableWidget->verticalHeader()->setVisible(false);
	m_fallTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_fallData = {
		{ "设计点", "壳体厚度[mm]", "跌落高度[m]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "10", " ", " ", " ", " " },
		{ "2", "2", "10", " ", " ", " ", " " },
		{ "3", "3", "10", " ", " ", " ", " " },
		{ "4", "1", "20", " ", " ", " ", " " },
		{ "5", "2", "20", " ", " ", " ", " " },
		{ "6", "3", "20", " ", " ", " ", " " },
		{ "7", "1", "30", " ", " ", " ", " " },
		{ "8", "2", "40", " ", " ", " ", " " },
		{ "9", "3", "30", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_fallData.size(); ++i) {
		for (int j = 0; j < m_fallData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_fallData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_fallTableWidget->setItem(i, j, item);
		}
	}
	m_fallTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_fastCombustionTableWidget = new QTableWidget();
	m_fastCombustionTableWidget->setRowCount(10); // 行数
	m_fastCombustionTableWidget->setColumnCount(7); // 列数
	m_fastCombustionTableWidget->verticalHeader()->setVisible(false);
	m_fastCombustionTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_fastCombustionData = {
		{ "设计点", "壳体厚度[mm]", "快烤平均温度[℃]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "600", " ", " ", " ", " " },
		{ "2", "2", "600", " ", " ", " ", " " },
		{ "3", "3", "600", " ", " ", " ", " " },
		{ "4", "1", "700", " ", " ", " ", " " },
		{ "5", "2", "700", " ", " ", " ", " " },
		{ "6", "3", "700", " ", " ", " ", " " },
		{ "7", "1", "800", " ", " ", " ", " " },
		{ "8", "2", "800", " ", " ", " ", " " },
		{ "9", "3", "800", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_fastCombustionData.size(); ++i) {
		for (int j = 0; j < m_fastCombustionData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_fastCombustionData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_fastCombustionTableWidget->setItem(i, j, item);
		}
	}
	m_fastCombustionTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_slowCombustionTableWidget = new QTableWidget();
	m_slowCombustionTableWidget->setRowCount(10); // 行数
	m_slowCombustionTableWidget->setColumnCount(7); // 列数
	m_slowCombustionTableWidget->verticalHeader()->setVisible(false);
	m_slowCombustionTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_slowCombustionData = {
		{ "设计点", "壳体厚度[mm]", "烘箱终止温度[℃]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "315", " ", " ", " ", " " },
		{ "2", "2", "315", " ", " ", " ", " " },
		{ "3", "3", "315", " ", " ", " ", " " },
		{ "4", "1", "330", " ", " ", " ", " " },
		{ "5", "2", "330", " ", " ", " ", " " },
		{ "6", "3", "330", " ", " ", " ", " " },
		{ "7", "1", "345", " ", " ", " ", " " },
		{ "8", "2", "345", " ", " ", " ", " " },
		{ "9", "3", "345", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_slowCombustionData.size(); ++i) {
		for (int j = 0; j < m_slowCombustionData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_slowCombustionData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_slowCombustionTableWidget->setItem(i, j, item);
		}
	}
	m_slowCombustionTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_shootTableWidget = new QTableWidget();
	m_shootTableWidget->setRowCount(10); // 行数
	m_shootTableWidget->setColumnCount(7); // 列数
	m_shootTableWidget->verticalHeader()->setVisible(false);
	m_shootTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_shootData = {
		{ "设计点", "壳体厚度[mm]", "撞击速度[m/s]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "620", " ", " ", " ", " " },
		{ "2", "2", "620", " ", " ", " ", " " },
		{ "3", "3", "620", " ", " ", " ", " " },
		{ "4", "1", "720", " ", " ", " ", " " },
		{ "5", "2", "720", " ", " ", " ", " " },
		{ "6", "3", "720", " ", " ", " ", " " },
		{ "7", "1", "820", " ", " ", " ", " " },
		{ "8", "2", "820", " ", " ", " ", " " },
		{ "9", "3", "820", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_shootData.size(); ++i) {
		for (int j = 0; j < m_shootData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_shootData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_shootTableWidget->setItem(i, j, item);
		}
	}
	m_shootTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_jetImpactTableWidget = new QTableWidget();
	m_jetImpactTableWidget->setRowCount(10); // 行数
	m_jetImpactTableWidget->setColumnCount(7); // 列数
	m_jetImpactTableWidget->verticalHeader()->setVisible(false);
	m_jetImpactTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_jetImpactData = {
		{ "设计点", "壳体厚度[mm]", "撞击速度[m/s]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "1630", " ", " ", " ", " " },
		{ "2", "2", "1630", " ", " ", " ", " " },
		{ "3", "3", "1630", " ", " ", " ", " " },
		{ "4", "1", "1730", " ", " ", " ", " " },
		{ "5", "2", "1730", " ", " ", " ", " " },
		{ "6", "3", "1730", " ", " ", " ", " " },
		{ "7", "1", "1830", " ", " ", " ", " " },
		{ "8", "2", "1830", " ", " ", " ", " " },
		{ "9", "3", "1830", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_jetImpactData.size(); ++i) {
		for (int j = 0; j < m_jetImpactData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_jetImpactData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_jetImpactTableWidget->setItem(i, j, item);
		}
	}
	m_jetImpactTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_fragmentationImpactTableWidget = new QTableWidget();
	m_fragmentationImpactTableWidget->setRowCount(10); // 行数
	m_fragmentationImpactTableWidget->setColumnCount(7); // 列数
	m_fragmentationImpactTableWidget->verticalHeader()->setVisible(false);
	m_fragmentationImpactTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_fragmentationImpactData = {
		{ "设计点", "壳体厚度[mm]", "聚能装药口径[mm]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "30", " ", " ", " ", " " },
		{ "2", "2", "30", " ", " ", " ", " " },
		{ "3", "3", "30", " ", " ", " ", " " },
		{ "4", "1", "40", " ", " ", " ", " " },
		{ "5", "2", "40", " ", " ", " ", " " },
		{ "6", "3", "40", " ", " ", " ", " " },
		{ "7", "1", "50", " ", " ", " ", " " },
		{ "8", "2", "50", " ", " ", " ", " " },
		{ "9", "3", "50", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_fragmentationImpactData.size(); ++i) {
		for (int j = 0; j < m_fragmentationImpactData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_fragmentationImpactData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_fragmentationImpactTableWidget->setItem(i, j, item);
		}
	}
	m_fragmentationImpactTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_explosiveBlastTableWidget = new QTableWidget();
	m_explosiveBlastTableWidget->setRowCount(10); // 行数
	m_explosiveBlastTableWidget->setColumnCount(7); // 列数
	m_explosiveBlastTableWidget->verticalHeader()->setVisible(false);
	m_explosiveBlastTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_explosiveBlastData = {
		{ "设计点", "壳体厚度[mm]", "TNT当量[mm]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "3", " ", " ", " ", " " },
		{ "2", "2", "3", " ", " ", " ", " " },
		{ "3", "3", "3", " ", " ", " ", " " },
		{ "4", "1", "4", " ", " ", " ", " " },
		{ "5", "2", "4", " ", " ", " ", " " },
		{ "6", "3", "4", " ", " ", " ", " " },
		{ "7", "1", "5", " ", " ", " ", " " },
		{ "8", "2", "6", " ", " ", " ", " " },
		{ "9", "3", "6", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_explosiveBlastData.size(); ++i) {
		for (int j = 0; j < m_explosiveBlastData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_explosiveBlastData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_explosiveBlastTableWidget->setItem(i, j, item);
		}
	}
	m_explosiveBlastTableWidget->resizeColumnsToContents(); // 根据内容调整列宽

	m_sacrificeExplosionTableWidget = new QTableWidget();
	m_sacrificeExplosionTableWidget->setRowCount(10); // 行数
	m_sacrificeExplosionTableWidget->setColumnCount(7); // 列数
	m_sacrificeExplosionTableWidget->verticalHeader()->setVisible(false);
	m_sacrificeExplosionTableWidget->horizontalHeader()->setVisible(false);
	std::vector<std::vector<QString>> m_sacrificeExplosionData = {
		{ "设计点", "壳体厚度[mm]", "殉爆距离[mm]", "壳体最大应力[MPa]", "推进剂最大应力[MPa]", "壳体最高温度[℃]", "推进剂最高温度[℃]" },
		{ "1", "1", "80", " ", " ", " ", " " },
		{ "2", "2", "80", " ", " ", " ", " " },
		{ "3", "3", "80", " ", " ", " ", " " },
		{ "4", "1", "90", " ", " ", " ", " " },
		{ "5", "2", "90", " ", " ", " ", " " },
		{ "6", "3", "90", " ", " ", " ", " " },
		{ "7", "1", "100", " ", " ", " ", " " },
		{ "8", "2", "100", " ", " ", " ", " " },
		{ "9", "3", "100", " ", " ", " ", " " },
	};
	for (int i = 0; i < m_sacrificeExplosionData.size(); ++i) {
		for (int j = 0; j < m_sacrificeExplosionData[i].size(); ++j) {
			QTableWidgetItem* item = new QTableWidgetItem(m_sacrificeExplosionData[i][j]);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
			if (i == 0)
			{
				item->setBackground(QBrush(QColor(0, 237, 252)));
			}
			m_sacrificeExplosionTableWidget->setItem(i, j, item);
		}
	}
	m_sacrificeExplosionTableWidget->resizeColumnsToContents(); // 根据内容调整列宽


	m_tableStackWidget->addWidget(m_fallTableWidget);
	m_tableStackWidget->addWidget(m_fastCombustionTableWidget);
	m_tableStackWidget->addWidget(m_slowCombustionTableWidget);
	m_tableStackWidget->addWidget(m_shootTableWidget);
	m_tableStackWidget->addWidget(m_jetImpactTableWidget);
	m_tableStackWidget->addWidget(m_fragmentationImpactTableWidget);
	m_tableStackWidget->addWidget(m_explosiveBlastTableWidget);
	m_tableStackWidget->addWidget(m_sacrificeExplosionTableWidget);



	graphicWid = new QWidget();
	graphicLayout = new QHBoxLayout();
	QLabel* x_label = new QLabel("X轴：");
	x_comboBox = new QComboBox();
	x_comboBox->addItems({ "壳体厚度", "撞击速度" });
	QLabel* y_label = new QLabel("Y轴：");
	y_comboBox = new QComboBox();
	y_comboBox->addItems({ "壳体最大应力 ", "进剂最大应力", "壳体最高温度", "推进剂最高温度"});
	// 连接信号槽
	connect(x_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &IntelligentAnalyWidget::onComboBoxIndexChanged);

	connect(y_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &IntelligentAnalyWidget::onComboBoxIndexChanged);


	chart = new QChart();
	chart->setTitle("正交试验");
	chart->legend()->hide();  // 隐藏图例

	// 创建坐标轴
	QValueAxis* axisX = new QValueAxis();
	QValueAxis* axisY = new QValueAxis();
	chart->addAxis(axisX, Qt::AlignBottom);
	chart->addAxis(axisY, Qt::AlignLeft);

	chartView = new QChartView(chart);
	chartView->setRenderHint(QPainter::Antialiasing);  // 抗锯齿
	chartView->setMinimumHeight(400);


	// 构建布局
	QVBoxLayout* m_leftLayout = new QVBoxLayout();

	QHBoxLayout* labelLayou = new QHBoxLayout();
	labelLayou->addWidget(x_label);
	labelLayou->addWidget(x_comboBox);
	labelLayou->addWidget(y_label);
	labelLayou->addWidget(y_comboBox);
	labelLayou->setSpacing(0);
	labelLayou->setContentsMargins(0,0,0,0);
	labelLayou->addStretch(200);

	m_leftLayout->addLayout(labelLayou);
	m_leftLayout->addWidget(chartView);


	QChart* chart1 = new QChart();
	QChartView* chartView2 = new QChartView(chart1);


	graphicLayout->addLayout(m_leftLayout, 1);
	graphicLayout->addWidget(chartView2, 1);
	graphicWid->setLayout(graphicLayout);



	



	// ------ 左侧垂直分割器（树结构与属性表） ------
	auto leftSplitter = new QSplitter(Qt::Vertical);
	leftSplitter->addWidget(m_treeModelWidget);
	leftSplitter->addWidget(m_propertyStackWidget);
	leftSplitter->setStretchFactor(0, 3);
	leftSplitter->setStretchFactor(1, 1);
	leftSplitter->setContentsMargins(0, 0, 0, 0);
	// 设置分割器的Handle宽度为0（消除视觉间隙）
	leftSplitter->setHandleWidth(1);



	// ------ 右侧垂直分割器（树结构与属性表） ------
	auto rightSplitter = new QSplitter(Qt::Vertical);
	rightSplitter->addWidget(m_tableStackWidget);
	rightSplitter->addWidget(graphicWid);
	rightSplitter->setStretchFactor(0, 3);
	rightSplitter->setStretchFactor(1, 1);
	rightSplitter->setContentsMargins(0, 0, 0, 0);
	// 设置分割器的Handle宽度为0（消除视觉间隙）
	rightSplitter->setHandleWidth(1);


	// ------ 主水平分割器（左侧与右侧） ------
	auto mainSplitter = new QSplitter(Qt::Horizontal);
	mainSplitter->addWidget(leftSplitter);
	mainSplitter->addWidget(rightSplitter);
	mainSplitter->setContentsMargins(0, 0, 0, 0);
	// 设置分割器的Handle宽度为0（消除视觉间隙）
	mainSplitter->setHandleWidth(1);

	mainSplitter->setStretchFactor(0, 2);
	mainSplitter->setStretchFactor(1, 8);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(mainSplitter);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	// 连接信号和槽
	connect(m_treeModelWidget, &IntelligentAnalyTreeWidget::itemClicked, this, &IntelligentAnalyWidget::onTreeItemClicked);
}

IntelligentAnalyWidget::~IntelligentAnalyWidget()
{}


void IntelligentAnalyWidget::onTreeItemClicked(const QString& itemData)
{
	if (itemData == "IntelligentAnaly") {
		m_propertyStackWidget->setCurrentWidget(m_intelligentPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_fallTableWidget);
		x_comboBox->setItemText(1, "跌落高度");
	}
	else if (itemData == "FallIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_fallPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_fallTableWidget);
		x_comboBox->setItemText(1, "跌落高度");
	}
	
	else if (itemData == "FastCombustionIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_fastCombustionPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_fastCombustionTableWidget);
		x_comboBox->setItemText(1, "快烤平均温度");
	}
	else if (itemData == "SlowCombustionIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_slowCombustionPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_slowCombustionTableWidget);
		x_comboBox->setItemText(1, "慢烤平均温度");
	}
	else if (itemData == "ShootIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_shootPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_shootTableWidget);
		x_comboBox->setItemText(1, "撞击速度温度");
	}
	else if (itemData == "JetImpactIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_jetImpactPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_jetImpactTableWidget);
		x_comboBox->setItemText(1, "撞击速度温度");
	}
	else if (itemData == "FragmentationImpactIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_fragmentationImpactPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_fragmentationImpactTableWidget);
		x_comboBox->setItemText(1, "聚能装药口径");
	}
	else if (itemData == "ExplosiveBlastIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_explosiveBlastPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_explosiveBlastTableWidget);
		x_comboBox->setItemText(1, "TNT当量");
	}
	else if (itemData == "SacrificeExplosionIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_sacrificeExplosionPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_sacrificeExplosionTableWidget);
		x_comboBox->setItemText(1, "殉爆距离");
	}
}


void IntelligentAnalyWidget::updateChartData(QVector<QPointF> data, QString xAxisTitle, QString yAxisTitle)
{
	if (!chartView || !chartView->chart()) return;

	QChart* chart = chartView->chart();

	// 清除原有系列和坐标轴
	chart->removeAllSeries();
	chart->removeAxis(chart->axisX());
	chart->removeAxis(chart->axisY());

	// 创建并添加数据
	QLineSeries* series = new QLineSeries();
	*series << data[0] << data[1] << data[2];
	series->setColor(Qt::blue);

	// 添加系列到图表并关联坐标轴
	chart->addSeries(series);
	
	chart->createDefaultAxes();
	// 设置坐标轴名称
	chart->axisX()->setTitleText(xAxisTitle);
	chart->axisY()->setTitleText(yAxisTitle);

	chartView->update();

}

void IntelligentAnalyWidget::onComboBoxIndexChanged(int index)
{
	QVector<QPointF> data1 = { QPointF(1, 2), QPointF(2, 4), QPointF(3, 6) };
	QVector<QPointF> data2 = { QPointF(1, 30), QPointF(2, 50), QPointF(3, 70) };
	if (index >= 0 ) {
		if (index == 0)
		{
			updateChartData(data1, x_comboBox->currentText(), y_comboBox->currentText());
		}
		else
		{
			updateChartData(data2, x_comboBox->currentText(), y_comboBox->currentText());
		}
	}
}
