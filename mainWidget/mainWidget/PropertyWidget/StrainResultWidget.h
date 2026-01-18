#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class StrainResultWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit StrainResultWidget(QWidget* parent = nullptr);

	void updateData(double shellMaxValue, double shellMinValue, double shellAvgValue, double shellStandardValue, double maxValue, double minValue, double avgValue, double standardValue,
		double outheatMaxValue, double outheatMinValue, double outheatAvgValue, double outheatStandardValue, double insulatingheatMaxValue, double insulatingheatMinValue, double insulatingheatAvgValue, double insulatingheatStandardValue);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }
private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
};