#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class ProjectPropertyWidge : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit ProjectPropertyWidge(QWidget* parent = nullptr);

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;
};