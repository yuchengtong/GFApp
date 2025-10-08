#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class SlowCombustionPropertyWidget : public BasePropertyWidget
{
    Q_OBJECT
public:
    explicit SlowCombustionPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;

};