#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QStackedWidget>
#include <QtCharts>
#include <QChartView>
#include <QComboBox>
#include <QLabel>


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


class IntelligentAnalyWidget : public QWidget
{
	Q_OBJECT

public:
	IntelligentAnalyWidget(QWidget* parent = nullptr);
	~IntelligentAnalyWidget();

private slots:
	void onTreeItemClicked(const QString& itemData);

	void onComboBoxIndexChanged(int index);

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

	QChart* chart = nullptr;
	QComboBox* m_comboBox;
	QChartView* chartView;
	QChartView* chartView2;

	QHBoxLayout* graphicLayout;
	QVBoxLayout* m_leftLayout;


	// 更新图表数据
	void updateChartData(QVector<QPointF> data, QString xAxisTitle, QString yAxisTitle);
};
