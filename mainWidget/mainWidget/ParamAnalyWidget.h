#pragma once

#include <QWidget>
#include "ui_ParamAnalyWidget.h"
#include "OccView.h"

class ParamAnalyWidget : public QWidget
{
	Q_OBJECT

public:
	ParamAnalyWidget(QWidget *parent = nullptr);
	~ParamAnalyWidget();

	OccView* GetOccView() { return ui.widget; }

private:
	Ui::ParamAnalyWidgetClass ui;

};
