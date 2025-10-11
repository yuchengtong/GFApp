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
	m_PropertyStackWidget = new QStackedWidget();
	m_tableWidget = new QTableWidget();
	m_tableWidget->setRowCount(10); // 行数
	m_tableWidget->setColumnCount(7); // 列数
	// 隐藏行号
	m_tableWidget->verticalHeader()->setVisible(false);
	// 隐藏列号
	m_tableWidget->horizontalHeader()->setVisible(false);

	// 假设你已经有一个QTableWidget实例叫table
	m_tableWidget->setItem(0, 0, new QTableWidgetItem("序号"));
	m_tableWidget->setItem(0, 1, new QTableWidgetItem(""));
	m_tableWidget->setItem(0, 2, new QTableWidgetItem(""));
	m_tableWidget->setItem(0, 3, new QTableWidgetItem("壳体最大应力"));
	m_tableWidget->setItem(0, 4, new QTableWidgetItem("壳体最高温度"));
	m_tableWidget->setItem(0, 5, new QTableWidgetItem("推进剂最大应力"));
	m_tableWidget->setItem(0, 6, new QTableWidgetItem("推进剂最高温度"));

	m_tableWidget->setItem(1, 0, new QTableWidgetItem("1"));
	m_tableWidget->setItem(1, 1, new QTableWidgetItem("1"));
	m_tableWidget->setItem(1, 2, new QTableWidgetItem("10"));
	m_tableWidget->setItem(1, 3, new QTableWidgetItem(""));
	m_tableWidget->setItem(1, 4, new QTableWidgetItem(""));
	m_tableWidget->setItem(1, 5, new QTableWidgetItem(""));
	m_tableWidget->setItem(1, 6, new QTableWidgetItem(""));

	m_tableWidget->setItem(2, 0, new QTableWidgetItem("2"));
	m_tableWidget->setItem(2, 1, new QTableWidgetItem("2"));
	m_tableWidget->setItem(2, 2, new QTableWidgetItem("10"));
	m_tableWidget->setItem(2, 3, new QTableWidgetItem(""));
	m_tableWidget->setItem(2, 4, new QTableWidgetItem(""));
	m_tableWidget->setItem(2, 5, new QTableWidgetItem(""));
	m_tableWidget->setItem(2, 6, new QTableWidgetItem(""));

	m_tableWidget->setItem(3, 0, new QTableWidgetItem("3"));
	m_tableWidget->setItem(3, 1, new QTableWidgetItem("3"));
	m_tableWidget->setItem(3, 2, new QTableWidgetItem("10"));
	m_tableWidget->setItem(3, 3, new QTableWidgetItem(""));
	m_tableWidget->setItem(3, 4, new QTableWidgetItem(""));
	m_tableWidget->setItem(3, 5, new QTableWidgetItem(""));
	m_tableWidget->setItem(3, 6, new QTableWidgetItem(""));

	m_tableWidget->setItem(4, 0, new QTableWidgetItem("4"));
	m_tableWidget->setItem(4, 1, new QTableWidgetItem("1"));
	m_tableWidget->setItem(4, 2, new QTableWidgetItem("20"));
	m_tableWidget->setItem(4, 3, new QTableWidgetItem(""));
	m_tableWidget->setItem(4, 4, new QTableWidgetItem(""));
	m_tableWidget->setItem(4, 5, new QTableWidgetItem(""));
	m_tableWidget->setItem(4, 6, new QTableWidgetItem(""));

	m_tableWidget->setItem(5, 0, new QTableWidgetItem("5"));
	m_tableWidget->setItem(5, 1, new QTableWidgetItem("2"));
	m_tableWidget->setItem(5, 2, new QTableWidgetItem("20"));
	m_tableWidget->setItem(5, 3, new QTableWidgetItem(""));
	m_tableWidget->setItem(5, 4, new QTableWidgetItem(""));
	m_tableWidget->setItem(5, 5, new QTableWidgetItem(""));
	m_tableWidget->setItem(5, 6, new QTableWidgetItem(""));

	m_tableWidget->setItem(6, 0, new QTableWidgetItem("6"));
	m_tableWidget->setItem(6, 1, new QTableWidgetItem("3"));
	m_tableWidget->setItem(6, 2, new QTableWidgetItem("20"));
	m_tableWidget->setItem(6, 3, new QTableWidgetItem(""));
	m_tableWidget->setItem(6, 4, new QTableWidgetItem(""));
	m_tableWidget->setItem(6, 5, new QTableWidgetItem(""));
	m_tableWidget->setItem(6, 6, new QTableWidgetItem(""));

	m_tableWidget->setItem(7, 0, new QTableWidgetItem("7"));
	m_tableWidget->setItem(7, 1, new QTableWidgetItem("1"));
	m_tableWidget->setItem(7, 2, new QTableWidgetItem("30"));
	m_tableWidget->setItem(7, 3, new QTableWidgetItem(""));
	m_tableWidget->setItem(7, 4, new QTableWidgetItem(""));
	m_tableWidget->setItem(7, 5, new QTableWidgetItem(""));
	m_tableWidget->setItem(7, 6, new QTableWidgetItem(""));

	m_tableWidget->setItem(8, 0, new QTableWidgetItem("8"));
	m_tableWidget->setItem(8, 1, new QTableWidgetItem("2"));
	m_tableWidget->setItem(8, 2, new QTableWidgetItem("30"));
	m_tableWidget->setItem(8, 3, new QTableWidgetItem(""));
	m_tableWidget->setItem(8, 4, new QTableWidgetItem(""));
	m_tableWidget->setItem(8, 5, new QTableWidgetItem(""));
	m_tableWidget->setItem(8, 6, new QTableWidgetItem(""));

	m_tableWidget->setItem(9, 0, new QTableWidgetItem("9"));
	m_tableWidget->setItem(9, 1, new QTableWidgetItem("3"));
	m_tableWidget->setItem(9, 2, new QTableWidgetItem("30"));
	m_tableWidget->setItem(9, 3, new QTableWidgetItem(""));
	m_tableWidget->setItem(9, 4, new QTableWidgetItem(""));
	m_tableWidget->setItem(9, 5, new QTableWidgetItem(""));
	m_tableWidget->setItem(9, 6, new QTableWidgetItem(""));

	




	// 设置m_PropertyStackWidget的背景为白色
	m_PropertyStackWidget->setStyleSheet("background-color: white;");

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
	m_PropertyStackWidget->addWidget(m_intelligentPropertyWidget);
	m_PropertyStackWidget->addWidget(m_fallPropertyWidget);
	m_PropertyStackWidget->addWidget(m_fastCombustionPropertyWidget);
	m_PropertyStackWidget->addWidget(m_slowCombustionPropertyWidget);
	m_PropertyStackWidget->addWidget(m_shootPropertyWidget);
	m_PropertyStackWidget->addWidget(m_jetImpactPropertyWidget);
	m_PropertyStackWidget->addWidget(m_fragmentationImpactPropertyWidget);
	m_PropertyStackWidget->addWidget(m_explosiveBlastPropertyWidget);
	m_PropertyStackWidget->addWidget(m_sacrificeExplosionPropertyWidget);


	// ------ 左侧垂直分割器（树结构与属性表） ------
	auto leftSplitter = new QSplitter(Qt::Vertical);
	leftSplitter->addWidget(m_treeModelWidget);
	leftSplitter->addWidget(m_PropertyStackWidget);
	leftSplitter->setStretchFactor(0, 3);
	leftSplitter->setStretchFactor(1, 1);
	leftSplitter->setContentsMargins(0, 0, 0, 0);
	// 设置分割器的Handle宽度为0（消除视觉间隙）
	leftSplitter->setHandleWidth(1);



	// ------ 右侧垂直分割器（树结构与属性表） ------
	auto rightSplitter = new QSplitter(Qt::Vertical);
	rightSplitter->addWidget(m_tableWidget);
	rightSplitter->addWidget(new QWidget());
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
		m_PropertyStackWidget->setCurrentWidget(m_intelligentPropertyWidget);
	}
	else if (itemData == "FallIntelligentAnaly")
	{
		m_PropertyStackWidget->setCurrentWidget(m_fallPropertyWidget);
	}
	
	else if (itemData == "FastCombustionIntelligentAnaly")
	{
		m_PropertyStackWidget->setCurrentWidget(m_fastCombustionPropertyWidget);
	}
	else if (itemData == "SlowCombustionIntelligentAnaly")
	{
		m_PropertyStackWidget->setCurrentWidget(m_slowCombustionPropertyWidget);
	}
	else if (itemData == "ShootIntelligentAnaly")
	{
		m_PropertyStackWidget->setCurrentWidget(m_shootPropertyWidget);
	}
	else if (itemData == "JetImpactIntelligentAnaly")
	{
		m_PropertyStackWidget->setCurrentWidget(m_jetImpactPropertyWidget);
	}
	else if (itemData == "FragmentationImpactIntelligentAnaly")
	{
		m_PropertyStackWidget->setCurrentWidget(m_fragmentationImpactPropertyWidget);
	}
	else if (itemData == "ExplosiveBlastIntelligentAnaly")
	{
		m_PropertyStackWidget->setCurrentWidget(m_explosiveBlastPropertyWidget);
	}
	else if (itemData == "SacrificeExplosionIntelligentAnaly")
	{
		m_PropertyStackWidget->setCurrentWidget(m_sacrificeExplosionPropertyWidget);
	}
}
