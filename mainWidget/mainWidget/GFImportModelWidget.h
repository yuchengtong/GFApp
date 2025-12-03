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
	FastCombustionPropertyWidget* GetFastCombustionPropertyWidget() { return m_fastCombustionPropertyWidget; };
	SlowCombustionPropertyWidget* GetSlowCombustionPropertyWidget() { return m_slowCombustionPropertyWidget; };
	ShootPropertyWidget* GetShootPropertyWidget() { return m_shootPropertyWidget; }
	JetImpactPropertyWidget* GetJetImpactPropertyWidget() { return m_jetImpactPropertyWidget; }
	FragmentationImpactPropertyWidget* GetFragmentationImpactPropertyWidget() { return m_fragmentationImpactPropertyWidget; }
	ExplosiveBlastPropertyWidget* GetExplosiveBlastPropertyWidget() { return m_explosiveBlastPropertyWidget; }
	SacrificeExplosionPropertyWidget* GetSacrificeExplosionPropertyWidget() { return m_sacrificeExplosionPropertyWidget; }

	StressResultWidget* GetShootStressResultWidget() { return m_shootStressResultWidget; }
	StrainResultWidget* GetShootStrainResultWidget() { return m_shootStrainResultWidget; }
	TemperatureResultWidget* GetShootTemperatureResultWidget() { return m_shootTemperatureResultWidget; }
	OverpressureResultWidget* GetShootOverpressureResultWidget() { return m_shootOverpressureResultWidge; }

	StressResultWidget* GetFragmentationImpactStressResultWidget() { return m_fragmentationImpactStressResultWidget; }
	StrainResultWidget* GetFragmentationImpactStrainResultWidget() { return m_fragmentationImpactStrainResultWidget; }
	TemperatureResultWidget* GetFragmentationImpactTemperatureResultWidget() { return m_fragmentationImpactTemperatureResultWidget; }
	OverpressureResultWidget* GetFragmentationImpactOverpressureResultWidget() { return m_fragmentationImpactOverpressureResultWidge; }

	TemperatureResultWidget* GetFastCombustionTemperatureResultWidget() { return m_fastCombustionTemperatureResultWidget; }

	TemperatureResultWidget* GetSlowCombustionTemperatureResultWidget() { return m_slowCombustionTemperatureResultWidget; }

	StressResultWidget* GetJetImpactStressResultWidget() { return m_jetImpactStressResultWidget; }
	StrainResultWidget* GetJetImpactStrainResultWidget() { return m_jetImpactStrainResultWidget; }
	TemperatureResultWidget* GetJetImpactTemperatureResultWidget() { return m_jetImpactTemperatureResultWidget; }
	OverpressureResultWidget* GetJetImpactOverpressureResultWidget() { return m_jetImpactOverpressureResultWidge; }

	StressResultWidget* GetExplosiveBlastStressResultWidget() { return m_explosiveBlastStressResultWidget; }
	StrainResultWidget* GetExplosiveBlastStrainResultWidget() { return m_explosiveBlastStrainResultWidget; }
	TemperatureResultWidget* GetExplosiveBlastTemperatureResultWidget() { return m_explosiveBlastTemperatureResultWidget; }
	OverpressureResultWidget* GetExplosiveBlastOverpressureResultWidget() { return m_explosiveBlastOverpressureResultWidge; }

	StressResultWidget* GetSacrificeExplosionStressResultWidget() { return m_sacrificeExplosionStressResultWidget; }
	StrainResultWidget* GetSacrificeExplosionStrainResultWidget() { return m_sacrificeExplosionStrainResultWidget; }
	TemperatureResultWidget* GetSacrificeExplosionTemperatureResultWidget() { return m_sacrificeExplosionTemperatureResultWidget; }
	OverpressureResultWidget* GetSacrificeExplosionOverpressureResultWidget() { return m_sacrificeExplosionOverpressureResultWidge; }



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
	SteelPropertyWidget* m_steelPropertyWidgett = nullptr;
	PropellantPropertyWidget* m_propellantPropertyWidget = nullptr;
	ProjectPropertyWidge* m_projectPropertyWidge = nullptr;
	CalculationPropertyWidget* m_calculationPropertyWidget = nullptr;
	JudgmentPropertyWidget* m_judgmentPropertyWidget = nullptr;
	InsulatingheatPropertyWidget* m_insulatingheatPropertyWidget = nullptr;
	OutheatPropertyWidget* m_outheatPropertyWidget = nullptr;
	ShootPropertyWidget* m_shootPropertyWidget = nullptr;
	JetImpactPropertyWidget* m_jetImpactPropertyWidget = nullptr;
	FragmentationImpactPropertyWidget* m_fragmentationImpactPropertyWidget = nullptr;
	ExplosiveBlastPropertyWidget* m_explosiveBlastPropertyWidget = nullptr;
	SacrificeExplosionPropertyWidget* m_sacrificeExplosionPropertyWidget = nullptr;
	DatabasePropertyWidget* m_databasePropertyWidget = nullptr;

	// 跌落结果
	StressResultWidget* m_stressResultWidget = nullptr;
	TemperatureResultWidget* m_temperatureResultWidget = nullptr;
	OverpressureResultWidget* m_overpressureResultWidge = nullptr;
	StrainResultWidget* m_strainResultWidget = nullptr;

	// 快烤结果
	TemperatureResultWidget* m_fastCombustionTemperatureResultWidget = nullptr;

	// 慢烤结果
	TemperatureResultWidget* m_slowCombustionTemperatureResultWidget = nullptr;

	// 枪击结果
	StressResultWidget* m_shootStressResultWidget = nullptr;
	TemperatureResultWidget* m_shootTemperatureResultWidget = nullptr;
	OverpressureResultWidget* m_shootOverpressureResultWidge = nullptr;
	StrainResultWidget* m_shootStrainResultWidget = nullptr;

	// 射流冲击结果
	StressResultWidget* m_jetImpactStressResultWidget = nullptr;
	TemperatureResultWidget* m_jetImpactTemperatureResultWidget = nullptr;
	OverpressureResultWidget* m_jetImpactOverpressureResultWidge = nullptr;
	StrainResultWidget* m_jetImpactStrainResultWidget = nullptr;

	// 破片结果
	StressResultWidget* m_fragmentationImpactStressResultWidget = nullptr;
	TemperatureResultWidget* m_fragmentationImpactTemperatureResultWidget = nullptr;
	OverpressureResultWidget* m_fragmentationImpactOverpressureResultWidge = nullptr;
	StrainResultWidget* m_fragmentationImpactStrainResultWidget = nullptr;

	// 爆炸冲击波结果
	StressResultWidget* m_explosiveBlastStressResultWidget = nullptr;
	TemperatureResultWidget* m_explosiveBlastTemperatureResultWidget = nullptr;
	OverpressureResultWidget* m_explosiveBlastOverpressureResultWidge = nullptr;
	StrainResultWidget* m_explosiveBlastStrainResultWidget = nullptr;

	// 殉爆结果
	StressResultWidget* m_sacrificeExplosionStressResultWidget = nullptr;
	TemperatureResultWidget* m_sacrificeExplosionTemperatureResultWidget = nullptr;
	OverpressureResultWidget* m_sacrificeExplosionOverpressureResultWidge = nullptr;
	StrainResultWidget* m_sacrificeExplosionStrainResultWidget = nullptr;
};

