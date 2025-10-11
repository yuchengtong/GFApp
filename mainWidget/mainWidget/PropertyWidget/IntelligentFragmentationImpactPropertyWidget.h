#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class IntelligentFragmentationImpactPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit IntelligentFragmentationImpactPropertyWidget(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
};