#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mainWidget.h"
#include <QAction>
#include <QTabWidget>
#include <QLabel>
#include <QTimer>
#include <Windows.h>

#include "GFImportModelWidget.h"
#include "DatabaseWidget.h"
#include "AuxiliaryAnalysisWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class mainWidgetClass; };
QT_END_NAMESPACE

class mainWidget : public QMainWindow
{
    Q_OBJECT

public:
    mainWidget(QWidget *parent = nullptr);
    ~mainWidget();

	void bindConnect();

	void deleteWidget(QLayout * layout);

	void changeChartView(QMap<int, QWidget*> chartWidMap);


	void refreshMemoryUsage(QLabel *m_statusLabel);

	void getMemoryUsage(QLabel *m_statusLabel);

	QTabWidget* getTabWidget() { return m_TabWidget; }

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


	QPushButton* m_importBtn = nullptr;
	QPushButton* m_saveBtn = nullptr;
	QPushButton* m_saveAsBtn = nullptr;
	QPushButton* m_exportBtn = nullptr;

	QPushButton* m_moveBtn = nullptr;
	QPushButton* m_rotateBtn = nullptr;
	QPushButton* m_zoomBtn = nullptr;
	QPushButton* m_fitAllBtn = nullptr;
	QPushButton* m_resetBtn = nullptr;

	QPushButton* m_XBtn = nullptr;
	QPushButton* m_YBtn = nullptr;
	QPushButton* m_ZBtn = nullptr;
	QPushButton* m__XBtn = nullptr;
	QPushButton* m__YBtn = nullptr;
	QPushButton* m__ZBtn = nullptr;

	GFImportModelWidget* m_importModelWid = nullptr;
	DatabaseWidget* m_dataBaseWid = nullptr;
	AuxiliaryAnalysisWidget* m_auxiliaryAnalysisWid = nullptr;


	QTimer* timer = nullptr; // 定时器指针（初始化为空）
	// 存储上一次采样的系统时间（类成员变量，跨函数保留基准）
	FILETIME prevIdleTime = { 0 };
	FILETIME prevKernelTime = { 0 };
	FILETIME prevUserTime = { 0 };
	bool isFirstSample = true; // 标记是否为首次采样

	// 辅助函数：将 FILETIME 转换为 64 位整数（100纳秒为单位）
	ULONGLONG fileTimeToULL(const FILETIME& ft);
};
