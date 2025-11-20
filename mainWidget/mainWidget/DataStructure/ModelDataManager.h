#pragma once
#include <Standard_Type.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>

#include <QObject>
#include <string>
#include <QString>
#include <QRandomGenerator>

#include "TriangleStructure.h"


struct UserInfo {
	QString username = "";
	QString password = "";
};

struct ModelGeometryInfo {
	TopoDS_Shape shape;
	QString path="";
	double theXmin = 0.0;
	double theYmin = 0.0;
	double theZmin = 0.0;
	double theXmax = 0.0;
	double theYmax = 0.0;
	double theZmax = 0.0;


	double length = 0.0; 
	double width = 0.0;
	double height = 0.0;
};

struct ModelMeshInfo {
	TriangleStructure triangleStructure;
	TriangleStructure triangleStructure45;
	TriangleStructure triangleStructure90;
	bool isChecked=false;
};

struct FallSettingInfo {
	double high = 20; // 跌落高度
	int angle = 0; // 跌落姿态
};

struct FallAnalysisResultInfo {
	//TriangleStructure triangleStructure;
	bool isChecked = false;

	double stressMaxValue;
	double stressMinValue;
	double strainMaxValue;
	double strainMinValue;
	double temperatureMaxValue;
	double temperatureMinValue;
	double overpressureMaxValue;
	double overpressureMinValue;
};


struct ShootSettingInfo {
	double speed = 20;
	double radius = 12.7; // 子弹直径
};

struct ShootAnalysisResultInfo {
	bool isChecked = false;

	double stressMaxValue;
	double stressMinValue;
	double strainMaxValue;
	double strainMinValue;
	double temperatureMaxValue;
	double temperatureMinValue;
	double overpressureMaxValue;
	double overpressureMinValue;
};

struct FragmentationSettingInfo {
	double speed = 20;
	double radius = 14.3; // 子弹直径
};

struct FragmentationAnalysisResultInfo {
	bool isChecked = false;

	double stressMaxValue;
	double stressMinValue;
	double strainMaxValue;
	double strainMinValue;
	double temperatureMaxValue;
	double temperatureMinValue;
	double overpressureMaxValue;
	double overpressureMinValue;
};



// 壳体材料
struct SteelPropertyInfo {
	QString materialGrade = ""; // 材料牌号
	double density = 0.0; // 密度
	double thermalExpansion = 0.0;// 热膨胀系数
	double modulus = 0.0;// 杨氏模量
	double poisonby = 0.0;// 泊松比
	double yieldStrength = 0.0;// 屈服强度
	double tensileStrength = 0.0;// 抗拉强度
	double thermalConductivity = 0.0;// 热导率
	double specificHeatCapacity = 0.0;// 比热容
	bool isChecked = false;
};
// 推进剂材料
struct PropellantPropertyInfo {
	QString materialGrade = ""; // 材料牌号
	double density = 0.0; // 密度
	double thermalExpansion = 0.0;// 热膨胀系数
	double modulus = 0.0;// 杨氏模量
	double poisonby = 0.0;// 泊松比
	double ignitionTemperature = 0.0;// 发火温度
	double fireOverpressure = 0.0;// 发火超压
	double thermalConductivity = 0.0;// 热导率
	double specificHeatCapacity = 0.0;// 比热容
	double i = 0.0;
	double a = 0.0;
	double b = 0.0;
	double c = 0.0;
	double d = 0.0;
	double g_1 = 0.0;
	double e = 0.0;
	double g = 0.0;
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
	double g_2 = 0.0;
	bool isChecked = false;
};

// 防隔热材料
struct InsulatingheatPropertyInfo {
	QString materialGrade = ""; // 材料牌号
	double density = 0.0; // 密度
	double thermalExpansion = 0.0;// 热膨胀系数
	double modulus = 0.0;// 杨氏模量
	double poisonby = 0.0;// 泊松比
	double yieldStrength = 0.0;// 屈服强度
	double tensileStrength = 0.0;// 抗拉强度
	double thermalConductivity = 0.0;// 热导率
	double specificHeatCapacity = 0.0;// 比热容
	bool isChecked = false;
};

// 外防热材料
struct OutheatPropertyInfo {
	QString materialGrade = ""; // 材料牌号
	double density = 0.0; // 密度
	double thermalExpansion = 0.0;// 热膨胀系数
	double modulus = 0.0;// 杨氏模量
	double poisonby = 0.0;// 泊松比
	double yieldStrength = 0.0;// 屈服强度
	double tensileStrength = 0.0;// 抗拉强度
	double thermalConductivity = 0.0;// 热导率
	double specificHeatCapacity = 0.0;// 比热容
	bool isChecked = false;
};

// 评判标准类型
struct JudgementPropertyInfo {
	QString name = ""; // 评判标准类型
	bool isChecked = false;
};

// 计算模型数据库
struct CalculationPropertyInfo {
	// 跌落计算模型
	QList<QString> calculation = {
"21.5318*A+0.0194*B+0.0000*C-0.2494*D+0.0193*E+0.0269*F-0.0000*G-0.4904*H+0.0415*I+0.0065*J-0.0493*K-0.6703*L-13.5786*M",
"-1913.1332*A+0.0631*B+0.0000*C+0.6478*D-0.0608*E+0.1858*F-0.0000*G+1.9523*H-0.0042*I+0.0237*J-0.0058*K+2.4911*L+100.7872*M",
"-1193.7939*A+0.0333*B+0.0000*C+1.3217*D-0.0832*E+0.1239*F+0.0000*G+0.3170*H-0.0271*I+0.0169*J-0.1005*K+2.3840*L+51.3554*M",
"-1877.9646*A+0.0632*B+0.0000*C+0.6640*D-0.0716*E+0.1865*F-0.0000*G+1.6218*H-0.0011*I+0.0239*J-0.0234*K+2.5297*L+98.2791*M",
"18.5237*A+0.0194*B+0.0000*C-0.2605*D+0.0194*E+0.0276*F-0.0000*G-0.4854*H+0.0407*I+0.0065*J-0.0474*K-0.6679*L-13.4602*M",
"88.9088*A+0.0420*B+0.0000*C-1.0392*D-0.0118*E+0.1325*F-0.0000*G-0.3935*H+0.0719*I+0.0207*J-0.2300*K-0.4417*L-63.8459*M",
"-238.5778*A+0.0041*B-0.0000*C+0.2595*D-0.0217*E+0.0338*F+0.0000*G+0.4805*H-0.0099*I+0.0036*J-0.0005*K+0.6810*L-0.3672*M",
"-122.1322*A+0.0026*B-0.0000*C+0.1534*D-0.0183*E+0.0225*F+0.0000*G+0.1976*H-0.0058*I+0.0024*J-0.0211*K+0.4253*L-1.8724*M",
"-237.7550*A+0.0042*B-0.0000*C+0.2624*D-0.0217*E+0.0335*F+0.0000*G+0.4934*H-0.0100*I+0.0036*J-0.0013*K+0.6779*L-0.3510*M",
"64.9370*A+0.0420*B+0.0000*C-1.0402*D-0.0172*E+0.1302*F-0.0000*G-0.5280*H+0.0777*I+0.0206*J-0.2174*K-0.4406*L-64.0496*M",
"77.3406*A+0.0248*B+0.0000*C-0.6045*D+0.0091*E+0.0988*F-0.0000*G-0.5802*H+0.0961*I+0.0147*J-0.1023*K-0.8176*L-50.9113*M",
"-110.6043*A+0.0033*B+0.0000*C+0.1579*D-0.0117*E+0.0202*F+0.0000*G+0.3026*H+0.0002*I+0.0025*J-0.0077*K+0.1316*L+0.7883*M",
"-76.1121*A+0.0025*B+0.0000*C+0.1049*D-0.0095*E+0.0176*F+0.0000*G+0.0990*H+0.0015*I+0.0020*J-0.0148*K+0.0705*L+1.0281*M",
"-110.7208*A+0.0033*B+0.0000*C+0.1573*D-0.0118*E+0.0202*F+0.0000*G+0.3070*H+0.0002*I+0.0025*J-0.0075*K+0.1321*L+0.8091*M",
"76.5164*A+0.0245*B+0.0000*C-0.6145*D+0.0117*E+0.0992*F-0.0000*G-0.5656*H+0.1013*I+0.0147*J-0.1026*K-0.8320*L-51.5045*M",
"-694.6278*A+0.0455*B+0.0000*C-0.5442*D+0.0598*E+0.2649*F-0.0000*G+0.3780*H+0.1002*I+0.0316*J-0.3283*K+3.7371*L-101.4766*M",
"-109.7874*A+0.0030*B+0.0000*C+0.1384*D-0.0073*E+0.0183*F+0.0000*G+0.2460*H+0.0026*I+0.0022*J-0.0020*K+0.1426*L+1.0918*M",
"-138.3104*A+0.0031*B+0.0000*C+0.1431*D-0.0068*E+0.0217*F+0.0000*G+0.1444*H+0.0037*I+0.0024*J-0.0058*K+0.2022*L+3.1402*M",
"-109.3954*A+0.0030*B+0.0000*C+0.1381*D-0.0072*E+0.0182*F+0.0000*G+0.2458*H+0.0028*I+0.0022*J-0.0022*K+0.1420*L+1.0652*M",
"-692.2612*A+0.0454*B+0.0000*C-0.5559*D+0.0637*E+0.2656*F-0.0000*G+0.2209*H+0.1029*I+0.0316*J-0.3299*K+3.7233*L-101.6211*M",
"-674.8931*A+0.0338*B+0.0000*C-0.4322*D+0.0727*E+0.2302*F-0.0000*G+1.0625*H+0.1003*I+0.0232*J-0.2966*K+3.1683*L-56.3468*M",
"-77.9199*A+0.0014*B+0.0000*C+0.0972*D-0.0017*E+0.0129*F+0.0000*G+0.1729*H+0.0026*I+0.0014*J-0.0043*K+0.1606*L+0.5266*M",
"-142.1110*A+0.0023*B+0.0000*C+0.1463*D-0.0005*E+0.0221*F+0.0000*G+0.2520*H+0.0057*I+0.0020*J+0.0102*K+0.1978*L+3.1599*M",
"-77.4417*A+0.0014*B+0.0000*C+0.0973*D-0.0016*E+0.0129*F+0.0000*G+0.1705*H+0.0026*I+0.0013*J-0.0045*K+0.1602*L+0.5039*M",
"-692.0747*A+0.0339*B+0.0000*C-0.4239*D+0.0715*E+0.2302*F-0.0000*G+1.0910*H+0.1004*I+0.0232*J-0.2904*K+3.1855*L-56.4163*M",
"140.5368*A+0.0081*B+0.0000*C-0.1295*D+0.0322*E+0.0347*F-0.0000*G-0.0987*H+0.0655*I+0.0055*J-0.0201*K-0.9373*L-16.4131*M",
"-20.8395*A+0.0009*B+0.0000*C+0.0451*D-0.0004*E+0.0055*F+0.0000*G+0.0358*H+0.0043*I+0.0006*J+0.0040*K-0.0468*L+0.4814*M",
"-51.0432*A+0.0013*B+0.0000*C+0.0511*D+0.0022*E+0.0117*F+0.0000*G+0.1186*H+0.0073*I+0.0010*J+0.0140*K-0.0795*L+1.8213*M",
"-20.8532*A+0.0009*B+0.0000*C+0.0451*D-0.0004*E+0.0055*F+0.0000*G+0.0373*H+0.0043*I+0.0006*J+0.0041*K-0.0474*L+0.4729*M",
"133.4387*A+0.0082*B+0.0000*C-0.1363*D+0.0305*E+0.0352*F-0.0000*G-0.0152*H+0.0661*I+0.0055*J-0.0167*K-0.9431*L-16.1459*M",
"76.1087*A+0.0042*B+0.0000*C-0.2895*D+0.0080*E+0.0491*F-0.0000*G+0.2943*H+0.0487*I+0.0063*J-0.0617*K-0.2377*L-19.7193*M",
"-16.8568*A+0.0009*B+0.0000*C+0.0160*D+0.0003*E+0.0040*F+0.0000*G+0.0481*H+0.0017*I+0.0005*J+0.0014*K-0.0276*L+0.5890*M",
"-45.4109*A+0.0017*B+0.0000*C+0.0055*D+0.0039*E+0.0085*F+0.0000*G+0.0961*H+0.0048*I+0.0008*J+0.0090*K-0.0433*L+1.7050*M",
"-16.5876*A+0.0009*B+0.0000*C+0.0161*D+0.0004*E+0.0040*F+0.0000*G+0.0468*H+0.0018*I+0.0005*J+0.0014*K-0.0281*L+0.5775*M",
"73.6455*A+0.0038*B+0.0000*C-0.2832*D+0.0087*E+0.0498*F-0.0000*G+0.3138*H+0.0510*I+0.0063*J-0.0602*K-0.2378*L-20.1816*M",
"-121.5260*A+0.0286*B+0.0000*C-0.0337*D-0.0226*E-0.0075*F+0.0000*G-0.1776*H+0.0216*I+0.0051*J+0.0323*K-0.0420*L-5.4284*M",
"-112.7293*A+0.0142*B+0.0000*C+0.0163*D+0.0047*E+0.0228*F+0.0000*G+0.5184*H+0.0677*I+0.0074*J-0.0766*K+0.0898*L-16.3086*M",
"-199.9882*A+0.0120*B+0.0000*C+0.0586*D+0.0291*E+0.0798*F-0.0000*G+1.7482*H+0.0265*I+0.0094*J+0.1803*K+0.3600*L-28.9311*M",
"-112.6211*A+0.0142*B+0.0000*C+0.0327*D+0.0052*E+0.0226*F+0.0000*G+0.4886*H+0.0684*I+0.0074*J-0.0745*K+0.0874*L-16.6976*M",
"-113.9565*A+0.0280*B+0.0000*C+0.0057*D-0.0166*E-0.0048*F+0.0000*G-0.1665*H+0.0169*I+0.0051*J+0.0195*K-0.0274*L-4.8968*M" 
	};


	// 枪击试验计算模型
	QList<QString> shootCalculation = {
"-464.3395+0.0006*B+0.0000*C+0.3502*D+0.0078*E+0.0160*F+0.0000*G+0.2746*H+0.0133*I+0.3298*J-0.0824*K+12.3105*L+0.0001*M",
"-407.7170+0.0034*B+0.0000*C+0.4335*D-0.0073*E+0.0300*F+0.0000*G+0.4042*H+0.0254*I+0.2306*J-0.1436*K+5.9609*L+0.0001*M",
"110.3868-0.0058*B+0.0000*C+0.9013*D-0.0561*E+0.0443*F+0.0000*G-1.4622*H-0.0120*I-0.0138*J-0.9380*K-5.5476*L+0.0002*M",
"383.5115-0.0051*B+0.0000*C+0.5519*D-0.0271*E+0.0191*F+0.0000*G-1.8176*H-0.0582*I-0.2177*J-1.0582*K-0.1148*L+0.0002*M",
"292.9054-0.0039*B+0.0000*C-0.0449*D+0.0045*E+0.0133*F-0.0000*G-0.4969*H-0.0363*I-0.1519*J-0.7897*K+5.4433*L+0.0001*M",
"-219.8817-0.0016*B+0.0000*C-0.1537*D-0.0191*E+0.0058*F-0.0000*G+0.1872*H+0.0342*I+0.1631*J-0.1817*K+5.2484*L+0.0001*M",
"-10.8697-0.0001*B+0.0000*C+0.0004*D-0.0008*E+0.0002*F+0.0000*G+0.0059*H+0.0008*I+0.0074*J+0.0012*K+0.2429*L+0.0000*M",
"-4.3251-0.0006*B+0.0000*C+0.0373*D-0.0046*E+0.0010*F+0.0000*G-0.0805*H+0.0038*I-0.0052*J-0.0072*K+0.7202*L+0.0000*M",
"11.0980-0.0001*B+0.0000*C+0.0015*D-0.0006*E+0.0007*F+0.0000*G-0.1021*H-0.0002*I-0.0100*J-0.0358*K+0.1478*L+0.0000*M",
"199.4088-0.0063*B+0.0000*C+0.3849*D-0.0043*E+0.0059*F-0.0000*G-0.3280*H-0.0193*I-0.1309*J-0.6625*K+0.8966*L+0.0001*M",
"-119.3268-0.0005*B+0.0000*C+0.1049*D-0.0107*E+0.0049*F-0.0000*G-0.2875*H+0.0134*I+0.0883*J-0.2076*K+0.3949*L+0.0001*M",
"-5.9847-0.0001*B+0.0000*C-0.0057*D-0.0004*E+0.0004*F+0.0000*G+0.0007*H+0.0003*I+0.0044*J-0.0016*K+0.1069*L+0.0000*M",
"0.0280-0.0007*B+0.0000*C-0.0017*D-0.0032*E+0.0012*F+0.0000*G-0.1194*H+0.0029*I-0.0079*J+0.0001*K+0.5431*L+0.0000*M",
"8.8106-0.0001*B+0.0000*C-0.0080*D-0.0006*E+0.0007*F+0.0000*G-0.0701*H-0.0008*I-0.0071*J-0.0245*K+0.0503*L+0.0000*M",
"140.6060-0.0017*B+0.0000*C+0.2376*D-0.0006*E+0.0134*F-0.0000*G-0.6855*H-0.0029*I-0.1120*J-0.5580*K-3.0542*L+0.0001*M",
"-73.0484-0.0004*B+0.0000*C+0.1385*D+0.0014*E-0.0033*F-0.0000*G+0.2926*H+0.0095*I+0.0505*J-0.1322*K-3.1965*L+0.0001*M",
"-1.5110-0.0001*B+0.0000*C-0.0042*D-0.0008*E+0.0002*F+0.0000*G-0.0208*H+0.0003*I+0.0010*J-0.0034*K+0.0450*L+0.0000*M",
"3.5165-0.0002*B+0.0000*C-0.0288*D-0.0017*E-0.0001*F+0.0000*G-0.1723*H+0.0034*I-0.0080*J-0.0044*K-0.1237*L+0.0000*M",
"5.1963-0.0000*B+0.0000*C-0.0014*D-0.0000*E+0.0010*F+0.0000*G-0.0465*H-0.0005*I-0.0062*J-0.0167*K-0.0937*L+0.0000*M",
"134.2464-0.0014*B+0.0000*C+0.4182*D+0.0064*E+0.0047*F-0.0000*G+0.0895*H-0.0203*I-0.0860*J-0.4004*K-8.2997*L+0.0001*M",
"-7.2274-0.0006*B+0.0000*C+0.0102*D-0.0018*E+0.0001*F-0.0948*H+0.0026*I+0.0054*J-0.0098*K-0.4654*L+0.0000*M",
"-0.1981-0.0000*B+0.0000*C+0.0004*D+0.0000*E-0.0001*F-0.0019*H+0.0001*I+0.0001*J+0.0000*K-0.0136*L+0.0000*M",
"1.0197-0.0002*B+0.0000*C+0.0009*D+0.0003*E-0.0006*F+0.0000*G-0.0355*H+0.0004*I+0.0008*J-0.0091*K-0.2728*L+0.0000*M",
"1.3167-0.0001*B+0.0000*C-0.0016*D-0.0007*E+0.0002*F-0.0158*H+0.0003*I-0.0010*J-0.0082*K-0.0635*L+0.0000*M",
"31.6322-0.0020*B+0.0000*C-0.0004*D-0.0077*E+0.0015*F-0.0000*G-0.1922*H+0.0013*I-0.0221*J-0.0976*K-0.7517*L+0.0000*M",
"-8.7223-0.0004*B+0.0000*C+0.0122*D-0.0011*E-0.0005*F-0.0353*H+0.0031*I+0.0050*J+0.0018*K-0.2850*L+0.0000*M",
"-0.1316-0.0000*B+0.0000*C+0.0003*D-0.0000*E-0.0001*F-0.0021*H+0.0000*I+0.0001*J-0.0001*K-0.0074*L+0.0000*M",
"-0.2214-0.0001*B+0.0000*C+0.0032*D-0.0005*E-0.0001*F+0.0000*G-0.0273*H+0.0004*I+0.0008*J-0.0064*K-0.0502*L+0.0000*M",
"1.0439-0.0001*B+0.0000*C-0.0038*D-0.0006*E+0.0001*F-0.0104*H+0.0002*I-0.0008*J-0.0055*K-0.0457*L+0.0000*M",
"18.0694-0.0015*B+0.0000*C+0.0105*D-0.0056*E+0.0013*F-0.0000*G-0.1204*H+0.0025*I-0.0145*J-0.0711*K-0.4236*L+0.0000*M",
"-12.2290-0.0002*B+0.0000*C+0.0129*D-0.0001*E-0.0003*F-0.0000*G-0.0026*H+0.0017*I+0.0079*J+0.0091*K-0.1105*L+0.0000*M",
"-0.1067-0.0000*B+0.0000*C+0.0003*D+0.0000*E-0.0001*F-0.0025*H-0.0000*I-0.0000*J+0.0002*K+0.0028*L+0.0000*M",
"0.0236-0.0001*B+0.0000*C+0.0014*D-0.0005*E+0.0000*F-0.0216*H+0.0003*I+0.0001*J-0.0056*K-0.0295*L+0.0000*M",
"0.7760-0.0001*B+0.0000*C-0.0031*D-0.0004*E+0.0001*F-0.0089*H+0.0002*I-0.0007*J-0.0047*K-0.0228*L+0.0000*M",
"17.7018-0.0013*B+0.0000*C+0.0093*D-0.0034*E+0.0008*F-0.0000*G-0.1167*H+0.0015*I-0.0169*J-0.0591*K-0.0155*L+0.0000*M",
"-32.6066-0.0001*B+0.0000*C+0.0134*D+0.0010*E+0.0001*F+0.1122*H+0.0031*I+0.0194*J+0.0319*K-0.2898*L+0.0000*M",
"-10.7688-0.0002*B+0.0000*C+0.0023*D+0.0014*E-0.0002*F-0.0191*H+0.0008*I+0.0063*J+0.0113*K-0.1705*L+0.0000*M",
"1.8899-0.0024*B+0.0000*C+0.0308*D-0.0043*E+0.0000*F-0.0000*G-0.3337*H+0.0031*I-0.0127*J-0.1053*K+1.4166*L+0.0000*M",
"21.5986-0.0011*B+0.0000*C+0.0167*D-0.0060*E+0.0013*F-0.0000*G-0.0444*H+0.0001*I-0.0216*J-0.0582*K-0.3725*L+0.0000*M",
"14.4457-0.0012*B+0.0000*C+0.0010*D-0.0003*E+0.0011*F-0.0000*G-0.0340*H-0.0003*I-0.0157*J-0.0377*K-0.4209*L+0.0000*M"
	};


	// 破片试验计算模型
	QList<QString> fragmentationImpactCalculation = {
"-139.6736-0.0050*B+0.0000*C-0.2124*D+0.0473*E+0.0260*F-0.0000*G+2.6710*H-0.0211*I+0.3241*J-0.3842*K-2.7188*L-0.0001*M",
"-150.6801+0.0042*B+0.0000*C+0.2666*D+0.0230*E+0.0265*F+0.0000*G+3.7113*H-0.0093*I+0.1428*J-0.5152*K+1.6753*L-0.0001*M",
"539.0658+0.0040*B+0.0000*C-0.0101*D+0.0550*E+0.0037*F-0.0000*G+6.5535*H+0.0349*I-0.0711*J-1.3393*K+2.5457*L-0.0002*M",
"970.3825-0.0032*B+0.0000*C-0.3465*D+0.0053*E-0.0081*F-0.0000*G+4.0598*H+0.0088*I-0.4477*J-1.3780*K+3.2058*L-0.0002*M",
"433.2151-0.0021*B+0.0000*C+0.0479*D+0.0848*E+0.0253*F-0.0000*G+2.7073*H+0.0004*I-0.3042*J-1.1106*K+4.5870*L-0.0001*M",
"-80.7179-0.0001*B+0.0000*C+0.0926*D+0.0320*E+0.0151*F+0.0000*G+2.6903*H-0.0047*I+0.0751*J-0.4117*K+2.6123*L-0.0001*M",
"1.7761-0.0001*B+0.0000*C-0.0008*D+0.0008*E+0.0002*F+0.0000*G+0.0877*H-0.0014*I+0.0038*J-0.0084*K+0.0543*L-0.0000*M",
"31.3849-0.0001*B+0.0000*C-0.0073*D-0.0010*E-0.0010*F+0.0000*G+0.0371*H+0.0016*I-0.0165*J-0.0276*K+0.5673*L-0.0000*M",
"44.5054+0.0001*B+0.0000*C-0.0269*D-0.0003*E+0.0001*F+0.0000*G+0.1876*H-0.0000*I-0.0228*J-0.0664*K-0.1174*L-0.0000*M",
"643.6441-0.0014*B+0.0000*C-0.2535*D+0.0014*E-0.0014*F+0.0000*G+3.0385*H+0.0025*I-0.3006*J-0.9734*K+0.1060*L-0.0001*M",
"-35.2369-0.0001*B+0.0000*C+0.0868*D+0.0090*E-0.0054*F+0.0000*G+1.9803*H-0.0181*I+0.0316*J-0.2903*K-3.3555*L+0.0000*M",
"2.4541-0.0001*B+0.0000*C+0.0007*D+0.0002*E+0.0003*F+0.0000*G+0.0309*H-0.0004*I+0.0026*J-0.0039*K+0.0484*L-0.0000*M",
"33.8077-0.0002*B+0.0000*C-0.0078*D-0.0020*E-0.0004*F+0.0000*G+0.0485*H+0.0015*I-0.0128*J-0.0244*K+0.3096*L-0.0000*M",
"37.2226+0.0001*B+0.0000*C-0.0205*D-0.0005*E+0.0003*F+0.0000*G+0.1292*H+0.0002*I-0.0197*J-0.0530*K-0.0981*L-0.0000*M",
"400.1964+0.0016*B+0.0000*C+0.0510*D+0.0267*E+0.0034*F+0.0000*G+2.1037*H-0.0232*I-0.2467*J-0.8380*K-6.2533*L-0.0001*M",
"-79.0359+0.0012*B+0.0000*C+0.0438*D+0.0264*E-0.0142*F+0.0000*G+0.6716*H-0.0058*I+0.0501*J-0.0463*K-7.8836*L+0.0000*M",
"-1.0667-0.0000*B+0.0000*C-0.0004*D+0.0000*E+0.0002*F+0.0000*G+0.0122*H-0.0003*I+0.0012*J-0.0020*K-0.0403*L+0.0000*M",
"26.1442+0.0001*B+0.0000*C-0.0275*D-0.0032*E-0.0004*F+0.0000*G+0.0776*H+0.0022*I-0.0205*J+0.0015*K+0.5332*L-0.0000*M",
"38.4823+0.0000*B+0.0000*C-0.0285*D-0.0017*E+0.0005*F+0.0000*G+0.1651*H-0.0001*I-0.0181*J-0.0456*K-0.1004*L-0.0000*M",
"398.9091+0.0015*B+0.0000*C-0.0542*D+0.0033*E-0.0032*F+0.0000*G+2.2065*H-0.0201*I-0.2426*J-0.7452*K-4.7710*L-0.0001*M",
"-24.2324-0.0005*B+0.0000*C+0.0697*D+0.0049*E-0.0020*F+0.0000*G-0.1500*H+0.0050*I+0.0341*J-0.1009*K-1.2197*L+0.0000*M",
"0.7839+0.0000*B+0.0000*C+0.0021*D+0.0004*E-0.0003*F+0.0000*G+0.0004*H-0.0003*I+0.0020*J-0.0069*K-0.1000*L-0.0000*M",
"29.1597+0.0008*B+0.0000*C-0.0089*D+0.0005*E-0.0018*F+0.0000*G-0.0619*H-0.0034*I-0.0019*J-0.0549*K-0.0250*L-0.0000*M",
"24.5833+0.0003*B+0.0000*C-0.0006*D+0.0003*E+0.0006*F+0.0000*G+0.0955*H-0.0011*I-0.0158*J-0.0520*K-0.3546*L-0.0000*M",
"373.4908+0.0017*B+0.0000*C+0.0469*D-0.0041*E+0.0012*F+0.0000*G+1.4105*H-0.0090*I-0.1859*J-0.7718*K-5.9016*L-0.0001*M",
"-28.0752-0.0009*B+0.0000*C+0.0971*D-0.0019*E-0.0041*F+0.0000*G-0.0794*H+0.0086*I+0.0390*J-0.0947*K-0.9369*L+0.0000*M",
"0.2487-0.0000*B+0.0000*C+0.0019*D-0.0000*E-0.0003*F+0.0000*G-0.0069*H-0.0002*I+0.0014*J-0.0056*K-0.0334*L+0.0000*M",
"25.5071+0.0008*B+0.0000*C-0.0222*D-0.0023*E-0.0026*F+0.0000*G-0.2287*H+0.0007*I-0.0051*J-0.0505*K-0.4830*L+0.0000*M",
"43.9758+0.0003*B+0.0000*C-0.0177*D-0.0013*E+0.0005*F+0.0000*G+0.1972*H-0.0004*I-0.0199*J-0.0573*K-0.3936*L-0.0000*M",
"334.2641+0.0015*B+0.0000*C-0.0792*D-0.0090*E+0.0015*F+0.0000*G+1.4890*H-0.0029*I-0.1756*J-0.7200*K-4.6210*L-0.0001*M",
"-25.1960-0.0008*B+0.0000*C+0.0579*D+0.0002*E-0.0033*F+0.0000*G-0.0216*H+0.0017*I+0.0360*J-0.0642*K-0.7679*L+0.0000*M",
"-0.9817-0.0000*B+0.0000*C+0.0019*D+0.0002*E-0.0003*F+0.0000*G-0.0071*H+0.0001*I+0.0010*J-0.0044*K+0.0312*L+0.0000*M",
"37.8351+0.0003*B+0.0000*C+0.0080*D-0.0039*E-0.0039*F+0.0000*G-0.1239*H-0.0008*I+0.0062*J-0.0760*K-0.1982*L-0.0000*M",
"47.3898+0.0001*B+0.0000*C-0.0091*D-0.0017*E+0.0009*F+0.0000*G+0.1620*H-0.0020*I-0.0174*J-0.0620*K-0.6092*L-0.0000*M",
"322.6650+0.0012*B+0.0000*C-0.0313*D-0.0093*E+0.0060*F+0.0000*G+1.4652*H-0.0101*I-0.1624*J-0.7098*K-4.0378*L-0.0001*M",
"-59.1798-0.0005*B+0.0000*C+0.0159*D+0.0035*E+0.0005*F+0.0000*G+0.0984*H+0.0025*I+0.0467*J-0.0155*K-1.1321*L+0.0000*M",
"-13.7867-0.0001*B+0.0000*C+0.0180*D+0.0020*E-0.0011*F+0.0000*G+0.0556*H+0.0032*I+0.0229*J-0.0594*K-0.1810*L+0.0000*M",
"326.4528+0.0075*B-0.0000*C-0.1872*D-0.0106*E-0.0350*F-0.0000*G-0.9133*H+0.0607*I+0.1309*J-0.9136*K-1.8329*L-0.0000*M",
"331.3298+0.0012*B+0.0000*C-0.1495*D-0.0111*E+0.0036*F+0.0000*G+1.7278*H-0.0018*I-0.1487*J-0.5204*K-2.9582*L-0.0001*M",
"202.0684+0.0007*B+0.0000*C-0.1148*D-0.0003*E+0.0040*F+0.0000*G+0.8944*H-0.0131*I-0.0868*J-0.4031*K-4.1022*L-0.0000*M"
	};

	bool isChecked = false;
};

// 应力分析结果
struct StressResult {
	double metalsMaxStress = QRandomGenerator::securelySeeded().bounded(200, 800); //发动机壳体最大应力
	double metalsMinStress = QRandomGenerator::securelySeeded().bounded(200, 800); //发动机壳体最小应力
	double metalsAvgStress = QRandomGenerator::securelySeeded().bounded(200, 800); //发动机壳体平均应力
	double metalsStandardStress = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //发动机壳体应力标准差

	double propellantsMaxStress = QRandomGenerator::securelySeeded().bounded(200, 800); //推进剂最大应力
	double propellantsMinStress = QRandomGenerator::securelySeeded().bounded(200, 800); //推进剂最小应力
	double propellantsAvgStress = QRandomGenerator::securelySeeded().bounded(200, 800);; //推进剂平均应力
	double propellantsStandardStress = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //推进剂应力标准差

	double outheatMaxStress = QRandomGenerator::securelySeeded().bounded(200, 800); //外防热最大应力
	double outheatMinStress = QRandomGenerator::securelySeeded().bounded(200, 800); //外防热最小应力
	double outheatAvgStress = QRandomGenerator::securelySeeded().bounded(200, 800); //外防热平均应力
	double outheatStandardStress = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //外防热应力标准差

	double insulatingheatMaxStress = QRandomGenerator::securelySeeded().bounded(200, 800); //防隔热最大应力
	double insulatingheatMinStress = QRandomGenerator::securelySeeded().bounded(200, 800); //防隔热最小应力
	double insulatingheatAvgStress = QRandomGenerator::securelySeeded().bounded(200, 800); //防隔热平均应力
	double insulatingheatStandardStress = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //防隔热应力标准差
		
};

// 应变分析结果
struct StrainResult {
	double metalsMaxStrain = 0.0001 + (0.001 - 0.0001) * QRandomGenerator::securelySeeded().generateDouble(); //发动机壳体最大应变
	double metalsMinStrain = 0.0001 + (0.001 - 0.0001) * QRandomGenerator::securelySeeded().generateDouble(); //发动机壳体最小应变
	double metalsAvgStrain = 0.0001 + (0.001 - 0.0001) * QRandomGenerator::securelySeeded().generateDouble(); //发动机壳体平均应变
	double metalsStandardStrain = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //发动机壳体应变标准差

	double propellantsMaxStrain = 0.0001 + (0.001 - 0.0001) * QRandomGenerator::securelySeeded().generateDouble(); //推进剂最大应变
	double propellantsMinStrain = 0.0001 + (0.001 - 0.0001) * QRandomGenerator::securelySeeded().generateDouble(); //推进剂最小应变
	double mpropellantsAvgStrain = 0.0001 + (0.001 - 0.0001) * QRandomGenerator::securelySeeded().generateDouble(); //推进剂平均应变
	double propellantsStandardStrain = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //推进剂应变标准差

	double outheatMaxStrain = 0.0001 + (0.001 - 0.0001) * QRandomGenerator::securelySeeded().generateDouble(); //外防热最大应变
	double outheatMinStrain = 0.0001 + (0.001 - 0.0001) * QRandomGenerator::securelySeeded().generateDouble(); //外防热最小应变
	double outheatAvgStrain = 0.0001 + (0.001 - 0.0001) * QRandomGenerator::securelySeeded().generateDouble(); //外防热平均应变
	double outheatStandardStrain = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //外防热应变标准差

	double insulatingheatMaxStrain = 0.0001 + (0.001 - 0.0001) * QRandomGenerator::securelySeeded().generateDouble(); //防隔热最大应变
	double insulatingheatMinStrain = 0.0001 + (0.001 - 0.0001) * QRandomGenerator::securelySeeded().generateDouble(); //防隔热最小应变
	double insulatingheatAvgStrain = 0.0001 + (0.001 - 0.0001) * QRandomGenerator::securelySeeded().generateDouble(); //防隔热平均应变
	double insulatingheatStandardStrain = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //防隔热应变标准差

};

// 温度分析结果
struct TemperatureResult {
	double metalsMaxTemperature = QRandomGenerator::securelySeeded().bounded(50, 101); //发动机壳体最高温度
	double metalsMinTemperature = QRandomGenerator::securelySeeded().bounded(50, 101); //发动机壳体最低温度
	double metalsAvgTemperature = QRandomGenerator::securelySeeded().bounded(50, 101); //发动机壳体平均温度
	double metalsStandardTemperature = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //发动机壳体温度标准差

	double propellantsMaxTemperature = QRandomGenerator::securelySeeded().bounded(50, 101); //推进剂最高温度
	double propellantsMinTemperature = QRandomGenerator::securelySeeded().bounded(50, 101); //推进剂最低温度
	double mpropellantsAvgTemperature = QRandomGenerator::securelySeeded().bounded(50, 101); //推进剂平均温度
	double propellantsStandardTemperature = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //推进剂温度标准差

	double outheatMaxTemperature = QRandomGenerator::securelySeeded().bounded(50, 101); //外防热最高温度
	double outheatMinTemperature = QRandomGenerator::securelySeeded().bounded(50, 101); //外防热最低温度
	double outheatAvgTemperature = QRandomGenerator::securelySeeded().bounded(50, 101); //外防热平均温度
	double outheatStandardTemperature = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //外防热温度标准差

	double insulatingheatMaxTemperature = QRandomGenerator::securelySeeded().bounded(50, 101); //防隔热最高温度
	double insulatingheatMinTemperature = QRandomGenerator::securelySeeded().bounded(50, 101); //防隔热最低温度
	double insulatingheatAvgTemperature = QRandomGenerator::securelySeeded().bounded(50, 101); //防隔热平均温度
	double insulatingheatStandardTemperature = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //防隔热温度标准差

};

// 超压分析结果
struct OverpressureResult {
	double metalsMaxOverpressure = QRandomGenerator::securelySeeded().bounded(2, 16); //发动机壳体最大超压
	double metalsMinOverpressure = QRandomGenerator::securelySeeded().bounded(2, 16); //发动机壳体最小超压
	double metalsAvgOverpressure = QRandomGenerator::securelySeeded().bounded(2, 16); //发动机壳体平均超压
	double metalsStandardOverpressure = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //发动机壳体超压标准差

	double propellantsMaxOverpressure = QRandomGenerator::securelySeeded().bounded(2, 16); //推进剂最大超压
	double propellantsMinOverpressure = QRandomGenerator::securelySeeded().bounded(2, 16); //推进剂最小超压
	double mpropellantsAvgOverpressure = QRandomGenerator::securelySeeded().bounded(2, 16); //推进剂平均超压
	double propellantsStandardOverpressure = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //推进剂超压标准差

	double outheatMaxOverpressure = QRandomGenerator::securelySeeded().bounded(2, 16); //外防热最大超压
	double outheatMinOverpressure = QRandomGenerator::securelySeeded().bounded(2, 16); //外防热最小超压
	double outheatAvgOverpressure = QRandomGenerator::securelySeeded().bounded(2, 16); //外防热平均超压
	double outheatStandardOverpressure = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //外防热超压标准差

	double insulatingheatMaxOverpressure = QRandomGenerator::securelySeeded().bounded(2, 16); //防隔热最大超压
	double insulatingheatMinOverpressure = QRandomGenerator::securelySeeded().bounded(2, 16); //防隔热最小超压
	double insulatingheatAvgOverpressure = QRandomGenerator::securelySeeded().bounded(2, 16); //防隔热平均超压
	double insulatingheatStandardOverpressure = 0.0 + (3.0 - 0.0) * QRandomGenerator::securelySeeded().generateDouble(); //防隔热超压标准差

};


// 评分结果
struct PointResult {
	double fallPoint = 0.0; // 跌落评分
	double fastCombustionPoint = 0.0; // 快烤评分
	double slowCombustionPoint = 0.0; // 慢烤评分
	double shootPoint = 0.0; // 枪击评分
	double jetImpactPoint = 0.0; // 射流冲击评分
	double fragmentationImpactPoint = 0.0; // 破片撞击评分
	double explosiveBlastPoint = 0.0; // 爆炸冲击波评分
	double sacrificeExplosionPoint = 0.0; // 殉爆评分

};

// 单例模式的模型数据管理类
class ModelDataManager : public QObject 
{
	Q_OBJECT
private:
	ModelDataManager();

public:
	// 禁止拷贝和赋值（确保单例唯一性）
	ModelDataManager(const ModelDataManager&) = delete;
	ModelDataManager& operator=(const ModelDataManager&) = delete;


	static ModelDataManager* GetInstance();

	// 设置模型数据
	void SetModelGeometryInfo(const ModelGeometryInfo& info);
	const ModelGeometryInfo& GetModelGeometryInfo() const;

	void SetModelMeshInfo(const ModelMeshInfo& info);
	const ModelMeshInfo& GetModelMeshInfo() const;

	void SetFallSettingInfo(const FallSettingInfo& info);
	const FallSettingInfo& GetFallSettingInfo() const;

	void SetShootSettingInfo(const ShootSettingInfo& info);
	const ShootSettingInfo& GetShootSettingInfo() const;

	void SetFragmentationSettingInfo(const FragmentationSettingInfo& info);
	const FragmentationSettingInfo& GetFragmentationSettingInfo() const;

	void SetFallAnalysisResultInfo(const FallAnalysisResultInfo& info);
	const FallAnalysisResultInfo& GetFallAnalysisResultInfo() const;

	void SetShootAnalysisResultInfo(const ShootAnalysisResultInfo& info);
	const ShootAnalysisResultInfo& GetShootAnalysisResultInfo() const;

	void SetFragmentationAnalysisResultInfo(const FragmentationAnalysisResultInfo& info);
	const FragmentationAnalysisResultInfo& GetFragmentationAnalysisResultInfo() const;



	void SetSteelPropertyInfo(const SteelPropertyInfo& info);
	const SteelPropertyInfo& GetSteelPropertyInfo() const;

	void SetPropellantPropertyInfo(const PropellantPropertyInfo& info);
	const PropellantPropertyInfo& GetPropellantPropertyInfo() const;

	void SetInsulatingheatPropertyInfo(const InsulatingheatPropertyInfo& info);
	const InsulatingheatPropertyInfo& GetInsulatingheatPropertyInfo() const;

	void SetOutheatPropertyInfo(const OutheatPropertyInfo& info);
	const OutheatPropertyInfo& GetOutheatPropertyInfo() const;

	void SetCalculationPropertyInfo(const CalculationPropertyInfo& info);
	const CalculationPropertyInfo& GetCalculationPropertyInfo() const;

	void SetUserInfo(const UserInfo& info);
	const UserInfo& GetUserInfo() const;



	void SetJudgementPropertyInfo(const JudgementPropertyInfo& info);
	const JudgementPropertyInfo& GetJudgementPropertyInfo() const;

	void Reset();

	// 跌落计算结果
	void SetFallStressResult(const StressResult& result);
	const StressResult& GetFallStressResult() const;

	void SetFallStrainResult(const StrainResult& result);
	const StrainResult& GetFallStrainResult() const;

	void SetFallTemperatureResult(const TemperatureResult& result);
	const TemperatureResult& GetFallTemperatureResult() const;

	void SetFallOverpressureResult(const OverpressureResult& result);
	const OverpressureResult& GetFallOverpressureResult() const;

	// 快烤计算结果
	void SetFastCombustionTemperatureResult(const TemperatureResult& result);
	const TemperatureResult& GetFastCombustionTemperatureResult() const;

	// 慢烤计算结果
	void SetSlowCombustionTemperatureResult(const TemperatureResult& result);
	const TemperatureResult& GetSlowCombustionTemperatureResult() const;

	// 枪击计算结果
	void SetShootStressResult(const StressResult& result);
	const StressResult& GetShootStressResult() const;

	void SetShootStrainResult(const StrainResult& result);
	const StrainResult& GetShootStrainResult() const;

	void SetShootTemperatureResult(const TemperatureResult& result);
	const TemperatureResult& GetShootTemperatureResult() const;

	void SetShootOverpressureResult(const OverpressureResult& result);
	const OverpressureResult& GetShootOverpressureResult() const;

	// 射流冲击计算结果
	void SetJetImpactStressResult(const StressResult& result);
	const StressResult& GetJetImpactStressResult() const;

	void SetJetImpactStrainResult(const StrainResult& result);
	const StrainResult& GetJetImpactStrainResult() const;

	void SetJetImpactTemperatureResult(const TemperatureResult& result);
	const TemperatureResult& GetJetImpactTemperatureResult() const;

	void SetJetImpactOverpressureResult(const OverpressureResult& result);
	const OverpressureResult& GetJetImpactOverpressureResult() const;

	// 破片撞击计算结果
	void SetFragmentationImpactStressResult(const StressResult& result);
	const StressResult& GetFragmentationImpactStressResult() const;

	void SetFragmentationImpactStrainResult(const StrainResult& result);
	const StrainResult& GetFragmentationImpactStrainResult() const;

	void SetFragmentationImpactTemperatureResult(const TemperatureResult& result);
	const TemperatureResult& GetFragmentationImpactTemperatureResult() const;

	void SetFragmentationImpactOverpressureResult(const OverpressureResult& result);
	const OverpressureResult& GetFragmentationImpactOverpressureResult() const;

	// 爆炸冲击波计算结果
	void SetExplosiveBlastStressResult(const StressResult& result);
	const StressResult& GetExplosiveBlastStressResult() const;

	void SetExplosiveBlastStrainResult(const StrainResult& result);
	const StrainResult& GetExplosiveBlastStrainResult() const;

	void SetExplosiveBlastTemperatureResult(const TemperatureResult& result);
	const TemperatureResult& GetExplosiveBlastTemperatureResult() const;

	void SetExplosiveBlastOverpressureResult(const OverpressureResult& result);
	const OverpressureResult& GetExplosiveBlastOverpressureResult() const;

	// 殉爆计算结果
	void SetSacrificeExplosionStressResult(const StressResult& result);
	const StressResult& GetSacrificeExplosionStressResult() const;

	void SetSacrificeExplosionStrainResult(const StrainResult& result);
	const StrainResult& GetSacrificeExplosionStrainResult() const;

	void SetSacrificeExplosionTemperatureResult(const TemperatureResult& result);
	const TemperatureResult& GetSacrificeExplosionTemperatureResult() const;

	void SetSacrificeExplosionOverpressureResult(const OverpressureResult& result);
	const OverpressureResult& GetSacrificeExplosionOverpressureResult() const;

	void SetPointResult(const PointResult& result);
	const PointResult& GetPointResult() const;

private:
	static ModelDataManager* m_Instance; 
	ModelGeometryInfo m_ModelGeometryInfo; 
	ModelMeshInfo m_ModelMeshInfo;

	FallSettingInfo m_FallSettingInfo;
	ShootSettingInfo m_ShootSettingInfo;
	FragmentationSettingInfo m_FragmentationSettingInfo;

	FallAnalysisResultInfo m_FallAnalysisResultInfo;
	ShootAnalysisResultInfo m_ShootAnalysisResultInfo;
	FragmentationAnalysisResultInfo m_FragmentationAnalysisResultInfo;


	SteelPropertyInfo m_SteelPropertyInfo;
	PropellantPropertyInfo m_PropellantPropertyInfo;
	CalculationPropertyInfo m_CalculationPropertyInfo;

	JudgementPropertyInfo m_JudgementPropertyInfo;
	InsulatingheatPropertyInfo m_InsulatingheatPropertyInfo;
	OutheatPropertyInfo m_OutheatPropertyInfo;

	UserInfo m_UserInfo;



	// 跌落计算结果
	StressResult m_FallStressResult;
	StrainResult m_FallStrainResult;
	TemperatureResult m_FallTemperatureResult;
	OverpressureResult m_FallOverpressureResult;

	// 快烤计算结果
	TemperatureResult m_FastCombustionTemperatureResult;

	// 慢烤计算结果
	TemperatureResult m_SlowCombustionTemperatureResult;

	// 枪击计算结果
	StressResult m_ShootStressResult;
	StrainResult m_ShootStrainResult;
	TemperatureResult m_ShootTemperatureResult;
	OverpressureResult m_ShootOverpressureResult;

	// 射流冲击计算结果
	StressResult m_JetImpactStressResult;
	StrainResult m_JetImpactStrainResult;
	TemperatureResult m_JetImpactTemperatureResult;
	OverpressureResult m_JetImpactOverpressureResult;

	// 破片撞击计算结果
	StressResult m_FragmentationImpactStressResult;
	StrainResult m_FragmentationImpactStrainResult;
	TemperatureResult m_FragmentationImpactTemperatureResult;
	OverpressureResult m_FragmentationImpactOverpressureResult;

	// 爆炸冲击波计算结果
	StressResult m_ExplosiveBlastStressResult;
	StrainResult m_ExplosiveBlastStrainResult;
	TemperatureResult m_ExplosiveBlastTemperatureResult;
	OverpressureResult m_ExplosiveBlastOverpressureResult;

	// 殉爆计算结果
	StressResult m_SacrificeExplosionStressResult;
	StrainResult m_SacrificeExplosionStrainResult;
	TemperatureResult m_SacrificeExplosionTemperatureResult;
	OverpressureResult m_SacrificeExplosionOverpressureResult;

	// 评分结果
	PointResult m_pointResult;

};



