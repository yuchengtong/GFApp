#pragma once
#include <QWidget>
#include <QTableWidget>
#include "BasePropertyWidget.h"

class JudgmentPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit JudgmentPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private slots:
	void showTableDialog();

private:
	QTableWidget* m_tableWidget = nullptr;
};