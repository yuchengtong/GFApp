#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class IntelligentJetImpactPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit IntelligentJetImpactPropertyWidget(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
};