#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"
#include "ModelDataManager.h"

class SlowCombustionPropertyWidget : public BasePropertyWidget
{
    Q_OBJECT
public:
    explicit SlowCombustionPropertyWidget(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
	QString m_finalTemperature = "200";
};