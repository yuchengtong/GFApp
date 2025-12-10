#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"
#include "ModelDataManager.h"

class SacrificeExplosionPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit SacrificeExplosionPropertyWidget(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;

};