#include "APICalculateHepler.h"

#include <V3d_View.hxx>

#include <QMap>

#include "ModelDataManager.h"

double calculate(const QString& formula,
	double B, double C, double D, double E,
	double F, double G, double H, double I,
	double J, double K, double L, double M, double A)
{
	QString processedFormula = formula;  // 复制到非const变量
	processedFormula.remove(' ');
	// 变量映射：通过变量名获取对应值（使用map提高可读性和可维护性）
	const QMap<QString, double> varMap = {
		{"A", A}, {"B", B}, {"C", C}, {"D", D}, {"E", E},
		{"F", F}, {"G", G}, {"H", H}, {"I", I}, {"J", J},
		{"K", K}, {"L", L}, {"M", M}
	};

	QRegExp regExp("([+-]?)(\\d+(?:\\.\\d*)?|\\.\\d+)(?:\\*([A-Z]))?");
	regExp.setMinimal(false);

	double result = 0.0;
	int pos = 0;
	int matchCount = 0; // 统计匹配到的项数，用于校验公式合法性

	// 处理公式开头的第一项（可能无符号）
	if (processedFormula[0] != '+' && processedFormula[0] != '-') {
		processedFormula = "+" + processedFormula; // 补全正号，统一格式
	}

	while ((pos = regExp.indexIn(processedFormula, pos)) != -1) {
		++matchCount;
		QString signStr = regExp.cap(1);       // 符号（+/-）
		QString coeffStr = regExp.cap(2);      // 系数
		QString varName = regExp.cap(3);       // 变量

		// 解析符号（默认正号）
		double sign = (signStr == "-") ? -1.0 : 1.0;

		// 解析系数（处理转换失败）
		bool ok = false;
		double coeff = coeffStr.toDouble(&ok);
		if (!ok) {
			throw std::invalid_argument(QString("无效系数: %1").arg(coeffStr).toStdString());
		}

		// 计算当前项的值
		double term = sign * coeff;
		if (!varName.isEmpty()) {
			if (!varMap.contains(varName)) {
				throw std::invalid_argument(QString("未知变量: %1").arg(varName).toStdString());
			}
			term *= varMap[varName];  // 变量项：符号×系数×变量值
		}

		result += term;
		pos += regExp.matchedLength();
	}

	// 校验公式是否完全解析（无残留无效字符）
	if (matchCount == 0) {
		throw std::invalid_argument(QString("公式格式错误: %1").arg(formula).toStdString());
	}

	return result;
}

double calculateStd(const std::vector<double> data)
{
	if (data.empty()) {
		return 0.0;
	}
	double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
	double accum = 0.0;
	std::for_each(data.begin(), data.end(), [&](double d) {
		accum += (d - mean) * (d - mean);
		});
	double variance = accum / data.size();
	return std::sqrt(variance);
}


bool APICalculateHepler::CalculateFallAnalysisResult(OccView* occView, std::vector<double>& propertyValue)
{  
	Handle(AIS_InteractiveContext) context = occView->getContext();
	auto view = occView->getView();
	context->EraseAll(true);

	view->SetProj(V3d_Yneg);
	view->Redraw();

	//auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	//auto aDataSource = &meshInfo.triangleStructure;

	auto steelInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto propellantInfo = ModelDataManager::GetInstance()->GetPropellantPropertyInfo();
	auto calInfo = ModelDataManager::GetInstance()->GetCalculationPropertyInfo();
	auto fallInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto modelGeomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();

	auto A = 1;
	auto B = steelInfo.density;
	auto C = 0;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = 0;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;
	auto J = fallInfo.high * 1000;//跌落高度
	auto K = modelGeomInfo.length;//长
	auto L = modelGeomInfo.width;//宽
	auto M = modelGeomInfo.thickness;//厚


	// 应力
	auto stressCalculation = calInfo.fallStressCalculation;

	std::vector<double> stressResults;
	stressResults.reserve(stressCalculation.size());
	for (int i = 0; i < stressCalculation.size(); ++i)
	{
		double res = calculate(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		stressResults.push_back(res);
	}
	for (size_t i = 0; i < stressResults.size(); ++i) {
		stressResults[i] = stressResults[i] * 0.7 * 0.6;
		if (stressResults[i] < 0)
		{
			stressResults[i] = 0;
		}
	}
	double stressMinValue = *std::min_element(stressResults.begin(), stressResults.end());
	double stressMaxValue = *std::max_element(stressResults.begin(), stressResults.end());

	// 更新结果
	double shellStressMaxValue = stressMaxValue; // 发动机壳体最大应力
	double shellStressMinValue = 0; // 发动机壳体最小应力
	double shellStressAvgValue = shellStressMaxValue * 0.6; // 发动机壳体平均应力
	double shellStressStandardValue = calculateStd(stressResults); // 发动机壳体应力标准差
	double propellantStressMaxValue = stressMaxValue * 0.6; // 固体推进剂最大应力
	double propellantStressMinValue = 0; // 固体推进剂最小应力
	double propellantStressAvgValue = propellantStressMaxValue * 0.6; // 固体推进剂平均应力
	double propellantStressStandardValue = 0; // 固体推进剂应力标准差

	propertyValue.clear();
	propertyValue.push_back(shellStressMaxValue);
	propertyValue.push_back(shellStressMinValue);
	propertyValue.push_back(shellStressAvgValue);
	propertyValue.push_back(shellStressStandardValue);
	propertyValue.push_back(propellantStressMaxValue);
	propertyValue.push_back(propellantStressMinValue);
	propertyValue.push_back(propellantStressAvgValue);
	propertyValue.push_back(propellantStressStandardValue);
	//gfParent->GetStressResultWidget()->updateData(shellMaxValue, shellMinValue, shellAvgValue, shellStandardValue, maxValue, minValue, avgValue, standardValue);



	

	// 应力分析结果
	StressResult fallStressResult;
	fallStressResult.metalsMaxStress = shellStressMaxValue;
	fallStressResult.metalsMinStress = shellStressMinValue;
	fallStressResult.metalsAvgStress = shellStressAvgValue;
	fallStressResult.metalsStandardStress = shellStressStandardValue;
	fallStressResult.propellantsMaxStress = propellantStressMaxValue;
	fallStressResult.propellantsMinStress = propellantStressMinValue;
	fallStressResult.propellantsAvgStress = propellantStressAvgValue;
	fallStressResult.propellantsStandardStress = propellantStressStandardValue;
	fallStressResult.outheatMaxStress = shellStressMaxValue;
	fallStressResult.outheatMinStress = shellStressMinValue;
	fallStressResult.outheatAvgStress = shellStressAvgValue;
	fallStressResult.outheatStandardStress = shellStressStandardValue;
	fallStressResult.insulatingheatMaxStress = propellantStressMaxValue;
	fallStressResult.insulatingheatMinStress = propellantStressMinValue;
	fallStressResult.insulatingheatAvgStress = propellantStressAvgValue;
	fallStressResult.insulatingheatStandardStress = propellantStressStandardValue;
	ModelDataManager::GetInstance()->SetFallStressResult(fallStressResult);

	// 应变分析结果
	StrainResult fallStrainResult;
	fallStrainResult.metalsMaxStrain = fallStressResult.metalsMaxStress / steelInfo.modulus / 1000;
	fallStrainResult.metalsMinStrain = fallStressResult.metalsMinStress / steelInfo.modulus / 1000;
	fallStrainResult.metalsAvgStrain = fallStressResult.metalsAvgStress / steelInfo.modulus / 1000;
	fallStrainResult.metalsStandardStrain = fallStressResult.metalsStandardStress / steelInfo.modulus / 1000;
	fallStrainResult.propellantsMaxStrain = fallStressResult.propellantsMaxStress / steelInfo.modulus / 1000;
	fallStrainResult.propellantsMinStrain = fallStressResult.propellantsMaxStress / steelInfo.modulus / 1000;
	fallStrainResult.mpropellantsAvgStrain = fallStressResult.propellantsAvgStress / steelInfo.modulus / 1000;
	fallStrainResult.propellantsStandardStrain = fallStressResult.propellantsStandardStress / steelInfo.modulus / 1000;
	fallStrainResult.outheatMaxStrain = fallStressResult.outheatMaxStress / steelInfo.modulus / 1000;
	fallStrainResult.outheatMinStrain = fallStressResult.outheatMinStress / steelInfo.modulus / 1000;
	fallStrainResult.outheatAvgStrain = fallStressResult.outheatAvgStress / steelInfo.modulus / 1000;
	fallStrainResult.outheatStandardStrain = fallStressResult.outheatStandardStress / steelInfo.modulus / 1000;
	fallStrainResult.insulatingheatMaxStrain = fallStressResult.insulatingheatMaxStress / steelInfo.modulus / 1000;
	fallStrainResult.insulatingheatMinStrain = fallStressResult.insulatingheatMinStress / steelInfo.modulus / 1000;
	fallStrainResult.insulatingheatAvgStrain = fallStressResult.insulatingheatAvgStress / steelInfo.modulus / 1000;
	fallStrainResult.insulatingheatStandardStrain = fallStressResult.insulatingheatStandardStress / steelInfo.modulus / 1000;
	ModelDataManager::GetInstance()->SetFallStrainResult(fallStrainResult);
    


	// 温度
	auto temperatureCalculation = calInfo.fallTemperatureCalculation;
	std::vector<double> temperatureResults;
	temperatureResults.reserve(temperatureCalculation.size());
	for (int i = 0; i < temperatureCalculation.size(); ++i)
	{
		double res = calculate(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		temperatureResults.push_back(res);
	}
	for (size_t i = 0; i < temperatureResults.size(); ++i) {
		stressResults[i] = temperatureResults[i];
		if (temperatureResults[i] < 0)
		{
			temperatureResults[i] = 0;
		}
	}
	double temperatureMinValue = *std::min_element(temperatureResults.begin(), temperatureResults.end());
	double temperatureMaxValue = *std::max_element(temperatureResults.begin(), temperatureResults.end());

	// 更新结果
	double shellTemperatureMaxValue = temperatureMaxValue; // 发动机壳体最大温度
	double shellTemperatureMinValue = temperatureMinValue; // 发动机壳体最小温度
	double shellTemperatureAvgValue = shellTemperatureMaxValue * 0.6; // 发动机壳体平均温度
	double shellTemperatureStandardValue = calculateStd(temperatureResults); // 发动机壳体温度标准差
	double propellantTemperatureMaxValue = temperatureMaxValue; // 固体推进剂最大温度
	double propellantTemperatureMinValue = temperatureMinValue; // 固体推进剂最小温度
	double propellantTemperatureAvgValue = propellantTemperatureMaxValue * 0.6; // 固体推进剂平均温度
	double propellantTemperatureStandardValue = calculateStd(temperatureResults); // 固体推进剂温度标准差

	
	// 温度分析结果
	TemperatureResult temperatureResult;
	temperatureResult.metalsMaxTemperature = shellTemperatureMaxValue;
	temperatureResult.metalsMinTemperature = shellTemperatureMinValue;
	temperatureResult.metalsAvgTemperature = shellTemperatureAvgValue;
	temperatureResult.metalsStandardTemperature = shellTemperatureStandardValue;
	temperatureResult.propellantsMaxTemperature = propellantTemperatureMaxValue;
	temperatureResult.propellantsMinTemperature = propellantTemperatureMinValue;
	temperatureResult.mpropellantsAvgTemperature = propellantTemperatureAvgValue;
	temperatureResult.propellantsStandardTemperature = propellantTemperatureStandardValue;
	temperatureResult.outheatMaxTemperature = shellTemperatureMaxValue;
	temperatureResult.outheatMinTemperature = shellTemperatureMinValue;
	temperatureResult.outheatAvgTemperature = shellTemperatureAvgValue;
	temperatureResult.outheatStandardTemperature = shellTemperatureStandardValue;
	temperatureResult.insulatingheatMaxTemperature = propellantTemperatureMaxValue;
	temperatureResult.insulatingheatMinTemperature = propellantTemperatureMinValue;
	temperatureResult.insulatingheatAvgTemperature = propellantTemperatureAvgValue;
	temperatureResult.insulatingheatStandardTemperature = propellantTemperatureStandardValue;
	ModelDataManager::GetInstance()->SetFallTemperatureResult(temperatureResult);



	// 超压
	auto overpressureCalculation = calInfo.fallOverpressureCalculation;
	std::vector<double> overpressureResults;
	overpressureResults.reserve(overpressureCalculation.size());
	for (int i = 0; i < overpressureCalculation.size(); ++i)
	{
		double res = calculate(overpressureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		overpressureResults.push_back(res);
	}
	for (size_t i = 0; i < overpressureResults.size(); ++i) {
		stressResults[i] = overpressureResults[i];
		if (overpressureResults[i] < 0)
		{
			overpressureResults[i] = 0;
		}
	}
	double overpressureMinValue = *std::min_element(overpressureResults.begin(), overpressureResults.end());
	double overpressureMaxValue = *std::max_element(overpressureResults.begin(), overpressureResults.end());

	// 更新结果
	double shellOverpressureMaxValue = overpressureMaxValue; // 发动机壳体最大超压
	double shellOverpressureMinValue = overpressureMinValue; // 发动机壳体最小超压
	double shellOverpressureAvgValue = shellOverpressureMaxValue * 0.6; // 发动机壳体平均超压
	double shellOverpressureStandardValue = calculateStd(overpressureResults); // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = overpressureMaxValue; // 固体推进剂最大超压
	double propellantOverpressureMinValue = overpressureMinValue; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = propellantOverpressureMaxValue * 0.6; // 固体推进剂平均超压
	double propellantOverpressureStandardValue = calculateStd(overpressureResults); // 固体推进剂超压标准差


	// 超压分析结果
	OverpressureResult overpressureResult;
	overpressureResult.metalsMaxOverpressure = shellOverpressureMaxValue;
	overpressureResult.metalsMinOverpressure = shellOverpressureMinValue;
	overpressureResult.metalsAvgOverpressure = shellOverpressureAvgValue;
	overpressureResult.metalsStandardOverpressure = shellOverpressureStandardValue;
	overpressureResult.propellantsMaxOverpressure = propellantOverpressureMaxValue;
	overpressureResult.propellantsMinOverpressure = propellantOverpressureMinValue;
	overpressureResult.mpropellantsAvgOverpressure = propellantOverpressureAvgValue;
	overpressureResult.propellantsStandardOverpressure = propellantOverpressureStandardValue;
	overpressureResult.outheatMaxOverpressure = shellOverpressureMaxValue;
	overpressureResult.outheatMinOverpressure = shellOverpressureMinValue;
	overpressureResult.outheatAvgOverpressure = shellOverpressureAvgValue;
	overpressureResult.outheatStandardOverpressure = shellOverpressureStandardValue;
	overpressureResult.insulatingheatMaxOverpressure = propellantOverpressureMaxValue;
	overpressureResult.insulatingheatMinOverpressure = propellantOverpressureMinValue;
	overpressureResult.insulatingheatAvgOverpressure = propellantOverpressureAvgValue;
	overpressureResult.insulatingheatStandardOverpressure = propellantOverpressureStandardValue;
	ModelDataManager::GetInstance()->SetFallOverpressureResult(overpressureResult);




	FallAnalysisResultInfo fallAnalysisResultInfo;

	fallAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	fallAnalysisResultInfo.stressMaxValue = stressMaxValue;
	fallAnalysisResultInfo.stressMinValue = stressMinValue;
	fallAnalysisResultInfo.strainMaxValue = stressMaxValue / steelInfo.modulus / 1000;
	fallAnalysisResultInfo.strainMinValue = stressMinValue / steelInfo.modulus / 1000;
	fallAnalysisResultInfo.temperatureMaxValue = temperatureMaxValue;
	fallAnalysisResultInfo.temperatureMinValue = temperatureMinValue;
	fallAnalysisResultInfo.overpressureMaxValue = overpressureMaxValue;
	fallAnalysisResultInfo.overpressureMinValue = overpressureMinValue;
	ModelDataManager::GetInstance()->SetFallAnalysisResultInfo(fallAnalysisResultInfo);
    return true;
}

bool APICalculateHepler::CalculateFastCombustionAnalysisResult(OccView* occView, std::vector<double>& propertyValue)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	auto view = occView->getView();
	context->EraseAll(true);

	view->SetProj(V3d_Yneg);
	view->Redraw();

	
	auto steelInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto propellantInfo = ModelDataManager::GetInstance()->GetPropellantPropertyInfo();
	auto calInfo = ModelDataManager::GetInstance()->GetCalculationPropertyInfo();
	auto fastCombustionSettingInfo = ModelDataManager::GetInstance()->GetFastCombustionSettingInfo();
	auto modelGeomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();

	auto A = 1;
	auto B = steelInfo.density;
	auto C = 0;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = 0;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width / 2;//半径
	auto L = modelGeomInfo.thickness;//厚
	auto M = fastCombustionSettingInfo.temperature;//环境温度

	auto formulaCal = calInfo.fastCombustionCalculation;

	

	std::vector<double> results;
	results.reserve(formulaCal.size());
	for (int i = 0; i < formulaCal.size(); ++i)
	{
		double res = calculate(formulaCal[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		results.push_back(res);
	}
	for (size_t i = 0; i < results.size(); ++i) {
		results[i] = results[i] + 800;
	}

	double min_value = *std::min_element(results.begin(), results.end());
	double max_value = *std::max_element(results.begin(), results.end());

	auto calculateStd = [](const std::vector<double>& data) -> double {
		if (data.empty()) {
			return 0.0;
		}
		double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
		double accum = 0.0;
		std::for_each(data.begin(), data.end(), [&](double d) {
			accum += (d - mean) * (d - mean);
			});
		double variance = accum / data.size();
		return std::sqrt(variance);
	};


	// 更新结果
	double shellMaxValue = max_value; // 发动机壳体最大温度
	double shellMinValue = min_value; // 发动机壳体最小温度
	double shellAvgValue = shellMaxValue * 0.6; // 发动机壳体平均温度
	double shellStandardValue = calculateStd(results); // 发动机壳体温度标准差
	double maxValue = max_value * 0.6; // 固体推进剂最大温度
	double minValue = min_value; // 固体推进剂最小温度
	double avgValue = maxValue * 0.6; // 固体推进剂平均温度
	double standardValue = 0; // 固体推进剂温度标准差

	propertyValue.clear();
	propertyValue.push_back(shellMaxValue);
	propertyValue.push_back(shellMinValue);
	propertyValue.push_back(shellAvgValue);
	propertyValue.push_back(shellStandardValue);
	propertyValue.push_back(maxValue);
	propertyValue.push_back(minValue);
	propertyValue.push_back(avgValue);
	propertyValue.push_back(standardValue);

	FastCombustionAnalysisResultInfo fastCombustionAnalysisResultInfo;

	fastCombustionAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	fastCombustionAnalysisResultInfo.temperatureMaxValue = max_value;
	fastCombustionAnalysisResultInfo.temperatureMinValue = min_value;
	
	ModelDataManager::GetInstance()->SetFastCombustionAnalysisResultInfo(fastCombustionAnalysisResultInfo);

	// 温度分析结果
	TemperatureResult temperatureResult;
	temperatureResult.metalsMaxTemperature = shellMaxValue;
	temperatureResult.metalsMinTemperature = shellMinValue;
	temperatureResult.metalsAvgTemperature = shellAvgValue;
	temperatureResult.metalsStandardTemperature = shellStandardValue;
	temperatureResult.propellantsMaxTemperature = maxValue;
	temperatureResult.propellantsMinTemperature = minValue;
	temperatureResult.mpropellantsAvgTemperature = avgValue;
	temperatureResult.propellantsStandardTemperature = standardValue;
	temperatureResult.outheatMaxTemperature = shellMaxValue;
	temperatureResult.outheatMinTemperature = shellMinValue;
	temperatureResult.outheatAvgTemperature = shellAvgValue;
	temperatureResult.outheatStandardTemperature = shellStandardValue;
	temperatureResult.insulatingheatMaxTemperature = maxValue;
	temperatureResult.insulatingheatMinTemperature = minValue;
	temperatureResult.insulatingheatAvgTemperature = avgValue;
	temperatureResult.insulatingheatStandardTemperature = standardValue;
	ModelDataManager::GetInstance()->SetFastCombustionTemperatureResult(temperatureResult);

	return true;
}

bool APICalculateHepler::CalculateSlowCombustionAnalysisResult(OccView* occView, std::vector<double>& propertyValue)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	auto view = occView->getView();
	context->EraseAll(true);

	view->SetProj(V3d_Yneg);
	view->Redraw();


	auto steelInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto propellantInfo = ModelDataManager::GetInstance()->GetPropellantPropertyInfo();
	auto calInfo = ModelDataManager::GetInstance()->GetCalculationPropertyInfo();
	auto slowCombustionSettingInfo = ModelDataManager::GetInstance()->GetSlowCombustionSettingInfo();
	auto modelGeomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();

	auto A = 1;
	auto B = steelInfo.density;
	auto C = 0;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = 0;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width / 2;//半径
	auto L = modelGeomInfo.thickness;//厚
	auto M = slowCombustionSettingInfo.temperature;//温度幅度

	auto formulaCal = calInfo.slowCombustionCalculation;

	

	std::vector<double> results;
	results.reserve(formulaCal.size());
	for (int i = 0; i < formulaCal.size(); ++i)
	{
		double res = calculate(formulaCal[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		results.push_back(res);
	}
	for (size_t i = 0; i < results.size(); ++i) {
		results[i] = results[i] + 400;
	}

	double min_value = *std::min_element(results.begin(), results.end());
	double max_value = *std::max_element(results.begin(), results.end());

	auto calculateStd = [](const std::vector<double>& data) -> double {
		if (data.empty()) {
			return 0.0;
		}
		double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
		double accum = 0.0;
		std::for_each(data.begin(), data.end(), [&](double d) {
			accum += (d - mean) * (d - mean);
			});
		double variance = accum / data.size();
		return std::sqrt(variance);
	};


	// 更新结果
	double shellMaxValue = max_value; // 发动机壳体最大温度
	double shellMinValue = min_value; // 发动机壳体最小温度
	double shellAvgValue = shellMaxValue * 0.6; // 发动机壳体平均温度
	double shellStandardValue = calculateStd(results); // 发动机壳体温度标准差
	double maxValue = max_value * 0.6; // 固体推进剂最大温度
	double minValue = min_value; // 固体推进剂最小温度
	double avgValue = maxValue * 0.6; // 固体推进剂平均温度
	double standardValue = 0; // 固体推进剂温度标准差

	propertyValue.clear();
	propertyValue.push_back(shellMaxValue);
	propertyValue.push_back(shellMinValue);
	propertyValue.push_back(shellAvgValue);
	propertyValue.push_back(shellStandardValue);
	propertyValue.push_back(maxValue);
	propertyValue.push_back(minValue);
	propertyValue.push_back(avgValue);
	propertyValue.push_back(standardValue);

	SlowCombustionAnalysisResultInfo slowCombustionAnalysisResultInfo;

	slowCombustionAnalysisResultInfo.isChecked = true;
	slowCombustionAnalysisResultInfo.temperatureMaxValue = max_value;
	slowCombustionAnalysisResultInfo.temperatureMinValue = min_value;

	ModelDataManager::GetInstance()->SetSlowCombustionAnalysisResultInfo(slowCombustionAnalysisResultInfo);

	// 温度分析结果
	TemperatureResult temperatureResult;
	temperatureResult.metalsMaxTemperature = shellMaxValue;
	temperatureResult.metalsMinTemperature = shellMinValue;
	temperatureResult.metalsAvgTemperature = shellAvgValue;
	temperatureResult.metalsStandardTemperature = shellStandardValue;
	temperatureResult.propellantsMaxTemperature = maxValue;
	temperatureResult.propellantsMinTemperature = minValue;
	temperatureResult.mpropellantsAvgTemperature = avgValue;
	temperatureResult.propellantsStandardTemperature = standardValue;
	temperatureResult.outheatMaxTemperature = shellMaxValue;
	temperatureResult.outheatMinTemperature = shellMinValue;
	temperatureResult.outheatAvgTemperature = shellAvgValue;
	temperatureResult.outheatStandardTemperature = shellStandardValue;
	temperatureResult.insulatingheatMaxTemperature = maxValue;
	temperatureResult.insulatingheatMinTemperature = minValue;
	temperatureResult.insulatingheatAvgTemperature = avgValue;
	temperatureResult.insulatingheatStandardTemperature = standardValue;
	ModelDataManager::GetInstance()->SetSlowCombustionTemperatureResult(temperatureResult);
	return true;
}

bool APICalculateHepler::CalculateShootingAnalysisResult(OccView* occView, std::vector<double>& propertyValue)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	auto view = occView->getView();
	context->EraseAll(true);

	view->SetProj(V3d_Yneg);
	view->Redraw();

	auto steelInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto propellantInfo = ModelDataManager::GetInstance()->GetPropellantPropertyInfo();
	auto calInfo = ModelDataManager::GetInstance()->GetCalculationPropertyInfo();
	auto shootInfo = ModelDataManager::GetInstance()->GetShootSettingInfo();
	auto modelGeomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();

	auto A = 1;
	auto B = steelInfo.density;
	auto C = 0;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = 0;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = shootInfo.radius;//子弹直径
	auto L = modelGeomInfo.thickness;//厚度
	auto M = shootInfo.speed * 1000;//撞击速度

	// 应力
	auto stressCalculation = calInfo.shootStressCalculation;

	std::vector<double> stressResults;
	stressResults.reserve(stressCalculation.size());
	for (int i = 0; i < stressCalculation.size(); ++i)
	{
		double res = calculate(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		stressResults.push_back(res);
	}
	for (size_t i = 0; i < stressResults.size(); ++i) {
		stressResults[i] = stressResults[i] * 0.7 * 0.6;
		if (stressResults[i] < 0)
		{
			stressResults[i] = 0;
		}
	}
	double stressMinValue = *std::min_element(stressResults.begin(), stressResults.end());
	double stressMaxValue = *std::max_element(stressResults.begin(), stressResults.end());

	// 更新结果
	double shellStressMaxValue = stressMaxValue; // 发动机壳体最大应力
	double shellStressMinValue = 0; // 发动机壳体最小应力
	double shellStressAvgValue = shellStressMaxValue * 0.6; // 发动机壳体平均应力
	double shellStressStandardValue = calculateStd(stressResults); // 发动机壳体应力标准差
	double propellantStressMaxValue = stressMaxValue * 0.6; // 固体推进剂最大应力
	double propellantStressMinValue = 0; // 固体推进剂最小应力
	double propellantStressAvgValue = propellantStressMaxValue * 0.6; // 固体推进剂平均应力
	double propellantStressStandardValue = 0; // 固体推进剂应力标准差

	propertyValue.clear();
	propertyValue.push_back(shellStressMaxValue);
	propertyValue.push_back(shellStressMinValue);
	propertyValue.push_back(shellStressAvgValue);
	propertyValue.push_back(shellStressStandardValue);
	propertyValue.push_back(propellantStressMaxValue);
	propertyValue.push_back(propellantStressMinValue);
	propertyValue.push_back(propellantStressAvgValue);
	propertyValue.push_back(propellantStressStandardValue);
	//gfParent->GetStressResultWidget()->updateData(shellMaxValue, shellMinValue, shellAvgValue, shellStandardValue, maxValue, minValue, avgValue, standardValue);


	// 应力分析结果
	StressResult stressResult;
	stressResult.metalsMaxStress = shellStressMaxValue;
	stressResult.metalsMinStress = shellStressMinValue;
	stressResult.metalsAvgStress = shellStressAvgValue;
	stressResult.metalsStandardStress = shellStressStandardValue;
	stressResult.propellantsMaxStress = propellantStressMaxValue;
	stressResult.propellantsMinStress = propellantStressMinValue;
	stressResult.propellantsAvgStress = propellantStressAvgValue;
	stressResult.propellantsStandardStress = propellantStressStandardValue;
	stressResult.outheatMaxStress = shellStressMaxValue;
	stressResult.outheatMinStress = shellStressMinValue;
	stressResult.outheatAvgStress = shellStressAvgValue;
	stressResult.outheatStandardStress = shellStressStandardValue;
	stressResult.insulatingheatMaxStress = propellantStressMaxValue;
	stressResult.insulatingheatMinStress = propellantStressMinValue;
	stressResult.insulatingheatAvgStress = propellantStressAvgValue;
	stressResult.insulatingheatStandardStress = propellantStressStandardValue;
	ModelDataManager::GetInstance()->SetShootStressResult(stressResult);

	// 应变分析结果
	StrainResult strainResult;
	strainResult.metalsMaxStrain = stressResult.metalsMaxStress / steelInfo.modulus / 1000;
	strainResult.metalsMinStrain = stressResult.metalsMinStress / steelInfo.modulus / 1000;
	strainResult.metalsAvgStrain = stressResult.metalsAvgStress / steelInfo.modulus / 1000;
	strainResult.metalsStandardStrain = stressResult.metalsStandardStress / steelInfo.modulus / 1000;
	strainResult.propellantsMaxStrain = stressResult.propellantsMaxStress / steelInfo.modulus / 1000;
	strainResult.propellantsMinStrain = stressResult.propellantsMaxStress / steelInfo.modulus / 1000;
	strainResult.mpropellantsAvgStrain = stressResult.propellantsAvgStress / steelInfo.modulus / 1000;
	strainResult.propellantsStandardStrain = stressResult.propellantsStandardStress / steelInfo.modulus / 1000;
	strainResult.outheatMaxStrain = stressResult.outheatMaxStress / steelInfo.modulus / 1000;
	strainResult.outheatMinStrain = stressResult.outheatMinStress / steelInfo.modulus / 1000;
	strainResult.outheatAvgStrain = stressResult.outheatAvgStress / steelInfo.modulus / 1000;
	strainResult.outheatStandardStrain = stressResult.outheatStandardStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatMaxStrain = stressResult.insulatingheatMaxStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatMinStrain = stressResult.insulatingheatMinStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatAvgStrain = stressResult.insulatingheatAvgStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatStandardStrain = stressResult.insulatingheatStandardStress / steelInfo.modulus / 1000;
	ModelDataManager::GetInstance()->SetShootStrainResult(strainResult);



	// 温度
	auto temperatureCalculation = calInfo.shootTemperatureCalculation;
	std::vector<double> temperatureResults;
	temperatureResults.reserve(temperatureCalculation.size());
	for (int i = 0; i < temperatureCalculation.size(); ++i)
	{
		double res = calculate(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		temperatureResults.push_back(res);
	}
	for (size_t i = 0; i < temperatureResults.size(); ++i) {
		stressResults[i] = temperatureResults[i];
		if (temperatureResults[i] < 0)
		{
			temperatureResults[i] = 0;
		}
	}
	double temperatureMinValue = *std::min_element(temperatureResults.begin(), temperatureResults.end());
	double temperatureMaxValue = *std::max_element(temperatureResults.begin(), temperatureResults.end());

	// 更新结果
	double shellTemperatureMaxValue = temperatureMaxValue; // 发动机壳体最大温度
	double shellTemperatureMinValue = temperatureMinValue; // 发动机壳体最小温度
	double shellTemperatureAvgValue = shellTemperatureMaxValue * 0.6; // 发动机壳体平均温度
	double shellTemperatureStandardValue = calculateStd(temperatureResults); // 发动机壳体温度标准差
	double propellantTemperatureMaxValue = temperatureMaxValue; // 固体推进剂最大温度
	double propellantTemperatureMinValue = temperatureMinValue; // 固体推进剂最小温度
	double propellantTemperatureAvgValue = propellantTemperatureMaxValue * 0.6; // 固体推进剂平均温度
	double propellantTemperatureStandardValue = calculateStd(temperatureResults); // 固体推进剂温度标准差


	// 温度分析结果
	TemperatureResult temperatureResult;
	temperatureResult.metalsMaxTemperature = shellTemperatureMaxValue;
	temperatureResult.metalsMinTemperature = shellTemperatureMinValue;
	temperatureResult.metalsAvgTemperature = shellTemperatureAvgValue;
	temperatureResult.metalsStandardTemperature = shellTemperatureStandardValue;
	temperatureResult.propellantsMaxTemperature = propellantTemperatureMaxValue;
	temperatureResult.propellantsMinTemperature = propellantTemperatureMinValue;
	temperatureResult.mpropellantsAvgTemperature = propellantTemperatureAvgValue;
	temperatureResult.propellantsStandardTemperature = propellantTemperatureStandardValue;
	temperatureResult.outheatMaxTemperature = shellTemperatureMaxValue;
	temperatureResult.outheatMinTemperature = shellTemperatureMinValue;
	temperatureResult.outheatAvgTemperature = shellTemperatureAvgValue;
	temperatureResult.outheatStandardTemperature = shellTemperatureStandardValue;
	temperatureResult.insulatingheatMaxTemperature = propellantTemperatureMaxValue;
	temperatureResult.insulatingheatMinTemperature = propellantTemperatureMinValue;
	temperatureResult.insulatingheatAvgTemperature = propellantTemperatureAvgValue;
	temperatureResult.insulatingheatStandardTemperature = propellantTemperatureStandardValue;
	ModelDataManager::GetInstance()->SetShootTemperatureResult(temperatureResult);



	// 超压
	auto overpressureCalculation = calInfo.shootOverpressureCalculation;
	std::vector<double> overpressureResults;
	overpressureResults.reserve(overpressureCalculation.size());
	for (int i = 0; i < overpressureCalculation.size(); ++i)
	{
		double res = calculate(overpressureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		overpressureResults.push_back(res);
	}
	for (size_t i = 0; i < overpressureResults.size(); ++i) {
		stressResults[i] = overpressureResults[i];
		if (overpressureResults[i] < 0)
		{
			overpressureResults[i] = 0;
		}
	}
	double overpressureMinValue = *std::min_element(overpressureResults.begin(), overpressureResults.end());
	double overpressureMaxValue = *std::max_element(overpressureResults.begin(), overpressureResults.end());

	// 更新结果
	double shellOverpressureMaxValue = overpressureMaxValue; // 发动机壳体最大超压
	double shellOverpressureMinValue = overpressureMinValue; // 发动机壳体最小超压
	double shellOverpressureAvgValue = shellOverpressureMaxValue * 0.6; // 发动机壳体平均超压
	double shellOverpressureStandardValue = calculateStd(overpressureResults); // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = overpressureMaxValue; // 固体推进剂最大超压
	double propellantOverpressureMinValue = overpressureMinValue; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = propellantOverpressureMaxValue * 0.6; // 固体推进剂平均超压
	double propellantOverpressureStandardValue = calculateStd(overpressureResults); // 固体推进剂超压标准差


	// 超压分析结果
	OverpressureResult overpressureResult;
	overpressureResult.metalsMaxOverpressure = shellOverpressureMaxValue;
	overpressureResult.metalsMinOverpressure = shellOverpressureMinValue;
	overpressureResult.metalsAvgOverpressure = shellOverpressureAvgValue;
	overpressureResult.metalsStandardOverpressure = shellOverpressureStandardValue;
	overpressureResult.propellantsMaxOverpressure = propellantOverpressureMaxValue;
	overpressureResult.propellantsMinOverpressure = propellantOverpressureMinValue;
	overpressureResult.mpropellantsAvgOverpressure = propellantOverpressureAvgValue;
	overpressureResult.propellantsStandardOverpressure = propellantOverpressureStandardValue;
	overpressureResult.outheatMaxOverpressure = shellOverpressureMaxValue;
	overpressureResult.outheatMinOverpressure = shellOverpressureMinValue;
	overpressureResult.outheatAvgOverpressure = shellOverpressureAvgValue;
	overpressureResult.outheatStandardOverpressure = shellOverpressureStandardValue;
	overpressureResult.insulatingheatMaxOverpressure = propellantOverpressureMaxValue;
	overpressureResult.insulatingheatMinOverpressure = propellantOverpressureMinValue;
	overpressureResult.insulatingheatAvgOverpressure = propellantOverpressureAvgValue;
	overpressureResult.insulatingheatStandardOverpressure = propellantOverpressureStandardValue;
	ModelDataManager::GetInstance()->SetShootOverpressureResult(overpressureResult);




	ShootAnalysisResultInfo shootAnalysisResultInfo;

	shootAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	shootAnalysisResultInfo.stressMaxValue = stressMaxValue;
	shootAnalysisResultInfo.stressMinValue = stressMinValue;
	shootAnalysisResultInfo.strainMaxValue = stressMaxValue / steelInfo.modulus / 1000;
	shootAnalysisResultInfo.strainMinValue = stressMinValue / steelInfo.modulus / 1000;
	shootAnalysisResultInfo.temperatureMaxValue = temperatureMaxValue;
	shootAnalysisResultInfo.temperatureMinValue = temperatureMinValue;
	shootAnalysisResultInfo.overpressureMaxValue = overpressureMaxValue;
	shootAnalysisResultInfo.overpressureMinValue = overpressureMinValue;
	ModelDataManager::GetInstance()->SetShootAnalysisResultInfo(shootAnalysisResultInfo);

	return true;
}

bool APICalculateHepler::CalculateJetImpactingAnalysisResult(OccView* occView, std::vector<double>& propertyValue)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	auto view = occView->getView();
	context->EraseAll(true);

	view->SetProj(V3d_Yneg);
	view->Redraw();


	auto steelInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto propellantInfo = ModelDataManager::GetInstance()->GetPropellantPropertyInfo();
	auto calInfo = ModelDataManager::GetInstance()->GetCalculationPropertyInfo();
	auto jetImpactingInfo = ModelDataManager::GetInstance()->GetJetImpactSettingInfo();
	auto modelGeomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();

	auto A = 1;
	auto B = steelInfo.density;
	auto C = 0;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = 0;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width/2;//半径
	auto L = modelGeomInfo.thickness;//厚
	auto M = jetImpactingInfo.caliber;// 聚能装药口径

	// 应力
	auto stressCalculation = calInfo.jetImpactStressCalculation;

	std::vector<double> stressResults;
	stressResults.reserve(stressCalculation.size());
	for (int i = 0; i < stressCalculation.size(); ++i)
	{
		double res = calculate(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		stressResults.push_back(res);
	}
	for (size_t i = 0; i < stressResults.size(); ++i) {
		stressResults[i] = stressResults[i] * 0.7 * 0.6;
		if (stressResults[i] < 0)
		{
			stressResults[i] = 0;
		}
	}
	double stressMinValue = *std::min_element(stressResults.begin(), stressResults.end());
	double stressMaxValue = *std::max_element(stressResults.begin(), stressResults.end());

	// 更新结果
	double shellStressMaxValue = stressMaxValue; // 发动机壳体最大应力
	double shellStressMinValue = 0; // 发动机壳体最小应力
	double shellStressAvgValue = shellStressMaxValue * 0.6; // 发动机壳体平均应力
	double shellStressStandardValue = calculateStd(stressResults); // 发动机壳体应力标准差
	double propellantStressMaxValue = stressMaxValue * 0.6; // 固体推进剂最大应力
	double propellantStressMinValue = 0; // 固体推进剂最小应力
	double propellantStressAvgValue = propellantStressMaxValue * 0.6; // 固体推进剂平均应力
	double propellantStressStandardValue = 0; // 固体推进剂应力标准差

	propertyValue.clear();
	propertyValue.push_back(shellStressMaxValue);
	propertyValue.push_back(shellStressMinValue);
	propertyValue.push_back(shellStressAvgValue);
	propertyValue.push_back(shellStressStandardValue);
	propertyValue.push_back(propellantStressMaxValue);
	propertyValue.push_back(propellantStressMinValue);
	propertyValue.push_back(propellantStressAvgValue);
	propertyValue.push_back(propellantStressStandardValue);
	//gfParent->GetStressResultWidget()->updateData(shellMaxValue, shellMinValue, shellAvgValue, shellStandardValue, maxValue, minValue, avgValue, standardValue);


	// 应力分析结果
	StressResult stressResult;
	stressResult.metalsMaxStress = shellStressMaxValue;
	stressResult.metalsMinStress = shellStressMinValue;
	stressResult.metalsAvgStress = shellStressAvgValue;
	stressResult.metalsStandardStress = shellStressStandardValue;
	stressResult.propellantsMaxStress = propellantStressMaxValue;
	stressResult.propellantsMinStress = propellantStressMinValue;
	stressResult.propellantsAvgStress = propellantStressAvgValue;
	stressResult.propellantsStandardStress = propellantStressStandardValue;
	stressResult.outheatMaxStress = shellStressMaxValue;
	stressResult.outheatMinStress = shellStressMinValue;
	stressResult.outheatAvgStress = shellStressAvgValue;
	stressResult.outheatStandardStress = shellStressStandardValue;
	stressResult.insulatingheatMaxStress = propellantStressMaxValue;
	stressResult.insulatingheatMinStress = propellantStressMinValue;
	stressResult.insulatingheatAvgStress = propellantStressAvgValue;
	stressResult.insulatingheatStandardStress = propellantStressStandardValue;
	ModelDataManager::GetInstance()->SetJetImpactStressResult(stressResult);

	// 应变分析结果
	StrainResult strainResult;
	strainResult.metalsMaxStrain = stressResult.metalsMaxStress / steelInfo.modulus / 1000;
	strainResult.metalsMinStrain = stressResult.metalsMinStress / steelInfo.modulus / 1000;
	strainResult.metalsAvgStrain = stressResult.metalsAvgStress / steelInfo.modulus / 1000;
	strainResult.metalsStandardStrain = stressResult.metalsStandardStress / steelInfo.modulus / 1000;
	strainResult.propellantsMaxStrain = stressResult.propellantsMaxStress / steelInfo.modulus / 1000;
	strainResult.propellantsMinStrain = stressResult.propellantsMaxStress / steelInfo.modulus / 1000;
	strainResult.mpropellantsAvgStrain = stressResult.propellantsAvgStress / steelInfo.modulus / 1000;
	strainResult.propellantsStandardStrain = stressResult.propellantsStandardStress / steelInfo.modulus / 1000;
	strainResult.outheatMaxStrain = stressResult.outheatMaxStress / steelInfo.modulus / 1000;
	strainResult.outheatMinStrain = stressResult.outheatMinStress / steelInfo.modulus / 1000;
	strainResult.outheatAvgStrain = stressResult.outheatAvgStress / steelInfo.modulus / 1000;
	strainResult.outheatStandardStrain = stressResult.outheatStandardStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatMaxStrain = stressResult.insulatingheatMaxStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatMinStrain = stressResult.insulatingheatMinStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatAvgStrain = stressResult.insulatingheatAvgStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatStandardStrain = stressResult.insulatingheatStandardStress / steelInfo.modulus / 1000;
	ModelDataManager::GetInstance()->SetJetImpactStrainResult(strainResult);



	// 温度
	auto temperatureCalculation = calInfo.jetImpactTemperatureCalculation;
	std::vector<double> temperatureResults;
	temperatureResults.reserve(temperatureCalculation.size());
	for (int i = 0; i < temperatureCalculation.size(); ++i)
	{
		double res = calculate(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		temperatureResults.push_back(res);
	}
	for (size_t i = 0; i < temperatureResults.size(); ++i) {
		stressResults[i] = temperatureResults[i];
		if (temperatureResults[i] < 0)
		{
			temperatureResults[i] = 0;
		}
	}
	double temperatureMinValue = *std::min_element(temperatureResults.begin(), temperatureResults.end());
	double temperatureMaxValue = *std::max_element(temperatureResults.begin(), temperatureResults.end());

	// 更新结果
	double shellTemperatureMaxValue = temperatureMaxValue; // 发动机壳体最大温度
	double shellTemperatureMinValue = temperatureMinValue; // 发动机壳体最小温度
	double shellTemperatureAvgValue = shellTemperatureMaxValue * 0.6; // 发动机壳体平均温度
	double shellTemperatureStandardValue = calculateStd(temperatureResults); // 发动机壳体温度标准差
	double propellantTemperatureMaxValue = temperatureMaxValue; // 固体推进剂最大温度
	double propellantTemperatureMinValue = temperatureMinValue; // 固体推进剂最小温度
	double propellantTemperatureAvgValue = propellantTemperatureMaxValue * 0.6; // 固体推进剂平均温度
	double propellantTemperatureStandardValue = calculateStd(temperatureResults); // 固体推进剂温度标准差


	// 温度分析结果
	TemperatureResult temperatureResult;
	temperatureResult.metalsMaxTemperature = shellTemperatureMaxValue;
	temperatureResult.metalsMinTemperature = shellTemperatureMinValue;
	temperatureResult.metalsAvgTemperature = shellTemperatureAvgValue;
	temperatureResult.metalsStandardTemperature = shellTemperatureStandardValue;
	temperatureResult.propellantsMaxTemperature = propellantTemperatureMaxValue;
	temperatureResult.propellantsMinTemperature = propellantTemperatureMinValue;
	temperatureResult.mpropellantsAvgTemperature = propellantTemperatureAvgValue;
	temperatureResult.propellantsStandardTemperature = propellantTemperatureStandardValue;
	temperatureResult.outheatMaxTemperature = shellTemperatureMaxValue;
	temperatureResult.outheatMinTemperature = shellTemperatureMinValue;
	temperatureResult.outheatAvgTemperature = shellTemperatureAvgValue;
	temperatureResult.outheatStandardTemperature = shellTemperatureStandardValue;
	temperatureResult.insulatingheatMaxTemperature = propellantTemperatureMaxValue;
	temperatureResult.insulatingheatMinTemperature = propellantTemperatureMinValue;
	temperatureResult.insulatingheatAvgTemperature = propellantTemperatureAvgValue;
	temperatureResult.insulatingheatStandardTemperature = propellantTemperatureStandardValue;
	ModelDataManager::GetInstance()->SetJetImpactTemperatureResult(temperatureResult);



	// 超压
	auto overpressureCalculation = calInfo.jetImpactOverpressureCalculation;
	std::vector<double> overpressureResults;
	overpressureResults.reserve(overpressureCalculation.size());
	for (int i = 0; i < overpressureCalculation.size(); ++i)
	{
		double res = calculate(overpressureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		overpressureResults.push_back(res);
	}
	for (size_t i = 0; i < overpressureResults.size(); ++i) {
		stressResults[i] = overpressureResults[i];
		if (overpressureResults[i] < 0)
		{
			overpressureResults[i] = 0;
		}
	}
	double overpressureMinValue = *std::min_element(overpressureResults.begin(), overpressureResults.end());
	double overpressureMaxValue = *std::max_element(overpressureResults.begin(), overpressureResults.end());

	// 更新结果
	double shellOverpressureMaxValue = overpressureMaxValue; // 发动机壳体最大超压
	double shellOverpressureMinValue = overpressureMinValue; // 发动机壳体最小超压
	double shellOverpressureAvgValue = shellOverpressureMaxValue * 0.6; // 发动机壳体平均超压
	double shellOverpressureStandardValue = calculateStd(overpressureResults); // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = overpressureMaxValue; // 固体推进剂最大超压
	double propellantOverpressureMinValue = overpressureMinValue; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = propellantOverpressureMaxValue * 0.6; // 固体推进剂平均超压
	double propellantOverpressureStandardValue = calculateStd(overpressureResults); // 固体推进剂超压标准差


	// 超压分析结果
	OverpressureResult overpressureResult;
	overpressureResult.metalsMaxOverpressure = shellOverpressureMaxValue;
	overpressureResult.metalsMinOverpressure = shellOverpressureMinValue;
	overpressureResult.metalsAvgOverpressure = shellOverpressureAvgValue;
	overpressureResult.metalsStandardOverpressure = shellOverpressureStandardValue;
	overpressureResult.propellantsMaxOverpressure = propellantOverpressureMaxValue;
	overpressureResult.propellantsMinOverpressure = propellantOverpressureMinValue;
	overpressureResult.mpropellantsAvgOverpressure = propellantOverpressureAvgValue;
	overpressureResult.propellantsStandardOverpressure = propellantOverpressureStandardValue;
	overpressureResult.outheatMaxOverpressure = shellOverpressureMaxValue;
	overpressureResult.outheatMinOverpressure = shellOverpressureMinValue;
	overpressureResult.outheatAvgOverpressure = shellOverpressureAvgValue;
	overpressureResult.outheatStandardOverpressure = shellOverpressureStandardValue;
	overpressureResult.insulatingheatMaxOverpressure = propellantOverpressureMaxValue;
	overpressureResult.insulatingheatMinOverpressure = propellantOverpressureMinValue;
	overpressureResult.insulatingheatAvgOverpressure = propellantOverpressureAvgValue;
	overpressureResult.insulatingheatStandardOverpressure = propellantOverpressureStandardValue;
	ModelDataManager::GetInstance()->SetJetImpactOverpressureResult(overpressureResult);




	JetImpactAnalysisResultInfo jetImpactAnalysisResultInfo;

	jetImpactAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	jetImpactAnalysisResultInfo.stressMaxValue = stressMaxValue;
	jetImpactAnalysisResultInfo.stressMinValue = stressMinValue;
	jetImpactAnalysisResultInfo.strainMaxValue = stressMaxValue / steelInfo.modulus / 1000;
	jetImpactAnalysisResultInfo.strainMinValue = stressMinValue / steelInfo.modulus / 1000;
	jetImpactAnalysisResultInfo.temperatureMaxValue = temperatureMaxValue;
	jetImpactAnalysisResultInfo.temperatureMinValue = temperatureMinValue;
	jetImpactAnalysisResultInfo.overpressureMaxValue = overpressureMaxValue;
	jetImpactAnalysisResultInfo.overpressureMinValue = overpressureMinValue;
	ModelDataManager::GetInstance()->SetJetImpactAnalysisResultInfo(jetImpactAnalysisResultInfo);


	return true;
}

bool APICalculateHepler::CalculateFragmentationAnalysisResult(OccView* occView, std::vector<double>& propertyValue)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	auto view = occView->getView();
	context->EraseAll(true);

	view->SetProj(V3d_Yneg);
	view->Redraw();

	auto steelInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto propellantInfo = ModelDataManager::GetInstance()->GetPropellantPropertyInfo();
	auto calInfo = ModelDataManager::GetInstance()->GetCalculationPropertyInfo();
	auto fragmentationSettingInfo = ModelDataManager::GetInstance()->GetFragmentationSettingInfo();
	auto modelGeomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();

	auto A = 1;
	auto B = steelInfo.density;
	auto C = 0;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = 0;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = fragmentationSettingInfo.radius;//破片直径
	auto L = modelGeomInfo.thickness;//厚度
	auto M = fragmentationSettingInfo.speed * 1000;//撞击速度

	// 应力
	auto stressCalculation = calInfo.fragmentationImpactStressCalculation;

	std::vector<double> stressResults;
	stressResults.reserve(stressCalculation.size());
	for (int i = 0; i < stressCalculation.size(); ++i)
	{
		double res = calculate(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		stressResults.push_back(res);
	}
	for (size_t i = 0; i < stressResults.size(); ++i) {
		stressResults[i] = stressResults[i] * 0.7 * 0.6;
		if (stressResults[i] < 0)
		{
			stressResults[i] = 0;
		}
	}
	double stressMinValue = *std::min_element(stressResults.begin(), stressResults.end());
	double stressMaxValue = *std::max_element(stressResults.begin(), stressResults.end());

	// 更新结果
	double shellStressMaxValue = stressMaxValue; // 发动机壳体最大应力
	double shellStressMinValue = 0; // 发动机壳体最小应力
	double shellStressAvgValue = shellStressMaxValue * 0.6; // 发动机壳体平均应力
	double shellStressStandardValue = calculateStd(stressResults); // 发动机壳体应力标准差
	double propellantStressMaxValue = stressMaxValue * 0.6; // 固体推进剂最大应力
	double propellantStressMinValue = 0; // 固体推进剂最小应力
	double propellantStressAvgValue = propellantStressMaxValue * 0.6; // 固体推进剂平均应力
	double propellantStressStandardValue = 0; // 固体推进剂应力标准差

	propertyValue.clear();
	propertyValue.push_back(shellStressMaxValue);
	propertyValue.push_back(shellStressMinValue);
	propertyValue.push_back(shellStressAvgValue);
	propertyValue.push_back(shellStressStandardValue);
	propertyValue.push_back(propellantStressMaxValue);
	propertyValue.push_back(propellantStressMinValue);
	propertyValue.push_back(propellantStressAvgValue);
	propertyValue.push_back(propellantStressStandardValue);
	//gfParent->GetStressResultWidget()->updateData(shellMaxValue, shellMinValue, shellAvgValue, shellStandardValue, maxValue, minValue, avgValue, standardValue);


	// 应力分析结果
	StressResult stressResult;
	stressResult.metalsMaxStress = shellStressMaxValue;
	stressResult.metalsMinStress = shellStressMinValue;
	stressResult.metalsAvgStress = shellStressAvgValue;
	stressResult.metalsStandardStress = shellStressStandardValue;
	stressResult.propellantsMaxStress = propellantStressMaxValue;
	stressResult.propellantsMinStress = propellantStressMinValue;
	stressResult.propellantsAvgStress = propellantStressAvgValue;
	stressResult.propellantsStandardStress = propellantStressStandardValue;
	stressResult.outheatMaxStress = shellStressMaxValue;
	stressResult.outheatMinStress = shellStressMinValue;
	stressResult.outheatAvgStress = shellStressAvgValue;
	stressResult.outheatStandardStress = shellStressStandardValue;
	stressResult.insulatingheatMaxStress = propellantStressMaxValue;
	stressResult.insulatingheatMinStress = propellantStressMinValue;
	stressResult.insulatingheatAvgStress = propellantStressAvgValue;
	stressResult.insulatingheatStandardStress = propellantStressStandardValue;
	ModelDataManager::GetInstance()->SetFragmentationImpactStressResult(stressResult);

	// 应变分析结果
	StrainResult strainResult;
	strainResult.metalsMaxStrain = stressResult.metalsMaxStress / steelInfo.modulus / 1000;
	strainResult.metalsMinStrain = stressResult.metalsMinStress / steelInfo.modulus / 1000;
	strainResult.metalsAvgStrain = stressResult.metalsAvgStress / steelInfo.modulus / 1000;
	strainResult.metalsStandardStrain = stressResult.metalsStandardStress / steelInfo.modulus / 1000;
	strainResult.propellantsMaxStrain = stressResult.propellantsMaxStress / steelInfo.modulus / 1000;
	strainResult.propellantsMinStrain = stressResult.propellantsMaxStress / steelInfo.modulus / 1000;
	strainResult.mpropellantsAvgStrain = stressResult.propellantsAvgStress / steelInfo.modulus / 1000;
	strainResult.propellantsStandardStrain = stressResult.propellantsStandardStress / steelInfo.modulus / 1000;
	strainResult.outheatMaxStrain = stressResult.outheatMaxStress / steelInfo.modulus / 1000;
	strainResult.outheatMinStrain = stressResult.outheatMinStress / steelInfo.modulus / 1000;
	strainResult.outheatAvgStrain = stressResult.outheatAvgStress / steelInfo.modulus / 1000;
	strainResult.outheatStandardStrain = stressResult.outheatStandardStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatMaxStrain = stressResult.insulatingheatMaxStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatMinStrain = stressResult.insulatingheatMinStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatAvgStrain = stressResult.insulatingheatAvgStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatStandardStrain = stressResult.insulatingheatStandardStress / steelInfo.modulus / 1000;
	ModelDataManager::GetInstance()->SetFragmentationImpactStrainResult(strainResult);



	// 温度
	auto temperatureCalculation = calInfo.fragmentationImpactTemperatureCalculation;
	std::vector<double> temperatureResults;
	temperatureResults.reserve(temperatureCalculation.size());
	for (int i = 0; i < temperatureCalculation.size(); ++i)
	{
		double res = calculate(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		temperatureResults.push_back(res);
	}
	for (size_t i = 0; i < temperatureResults.size(); ++i) {
		stressResults[i] = temperatureResults[i];
		if (temperatureResults[i] < 0)
		{
			temperatureResults[i] = 0;
		}
	}
	double temperatureMinValue = *std::min_element(temperatureResults.begin(), temperatureResults.end());
	double temperatureMaxValue = *std::max_element(temperatureResults.begin(), temperatureResults.end());

	// 更新结果
	double shellTemperatureMaxValue = temperatureMaxValue; // 发动机壳体最大温度
	double shellTemperatureMinValue = temperatureMinValue; // 发动机壳体最小温度
	double shellTemperatureAvgValue = shellTemperatureMaxValue * 0.6; // 发动机壳体平均温度
	double shellTemperatureStandardValue = calculateStd(temperatureResults); // 发动机壳体温度标准差
	double propellantTemperatureMaxValue = temperatureMaxValue; // 固体推进剂最大温度
	double propellantTemperatureMinValue = temperatureMinValue; // 固体推进剂最小温度
	double propellantTemperatureAvgValue = propellantTemperatureMaxValue * 0.6; // 固体推进剂平均温度
	double propellantTemperatureStandardValue = calculateStd(temperatureResults); // 固体推进剂温度标准差


	// 温度分析结果
	TemperatureResult temperatureResult;
	temperatureResult.metalsMaxTemperature = shellTemperatureMaxValue;
	temperatureResult.metalsMinTemperature = shellTemperatureMinValue;
	temperatureResult.metalsAvgTemperature = shellTemperatureAvgValue;
	temperatureResult.metalsStandardTemperature = shellTemperatureStandardValue;
	temperatureResult.propellantsMaxTemperature = propellantTemperatureMaxValue;
	temperatureResult.propellantsMinTemperature = propellantTemperatureMinValue;
	temperatureResult.mpropellantsAvgTemperature = propellantTemperatureAvgValue;
	temperatureResult.propellantsStandardTemperature = propellantTemperatureStandardValue;
	temperatureResult.outheatMaxTemperature = shellTemperatureMaxValue;
	temperatureResult.outheatMinTemperature = shellTemperatureMinValue;
	temperatureResult.outheatAvgTemperature = shellTemperatureAvgValue;
	temperatureResult.outheatStandardTemperature = shellTemperatureStandardValue;
	temperatureResult.insulatingheatMaxTemperature = propellantTemperatureMaxValue;
	temperatureResult.insulatingheatMinTemperature = propellantTemperatureMinValue;
	temperatureResult.insulatingheatAvgTemperature = propellantTemperatureAvgValue;
	temperatureResult.insulatingheatStandardTemperature = propellantTemperatureStandardValue;
	ModelDataManager::GetInstance()->SetFragmentationImpactTemperatureResult(temperatureResult);



	// 超压
	auto overpressureCalculation = calInfo.fragmentationImpactOverpressureCalculationO;
	std::vector<double> overpressureResults;
	overpressureResults.reserve(overpressureCalculation.size());
	for (int i = 0; i < overpressureCalculation.size(); ++i)
	{
		double res = calculate(overpressureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		overpressureResults.push_back(res);
	}
	for (size_t i = 0; i < overpressureResults.size(); ++i) {
		stressResults[i] = overpressureResults[i];
		if (overpressureResults[i] < 0)
		{
			overpressureResults[i] = 0;
		}
	}
	double overpressureMinValue = *std::min_element(overpressureResults.begin(), overpressureResults.end());
	double overpressureMaxValue = *std::max_element(overpressureResults.begin(), overpressureResults.end());

	// 更新结果
	double shellOverpressureMaxValue = overpressureMaxValue; // 发动机壳体最大超压
	double shellOverpressureMinValue = overpressureMinValue; // 发动机壳体最小超压
	double shellOverpressureAvgValue = shellOverpressureMaxValue * 0.6; // 发动机壳体平均超压
	double shellOverpressureStandardValue = calculateStd(overpressureResults); // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = overpressureMaxValue; // 固体推进剂最大超压
	double propellantOverpressureMinValue = overpressureMinValue; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = propellantOverpressureMaxValue * 0.6; // 固体推进剂平均超压
	double propellantOverpressureStandardValue = calculateStd(overpressureResults); // 固体推进剂超压标准差


	// 超压分析结果
	OverpressureResult overpressureResult;
	overpressureResult.metalsMaxOverpressure = shellOverpressureMaxValue;
	overpressureResult.metalsMinOverpressure = shellOverpressureMinValue;
	overpressureResult.metalsAvgOverpressure = shellOverpressureAvgValue;
	overpressureResult.metalsStandardOverpressure = shellOverpressureStandardValue;
	overpressureResult.propellantsMaxOverpressure = propellantOverpressureMaxValue;
	overpressureResult.propellantsMinOverpressure = propellantOverpressureMinValue;
	overpressureResult.mpropellantsAvgOverpressure = propellantOverpressureAvgValue;
	overpressureResult.propellantsStandardOverpressure = propellantOverpressureStandardValue;
	overpressureResult.outheatMaxOverpressure = shellOverpressureMaxValue;
	overpressureResult.outheatMinOverpressure = shellOverpressureMinValue;
	overpressureResult.outheatAvgOverpressure = shellOverpressureAvgValue;
	overpressureResult.outheatStandardOverpressure = shellOverpressureStandardValue;
	overpressureResult.insulatingheatMaxOverpressure = propellantOverpressureMaxValue;
	overpressureResult.insulatingheatMinOverpressure = propellantOverpressureMinValue;
	overpressureResult.insulatingheatAvgOverpressure = propellantOverpressureAvgValue;
	overpressureResult.insulatingheatStandardOverpressure = propellantOverpressureStandardValue;
	ModelDataManager::GetInstance()->SetFragmentationImpactOverpressureResult(overpressureResult);




	FragmentationAnalysisResultInfo fragmentationAnalysisResultInfo;

	fragmentationAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	fragmentationAnalysisResultInfo.stressMaxValue = stressMaxValue;
	fragmentationAnalysisResultInfo.stressMinValue = stressMinValue;
	fragmentationAnalysisResultInfo.strainMaxValue = stressMaxValue / steelInfo.modulus / 1000;
	fragmentationAnalysisResultInfo.strainMinValue = stressMinValue / steelInfo.modulus / 1000;
	fragmentationAnalysisResultInfo.temperatureMaxValue = temperatureMaxValue;
	fragmentationAnalysisResultInfo.temperatureMinValue = temperatureMinValue;
	fragmentationAnalysisResultInfo.overpressureMaxValue = overpressureMaxValue;
	fragmentationAnalysisResultInfo.overpressureMinValue = overpressureMinValue;
	ModelDataManager::GetInstance()->SetFragmentationAnalysisResultInfo(fragmentationAnalysisResultInfo);
	return true;
}

bool APICalculateHepler::CalculateExplosiveBlastAnalysisResult(OccView* occView, std::vector<double>& propertyValue)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	auto view = occView->getView();
	context->EraseAll(true);

	view->SetProj(V3d_Yneg);
	view->Redraw();

	auto steelInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto propellantInfo = ModelDataManager::GetInstance()->GetPropellantPropertyInfo();
	auto calInfo = ModelDataManager::GetInstance()->GetCalculationPropertyInfo();
	auto explosiveBlastSettingInfo = ModelDataManager::GetInstance()->GetExplosiveBlastSettingInfo();
	auto modelGeomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();

	auto A = 1;
	auto B = steelInfo.density;
	auto C = 0;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = 0;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width / 2;//半径
	auto L = modelGeomInfo.thickness;//厚
	auto M = explosiveBlastSettingInfo.tnt;// TNT当量


	// 应力
	auto stressCalculation = calInfo.explosiveBlastStressCalculation;

	std::vector<double> stressResults;
	stressResults.reserve(stressCalculation.size());
	for (int i = 0; i < stressCalculation.size(); ++i)
	{
		double res = calculate(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		stressResults.push_back(res);
	}
	for (size_t i = 0; i < stressResults.size(); ++i) {
		stressResults[i] = stressResults[i] * 0.7 * 0.6;
		if (stressResults[i] < 0)
		{
			stressResults[i] = 0;
		}
	}
	double stressMinValue = *std::min_element(stressResults.begin(), stressResults.end());
	double stressMaxValue = *std::max_element(stressResults.begin(), stressResults.end());

	// 更新结果
	double shellStressMaxValue = stressMaxValue; // 发动机壳体最大应力
	double shellStressMinValue = 0; // 发动机壳体最小应力
	double shellStressAvgValue = shellStressMaxValue * 0.6; // 发动机壳体平均应力
	double shellStressStandardValue = calculateStd(stressResults); // 发动机壳体应力标准差
	double propellantStressMaxValue = stressMaxValue * 0.6; // 固体推进剂最大应力
	double propellantStressMinValue = 0; // 固体推进剂最小应力
	double propellantStressAvgValue = propellantStressMaxValue * 0.6; // 固体推进剂平均应力
	double propellantStressStandardValue = 0; // 固体推进剂应力标准差

	propertyValue.clear();
	propertyValue.push_back(shellStressMaxValue);
	propertyValue.push_back(shellStressMinValue);
	propertyValue.push_back(shellStressAvgValue);
	propertyValue.push_back(shellStressStandardValue);
	propertyValue.push_back(propellantStressMaxValue);
	propertyValue.push_back(propellantStressMinValue);
	propertyValue.push_back(propellantStressAvgValue);
	propertyValue.push_back(propellantStressStandardValue);
	//gfParent->GetStressResultWidget()->updateData(shellMaxValue, shellMinValue, shellAvgValue, shellStandardValue, maxValue, minValue, avgValue, standardValue);


	// 应力分析结果
	StressResult stressResult;
	stressResult.metalsMaxStress = shellStressMaxValue;
	stressResult.metalsMinStress = shellStressMinValue;
	stressResult.metalsAvgStress = shellStressAvgValue;
	stressResult.metalsStandardStress = shellStressStandardValue;
	stressResult.propellantsMaxStress = propellantStressMaxValue;
	stressResult.propellantsMinStress = propellantStressMinValue;
	stressResult.propellantsAvgStress = propellantStressAvgValue;
	stressResult.propellantsStandardStress = propellantStressStandardValue;
	stressResult.outheatMaxStress = shellStressMaxValue;
	stressResult.outheatMinStress = shellStressMinValue;
	stressResult.outheatAvgStress = shellStressAvgValue;
	stressResult.outheatStandardStress = shellStressStandardValue;
	stressResult.insulatingheatMaxStress = propellantStressMaxValue;
	stressResult.insulatingheatMinStress = propellantStressMinValue;
	stressResult.insulatingheatAvgStress = propellantStressAvgValue;
	stressResult.insulatingheatStandardStress = propellantStressStandardValue;
	ModelDataManager::GetInstance()->SetExplosiveBlastStressResult(stressResult);

	// 应变分析结果
	StrainResult strainResult;
	strainResult.metalsMaxStrain = stressResult.metalsMaxStress / steelInfo.modulus / 1000;
	strainResult.metalsMinStrain = stressResult.metalsMinStress / steelInfo.modulus / 1000;
	strainResult.metalsAvgStrain = stressResult.metalsAvgStress / steelInfo.modulus / 1000;
	strainResult.metalsStandardStrain = stressResult.metalsStandardStress / steelInfo.modulus / 1000;
	strainResult.propellantsMaxStrain = stressResult.propellantsMaxStress / steelInfo.modulus / 1000;
	strainResult.propellantsMinStrain = stressResult.propellantsMaxStress / steelInfo.modulus / 1000;
	strainResult.mpropellantsAvgStrain = stressResult.propellantsAvgStress / steelInfo.modulus / 1000;
	strainResult.propellantsStandardStrain = stressResult.propellantsStandardStress / steelInfo.modulus / 1000;
	strainResult.outheatMaxStrain = stressResult.outheatMaxStress / steelInfo.modulus / 1000;
	strainResult.outheatMinStrain = stressResult.outheatMinStress / steelInfo.modulus / 1000;
	strainResult.outheatAvgStrain = stressResult.outheatAvgStress / steelInfo.modulus / 1000;
	strainResult.outheatStandardStrain = stressResult.outheatStandardStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatMaxStrain = stressResult.insulatingheatMaxStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatMinStrain = stressResult.insulatingheatMinStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatAvgStrain = stressResult.insulatingheatAvgStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatStandardStrain = stressResult.insulatingheatStandardStress / steelInfo.modulus / 1000;
	ModelDataManager::GetInstance()->SetExplosiveBlastStrainResult(strainResult);



	// 超压
	auto overpressureCalculation = calInfo.explosiveBlastOverpressureCalculation;
	std::vector<double> overpressureResults;
	overpressureResults.reserve(overpressureCalculation.size());
	for (int i = 0; i < overpressureCalculation.size(); ++i)
	{
		double res = calculate(overpressureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		overpressureResults.push_back(res);
	}
	for (size_t i = 0; i < overpressureResults.size(); ++i) {
		stressResults[i] = overpressureResults[i];
		if (overpressureResults[i] < 0)
		{
			overpressureResults[i] = 0;
		}
	}
	double overpressureMinValue = *std::min_element(overpressureResults.begin(), overpressureResults.end());
	double overpressureMaxValue = *std::max_element(overpressureResults.begin(), overpressureResults.end());

	// 更新结果
	double shellOverpressureMaxValue = overpressureMaxValue; // 发动机壳体最大超压
	double shellOverpressureMinValue = overpressureMinValue; // 发动机壳体最小超压
	double shellOverpressureAvgValue = shellOverpressureMaxValue * 0.6; // 发动机壳体平均超压
	double shellOverpressureStandardValue = calculateStd(overpressureResults); // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = overpressureMaxValue; // 固体推进剂最大超压
	double propellantOverpressureMinValue = overpressureMinValue; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = propellantOverpressureMaxValue * 0.6; // 固体推进剂平均超压
	double propellantOverpressureStandardValue = calculateStd(overpressureResults); // 固体推进剂超压标准差


	// 超压分析结果
	OverpressureResult overpressureResult;
	overpressureResult.metalsMaxOverpressure = shellOverpressureMaxValue;
	overpressureResult.metalsMinOverpressure = shellOverpressureMinValue;
	overpressureResult.metalsAvgOverpressure = shellOverpressureAvgValue;
	overpressureResult.metalsStandardOverpressure = shellOverpressureStandardValue;
	overpressureResult.propellantsMaxOverpressure = propellantOverpressureMaxValue;
	overpressureResult.propellantsMinOverpressure = propellantOverpressureMinValue;
	overpressureResult.mpropellantsAvgOverpressure = propellantOverpressureAvgValue;
	overpressureResult.propellantsStandardOverpressure = propellantOverpressureStandardValue;
	overpressureResult.outheatMaxOverpressure = shellOverpressureMaxValue;
	overpressureResult.outheatMinOverpressure = shellOverpressureMinValue;
	overpressureResult.outheatAvgOverpressure = shellOverpressureAvgValue;
	overpressureResult.outheatStandardOverpressure = shellOverpressureStandardValue;
	overpressureResult.insulatingheatMaxOverpressure = propellantOverpressureMaxValue;
	overpressureResult.insulatingheatMinOverpressure = propellantOverpressureMinValue;
	overpressureResult.insulatingheatAvgOverpressure = propellantOverpressureAvgValue;
	overpressureResult.insulatingheatStandardOverpressure = propellantOverpressureStandardValue;
	ModelDataManager::GetInstance()->SetExplosiveBlastOverpressureResult(overpressureResult);




	ExplosiveBlastAnalysisResultInfo explosiveBlastAnalysisResultInfo;

	explosiveBlastAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	explosiveBlastAnalysisResultInfo.stressMaxValue = stressMaxValue;
	explosiveBlastAnalysisResultInfo.stressMinValue = stressMinValue;
	explosiveBlastAnalysisResultInfo.strainMaxValue = stressMaxValue / steelInfo.modulus / 1000;
	explosiveBlastAnalysisResultInfo.strainMinValue = stressMinValue / steelInfo.modulus / 1000;
	explosiveBlastAnalysisResultInfo.temperatureMaxValue = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult().propellantsMaxTemperature;
	explosiveBlastAnalysisResultInfo.temperatureMinValue = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult().propellantsMinTemperature;
	explosiveBlastAnalysisResultInfo.overpressureMaxValue = overpressureMaxValue;
	explosiveBlastAnalysisResultInfo.overpressureMinValue = overpressureMinValue;
	ModelDataManager::GetInstance()->SetExplosiveBlastAnalysisResultInfo(explosiveBlastAnalysisResultInfo);

	return true;
}

bool APICalculateHepler::CalculateSacrificeExplosionAnalysisResult(OccView* occView, std::vector<double>& propertyValue)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	auto view = occView->getView();
	context->EraseAll(true);

	view->SetProj(V3d_Yneg);
	view->Redraw();


	auto steelInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto propellantInfo = ModelDataManager::GetInstance()->GetPropellantPropertyInfo();
	auto calInfo = ModelDataManager::GetInstance()->GetCalculationPropertyInfo();
	auto sacrificeExplosionInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionSettingInfo();
	auto modelGeomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();

	auto A = 1;
	auto B = steelInfo.density;
	auto C = 0;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = 0;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;
	
	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width;//宽
	auto L = modelGeomInfo.thickness;//厚
	auto M = sacrificeExplosionInfo.distance;//距离

		// 应力
	auto stressCalculation = calInfo.sacrificeExplosionStressCalculation;

	std::vector<double> stressResults;
	stressResults.reserve(stressCalculation.size());
	for (int i = 0; i < stressCalculation.size(); ++i)
	{
		double res = calculate(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		stressResults.push_back(res);
	}
	for (size_t i = 0; i < stressResults.size(); ++i) {
		stressResults[i] = stressResults[i] * 0.7 * 0.6;
		if (stressResults[i] < 0)
		{
			stressResults[i] = 0;
		}
	}
	double stressMinValue = *std::min_element(stressResults.begin(), stressResults.end());
	double stressMaxValue = *std::max_element(stressResults.begin(), stressResults.end());

	// 更新结果
	double shellStressMaxValue = stressMaxValue; // 发动机壳体最大应力
	double shellStressMinValue = 0; // 发动机壳体最小应力
	double shellStressAvgValue = shellStressMaxValue * 0.6; // 发动机壳体平均应力
	double shellStressStandardValue = calculateStd(stressResults); // 发动机壳体应力标准差
	double propellantStressMaxValue = stressMaxValue * 0.6; // 固体推进剂最大应力
	double propellantStressMinValue = 0; // 固体推进剂最小应力
	double propellantStressAvgValue = propellantStressMaxValue * 0.6; // 固体推进剂平均应力
	double propellantStressStandardValue = 0; // 固体推进剂应力标准差

	propertyValue.clear();
	propertyValue.push_back(shellStressMaxValue);
	propertyValue.push_back(shellStressMinValue);
	propertyValue.push_back(shellStressAvgValue);
	propertyValue.push_back(shellStressStandardValue);
	propertyValue.push_back(propellantStressMaxValue);
	propertyValue.push_back(propellantStressMinValue);
	propertyValue.push_back(propellantStressAvgValue);
	propertyValue.push_back(propellantStressStandardValue);
	//gfParent->GetStressResultWidget()->updateData(shellMaxValue, shellMinValue, shellAvgValue, shellStandardValue, maxValue, minValue, avgValue, standardValue);


	// 应力分析结果
	StressResult stressResult;
	stressResult.metalsMaxStress = shellStressMaxValue;
	stressResult.metalsMinStress = shellStressMinValue;
	stressResult.metalsAvgStress = shellStressAvgValue;
	stressResult.metalsStandardStress = shellStressStandardValue;
	stressResult.propellantsMaxStress = propellantStressMaxValue;
	stressResult.propellantsMinStress = propellantStressMinValue;
	stressResult.propellantsAvgStress = propellantStressAvgValue;
	stressResult.propellantsStandardStress = propellantStressStandardValue;
	stressResult.outheatMaxStress = shellStressMaxValue;
	stressResult.outheatMinStress = shellStressMinValue;
	stressResult.outheatAvgStress = shellStressAvgValue;
	stressResult.outheatStandardStress = shellStressStandardValue;
	stressResult.insulatingheatMaxStress = propellantStressMaxValue;
	stressResult.insulatingheatMinStress = propellantStressMinValue;
	stressResult.insulatingheatAvgStress = propellantStressAvgValue;
	stressResult.insulatingheatStandardStress = propellantStressStandardValue;
	ModelDataManager::GetInstance()->SetSacrificeExplosionStressResult(stressResult);

	// 应变分析结果
	StrainResult strainResult;
	strainResult.metalsMaxStrain = stressResult.metalsMaxStress / steelInfo.modulus / 1000;
	strainResult.metalsMinStrain = stressResult.metalsMinStress / steelInfo.modulus / 1000;
	strainResult.metalsAvgStrain = stressResult.metalsAvgStress / steelInfo.modulus / 1000;
	strainResult.metalsStandardStrain = stressResult.metalsStandardStress / steelInfo.modulus / 1000;
	strainResult.propellantsMaxStrain = stressResult.propellantsMaxStress / steelInfo.modulus / 1000;
	strainResult.propellantsMinStrain = stressResult.propellantsMaxStress / steelInfo.modulus / 1000;
	strainResult.mpropellantsAvgStrain = stressResult.propellantsAvgStress / steelInfo.modulus / 1000;
	strainResult.propellantsStandardStrain = stressResult.propellantsStandardStress / steelInfo.modulus / 1000;
	strainResult.outheatMaxStrain = stressResult.outheatMaxStress / steelInfo.modulus / 1000;
	strainResult.outheatMinStrain = stressResult.outheatMinStress / steelInfo.modulus / 1000;
	strainResult.outheatAvgStrain = stressResult.outheatAvgStress / steelInfo.modulus / 1000;
	strainResult.outheatStandardStrain = stressResult.outheatStandardStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatMaxStrain = stressResult.insulatingheatMaxStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatMinStrain = stressResult.insulatingheatMinStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatAvgStrain = stressResult.insulatingheatAvgStress / steelInfo.modulus / 1000;
	strainResult.insulatingheatStandardStrain = stressResult.insulatingheatStandardStress / steelInfo.modulus / 1000;
	ModelDataManager::GetInstance()->SetSacrificeExplosionStrainResult(strainResult);



	// 超压
	auto overpressureCalculation = calInfo.sacrificeExplosionOverpressureCalculation;
	std::vector<double> overpressureResults;
	overpressureResults.reserve(overpressureCalculation.size());
	for (int i = 0; i < overpressureCalculation.size(); ++i)
	{
		double res = calculate(overpressureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		overpressureResults.push_back(res);
	}
	for (size_t i = 0; i < overpressureResults.size(); ++i) {
		stressResults[i] = overpressureResults[i];
		if (overpressureResults[i] < 0)
		{
			overpressureResults[i] = 0;
		}
	}
	double overpressureMinValue = *std::min_element(overpressureResults.begin(), overpressureResults.end());
	double overpressureMaxValue = *std::max_element(overpressureResults.begin(), overpressureResults.end());

	// 更新结果
	double shellOverpressureMaxValue = overpressureMaxValue; // 发动机壳体最大超压
	double shellOverpressureMinValue = overpressureMinValue; // 发动机壳体最小超压
	double shellOverpressureAvgValue = shellOverpressureMaxValue * 0.6; // 发动机壳体平均超压
	double shellOverpressureStandardValue = calculateStd(overpressureResults); // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = overpressureMaxValue; // 固体推进剂最大超压
	double propellantOverpressureMinValue = overpressureMinValue; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = propellantOverpressureMaxValue * 0.6; // 固体推进剂平均超压
	double propellantOverpressureStandardValue = calculateStd(overpressureResults); // 固体推进剂超压标准差


	// 超压分析结果
	OverpressureResult overpressureResult;
	overpressureResult.metalsMaxOverpressure = shellOverpressureMaxValue;
	overpressureResult.metalsMinOverpressure = shellOverpressureMinValue;
	overpressureResult.metalsAvgOverpressure = shellOverpressureAvgValue;
	overpressureResult.metalsStandardOverpressure = shellOverpressureStandardValue;
	overpressureResult.propellantsMaxOverpressure = propellantOverpressureMaxValue;
	overpressureResult.propellantsMinOverpressure = propellantOverpressureMinValue;
	overpressureResult.mpropellantsAvgOverpressure = propellantOverpressureAvgValue;
	overpressureResult.propellantsStandardOverpressure = propellantOverpressureStandardValue;
	overpressureResult.outheatMaxOverpressure = shellOverpressureMaxValue;
	overpressureResult.outheatMinOverpressure = shellOverpressureMinValue;
	overpressureResult.outheatAvgOverpressure = shellOverpressureAvgValue;
	overpressureResult.outheatStandardOverpressure = shellOverpressureStandardValue;
	overpressureResult.insulatingheatMaxOverpressure = propellantOverpressureMaxValue;
	overpressureResult.insulatingheatMinOverpressure = propellantOverpressureMinValue;
	overpressureResult.insulatingheatAvgOverpressure = propellantOverpressureAvgValue;
	overpressureResult.insulatingheatStandardOverpressure = propellantOverpressureStandardValue;
	ModelDataManager::GetInstance()->SetSacrificeExplosionOverpressureResult(overpressureResult);




	SacrificeExplosionAnalysisResultInfo sacrificeExplosionAnalysisResultInfo;

	sacrificeExplosionAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	sacrificeExplosionAnalysisResultInfo.stressMaxValue = stressMaxValue;
	sacrificeExplosionAnalysisResultInfo.stressMinValue = stressMinValue;
	sacrificeExplosionAnalysisResultInfo.strainMaxValue = stressMaxValue / steelInfo.modulus / 1000;
	sacrificeExplosionAnalysisResultInfo.strainMinValue = stressMinValue / steelInfo.modulus / 1000;
	sacrificeExplosionAnalysisResultInfo.temperatureMaxValue = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult().propellantsMaxTemperature;
	sacrificeExplosionAnalysisResultInfo.temperatureMinValue = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult().propellantsMinTemperature;
	sacrificeExplosionAnalysisResultInfo.overpressureMaxValue = overpressureMaxValue;
	sacrificeExplosionAnalysisResultInfo.overpressureMinValue = overpressureMinValue;
	ModelDataManager::GetInstance()->SetSacrificeExplosionAnalysisResultInfo(sacrificeExplosionAnalysisResultInfo);

	return true;
}