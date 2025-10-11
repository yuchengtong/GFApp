#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class IntelligentFallPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit IntelligentFallPropertyWidget(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
};