#include "CalculateDataManager.h"
#include <fstream>
#include <iostream>
#include <sstream>

// 初始化静态成员（单例实例）
CalculateDataManager* CalculateDataManager::m_Instance = nullptr;

CalculateDataManager::CalculateDataManager()
{
	
}

// 获取单例实例
CalculateDataManager* CalculateDataManager::GetInstance() {
	if (m_Instance == nullptr) {
		m_Instance = new CalculateDataManager();
	}
	return m_Instance;
}


const CalculateDataInfo& CalculateDataManager::GetCalculateDataInfo() const
{
	return m_CalculateDataInfo;
}
