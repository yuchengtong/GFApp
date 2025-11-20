#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class ShootPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit ShootPropertyWidget(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
	QString m_impactVelocityValue = "300";
};