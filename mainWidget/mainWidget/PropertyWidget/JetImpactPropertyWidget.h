#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class JetImpactPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit JetImpactPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;

};