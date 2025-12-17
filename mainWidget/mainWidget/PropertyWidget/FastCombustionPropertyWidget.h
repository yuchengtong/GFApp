#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"
#include "ModelDataManager.h"

class FastCombustionPropertyWidget : public BasePropertyWidget
{
    Q_OBJECT
public:
    explicit FastCombustionPropertyWidget(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
	QString m_averageTemperature = "300";
};