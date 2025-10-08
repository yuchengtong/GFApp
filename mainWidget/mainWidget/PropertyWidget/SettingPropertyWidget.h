#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class SettingPropertyWidget : public BasePropertyWidget
{
    Q_OBJECT
public:
    explicit SettingPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

};