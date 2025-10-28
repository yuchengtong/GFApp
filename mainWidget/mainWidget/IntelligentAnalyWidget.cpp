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
		{ "8", "2", "30", " ", " ", " ", " " },
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
	m_tableWidget = m_fallTableWidget;

	m_dataMap["壳体厚度"] = QStringList() << "1" << "2" << "3";
	m_dataMap["跌落高度"] = QStringList() << "10" << "20" << "30";
	m_dataMap["快烤平均温度"] = QStringList() << "600" << "700" << "800";
	m_dataMap["慢烤平均温度"] = QStringList() << "315" << "330" << "345";
	m_dataMap["子弹撞击速度"] = QStringList() << "620" << "720" << "820";
	m_dataMap["破片撞击速度"] = QStringList() << "1630" << "1730" << "1830";
	m_dataMap["聚能装药口径"] = QStringList() << "30" << "40" << "50";
	m_dataMap["TNT当量"] = QStringList() << "3" << "4" << "5";
	m_dataMap["殉爆距离"] = QStringList() << "80" << "90" << "100";

	graphicWid = new QWidget();
	graphicLayout = new QHBoxLayout();
	QLabel* x_label = new QLabel("X轴：");
	x_comboBox = new QComboBox();
	x_comboBox->addItems({ "壳体厚度", "跌落高度" });
	x_comboBox->setFixedWidth(180);
	QLabel* y_label = new QLabel("Y轴：");
	y_comboBox = new QComboBox();
	y_comboBox->addItems({ "壳体最大应力 ", "进剂最大应力", "壳体最高温度", "推进剂最高温度"});
	x_valueLable = new QLabel("跌落高度：");
	x_valueComboBox = new QComboBox();
	x_valueComboBox->addItems({ "10", "20", "30"});
	x_valueComboBox->setFixedWidth(80);
	// 连接信号槽
	connect(x_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &IntelligentAnalyWidget::onComboBoxIndexChanged);

	connect(y_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &IntelligentAnalyWidget::onComboBoxIndexChanged);

	connect(x_valueComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &IntelligentAnalyWidget::dataChange);

	

	


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
	labelLayou->addWidget(x_valueLable);
	labelLayou->addWidget(x_valueComboBox);
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
		m_tableWidget = m_fallTableWidget;
		x_comboBox->setItemText(1, "跌落高度");
	}
	else if (itemData == "FallIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_fallPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_fallTableWidget);
		m_tableWidget = m_fallTableWidget;
		x_comboBox->setItemText(1, "跌落高度");
	}
	
	else if (itemData == "FastCombustionIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_fastCombustionPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_fastCombustionTableWidget);
		m_tableWidget = m_fastCombustionTableWidget;
		x_comboBox->setItemText(1, "快烤平均温度");
	}
	else if (itemData == "SlowCombustionIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_slowCombustionPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_slowCombustionTableWidget);
		m_tableWidget = m_slowCombustionTableWidget;
		x_comboBox->setItemText(1, "慢烤平均温度");
	}
	else if (itemData == "ShootIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_shootPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_shootTableWidget);
		m_tableWidget = m_shootTableWidget;
		x_comboBox->setItemText(1, "子弹撞击速度");
	}
	else if (itemData == "JetImpactIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_jetImpactPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_jetImpactTableWidget);
		m_tableWidget = m_jetImpactTableWidget;
		x_comboBox->setItemText(1, "破片撞击速度");
	}
	else if (itemData == "FragmentationImpactIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_fragmentationImpactPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_fragmentationImpactTableWidget);
		m_tableWidget = m_fragmentationImpactTableWidget;
		x_comboBox->setItemText(1, "聚能装药口径");
	}
	else if (itemData == "ExplosiveBlastIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_explosiveBlastPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_explosiveBlastTableWidget);
		m_tableWidget = m_explosiveBlastTableWidget;
		x_comboBox->setItemText(1, "TNT当量");
	}
	else if (itemData == "SacrificeExplosionIntelligentAnaly")
	{
		m_propertyStackWidget->setCurrentWidget(m_sacrificeExplosionPropertyWidget);
		m_tableStackWidget->setCurrentWidget(m_sacrificeExplosionTableWidget);
		m_tableWidget = m_sacrificeExplosionTableWidget;
		x_comboBox->setItemText(1, "殉爆距离");
	}

	onComboBoxIndexChanged(0);
}

void IntelligentAnalyWidget::onComboBoxIndexChanged(int index)
{
	// 修改第三个下拉框的值
	int x_index = x_comboBox->currentIndex();
	int other = 0;
	if (x_index == 0)
	{
		other = 1;
	}
	else
	{
		other = 0;
	}
	QString text = x_comboBox->itemText(other);
	x_comboBox->currentIndex();
	x_valueComboBox->clear();
	QStringList childItems = m_dataMap.value(text);
	x_valueComboBox->addItems(childItems);
	x_valueLable->setText(text + ":");
	

	dataChange(0);
	
}

void IntelligentAnalyWidget::dataChange(int index)
{
	QList<int> rowList;
	int x_col = 3;
	int y_col = 1;

	int x_index = x_comboBox->currentIndex();
	QString x_valueindex = x_valueComboBox->currentIndex();
	int y_index = y_comboBox->currentIndex();
	if (x_index == 0)
	{
		if (x_valueindex == 0)
		{
			rowList = { 1,2,3 };
		}
		else if (x_valueindex == 1)
		{
			rowList = { 4,5,6 };
		}
		else
		{
			rowList = { 7,8,9 };
		}
		x_col = 1;
	}
	else
	{
		if (x_valueindex == 0)
		{
			rowList = { 1,4,7 };
		}
		else if (x_valueindex == 1)
		{
			rowList = { 2,5,8 };
		}
		else
		{
			rowList = { 3,6,9 };
		}
		x_col = 2;
	}


	if (y_index == 0)
	{
		y_col = 3;
	}
	else if (y_index == 1)
	{
		y_col = 4;
	}
	else if (y_index == 2)
	{
		y_col = 5;
	}
	else if (y_index == 3)
	{
		y_col = 6;
	}

	QVector<QPointF> data;
	foreach(int row, rowList) {
		if (m_tableWidget->item(row, x_col) && m_tableWidget->item(row, y_col))
		{
			data.append(QPointF(m_tableWidget->item(row, x_col)->text().toDouble(), m_tableWidget->item(row, y_col)->text().toDouble()));
		}
	}
	updateChartData(data, x_comboBox->currentText(), y_comboBox->currentText());

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

	// 创建坐标轴（X轴和Y轴）
	QValueAxis* axisX = new QValueAxis();
	QValueAxis* axisY = new QValueAxis();

	// 设置坐标轴名称
	axisX->setTitleText(xAxisTitle);
	axisY->setTitleText(yAxisTitle);

	qreal maxX = calculateMaxValue(data, true);
	qreal maxY = calculateMaxValue(data, false);

	// 设置X轴范围（最小值固定为0，最大值为计算值）
	axisX->setRange(0, maxX);
	// 设置Y轴范围（最小值固定为0，最大值为计算值）
	axisY->setRange(0, maxY);

	// 添加坐标轴到图表
	chart->addAxis(axisX, Qt::AlignBottom);
	chart->addAxis(axisY, Qt::AlignLeft);

	// 将数据系列关联到坐标轴
	series->attachAxis(axisX);
	series->attachAxis(axisY);

	chartView->update();

}


// 计算数据最大值（确保不小于0）
qreal IntelligentAnalyWidget::calculateMaxValue(const QVector<QPointF>& series, bool isX)
{
	if (series.isEmpty()) return 0;

	qreal maxVal = isX ? series.first().x() : series.first().y();
	for (const QPointF& point : series) {
		qreal val = isX ? point.x() : point.y();
		if (val > maxVal) {
			maxVal = val;
		}
	}

	// 确保最大值不小于0
	return qMax(maxVal, 0.0);
}
