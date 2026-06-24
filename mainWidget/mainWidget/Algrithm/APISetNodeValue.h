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

	static bool SetShellFallStrainNephogram(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantFallStrainNephogram(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellFallTempNephogram(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantFallTempNephogram(OccView* occView, std::vector<double>& nodeValues);

	static bool SetShellFallPressureNephogram(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPropellantFallPressureNephogram(OccView* occView, std::vector<double>& nodeValues);



	//static bool SetFallStrainResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetFallTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetFallOverpressureResult(OccView* occView, std::vector<double>& nodeValues);
	////¿ìËÙ¿¾È¼
	//static bool SetFastCombustionTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	////ÂýËÙ¿¾È¼
	//static bool SetSlowCombustionTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	////Ç¹»÷ÊÔÑé
	//static bool SetShootStressResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetShootStrainResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetShootTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetShootOverpressureResult(OccView* occView, std::vector<double>& nodeValues);
	////ÉäÁ÷³å»÷ÊÔÑé
	//static bool SetJetImpactStressResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetJetImpactStrainResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetJetImpactTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetJetImpactOverpressureResult(OccView* occView, std::vector<double>& nodeValues);
	//
	////ÆÆÆ¬ÊÔÑé
	//static bool SetFragmentationStressResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetFragmentationStrainResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetFragmentationTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetFragmentationOverpressureResult(OccView* occView, std::vector<double>& nodeValues);
	////±¬Õ¨³å»÷²¨ÊÔÑé
	//static bool SetExplosiveBlastStressResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetExplosiveBlastStrainResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetExplosiveBlastTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetExplosiveBlastOverpressureResult(OccView* occView, std::vector<double>& nodeValues);
	//// Ñ³±¬ÊÔÑé
	//static bool SetSacrificeExplosionStressResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetSacrificeExplosionStrainResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetSacrificeExplosionTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	//static bool SetSacrificeExplosionOverpressureResult(OccView* occView, std::vector<double>& nodeValues);
};