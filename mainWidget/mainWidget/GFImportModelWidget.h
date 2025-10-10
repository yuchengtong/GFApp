#pragma once
#include <qwidget.h>
#include <QStackedWidget>

#include "GFTreeModelWidget.h"
#include "OccView.h"
#include "GFLogWidget.h"
#include "GeomPropertyWidget.h"
#include "MaterialPropertyWidget.h"
#include "MeshPropertyWidget.h"
#include "SettingPropertyWidget.h"
#include "ResultsPropertyWidget.h"
#include "FallPropertyWidget.h"
#include "FastCombustionPropertyWidget.h"
#include "SlowCombustionPropertyWidget.h"
#include "StressResultWidget.h"
#include "TemperatureResultWidget.h"
#include "OverpressureResultWidget.h"
#include "SteelPropertyWidget.h"
#include "PropellantPropertyWidget.h"
#include "ProjectPropertyWidge.h"
#include "CalculationPropertyWidget.h"
#include "JudgmentPropertyWidget.h"
#include "InsulatingheatPropertyWidget.h"
#include "OutheatPropertyWidget.h"
#include "StrainResultWidget.h"
#include "ShootPropertyWidget.h"
#include "JetImpactPropertyWidget.h"
#include "FragmentationImpactPropertyWidget.h"
#include "ExplosiveBlastPropertyWidget.h"
#include "SacrificeExplosionPropertyWidget.h"
#include "DatabasePropertyWidget.h"

class GFImportModelWidget :public QWidget
{
	Q_OBJECT
public:
	GFImportModelWidget(QWidget* parent = nullptr);
	~GFImportModelWidget();


	OccView* GetOccView() { return m_OccView; }

	GFLogWidget* GetLogWidget() { return m_LogWidget; }

	GFTreeModelWidget* GetGFTreeModelWidget() { return m_treeModelWidget; }

	StressResultWidget* GetStressResultWidget() { return m_stressResultWidget; }
	StrainResultWidget* GetStrainResultWidget() { return m_strainResultWidget; }
	TemperatureResultWidget* GetTemperatureResultWidget() { return m_temperatureResultWidget; }
	OverpressureResultWidget* GetOverpressureResultWidget() { return m_overpressureResultWidge; }

	ProjectPropertyWidge* GetProjectPropertyWidget() { return m_projectPropertyWidge; }
	GeomPropertyWidget* GetGeomPropertyWidget() { return m_geomPropertyWidget; }
	MaterialPropertyWidget* GetMaterialPropertyWidget() { return m_materialPropertyWidget; }
	DatabasePropertyWidget* GetDatabasePropertyWidget() { return m_databasePropertyWidget; }

	FallPropertyWidget* GetFallPropertyWidget() { return m_fallPropertyWidget; }

private slots:
	void onTreeItemClicked(const QString& itemData);

private:
	OccView* m_OccView = nullptr;
	GFLogWidget* m_LogWidget = nullptr;
	QStackedWidget* m_PropertyStackWidget = nullptr;
	GFTreeModelWidget* m_treeModelWidget = nullptr;

	GeomPropertyWidget* m_geomPropertyWidget = nullptr;
	MaterialPropertyWidget* m_materialPropertyWidget = nullptr;
	MeshPropertyWidget* m_meshPropertyWidget = nullptr;
	SettingPropertyWidget* m_settingPropertyWidget = nullptr;
	ResultsPropertyWidget* m_resultsPropertyWidget = nullptr;
	FallPropertyWidget* m_fallPropertyWidget = nullptr;
	FastCombustionPropertyWidget* m_fastCombustionPropertyWidget = nullptr;
	SlowCombustionPropertyWidget* m_slowCombustionPropertyWidget = nullptr;
	StressResultWidget* m_stressResultWidget = nullptr;
	TemperatureResultWidget* m_temperatureResultWidget = nullptr;
	OverpressureResultWidget* m_overpressureResultWidge = nullptr;
	SteelPropertyWidget* m_steelPropertyWidgett = nullptr;
	PropellantPropertyWidget* m_propellantPropertyWidget = nullptr;
	ProjectPropertyWidge* m_projectPropertyWidge = nullptr;
	CalculationPropertyWidget* m_calculationPropertyWidget = nullptr;
	JudgmentPropertyWidget* m_judgmentPropertyWidget = nullptr;
	InsulatingheatPropertyWidget* m_insulatingheatPropertyWidget = nullptr;
	OutheatPropertyWidget* m_outheatPropertyWidget = nullptr;
	StrainResultWidget* m_strainResultWidget = nullptr;
	ShootPropertyWidget* m_shootPropertyWidget = nullptr;
	JetImpactPropertyWidget* m_jetImpactPropertyWidget = nullptr;
	FragmentationImpactPropertyWidget* m_fragmentationImpactPropertyWidget = nullptr;
	ExplosiveBlastPropertyWidget* m_explosiveBlastPropertyWidget = nullptr;
	SacrificeExplosionPropertyWidget* m_sacrificeExplosionPropertyWidget = nullptr;
	DatabasePropertyWidget* m_databasePropertyWidget = nullptr;

};

