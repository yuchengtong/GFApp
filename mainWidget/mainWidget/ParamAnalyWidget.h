#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QStackedWidget>

#include "ParamAnalyTreeWidget.h"


class ParamAnalyWidget : public QWidget
{
	Q_OBJECT

public:
	ParamAnalyWidget(QWidget* parent = nullptr);
	~ParamAnalyWidget();

private slots:
	void onTreeItemClicked(const QString& itemData);

private:

	ParamAnalyTreeWidget* m_treeModelWidget = nullptr;
	QTableWidget* m_tableWidget = nullptr;

	

};
