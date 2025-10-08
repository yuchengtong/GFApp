#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class FallPropertyWidget : public BasePropertyWidget
{
    Q_OBJECT
public:
    explicit FallPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
};