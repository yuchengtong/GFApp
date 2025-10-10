#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class ProjectPropertyWidge : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit ProjectPropertyWidge(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
};