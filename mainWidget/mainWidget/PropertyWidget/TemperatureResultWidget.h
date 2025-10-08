#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class TemperatureResultWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit TemperatureResultWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
};