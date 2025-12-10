#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"
#include "ModelDataManager.h"

class JetImpactPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit JetImpactPropertyWidget(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;

};