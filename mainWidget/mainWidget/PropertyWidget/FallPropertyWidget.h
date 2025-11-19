#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class FallPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit FallPropertyWidget(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;

	QString m_highValue = "20";
};