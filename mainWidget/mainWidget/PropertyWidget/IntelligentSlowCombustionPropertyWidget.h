#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class IntelligentSlowCombustionPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit IntelligentSlowCombustionPropertyWidget(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
};