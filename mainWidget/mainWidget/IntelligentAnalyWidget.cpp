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
	y_comboBox->addItems({ "壳体最大应力 ", "推进剂最大应力", "壳体最高温度", "推进剂最高温度"});
	
	// 连接信号槽
	connect(x_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &IntelligentAnalyWidget::onComboBoxIndexChanged);

	connect(y_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &IntelligentAnalyWidget::onComboBoxIndexChanged);

	

	m_chart = new QChart();
	m_chart->setTitle("正交试验");
	m_chart->setMargins(QMargins(15, 15, 15, 80));
	createChartDataGroup(m_lineSeries1, m_scatter1, "数据组1", Qt::red);
	createChartDataGroup(m_lineSeries2, m_scatter2, "数据组2", Qt::blue);
	createChartDataGroup(m_lineSeries3, m_scatter3, "数据组3", Qt::green);

	m_chart->addSeries(m_lineSeries1);
	m_chart->addSeries(m_scatter1);
	m_chart->addSeries(m_lineSeries2);
	m_chart->addSeries(m_scatter2);
	m_chart->addSeries(m_lineSeries3);
	m_chart->addSeries(m_scatter3);

	m_chart->legend()->hide();
	//m_chart->legend()->setAlignment(Qt::AlignBottom);

	// 4. 创建X轴和Y轴（初始范围适配空数据）
	m_axisX = new QValueAxis();
	m_axisY = new QValueAxis();
	m_axisX->setTitleText("壳体厚度");
	m_axisY->setTitleText("壳体最大应力");
	m_axisX->setRange(0, 5);
	m_axisY->setRange(0, 10);
	m_axisX->setTickCount(4);
	m_axisY->setTickCount(4);

	// 5. 所有系列绑定到同一组轴（确保曲线和点对齐）
	QList<QAbstractSeries*> allSeries = {
		m_lineSeries1, m_scatter1, m_lineSeries2, m_scatter2, m_lineSeries3, m_scatter3
	};
	for (QAbstractSeries* series : allSeries) {
		m_chart->setAxisX(m_axisX, series);
		m_chart->setAxisY(m_axisY, series);
	}

	m_chartView = new QChartView(m_chart);
	m_chartView->setBackgroundBrush(QBrush(Qt::white));
	m_chartView->setRenderHint(QPainter::Antialiasing);  // 抗锯齿
	m_chartView->setMinimumHeight(400);


	// 构建布局
	QVBoxLayout* m_leftLayout = new QVBoxLayout();

	QHBoxLayout* labelLayou = new QHBoxLayout();
	labelLayou->addWidget(x_label);
	labelLayou->addWidget(x_comboBox);
	labelLayou->addWidget(y_label);
	labelLayou->addWidget(y_comboBox);
	labelLayou->setContentsMargins(0,0,0,0);
	labelLayou->addStretch(200);

	m_leftLayout->addLayout(labelLayou);
	m_leftLayout->addWidget(m_chartView);



	// 三维图形

	

	QLabel* grapgicLabel = new QLabel("结果集：");
	m_grapgicComboBox = new QComboBox();
	m_grapgicComboBox->addItems({ "壳体最大应力 ", "推进剂最大应力", "壳体最高温度", "推进剂最高温度" });
	connect(m_grapgicComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &IntelligentAnalyWidget::onComboBoxIndexGraphicChanged);

	QLabel* hspinBoxLabel = new QLabel("水平：");
	QSpinBox* hspinBox = new QSpinBox();
	hspinBox->setMinimum(-180);
	hspinBox->setMaximum(180);
	hspinBox->setSingleStep(5);
	hspinBox->setMinimumWidth(200);
	hspinBox->setSuffix("°");
	connect(hspinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		this, &IntelligentAnalyWidget::hspinChange);

	QLabel* vspinBoxLabel = new QLabel("垂直：");
	QSpinBox* vspinBox = new QSpinBox();
	vspinBox->setMinimum(0);
	vspinBox->setMaximum(90);
	vspinBox->setSingleStep(5);
	vspinBox->setMinimumWidth(200);
	vspinBox->setSuffix("°");
	connect(vspinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		this, &IntelligentAnalyWidget::vspinChange);


	QHBoxLayout* choseLayou = new QHBoxLayout();
	choseLayou->addWidget(grapgicLabel);
	choseLayou->addWidget(m_grapgicComboBox);

	choseLayou->addWidget(hspinBoxLabel);
	choseLayou->addWidget(hspinBox);

	choseLayou->addWidget(vspinBoxLabel);
	choseLayou->addWidget(vspinBox);

	choseLayou->setContentsMargins(0, 0, 0, 0);
	choseLayou->addStretch(200);


	m_3dGraphicWid = new GraphicWidget();


	// 构建布局
	QVBoxLayout* m_rightLayout = new QVBoxLayout();
	m_rightLayout->addLayout(choseLayou);
	m_rightLayout->addWidget(m_3dGraphicWid);

	graphicLayout->addLayout(m_leftLayout, 1);
	graphicLayout->addLayout(m_rightLayout, 1);
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

	mainSplitter->setStretchFactor(0, 3);
	mainSplitter->setStretchFactor(1, 7);

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

	// 更新三维模型数据
	QVector<QVector<double>> newData;
	QVector<double> data1;
	data1.append(m_tableWidget->item(1, 3)->text().toDouble());
	data1.append(m_tableWidget->item(4, 3)->text().toDouble());
	data1.append(m_tableWidget->item(7, 3)->text().toDouble());
	newData.append(data1);
	QVector<double> data2;
	data2.append(m_tableWidget->item(2, 3)->text().toDouble());
	data2.append(m_tableWidget->item(5, 3)->text().toDouble());
	data2.append(m_tableWidget->item(8, 3)->text().toDouble());
	newData.append(data2);
	QVector<double> data3;
	data3.append(m_tableWidget->item(3, 3)->text().toDouble());
	data3.append(m_tableWidget->item(6, 3)->text().toDouble());
	data3.append(m_tableWidget->item(9, 3)->text().toDouble());
	newData.append(data3);

	QVector<double> xCoords;
	xCoords.append(m_tableWidget->item(1, 1)->text().toDouble());
	xCoords.append(m_tableWidget->item(2, 1)->text().toDouble());
	xCoords.append(m_tableWidget->item(3, 1)->text().toDouble());

	QVector<double> yCoords;
	yCoords.append(m_tableWidget->item(1, 2)->text().toDouble());
	yCoords.append(m_tableWidget->item(4, 2)->text().toDouble());
	yCoords.append(m_tableWidget->item(7, 2)->text().toDouble());


	updateGraphicData("", x_comboBox->itemText(1), "壳体最大应力", xCoords, yCoords, newData,
		m_tableWidget->item(1, 1)->text().toDouble(), m_tableWidget->item(3, 1)->text().toDouble(),
		m_tableWidget->item(1, 2)->text().toDouble(), m_tableWidget->item(7, 2)->text().toDouble());

}

void IntelligentAnalyWidget::onComboBoxIndexChanged(int index)
{
	x_comboBox->currentIndex();
	dataChange(0);
	
}
void IntelligentAnalyWidget::onComboBoxIndexGraphicChanged(int index)
{
	int z = 3;
	QString zName = "壳体最大应力";
	if (index == 0)
	{
		z = 3;
		zName = "壳体最大应力";
	}
	else if (index == 1)
	{
		z = 4;
		zName = "推进剂最大应力";
	}
	else if (index == 2)
	{
		z = 5;
		zName = "壳体最高温度";
	}
	else if (index == 3)
	{
		z = 6;
		zName = "推进剂最高温度";
	}
	// 更新三维模型数据
	QVector<QVector<double>> newData;
	QVector<double> data1;
	data1.append(m_tableWidget->item(1, z)->text().toDouble());
	data1.append(m_tableWidget->item(4, z)->text().toDouble());
	data1.append(m_tableWidget->item(7, z)->text().toDouble());
	newData.append(data1);
	QVector<double> data2;
	data2.append(m_tableWidget->item(2, z)->text().toDouble());
	data2.append(m_tableWidget->item(5, z)->text().toDouble());
	data2.append(m_tableWidget->item(8, z)->text().toDouble());
	newData.append(data2);
	QVector<double> data3;
	data3.append(m_tableWidget->item(3, z)->text().toDouble());
	data3.append(m_tableWidget->item(6, z)->text().toDouble());
	data3.append(m_tableWidget->item(9, z)->text().toDouble());
	newData.append(data3);

	QVector<double> xCoords;
	xCoords.append(m_tableWidget->item(1, 1)->text().toDouble());
	xCoords.append(m_tableWidget->item(2, 1)->text().toDouble());
	xCoords.append(m_tableWidget->item(3, 1)->text().toDouble());

	QVector<double> yCoords;
	yCoords.append(m_tableWidget->item(1, 2)->text().toDouble());
	yCoords.append(m_tableWidget->item(4, 2)->text().toDouble());
	yCoords.append(m_tableWidget->item(7, 2)->text().toDouble());



	updateGraphicData("", "", zName, xCoords, yCoords, newData,
		m_tableWidget->item(1, 1)->text().toDouble(), m_tableWidget->item(3, 1)->text().toDouble(),
		m_tableWidget->item(1, 2)->text().toDouble(), m_tableWidget->item(7, 2)->text().toDouble());


}

void IntelligentAnalyWidget::dataChange(int index)
{
	QList<int> rowList1;
	QList<int> rowList2;
	QList<int> rowList3;
	int x_col = 3;
	int y_col = 1;

	int x_index = x_comboBox->currentIndex();
	int y_index = y_comboBox->currentIndex();
	if (x_index == 0)
	{
		rowList1 = { 1,2,3 };
		rowList2 = { 4,5,6 };
		rowList3 = { 7,8,9 };
		x_col = 1;
	}
	else
	{
		rowList1 = { 1,4,7 };
		rowList2 = { 2,5,8 };
		rowList3 = { 3,6,9 };
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

	QVector<QPointF> data1;
	foreach(int row, rowList1) {
		if (m_tableWidget->item(row, x_col) && m_tableWidget->item(row, y_col))
		{
			data1.append(QPointF(m_tableWidget->item(row, x_col)->text().toDouble(), m_tableWidget->item(row, y_col)->text().toDouble()));
		}
	}
	QVector<QPointF> data2;
	foreach(int row, rowList2) {
		if (m_tableWidget->item(row, x_col) && m_tableWidget->item(row, y_col))
		{
			data2.append(QPointF(m_tableWidget->item(row, x_col)->text().toDouble(), m_tableWidget->item(row, y_col)->text().toDouble()));
		}
	}
	QVector<QPointF> data3;
	foreach(int row, rowList3) {
		if (m_tableWidget->item(row, x_col) && m_tableWidget->item(row, y_col))
		{
			data3.append(QPointF(m_tableWidget->item(row, x_col)->text().toDouble(), m_tableWidget->item(row, y_col)->text().toDouble()));
		}
	}
	updateChartData(data1, data2, data3, x_comboBox->currentText(), y_comboBox->currentText());

}


void IntelligentAnalyWidget::updateChartData(QVector<QPointF> data1, QVector<QPointF> data2, QVector<QPointF> data3, QString xAxisTitle, QString yAxisTitle)
{
	if (!m_chartView || !m_chart || !m_scatter1 || !m_scatter2 || !m_scatter3) return;

	// 同步更新：曲线和散点的坐标完全一致
	m_lineSeries1->clear();
	m_scatter1->clear();
	for (QPointF value : data1) {
		m_lineSeries1->append(value);
		m_scatter1->append(value);
	}
	

	m_lineSeries2->clear();
	m_scatter2->clear();
	for (QPointF value : data2) {
		m_lineSeries2->append(value);
		m_scatter2->append(value);
	}

	m_lineSeries3->clear();
	m_scatter3->clear();
	for (QPointF value : data3) {
		m_lineSeries3->append(value);
		m_scatter3->append(value);
	}

	// 自动调整轴范围（适配新数据）
	m_chart->createDefaultAxes();
	m_axisX = qobject_cast<QValueAxis*>(m_chart->axisX());
	m_axisY = qobject_cast<QValueAxis*>(m_chart->axisY());
	if (m_axisX) {
		m_axisY->setTitleText(xAxisTitle);
	}
	if (m_axisY) {
		m_axisY->setTitleText(yAxisTitle);
	}
	QVector<QPointF> data;
	data.append(data1);
	data.append(data2);
	data.append(data3);
	qreal maxX = calculateMaxValue(data, true);
	qreal maxY = calculateMaxValue(data, false);
	qreal minX = calculateMinValue(data, true);
	qreal minY = calculateMinValue(data, false);
	m_axisX->setRange(minX * 0.9, maxX * 1.1);
	m_axisY->setRange(minY * 0.9, maxY * 1.1);
	m_axisX->setTickCount(4);
	m_axisY->setTickCount(4);
	m_chartView->update();

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

// 计算数据最小值（确保不小于0）
qreal IntelligentAnalyWidget::calculateMinValue(const QVector<QPointF>& series, bool isX)
{
	if (series.isEmpty()) return 0;

	qreal minVal = isX ? series.first().x() : series.first().y();
	for (const QPointF& point : series) {
		qreal val = isX ? point.x() : point.y();
		if (val < minVal) {
			minVal = val;
		}
	}

	// 确保最小值不小于0
	return qMax(minVal, 0.0);
}

//创建一组数据（曲线 + 圆点），统一配置样式
void IntelligentAnalyWidget::createChartDataGroup(QLineSeries * &lineSeries, QScatterSeries * &scatterSeries,
	const QString & name, const QColor & color)
{
	// 1. 创建曲线系列（只负责线条）
	lineSeries = new QLineSeries();
	lineSeries->setName(name);  // 系列名称（与散点共享，图例只显示1次）
	QPen linePen(color);
	linePen.setWidth(2);        // 曲线宽度
	lineSeries->setPen(linePen);

	// 2. 创建散点系列（只负责圆点）
	scatterSeries = new QScatterSeries();
	scatterSeries->setName(name);  // 与曲线同名，图例合并显示
	scatterSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);  // 圆点（原生支持）
	scatterSeries->setMarkerSize(8);                                  // 圆点大小（8px）
	scatterSeries->setBrush(QBrush(color));                            // 圆点填充色
	scatterSeries->setPen(QPen(Qt::black, 1));                         // 圆点边框（黑色，1px）
}



void IntelligentAnalyWidget::hspinChange(int val)
{
	m_3dGraphicWid->on_angleValueChange(0, val);
}

void IntelligentAnalyWidget::vspinChange(int val)
{
	m_3dGraphicWid->on_angleValueChange(1, val);
}

void IntelligentAnalyWidget::updateGraphicData(QString xName, QString yName, QString zName, 
	const QVector<double>& xCoords,
	const QVector<double>& yCoords, 
	const QVector<QVector<double>>& newData,
	double xMin,
	double xMax,
	double yMin,
	double yMax)
{
	if (zName == "壳体最大应力")
	{
		m_grapgicComboBox->setCurrentIndex(0);
	} 
	else if (zName == "推进剂最大应力")
	{
		m_grapgicComboBox->setCurrentIndex(1);
	}
	else if (zName == "壳体最高温度")
	{
		m_grapgicComboBox->setCurrentIndex(2);
	}
	else if (zName == "推进剂最高温度")
	{
		m_grapgicComboBox->setCurrentIndex(3);
	}
	m_3dGraphicWid->axisTitleChange(xName, yName, zName);
	m_3dGraphicWid->dataUpdate(xCoords, yCoords, newData, newData.size(), newData.at(0).size(), xMin, xMax, yMin, yMax);
}