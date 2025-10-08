#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class SteelPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit SteelPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private slots:
	void showTableDialog();

private:
	QTableWidget* m_tableWidget = nullptr;
};