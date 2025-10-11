#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QStackedWidget>

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

private:

	QStackedWidget* m_PropertyStackWidget = nullptr;
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

};
