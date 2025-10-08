#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class PropellantPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit PropellantPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;


private slots: 
	void showTableDialog();

private:
	QTableWidget* m_tableWidget = nullptr;
};