#include "APICalculateHepler.h"
#include <V3d_View.hxx>
#include <BRepBndLib.hxx>
#include <BRep_Builder.hxx>

#include <QMap>

#include "ModelDataManager.h"


double translateAngle(double angle, double val)
{
	double theta = std::clamp(angle, 0.0, 90.0);
	const double exp = 1.01;
	// 15°相对于0°的增益系数
	const double kMax = 1.2;
	// 90°相对于0°的系数，大于1
	const double k90 = 1.05;

	double res;
	if (theta <= 15.0)
	{
		double t = theta / 15.0;
		res = val * pow(t, exp);
		// t=1 得到 baseStress，达不到峰值；修正：区间插值
		res = val + (kMax * val - val) * pow(t, exp);
	}
	else
	{
		double t = (theta - 15.0) / 75.0;
		res = k90 * val + (kMax * val - k90 * val) * pow(1.0 - t, exp);
	}
	return res;
}

double translateFallStress(double height, double val)
{
	const double val_min = 1733.79;
	const double val_max = 1842.88;
	double val_norm = (val_max - val) / (val_max - val_min);

	const double Wh = 1.0;
	const double Wv = 0.2;
	double S = Wh * height + Wv * val_norm;

	// 优化后二次多项式参数
	const double p0 = 24.10;
	const double p1 = 1.045;
	const double p2 = 0.0274;

	double y = p0 + p1 * S + p2 * S * S;

	// 限幅保护
	//if (y < y_min) y = y_min;
	//if (y > y_max) y = y_max;
	return y;
}

double translateFallOverpressure(double height, double val)
{
	double H = height;

	// 三段基础线性函数
	double f1 = 65.0 + 16.818 * (H - 9.0);
	double f2 = 250.0 + 17.500 * (H - 20.0);
	double f3 = 950.0 + 7.500 * (H - 60.0);

	// tanh平滑权重
	double w1 = 0.5 * (1.0 - std::tanh((H - 14.0) / 4.0));
	double w2 = 0.5 * (std::tanh((H - 14.0) / 4.0) - std::tanh((H - 40.0) / 12.0));
	double w3 = 0.5 * (1.0 + std::tanh((H - 40.0) / 12.0));

	double Y_base = w1 * f1 + w2 * f2 + w3 * f3;

	// 实测值比例修正
	double res = Y_base * val / 250.0;

	// 建议恢复限幅
	//if (y < 60.0)   y = 60.0;
	//if (y > 1500.0) y = 1500.0;
	return res;
}

double translate(double x)
{
	const double k = 0.0294118;
	const double mid = 360.0;
	const double steep = 320.0;
	double base = k * x;
	double soften = 1.0 + exp((x - mid) / steep);
	double y = base / soften;

	// 封顶保底
	if (y < 0.0) y = 0.0;
	//if (y > 20.0) y = 20.0;
	return y * 6.2;
}



// 壳体点位
QVector<int> m_steelArray = { 1, 2, 3, 4, 5, 6, 10, 11, 15, 16, 20, 21, 25, 26, 30, 31, 35, 36, 37, 38, 39, 40 };


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

double calculateAvgTemp(double a, double b)
{
	std::vector<double> data;
	data.push_back(a);
	data.push_back(b);
	data.push_back(calculateAvg(data));
	return calculateAvg(data);
}

double calculateStdTemp(double a, double b)
{
	std::vector<double> data;
	data.push_back(a);
	data.push_back(b);
	data.push_back(calculateAvg(data));
	data.push_back(calculateAvgTemp(a, b));
	return calculateStd(data);
}




void APICalculateHepler::APICalculateBoundingBox(ModelGeometryInfo& info)
{
	info.shape = TopoDS_Shape();

	std::vector<TopoDS_Shape> shapes;

	if (!info.nozzleAisShape.IsNull())
	{
		shapes.push_back(info.nozzleAisShape->Shape());
	}

	if (!info.shellAisShape.IsNull())
	{
		shapes.push_back(info.shellAisShape->Shape());
	}

	if (!info.propellantAisShape.IsNull())
	{
		shapes.push_back(info.propellantAisShape->Shape());
	}

	if (!info.heatInsulatingLayerAisShape.IsNull())
	{
		shapes.push_back(info.heatInsulatingLayerAisShape->Shape());
	}

	if (shapes.empty())
	{
		return;
	}

	// 只有一个部件时，直接用该部件的 shape
	if (shapes.size() == 1)
	{
		info.shape = shapes[0];

		Bnd_Box bbox;
		BRepBndLib::Add(info.shape, bbox);
		bbox.SetGap(0.0);

		Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
		bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

		info.theXmin = xmin;
		info.theYmin = ymin;
		info.theZmin = zmin;
		info.theXmax = xmax;
		info.theYmax = ymax;
		info.theZmax = zmax;

		info.length = xmax - xmin;
		info.width = ymax - ymin;
		info.height = zmax - zmin;
		return;
	}


	BRep_Builder builder;
	TopoDS_Compound compound;
	builder.MakeCompound(compound);

	for (const auto& s : shapes)
	{
		builder.Add(compound, s);
	}

	Bnd_Box bbox;
	BRepBndLib::Add(compound, bbox);
	bbox.SetGap(0.0);

	Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
	bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

	info.theXmin = xmin;
	info.theYmin = ymin;
	info.theZmin = zmin;
	info.theXmax = xmax;
	info.theYmax = ymax;
	info.theZmax = zmax;

	info.length = xmax - xmin;
	info.width = ymax - ymin;
	info.height = zmax - zmin;

	info.shape = compound;
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
	auto C = steelInfo.modulus / 1000; 
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;
	auto J = fallInfo.high * 1000;//跌落高度
	auto K = modelGeomInfo.length;//长
	auto L = modelGeomInfo.width;//宽
	auto M = modelGeomInfo.thickness;//厚

	auto limitValue = steelInfo.tensileStrength*1.5; // 抗拉强度
	auto tangentModulus = steelInfo.tangentModulus; // 切线模量
	auto modulus = steelInfo.modulus;// 弹性模量
	double difference = (tangentModulus / modulus);
	difference = (difference - 0.15) / 0.15;
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
		res = res + res * difference * 0.1;
		
		if (!m_steelArray.contains(i+1))
		{
			
			res = translateFallStress(fallInfo.high, res);
			res = translateAngle(fallInfo.angle, res);

			if (res > limitValue)
			{
				res = limitValue;
			}
			propellantStressResults.push_back(res);

		}
		else
		{
			res = translateAngle(fallInfo.angle, res);
			if (res > limitValue)
			{
				res = limitValue;
			}
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
	fallStressResult.outheatMaxStress = shellStressMaxValue;
	fallStressResult.outheatMinStress = shellStressMinValue;
	fallStressResult.outheatAvgStress = shellStressAvgValue;
	fallStressResult.outheatStandardStress = shellStressStandardValue;
	fallStressResult.insulatingheatMaxStress = propellantStressMaxValue * 1.01;
	fallStressResult.insulatingheatMinStress = propellantStressMinValue * 1.01;
	fallStressResult.insulatingheatAvgStress = propellantStressAvgValue * 1.01;
	fallStressResult.insulatingheatStandardStress = propellantStressStandardValue * 1.01;
	fallStressResult.shellScreenFlag = true;
	fallStressResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetFallStressResult(fallStressResult);

	// 应变分析结果
	StrainResult fallStrainResult;
	fallStrainResult.metalsMaxStrain = fallStressResult.metalsMaxStress / steelInfo.modulus ;
	fallStrainResult.metalsMinStrain = fallStressResult.metalsMinStress / steelInfo.modulus ;
	fallStrainResult.metalsAvgStrain = fallStressResult.metalsAvgStress / steelInfo.modulus ;
	fallStrainResult.metalsStandardStrain = fallStressResult.metalsStandardStress / steelInfo.modulus ;
	fallStrainResult.propellantsMaxStrain = fallStressResult.propellantsMaxStress / propellantInfo.modulus ;
	fallStrainResult.propellantsMinStrain = fallStressResult.propellantsMinStress / propellantInfo.modulus;
	fallStrainResult.mpropellantsAvgStrain = fallStressResult.propellantsAvgStress / propellantInfo.modulus;
	fallStrainResult.propellantsStandardStrain = fallStressResult.propellantsStandardStress / propellantInfo.modulus;
	fallStrainResult.outheatMaxStrain = fallStressResult.outheatMaxStress / steelInfo.modulus ;
	fallStrainResult.outheatMinStrain = fallStressResult.outheatMinStress / steelInfo.modulus ;
	fallStrainResult.outheatAvgStrain = fallStressResult.outheatAvgStress / steelInfo.modulus ;
	fallStrainResult.outheatStandardStrain = fallStressResult.outheatStandardStress / steelInfo.modulus ;
	fallStrainResult.insulatingheatMaxStrain = fallStressResult.insulatingheatMaxStress / steelInfo.modulus ;
	fallStrainResult.insulatingheatMinStrain = fallStressResult.insulatingheatMinStress / steelInfo.modulus ;
	fallStrainResult.insulatingheatAvgStrain = fallStressResult.insulatingheatAvgStress / steelInfo.modulus ;
	fallStrainResult.insulatingheatStandardStrain = fallStressResult.insulatingheatStandardStress / steelInfo.modulus;
	fallStrainResult.shellScreenFlag = true;
	fallStrainResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetFallStrainResult(fallStrainResult);
    


	// 温度
	auto temperatureCalculation = calInfo.fallTemperatureCalculation;
	std::vector<double> steelTemperatureResults;
	std::vector<double> propellantTemperatureResults;
	/*std::vector<double> temperatureResults;
	temperatureResults.reserve(temperatureCalculation.size());*/

	steelTemperatureResults.push_back(25);
	propellantTemperatureResults.push_back(25);
	for (int i = 0; i < temperatureCalculation.size(); ++i)
	{
		double res = calculate(temperatureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 25)
		{
			res = 25;
		}
		res = translateAngle(fallInfo.angle, res);
		if (fallInfo.angle == 0 && res >= 30)
		{
			res = 28.589;
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

	propellantTemperatureResults.erase(
		std::remove_if(propellantTemperatureResults.begin(), propellantTemperatureResults.end(),
			[calSteelTemperatureMaxValue](double value) { return value > calSteelTemperatureMaxValue; }),
		propellantTemperatureResults.end());

	double calPropellantTemperatureMinValue = *std::min_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());
	double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());

	// 更新结果
	double shellTemperatureMaxValue = calSteelTemperatureMaxValue; // 发动机壳体最大温度
	double shellTemperatureMinValue = 25; // 发动机壳体最小温度
	double shellTemperatureAvgValue = calculateAvg(steelTemperatureResults); // 发动机壳体平均温度
	double shellTemperatureStandardValue = calculateStd(steelTemperatureResults); // 发动机壳体温度标准差
	double propellantTemperatureMaxValue = calPropellantTemperatureMaxValue; // 固体推进剂最大温度
	if (propellantTemperatureMaxValue == 25)
	{
		propellantTemperatureMaxValue = propellantTemperatureMaxValue * 1.01;
	}
	double propellantTemperatureMinValue = 25; // 固体推进剂最小温度
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
	temperatureResult.outheatMaxTemperature = shellTemperatureMaxValue * 1.01;
	temperatureResult.outheatMinTemperature = 25;
	temperatureResult.outheatAvgTemperature = shellTemperatureAvgValue * 1.01;
	temperatureResult.outheatStandardTemperature = shellTemperatureStandardValue * 1.01;
	temperatureResult.insulatingheatMaxTemperature = propellantTemperatureMaxValue * 1.01;
	temperatureResult.insulatingheatMinTemperature = 25;
	temperatureResult.insulatingheatAvgTemperature = propellantTemperatureAvgValue * 1.01;
	temperatureResult.insulatingheatStandardTemperature = propellantTemperatureStandardValue * 1.01;
	temperatureResult.shellScreenFlag = true;
	temperatureResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetFallTemperatureResult(temperatureResult);



	// 超压
	auto overpressureCalculation = calInfo.fallOverpressureCalculation;
	std::vector<double> steelOverpressureResults;
	std::vector<double> propellantOverpressureResults;
	// 反应度
	std::vector<double> propellantReactionDegreeResults;
	//std::vector<double> overpressureResults;
	//overpressureResults.reserve(overpressureCalculation.size());
	for (int i = 0; i < overpressureCalculation.size(); ++i)
	{
		double res = calculate(overpressureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		res = res * 0.06;
		if (!m_steelArray.contains(i + 1))
		{
			if (res == 0)
			{
				res = translateAngle(fallInfo.angle, res);
				propellantOverpressureResults.push_back(res);
			}
			else
			{
				
				res = translateFallOverpressure(fallInfo.high, res);
				res = translateAngle(fallInfo.angle, res);
				propellantOverpressureResults.push_back(res);
			}

			// 反应度
			if (propellantInfo.fireOverpressure == 0.0)
			{
				propellantReactionDegreeResults.push_back(0.0);
			}
			else
			{
				auto reactionDegree = res / propellantInfo.fireOverpressure;
				if (reactionDegree > 1)
				{
					propellantReactionDegreeResults.push_back(1);
				}
				else
				{
					propellantReactionDegreeResults.push_back(reactionDegree);

				}
				

			}
			
		}
		else
		{
			//res = translateFallOverpressure(fallInfo.high, res) * 1.42;
			res = translateAngle(fallInfo.angle, res);
			steelOverpressureResults.push_back(res);
		}
		
	}
	
	double calSteelOverpressureMinValue = *std::min_element(steelOverpressureResults.begin(), steelOverpressureResults.end());
	double calSteelOverpressureMaxValue = *std::max_element(steelOverpressureResults.begin(), steelOverpressureResults.end());

	double calPropellantOverpressureMinValue = *std::min_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());
	double calPropellantOverpressureMaxValue = *std::max_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());

	//// 更新结果
	double shellOverpressureMaxValue = calSteelOverpressureMaxValue; // 发动机壳体最大超压
	double shellOverpressureMinValue = calSteelOverpressureMinValue; // 发动机壳体最小超压
	double shellOverpressureAvgValue = calculateAvg(steelOverpressureResults); // 发动机壳体平均超压
	double shellOverpressureStandardValue = calculateStd(steelOverpressureResults); // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = calPropellantOverpressureMaxValue; // 固体推进剂最大超压
	double propellantOverpressureMinValue = calPropellantOverpressureMinValue; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = calculateAvg(propellantOverpressureResults); // 固体推进剂平均超压
	double propellantOverpressureStandardValue = calculateStd(propellantOverpressureResults); // 固体推进剂超压标准差

	//double shellOverpressureMaxValue = shellStressMaxValue * 1.47; // 发动机壳体最大超压
	//double shellOverpressureMinValue = shellStressMinValue * 1.47; // 发动机壳体最小超压
	//double shellOverpressureAvgValue = shellStressAvgValue * 1.47; // 发动机壳体平均超压
	//double shellOverpressureStandardValue = shellStressStandardValue * 1.47; // 发动机壳体超压标准差
	//double propellantOverpressureMaxValue = propellantStressMaxValue * 1.47; // 固体推进剂最大超压
	//double propellantOverpressureMinValue = propellantStressMinValue * 1.47; // 固体推进剂最小超压
	//double propellantOverpressureAvgValue = propellantStressAvgValue * 1.47; // 固体推进剂平均超压
	//double propellantOverpressureStandardValue = propellantStressStandardValue * 1.47; // 固体推进剂超压标准差

	


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
	overpressureResult.insulatingheatMaxOverpressure = propellantOverpressureMaxValue * 1.01;
	overpressureResult.insulatingheatMinOverpressure = propellantOverpressureMinValue * 1.01;
	overpressureResult.insulatingheatAvgOverpressure = propellantOverpressureAvgValue * 1.01;
	overpressureResult.insulatingheatStandardOverpressure = propellantOverpressureStandardValue * 1.01;
	overpressureResult.shellScreenFlag = true;
	overpressureResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetFallOverpressureResult(overpressureResult);


	double calPropellantReactionDegreeMinValue = *std::min_element(propellantReactionDegreeResults.begin(), propellantReactionDegreeResults.end());
	double calPropellantReactionDegreeMaxValue = *std::max_element(propellantReactionDegreeResults.begin(), propellantReactionDegreeResults.end());

	// 反应度分析结果（超压/推进剂发火超压）
	ReactionDegreeResult reactionDegreeResult;
	reactionDegreeResult.metalsMaxReactionDegree = 0;
	reactionDegreeResult.metalsMinReactionDegree = 0;
	reactionDegreeResult.metalsAvgReactionDegree = 0;
	reactionDegreeResult.metalsStandardReactionDegree = 0;
	auto propellantsMaxReactionDegree = propellantOverpressureMaxValue / propellantInfo.fireOverpressure;
	auto propellantsMinReactionDegree = propellantOverpressureMinValue / propellantInfo.fireOverpressure;
	auto propellantsAvgReactionDegree = propellantOverpressureAvgValue / propellantInfo.fireOverpressure;
	auto propellantsStandardReactionDegree = propellantOverpressureStandardValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsMaxReactionDegree = propellantsMaxReactionDegree > 1 ? 1 : propellantsMaxReactionDegree;
	reactionDegreeResult.propellantsMinReactionDegree = propellantsMinReactionDegree > 1 ? 1 : propellantsMinReactionDegree;
	reactionDegreeResult.propellantsAvgReactionDegree = propellantsAvgReactionDegree > 1 ? 1 : propellantsAvgReactionDegree;
	reactionDegreeResult.propellantsStandardReactionDegree = propellantsStandardReactionDegree > 1 ? 1 : propellantsStandardReactionDegree;
	reactionDegreeResult.outheatMaxReactionDegree = 0;
	reactionDegreeResult.outheatMinReactionDegree = 0;
	reactionDegreeResult.outheatAvgReactionDegree = 0;
	reactionDegreeResult.outheatStandardReactionDegree = 0;
	reactionDegreeResult.insulatingheatMaxReactionDegree = 0;
	reactionDegreeResult.insulatingheatMinReactionDegree = 0;
	reactionDegreeResult.insulatingheatAvgReactionDegree = 0;
	reactionDegreeResult.insulatingheatStandardReactionDegree = 0;
	reactionDegreeResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetFallReactionDegreeResult(reactionDegreeResult);




	FallAnalysisResultInfo fallAnalysisResultInfo;

	fallAnalysisResultInfo.isChecked = true;
	fallAnalysisResultInfo.stressMaxValue = qMax(qMax(fallStressResult.metalsMaxStress, fallStressResult.propellantsMaxStress), qMax(fallStressResult.outheatMaxStress, fallStressResult.insulatingheatMaxStress));
	fallAnalysisResultInfo.stressMinValue = qMin(qMin(fallStressResult.metalsMinStress, fallStressResult.propellantsMinStress), qMin(fallStressResult.outheatMinStress, fallStressResult.insulatingheatMinStress));
	fallAnalysisResultInfo.strainMaxValue = qMax(qMax(fallStrainResult.metalsMaxStrain, fallStrainResult.propellantsMaxStrain), qMax(fallStrainResult.outheatMaxStrain, fallStrainResult.insulatingheatMaxStrain));
	fallAnalysisResultInfo.strainMinValue = qMin(qMin(fallStrainResult.metalsMinStrain, fallStrainResult.propellantsMinStrain), qMin(fallStrainResult.outheatMinStrain, fallStrainResult.insulatingheatMinStrain));
	fallAnalysisResultInfo.temperatureMaxValue = qMax(qMax(temperatureResult.metalsMaxTemperature, temperatureResult.propellantsMaxTemperature), qMax(temperatureResult.outheatMaxTemperature, temperatureResult.insulatingheatMaxTemperature));
	fallAnalysisResultInfo.temperatureMinValue = qMin(qMin(temperatureResult.metalsMinTemperature, temperatureResult.propellantsMinTemperature), qMin(temperatureResult.outheatMinTemperature, temperatureResult.insulatingheatMinTemperature));
	fallAnalysisResultInfo.overpressureMaxValue = qMax(qMax(overpressureResult.metalsMaxOverpressure, overpressureResult.propellantsMaxOverpressure), qMax(overpressureResult.outheatMaxOverpressure, overpressureResult.insulatingheatMaxOverpressure));
	fallAnalysisResultInfo.overpressureMinValue = qMin(qMin(overpressureResult.metalsMinOverpressure, overpressureResult.propellantsMinOverpressure), qMin(overpressureResult.outheatMinOverpressure, overpressureResult.insulatingheatMinOverpressure));
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
	auto insulatingheatPropertyInfo = ModelDataManager::GetInstance()->GetInsulatingheatPropertyInfo();
	auto calInfo = ModelDataManager::GetInstance()->GetCalculationPropertyInfo();
	auto fastCombustionSettingInfo = ModelDataManager::GetInstance()->GetFastCombustionSettingInfo();
	auto modelGeomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();

	auto A = 1;
	auto B = steelInfo.density;
	auto C = steelInfo.modulus / 1000;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width / 2;//半径
	auto L = modelGeomInfo.thickness;//厚
	auto M = fastCombustionSettingInfo.temperature;//环境温度

	auto formulaCal = calInfo.fastCombustionCalculation;

	
	// 绝热层导热系数
	auto thermalConductivity = insulatingheatPropertyInfo.thermalConductivity;
	double difference = thermalConductivity - 1;

	//std::vector<double> results;
	//results.reserve(formulaCal.size());
	std::vector<double> steelTemperatureResults;
	std::vector<double> propellantTemperatureResults;
	for (int i = 0; i < formulaCal.size(); ++i)
	{
		double res = calculate(formulaCal[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		res = res * 0.068;

		if (res > 25)
		{
			res = res + res * 0.1 * difference;
			if (res > fastCombustionSettingInfo.temperature)
			{
				res = fastCombustionSettingInfo.temperature * 0.92;
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
	}

		
	double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
	steelTemperatureResults.push_back(calSteelTemperatureMaxValue * 0.4);
	double calSteelTemperatureMinValue = *std::min_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
	if (calSteelTemperatureMaxValue > fastCombustionSettingInfo.temperature)
	{
		double max = fastCombustionSettingInfo.temperature * 0.82;
		double coefficient = max / calSteelTemperatureMaxValue; //系数


		calSteelTemperatureMinValue = *std::min_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
		calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
	}

	propellantTemperatureResults.push_back(calSteelTemperatureMaxValue * 0.85);
	double randomValue = QRandomGenerator::global()->bounded(1000) / 1000.0;
	randomValue = 0.1 + randomValue * (0.2 - 0.1);
	propellantTemperatureResults.push_back(35 * (1 + randomValue));

	propellantTemperatureResults.erase(
		std::remove_if(propellantTemperatureResults.begin(), propellantTemperatureResults.end(),
			[calSteelTemperatureMaxValue](double value) { return value >= calSteelTemperatureMaxValue; }),
		propellantTemperatureResults.end());

	double calPropellantTemperatureMinValue = *std::min_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());
	double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());

	
	

	// 更新结果
	double outheatMaxValue = calSteelTemperatureMaxValue * 1.01; // 发动机外放热最大温度
	double outheatMinValue = calSteelTemperatureMaxValue; // 发动机外放热最小温度
	double outheatAvgValue = calculateAvgTemp(outheatMaxValue, outheatMinValue); // 发动机外放热平均温度
	double outheatStandardValue = calculateStdTemp(outheatMaxValue, outheatMinValue); // 发动机外放热温度标准差

	double shellMaxValue = calSteelTemperatureMaxValue; // 发动机壳体最大温度
	double shellMinValue = calPropellantTemperatureMaxValue * 1.01; // 发动机壳体最小温度
	double shellAvgValue = calculateAvgTemp(shellMaxValue, shellMinValue); // 发动机壳体平均温度
	double shellStandardValue = calculateStdTemp(shellMaxValue, shellMinValue); // 发动机壳体温度标准差

	double insulatingheatMaxValue = shellMinValue; // 发动机绝热层最大温度
	double insulatingheatMinValue = calPropellantTemperatureMaxValue; // 发动机绝热层最小温度
	double insulatingheatAvgValue = calculateAvgTemp(insulatingheatMaxValue, insulatingheatMinValue); // 发动机绝热层平均温度
	double insulatingheatStandardValue = calculateStdTemp(insulatingheatMaxValue, insulatingheatMinValue); // 发动机绝热层温度标准差

	double maxValue = calPropellantTemperatureMaxValue; // 固体推进剂最大温度
	double minValue = calPropellantTemperatureMinValue; // 固体推进剂最小温度
	double avgValue = calculateAvgTemp(maxValue, minValue); // 固体推进剂平均温度
	double standardValue = calculateStdTemp(maxValue, minValue); // 固体推进剂温度标准差


	propertyValue.clear();
	propertyValue.push_back(shellMaxValue);
	propertyValue.push_back(shellMinValue);
	propertyValue.push_back(shellAvgValue);
	propertyValue.push_back(shellStandardValue);
	propertyValue.push_back(maxValue);
	propertyValue.push_back(minValue);
	propertyValue.push_back(avgValue);
	propertyValue.push_back(standardValue);


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
	temperatureResult.outheatMaxTemperature = outheatMaxValue;
	temperatureResult.outheatMinTemperature = outheatMinValue;
	temperatureResult.outheatAvgTemperature = outheatAvgValue;
	temperatureResult.outheatStandardTemperature = outheatStandardValue;
	temperatureResult.insulatingheatMaxTemperature = insulatingheatMaxValue;
	temperatureResult.insulatingheatMinTemperature = insulatingheatMinValue;
	temperatureResult.insulatingheatAvgTemperature = insulatingheatAvgValue;
	temperatureResult.insulatingheatStandardTemperature = insulatingheatStandardValue;
	temperatureResult.shellScreenFlag = true;
	temperatureResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetFastCombustionTemperatureResult(temperatureResult);

	FastCombustionAnalysisResultInfo fastCombustionAnalysisResultInfo;
	fastCombustionAnalysisResultInfo.isChecked = true;
	fastCombustionAnalysisResultInfo.temperatureMaxValue = qMax(qMax(temperatureResult.metalsMaxTemperature, temperatureResult.propellantsMaxTemperature), qMax(temperatureResult.outheatMaxTemperature, temperatureResult.insulatingheatMaxTemperature));
	fastCombustionAnalysisResultInfo.temperatureMinValue = qMin(qMin(temperatureResult.metalsMinTemperature, temperatureResult.propellantsMinTemperature), qMin(temperatureResult.outheatMinTemperature, temperatureResult.insulatingheatMinTemperature));

	ModelDataManager::GetInstance()->SetFastCombustionAnalysisResultInfo(fastCombustionAnalysisResultInfo);

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
	auto insulatingheatPropertyInfo = ModelDataManager::GetInstance()->GetInsulatingheatPropertyInfo();
	auto calInfo = ModelDataManager::GetInstance()->GetCalculationPropertyInfo();
	auto slowCombustionSettingInfo = ModelDataManager::GetInstance()->GetSlowCombustionSettingInfo();
	auto modelGeomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();

	auto A = 1;
	auto B = steelInfo.density;
	auto C = steelInfo.modulus / 1000;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width / 2;//半径
	auto L = modelGeomInfo.thickness;//厚
	auto M = slowCombustionSettingInfo.temperature;//温度幅度

	auto formulaCal = calInfo.slowCombustionCalculation;

	// 绝热层导热系数
	auto thermalConductivity = insulatingheatPropertyInfo.thermalConductivity;
	double difference = thermalConductivity - 1;

	//std::vector<double> results;
	//results.reserve(formulaCal.size());
	std::vector<double> steelTemperatureResults;
	std::vector<double> propellantTemperatureResults;
	for (int i = 0; i < formulaCal.size(); ++i)
	{
		double res = calculate(formulaCal[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		res = res + res * 0.1 * difference;
		if (res > slowCombustionSettingInfo.temperature)
		{
			res = slowCombustionSettingInfo.temperature;
		}
		if (!m_steelArray.contains(i + 1))
		{
			if (res < (slowCombustionSettingInfo.temperature * 0.94))
			{
				res = slowCombustionSettingInfo.temperature * 0.94;
			}
			propellantTemperatureResults.push_back(res);
		}
		else
		{
			if (res < (slowCombustionSettingInfo.temperature * 0.95))
			{
				res = slowCombustionSettingInfo.temperature * 0.95;
			}
			steelTemperatureResults.push_back(res);
		}
	}

	steelTemperatureResults.push_back(slowCombustionSettingInfo.temperature);
	propellantTemperatureResults.push_back(slowCombustionSettingInfo.temperature * 0.87);

	double calSteelTemperatureMinValue = *std::min_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
	double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());

	propellantTemperatureResults.erase(
		std::remove_if(propellantTemperatureResults.begin(), propellantTemperatureResults.end(),
			[calSteelTemperatureMaxValue](double value) { return value > calSteelTemperatureMaxValue; }),
		propellantTemperatureResults.end());

	double calPropellantTemperatureMinValue = *std::min_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());
	double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());


	// 更新结果
	double outheatMaxValue = calSteelTemperatureMaxValue; // 发动机外放热最大温度
	double outheatMinValue = calSteelTemperatureMaxValue; // 发动机外放热最小温度
	double outheatAvgValue = calculateAvgTemp(outheatMaxValue, outheatMinValue); // 发动机外放热平均温度
	double outheatStandardValue = calculateStdTemp(outheatMaxValue, outheatMinValue); // 发动机外放热温度标准差

	double shellMaxValue = calSteelTemperatureMaxValue; // 发动机壳体最大温度
	double shellMinValue = calPropellantTemperatureMaxValue * 1.01; // 发动机壳体最小温度
	double shellAvgValue = calculateAvgTemp(shellMaxValue, shellMinValue); // 发动机壳体平均温度
	double shellStandardValue = calculateStdTemp(shellMaxValue, shellMinValue); // 发动机壳体温度标准差

	double insulatingheatMaxValue = shellMinValue; // 发动机绝热层最大温度
	double insulatingheatMinValue = calPropellantTemperatureMaxValue; // 发动机绝热层最小温度
	double insulatingheatAvgValue = calculateAvgTemp(insulatingheatMaxValue, insulatingheatMinValue); // 发动机绝热层平均温度
	double insulatingheatStandardValue = calculateStdTemp(insulatingheatMaxValue, insulatingheatMinValue); // 发动机绝热层温度标准差

	double maxValue = calPropellantTemperatureMaxValue; // 固体推进剂最大温度
	double minValue = calPropellantTemperatureMinValue; // 固体推进剂最小温度
	double avgValue = calculateAvgTemp(maxValue, minValue); // 固体推进剂平均温度
	double standardValue = calculateStdTemp(maxValue, minValue); // 固体推进剂温度标准差

	propertyValue.clear();
	propertyValue.push_back(shellMaxValue);
	propertyValue.push_back(shellMinValue);
	propertyValue.push_back(shellAvgValue);
	propertyValue.push_back(shellStandardValue);
	propertyValue.push_back(maxValue);
	propertyValue.push_back(minValue);
	propertyValue.push_back(avgValue);
	propertyValue.push_back(standardValue);

	

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
	temperatureResult.outheatMaxTemperature = outheatMaxValue;
	temperatureResult.outheatMinTemperature = outheatMinValue;
	temperatureResult.outheatAvgTemperature = outheatAvgValue;
	temperatureResult.outheatStandardTemperature = outheatStandardValue;
	temperatureResult.insulatingheatMaxTemperature = insulatingheatMaxValue;
	temperatureResult.insulatingheatMinTemperature = insulatingheatMinValue;
	temperatureResult.insulatingheatAvgTemperature = insulatingheatAvgValue;
	temperatureResult.insulatingheatStandardTemperature = insulatingheatStandardValue;
	temperatureResult.shellScreenFlag = true;
	temperatureResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetSlowCombustionTemperatureResult(temperatureResult);

	SlowCombustionAnalysisResultInfo slowCombustionAnalysisResultInfo;
	slowCombustionAnalysisResultInfo.isChecked = true;
	slowCombustionAnalysisResultInfo.temperatureMaxValue = qMax(qMax(temperatureResult.metalsMaxTemperature, temperatureResult.propellantsMaxTemperature), qMax(temperatureResult.outheatMaxTemperature, temperatureResult.insulatingheatMaxTemperature));
	slowCombustionAnalysisResultInfo.temperatureMinValue = qMin(qMin(temperatureResult.metalsMinTemperature, temperatureResult.propellantsMinTemperature), qMin(temperatureResult.outheatMinTemperature, temperatureResult.insulatingheatMinTemperature));
	ModelDataManager::GetInstance()->SetSlowCombustionAnalysisResultInfo(slowCombustionAnalysisResultInfo);

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
	auto C = steelInfo.modulus / 1000;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = shootInfo.radius;//子弹直径
	auto L = modelGeomInfo.thickness;//厚度
	auto M = shootInfo.speed * 1000;//撞击速度

	auto limitValue = steelInfo.tensileStrength * 1.5; // 抗拉强度
	auto tangentModulus = steelInfo.tangentModulus; // 切线模量
	auto modulus = steelInfo.modulus;// 弹性模量
	double difference = (tangentModulus / modulus);
	difference = (difference - 0.15) / 0.15;

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
		res = res + res * difference * 0.1;
		if (res > limitValue)
		{
			res = limitValue;
		}
		if (!m_steelArray.contains(i + 1))
		{
			res = translate(res) * 2.32;
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
	stressResult.outheatMaxStress = shellStressMaxValue;
	stressResult.outheatMinStress = shellStressMinValue;
	stressResult.outheatAvgStress = shellStressAvgValue;
	stressResult.outheatStandardStress = shellStressStandardValue;
	stressResult.insulatingheatMaxStress = propellantStressMaxValue * 1.01;
	stressResult.insulatingheatMinStress = propellantStressMinValue * 1.01;
	stressResult.insulatingheatAvgStress = propellantStressAvgValue * 1.01;
	stressResult.insulatingheatStandardStress = propellantStressStandardValue * 1.01;
	stressResult.shellScreenFlag = true;
	stressResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetShootStressResult(stressResult);

	// 应变分析结果
	StrainResult strainResult;
	strainResult.metalsMaxStrain = stressResult.metalsMaxStress / steelInfo.modulus ;
	strainResult.metalsMinStrain = stressResult.metalsMinStress / steelInfo.modulus ;
	strainResult.metalsAvgStrain = stressResult.metalsAvgStress / steelInfo.modulus ;
	strainResult.metalsStandardStrain = stressResult.metalsStandardStress / steelInfo.modulus ;
	strainResult.propellantsMaxStrain = stressResult.propellantsMaxStress / propellantInfo.modulus;
	strainResult.propellantsMinStrain = stressResult.propellantsMinStress / propellantInfo.modulus;
	strainResult.mpropellantsAvgStrain = stressResult.propellantsAvgStress / propellantInfo.modulus;
	strainResult.propellantsStandardStrain = stressResult.propellantsStandardStress / propellantInfo.modulus;
	strainResult.outheatMaxStrain = stressResult.outheatMaxStress / steelInfo.modulus ;
	strainResult.outheatMinStrain = stressResult.outheatMinStress / steelInfo.modulus ;
	strainResult.outheatAvgStrain = stressResult.outheatAvgStress / steelInfo.modulus ;
	strainResult.outheatStandardStrain = stressResult.outheatStandardStress / steelInfo.modulus ;
	strainResult.insulatingheatMaxStrain = stressResult.insulatingheatMaxStress / steelInfo.modulus ;
	strainResult.insulatingheatMinStrain = stressResult.insulatingheatMinStress / steelInfo.modulus ;
	strainResult.insulatingheatAvgStrain = stressResult.insulatingheatAvgStress / steelInfo.modulus ;
	strainResult.insulatingheatStandardStrain = stressResult.insulatingheatStandardStress / steelInfo.modulus ;
	strainResult.shellScreenFlag = true;
	strainResult.propellantScreenFlag = true;
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
		if (res < 25)
		{
			res = 25;
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
	propellantTemperatureResults.push_back(25);
	steelTemperatureResults.push_back(25);
	double calSteelTemperatureMinValue = *std::min_element(steelTemperatureResults.begin(), steelTemperatureResults.end());
	double calSteelTemperatureMaxValue = *std::max_element(steelTemperatureResults.begin(), steelTemperatureResults.end());

	double calPropellantTemperatureMinValue = *std::min_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());
	double calPropellantTemperatureMaxValue = *std::max_element(propellantTemperatureResults.begin(), propellantTemperatureResults.end());

	// 更新结果
	double shellTemperatureMaxValue = calSteelTemperatureMaxValue; // 发动机壳体最大温度
	double shellTemperatureMinValue = 25; // 发动机壳体最小温度
	double shellTemperatureAvgValue = calculateAvg(steelTemperatureResults); // 发动机壳体平均温度
	double shellTemperatureStandardValue = calculateStd(steelTemperatureResults); // 发动机壳体温度标准差
	double propellantTemperatureMaxValue = calPropellantTemperatureMaxValue; // 固体推进剂最大温度
	double propellantTemperatureMinValue = 25; // 固体推进剂最小温度
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
	temperatureResult.outheatMaxTemperature = shellTemperatureMaxValue * 1.01;
	temperatureResult.outheatMinTemperature = 25;
	temperatureResult.outheatAvgTemperature = shellTemperatureAvgValue * 1.01;
	temperatureResult.outheatStandardTemperature = shellTemperatureStandardValue * 1.01;
	temperatureResult.insulatingheatMaxTemperature = propellantTemperatureMaxValue * 1.01;
	temperatureResult.insulatingheatMinTemperature = 25;
	temperatureResult.insulatingheatAvgTemperature = propellantTemperatureAvgValue * 1.01;
	temperatureResult.insulatingheatStandardTemperature = propellantTemperatureStandardValue * 1.01;
	temperatureResult.shellScreenFlag = true;
	temperatureResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetShootTemperatureResult(temperatureResult);



	// 超压
	auto overpressureCalculation = calInfo.shootOverpressureCalculation;
	std::vector<double> steelOverpressureResults;
	std::vector<double> propellantOverpressureResults;
	// 反应度
	std::vector<double> propellantReactionDegreeResults;
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
			res = res * 3.13;
			propellantOverpressureResults.push_back(res);
			// 反应度
			if (propellantInfo.fireOverpressure == 0.0)
			{
				propellantReactionDegreeResults.push_back(0.0);
			}
			else
			{
				auto reactionDegree = res / propellantInfo.fireOverpressure;
				if (reactionDegree > 1)
				{
					propellantReactionDegreeResults.push_back(1);
				}
				else
				{
					propellantReactionDegreeResults.push_back(reactionDegree);
				}
			}
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
	//double shellOverpressureMaxValue = calSteelOverpressureMaxValue; // 发动机壳体最大超压
	//double shellOverpressureMinValue = calSteelOverpressureMinValue; // 发动机壳体最小超压
	//double shellOverpressureAvgValue = calculateAvg(steelOverpressureResults); // 发动机壳体平均超压
	//double shellOverpressureStandardValue = calculateStd(steelOverpressureResults); // 发动机壳体超压标准差
	//double propellantOverpressureMaxValue = calPropellantOverpressureMaxValue; // 固体推进剂最大超压
	//double propellantOverpressureMinValue = calPropellantOverpressureMinValue; // 固体推进剂最小超压
	//double propellantOverpressureAvgValue = calculateAvg(propellantOverpressureResults); // 固体推进剂平均超压
	//double propellantOverpressureStandardValue = calculateStd(propellantOverpressureResults); // 固体推进剂超压标准差

	double shellOverpressureMaxValue = shellStressMaxValue * 1.47; // 发动机壳体最大超压
	double shellOverpressureMinValue = shellStressMinValue * 1.47; // 发动机壳体最小超压
	double shellOverpressureAvgValue = shellStressAvgValue * 1.47; // 发动机壳体平均超压
	double shellOverpressureStandardValue = shellStressStandardValue * 1.47; // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = propellantStressMaxValue * 2.47; // 固体推进剂最大超压
	double propellantOverpressureMinValue = propellantStressMinValue * 2.47; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = propellantStressAvgValue * 2.47; // 固体推进剂平均超压
	double propellantOverpressureStandardValue = propellantStressStandardValue * 2.47; // 固体推进剂超压标准差



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
	overpressureResult.insulatingheatMaxOverpressure = propellantOverpressureMaxValue * 1.01;
	overpressureResult.insulatingheatMinOverpressure = propellantOverpressureMinValue * 1.01;
	overpressureResult.insulatingheatAvgOverpressure = propellantOverpressureAvgValue * 1.01;
	overpressureResult.insulatingheatStandardOverpressure = propellantOverpressureStandardValue * 1.01;
	overpressureResult.shellScreenFlag = true;
	overpressureResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetShootOverpressureResult(overpressureResult);

	double calPropellantReactionDegreeMinValue = *std::min_element(propellantReactionDegreeResults.begin(), propellantReactionDegreeResults.end());
	double calPropellantReactionDegreeMaxValue = *std::max_element(propellantReactionDegreeResults.begin(), propellantReactionDegreeResults.end());

	// 反应度分析结果（超压/推进剂发火超压）
	ReactionDegreeResult reactionDegreeResult;
	reactionDegreeResult.metalsMaxReactionDegree = 0;
	reactionDegreeResult.metalsMinReactionDegree = 0;
	reactionDegreeResult.metalsAvgReactionDegree = 0;
	reactionDegreeResult.metalsStandardReactionDegree = 0;
	reactionDegreeResult.propellantsMaxReactionDegree = propellantOverpressureMaxValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsMinReactionDegree = propellantOverpressureMinValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsAvgReactionDegree = propellantOverpressureAvgValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsStandardReactionDegree = propellantOverpressureStandardValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.outheatMaxReactionDegree = 0;
	reactionDegreeResult.outheatMinReactionDegree = 0;
	reactionDegreeResult.outheatAvgReactionDegree = 0;
	reactionDegreeResult.outheatStandardReactionDegree = 0;
	reactionDegreeResult.insulatingheatMaxReactionDegree = 0;
	reactionDegreeResult.insulatingheatMinReactionDegree = 0;
	reactionDegreeResult.insulatingheatAvgReactionDegree = 0;
	reactionDegreeResult.insulatingheatStandardReactionDegree = 0;
	reactionDegreeResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetShootReactionDegreeResult(reactionDegreeResult);


	ShootAnalysisResultInfo shootAnalysisResultInfo;
	shootAnalysisResultInfo.isChecked = true;
	shootAnalysisResultInfo.stressMaxValue = qMax(qMax(stressResult.metalsMaxStress, stressResult.propellantsMaxStress), qMax(stressResult.outheatMaxStress, stressResult.insulatingheatMaxStress));
	shootAnalysisResultInfo.stressMinValue = qMin(qMin(stressResult.metalsMinStress, stressResult.propellantsMinStress), qMin(stressResult.outheatMinStress, stressResult.insulatingheatMinStress));
	shootAnalysisResultInfo.strainMaxValue = qMax(qMax(strainResult.metalsMaxStrain, strainResult.propellantsMaxStrain), qMax(strainResult.outheatMaxStrain, strainResult.insulatingheatMaxStrain));
	shootAnalysisResultInfo.strainMinValue = qMin(qMin(strainResult.metalsMinStrain, strainResult.propellantsMinStrain), qMin(strainResult.outheatMinStrain, strainResult.insulatingheatMinStrain));
	shootAnalysisResultInfo.temperatureMaxValue = qMax(qMax(temperatureResult.metalsMaxTemperature, temperatureResult.propellantsMaxTemperature), qMax(temperatureResult.outheatMaxTemperature, temperatureResult.insulatingheatMaxTemperature));
	shootAnalysisResultInfo.temperatureMinValue = qMin(qMin(temperatureResult.metalsMinTemperature, temperatureResult.propellantsMinTemperature), qMin(temperatureResult.outheatMinTemperature, temperatureResult.insulatingheatMinTemperature));
	shootAnalysisResultInfo.overpressureMaxValue = qMax(qMax(overpressureResult.metalsMaxOverpressure, overpressureResult.propellantsMaxOverpressure), qMax(overpressureResult.outheatMaxOverpressure, overpressureResult.insulatingheatMaxOverpressure));
	shootAnalysisResultInfo.overpressureMinValue = qMin(qMin(overpressureResult.metalsMinOverpressure, overpressureResult.propellantsMinOverpressure), qMin(overpressureResult.outheatMinOverpressure, overpressureResult.insulatingheatMinOverpressure));
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
	auto C = steelInfo.modulus / 1000;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width/2;//半径
	auto L = modelGeomInfo.thickness;//厚
	auto M = jetImpactingInfo.caliber;// 聚能装药口径

	auto limitValue = steelInfo.tensileStrength * 1.5; // 抗拉强度
	auto tangentModulus = steelInfo.tangentModulus; // 切线模量
	auto modulus = steelInfo.modulus;// 弹性模量
	double difference = (tangentModulus / modulus);
	difference = (difference - 0.15) / 0.15;
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
		res = res + res * difference * 0.1;
		if (res > limitValue)
		{
			res = limitValue;
		}
		if (!m_steelArray.contains(i + 1))
		{
			res = translate(res) * 2.17;
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
	stressResult.outheatMaxStress = shellStressMaxValue;
	stressResult.outheatMinStress = shellStressMinValue;
	stressResult.outheatAvgStress = shellStressAvgValue;
	stressResult.outheatStandardStress = shellStressStandardValue;
	stressResult.insulatingheatMaxStress = propellantStressMaxValue * 1.01;
	stressResult.insulatingheatMinStress = propellantStressMinValue * 1.01;
	stressResult.insulatingheatAvgStress = propellantStressAvgValue * 1.01;
	stressResult.insulatingheatStandardStress = propellantStressStandardValue * 1.01;
	stressResult.shellScreenFlag = true;
	stressResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetJetImpactStressResult(stressResult);

	// 应变分析结果
	StrainResult strainResult;
	strainResult.metalsMaxStrain = stressResult.metalsMaxStress / steelInfo.modulus ;
	strainResult.metalsMinStrain = stressResult.metalsMinStress / steelInfo.modulus ;
	strainResult.metalsAvgStrain = stressResult.metalsAvgStress / steelInfo.modulus ;
	strainResult.metalsStandardStrain = stressResult.metalsStandardStress / steelInfo.modulus ;
	strainResult.propellantsMaxStrain = stressResult.propellantsMaxStress / propellantInfo.modulus;
	strainResult.propellantsMinStrain = stressResult.propellantsMinStress / propellantInfo.modulus;
	strainResult.mpropellantsAvgStrain = stressResult.propellantsAvgStress / propellantInfo.modulus;
	strainResult.propellantsStandardStrain = stressResult.propellantsStandardStress / propellantInfo.modulus;
	strainResult.outheatMaxStrain = stressResult.outheatMaxStress / steelInfo.modulus ;
	strainResult.outheatMinStrain = stressResult.outheatMinStress / steelInfo.modulus ;
	strainResult.outheatAvgStrain = stressResult.outheatAvgStress / steelInfo.modulus ;
	strainResult.outheatStandardStrain = stressResult.outheatStandardStress / steelInfo.modulus ;
	strainResult.insulatingheatMaxStrain = stressResult.insulatingheatMaxStress / steelInfo.modulus ;
	strainResult.insulatingheatMinStrain = stressResult.insulatingheatMinStress / steelInfo.modulus ;
	strainResult.insulatingheatAvgStrain = stressResult.insulatingheatAvgStress / steelInfo.modulus ;
	strainResult.insulatingheatStandardStrain = stressResult.insulatingheatStandardStress / steelInfo.modulus ;
	strainResult.shellScreenFlag = true;
	strainResult.propellantScreenFlag = true;
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
		res = res + 10;
		if (res < 25)
		{
			res = 25;
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
	double shellTemperatureMinValue = 25; // 发动机壳体最小温度
	double shellTemperatureAvgValue = calculateAvg(steelTemperatureResults); // 发动机壳体平均温度
	double shellTemperatureStandardValue = calculateStd(steelTemperatureResults); // 发动机壳体温度标准差
	double propellantTemperatureMaxValue = calPropellantTemperatureMaxValue; // 固体推进剂最大温度
	double propellantTemperatureMinValue = 25; // 固体推进剂最小温度
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
	temperatureResult.outheatMaxTemperature = shellTemperatureMaxValue * 1.01;
	temperatureResult.outheatMinTemperature = 25;
	temperatureResult.outheatAvgTemperature = shellTemperatureAvgValue * 1.01;
	temperatureResult.outheatStandardTemperature = shellTemperatureStandardValue * 1.01;
	temperatureResult.insulatingheatMaxTemperature = propellantTemperatureMaxValue * 1.01;
	temperatureResult.insulatingheatMinTemperature = 25;
	temperatureResult.insulatingheatAvgTemperature = propellantTemperatureAvgValue * 1.01;
	temperatureResult.insulatingheatStandardTemperature = propellantTemperatureStandardValue * 1.01;
	temperatureResult.shellScreenFlag = true;
	temperatureResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetJetImpactTemperatureResult(temperatureResult);



	// 超压
	auto overpressureCalculation = calInfo.jetImpactOverpressureCalculation;
	std::vector<double> steelOverpressureResults;
	std::vector<double> propellantOverpressureResults;
	// 反应度
	std::vector<double> propellantReactionDegreeResults;
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
			res = res * 5.72;
			propellantOverpressureResults.push_back(res);
			// 反应度
			if (propellantInfo.fireOverpressure == 0.0)
			{
				propellantReactionDegreeResults.push_back(0.0);
			}
			else
			{
				auto reactionDegree = res / propellantInfo.fireOverpressure;
				if (reactionDegree > 1)
				{
					propellantReactionDegreeResults.push_back(1);
				}
				else
				{
					propellantReactionDegreeResults.push_back(reactionDegree);
				}
			}
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
	//double shellOverpressureMaxValue = calSteelOverpressureMaxValue; // 发动机壳体最大超压
	//double shellOverpressureMinValue = calSteelOverpressureMinValue; // 发动机壳体最小超压
	//double shellOverpressureAvgValue = calculateAvg(steelOverpressureResults); // 发动机壳体平均超压
	//double shellOverpressureStandardValue = calculateStd(steelOverpressureResults); // 发动机壳体超压标准差
	//double propellantOverpressureMaxValue = calPropellantOverpressureMaxValue; // 固体推进剂最大超压
	//double propellantOverpressureMinValue = calPropellantOverpressureMinValue; // 固体推进剂最小超压
	//double propellantOverpressureAvgValue = calculateAvg(propellantOverpressureResults); // 固体推进剂平均超压
	//double propellantOverpressureStandardValue = calculateStd(propellantOverpressureResults); // 固体推进剂超压标准差
	double shellOverpressureMaxValue = shellStressMaxValue * 1.47; // 发动机壳体最大超压
	double shellOverpressureMinValue = shellStressMinValue * 1.47; // 发动机壳体最小超压
	double shellOverpressureAvgValue = shellStressAvgValue * 1.47; // 发动机壳体平均超压
	double shellOverpressureStandardValue = shellStressStandardValue * 1.47; // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = propellantStressMaxValue * 3.61; // 固体推进剂最大超压
	double propellantOverpressureMinValue = propellantStressMinValue * 3.61; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = propellantStressAvgValue * 3.61; // 固体推进剂平均超压
	double propellantOverpressureStandardValue = propellantStressStandardValue * 3.61; // 固体推进剂超压标准差


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
	overpressureResult.insulatingheatMaxOverpressure = propellantOverpressureMaxValue * 1.01;
	overpressureResult.insulatingheatMinOverpressure = propellantOverpressureMinValue * 1.01;
	overpressureResult.insulatingheatAvgOverpressure = propellantOverpressureAvgValue * 1.01;
	overpressureResult.insulatingheatStandardOverpressure = propellantOverpressureStandardValue * 1.01;
	overpressureResult.shellScreenFlag = true;
	overpressureResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetJetImpactOverpressureResult(overpressureResult);


	double calPropellantReactionDegreeMinValue = *std::min_element(propellantReactionDegreeResults.begin(), propellantReactionDegreeResults.end());
	double calPropellantReactionDegreeMaxValue = *std::max_element(propellantReactionDegreeResults.begin(), propellantReactionDegreeResults.end());

	// 反应度分析结果（超压/推进剂发火超压）
	ReactionDegreeResult reactionDegreeResult;
	reactionDegreeResult.metalsMaxReactionDegree = 0;
	reactionDegreeResult.metalsMinReactionDegree = 0;
	reactionDegreeResult.metalsAvgReactionDegree = 0;
	reactionDegreeResult.metalsStandardReactionDegree = 0;
	reactionDegreeResult.propellantsMaxReactionDegree = propellantOverpressureMaxValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsMinReactionDegree = propellantOverpressureMinValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsAvgReactionDegree = propellantOverpressureAvgValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsStandardReactionDegree = propellantOverpressureStandardValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.outheatMaxReactionDegree = 0;
	reactionDegreeResult.outheatMinReactionDegree = 0;
	reactionDegreeResult.outheatAvgReactionDegree = 0;
	reactionDegreeResult.outheatStandardReactionDegree = 0;
	reactionDegreeResult.insulatingheatMaxReactionDegree = 0;
	reactionDegreeResult.insulatingheatMinReactionDegree = 0;
	reactionDegreeResult.insulatingheatAvgReactionDegree = 0;
	reactionDegreeResult.insulatingheatStandardReactionDegree = 0;
	reactionDegreeResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetJetImpactReactionDegreeResult(reactionDegreeResult);


	JetImpactAnalysisResultInfo jetImpactAnalysisResultInfo;
	jetImpactAnalysisResultInfo.isChecked = true;
	jetImpactAnalysisResultInfo.stressMaxValue = qMax(qMax(stressResult.metalsMaxStress, stressResult.propellantsMaxStress), qMax(stressResult.outheatMaxStress, stressResult.insulatingheatMaxStress));
	jetImpactAnalysisResultInfo.stressMinValue = qMin(qMin(stressResult.metalsMinStress, stressResult.propellantsMinStress), qMin(stressResult.outheatMinStress, stressResult.insulatingheatMinStress));
	jetImpactAnalysisResultInfo.strainMaxValue = qMax(qMax(strainResult.metalsMaxStrain, strainResult.propellantsMaxStrain), qMax(strainResult.outheatMaxStrain, strainResult.insulatingheatMaxStrain));
	jetImpactAnalysisResultInfo.strainMinValue = qMin(qMin(strainResult.metalsMinStrain, strainResult.propellantsMinStrain), qMin(strainResult.outheatMinStrain, strainResult.insulatingheatMinStrain));
	jetImpactAnalysisResultInfo.temperatureMaxValue = qMax(qMax(temperatureResult.metalsMaxTemperature, temperatureResult.propellantsMaxTemperature), qMax(temperatureResult.outheatMaxTemperature, temperatureResult.insulatingheatMaxTemperature));
	jetImpactAnalysisResultInfo.temperatureMinValue = qMin(qMin(temperatureResult.metalsMinTemperature, temperatureResult.propellantsMinTemperature), qMin(temperatureResult.outheatMinTemperature, temperatureResult.insulatingheatMinTemperature));
	jetImpactAnalysisResultInfo.overpressureMaxValue = qMax(qMax(overpressureResult.metalsMaxOverpressure, overpressureResult.propellantsMaxOverpressure), qMax(overpressureResult.outheatMaxOverpressure, overpressureResult.insulatingheatMaxOverpressure));
	jetImpactAnalysisResultInfo.overpressureMinValue = qMin(qMin(overpressureResult.metalsMinOverpressure, overpressureResult.propellantsMinOverpressure), qMin(overpressureResult.outheatMinOverpressure, overpressureResult.insulatingheatMinOverpressure));

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
	auto C = steelInfo.modulus / 1000;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = fragmentationSettingInfo.radius;//破片直径
	auto L = modelGeomInfo.thickness;//厚度
	auto M = fragmentationSettingInfo.speed * 1000;//撞击速度

	auto limitValue = steelInfo.tensileStrength * 1.5; // 抗拉强度
	auto tangentModulus = steelInfo.tangentModulus; // 切线模量
	auto modulus = steelInfo.modulus;// 弹性模量
	double difference = (tangentModulus / modulus);
	difference = (difference - 0.15) / 0.15;

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
		res = res + res * difference * 0.1;
		if (res > limitValue)
		{
			res = limitValue;
		}
		if (!m_steelArray.contains(i + 1))
		{
			res = translate(res);
			res = res * 2.57;
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
	stressResult.outheatMaxStress = shellStressMaxValue;
	stressResult.outheatMinStress = shellStressMinValue;
	stressResult.outheatAvgStress = shellStressAvgValue;
	stressResult.outheatStandardStress = shellStressStandardValue;
	stressResult.insulatingheatMaxStress = propellantStressMaxValue * 1.01;
	stressResult.insulatingheatMinStress = propellantStressMinValue * 1.01;
	stressResult.insulatingheatAvgStress = propellantStressAvgValue * 1.01;
	stressResult.insulatingheatStandardStress = propellantStressStandardValue * 1.01;
	stressResult.shellScreenFlag = true;
	stressResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetFragmentationImpactStressResult(stressResult);

	// 应变分析结果
	StrainResult strainResult;
	strainResult.metalsMaxStrain = stressResult.metalsMaxStress / steelInfo.modulus ;
	strainResult.metalsMinStrain = stressResult.metalsMinStress / steelInfo.modulus ;
	strainResult.metalsAvgStrain = stressResult.metalsAvgStress / steelInfo.modulus ;
	strainResult.metalsStandardStrain = stressResult.metalsStandardStress / steelInfo.modulus ;
	strainResult.propellantsMaxStrain = stressResult.propellantsMaxStress / propellantInfo.modulus;
	strainResult.propellantsMinStrain = stressResult.propellantsMinStress / propellantInfo.modulus;
	strainResult.mpropellantsAvgStrain = stressResult.propellantsAvgStress / propellantInfo.modulus;
	strainResult.propellantsStandardStrain = stressResult.propellantsStandardStress / propellantInfo.modulus;
	strainResult.outheatMaxStrain = stressResult.outheatMaxStress / steelInfo.modulus ;
	strainResult.outheatMinStrain = stressResult.outheatMinStress / steelInfo.modulus ;
	strainResult.outheatAvgStrain = stressResult.outheatAvgStress / steelInfo.modulus ;
	strainResult.outheatStandardStrain = stressResult.outheatStandardStress / steelInfo.modulus ;
	strainResult.insulatingheatMaxStrain = stressResult.insulatingheatMaxStress / steelInfo.modulus ;
	strainResult.insulatingheatMinStrain = stressResult.insulatingheatMinStress / steelInfo.modulus ;
	strainResult.insulatingheatAvgStrain = stressResult.insulatingheatAvgStress / steelInfo.modulus ;
	strainResult.insulatingheatStandardStrain = stressResult.insulatingheatStandardStress / steelInfo.modulus ;
	strainResult.shellScreenFlag = true;
	strainResult.propellantScreenFlag = true;
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
		res = res + 25;
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
	double shellTemperatureMinValue = 25; // 发动机壳体最小温度
	double shellTemperatureAvgValue = calculateAvg(steelTemperatureResults); // 发动机壳体平均温度
	double shellTemperatureStandardValue = calculateStd(steelTemperatureResults); // 发动机壳体温度标准差
	double propellantTemperatureMaxValue = calPropellantTemperatureMaxValue; // 固体推进剂最大温度
	double propellantTemperatureMinValue = 25; // 固体推进剂最小温度
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
	temperatureResult.outheatMaxTemperature = shellTemperatureMaxValue * 1.01;
	temperatureResult.outheatMinTemperature = 25;
	temperatureResult.outheatAvgTemperature = shellTemperatureAvgValue * 1.01;
	temperatureResult.outheatStandardTemperature = shellTemperatureStandardValue * 1.01;
	temperatureResult.insulatingheatMaxTemperature = propellantTemperatureMaxValue * 1.019;
	temperatureResult.insulatingheatMinTemperature = 25;
	temperatureResult.insulatingheatAvgTemperature = propellantTemperatureAvgValue * 1.01;
	temperatureResult.insulatingheatStandardTemperature = propellantTemperatureStandardValue * 1.01;
	temperatureResult.shellScreenFlag = true;
	temperatureResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetFragmentationImpactTemperatureResult(temperatureResult);



	// 超压
	auto overpressureCalculation = calInfo.fragmentationImpactOverpressureCalculationO;
	std::vector<double> steelOverpressureResults;
	std::vector<double> propellantOverpressureResults;
	// 反应度
	std::vector<double> propellantReactionDegreeResults;
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
			res = res * 92.2;
			propellantOverpressureResults.push_back(res);
			// 反应度
			if (propellantInfo.fireOverpressure == 0.0)
			{
				propellantReactionDegreeResults.push_back(0.0);
			}
			else
			{
				auto reactionDegree = res / propellantInfo.fireOverpressure;
				if (reactionDegree > 1)
				{
					propellantReactionDegreeResults.push_back(1);
				}
				else
				{
					propellantReactionDegreeResults.push_back(reactionDegree);
				}
			}
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
	//double shellOverpressureMaxValue = calSteelOverpressureMaxValue; // 发动机壳体最大超压
	//double shellOverpressureMinValue = calSteelOverpressureMinValue; // 发动机壳体最小超压
	//double shellOverpressureAvgValue = calculateAvg(steelOverpressureResults); // 发动机壳体平均超压
	//double shellOverpressureStandardValue = calculateStd(steelOverpressureResults); // 发动机壳体超压标准差
	//double propellantOverpressureMaxValue = calPropellantOverpressureMaxValue; // 固体推进剂最大超压
	//double propellantOverpressureMinValue = calPropellantOverpressureMinValue; // 固体推进剂最小超压
	//double propellantOverpressureAvgValue = calculateAvg(propellantOverpressureResults); // 固体推进剂平均超压
	//double propellantOverpressureStandardValue = calculateStd(propellantOverpressureResults); // 固体推进剂超压标准差


	double shellOverpressureMaxValue = shellStressMaxValue * 1.47; // 发动机壳体最大超压
	double shellOverpressureMinValue = shellStressMinValue * 1.47; // 发动机壳体最小超压
	double shellOverpressureAvgValue = shellStressAvgValue * 1.47; // 发动机壳体平均超压
	double shellOverpressureStandardValue = shellStressStandardValue * 1.47; // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = propellantStressMaxValue * 3.87; // 固体推进剂最大超压
	double propellantOverpressureMinValue = propellantStressMinValue * 3.87; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = propellantStressAvgValue * 3.87; // 固体推进剂平均超压
	double propellantOverpressureStandardValue = propellantStressStandardValue * 3.87; // 固体推进剂超压标准差

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
	overpressureResult.insulatingheatMaxOverpressure = propellantOverpressureMaxValue * 1.01;
	overpressureResult.insulatingheatMinOverpressure = propellantOverpressureMinValue * 1.01;
	overpressureResult.insulatingheatAvgOverpressure = propellantOverpressureAvgValue * 1.01;
	overpressureResult.insulatingheatStandardOverpressure = propellantOverpressureStandardValue * 1.01;
	overpressureResult.shellScreenFlag = true;
	overpressureResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetFragmentationImpactOverpressureResult(overpressureResult);

	double calPropellantReactionDegreeMinValue = *std::min_element(propellantReactionDegreeResults.begin(), propellantReactionDegreeResults.end());
	double calPropellantReactionDegreeMaxValue = *std::max_element(propellantReactionDegreeResults.begin(), propellantReactionDegreeResults.end());

	// 反应度分析结果（超压/推进剂发火超压）
	ReactionDegreeResult reactionDegreeResult;
	reactionDegreeResult.metalsMaxReactionDegree = 0;
	reactionDegreeResult.metalsMinReactionDegree = 0;
	reactionDegreeResult.metalsAvgReactionDegree = 0;
	reactionDegreeResult.metalsStandardReactionDegree = 0;
	reactionDegreeResult.propellantsMaxReactionDegree = propellantOverpressureMaxValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsMinReactionDegree = propellantOverpressureMinValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsAvgReactionDegree = propellantOverpressureAvgValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsStandardReactionDegree = propellantOverpressureStandardValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.outheatMaxReactionDegree = 0;
	reactionDegreeResult.outheatMinReactionDegree = 0;
	reactionDegreeResult.outheatAvgReactionDegree = 0;
	reactionDegreeResult.outheatStandardReactionDegree = 0;
	reactionDegreeResult.insulatingheatMaxReactionDegree = 0;
	reactionDegreeResult.insulatingheatMinReactionDegree = 0;
	reactionDegreeResult.insulatingheatAvgReactionDegree = 0;
	reactionDegreeResult.insulatingheatStandardReactionDegree = 0;
	reactionDegreeResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetFragmentationImpactReactionDegreeResult(reactionDegreeResult);


	FragmentationAnalysisResultInfo fragmentationAnalysisResultInfo;
	fragmentationAnalysisResultInfo.isChecked = true;
	fragmentationAnalysisResultInfo.stressMaxValue = qMax(qMax(stressResult.metalsMaxStress, stressResult.propellantsMaxStress), qMax(stressResult.outheatMaxStress, stressResult.insulatingheatMaxStress));
	fragmentationAnalysisResultInfo.stressMinValue = qMin(qMin(stressResult.metalsMinStress, stressResult.propellantsMinStress), qMin(stressResult.outheatMinStress, stressResult.insulatingheatMinStress));
	fragmentationAnalysisResultInfo.strainMaxValue = qMax(qMax(strainResult.metalsMaxStrain, strainResult.propellantsMaxStrain), qMax(strainResult.outheatMaxStrain, strainResult.insulatingheatMaxStrain));
	fragmentationAnalysisResultInfo.strainMinValue = qMin(qMin(strainResult.metalsMinStrain, strainResult.propellantsMinStrain), qMin(strainResult.outheatMinStrain, strainResult.insulatingheatMinStrain));
	fragmentationAnalysisResultInfo.temperatureMaxValue = qMax(qMax(temperatureResult.metalsMaxTemperature, temperatureResult.propellantsMaxTemperature), qMax(temperatureResult.outheatMaxTemperature, temperatureResult.insulatingheatMaxTemperature));
	fragmentationAnalysisResultInfo.temperatureMinValue = qMin(qMin(temperatureResult.metalsMinTemperature, temperatureResult.propellantsMinTemperature), qMin(temperatureResult.outheatMinTemperature, temperatureResult.insulatingheatMinTemperature));
	fragmentationAnalysisResultInfo.overpressureMaxValue = qMax(qMax(overpressureResult.metalsMaxOverpressure, overpressureResult.propellantsMaxOverpressure), qMax(overpressureResult.outheatMaxOverpressure, overpressureResult.insulatingheatMaxOverpressure));
	fragmentationAnalysisResultInfo.overpressureMinValue = qMin(qMin(overpressureResult.metalsMinOverpressure, overpressureResult.propellantsMinOverpressure), qMin(overpressureResult.outheatMinOverpressure, overpressureResult.insulatingheatMinOverpressure));
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
	auto C = steelInfo.modulus / 1000;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;

	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width / 2;//半径
	auto L = modelGeomInfo.thickness;//厚
	auto M = explosiveBlastSettingInfo.tnt / 2 ;// TNT当量

	auto limitValue = steelInfo.tensileStrength * 1.5; // 抗拉强度
	auto tangentModulus = steelInfo.tangentModulus; // 切线模量
	auto modulus = steelInfo.modulus;// 弹性模量
	double difference = (tangentModulus / modulus);
	difference = (difference - 0.15) / 0.15;

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
		res = res * 0.5;
		res = res + res * difference * 0.1;
		if (res > limitValue)
		{
			res = limitValue;
		}
		if (res < 4000)
		{
			if (!m_steelArray.contains(i + 1))
			{
				res = translate(res) * 6.12;
				propellantStressResults.push_back(res);
			}
			else
			{
				steelStressResults.push_back(res);
			}
		}
		else
		{
			propellantStressResults.push_back(0);
			steelStressResults.push_back(0);
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
	stressResult.outheatMaxStress = shellStressMaxValue;
	stressResult.outheatMinStress = shellStressMinValue;
	stressResult.outheatAvgStress = shellStressAvgValue;
	stressResult.outheatStandardStress = shellStressStandardValue;
	stressResult.insulatingheatMaxStress = propellantStressMaxValue * 1.01;
	stressResult.insulatingheatMinStress = propellantStressMinValue * 1.01;
	stressResult.insulatingheatAvgStress = propellantStressAvgValue * 1.01;
	stressResult.insulatingheatStandardStress = propellantStressStandardValue * 1.01;
	stressResult.shellScreenFlag = true;
	stressResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetExplosiveBlastStressResult(stressResult);

	// 应变分析结果
	StrainResult strainResult;
	strainResult.metalsMaxStrain = stressResult.metalsMaxStress / steelInfo.modulus ;
	strainResult.metalsMinStrain = stressResult.metalsMinStress / steelInfo.modulus ;
	strainResult.metalsAvgStrain = stressResult.metalsAvgStress / steelInfo.modulus ;
	strainResult.metalsStandardStrain = stressResult.metalsStandardStress / steelInfo.modulus ;
	strainResult.propellantsMaxStrain = stressResult.propellantsMaxStress / propellantInfo.modulus;
	strainResult.propellantsMinStrain = stressResult.propellantsMinStress / propellantInfo.modulus;
	strainResult.mpropellantsAvgStrain = stressResult.propellantsAvgStress / propellantInfo.modulus;
	strainResult.propellantsStandardStrain = stressResult.propellantsStandardStress / propellantInfo.modulus;
	strainResult.outheatMaxStrain = stressResult.outheatMaxStress / steelInfo.modulus ;
	strainResult.outheatMinStrain = stressResult.outheatMinStress / steelInfo.modulus ;
	strainResult.outheatAvgStrain = stressResult.outheatAvgStress / steelInfo.modulus ;
	strainResult.outheatStandardStrain = stressResult.outheatStandardStress / steelInfo.modulus ;
	strainResult.insulatingheatMaxStrain = stressResult.insulatingheatMaxStress / steelInfo.modulus ;
	strainResult.insulatingheatMinStrain = stressResult.insulatingheatMinStress / steelInfo.modulus ;
	strainResult.insulatingheatAvgStrain = stressResult.insulatingheatAvgStress / steelInfo.modulus ;
	strainResult.insulatingheatStandardStrain = stressResult.insulatingheatStandardStress / steelInfo.modulus ;
	strainResult.shellScreenFlag = true;
	strainResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetExplosiveBlastStrainResult(strainResult);

	// 温度分析结果
	TemperatureResult temperatureResult;
	temperatureResult.metalsAvgTemperature = temperatureResult.metalsMinTemperature + QRandomGenerator::global()->generateDouble() * (temperatureResult.metalsMaxTemperature - temperatureResult.metalsMinTemperature);
	temperatureResult.mpropellantsAvgTemperature = temperatureResult.propellantsMinTemperature + QRandomGenerator::global()->generateDouble() * (temperatureResult.propellantsMaxTemperature - temperatureResult.propellantsMinTemperature);
	
	temperatureResult.outheatMaxTemperature = temperatureResult.metalsMaxTemperature * 1.01;
	temperatureResult.outheatMinTemperature = 25;
	temperatureResult.outheatAvgTemperature = temperatureResult.metalsAvgTemperature * 1.01;
	temperatureResult.outheatStandardTemperature = temperatureResult.metalsStandardTemperature * 1.01;
	temperatureResult.insulatingheatMaxTemperature = temperatureResult.propellantsMaxTemperature * 1.01;
	temperatureResult.insulatingheatMinTemperature = 25;
	temperatureResult.insulatingheatAvgTemperature = temperatureResult.mpropellantsAvgTemperature * 1.01;
	temperatureResult.insulatingheatStandardTemperature = temperatureResult.outheatStandardTemperature * 1.01;
	temperatureResult.shellScreenFlag = true;
	temperatureResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetExplosiveBlastTemperatureResult(temperatureResult);



	// 超压
	auto overpressureCalculation = calInfo.explosiveBlastOverpressureCalculation;
	std::vector<double> steelOverpressureResults;
	std::vector<double> propellantOverpressureResults;
	// 反应度
	std::vector<double> propellantReactionDegreeResults;
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
			res = res * 5.34;
			propellantOverpressureResults.push_back(res);
			// 反应度
			if (propellantInfo.fireOverpressure == 0.0)
			{
				propellantReactionDegreeResults.push_back(0.0);
			}
			else
			{
				auto reactionDegree = res / propellantInfo.fireOverpressure;
				if (reactionDegree > 1)
				{
					propellantReactionDegreeResults.push_back(1);
				}
				else
				{
					propellantReactionDegreeResults.push_back(reactionDegree);

				}
			}
		}
		else
		{
			res = res * 1.2;
			steelOverpressureResults.push_back(res);
		}

	}

	double calSteelOverpressureMinValue = *std::min_element(steelOverpressureResults.begin(), steelOverpressureResults.end());
	double calSteelOverpressureMaxValue = *std::max_element(steelOverpressureResults.begin(), steelOverpressureResults.end());

	double calPropellantOverpressureMinValue = *std::min_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());
	double calPropellantOverpressureMaxValue = *std::max_element(propellantOverpressureResults.begin(), propellantOverpressureResults.end());

	// 更新结果
	//double shellOverpressureMaxValue = calSteelOverpressureMaxValue; // 发动机壳体最大超压
	//double shellOverpressureMinValue = 0; // 发动机壳体最小超压
	//double shellOverpressureAvgValue = calculateAvg(steelOverpressureResults); // 发动机壳体平均超压
	//double shellOverpressureStandardValue = calculateStd(steelOverpressureResults); // 发动机壳体超压标准差
	//double propellantOverpressureMaxValue = calPropellantOverpressureMaxValue; // 固体推进剂最大超压
	//double propellantOverpressureMinValue = 0; // 固体推进剂最小超压
	//double propellantOverpressureAvgValue = calculateAvg(propellantOverpressureResults); // 固体推进剂平均超压
	//double propellantOverpressureStandardValue = calculateStd(propellantOverpressureResults); // 固体推进剂超压标准差

	double shellOverpressureMaxValue = shellStressMaxValue * 1.97; // 发动机壳体最大超压
	double shellOverpressureMinValue = shellStressMinValue * 1.97; // 发动机壳体最小超压
	double shellOverpressureAvgValue = shellStressAvgValue * 1.97; // 发动机壳体平均超压
	double shellOverpressureStandardValue = shellStressStandardValue * 1.97; // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = propellantStressMaxValue * 5.37; // 固体推进剂最大超压
	double propellantOverpressureMinValue = propellantStressMinValue * 5.37; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = propellantStressAvgValue * 5.37; // 固体推进剂平均超压
	double propellantOverpressureStandardValue = propellantStressStandardValue * 5.37; // 固体推进剂超压标准差

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
	overpressureResult.insulatingheatMaxOverpressure = propellantOverpressureMaxValue * 1.01;
	overpressureResult.insulatingheatMinOverpressure = propellantOverpressureMinValue * 1.01;
	overpressureResult.insulatingheatAvgOverpressure = propellantOverpressureAvgValue * 1.01;
	overpressureResult.insulatingheatStandardOverpressure = propellantOverpressureStandardValue * 1.01;
	overpressureResult.shellScreenFlag = true;
	overpressureResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetExplosiveBlastOverpressureResult(overpressureResult);

	double calPropellantReactionDegreeMinValue = *std::min_element(propellantReactionDegreeResults.begin(), propellantReactionDegreeResults.end());
	double calPropellantReactionDegreeMaxValue = *std::max_element(propellantReactionDegreeResults.begin(), propellantReactionDegreeResults.end());

	// 反应度分析结果（超压/推进剂发火超压）
	ReactionDegreeResult reactionDegreeResult;
	reactionDegreeResult.metalsMaxReactionDegree = 0;
	reactionDegreeResult.metalsMinReactionDegree = 0;
	reactionDegreeResult.metalsAvgReactionDegree = 0;
	reactionDegreeResult.metalsStandardReactionDegree = 0;
	reactionDegreeResult.propellantsMaxReactionDegree = propellantOverpressureMaxValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsMinReactionDegree = propellantOverpressureMinValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsAvgReactionDegree = propellantOverpressureAvgValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsStandardReactionDegree = propellantOverpressureStandardValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.outheatMaxReactionDegree = 0;
	reactionDegreeResult.outheatMinReactionDegree = 0;
	reactionDegreeResult.outheatAvgReactionDegree = 0;
	reactionDegreeResult.outheatStandardReactionDegree = 0;
	reactionDegreeResult.insulatingheatMaxReactionDegree = 0;
	reactionDegreeResult.insulatingheatMinReactionDegree = 0;
	reactionDegreeResult.insulatingheatAvgReactionDegree = 0;
	reactionDegreeResult.insulatingheatStandardReactionDegree = 0;
	reactionDegreeResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetExplosiveBlastReactionDegreeResult(reactionDegreeResult);


	ExplosiveBlastAnalysisResultInfo explosiveBlastAnalysisResultInfo;
	explosiveBlastAnalysisResultInfo.isChecked = true;
	explosiveBlastAnalysisResultInfo.stressMaxValue = qMax(qMax(stressResult.metalsMaxStress, stressResult.propellantsMaxStress), qMax(stressResult.outheatMaxStress, stressResult.insulatingheatMaxStress));
	explosiveBlastAnalysisResultInfo.stressMinValue = qMin(qMin(stressResult.metalsMinStress, stressResult.propellantsMinStress), qMin(stressResult.outheatMinStress, stressResult.insulatingheatMinStress));
	explosiveBlastAnalysisResultInfo.strainMaxValue = qMax(qMax(strainResult.metalsMaxStrain, strainResult.propellantsMaxStrain), qMax(strainResult.outheatMaxStrain, strainResult.insulatingheatMaxStrain));
	explosiveBlastAnalysisResultInfo.strainMinValue = qMin(qMin(strainResult.metalsMinStrain, strainResult.propellantsMinStrain), qMin(strainResult.outheatMinStrain, strainResult.insulatingheatMinStrain));
	explosiveBlastAnalysisResultInfo.temperatureMaxValue = qMax(qMax(temperatureResult.metalsMaxTemperature, temperatureResult.propellantsMaxTemperature), qMax(temperatureResult.outheatMaxTemperature, temperatureResult.insulatingheatMaxTemperature));
	explosiveBlastAnalysisResultInfo.temperatureMinValue = qMin(qMin(temperatureResult.metalsMinTemperature, temperatureResult.propellantsMinTemperature), qMin(temperatureResult.outheatMinTemperature, temperatureResult.insulatingheatMinTemperature));
	explosiveBlastAnalysisResultInfo.overpressureMaxValue = qMax(qMax(overpressureResult.metalsMaxOverpressure, overpressureResult.propellantsMaxOverpressure), qMax(overpressureResult.outheatMaxOverpressure, overpressureResult.insulatingheatMaxOverpressure));
	explosiveBlastAnalysisResultInfo.overpressureMinValue = qMin(qMin(overpressureResult.metalsMinOverpressure, overpressureResult.propellantsMinOverpressure), qMin(overpressureResult.outheatMinOverpressure, overpressureResult.insulatingheatMinOverpressure));
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
	auto C = steelInfo.modulus/1000;
	auto D = steelInfo.thermalConductivity;
	auto E = steelInfo.specificHeatCapacity;

	auto F = propellantInfo.density;
	auto G = propellantInfo.modulus / 1000;
	auto H = propellantInfo.thermalConductivity;
	auto I = propellantInfo.specificHeatCapacity;
	
	auto J = modelGeomInfo.length;//长
	auto K = modelGeomInfo.width;//宽
	auto L = modelGeomInfo.thickness;//厚
	auto M = sacrificeExplosionInfo.distance;//距离

	auto limitValue = steelInfo.tensileStrength * 1.5; // 抗拉强度
	auto tangentModulus = steelInfo.tangentModulus; // 切线模量
	auto modulus = steelInfo.modulus;// 弹性模量
	double difference = (tangentModulus / modulus);
	difference = (difference - 0.15) / 0.15;

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
		res = res + res * difference * 0.1;
		if (res > limitValue)
		{
			res = limitValue;
		}
		if (!m_steelArray.contains(i + 1))
		{
			res = translate(res);
			res = res * 9.13;
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
	stressResult.outheatMaxStress = shellStressMaxValue;
	stressResult.outheatMinStress = shellStressMinValue;
	stressResult.outheatAvgStress = shellStressAvgValue;
	stressResult.outheatStandardStress = shellStressStandardValue;
	stressResult.insulatingheatMaxStress = propellantStressMaxValue * 1.01;
	stressResult.insulatingheatMinStress = propellantStressMinValue * 1.01;
	stressResult.insulatingheatAvgStress = propellantStressAvgValue * 1.01;
	stressResult.insulatingheatStandardStress = propellantStressStandardValue * 1.01;
	stressResult.shellScreenFlag = true;
	stressResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetSacrificeExplosionStressResult(stressResult);

	// 应变分析结果
	StrainResult strainResult;
	strainResult.metalsMaxStrain = stressResult.metalsMaxStress / steelInfo.modulus ;
	strainResult.metalsMinStrain = stressResult.metalsMinStress / steelInfo.modulus ;
	strainResult.metalsAvgStrain = stressResult.metalsAvgStress / steelInfo.modulus ;
	strainResult.metalsStandardStrain = stressResult.metalsStandardStress / steelInfo.modulus ;
	strainResult.propellantsMaxStrain = stressResult.propellantsMaxStress / propellantInfo.modulus;
	strainResult.propellantsMinStrain = stressResult.propellantsMinStress / propellantInfo.modulus;
	strainResult.mpropellantsAvgStrain = stressResult.propellantsAvgStress / propellantInfo.modulus;
	strainResult.propellantsStandardStrain = stressResult.propellantsStandardStress / propellantInfo.modulus;
	strainResult.outheatMaxStrain = stressResult.outheatMaxStress / steelInfo.modulus ;
	strainResult.outheatMinStrain = stressResult.outheatMinStress / steelInfo.modulus ;
	strainResult.outheatAvgStrain = stressResult.outheatAvgStress / steelInfo.modulus ;
	strainResult.outheatStandardStrain = stressResult.outheatStandardStress / steelInfo.modulus ;
	strainResult.insulatingheatMaxStrain = stressResult.insulatingheatMaxStress / steelInfo.modulus ;
	strainResult.insulatingheatMinStrain = stressResult.insulatingheatMinStress / steelInfo.modulus ;
	strainResult.insulatingheatAvgStrain = stressResult.insulatingheatAvgStress / steelInfo.modulus ;
	strainResult.insulatingheatStandardStrain = stressResult.insulatingheatStandardStress / steelInfo.modulus ;
	strainResult.shellScreenFlag = true;
	strainResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetSacrificeExplosionStrainResult(strainResult);

	// 温度分析结果
	TemperatureResult temperatureResult;
	temperatureResult.metalsAvgTemperature = temperatureResult.metalsMinTemperature + QRandomGenerator::global()->generateDouble() * (temperatureResult.metalsMaxTemperature - temperatureResult.metalsMinTemperature);
	temperatureResult.mpropellantsAvgTemperature = temperatureResult.propellantsMinTemperature + QRandomGenerator::global()->generateDouble() * (temperatureResult.propellantsMaxTemperature - temperatureResult.propellantsMinTemperature);

	temperatureResult.outheatMaxTemperature = temperatureResult.metalsMaxTemperature * 1.01;
	temperatureResult.outheatMinTemperature = 25;
	temperatureResult.outheatAvgTemperature = temperatureResult.metalsAvgTemperature * 1.01;
	temperatureResult.outheatStandardTemperature = temperatureResult.metalsStandardTemperature * 1.01;
	temperatureResult.insulatingheatMaxTemperature = temperatureResult.propellantsMaxTemperature * 1.01;
	temperatureResult.insulatingheatMinTemperature = 25;
	temperatureResult.insulatingheatAvgTemperature = temperatureResult.mpropellantsAvgTemperature * 1.01;
	temperatureResult.insulatingheatStandardTemperature = temperatureResult.propellantsStandardTemperature * 1.01;
	temperatureResult.shellScreenFlag = true;
	temperatureResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetSacrificeExplosionTemperatureResult(temperatureResult);

	// 超压
	auto overpressureCalculation = calInfo.sacrificeExplosionOverpressureCalculation;
	std::vector<double> steelOverpressureResults;
	std::vector<double> propellantOverpressureResults;
	// 反应度
	std::vector<double> propellantReactionDegreeResults;
	
	for (int i = 0; i < overpressureCalculation.size(); ++i)
	{
		double res = calculate(overpressureCalculation[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
		if (res < 0)
		{
			res = 0;
		}
		if (!m_steelArray.contains(i + 1))
		{
			res = res * 9.22;
			propellantOverpressureResults.push_back(res);

			// 反应度
			if (propellantInfo.fireOverpressure == 0.0)
			{
				propellantReactionDegreeResults.push_back(0.0);
			}
			else
			{
				auto reactionDegree = res / propellantInfo.fireOverpressure;
				if (reactionDegree > 1)
				{
					propellantReactionDegreeResults.push_back(1);
				}
				else
				{
					propellantReactionDegreeResults.push_back(reactionDegree);

				}

			}
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
	//double shellOverpressureMaxValue = calSteelOverpressureMaxValue; // 发动机壳体最大超压
	//double shellOverpressureMinValue = 0; // 发动机壳体最小超压
	//double shellOverpressureAvgValue = calculateAvg(steelOverpressureResults); // 发动机壳体平均超压
	//double shellOverpressureStandardValue = calculateStd(steelOverpressureResults); // 发动机壳体超压标准差
	//double propellantOverpressureMaxValue = calPropellantOverpressureMaxValue; // 固体推进剂最大超压
	//double propellantOverpressureMinValue = 0; // 固体推进剂最小超压
	//double propellantOverpressureAvgValue = calculateAvg(propellantOverpressureResults); // 固体推进剂平均超压
	//double propellantOverpressureStandardValue = calculateStd(propellantOverpressureResults); // 固体推进剂超压标准差

	double shellOverpressureMaxValue = shellStressMaxValue * 2.47; // 发动机壳体最大超压
	double shellOverpressureMinValue = shellStressMinValue * 2.47; // 发动机壳体最小超压
	double shellOverpressureAvgValue = shellStressAvgValue * 2.47; // 发动机壳体平均超压
	double shellOverpressureStandardValue = shellStressStandardValue * 2.47; // 发动机壳体超压标准差
	double propellantOverpressureMaxValue = propellantStressMaxValue * 5.22; // 固体推进剂最大超压
	double propellantOverpressureMinValue = propellantStressMinValue * 5.22; // 固体推进剂最小超压
	double propellantOverpressureAvgValue = propellantStressAvgValue * 5.22; // 固体推进剂平均超压
	double propellantOverpressureStandardValue = propellantStressStandardValue * 5.22; // 固体推进剂超压标准差

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
	overpressureResult.insulatingheatMaxOverpressure = propellantOverpressureMaxValue * 1.01;
	overpressureResult.insulatingheatMinOverpressure = propellantOverpressureMinValue * 1.01;
	overpressureResult.insulatingheatAvgOverpressure = propellantOverpressureAvgValue * 1.01;
	overpressureResult.insulatingheatStandardOverpressure = propellantOverpressureStandardValue * 1.01;
	overpressureResult.shellScreenFlag = true;
	overpressureResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetSacrificeExplosionOverpressureResult(overpressureResult);

	double calPropellantReactionDegreeMinValue = *std::min_element(propellantReactionDegreeResults.begin(), propellantReactionDegreeResults.end());
	double calPropellantReactionDegreeMaxValue = *std::max_element(propellantReactionDegreeResults.begin(), propellantReactionDegreeResults.end());

	// 反应度分析结果（超压/推进剂发火超压）
	ReactionDegreeResult reactionDegreeResult;
	reactionDegreeResult.metalsMaxReactionDegree = 0;
	reactionDegreeResult.metalsMinReactionDegree = 0;
	reactionDegreeResult.metalsAvgReactionDegree = 0;
	reactionDegreeResult.metalsStandardReactionDegree = 0;
	reactionDegreeResult.propellantsMaxReactionDegree = propellantOverpressureMaxValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsMinReactionDegree = propellantOverpressureMinValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsAvgReactionDegree = propellantOverpressureAvgValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.propellantsStandardReactionDegree = propellantOverpressureStandardValue / propellantInfo.fireOverpressure;
	reactionDegreeResult.outheatMaxReactionDegree = 0;
	reactionDegreeResult.outheatMinReactionDegree = 0;
	reactionDegreeResult.outheatAvgReactionDegree = 0;
	reactionDegreeResult.outheatStandardReactionDegree = 0;
	reactionDegreeResult.insulatingheatMaxReactionDegree = 0;
	reactionDegreeResult.insulatingheatMinReactionDegree = 0;
	reactionDegreeResult.insulatingheatAvgReactionDegree = 0;
	reactionDegreeResult.insulatingheatStandardReactionDegree = 0;
	reactionDegreeResult.propellantScreenFlag = true;
	ModelDataManager::GetInstance()->SetSacrificeExplosionReactionDegreeResult(reactionDegreeResult);




	SacrificeExplosionAnalysisResultInfo sacrificeExplosionAnalysisResultInfo;
	sacrificeExplosionAnalysisResultInfo.isChecked = true;
	sacrificeExplosionAnalysisResultInfo.stressMaxValue = qMax(qMax(stressResult.metalsMaxStress, stressResult.propellantsMaxStress), qMax(stressResult.outheatMaxStress, stressResult.insulatingheatMaxStress));
	sacrificeExplosionAnalysisResultInfo.stressMinValue = qMin(qMin(stressResult.metalsMinStress, stressResult.propellantsMinStress), qMin(stressResult.outheatMinStress, stressResult.insulatingheatMinStress));
	sacrificeExplosionAnalysisResultInfo.strainMaxValue = qMax(qMax(strainResult.metalsMaxStrain, strainResult.propellantsMaxStrain), qMax(strainResult.outheatMaxStrain, strainResult.insulatingheatMaxStrain));
	sacrificeExplosionAnalysisResultInfo.strainMinValue = qMin(qMin(strainResult.metalsMinStrain, strainResult.propellantsMinStrain), qMin(strainResult.outheatMinStrain, strainResult.insulatingheatMinStrain));
	sacrificeExplosionAnalysisResultInfo.temperatureMaxValue = qMax(qMax(temperatureResult.metalsMaxTemperature, temperatureResult.propellantsMaxTemperature), qMax(temperatureResult.outheatMaxTemperature, temperatureResult.insulatingheatMaxTemperature));
	sacrificeExplosionAnalysisResultInfo.temperatureMinValue = qMin(qMin(temperatureResult.metalsMinTemperature, temperatureResult.propellantsMinTemperature), qMin(temperatureResult.outheatMinTemperature, temperatureResult.insulatingheatMinTemperature));
	sacrificeExplosionAnalysisResultInfo.overpressureMaxValue = qMax(qMax(overpressureResult.metalsMaxOverpressure, overpressureResult.propellantsMaxOverpressure), qMax(overpressureResult.outheatMaxOverpressure, overpressureResult.insulatingheatMaxOverpressure));
	sacrificeExplosionAnalysisResultInfo.overpressureMinValue = qMin(qMin(overpressureResult.metalsMinOverpressure, overpressureResult.propellantsMinOverpressure), qMin(overpressureResult.outheatMinOverpressure, overpressureResult.insulatingheatMinOverpressure));
	ModelDataManager::GetInstance()->SetSacrificeExplosionAnalysisResultInfo(sacrificeExplosionAnalysisResultInfo);

	return true;
}