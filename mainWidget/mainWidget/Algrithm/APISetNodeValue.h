#pragma once
#include "occView.h"
#include <MeshVS_DataMapOfIntegerColor.HXX>

class APISetNodeValue
{
public:
	static void HSVtoRGB(double h, double s, double v, double& r, double& g, double& b);
	static MeshVS_DataMapOfIntegerColor GetMeshDataMap(std::vector<double> tt, double min, double max);
	
	static bool SetFallStressResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetFallStrainResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetFallTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetFallOverpressureResult(OccView* occView, std::vector<double>& nodeValues);



	static bool SetShootStressResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetShootStrainResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetShootTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetShootOverpressureResult(OccView* occView, std::vector<double>& nodeValues);

	static bool SetFragmentationStressResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetFragmentationStrainResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetFragmentationTemperatureResult(OccView* occView, std::vector<double>& nodeValues);
	static bool SetFragmentationOverpressureResult(OccView* occView, std::vector<double>& nodeValues);

};