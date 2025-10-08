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
