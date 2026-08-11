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

	static double CalculateStd(const std::vector<double> data);

	//µøÂä
	static bool CalculateAllFallStressNodeValues();

	static bool CalculateAllFallStrainNodeValues();

	static bool CalculateAllFallTempNodeValues();

	static bool CalculateAllFallPressureNodeValues();

	static bool CalculateAllFallReactionDegreeNodeValues();
	//¿ìËÙ¿¾È¼
	static bool CalculateAllFastCombustionTempNodeValues();
	//ÂýËÙ¿¾È¼
	static bool CalculateAllSlowCombustionTempNodeValues();
	//Ç¹»÷ÊÔÑé
	static bool CalculateAllShootStressNodeValues();

	static bool CalculateAllShootStrainNodeValues();

	static bool CalculateAllShootTempNodeValues();

	static bool CalculateAllShootPressureNodeValues();

	static bool CalculateAllShootReactionDegreeNodeValues();
	//ÉäÁ÷³å»÷ÊÔÑé
	static bool CalculateAllJetImpactStressNodeValues();

	static bool CalculateAllJetImpactStrainNodeValues();

	static bool CalculateAllJetImpactTempNodeValues();

	static bool CalculateAllJetImpactPressureNodeValues();

	static bool CalculateAllJetImpactReactionDegreeNodeValues();
	//ÆÆÆ¬ÊÔÑé
	static bool CalculateAllFragmentationStressNodeValues();

	static bool CalculateAllFragmentationStrainNodeValues();

	static bool CalculateAllFragmentationTemperatureNodeValues();

	static bool CalculateAllFragmentationOverpressureNodeValues();

	static bool CalculateAllFragmentationReactionDegreeNodeValues();

	//±¬Õ¨³å»÷²¨ÊÔÑé
	static bool CalculateAllExplosiveBlastStressNodeValues();

	static bool CalculateAllExplosiveBlastStrainNodeValues();

	static bool CalculateAllExplosiveBlastTemperatureNodeValues();

	static bool CalculateAllExplosiveBlastOverpressureNodeValues();
	
	static bool CalculateAllExplosiveBlastReactionDegreeNodeValues();
	// Ñ³±¬ÊÔÑé
	static bool CalculateAllSacrificeExplosionStressNodeValues();

	static bool CalculateAllSacrificeExplosionStrainNodeValues();

	static bool CalculateAllSacrificeExplosionTemperatureNodeValues();

	static bool CalculateAllSacrificeExplosionOverpressureNodeValues();

	static bool CalculateAllSacrificeExplosionReactionDegreeNodeValues();
};