#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class OutheatPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit OutheatPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private slots:
	void showTableDialog();

private:
	QTableWidget* m_tableWidget = nullptr;
};