#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class ResultsPropertyWidget : public BasePropertyWidget
{
    Q_OBJECT
public:
    explicit ResultsPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

};