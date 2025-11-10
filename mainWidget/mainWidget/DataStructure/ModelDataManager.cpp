#include "ModelDataManager.h"
#include <fstream>
#include <iostream>
#include <sstream>

// 初始化静态成员（单例实例）
ModelDataManager* ModelDataManager::m_Instance = nullptr;

ModelDataManager::ModelDataManager()
{
	//初始化
	Reset();
}

// 获取单例实例
ModelDataManager* ModelDataManager::GetInstance() {
	if (m_Instance == nullptr) {
		m_Instance = new ModelDataManager();
	}
	return m_Instance;
}

// 设置模型数据
void ModelDataManager::SetModelGeometryInfo(const ModelGeometryInfo& info)
{
	m_ModelGeometryInfo = info;
}

// 获取模型数据
const ModelGeometryInfo& ModelDataManager::GetModelGeometryInfo() const 
{
	return m_ModelGeometryInfo;
}

void ModelDataManager::SetModelMeshInfo(const ModelMeshInfo & info)
{
	m_ModelMeshInfo = info;
}

const ModelMeshInfo & ModelDataManager::GetModelMeshInfo() const
{
	return m_ModelMeshInfo;
}

void ModelDataManager::SetFallAnalysisResultInfo(const FallAnalysisResultInfo & info)
{
	m_FallAnalysisResultInfo = info;
}

const FallAnalysisResultInfo & ModelDataManager::GetFallAnalysisResultInfo() const
{
	return m_FallAnalysisResultInfo;
}


// 材料库
const SteelPropertyInfo & ModelDataManager::GetSteelPropertyInfo() const
{
	return m_SteelPropertyInfo;
}

void ModelDataManager::SetSteelPropertyInfo(const SteelPropertyInfo& info)
{
	m_SteelPropertyInfo = info;
}

const PropellantPropertyInfo & ModelDataManager::GetPropellantPropertyInfo() const
{
	return m_PropellantPropertyInfo;
}

void ModelDataManager::SetPropellantPropertyInfo(const PropellantPropertyInfo& info)
{
	m_PropellantPropertyInfo = info;
}

const InsulatingheatPropertyInfo & ModelDataManager::GetInsulatingheatPropertyInfo() const
{
	return m_InsulatingheatPropertyInfo;
}

void ModelDataManager::SetInsulatingheatPropertyInfo(const InsulatingheatPropertyInfo& info)
{
	m_InsulatingheatPropertyInfo = info;
}

const OutheatPropertyInfo & ModelDataManager::GetOutheatPropertyInfo() const
{
	return m_OutheatPropertyInfo;
}

void ModelDataManager::SetOutheatPropertyInfo(const OutheatPropertyInfo& info)
{
	m_OutheatPropertyInfo = info;
}

const CalculationPropertyInfo & ModelDataManager::GetCalculationPropertyInfo() const
{
	return m_CalculationPropertyInfo;
}

void ModelDataManager::SetCalculationPropertyInfo(const CalculationPropertyInfo& info)
{
	m_CalculationPropertyInfo = info;
}

const UserInfo & ModelDataManager::GetUserInfo() const
{
	return m_UserInfo;
}

void ModelDataManager::SetFallSettingInfo(const FallSettingInfo & info)
{
	m_FallSettingInfo = info;
}

const FallSettingInfo & ModelDataManager::GetFallSettingInfo() const
{
	return m_FallSettingInfo;
}

void ModelDataManager::SetUserInfo(const UserInfo& info)
{
	m_UserInfo = info;
}

void ModelDataManager::SetJudgementPropertyInfo(const JudgementPropertyInfo & info)
{
	m_JudgementPropertyInfo = info;
}

const JudgementPropertyInfo & ModelDataManager::GetJudgementPropertyInfo() const
{
	return m_JudgementPropertyInfo;
}

// 重置数据
void ModelDataManager::Reset() 
{
	m_ModelGeometryInfo = ModelGeometryInfo();
	m_ModelMeshInfo = ModelMeshInfo();

	m_SteelPropertyInfo = SteelPropertyInfo();
	m_PropellantPropertyInfo = PropellantPropertyInfo();
	m_CalculationPropertyInfo = CalculationPropertyInfo();
}


void ModelDataManager::SetFallStressResult(const StressResult& result)
{
	m_FallStressResult = result;
}

const StressResult& ModelDataManager::GetFallStressResult() const
{
	return m_FallStressResult;
}

void ModelDataManager::SetFallStrainResult(const StrainResult& result)
{
	m_FallStrainResult = result;
}

const StrainResult& ModelDataManager::GetFallStrainResult() const
{
	return m_FallStrainResult;
}

void ModelDataManager::SetFallTemperatureResult(const TemperatureResult& result)
{
	m_FallTemperatureResult = result;
}

const TemperatureResult& ModelDataManager::GetFallTemperatureResult() const
{
	return m_FallTemperatureResult;
}

void ModelDataManager::SetFallOverpressureResult(const OverpressureResult& result)
{
	m_FallOverpressureResult = result;
}

const OverpressureResult& ModelDataManager::GetFallOverpressureResult() const
{
	return m_FallOverpressureResult;
}

void ModelDataManager::SetFastCombustionTemperatureResult(const TemperatureResult& result)
{
	m_FastCombustionTemperatureResult = result;
}

const TemperatureResult& ModelDataManager::GetFastCombustionTemperatureResult() const
{
	return m_FastCombustionTemperatureResult;
}

void ModelDataManager::SetSlowCombustionTemperatureResult(const TemperatureResult& result)
{
	m_SlowCombustionTemperatureResult = result;
}

const TemperatureResult& ModelDataManager::GetSlowCombustionTemperatureResult() const
{
	return m_SlowCombustionTemperatureResult;
}

void ModelDataManager::SetShootStressResult(const StressResult& result)
{
	m_ShootStressResult = result;
}

const StressResult& ModelDataManager::GetShootStressResult() const
{
	return m_ShootStressResult;
}

void ModelDataManager::SetShootStrainResult(const StrainResult& result)
{
	m_ShootStrainResult = result;
}

const StrainResult& ModelDataManager::GetShootStrainResult() const
{
	return m_ShootStrainResult;
}

void ModelDataManager::SetShootTemperatureResult(const TemperatureResult& result)
{
	m_ShootTemperatureResult = result;
}

const TemperatureResult& ModelDataManager::GetShootTemperatureResult() const
{
	return m_ShootTemperatureResult;
}

void ModelDataManager::SetShootOverpressureResult(const OverpressureResult& result)
{
	m_ShootOverpressureResult = result;
}

const OverpressureResult& ModelDataManager::GetShootOverpressureResult() const
{
	return m_ShootOverpressureResult;
}

void ModelDataManager::SetJetImpactStressResult(const StressResult& result)
{
	m_JetImpactStressResult = result;
}

const StressResult& ModelDataManager::GetJetImpactStressResult() const
{
	return m_JetImpactStressResult;
}

void ModelDataManager::SetJetImpactStrainResult(const StrainResult& result)
{
	m_JetImpactStrainResult = result;
}

const StrainResult& ModelDataManager::GetJetImpactStrainResult() const
{
	return m_JetImpactStrainResult;
}

void ModelDataManager::SetJetImpactTemperatureResult(const TemperatureResult& result)
{
	m_JetImpactTemperatureResult = result;
}

const TemperatureResult& ModelDataManager::GetJetImpactTemperatureResult() const
{
	return m_JetImpactTemperatureResult;
}

void ModelDataManager::SetJetImpactOverpressureResult(const OverpressureResult& result)
{
	m_JetImpactOverpressureResult = result;
}

const OverpressureResult& ModelDataManager::GetJetImpactOverpressureResult() const
{
	return m_JetImpactOverpressureResult;
}

void ModelDataManager::SetFragmentationImpactStressResult(const StressResult& result)
{
	m_FragmentationImpactStressResult = result;
}

const StressResult& ModelDataManager::GetFragmentationImpactStressResult() const
{
	return m_FragmentationImpactStressResult;
}

void ModelDataManager::SetFragmentationImpactStrainResult(const StrainResult& result)
{
	m_FragmentationImpactStrainResult = result;
}

const StrainResult& ModelDataManager::GetFragmentationImpactStrainResult() const
{
	return m_FragmentationImpactStrainResult;
}

void ModelDataManager::SetFragmentationImpactTemperatureResult(const TemperatureResult& result)
{
	m_FragmentationImpactTemperatureResult = result;
}

const TemperatureResult& ModelDataManager::GetFragmentationImpactTemperatureResult() const
{
	return m_FragmentationImpactTemperatureResult;
}

void ModelDataManager::SetFragmentationImpactOverpressureResult(const OverpressureResult& result)
{
	m_FragmentationImpactOverpressureResult = result;
}

const OverpressureResult& ModelDataManager::GetFragmentationImpactOverpressureResult() const
{
	return m_FragmentationImpactOverpressureResult;
}

void ModelDataManager::SetExplosiveBlastStressResult(const StressResult& result)
{
	m_ExplosiveBlastStressResult = result;
}

const StressResult& ModelDataManager::GetExplosiveBlastStressResult() const
{
	return m_ExplosiveBlastStressResult;
}

void ModelDataManager::SetExplosiveBlastStrainResult(const StrainResult& result)
{
	m_ExplosiveBlastStrainResult = result;
}

const StrainResult& ModelDataManager::GetExplosiveBlastStrainResult() const
{
	return m_ExplosiveBlastStrainResult;
}

void ModelDataManager::SetExplosiveBlastTemperatureResult(const TemperatureResult& result)
{
	m_ExplosiveBlastTemperatureResult = result;
}

const TemperatureResult& ModelDataManager::GetExplosiveBlastTemperatureResult() const
{
	return m_ExplosiveBlastTemperatureResult;
}

void ModelDataManager::SetExplosiveBlastOverpressureResult(const OverpressureResult& result)
{
	m_ExplosiveBlastOverpressureResult = result;
}

const OverpressureResult& ModelDataManager::GetExplosiveBlastOverpressureResult() const
{
	return m_ExplosiveBlastOverpressureResult;
}

void ModelDataManager::SetSacrificeExplosionStressResult(const StressResult& result)
{
	m_SacrificeExplosionStressResult = result;
}

const StressResult& ModelDataManager::GetSacrificeExplosionStressResult() const
{
	return m_SacrificeExplosionStressResult;
}

void ModelDataManager::SetSacrificeExplosionStrainResult(const StrainResult& result)
{
	m_SacrificeExplosionStrainResult = result;
}

const StrainResult& ModelDataManager::GetSacrificeExplosionStrainResult() const
{
	return m_SacrificeExplosionStrainResult;
}

void ModelDataManager::SetSacrificeExplosionTemperatureResult(const TemperatureResult& result)
{
	m_SacrificeExplosionTemperatureResult = result;
}

const TemperatureResult& ModelDataManager::GetSacrificeExplosionTemperatureResult() const
{
	return m_SacrificeExplosionTemperatureResult;
}

void ModelDataManager::SetSacrificeExplosionOverpressureResult(const OverpressureResult& result)
{
	m_SacrificeExplosionOverpressureResult = result;
}

const OverpressureResult& ModelDataManager::GetSacrificeExplosionOverpressureResult() const
{
	return m_SacrificeExplosionOverpressureResult;
}

void ModelDataManager::SetPointResult(const PointResult& result)
{
	m_pointResult = result;
}
const PointResult& ModelDataManager::GetPointResult() const
{
	return m_pointResult;
}