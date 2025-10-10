#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class StrainResultWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit StrainResultWidget(QWidget* parent = nullptr);

	void updateData(double shellMaxValue, double shellMinValue, double shellAvgValue, double shellStandardValue, double maxValue, double minValue, double avgValue, double standardValue);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }
private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
};