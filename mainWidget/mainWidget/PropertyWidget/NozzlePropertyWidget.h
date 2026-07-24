#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class NozzlePropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit NozzlePropertyWidget(QWidget* parent = nullptr);

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private slots:
	void showTableDialog();

private:
	QTableWidget* m_tableWidget = nullptr;
};