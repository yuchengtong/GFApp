#pragma once
#include "OccView.h"

class APICalculateHepler
{
public:

	static bool CalculateFallAnalysisResult(OccView* occView,std::vector<double>& propertyValue);

	static bool CalculateFastCombustionAnalysisResult(OccView* occView, std::vector<double>& propertyValue);

	static bool CalculateSlowCombustionAnalysisResult(OccView* occView, std::vector<double>& propertyValue);

	static bool CalculateShootingAnalysisResult(OccView* occView, std::vector<double>& propertyValue);

	static bool CalculateJetImpactingAnalysisResult(OccView* occView, std::vector<double>& propertyValue);

	static bool CalculateFragmentationAnalysisResult(OccView* occView, std::vector<double>& propertyValue);

	static bool CalculateExplosiveBlastAnalysisResult(OccView* occView, std::vector<double>& propertyValue);

	static bool CalculateSacrificeExplosionAnalysisResult(OccView* occView, std::vector<double>& propertyValue);
};