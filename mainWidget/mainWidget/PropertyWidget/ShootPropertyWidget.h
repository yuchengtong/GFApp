#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class ShootPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit ShootPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;

};