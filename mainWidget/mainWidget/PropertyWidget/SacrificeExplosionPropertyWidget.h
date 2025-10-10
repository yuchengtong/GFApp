#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class SacrificeExplosionPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit SacrificeExplosionPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;

};