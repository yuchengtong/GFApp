#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class OverpressureResultWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit OverpressureResultWidget(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
};