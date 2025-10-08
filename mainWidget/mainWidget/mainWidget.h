#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mainWidget.h"
#include <QAction>
#include <QTabWidget>
#include <QLabel>
#include <QTimer>
#include <Windows.h>

QT_BEGIN_NAMESPACE
namespace Ui { class mainWidgetClass; };
QT_END_NAMESPACE

class mainWidget : public QMainWindow
{
    Q_OBJECT

public:
    mainWidget(QWidget *parent = nullptr);
    ~mainWidget();

	void deleteWidget(QLayout * layout);

	void changeChartView(QMap<int, QWidget*> chartWidMap);


	void refreshMemoryUsage(QLabel *m_statusLabel);

	void getMemoryUsage(QLabel *m_statusLabel);

private:
    Ui::mainWidgetClass *ui;

	QAction* m_ImportModelWidAct = nullptr;
	QAction* m_DataBaseWidAct = nullptr;
	QAction* m_ParamAnalyWidAct = nullptr;
	QAction* m_IntelligentAnalyWidAct = nullptr;
	QAction* m_AuxiliaryAnalyWidAct = nullptr;
	QAction* m_AnalyEvalWidAct = nullptr;
	QAction* m_HelpAct = nullptr;
	
	QTabWidget*m_TabWidget = nullptr;


	QAction* m_ImportAct = nullptr;
	QAction* m_SaveAct = nullptr;
	QAction* m_SaveAsAct = nullptr;
	QAction*m_Export = nullptr;
	QAction* m_SettingAct = nullptr;
	QAction* XAct = nullptr;
	QAction* YAct = nullptr;
	QAction* ZAct = nullptr;
	QAction* _XAct = nullptr;
	QAction* _YAct = nullptr;
	QAction* _ZAct = nullptr;
	//QAction* m_IntelligentAnalyWidAct = nullptr;
	//QAction* m_AuxiliaryAnalyWidAct = nullptr;
	//QAction* m_AnalyEvalWidAct = nullptr;
	//QAction* m_HelpAct = nullptr;


	QTimer *timer;

	// 保存上一次的CPU时间
	FILETIME prevIdleTime;
	FILETIME prevKernelTime;
	FILETIME prevUserTime;


};
