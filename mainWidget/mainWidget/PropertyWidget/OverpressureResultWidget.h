#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class OverpressureResultWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit OverpressureResultWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
};