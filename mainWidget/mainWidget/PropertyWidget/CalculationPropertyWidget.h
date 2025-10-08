#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class CalculationPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit CalculationPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;


private:
	QTableWidget* m_tableWidget = nullptr;
};