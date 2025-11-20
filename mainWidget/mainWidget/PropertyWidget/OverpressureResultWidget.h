#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class OverpressureResultWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit OverpressureResultWidget(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

	void updateData(double shellMaxValue, double shellMinValue, double shellAvgValue, double shellStandardValue, double maxValue, double minValue, double avgValue, double standardValue);


private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
};