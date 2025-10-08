#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class InsulatingheatPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit InsulatingheatPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private slots:
	void showTableDialog();

private:
	QTableWidget* m_tableWidget = nullptr;
};