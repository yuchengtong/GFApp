#pragma once
#include "occView.h"
#include <MeshVS_DataMapOfIntegerColor.HXX>
#include <AIS_Shape.hxx>

class APISetNodeValue
{
public:
	static void HSVtoRGB(double h, double s, double v, double& r, double& g, double& b);
	static MeshVS_DataMapOfIntegerColor GetMeshDataMap(std::vector<double> tt, double min, double max);
	static Handle(AIS_Shape) RotateAIS_ShapeXY(const Handle(AIS_Shape)& aisShape,
		Standard_Real angleDeg,
		Standard_Real x0,
		Standard_Real y0);

	//µøÂä
	static bool SetShellFallStressNephogram(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantFallStressNephogram(OccView* occView, std::vector<double>& nodeValues);

	bool CalculateAllFallStressNodeValues();

	static bool SetShellFallStrainNephogram(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantFallStrainNephogram(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellFallTempNephogram(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantFallTempNephogram(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellFallPressureNephogram(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantFallPressureNephogram(OccView* occView, std::vector<double>& nodeValues);

	static bool SetPropellantFallReactionDegreeNephogram(OccView* occView, std::vector<double>& nodeValues);
	//¿ìËÙ¿¾È¼
	static bool SetShellFastCombustionTempNephogram(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantFastCombustionTempNephogram(OccView* occView, std::vector<double>& nodeValues);
	//ÂýËÙ¿¾È¼
	static bool SetShellSlowCombustionTempNephogram(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantSlowCombustionTempNephogram(OccView* occView, std::vector<double>& nodeValues);
	//Ç¹»÷ÊÔÑé
	static bool SetShellShootStressResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantShootStressResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellShootStrainResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantShootStrainResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellShootTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantShootTemperatureResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellShootOverpressureResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantShootOverpressureResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetPropellantShootReactionDegreeNephogram(OccView* occView, std::vector<double>& nodeValues);
	//ÉäÁ÷³å»÷ÊÔÑé
	static bool SetShellJetImpactStressResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantJetImpactStressResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellJetImpactStrainResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantJetImpactStrainResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellJetImpactTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantJetImpactTemperatureResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellJetImpactOverpressureResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantJetImpactOverpressureResult(OccView* occView, std::vector<double>& nodeValues);
	
	static bool SetPropellantJetImpactReactionDegreeNephogram(OccView* occView, std::vector<double>& nodeValues);
	//ÆÆÆ¬ÊÔÑé
	static bool SetShellFragmentationStressResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantFragmentationStressResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellFragmentationStrainResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantFragmentationStrainResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellFragmentationTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantFragmentationTemperatureResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellFragmentationOverpressureResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantFragmentationOverpressureResult(OccView* occView, std::vector<double>& nodeValues);
	
	static bool SetPropellantFragmentationReactionDegreeNephogram(OccView* occView, std::vector<double>& nodeValues);
	//±¬Õ¨³å»÷²¨ÊÔÑé
	static bool SetShellExplosiveBlastStressResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantExplosiveBlastStressResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellExplosiveBlastStrainResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantExplosiveBlastStrainResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellExplosiveBlastTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantExplosiveBlastTemperatureResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellExplosiveBlastOverpressureResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantExplosiveBlastOverpressureResult(OccView* occView, std::vector<double>& nodeValues);
	
	static bool SetPropellantExplosiveBlastReactionDegreeNephogram(OccView* occView, std::vector<double>& nodeValues);
	// Ñ³±¬ÊÔÑé
	static bool SetShellSacrificeExplosionStressResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantSacrificeExplosionStressResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellSacrificeExplosionStrainResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantSacrificeExplosionStrainResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellSacrificeExplosionTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantSacrificeExplosionTemperatureResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellSacrificeExplosionOverpressureResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantSacrificeExplosionOverpressureResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetPropellantSacrificeExplosionReactionDegreeNephogram(OccView* occView, std::vector<double>& nodeValues);
};