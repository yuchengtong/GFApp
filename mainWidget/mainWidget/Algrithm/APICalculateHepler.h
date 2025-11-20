#pragma once
#include "OccView.h"

class APICalculateHepler
{
public:
	static bool CalculateFallAnalysisResult(OccView* occView,std::vector<double>& propertyValue);

	static bool CalculateShootingAnalysisResult(OccView* occView, std::vector<double>& propertyValue);

	static bool CalculateFragmentationAnalysisResult(OccView* occView, std::vector<double>& propertyValue);
};