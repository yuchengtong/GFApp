#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class TemperatureResultWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit TemperatureResultWidget(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

	void updateData(double shellMaxValue, double shellMinValue, double shellAvgValue, double shellStandardValue, double maxValue, double minValue, double avgValue, double standardValue,
		double outheatMaxValue, double outheatMinValue, double outheatAvgValue, double outheatStandardValue, double insulatingheatMaxValue, double insulatingheatMinValue, double insulatingheatAvgValue, double insulatingheatStandardValue);

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
};