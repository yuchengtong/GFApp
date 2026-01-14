#include "APICalculateHepler.h"

#include <V3d_View.hxx>

#include <QMap>

#include "ModelDataManager.h"


QVector<int> m_steelArray = { 1, 2, 3, 4, 5, 6, 10, 11, 15, 16, 20, 21, 25, 26, 30, 31, 35, 36 };


double calculate(const QString& formula,
	double B, double C, double D, double E,
	double F, double G, double H, double I,
	double J, double K, double L, double M, double A)
{
	QString processedFormula = formula;
	processedFormula.remove(' '); // 移除所有空格，避免干扰匹配

	// 变量映射：保持原映射关系，兼容A-M变量
	const QMap<QString, double> varMap = {
		{"A", A}, {"B", B}, {"C", C}, {"D", D}, {"E", E},
		{"F", F}, {"G", G}, {"H", H}, {"I", I}, {"J", J},
		{"K", K}, {"L", L}, {"M", M}
	};

	/************************ 核心修改：正则表达式（支持二次项） ************************/
	// 匹配格式：符号 + 系数 + （变量部分：纯数字 / 单个变量 / 变量平方 / 两个变量乘积）
	QRegExp regExp("([+-]?)((?:\\d+(?:\\.\\d*)?)|(?:\\.\\d+))(?:(?:\\*([A-Z])(?:\\^2|\\*([A-Z]))?)?)");
	regExp.setMinimal(false); // 贪婪匹配，确保获取完整项

	double result = 0.0;
	int pos = 0;
	int matchCount = 0; // 统计有效匹配项数

	// 补全公式开头符号，统一处理逻辑（原逻辑保留）
	if (!processedFormula.isEmpty() && processedFormula[0] != '+' && processedFormula[0] != '-') {
		processedFormula = "+" + processedFormula;
	}

	// 循环匹配所有有效项（原循环结构保留，内部逻辑升级）
	while ((pos = regExp.indexIn(processedFormula, pos)) != -1) {
		++matchCount;
		// 捕获分组内容
		QString signStr = regExp.cap(1);       // 符号（+/-）
		QString coeffStr = regExp.cap(2);      // 系数（整数/小数）
		QString var1Str = regExp.cap(3);       // 第一个变量（X in X^2 或 X in X*Y）
		QString var2Str = regExp.cap(4);       // 第二个变量（Y in X*Y，平方项时为空）

		// 1. 解析符号（原逻辑保留）
		double sign = (signStr == "-") ? -1.0 : 1.0;

		// 2. 解析系数（原逻辑保留，含异常处理）
		bool ok = false;
		double coeff = coeffStr.toDouble(&ok);
		if (!ok) {
			throw std::invalid_argument(QString("无效系数: %1").arg(coeffStr).toStdString());
		}

		// 3. 计算当前项的值（核心升级：支持二次项）
		double term = sign * coeff;
		if (!var1Str.isEmpty()) {
			// 检查第一个变量是否合法
			if (!varMap.contains(var1Str)) {
				throw std::invalid_argument(QString("未知变量: %1").arg(var1Str).toStdString());
			}
			double var1Val = varMap[var1Str];

			if (!var2Str.isEmpty()) {
				// 情况1：变量乘积项（X*Y，如B*C、B*D）
				if (!varMap.contains(var2Str)) {
					throw std::invalid_argument(QString("未知变量: %1").arg(var2Str).toStdString());
				}
				double var2Val = varMap[var2Str];
				term *= (var1Val * var2Val); // 符号×系数×变量1×变量2
			}
			else {
				// 判断是否是平方项（通过正则匹配的结构，var1Str存在且var2Str为空时，要么是一次项，要么是平方项）
				// 提取变量部分的原始匹配，判断是否包含^2
				QString varPart = regExp.cap(0).mid(signStr.length() + coeffStr.length());
				if (varPart.contains("^2")) {
					// 情况2：变量平方项（X^2，如B^2、C^2）
					term *= (var1Val * var1Val); // 符号×系数×变量^2
				}
				else {
					// 情况3：一次项（X，如B、C，原线性项逻辑）
					term *= var1Val; // 符号×系数×变量
				}
			}
		}
		// 情况4：纯数字项（var1Str为空，直接使用 term = sign*coeff，无需额外计算）

		// 累加当前项到结果
		result += term;
		pos += regExp.matchedLength();
	}

	// 公式合法性校验（原逻辑保留）
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

double calculateAvg(const std::vector<double> data)
{
	if (data.empty()) {
		return 0.0;
	}
	double sum = std::accumulate(data.begin(), data.end(), 0.0);
	double mean = sum / data.size();
	return mean;
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
	auto C = steelInfo.modulus / 1000000; //训练模型是Pa*e-9
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;
	auto J = fallInfo.high * 1000;//跌落高度
	auto K = modelGeomInfo.length;//长
	auto L = modelGeomInfo.width;//宽
	auto M = modelGeomInfo.thickness;//厚


	// 应力
	auto stressCalculation = calInfo.fallStressCalculation;

	std::vector<double> steelStressResults;
	std::vector<double> propellantStressResults;

	//std::vector<double> stressResults;
	//stressResults.reserve(stressCalculation.size());
	for (int i = 0; i < stressCalculation.size(); ++i)
	{
		double res = calculate(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		res = res * 0.5;
		if (!m_steelArray.contains(i+1))
		{
			propellantStressResults.push_back(res);
		}
		else
		{
			steelStressResults.push_back(res);
		}
	}
	
	double calSteelStressMinValue = *std::min_element(steelStressResults.begin(), steelStressResults.end());
	double calSteelStressMaxValue = *std::max_element(steelStressResults.begin(), steelStressResults.end());

	double calPropellantStressMinValue = *std::min_element(propellantStressResults.begin(), propellantStressResults.end());
	double calPropellantStressMaxValue = *std::max_element(propellantStressResults.begin(), propellantStressResults.end());

	// 更新结果
	double shellStressMaxValue = calSteelStressMaxValue; // 发动机壳体最大应力
	double shellStressMinValue = 0; // 发动机壳体最小应力
	double shellStressAvgValue = calculateAvg(steelStressResults); // 发动机壳体平均应力
	double shellStressStandardValue = calculateStd(steelStressResults); // 发动机壳体应力标准差
	double propellantStressMaxValue = calPropellantStressMaxValue; // 固体推进剂最大应力
	double propellantStressMinValue = 0; // 固体推进剂最小应力
	double propellantStressAvgValue = calculateAvg(propellantStressResults); // 固体推进剂平均应力
	double propellantStressStandardValue = calculateStd(propellantStressResults); // 固体推进剂应力标准差

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
	fallStressResult.outheatMaxStress = shellStressMaxValue * 1.05;
	fallStressResult.outheatMinStress = shellStressMinValue * 1.05;
	fallStressResult.outheatAvgStress = shellStressAvgValue * 1.05;
	fallStressResult.outheatStandardStress = shellStressStandardValue * 1.05;
	fallStressResult.insulatingheatMaxStress = shellStressMaxValue * 1.02;
	fallStressResult.insulatingheatMinStress = shellStressMinValue * 1.02;
	fallStressResult.insulatingheatAvgStress = shellStressAvgValue * 1.02;
	fallStressResult.insulatingheatStandardStress = shellStressStandardValue * 1.02;
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
	std::vector<double> steelTemperatureResults;
	std::vector<double> propellantTemperatureResults;
	/*std::vector<double> temperatureResults;
	temperatureResults.reserve(temperatureCalculation.size());*/
	for (int i = 0; i < temperatureCalculation.size(); ++i)
	{
		double res = calculate(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		if (!m_steelArray.contains(i + 1))
		{
			propellantTemperatureResults.push_back(res);
		}
		else
		{
			steelTemperatureResults.push_back(res);
		}
	}
	
	double calSteelTemperatureMinValue = *std::min_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
	double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());

	double calPropellantTemperatureMinValue = *std::min_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());
	double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());

	// 更新结果
	double shellTemperatureMaxValue = calSteelTemperatureMaxValue; // 发动机壳体最大温度
	double shellTemperatureMinValue = calSteelTemperatureMinValue; // 发动机壳体最小温度
	double shellTemperatureAvgValue = calculateAvg(steelTemperatureResults); // 发动机壳体平均温度
	double shellTemperatureStandardValue = calculateStd(steelTemperatureResults); // 发动机壳体温度标准差
	double propellantTemperatureMaxValue = calPropellantTemperatureMaxValue; // 固体推进剂最大温度
	double propellantTemperatureMinValue = calPropellantTemperatureMinValue; // 固体推进剂最小温度
	double propellantTemperatureAvgValue = calculateAvg(propellantTemperatureResults); // 固体推进剂平均温度
	double propellantTemperatureStandardValue = calculateStd(propellantTemperatureResults); // 固体推进剂温度标准差

	
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
	temperatureResult.outheatMaxTemperature = shellTemperatureMaxValue * 1.05;
	temperatureResult.outheatMinTemperature = shellTemperatureMinValue * 1.05;
	temperatureResult.outheatAvgTemperature = shellTemperatureAvgValue * 1.05;
	temperatureResult.outheatStandardTemperature = shellTemperatureStandardValue * 1.05;
	temperatureResult.insulatingheatMaxTemperature = shellTemperatureMaxValue * 1.02;
	temperatureResult.insulatingheatMinTemperature = shellTemperatureMinValue * 1.02;
	temperatureResult.insulatingheatAvgTemperature = shellTemperatureAvgValue * 1.02;
	temperatureResult.insulatingheatStandardTemperature = shellTemperatureStandardValue * 1.02;
	ModelDataManager::GetInstance()->SetFallTemperatureResult(temperatureResult);



	// 超压
	auto overpressureCalculation = calInfo.fallOverpressureCalculation;
	std::vector<double> steelOverpressureResults;
	std::vector<double> propellantOverpressureResults;
	//std::vector<double> overpressureResults;
	//overpressureResults.reserve(overpressureCalculation.size());
	for (int i = 0; i < overpressureCalculation.size(); ++i)
	{
		double res = calculate(overpressureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		if (!m_steelArray.contains(i + 1))
		{
			propellantOverpressureResults.push_back(res);
		}
		else
		{
			steelOverpressureResults.push_back(res);
		}
		
	}
	
	double calSteelOverpressureMinValue = *std::min_element(steelOverpressureResults.begin(), steelOverpressureResults.end());
	double calSteelOverpressureMaxValue = *std::max_element(steelOverpressureResults.begin(), steelOverpressureResults.end());

	double calPropellantOverpressureMinValue = *std::min_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());
	double calPropellantOverpressureMaxValue = *std::max_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());

	// 更新结果
	double shellOverpressureMaxValue = calSteelOverpressureMaxValue; // 发动机壳体最大超压
	double shellOverpressureMinValue = calSteelOverpressureMinValue; // 发动机壳体最小超压
	double shellOverpressureAvgValue = calculateAvg(steelOverpressureResults); // 发动机壳体平均超压
	double shellOverpressureStandardValue = calculateStd(steelOverpressureResults); // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = calPropellantOverpressureMaxValue; // 固体推进剂最大超压
	double propellantOverpressureMinValue = calPropellantOverpressureMinValue; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = calculateAvg(propellantOverpressureResults); // 固体推进剂平均超压
	double propellantOverpressureStandardValue = calculateStd(propellantOverpressureResults); // 固体推进剂超压标准差


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
	overpressureResult.outheatMaxOverpressure = shellOverpressureMaxValue * 1.05;
	overpressureResult.outheatMinOverpressure = shellOverpressureMinValue * 1.05;
	overpressureResult.outheatAvgOverpressure = shellOverpressureAvgValue * 1.05;
	overpressureResult.outheatStandardOverpressure = shellOverpressureStandardValue * 1.05;
	overpressureResult.insulatingheatMaxOverpressure = shellOverpressureMaxValue * 1.02;
	overpressureResult.insulatingheatMinOverpressure = shellOverpressureMinValue * 1.02;
	overpressureResult.insulatingheatAvgOverpressure = shellOverpressureAvgValue * 1.02;
	overpressureResult.insulatingheatStandardOverpressure = shellOverpressureStandardValue * 1.02;
	ModelDataManager::GetInstance()->SetFallOverpressureResult(overpressureResult);




	FallAnalysisResultInfo fallAnalysisResultInfo;

	fallAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	fallAnalysisResultInfo.stressMaxValue = calSteelStressMaxValue;
	fallAnalysisResultInfo.stressMinValue = calSteelStressMinValue;
	fallAnalysisResultInfo.strainMaxValue = calSteelStressMaxValue / steelInfo.modulus / 1000;
	fallAnalysisResultInfo.strainMinValue = calSteelStressMinValue / steelInfo.modulus / 1000;
	fallAnalysisResultInfo.temperatureMaxValue = calSteelTemperatureMaxValue;
	fallAnalysisResultInfo.temperatureMinValue = calSteelTemperatureMinValue;
	fallAnalysisResultInfo.overpressureMaxValue = calPropellantOverpressureMaxValue;
	fallAnalysisResultInfo.overpressureMinValue = calPropellantOverpressureMinValue;
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
	auto C = steelInfo.modulus / 1000000;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width / 2;//半径
	auto L = modelGeomInfo.thickness;//厚
	auto M = fastCombustionSettingInfo.temperature;//环境温度

	auto formulaCal = calInfo.fastCombustionCalculation;

	

	//std::vector<double> results;
	//results.reserve(formulaCal.size());
	std::vector<double> steelTemperatureResults;
	std::vector<double> propellantTemperatureResults;
	for (int i = 0; i < formulaCal.size(); ++i)
	{
		double res = calculate(formulaCal[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		if (!m_steelArray.contains(i + 1))
		{
			propellantTemperatureResults.push_back(res);
		}
		else
		{
			steelTemperatureResults.push_back(res);
		}
	}

	double calSteelTemperatureMinValue = *std::min_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
	double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());

	double calPropellantTemperatureMinValue = *std::min_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());
	double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());

	
	// 更新结果
	double shellMaxValue = calSteelTemperatureMaxValue; // 发动机壳体最大温度
	double shellMinValue = calSteelTemperatureMinValue; // 发动机壳体最小温度
	double shellAvgValue = calculateAvg(steelTemperatureResults); // 发动机壳体平均温度
	double shellStandardValue = calculateStd(steelTemperatureResults); // 发动机壳体温度标准差
	double maxValue = calPropellantTemperatureMaxValue; // 固体推进剂最大温度
	double minValue = calPropellantTemperatureMinValue; // 固体推进剂最小温度
	double avgValue = calculateAvg(propellantTemperatureResults); // 固体推进剂平均温度
	double standardValue = calculateStd(propellantTemperatureResults); // 固体推进剂温度标准差

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
	fastCombustionAnalysisResultInfo.temperatureMaxValue = calSteelTemperatureMaxValue;
	fastCombustionAnalysisResultInfo.temperatureMinValue = calSteelTemperatureMinValue;
	
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
	temperatureResult.outheatMaxTemperature = shellMaxValue * 1.05;
	temperatureResult.outheatMinTemperature = shellMinValue * 1.05;
	temperatureResult.outheatAvgTemperature = shellAvgValue * 1.05;
	temperatureResult.outheatStandardTemperature = shellStandardValue * 1.05;
	temperatureResult.insulatingheatMaxTemperature = shellMaxValue * 1.02;
	temperatureResult.insulatingheatMinTemperature = shellMinValue * 1.02;
	temperatureResult.insulatingheatAvgTemperature = shellAvgValue * 1.02;
	temperatureResult.insulatingheatStandardTemperature = shellStandardValue * 1.02;
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
	auto C = steelInfo.modulus / 1000000;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width / 2;//半径
	auto L = modelGeomInfo.thickness;//厚
	auto M = slowCombustionSettingInfo.temperature;//温度幅度

	auto formulaCal = calInfo.slowCombustionCalculation;

	

	//std::vector<double> results;
	//results.reserve(formulaCal.size());
	std::vector<double> steelTemperatureResults;
	std::vector<double> propellantTemperatureResults;
	for (int i = 0; i < formulaCal.size(); ++i)
	{
		double res = calculate(formulaCal[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		if (!m_steelArray.contains(i + 1))
		{
			propellantTemperatureResults.push_back(res);
		}
		else
		{
			steelTemperatureResults.push_back(res);
		}
	}

	double calSteelTemperatureMinValue = *std::min_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
	double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());

	double calPropellantTemperatureMinValue = *std::min_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());
	double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());


	// 更新结果
	double shellMaxValue = calSteelTemperatureMaxValue; // 发动机壳体最大温度
	double shellMinValue = calSteelTemperatureMinValue; // 发动机壳体最小温度
	double shellAvgValue = calculateAvg(steelTemperatureResults); // 发动机壳体平均温度
	double shellStandardValue = calculateStd(steelTemperatureResults); // 发动机壳体温度标准差
	double maxValue = calPropellantTemperatureMaxValue; // 固体推进剂最大温度
	double minValue = calPropellantTemperatureMinValue; // 固体推进剂最小温度
	double avgValue = calculateAvg(propellantTemperatureResults); // 固体推进剂平均温度
	double standardValue = calculateStd(propellantTemperatureResults); // 固体推进剂温度标准差

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
	slowCombustionAnalysisResultInfo.temperatureMaxValue = calSteelTemperatureMaxValue;
	slowCombustionAnalysisResultInfo.temperatureMinValue = calSteelTemperatureMinValue;

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
	temperatureResult.outheatMaxTemperature = shellMaxValue * 1.05;
	temperatureResult.outheatMinTemperature = shellMinValue * 1.05;
	temperatureResult.outheatAvgTemperature = shellAvgValue * 1.05;
	temperatureResult.outheatStandardTemperature = shellStandardValue * 1.05;
	temperatureResult.insulatingheatMaxTemperature = shellMaxValue * 1.02;
	temperatureResult.insulatingheatMinTemperature = shellMinValue * 1.02;
	temperatureResult.insulatingheatAvgTemperature = shellAvgValue * 1.02;
	temperatureResult.insulatingheatStandardTemperature = shellStandardValue * 1.02;
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
	auto C = steelInfo.modulus / 1000000;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = shootInfo.radius;//子弹直径
	auto L = modelGeomInfo.thickness;//厚度
	auto M = shootInfo.speed * 1000;//撞击速度

	// 应力
	auto stressCalculation = calInfo.shootStressCalculation;

	std::vector<double> steelStressResults;
	std::vector<double> propellantStressResults;

	//std::vector<double> stressResults;
	//stressResults.reserve(stressCalculation.size());
	for (int i = 0; i < stressCalculation.size(); ++i)
	{
		double res = calculate(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		res = res * 0.5;
		if (!m_steelArray.contains(i + 1))
		{
			propellantStressResults.push_back(res);
		}
		else
		{
			steelStressResults.push_back(res);
		}
	}
	double calSteelStressMinValue = *std::min_element(steelStressResults.begin(), steelStressResults.end());
	double calSteelStressMaxValue = *std::max_element(steelStressResults.begin(), steelStressResults.end());

	double calPropellantStressMinValue = *std::min_element(propellantStressResults.begin(), propellantStressResults.end());
	double calPropellantStressMaxValue = *std::max_element(propellantStressResults.begin(), propellantStressResults.end());

	// 更新结果
	double shellStressMaxValue = calSteelStressMaxValue; // 发动机壳体最大应力
	double shellStressMinValue = 0; // 发动机壳体最小应力
	double shellStressAvgValue = calculateAvg(steelStressResults); // 发动机壳体平均应力
	double shellStressStandardValue = calculateStd(steelStressResults); // 发动机壳体应力标准差
	double propellantStressMaxValue = calPropellantStressMaxValue; // 固体推进剂最大应力
	double propellantStressMinValue = 0; // 固体推进剂最小应力
	double propellantStressAvgValue = calculateAvg(propellantStressResults); // 固体推进剂平均应力
	double propellantStressStandardValue = calculateStd(propellantStressResults); // 固体推进剂应力标准差

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
	stressResult.outheatMaxStress = shellStressMaxValue * 1.05;
	stressResult.outheatMinStress = shellStressMinValue * 1.05;
	stressResult.outheatAvgStress = shellStressAvgValue * 1.05;
	stressResult.outheatStandardStress = shellStressStandardValue * 1.05;
	stressResult.insulatingheatMaxStress = shellStressMaxValue * 1.02;
	stressResult.insulatingheatMinStress = shellStressMinValue * 1.02;
	stressResult.insulatingheatAvgStress = shellStressAvgValue * 1.02;
	stressResult.insulatingheatStandardStress = shellStressStandardValue * 1.02;
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
	std::vector<double> steelTemperatureResults;
	std::vector<double> propellantTemperatureResults;
	/*std::vector<double> temperatureResults;
	temperatureResults.reserve(temperatureCalculation.size());*/
	for (int i = 0; i < temperatureCalculation.size(); ++i)
	{
		double res = calculate(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		if (!m_steelArray.contains(i + 1))
		{
			propellantTemperatureResults.push_back(res);
		}
		else
		{
			steelTemperatureResults.push_back(res);
		}
	}
	double calSteelTemperatureMinValue = *std::min_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
	double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());

	double calPropellantTemperatureMinValue = *std::min_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());
	double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());

	// 更新结果
	double shellTemperatureMaxValue = calSteelTemperatureMaxValue; // 发动机壳体最大温度
	double shellTemperatureMinValue = calSteelTemperatureMinValue; // 发动机壳体最小温度
	double shellTemperatureAvgValue = calculateAvg(steelTemperatureResults); // 发动机壳体平均温度
	double shellTemperatureStandardValue = calculateStd(steelTemperatureResults); // 发动机壳体温度标准差
	double propellantTemperatureMaxValue = calPropellantTemperatureMaxValue; // 固体推进剂最大温度
	double propellantTemperatureMinValue = calPropellantTemperatureMinValue; // 固体推进剂最小温度
	double propellantTemperatureAvgValue = calculateAvg(propellantTemperatureResults); // 固体推进剂平均温度
	double propellantTemperatureStandardValue = calculateStd(propellantTemperatureResults); // 固体推进剂温度标准差


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
	temperatureResult.outheatMaxTemperature = shellTemperatureMaxValue * 1.05;
	temperatureResult.outheatMinTemperature = shellTemperatureMinValue * 1.05;
	temperatureResult.outheatAvgTemperature = shellTemperatureAvgValue * 1.05;
	temperatureResult.outheatStandardTemperature = shellTemperatureStandardValue * 1.05;
	temperatureResult.insulatingheatMaxTemperature = shellTemperatureMaxValue * 1.02;
	temperatureResult.insulatingheatMinTemperature = shellTemperatureMinValue * 1.02;
	temperatureResult.insulatingheatAvgTemperature = shellTemperatureAvgValue * 1.02;
	temperatureResult.insulatingheatStandardTemperature = shellTemperatureStandardValue * 1.02;
	ModelDataManager::GetInstance()->SetShootTemperatureResult(temperatureResult);



	// 超压
	auto overpressureCalculation = calInfo.shootOverpressureCalculation;
	std::vector<double> steelOverpressureResults;
	std::vector<double> propellantOverpressureResults;
	//std::vector<double> overpressureResults;
	//overpressureResults.reserve(overpressureCalculation.size());
	for (int i = 0; i < overpressureCalculation.size(); ++i)
	{
		double res = calculate(overpressureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		if (!m_steelArray.contains(i + 1))
		{
			propellantOverpressureResults.push_back(res);
		}
		else
		{
			steelOverpressureResults.push_back(res);
		}

	}

	double calSteelOverpressureMinValue = *std::min_element(steelOverpressureResults.begin(), steelOverpressureResults.end());
	double calSteelOverpressureMaxValue = *std::max_element(steelOverpressureResults.begin(), steelOverpressureResults.end());

	double calPropellantOverpressureMinValue = *std::min_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());
	double calPropellantOverpressureMaxValue = *std::max_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());

	// 更新结果
	double shellOverpressureMaxValue = calSteelOverpressureMaxValue; // 发动机壳体最大超压
	double shellOverpressureMinValue = calSteelOverpressureMinValue; // 发动机壳体最小超压
	double shellOverpressureAvgValue = calculateAvg(steelOverpressureResults); // 发动机壳体平均超压
	double shellOverpressureStandardValue = calculateStd(steelOverpressureResults); // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = calPropellantOverpressureMaxValue; // 固体推进剂最大超压
	double propellantOverpressureMinValue = calPropellantOverpressureMinValue; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = calculateAvg(propellantOverpressureResults); // 固体推进剂平均超压
	double propellantOverpressureStandardValue = calculateStd(propellantOverpressureResults); // 固体推进剂超压标准差


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
	overpressureResult.outheatMaxOverpressure = shellOverpressureMaxValue * 1.05;
	overpressureResult.outheatMinOverpressure = shellOverpressureMinValue * 1.05;
	overpressureResult.outheatAvgOverpressure = shellOverpressureAvgValue * 1.05;
	overpressureResult.outheatStandardOverpressure = shellOverpressureStandardValue * 1.05;
	overpressureResult.insulatingheatMaxOverpressure = shellOverpressureMaxValue * 1.02;
	overpressureResult.insulatingheatMinOverpressure = shellOverpressureMinValue * 1.02;
	overpressureResult.insulatingheatAvgOverpressure = shellOverpressureAvgValue * 1.02;
	overpressureResult.insulatingheatStandardOverpressure = shellOverpressureStandardValue * 1.02;
	ModelDataManager::GetInstance()->SetShootOverpressureResult(overpressureResult);




	ShootAnalysisResultInfo shootAnalysisResultInfo;

	shootAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	shootAnalysisResultInfo.stressMaxValue = calSteelStressMaxValue;
	shootAnalysisResultInfo.stressMinValue = calSteelStressMinValue;
	shootAnalysisResultInfo.strainMaxValue = calSteelStressMaxValue / steelInfo.modulus / 1000;
	shootAnalysisResultInfo.strainMinValue = calSteelStressMinValue / steelInfo.modulus / 1000;
	shootAnalysisResultInfo.temperatureMaxValue = calSteelTemperatureMaxValue;
	shootAnalysisResultInfo.temperatureMinValue = calSteelTemperatureMinValue;
	shootAnalysisResultInfo.overpressureMaxValue = calPropellantOverpressureMaxValue;
	shootAnalysisResultInfo.overpressureMinValue = calPropellantOverpressureMinValue;
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
	auto C = steelInfo.modulus / 1000000;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width/2;//半径
	auto L = modelGeomInfo.thickness;//厚
	auto M = jetImpactingInfo.caliber;// 聚能装药口径

	// 应力
	auto stressCalculation = calInfo.jetImpactStressCalculation;

	std::vector<double> steelStressResults;
	std::vector<double> propellantStressResults;

	//std::vector<double> stressResults;
	//stressResults.reserve(stressCalculation.size());
	for (int i = 0; i < stressCalculation.size(); ++i)
	{
		double res = calculate(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		res = res * 0.5;
		if (!m_steelArray.contains(i + 1))
		{
			propellantStressResults.push_back(res);
		}
		else
		{
			steelStressResults.push_back(res);
		}
	}
	double calSteelStressMinValue = *std::min_element(steelStressResults.begin(), steelStressResults.end());
	double calSteelStressMaxValue = *std::max_element(steelStressResults.begin(), steelStressResults.end());

	double calPropellantStressMinValue = *std::min_element(propellantStressResults.begin(), propellantStressResults.end());
	double calPropellantStressMaxValue = *std::max_element(propellantStressResults.begin(), propellantStressResults.end());

	// 更新结果
	double shellStressMaxValue = calSteelStressMaxValue; // 发动机壳体最大应力
	double shellStressMinValue = 0; // 发动机壳体最小应力
	double shellStressAvgValue = calculateAvg(steelStressResults); // 发动机壳体平均应力
	double shellStressStandardValue = calculateStd(steelStressResults); // 发动机壳体应力标准差
	double propellantStressMaxValue = calPropellantStressMaxValue; // 固体推进剂最大应力
	double propellantStressMinValue = 0; // 固体推进剂最小应力
	double propellantStressAvgValue = calculateAvg(propellantStressResults); // 固体推进剂平均应力
	double propellantStressStandardValue = calculateStd(propellantStressResults); // 固体推进剂应力标准差

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
	stressResult.outheatMaxStress = shellStressMaxValue * 1.05;
	stressResult.outheatMinStress = shellStressMinValue * 1.05;
	stressResult.outheatAvgStress = shellStressAvgValue * 1.05;
	stressResult.outheatStandardStress = shellStressStandardValue * 1.05;
	stressResult.insulatingheatMaxStress = shellStressMaxValue * 1.02;
	stressResult.insulatingheatMinStress = shellStressMinValue * 1.02;
	stressResult.insulatingheatAvgStress = shellStressAvgValue * 1.02;
	stressResult.insulatingheatStandardStress = shellStressStandardValue * 1.02;
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
	std::vector<double> steelTemperatureResults;
	std::vector<double> propellantTemperatureResults;
	/*std::vector<double> temperatureResults;
	temperatureResults.reserve(temperatureCalculation.size());*/
	for (int i = 0; i < temperatureCalculation.size(); ++i)
	{
		double res = calculate(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		if (!m_steelArray.contains(i + 1))
		{
			propellantTemperatureResults.push_back(res);
		}
		else
		{
			steelTemperatureResults.push_back(res);
		}
	}
	double calSteelTemperatureMinValue = *std::min_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
	double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());

	double calPropellantTemperatureMinValue = *std::min_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());
	double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());

	// 更新结果
	double shellTemperatureMaxValue = calSteelTemperatureMaxValue; // 发动机壳体最大温度
	double shellTemperatureMinValue = calSteelTemperatureMinValue; // 发动机壳体最小温度
	double shellTemperatureAvgValue = calculateAvg(steelTemperatureResults); // 发动机壳体平均温度
	double shellTemperatureStandardValue = calculateStd(steelTemperatureResults); // 发动机壳体温度标准差
	double propellantTemperatureMaxValue = calPropellantTemperatureMaxValue; // 固体推进剂最大温度
	double propellantTemperatureMinValue = calPropellantTemperatureMinValue; // 固体推进剂最小温度
	double propellantTemperatureAvgValue = calculateAvg(propellantTemperatureResults); // 固体推进剂平均温度
	double propellantTemperatureStandardValue = calculateStd(propellantTemperatureResults); // 固体推进剂温度标准差


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
	temperatureResult.outheatMaxTemperature = shellTemperatureMaxValue * 1.05;
	temperatureResult.outheatMinTemperature = shellTemperatureMinValue * 1.05;
	temperatureResult.outheatAvgTemperature = shellTemperatureAvgValue * 1.05;
	temperatureResult.outheatStandardTemperature = shellTemperatureStandardValue * 1.05;
	temperatureResult.insulatingheatMaxTemperature = shellTemperatureMaxValue * 1.02;
	temperatureResult.insulatingheatMinTemperature = shellTemperatureMinValue * 1.02;
	temperatureResult.insulatingheatAvgTemperature = shellTemperatureAvgValue * 1.02;
	temperatureResult.insulatingheatStandardTemperature = shellTemperatureStandardValue * 1.02;
	ModelDataManager::GetInstance()->SetJetImpactTemperatureResult(temperatureResult);



	// 超压
	auto overpressureCalculation = calInfo.jetImpactOverpressureCalculation;
	std::vector<double> steelOverpressureResults;
	std::vector<double> propellantOverpressureResults;
	//std::vector<double> overpressureResults;
	//overpressureResults.reserve(overpressureCalculation.size());
	for (int i = 0; i < overpressureCalculation.size(); ++i)
	{
		double res = calculate(overpressureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		if (!m_steelArray.contains(i + 1))
		{
			propellantOverpressureResults.push_back(res);
		}
		else
		{
			steelOverpressureResults.push_back(res);
		}

	}

	double calSteelOverpressureMinValue = *std::min_element(steelOverpressureResults.begin(), steelOverpressureResults.end());
	double calSteelOverpressureMaxValue = *std::max_element(steelOverpressureResults.begin(), steelOverpressureResults.end());

	double calPropellantOverpressureMinValue = *std::min_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());
	double calPropellantOverpressureMaxValue = *std::max_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());

	// 更新结果
	double shellOverpressureMaxValue = calSteelOverpressureMaxValue; // 发动机壳体最大超压
	double shellOverpressureMinValue = calSteelOverpressureMinValue; // 发动机壳体最小超压
	double shellOverpressureAvgValue = calculateAvg(steelOverpressureResults); // 发动机壳体平均超压
	double shellOverpressureStandardValue = calculateStd(steelOverpressureResults); // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = calPropellantOverpressureMaxValue; // 固体推进剂最大超压
	double propellantOverpressureMinValue = calPropellantOverpressureMinValue; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = calculateAvg(propellantOverpressureResults); // 固体推进剂平均超压
	double propellantOverpressureStandardValue = calculateStd(propellantOverpressureResults); // 固体推进剂超压标准差


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
	overpressureResult.outheatMaxOverpressure = shellOverpressureMaxValue * 1.05;
	overpressureResult.outheatMinOverpressure = shellOverpressureMinValue * 1.05;
	overpressureResult.outheatAvgOverpressure = shellOverpressureAvgValue * 1.05;
	overpressureResult.outheatStandardOverpressure = shellOverpressureStandardValue * 1.05;
	overpressureResult.insulatingheatMaxOverpressure = shellOverpressureMaxValue * 1.02;
	overpressureResult.insulatingheatMinOverpressure = shellOverpressureMinValue * 1.02;
	overpressureResult.insulatingheatAvgOverpressure = shellOverpressureAvgValue * 1.02;
	overpressureResult.insulatingheatStandardOverpressure = shellOverpressureStandardValue * 1.02;
	ModelDataManager::GetInstance()->SetJetImpactOverpressureResult(overpressureResult);




	JetImpactAnalysisResultInfo jetImpactAnalysisResultInfo;

	jetImpactAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	jetImpactAnalysisResultInfo.stressMaxValue = calSteelStressMaxValue;
	jetImpactAnalysisResultInfo.stressMinValue = calSteelStressMinValue;
	jetImpactAnalysisResultInfo.strainMaxValue = calSteelStressMaxValue / steelInfo.modulus / 1000;
	jetImpactAnalysisResultInfo.strainMinValue = calSteelStressMinValue / steelInfo.modulus / 1000;
	jetImpactAnalysisResultInfo.temperatureMaxValue = calSteelTemperatureMaxValue;
	jetImpactAnalysisResultInfo.temperatureMinValue = calSteelTemperatureMinValue;
	jetImpactAnalysisResultInfo.overpressureMaxValue = calPropellantOverpressureMaxValue;
	jetImpactAnalysisResultInfo.overpressureMinValue = calPropellantOverpressureMinValue;
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
	auto C = steelInfo.modulus / 1000000;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = fragmentationSettingInfo.radius;//破片直径
	auto L = modelGeomInfo.thickness;//厚度
	auto M = fragmentationSettingInfo.speed * 1000;//撞击速度

	// 应力
	auto stressCalculation = calInfo.fragmentationImpactStressCalculation;

	std::vector<double> steelStressResults;
	std::vector<double> propellantStressResults;

	//std::vector<double> stressResults;
	//stressResults.reserve(stressCalculation.size());
	for (int i = 0; i < stressCalculation.size(); ++i)
	{
		double res = calculate(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		res = res * 0.5;
		if (!m_steelArray.contains(i + 1))
		{
			propellantStressResults.push_back(res);
		}
		else
		{
			steelStressResults.push_back(res);
		}
	}
	double calSteelStressMinValue = *std::min_element(steelStressResults.begin(), steelStressResults.end());
	double calSteelStressMaxValue = *std::max_element(steelStressResults.begin(), steelStressResults.end());

	double calPropellantStressMinValue = *std::min_element(propellantStressResults.begin(), propellantStressResults.end());
	double calPropellantStressMaxValue = *std::max_element(propellantStressResults.begin(), propellantStressResults.end());

	// 更新结果
	double shellStressMaxValue = calSteelStressMaxValue; // 发动机壳体最大应力
	double shellStressMinValue = 0; // 发动机壳体最小应力
	double shellStressAvgValue = calculateAvg(steelStressResults); // 发动机壳体平均应力
	double shellStressStandardValue = calculateStd(steelStressResults); // 发动机壳体应力标准差
	double propellantStressMaxValue = calPropellantStressMaxValue; // 固体推进剂最大应力
	double propellantStressMinValue = 0; // 固体推进剂最小应力
	double propellantStressAvgValue = calculateAvg(propellantStressResults); // 固体推进剂平均应力
	double propellantStressStandardValue = calculateStd(propellantStressResults); // 固体推进剂应力标准差

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
	stressResult.outheatMaxStress = shellStressMaxValue * 1.05;
	stressResult.outheatMinStress = shellStressMinValue * 1.05;
	stressResult.outheatAvgStress = shellStressAvgValue * 1.05;
	stressResult.outheatStandardStress = shellStressStandardValue * 1.05;
	stressResult.insulatingheatMaxStress = shellStressMaxValue * 1.02;
	stressResult.insulatingheatMinStress = shellStressMinValue * 1.02;
	stressResult.insulatingheatAvgStress = shellStressAvgValue * 1.02;
	stressResult.insulatingheatStandardStress = shellStressStandardValue * 1.02;
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
	std::vector<double> steelTemperatureResults;
	std::vector<double> propellantTemperatureResults;
	/*std::vector<double> temperatureResults;
	temperatureResults.reserve(temperatureCalculation.size());*/
	for (int i = 0; i < temperatureCalculation.size(); ++i)
	{
		double res = calculate(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		if (!m_steelArray.contains(i + 1))
		{
			propellantTemperatureResults.push_back(res);
		}
		else
		{
			steelTemperatureResults.push_back(res);
		}
	}
	double calSteelTemperatureMinValue = *std::min_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
	double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());

	double calPropellantTemperatureMinValue = *std::min_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());
	double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());

	// 更新结果
	double shellTemperatureMaxValue = calSteelTemperatureMaxValue; // 发动机壳体最大温度
	double shellTemperatureMinValue = calSteelTemperatureMinValue; // 发动机壳体最小温度
	double shellTemperatureAvgValue = calculateAvg(steelTemperatureResults); // 发动机壳体平均温度
	double shellTemperatureStandardValue = calculateStd(steelTemperatureResults); // 发动机壳体温度标准差
	double propellantTemperatureMaxValue = calPropellantTemperatureMaxValue; // 固体推进剂最大温度
	double propellantTemperatureMinValue = calPropellantTemperatureMinValue; // 固体推进剂最小温度
	double propellantTemperatureAvgValue = calculateAvg(propellantTemperatureResults); // 固体推进剂平均温度
	double propellantTemperatureStandardValue = calculateStd(propellantTemperatureResults); // 固体推进剂温度标准差


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
	temperatureResult.outheatMaxTemperature = shellTemperatureMaxValue * 1.05;
	temperatureResult.outheatMinTemperature = shellTemperatureMinValue * 1.05;
	temperatureResult.outheatAvgTemperature = shellTemperatureAvgValue * 1.05;
	temperatureResult.outheatStandardTemperature = shellTemperatureStandardValue * 1.05;
	temperatureResult.insulatingheatMaxTemperature = shellTemperatureMaxValue * 1.02;
	temperatureResult.insulatingheatMinTemperature = shellTemperatureMinValue * 1.02;
	temperatureResult.insulatingheatAvgTemperature = shellTemperatureAvgValue * 1.02;
	temperatureResult.insulatingheatStandardTemperature = shellTemperatureStandardValue * 1.02;
	ModelDataManager::GetInstance()->SetFragmentationImpactTemperatureResult(temperatureResult);



	// 超压
	auto overpressureCalculation = calInfo.fragmentationImpactOverpressureCalculationO;
	std::vector<double> steelOverpressureResults;
	std::vector<double> propellantOverpressureResults;
	//std::vector<double> overpressureResults;
	//overpressureResults.reserve(overpressureCalculation.size());
	for (int i = 0; i < overpressureCalculation.size(); ++i)
	{
		double res = calculate(overpressureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		if (!m_steelArray.contains(i + 1))
		{
			propellantOverpressureResults.push_back(res);
		}
		else
		{
			steelOverpressureResults.push_back(res);
		}

	}

	double calSteelOverpressureMinValue = *std::min_element(steelOverpressureResults.begin(), steelOverpressureResults.end());
	double calSteelOverpressureMaxValue = *std::max_element(steelOverpressureResults.begin(), steelOverpressureResults.end());

	double calPropellantOverpressureMinValue = *std::min_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());
	double calPropellantOverpressureMaxValue = *std::max_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());

	// 更新结果
	double shellOverpressureMaxValue = calSteelOverpressureMaxValue; // 发动机壳体最大超压
	double shellOverpressureMinValue = calSteelOverpressureMinValue; // 发动机壳体最小超压
	double shellOverpressureAvgValue = calculateAvg(steelOverpressureResults); // 发动机壳体平均超压
	double shellOverpressureStandardValue = calculateStd(steelOverpressureResults); // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = calPropellantOverpressureMaxValue; // 固体推进剂最大超压
	double propellantOverpressureMinValue = calPropellantOverpressureMinValue; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = calculateAvg(propellantOverpressureResults); // 固体推进剂平均超压
	double propellantOverpressureStandardValue = calculateStd(propellantOverpressureResults); // 固体推进剂超压标准差


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
	overpressureResult.outheatMaxOverpressure = shellOverpressureMaxValue * 1.05;
	overpressureResult.outheatMinOverpressure = shellOverpressureMinValue * 1.05;
	overpressureResult.outheatAvgOverpressure = shellOverpressureAvgValue * 1.05;
	overpressureResult.outheatStandardOverpressure = shellOverpressureStandardValue * 1.05;
	overpressureResult.insulatingheatMaxOverpressure = shellOverpressureMaxValue * 1.02;
	overpressureResult.insulatingheatMinOverpressure = shellOverpressureMinValue * 1.02;
	overpressureResult.insulatingheatAvgOverpressure = shellOverpressureAvgValue * 1.02;
	overpressureResult.insulatingheatStandardOverpressure = shellOverpressureStandardValue * 1.02;
	ModelDataManager::GetInstance()->SetFragmentationImpactOverpressureResult(overpressureResult);




	FragmentationAnalysisResultInfo fragmentationAnalysisResultInfo;

	fragmentationAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	fragmentationAnalysisResultInfo.stressMaxValue = calSteelStressMaxValue;
	fragmentationAnalysisResultInfo.stressMinValue = calSteelStressMinValue;
	fragmentationAnalysisResultInfo.strainMaxValue = calSteelStressMaxValue / steelInfo.modulus / 1000;
	fragmentationAnalysisResultInfo.strainMinValue = calSteelStressMinValue / steelInfo.modulus / 1000;
	fragmentationAnalysisResultInfo.temperatureMaxValue = calSteelTemperatureMaxValue;
	fragmentationAnalysisResultInfo.temperatureMinValue = calSteelTemperatureMinValue;
	fragmentationAnalysisResultInfo.overpressureMaxValue = calPropellantOverpressureMaxValue;
	fragmentationAnalysisResultInfo.overpressureMinValue = calPropellantOverpressureMinValue;
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
	auto C = steelInfo.modulus / 1000000;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width / 2;//半径
	auto L = modelGeomInfo.thickness;//厚
	auto M = explosiveBlastSettingInfo.tnt;// TNT当量


	// 应力
	auto stressCalculation = calInfo.explosiveBlastStressCalculation;

	std::vector<double> steelStressResults;
	std::vector<double> propellantStressResults;

	//std::vector<double> stressResults;
	//stressResults.reserve(stressCalculation.size());
	for (int i = 0; i < stressCalculation.size(); ++i)
	{
		double res = calculate(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		res = res * 0.1;
		if (!m_steelArray.contains(i + 1))
		{
			propellantStressResults.push_back(res);
		}
		else
		{
			steelStressResults.push_back(res);
		}
	}
	double calSteelStressMinValue = *std::min_element(steelStressResults.begin(), steelStressResults.end());
	double calSteelStressMaxValue = *std::max_element(steelStressResults.begin(), steelStressResults.end());

	double calPropellantStressMinValue = *std::min_element(propellantStressResults.begin(), propellantStressResults.end());
	double calPropellantStressMaxValue = *std::max_element(propellantStressResults.begin(), propellantStressResults.end());

	// 更新结果
	double shellStressMaxValue = calSteelStressMaxValue; // 发动机壳体最大应力
	double shellStressMinValue = 0; // 发动机壳体最小应力
	double shellStressAvgValue = calculateAvg(steelStressResults); // 发动机壳体平均应力
	double shellStressStandardValue = calculateStd(steelStressResults); // 发动机壳体应力标准差
	double propellantStressMaxValue = calPropellantStressMaxValue; // 固体推进剂最大应力
	double propellantStressMinValue = 0; // 固体推进剂最小应力
	double propellantStressAvgValue = calculateAvg(propellantStressResults); // 固体推进剂平均应力
	double propellantStressStandardValue = calculateStd(propellantStressResults); // 固体推进剂应力标准差

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
	stressResult.outheatMaxStress = shellStressMaxValue * 1.05;
	stressResult.outheatMinStress = shellStressMinValue * 1.05;
	stressResult.outheatAvgStress = shellStressAvgValue * 1.05;
	stressResult.outheatStandardStress = shellStressStandardValue * 1.05;
	stressResult.insulatingheatMaxStress = shellStressMaxValue * 1.02;
	stressResult.insulatingheatMinStress = shellStressMinValue * 1.02;
	stressResult.insulatingheatAvgStress = shellStressAvgValue * 1.02;
	stressResult.insulatingheatStandardStress = shellStressStandardValue * 1.02;
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
	std::vector<double> steelOverpressureResults;
	std::vector<double> propellantOverpressureResults;
	//std::vector<double> overpressureResults;
	//overpressureResults.reserve(overpressureCalculation.size());
	for (int i = 0; i < overpressureCalculation.size(); ++i)
	{
		double res = calculate(overpressureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		if (!m_steelArray.contains(i + 1))
		{
			propellantOverpressureResults.push_back(res);
		}
		else
		{
			steelOverpressureResults.push_back(res);
		}

	}

	double calSteelOverpressureMinValue = *std::min_element(steelOverpressureResults.begin(), steelOverpressureResults.end());
	double calSteelOverpressureMaxValue = *std::max_element(steelOverpressureResults.begin(), steelOverpressureResults.end());

	double calPropellantOverpressureMinValue = *std::min_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());
	double calPropellantOverpressureMaxValue = *std::max_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());

	// 更新结果
	double shellOverpressureMaxValue = calSteelOverpressureMaxValue; // 发动机壳体最大超压
	double shellOverpressureMinValue = calSteelOverpressureMinValue; // 发动机壳体最小超压
	double shellOverpressureAvgValue = calculateAvg(steelOverpressureResults); // 发动机壳体平均超压
	double shellOverpressureStandardValue = calculateStd(steelOverpressureResults); // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = calPropellantOverpressureMaxValue; // 固体推进剂最大超压
	double propellantOverpressureMinValue = calPropellantOverpressureMinValue; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = calculateAvg(propellantOverpressureResults); // 固体推进剂平均超压
	double propellantOverpressureStandardValue = calculateStd(propellantOverpressureResults); // 固体推进剂超压标准差



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
	overpressureResult.outheatMaxOverpressure = shellOverpressureMaxValue * 1.05;
	overpressureResult.outheatMinOverpressure = shellOverpressureMinValue * 1.05;
	overpressureResult.outheatAvgOverpressure = shellOverpressureAvgValue * 1.05;
	overpressureResult.outheatStandardOverpressure = shellOverpressureStandardValue * 1.05;
	overpressureResult.insulatingheatMaxOverpressure = shellOverpressureMaxValue * 1.02;
	overpressureResult.insulatingheatMinOverpressure = shellOverpressureMinValue * 1.02;
	overpressureResult.insulatingheatAvgOverpressure = shellOverpressureAvgValue * 1.02;
	overpressureResult.insulatingheatStandardOverpressure = shellOverpressureStandardValue * 1.02;
	ModelDataManager::GetInstance()->SetExplosiveBlastOverpressureResult(overpressureResult);




	ExplosiveBlastAnalysisResultInfo explosiveBlastAnalysisResultInfo;

	explosiveBlastAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	explosiveBlastAnalysisResultInfo.stressMaxValue = calSteelStressMaxValue;
	explosiveBlastAnalysisResultInfo.stressMinValue = calSteelStressMinValue;
	explosiveBlastAnalysisResultInfo.strainMaxValue = calSteelStressMaxValue / steelInfo.modulus / 1000;
	explosiveBlastAnalysisResultInfo.strainMinValue = calSteelStressMinValue / steelInfo.modulus / 1000;
	explosiveBlastAnalysisResultInfo.temperatureMaxValue = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult().propellantsMaxTemperature;
	explosiveBlastAnalysisResultInfo.temperatureMinValue = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult().propellantsMinTemperature;
	explosiveBlastAnalysisResultInfo.overpressureMaxValue = calPropellantOverpressureMaxValue;
	explosiveBlastAnalysisResultInfo.overpressureMinValue = calPropellantOverpressureMinValue;
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
	auto C = steelInfo.modulus/1000000;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;
	
	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width;//宽
	auto L = modelGeomInfo.thickness;//厚
	auto M = sacrificeExplosionInfo.distance;//距离

		// 应力
	auto stressCalculation = calInfo.sacrificeExplosionStressCalculation;

	std::vector<double> steelStressResults;
	std::vector<double> propellantStressResults;

	//std::vector<double> stressResults;
	//stressResults.reserve(stressCalculation.size());
	for (int i = 0; i < stressCalculation.size(); ++i)
	{
		double res = calculate(stressCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		res = res * 0.1;
		if (!m_steelArray.contains(i + 1))
		{
			propellantStressResults.push_back(res);
		}
		else
		{
			steelStressResults.push_back(res);
		}
	}
	double calSteelStressMinValue = *std::min_element(steelStressResults.begin(), steelStressResults.end());
	double calSteelStressMaxValue = *std::max_element(steelStressResults.begin(), steelStressResults.end());

	double calPropellantStressMinValue = *std::min_element(propellantStressResults.begin(), propellantStressResults.end());
	double calPropellantStressMaxValue = *std::max_element(propellantStressResults.begin(), propellantStressResults.end());

	// 更新结果
	double shellStressMaxValue = calSteelStressMaxValue; // 发动机壳体最大应力
	double shellStressMinValue = 0; // 发动机壳体最小应力
	double shellStressAvgValue = calculateAvg(steelStressResults); // 发动机壳体平均应力
	double shellStressStandardValue = calculateStd(steelStressResults); // 发动机壳体应力标准差
	double propellantStressMaxValue = calPropellantStressMaxValue; // 固体推进剂最大应力
	double propellantStressMinValue = 0; // 固体推进剂最小应力
	double propellantStressAvgValue = calculateAvg(propellantStressResults); // 固体推进剂平均应力
	double propellantStressStandardValue = calculateStd(propellantStressResults); // 固体推进剂应力标准差

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
	stressResult.outheatMaxStress = shellStressMaxValue * 1.05;
	stressResult.outheatMinStress = shellStressMinValue * 1.05;
	stressResult.outheatAvgStress = shellStressAvgValue * 1.05;
	stressResult.outheatStandardStress = shellStressStandardValue * 1.05;
	stressResult.insulatingheatMaxStress = shellStressMaxValue * 1.02;
	stressResult.insulatingheatMinStress = shellStressMinValue * 1.02;
	stressResult.insulatingheatAvgStress = shellStressAvgValue * 1.02;
	stressResult.insulatingheatStandardStress = shellStressStandardValue * 1.02;
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
	std::vector<double> steelOverpressureResults;
	std::vector<double> propellantOverpressureResults;
	//std::vector<double> overpressureResults;
	//overpressureResults.reserve(overpressureCalculation.size());
	for (int i = 0; i < overpressureCalculation.size(); ++i)
	{
		double res = calculate(overpressureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		if (!m_steelArray.contains(i + 1))
		{
			propellantOverpressureResults.push_back(res);
		}
		else
		{
			steelOverpressureResults.push_back(res);
		}

	}

	double calSteelOverpressureMinValue = *std::min_element(steelOverpressureResults.begin(), steelOverpressureResults.end());
	double calSteelOverpressureMaxValue = *std::max_element(steelOverpressureResults.begin(), steelOverpressureResults.end());

	double calPropellantOverpressureMinValue = *std::min_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());
	double calPropellantOverpressureMaxValue = *std::max_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());

	// 更新结果
	double shellOverpressureMaxValue = calSteelOverpressureMaxValue; // 发动机壳体最大超压
	double shellOverpressureMinValue = calSteelOverpressureMinValue; // 发动机壳体最小超压
	double shellOverpressureAvgValue = calculateAvg(steelOverpressureResults); // 发动机壳体平均超压
	double shellOverpressureStandardValue = calculateStd(steelOverpressureResults); // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = calPropellantOverpressureMaxValue; // 固体推进剂最大超压
	double propellantOverpressureMinValue = calPropellantOverpressureMinValue; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = calculateAvg(propellantOverpressureResults); // 固体推进剂平均超压
	double propellantOverpressureStandardValue = calculateStd(propellantOverpressureResults); // 固体推进剂超压标准差


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
	overpressureResult.outheatMaxOverpressure = shellOverpressureMaxValue * 1.05;
	overpressureResult.outheatMinOverpressure = shellOverpressureMinValue * 1.05;
	overpressureResult.outheatAvgOverpressure = shellOverpressureAvgValue * 1.05;
	overpressureResult.outheatStandardOverpressure = shellOverpressureStandardValue * 1.05;
	overpressureResult.insulatingheatMaxOverpressure = shellOverpressureMaxValue * 1.02;
	overpressureResult.insulatingheatMinOverpressure = shellOverpressureMinValue * 1.02;
	overpressureResult.insulatingheatAvgOverpressure = shellOverpressureAvgValue * 1.02;
	overpressureResult.insulatingheatStandardOverpressure = shellOverpressureStandardValue * 1.02;
	ModelDataManager::GetInstance()->SetSacrificeExplosionOverpressureResult(overpressureResult);




	SacrificeExplosionAnalysisResultInfo sacrificeExplosionAnalysisResultInfo;

	sacrificeExplosionAnalysisResultInfo.isChecked = true;
	//fallAnalysisResultInfo.triangleStructure = *aDataSource;
	sacrificeExplosionAnalysisResultInfo.stressMaxValue = calSteelStressMaxValue;
	sacrificeExplosionAnalysisResultInfo.stressMinValue = calSteelStressMinValue;
	sacrificeExplosionAnalysisResultInfo.strainMaxValue = calSteelStressMaxValue / steelInfo.modulus / 1000;
	sacrificeExplosionAnalysisResultInfo.strainMinValue = calSteelStressMinValue / steelInfo.modulus / 1000;
	sacrificeExplosionAnalysisResultInfo.temperatureMaxValue = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult().propellantsMaxTemperature;
	sacrificeExplosionAnalysisResultInfo.temperatureMinValue = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult().propellantsMinTemperature;
	sacrificeExplosionAnalysisResultInfo.overpressureMaxValue = calPropellantOverpressureMaxValue;
	sacrificeExplosionAnalysisResultInfo.overpressureMinValue = calPropellantOverpressureMinValue;
	ModelDataManager::GetInstance()->SetSacrificeExplosionAnalysisResultInfo(sacrificeExplosionAnalysisResultInfo);

	return true;
}