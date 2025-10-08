#pragma once
#include <qwidget.h>

#include "GFTreeModelWidget.h"
#include "OccView.h"
#include "GFLogWidget.h"

class GFParamAnalyWidget :public QWidget
{
	Q_OBJECT
public:
	GFParamAnalyWidget(QWidget*parent = nullptr);
	~GFParamAnalyWidget();

	OccView* GetOccView() { return m_OccView; }

	GFLogWidget*GetLogWidget() { return m_LogWidget; }

private:
	OccView*m_OccView = nullptr;

	GFLogWidget*m_LogWidget = nullptr;
};

