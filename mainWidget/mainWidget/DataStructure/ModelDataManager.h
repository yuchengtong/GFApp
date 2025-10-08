#pragma once
#include <Standard_Type.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>

#include <QObject>
#include <string>
#include <QString>

#include "TriangleStructure.h"


struct UserInfo {
	QString username = "";
	QString password = "";

};

struct ModelGeometryInfo {
	TopoDS_Shape shape;
	QString path="";
	double length = 0.0; 
	double width = 0.0;
	double height = 0.0;
};

struct ModelMeshInfo {
	TriangleStructure triangleStructure;
	bool isChecked=false;
};

struct FallAnalysisResultInfo {
	Handle(TriangleStructure) triangleStructure;
	bool isChecked = false;
	double maxValue;
	double minValue;
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
	// 计算模型
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

	bool isChecked = false;
};

struct FallSettingInfo {
	double high = 0;
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

	void SetFallAnalysisResultInfo(const FallAnalysisResultInfo& info);
	const FallAnalysisResultInfo& GetFallAnalysisResultInfo() const;

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

	void SetFallSettingInfo(const FallSettingInfo& info);
	const FallSettingInfo& GetFallSettingInfo() const;

	void SetJudgementPropertyInfo(const JudgementPropertyInfo& info);
	const JudgementPropertyInfo& GetJudgementPropertyInfo() const;

	void Reset();

private:
	static ModelDataManager* m_Instance; 
	ModelGeometryInfo m_ModelGeometryInfo; 
	ModelMeshInfo m_ModelMeshInfo;
	FallAnalysisResultInfo m_FallAnalysisResultInfo;

	SteelPropertyInfo m_SteelPropertyInfo;
	PropellantPropertyInfo m_PropellantPropertyInfo;
	CalculationPropertyInfo m_CalculationPropertyInfo;

	JudgementPropertyInfo m_JudgementPropertyInfo;
	InsulatingheatPropertyInfo m_InsulatingheatPropertyInfo;
	OutheatPropertyInfo m_OutheatPropertyInfo;

	UserInfo m_UserInfo;

	FallSettingInfo m_FallSettingInfo;
};