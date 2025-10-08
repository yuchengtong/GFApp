#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class MeshPropertyWidget : public BasePropertyWidget
{
    Q_OBJECT
public:
    explicit MeshPropertyWidget(QWidget* parent = nullptr);

	void UpdataPropertyInfo();

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;

};