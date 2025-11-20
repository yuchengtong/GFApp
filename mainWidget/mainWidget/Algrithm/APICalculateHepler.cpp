#include "APICalculateHepler.h"

#include <V3d_View.hxx>

#include <QMap>

#include "ModelDataManager.h"

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
	auto M = 5;//厚

	auto formulaCal = calInfo.calculation;

	auto calculateFormula = [](const QString& formula,
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
	};

	std::vector<double> results;
	results.reserve(formulaCal.size());
	for (int i = 0; i < formulaCal.size(); ++i)
	{
		double res = calculateFormula(formulaCal[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		results.push_back(res);
	}
	for (size_t i = 0; i < results.size(); ++i) {
		results[i] = results[i] * 0.7 * 0.6;
		if (results[i] < 0)
		{
			results[i] = 0;
		}
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
	double shellMaxValue = max_value; // 发动机壳体最大应力
	double shellMinValue = 0; // 发动机壳体最小应力
	double shellAvgValue = shellMaxValue * 0.6; // 发动机壳体平均应力
	double shellStandardValue = calculateStd(results); // 发动机壳体应力标准差
	double maxValue = max_value * 0.6; // 固体推进剂最大应力
	double minValue = 0; // 固体推进剂最小应力
	double avgValue = maxValue * 0.6; // 固体推进剂平均应力
	double standardValue = 0; // 固体推进剂应力标准差

	propertyValue.clear();
	propertyValue.push_back(shellMaxValue);
	propertyValue.push_back(shellMinValue);
	propertyValue.push_back(shellAvgValue);
	propertyValue.push_back(shellStandardValue);
	propertyValue.push_back(maxValue);
	propertyValue.push_back(minValue);
	propertyValue.push_back(avgValue);
	propertyValue.push_back(standardValue);
	//gfParent->GetStressResultWidget()->updateData(shellMaxValue, shellMinValue, shellAvgValue, shellStandardValue, maxValue, minValue, avgValue, standardValue);


	FallAnalysisResultInfo fallAnalysisResultInfo;

	fallAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	fallAnalysisResultInfo.stressMaxValue = max_value;
	fallAnalysisResultInfo.stressMinValue = min_value;
	fallAnalysisResultInfo.strainMaxValue = max_value * steelInfo.modulus;
	fallAnalysisResultInfo.strainMinValue = min_value * steelInfo.modulus;
	fallAnalysisResultInfo.temperatureMaxValue = ModelDataManager::GetInstance()->GetFallTemperatureResult().propellantsMaxTemperature;
	fallAnalysisResultInfo.temperatureMinValue = ModelDataManager::GetInstance()->GetFallTemperatureResult().propellantsMinTemperature;
	fallAnalysisResultInfo.overpressureMaxValue = ModelDataManager::GetInstance()->GetFallOverpressureResult().propellantsMaxOverpressure;
	fallAnalysisResultInfo.overpressureMinValue = ModelDataManager::GetInstance()->GetFallOverpressureResult().propellantsMinOverpressure;
	ModelDataManager::GetInstance()->SetFallAnalysisResultInfo(fallAnalysisResultInfo);

	// 应力分析结果
	StressResult fallStressResult;
	fallStressResult.metalsMaxStress = shellMaxValue;
	fallStressResult.metalsMinStress = shellMinValue;
	fallStressResult.metalsAvgStress = shellAvgValue;
	fallStressResult.metalsStandardStress = shellStandardValue;
	fallStressResult.propellantsMaxStress = maxValue;
	fallStressResult.propellantsMinStress = minValue;
	fallStressResult.propellantsAvgStress = avgValue;
	fallStressResult.propellantsStandardStress = standardValue;
	fallStressResult.outheatMaxStress = shellMaxValue;
	fallStressResult.outheatMinStress = shellMinValue;
	fallStressResult.outheatAvgStress = shellAvgValue;
	fallStressResult.outheatStandardStress = shellStandardValue;
	fallStressResult.insulatingheatMaxStress = maxValue;
	fallStressResult.insulatingheatMinStress = minValue;
	fallStressResult.insulatingheatAvgStress = avgValue;
	fallStressResult.insulatingheatStandardStress = standardValue;
	ModelDataManager::GetInstance()->SetFallStressResult(fallStressResult);

	// 应变分析结果
	StrainResult fallStrainResult;
	fallStrainResult.metalsMaxStrain = fallStressResult.metalsMaxStress * steelInfo.modulus;
	fallStrainResult.metalsMinStrain = fallStressResult.metalsMinStress * steelInfo.modulus;
	fallStrainResult.metalsAvgStrain = fallStressResult.metalsAvgStress * steelInfo.modulus;
	fallStrainResult.metalsStandardStrain = fallStressResult.metalsStandardStress * steelInfo.modulus;
	fallStrainResult.propellantsMaxStrain = fallStressResult.propellantsMaxStress * steelInfo.modulus;
	fallStrainResult.propellantsMinStrain = fallStressResult.propellantsMaxStress * steelInfo.modulus;
	fallStrainResult.mpropellantsAvgStrain = fallStressResult.propellantsAvgStress * steelInfo.modulus;
	fallStrainResult.propellantsStandardStrain = fallStressResult.propellantsStandardStress * steelInfo.modulus;
	fallStrainResult.outheatMaxStrain = fallStressResult.outheatMaxStress * steelInfo.modulus;
	fallStrainResult.outheatMinStrain = fallStressResult.outheatMinStress * steelInfo.modulus;
	fallStrainResult.outheatAvgStrain = fallStressResult.outheatAvgStress * steelInfo.modulus;
	fallStrainResult.outheatStandardStrain = fallStressResult.outheatStandardStress * steelInfo.modulus;
	fallStrainResult.insulatingheatMaxStrain = fallStressResult.insulatingheatMaxStress * steelInfo.modulus;
	fallStrainResult.insulatingheatMinStrain = fallStressResult.insulatingheatMinStress * steelInfo.modulus;
	fallStrainResult.insulatingheatAvgStrain = fallStressResult.insulatingheatAvgStress * steelInfo.modulus;
	fallStrainResult.insulatingheatStandardStrain = fallStressResult.insulatingheatStandardStress * steelInfo.modulus;
	ModelDataManager::GetInstance()->SetFallStrainResult(fallStrainResult);
    
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
	auto L = 5;//厚度
	auto M = shootInfo.speed * 1000;//撞击速度

	auto formulaCal = calInfo.shootCalculation;

	auto calculateFormula = [](const QString& formula,
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
	};

	std::vector<double> results;
	results.reserve(formulaCal.size());
	for (int i = 0; i < formulaCal.size(); ++i)
	{
		double res = calculateFormula(formulaCal[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		results.push_back(res);
	}
	for (size_t i = 0; i < results.size(); ++i) {
		results[i] = results[i] * 0.7 * 0.6;
		if (results[i] < 0)
		{
			results[i] = 0;
		}
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
	double shellMaxValue = max_value; // 发动机壳体最大应力
	double shellMinValue = 0; // 发动机壳体最小应力
	double shellAvgValue = shellMaxValue * 0.6; // 发动机壳体平均应力
	double shellStandardValue = calculateStd(results); // 发动机壳体应力标准差
	double maxValue = max_value * 0.6; // 固体推进剂最大应力
	double minValue = 0; // 固体推进剂最小应力
	double avgValue = maxValue * 0.6; // 固体推进剂平均应力
	double standardValue = 0; // 固体推进剂应力标准差

	propertyValue.clear();
	propertyValue.push_back(shellMaxValue);
	propertyValue.push_back(shellMinValue);
	propertyValue.push_back(shellAvgValue);
	propertyValue.push_back(shellStandardValue);
	propertyValue.push_back(maxValue);
	propertyValue.push_back(minValue);
	propertyValue.push_back(avgValue);
	propertyValue.push_back(standardValue);
	//gfParent->GetStressResultWidget()->updateData(shellMaxValue, shellMinValue, shellAvgValue, shellStandardValue, maxValue, minValue, avgValue, standardValue);


	ShootAnalysisResultInfo shootAnalysisResultInfo;

	shootAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	shootAnalysisResultInfo.stressMaxValue = max_value;
	shootAnalysisResultInfo.stressMinValue = min_value;
	shootAnalysisResultInfo.strainMaxValue = max_value * steelInfo.modulus;
	shootAnalysisResultInfo.strainMinValue = min_value * steelInfo.modulus;
	shootAnalysisResultInfo.temperatureMaxValue = ModelDataManager::GetInstance()->GetShootTemperatureResult().propellantsMaxTemperature;
	shootAnalysisResultInfo.temperatureMinValue = ModelDataManager::GetInstance()->GetShootTemperatureResult().propellantsMinTemperature;
	shootAnalysisResultInfo.overpressureMaxValue = ModelDataManager::GetInstance()->GetShootOverpressureResult().propellantsMaxOverpressure;
	shootAnalysisResultInfo.overpressureMinValue = ModelDataManager::GetInstance()->GetShootOverpressureResult().propellantsMinOverpressure;
	ModelDataManager::GetInstance()->SetShootAnalysisResultInfo(shootAnalysisResultInfo);

	// 应力分析结果
	StressResult shootStressResult;
	shootStressResult.metalsMaxStress = shellMaxValue;
	shootStressResult.metalsMinStress = shellMinValue;
	shootStressResult.metalsAvgStress = shellAvgValue;
	shootStressResult.metalsStandardStress = shellStandardValue;
	shootStressResult.propellantsMaxStress = maxValue;
	shootStressResult.propellantsMinStress = minValue;
	shootStressResult.propellantsAvgStress = avgValue;
	shootStressResult.propellantsStandardStress = standardValue;
	shootStressResult.outheatMaxStress = shellMaxValue;
	shootStressResult.outheatMinStress = shellMinValue;
	shootStressResult.outheatAvgStress = shellAvgValue;
	shootStressResult.outheatStandardStress = shellStandardValue;
	shootStressResult.insulatingheatMaxStress = maxValue;
	shootStressResult.insulatingheatMinStress = minValue;
	shootStressResult.insulatingheatAvgStress = avgValue;
	shootStressResult.insulatingheatStandardStress = standardValue;
	ModelDataManager::GetInstance()->SetShootStressResult(shootStressResult);

	// 应变分析结果
	StrainResult shootStrainResult;
	shootStrainResult.metalsMaxStrain = shootStressResult.metalsMaxStress * steelInfo.modulus;
	shootStrainResult.metalsMinStrain = shootStressResult.metalsMinStress * steelInfo.modulus;
	shootStrainResult.metalsAvgStrain = shootStressResult.metalsAvgStress * steelInfo.modulus;
	shootStrainResult.metalsStandardStrain = shootStressResult.metalsStandardStress * steelInfo.modulus;
	shootStrainResult.propellantsMaxStrain = shootStressResult.propellantsMaxStress * steelInfo.modulus;
	shootStrainResult.propellantsMinStrain = shootStressResult.propellantsMaxStress * steelInfo.modulus;
	shootStrainResult.mpropellantsAvgStrain = shootStressResult.propellantsAvgStress * steelInfo.modulus;
	shootStrainResult.propellantsStandardStrain = shootStressResult.propellantsStandardStress * steelInfo.modulus;
	shootStrainResult.outheatMaxStrain = shootStressResult.outheatMaxStress * steelInfo.modulus;
	shootStrainResult.outheatMinStrain = shootStressResult.outheatMinStress * steelInfo.modulus;
	shootStrainResult.outheatAvgStrain = shootStressResult.outheatAvgStress * steelInfo.modulus;
	shootStrainResult.outheatStandardStrain = shootStressResult.outheatStandardStress * steelInfo.modulus;
	shootStrainResult.insulatingheatMaxStrain = shootStressResult.insulatingheatMaxStress * steelInfo.modulus;
	shootStrainResult.insulatingheatMinStrain = shootStressResult.insulatingheatMinStress * steelInfo.modulus;
	shootStrainResult.insulatingheatAvgStrain = shootStressResult.insulatingheatAvgStress * steelInfo.modulus;
	shootStrainResult.insulatingheatStandardStrain = shootStressResult.insulatingheatStandardStress * steelInfo.modulus;
	ModelDataManager::GetInstance()->SetShootStrainResult(shootStrainResult);

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
	auto L = 5;//厚度
	auto M = fragmentationSettingInfo.speed * 1000;//撞击速度

	auto formulaCal = calInfo.shootCalculation;

	auto calculateFormula = [](const QString& formula,
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
	};

	std::vector<double> results;
	results.reserve(formulaCal.size());
	for (int i = 0; i < formulaCal.size(); ++i)
	{
		double res = calculateFormula(formulaCal[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		results.push_back(res);
	}
	for (size_t i = 0; i < results.size(); ++i) {
		results[i] = results[i] * 0.7 * 0.6;
		if (results[i] < 0)
		{
			results[i] = 0;
		}
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
	double shellMaxValue = max_value; // 发动机壳体最大应力
	double shellMinValue = 0; // 发动机壳体最小应力
	double shellAvgValue = shellMaxValue * 0.6; // 发动机壳体平均应力
	double shellStandardValue = calculateStd(results); // 发动机壳体应力标准差
	double maxValue = max_value * 0.6; // 固体推进剂最大应力
	double minValue = 0; // 固体推进剂最小应力
	double avgValue = maxValue * 0.6; // 固体推进剂平均应力
	double standardValue = 0; // 固体推进剂应力标准差

	propertyValue.clear();
	propertyValue.push_back(shellMaxValue);
	propertyValue.push_back(shellMinValue);
	propertyValue.push_back(shellAvgValue);
	propertyValue.push_back(shellStandardValue);
	propertyValue.push_back(maxValue);
	propertyValue.push_back(minValue);
	propertyValue.push_back(avgValue);
	propertyValue.push_back(standardValue);
	//gfParent->GetStressResultWidget()->updateData(shellMaxValue, shellMinValue, shellAvgValue, shellStandardValue, maxValue, minValue, avgValue, standardValue);


	FragmentationAnalysisResultInfo fragmentationAnalysisResultInfo;

	fragmentationAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	fragmentationAnalysisResultInfo.stressMaxValue = max_value;
	fragmentationAnalysisResultInfo.stressMinValue = min_value;
	fragmentationAnalysisResultInfo.strainMaxValue = max_value * steelInfo.modulus;
	fragmentationAnalysisResultInfo.strainMinValue = min_value * steelInfo.modulus;
	fragmentationAnalysisResultInfo.temperatureMaxValue = ModelDataManager::GetInstance()->GetFragmentationImpactTemperatureResult().propellantsMaxTemperature;
	fragmentationAnalysisResultInfo.temperatureMinValue = ModelDataManager::GetInstance()->GetFragmentationImpactTemperatureResult().propellantsMinTemperature;
	fragmentationAnalysisResultInfo.overpressureMaxValue = ModelDataManager::GetInstance()->GetFragmentationImpactOverpressureResult().propellantsMaxOverpressure;
	fragmentationAnalysisResultInfo.overpressureMinValue = ModelDataManager::GetInstance()->GetFragmentationImpactOverpressureResult().propellantsMinOverpressure;
	ModelDataManager::GetInstance()->SetFragmentationAnalysisResultInfo(fragmentationAnalysisResultInfo);

	// 应力分析结果
	StressResult fragmentationImpactStressResult;
	fragmentationImpactStressResult.metalsMaxStress = shellMaxValue;
	fragmentationImpactStressResult.metalsMinStress = shellMinValue;
	fragmentationImpactStressResult.metalsAvgStress = shellAvgValue;
	fragmentationImpactStressResult.metalsStandardStress = shellStandardValue;
	fragmentationImpactStressResult.propellantsMaxStress = maxValue;
	fragmentationImpactStressResult.propellantsMinStress = minValue;
	fragmentationImpactStressResult.propellantsAvgStress = avgValue;
	fragmentationImpactStressResult.propellantsStandardStress = standardValue;
	fragmentationImpactStressResult.outheatMaxStress = shellMaxValue;
	fragmentationImpactStressResult.outheatMinStress = shellMinValue;
	fragmentationImpactStressResult.outheatAvgStress = shellAvgValue;
	fragmentationImpactStressResult.outheatStandardStress = shellStandardValue;
	fragmentationImpactStressResult.insulatingheatMaxStress = maxValue;
	fragmentationImpactStressResult.insulatingheatMinStress = minValue;
	fragmentationImpactStressResult.insulatingheatAvgStress = avgValue;
	fragmentationImpactStressResult.insulatingheatStandardStress = standardValue;
	ModelDataManager::GetInstance()->SetFragmentationImpactStressResult(fragmentationImpactStressResult);

	// 应变分析结果
	StrainResult fragmentationImpactStrainResult;
	fragmentationImpactStrainResult.metalsMaxStrain = fragmentationImpactStressResult.metalsMaxStress * steelInfo.modulus;
	fragmentationImpactStrainResult.metalsMinStrain = fragmentationImpactStressResult.metalsMinStress * steelInfo.modulus;
	fragmentationImpactStrainResult.metalsAvgStrain = fragmentationImpactStressResult.metalsAvgStress * steelInfo.modulus;
	fragmentationImpactStrainResult.metalsStandardStrain = fragmentationImpactStressResult.metalsStandardStress * steelInfo.modulus;
	fragmentationImpactStrainResult.propellantsMaxStrain = fragmentationImpactStressResult.propellantsMaxStress * steelInfo.modulus;
	fragmentationImpactStrainResult.propellantsMinStrain = fragmentationImpactStressResult.propellantsMinStress * steelInfo.modulus;
	fragmentationImpactStrainResult.mpropellantsAvgStrain = fragmentationImpactStressResult.propellantsAvgStress * steelInfo.modulus;
	fragmentationImpactStrainResult.propellantsStandardStrain = fragmentationImpactStressResult.propellantsStandardStress * steelInfo.modulus;
	fragmentationImpactStrainResult.outheatMaxStrain = fragmentationImpactStressResult.outheatMaxStress * steelInfo.modulus;
	fragmentationImpactStrainResult.outheatMinStrain = fragmentationImpactStressResult.outheatMinStress * steelInfo.modulus;
	fragmentationImpactStrainResult.outheatAvgStrain = fragmentationImpactStressResult.outheatAvgStress * steelInfo.modulus;
	fragmentationImpactStrainResult.outheatStandardStrain = fragmentationImpactStressResult.outheatStandardStress * steelInfo.modulus;
	fragmentationImpactStrainResult.insulatingheatMaxStrain = fragmentationImpactStressResult.insulatingheatMaxStress * steelInfo.modulus;
	fragmentationImpactStrainResult.insulatingheatMinStrain = fragmentationImpactStressResult.insulatingheatMinStress * steelInfo.modulus;
	fragmentationImpactStrainResult.insulatingheatAvgStrain = fragmentationImpactStressResult.insulatingheatAvgStress * steelInfo.modulus;
	fragmentationImpactStrainResult.insulatingheatStandardStrain = fragmentationImpactStressResult.insulatingheatStandardStress * steelInfo.modulus;
	ModelDataManager::GetInstance()->SetShootStrainResult(fragmentationImpactStrainResult);


	return true;
}

