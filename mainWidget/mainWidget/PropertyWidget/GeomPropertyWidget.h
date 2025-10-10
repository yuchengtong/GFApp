#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"
#include "ModelDataManager.h"

class GeomPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit GeomPropertyWidget(QWidget* parent = nullptr);

	void UpdataPropertyInfo();

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;

};