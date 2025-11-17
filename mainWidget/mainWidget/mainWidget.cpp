#pragma execution_character_set("utf-8")
#include "mainWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QGraphicsView>
#include <QBrush>
#include <QLinearGradient>
#include <QPen>
#include <QtCharts>
#include <QLineSeries>
#include <QBarSeries>
#include <QtCharts\qchartview.h>
#include <QFileDialog>
#include "xlsxdocument.h"

#include <AIS_Shape.hxx>
#include <STEPControl_Reader.hxx>
#include <Prs3d_LineAspect.hxx>
#include <V3d_View.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepMesh_Context.hxx>  
#include <BRepBndLib.hxx>
#include <StlAPI_Reader.hxx>

#include "GFImportModelWidget.h"
#include "GFLogWidget.h"
#include "DatabaseWidget.h"
#include "ParamAnalyWidget.h"
#include "AuxiliaryAnalysisWidget.h"
#include "GFParamAnalyWidget.h"
#include "OccView.h"
#include "GFTreeModelWidget.h"
#include "IntelligentAnalyWidget.h"



mainWidget::mainWidget(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::mainWidgetClass())
{
	setWindowIcon(QIcon(":/src/engine.svg"));
	setStyleSheet("QPushButton {"
                           "background-color:  rgba(0, 0, 0, 0);"
                           "}"
                           "QPushButton:hover {"
                           "background-color: white;"
		"}");

    ui->setupUi(this);
	setWindowTitle("固体发动机安全性分析与评估系统");
	//showMaximized();
	//setMinimumSize(1050, 800);

	// 状态栏
	QStatusBar* statusbar = this->statusBar();
	this->setStatusBar(statusbar);
	QLabel *m_statusLabel = new QLabel("内存使用：0%，CPU使用：0%");
	statusbar->addPermanentWidget(m_statusLabel);
	refreshMemoryUsage(m_statusLabel);

	m_ImportModelWidAct = new QAction("安全性特性参数分析", ui->menuBar);
	m_DataBaseWidAct = new QAction("数据库", ui->menuBar);
	//m_ParamAnalyWidAct = new QAction("安全性特性参数分析", ui->menuBar);
	m_IntelligentAnalyWidAct= new QAction("数据智能分析", ui->menuBar);
	m_AnalyEvalWidAct = new QAction("安全性分析与评估", ui->menuBar);
	m_AuxiliaryAnalyWidAct = new QAction("安全性指标预计、权衡和辅助分析", ui->menuBar);
	m_HelpAct = new QAction("帮助", ui->menuBar);

	ui->menuBar->addAction(m_DataBaseWidAct);
	ui->menuBar->addAction(m_ImportModelWidAct);
	//ui->menuBar->addAction(m_ParamAnalyWidAct);
	ui->menuBar->addAction(m_IntelligentAnalyWidAct);
	ui->menuBar->addAction(m_AnalyEvalWidAct);
	ui->menuBar->addAction(m_AuxiliaryAnalyWidAct);
	ui->menuBar->addAction(m_HelpAct);


	ui->mainToolBar->setMovable(false);
	ui->mainToolBar->setFloatable(false);
//////////////////////////////////////////////////////////ToolBar
	auto ImportBtn = new QPushButton();
	auto SaveBtn = new QPushButton();
	auto SaveAsBtn = new QPushButton();
	auto ExportBtn= new QPushButton();
	ImportBtn->setIcon(QIcon(":/src/Import.svg"));
	SaveBtn->setIcon(QIcon(":/src/Save.svg"));
	SaveAsBtn->setIcon(QIcon(":/src/Save_as.svg"));
	ExportBtn->setIcon(QIcon(":/src/Export.svg"));
	ImportBtn->setFixedSize(32,32);
	SaveBtn->setFixedSize(32, 32);
	SaveAsBtn->setFixedSize(32, 32);
	ExportBtn->setFixedSize(32, 32);

	auto ImportLabel = new QLabel("导入文件");
	auto SaveLabel = new QLabel("保存文件");
	auto SaveAsLabel= new QLabel("另存为...");
	auto ExportLabel= new QLabel("导出文件");
	auto bottomTitleLab1 = new QLabel("几何");

	auto geomWidget = new QWidget();
	geomWidget->setFixedWidth(172);
	auto hLayout1 = new QHBoxLayout();
	auto hLayout2 = new QHBoxLayout();
	auto hLayout3 = new QHBoxLayout();
	hLayout1->addWidget(ImportBtn);
	hLayout1->addWidget(ImportLabel);
	hLayout1->setSpacing(0);
	hLayout1->addWidget(SaveAsBtn);
	hLayout1->addWidget(SaveAsLabel);
	hLayout1->setContentsMargins(0, 0, 0, 0);
	hLayout2->addWidget(SaveBtn);
	hLayout2->addWidget(SaveLabel);
	hLayout2->setSpacing(0);
	hLayout2->addWidget(ExportBtn);
	hLayout2->addWidget(ExportLabel);
	hLayout2->setContentsMargins(0, 0, 0, 0);
	hLayout3->addStretch();
	hLayout3->addWidget(bottomTitleLab1);
	hLayout3->addStretch();
	hLayout3->setContentsMargins(0, 0, 0, 0);
	auto vLayout = new QVBoxLayout();
	vLayout->addLayout(hLayout1);
	vLayout->addLayout(hLayout2);
	vLayout->addLayout(hLayout3);
	vLayout->setContentsMargins(0, 0, 0, 0);
	geomWidget->setLayout(vLayout);



	auto MoveBtn = new QPushButton();
	auto RotateBtn = new QPushButton();
	auto ZoomBtn = new QPushButton();
	auto FitAllBtn = new QPushButton();
	auto ResetBtn= new QPushButton();
	auto nullBtn= new QPushButton();
	nullBtn->setEnabled(false);
	MoveBtn->setIcon(QIcon(":/src/Move.svg"));
	RotateBtn->setIcon(QIcon(":/src/Rotate.svg"));
	ZoomBtn->setIcon(QIcon(":/src/Zoom.png"));
	FitAllBtn->setIcon(QIcon(":/src/FitAll.png"));
	ResetBtn->setIcon(QIcon(":/src/Reset.svg"));
	MoveBtn->setFixedSize(32, 32);
	RotateBtn->setFixedSize(32, 32);
	ZoomBtn->setFixedSize(32, 32);
	FitAllBtn->setFixedSize(32, 32);
	ResetBtn->setFixedSize(32, 32);
	auto MoveLabel = new QLabel("移动");
	auto RotateLabel = new QLabel("旋转");
	auto ZoomLabel = new QLabel("缩放");
	auto FitAllLabel = new QLabel("聚焦");
	auto ResetLabel= new QLabel("重置");
	auto nullLabel = new QLabel("");

	auto bottomTitleLab_o = new QLabel("操作");

	auto operationWidget = new QWidget();
	operationWidget->setFixedWidth(184);
	auto hLayout_o1 = new QHBoxLayout();
	auto hLayout_o2 = new QHBoxLayout();
	auto hLayout_o3 = new QHBoxLayout();
	hLayout_o1->addWidget(MoveBtn);
	hLayout_o1->addWidget(MoveLabel);
	hLayout_o1->setSpacing(0);
	hLayout_o1->addWidget(ZoomBtn);
	hLayout_o1->addWidget(ZoomLabel);
	hLayout_o1->setSpacing(0);
	hLayout_o1->addWidget(ResetBtn);
	hLayout_o1->addWidget(ResetLabel);
	hLayout_o1->setContentsMargins(0, 0, 0, 0);
	hLayout_o2->addWidget(RotateBtn);
	hLayout_o2->addWidget(RotateLabel);
	hLayout_o2->setSpacing(0);
	hLayout_o2->addWidget(FitAllBtn);
	hLayout_o2->addWidget(FitAllLabel);
	hLayout_o2->setSpacing(0);
	hLayout_o2->addWidget(nullBtn);
	hLayout_o2->addWidget(nullLabel);

	hLayout_o2->setContentsMargins(0, 0, 0, 0);
	hLayout_o3->addStretch();
	hLayout_o3->addWidget(bottomTitleLab_o);
	hLayout_o3->addStretch();
	hLayout_o3->setContentsMargins(0, 0, 0, 0);
	auto vLayout_o = new QVBoxLayout();
	vLayout_o->addLayout(hLayout_o1);
	vLayout_o->addLayout(hLayout_o2);
	vLayout_o->addLayout(hLayout_o3);
	vLayout_o->setContentsMargins(0, 0, 0, 0);
	operationWidget->setLayout(vLayout_o);

	auto XBtn = new QPushButton();
	auto YBtn = new QPushButton();
	auto ZBtn = new QPushButton();
	auto _XBtn = new QPushButton();
	auto _YBtn = new QPushButton();
	auto _ZBtn = new QPushButton();
	XBtn->setFixedSize(32, 32);
	YBtn->setFixedSize(32, 32);
	ZBtn->setFixedSize(32, 32);
	_XBtn->setFixedSize(32, 32);
	_YBtn->setFixedSize(32, 32);
	_ZBtn->setFixedSize(32, 32);

	XBtn->setIcon(QIcon(":/src/View all From +X.png"));
	YBtn->setIcon(QIcon(":/src/View all From +Y.png"));
	ZBtn->setIcon(QIcon(":/src/View all From +Z.png"));
	_XBtn->setIcon(QIcon(":/src/View all From -X.png"));
	_YBtn->setIcon(QIcon(":/src/View all From -Y.png"));
	_ZBtn->setIcon(QIcon(":/src/View all From -Z.png"));
	auto XLabel = new QLabel("X轴方向");
	auto YLabel = new QLabel("Y轴方向");
	auto ZLabel = new QLabel("Z轴方向");
	auto _XLabel = new QLabel("负X轴方向");
	auto _YLabel = new QLabel("负Y轴方向");
	auto _ZLabel = new QLabel("负Z轴方向");
	auto bottomTitleLab2 = new QLabel("视图");

	auto viewWidget = new QWidget();
	viewWidget->setFixedWidth(265);
	auto hLayout_v1 = new QHBoxLayout();
	auto hLayout_v2 = new QHBoxLayout();
	auto hLayout_v3 = new QHBoxLayout();
	hLayout_v1->addWidget(XBtn);
	hLayout_v1->addWidget(XLabel);
	hLayout_v1->setSpacing(0);
	hLayout_v1->addWidget(YBtn);
	hLayout_v1->addWidget(YLabel);
	hLayout_v1->setSpacing(0);
	hLayout_v1->addWidget(ZBtn);
	hLayout_v1->addWidget(ZLabel);
	hLayout_v1->setContentsMargins(0,0,0,0);
	hLayout_v2->addWidget(_XBtn);
	hLayout_v2->addWidget(_XLabel);
	hLayout_v2->setSpacing(0);
	hLayout_v2->addWidget(_YBtn);
	hLayout_v2->addWidget(_YLabel);
	hLayout_v2->setSpacing(0);
	hLayout_v2->addWidget(_ZBtn);
	hLayout_v2->addWidget(_ZLabel);
	hLayout_v2->setContentsMargins(0, 0, 0, 0);
	hLayout_v3->addStretch();
	hLayout_v3->addWidget(bottomTitleLab2);
	hLayout_v3->addStretch();
	auto vLayout_v = new QVBoxLayout();
	vLayout_v->addLayout(hLayout_v1);
	vLayout_v->addLayout(hLayout_v2);
	vLayout_v->addLayout(hLayout_v3);
	vLayout_v->setContentsMargins(0, 0, 0, 0);
	viewWidget->setLayout(vLayout_v);


	auto SettingBtn = new QPushButton();
	SettingBtn->setFixedSize(67, 67);
	SettingBtn->setIcon(QIcon(":/src/Setting.svg"));
	SettingBtn->setIconSize(QSize(50, 50));
	auto SettingLabel = new QLabel("设置");

	auto settingWidget = new QWidget();
	settingWidget->setFixedWidth(69);
	auto hLayout_s = new QHBoxLayout();
	hLayout_s->addStretch();
	hLayout_s->addWidget(SettingLabel);
	hLayout_s->addStretch();

	auto vLayout_s = new QVBoxLayout();
	vLayout_s->addWidget(SettingBtn);
	vLayout_s->addLayout(hLayout_s);
	vLayout_s->setContentsMargins(0, 0, 0, 0);
	settingWidget->setLayout(vLayout_s);

	ui->mainToolBar->addWidget(geomWidget);
	ui->mainToolBar->addSeparator();
	ui->mainToolBar->addWidget(operationWidget);
	ui->mainToolBar->addSeparator();
	ui->mainToolBar->addWidget(viewWidget);
	ui->mainToolBar->addSeparator();
	ui->mainToolBar->addWidget(settingWidget);
	ui->mainToolBar->addSeparator();


	m_TabWidget = new QTabWidget(this);

	GFImportModelWidget*importModelWid = new GFImportModelWidget(m_TabWidget);
	{
	}

	DatabaseWidget*dataBaseWid = new DatabaseWidget(m_TabWidget);
	{
	}

	/*ParamAnalyWidget*paramAnalysisWid = new ParamAnalyWidget(m_TabWidget);
	{
	}*/

	IntelligentAnalyWidget*IntelligenAnalysisWid = new IntelligentAnalyWidget(m_TabWidget);
	{
	}
	AuxiliaryAnalysisWidget*auxiliaryAnalysisWid = new AuxiliaryAnalysisWidget(m_TabWidget);
	{
	}
	ParamAnalyWidget*analysisEvaluationWid = new ParamAnalyWidget(m_TabWidget);
	{
	}


	m_TabWidget->addTab(importModelWid, "importModelWid");
	m_TabWidget->addTab(dataBaseWid, "dataBaseWid");
	//m_TabWidget->addTab(paramAnalysisWid, "paramAnalysisWid");
	m_TabWidget->addTab(IntelligenAnalysisWid, "IntelligenAnalysisWid");
	m_TabWidget->addTab(analysisEvaluationWid, "analysisEvaluationWid");
	m_TabWidget->addTab(auxiliaryAnalysisWid, "auxiliaryAnalysisWid");
	m_TabWidget->tabBar()->setVisible(false);


	setCentralWidget(m_TabWidget);


	QObject::connect(m_ImportModelWidAct, &QAction::triggered, [=]() {
		m_TabWidget->setCurrentIndex(0);
		// 显示工具栏
		ui->mainToolBar->setVisible(true);
	});
		
	QObject::connect(m_DataBaseWidAct, &QAction::triggered, [=]() {
		m_TabWidget->setCurrentIndex(1);
		// 隐藏工具栏
		ui->mainToolBar->setVisible(false);
		// 非admin用户，隐藏用户数据库
		QTreeWidget* treeWidget = dataBaseWid->getQTreeWid();
		auto ins = ModelDataManager::GetInstance();
		UserInfo info = ins->GetUserInfo();
		if (info.username != "admin")
		{
			QTreeWidgetItem *child;
			int size = treeWidget->topLevelItemCount();
			for (int i = 0; i < size; i++)
			{
				child = treeWidget->topLevelItem(i);
				if (child->text(0).contains("用户数据库"))
				{
					child->setHidden(true);
				}
			}
		}
	});

	//QObject::connect(m_ParamAnalyWidAct, &QAction::triggered, [=]() {
	//	m_TabWidget->setCurrentIndex(2);
	//	// 显示工具栏
	//	ui->mainToolBar->setVisible(true);
	//	auto occView2 = paramAnalysisWid->GetOccView();
	//	auto myview=occView2->getView();
	//	myview->MustBeResized();
	//});

	QObject::connect(m_IntelligentAnalyWidAct, &QAction::triggered, [=]() {
		m_TabWidget->setCurrentIndex(2);
		// 显示工具栏
		ui->mainToolBar->setVisible(false);
		/*auto occView3 = IntelligenAnalysisWid->GetOccView();
		auto myview = occView3->getView();
		myview->MustBeResized();*/
	});

	QObject::connect(m_AnalyEvalWidAct, &QAction::triggered, [=]() {
		m_TabWidget->setCurrentIndex(3);
		// 显示工具栏
		ui->mainToolBar->setVisible(true);
		/*auto occView5 = analysisEvaluationWid->GetOccView();
		auto myview = occView5->getView();
		myview->MustBeResized();*/
		});

	QObject::connect(m_AuxiliaryAnalyWidAct, &QAction::triggered, [=]() {
		m_TabWidget->setCurrentIndex(4);
		// 显示工具栏
		ui->mainToolBar->setVisible(false);
		// 更新echart数据
		auxiliaryAnalysisWid->updateAllData();
	});

	


	QObject::connect(m_HelpAct, &QAction::triggered, [=]() {
		QString aboutText = "**[软件名称] - 固体发动机安全性分析与评估系统**\n\n"
			"**版本信息**:\n"
			"- **软件版本**: [v1.0.0]\n"
			"**版权信息**:\n"
			"- **版权所有**: [版权所有者或公司名称]\n"
			"- **版权声明**:\n"
			"本软件受版权法和国际条约的保护。未经版权所有者的明确书面许可，严禁对本软件进行任何形式的复制、分发、修改或逆向工程。本软件的部分组件可能使用了第三方的开源软件，这些软件遵循各自的开源许可协议，具体信息可在软件的 LICENSE 文件或相应组件的文档中找到。\n\n"
			"**开发团队**:\n"
			"- **开发团队**: [开发团队或组织的名称]\n"
			"**联系我们**:\n"
			"如果您对本软件有任何问题、建议或反馈，请随时联系我们:\n"
			"- **电子邮件**: [联系邮箱]\n"
			"- **网站**: [官方网站]";
		QMessageBox::about(nullptr, "固体发动机安全性分析与评估系统", aboutText);
	});
	


	QObject::connect(ImportBtn, &QPushButton::clicked, [this,importModelWid]() {
		if (m_TabWidget->currentIndex() == 0) {
			// 打开文件对话框
			QString filePath = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(),
				"STEP Files (*.stp *.step);;IGES Files (*.iges *.igs);;All Files (*.*)");

			if (filePath.isEmpty()) 
				return;
			QDateTime currentTime = QDateTime::currentDateTime();
			QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
			auto logWidget = importModelWid->GetLogWidget();
			auto textEdit = logWidget->GetTextEdit();
			textEdit->appendPlainText(timeStr + "[信息]>开始导入几何模型");
			logWidget->update();

			TopoDS_Shape aShape;

			bool loadSuccess = false;

			ModelGeometryInfo info;

			// 根据文件扩展名选择适当的读取器
			if (filePath.endsWith(".stp", Qt::CaseInsensitive) || filePath.endsWith(".step", Qt::CaseInsensitive))
			{
				STEPControl_Reader aReader_Step;
				if (aReader_Step.ReadFile(filePath.toStdString().c_str()) == IFSelect_RetDone) {
					aReader_Step.PrintCheckLoad(Standard_False, IFSelect_ItemsByEntity);
					Standard_Integer NbRoots = aReader_Step.NbRootsForTransfer();
					Standard_Integer num = aReader_Step.TransferRoots();
					aShape = aReader_Step.OneShape();

					Bnd_Box bbox;
					BRepBndLib::Add(aShape, bbox);
					bbox.SetGap(0.0); // 消除间隙

					gp_Pnt bboxMin, bboxMax;
					Standard_Real theXmin, theYmin, theZmin, theXmax, theYmax, theZmax;
					bbox.Get(theXmin, theYmin, theZmin, theXmax, theYmax, theZmax); // 获取边界盒最小/最大点
					auto length = double(theXmax - theXmin);
					auto width = double(theYmax - theYmin);
					auto height = double(theZmax - theZmin);

					info.shape = aShape;
					info.path = filePath;
					info.length = length;
					info.width = width;
					info.height = height;
					ModelDataManager::GetInstance()->SetModelGeometryInfo(info);

					importModelWid->GetGFTreeModelWidget()->updataIcon();

					loadSuccess = true;
				}
			}
			else if (filePath.endsWith(".stl", Qt::CaseInsensitive)) {
				StlAPI_Reader aReader_Stl;
				// 读取STL文件
				if (aReader_Stl.Read(aShape, filePath.toStdString().c_str()))
				{
					// 计算边界盒（与STEP处理方式一致）
					Bnd_Box bbox;
					BRepBndLib::Add(aShape, bbox);
					bbox.SetGap(0.0); // 消除间隙

					Standard_Real theXmin, theYmin, theZmin, theXmax, theYmax, theZmax;
					bbox.Get(theXmin, theYmin, theZmin, theXmax, theYmax, theZmax);

					// 计算尺寸（与STEP处理方式一致）
					auto length = double(theXmax - theXmin);
					auto width = double(theYmax - theYmin);
					auto height = double(theZmax - theZmin);

					// 统一的信息存储（与STEP使用相同的数据结构）
					info.shape = aShape;
					info.path = filePath;
					info.length = length;
					info.width = width;
					info.height = height;
					ModelDataManager::GetInstance()->SetModelGeometryInfo(info);

					importModelWid->GetGFTreeModelWidget()->updataIcon();

					loadSuccess = true;
				}
			}


			if (!loadSuccess || aShape.IsNull()) {
				QMessageBox::warning(this, "Error", "Failed to load model");
				return;
			}

			// 获取OCC视图和上下文
			auto occView = importModelWid->GetOccView();
			Handle(AIS_InteractiveContext) context = occView->getContext();

			// 清除之前的显示
			context->RemoveAll(true);

			// 创建模型的AIS表示
			Handle(AIS_Shape) modelPresentation = new AIS_Shape(aShape);

			// 设置模型显示属性
			context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
			context->SetColor(modelPresentation, Quantity_NOC_CYAN, true);
			context->Display(modelPresentation, false);

					
			// 调整视图以适应模型
			occView->fitAll();


			currentTime = QDateTime::currentDateTime();
			timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
			QString text = timeStr + "[信息]>导入几何模型,路径为：" + filePath;
			textEdit->appendPlainText(text);
		}
		else if (m_TabWidget->currentIndex() == 1)
		{
			QString filter = "Image files (*.xlsx *.xlx )";
			QString filePath = QFileDialog::getOpenFileName(nullptr, QObject::tr("Open Excle"),
				QDir::currentPath(), filter);
		}

	});

	auto occView = importModelWid->GetOccView();
	connect(MoveBtn, &QPushButton::clicked, occView, &OccView::pan);
	connect(RotateBtn, &QPushButton::clicked, occView, &OccView::rotate);
	connect(ZoomBtn, &QPushButton::clicked, occView, &OccView::zoom);
	connect(FitAllBtn, &QPushButton::clicked, occView, &OccView::fitAll);
	connect(ResetBtn, &QPushButton::clicked, occView, &OccView::reset);
}

mainWidget::~mainWidget()
{
    delete ui;
}


void deleteWidget(QLayout *layout)
{
	if (layout) {
		for (int i = layout->count() - 1; i >= 0; --i) {
			QLayoutItem *item = layout->itemAt(i);
			QWidget *widget = item->widget();
			if (widget) {
				delete widget;
			}
			else {
				delete item;
			}
		}
	}
}

void mainWidget::changeChartView(QMap<int, QWidget*> chartWidMap)
{
	QLinearGradient gradient(0, 0, 1, 1); // 参数为起点和终点坐标，(0,0)是左下角，(1,1)是右上角
	gradient.setColorAt(0, QColor(5, 96, 135)); // 设置起点颜色为蓝色
	gradient.setColorAt(1, QColor(16, 27, 50)); // 设置终点颜色为红色

	QWidget*widget_1 = chartWidMap.value(1);
	QSplineSeries *series_1 = new QSplineSeries();
	series_1->setName("spline");
	series_1->append(10, 36);
	series_1->append(12, 54);
	series_1->append(13, 78);
	series_1->append(17, 34);
	series_1->append(20, 15);
	QChart *chart_1 = new QChart();
	chart_1->legend()->hide();//隐藏图例
	chart_1->addSeries(series_1);//添加数据
	chart_1->setTitle("Simple example");//标题
	chart_1->createDefaultAxes();//坐标系
	chart_1->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_1->setBackgroundBrush(QBrush(gradient));
	chart_1->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis *xAxis_1 = chart_1->axisX();
	QAbstractAxis *yAxis_1 = chart_1->axisY();
	xAxis_1->setLabelsColor(Qt::white);
	yAxis_1->setLabelsColor(Qt::white);
	QChartView *chartView_1 = new QChartView();
	chartView_1->setChart(chart_1);
	chartView_1->resize(widget_1->size());
	chartView_1->setRenderHint(QPainter::Antialiasing);
	QLayout *layout_1 = widget_1->layout();
	deleteWidget(layout_1);
	widget_1->layout()->addWidget(chartView_1);

	QWidget*widget_2 = chartWidMap.value(2);
	QSplineSeries *series_2 = new QSplineSeries();
	series_2->setName("spline");
	series_2->append(13, 36);
	series_2->append(22, 64);
	series_2->append(36, 88);
	series_2->append(77, 24);
	series_2->append(90, 5);
	QChart *chart_2 = new QChart();
	chart_2->legend()->hide();//隐藏图例
	chart_2->addSeries(series_2);//添加数据
	chart_2->setTitle("Simple example");//标题
	chart_2->createDefaultAxes();//坐标系
	chart_2->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_2->setBackgroundBrush(QBrush(gradient));
	chart_2->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis *xAxis_2 = chart_2->axisX();
	QAbstractAxis *yAxis_2 = chart_2->axisY();
	xAxis_2->setLabelsColor(Qt::white);
	yAxis_2->setLabelsColor(Qt::white);
	QChartView *chartView_2 = new QChartView();
	chartView_2->setChart(chart_2);
	chartView_2->resize(widget_2->size());
	chartView_2->setRenderHint(QPainter::Antialiasing);
	QLayout *layout_2 = widget_2->layout();
	deleteWidget(layout_2);
	widget_2->layout()->addWidget(chartView_2);

	QWidget*widget_3 = chartWidMap.value(3);
	QSplineSeries *series_3 = new QSplineSeries();
	series_3->setName("spline");
	series_3->append(20, 36);
	series_3->append(42, 54);
	series_3->append(63, 18);
	series_3->append(77, 84);
	series_3->append(80, 55);
	QChart *chart_3 = new QChart();
	chart_3->legend()->hide();//隐藏图例
	chart_3->addSeries(series_3);//添加数据
	chart_3->setTitle("Simple example");//标题
	chart_3->createDefaultAxes();//坐标系
	chart_3->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_3->setBackgroundBrush(QBrush(gradient));
	chart_3->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis *xAxis_3 = chart_3->axisX();
	QAbstractAxis *yAxis_3 = chart_3->axisY();
	xAxis_3->setLabelsColor(Qt::white);
	yAxis_3->setLabelsColor(Qt::white);
	QChartView *chartView_3 = new QChartView();
	chartView_3->setChart(chart_3);
	chartView_3->resize(widget_3->size());
	chartView_3->setRenderHint(QPainter::Antialiasing);
	QLayout *layout_3 = widget_3->layout();
	deleteWidget(layout_3);
	widget_3->layout()->addWidget(chartView_3);

	QWidget*widget_4 = chartWidMap.value(4);
	QSplineSeries *series_4 = new QSplineSeries();
	series_4->setName("spline");
	series_4->append(2, 56);
	series_4->append(23, 74);
	series_4->append(43, 28);
	series_4->append(67, 94);
	series_4->append(80, 100);
	QChart *chart_4 = new QChart();
	chart_4->legend()->hide();//隐藏图例
	chart_4->addSeries(series_4);//添加数据
	chart_4->setTitle("Simple example");//标题
	chart_4->createDefaultAxes();//坐标系
	chart_4->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_4->setBackgroundBrush(QBrush(gradient));
	chart_4->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis *xAxis_4 = chart_4->axisX();
	QAbstractAxis *yAxis_4 = chart_4->axisY();
	xAxis_4->setLabelsColor(Qt::white);
	yAxis_4->setLabelsColor(Qt::white);
	QChartView *chartView_4 = new QChartView();
	chartView_4->setChart(chart_4);
	chartView_4->resize(widget_4->size());
	chartView_4->setRenderHint(QPainter::Antialiasing);
	QLayout *layout_4 = widget_4->layout();
	deleteWidget(layout_4);
	widget_4->layout()->addWidget(chartView_4);

	QWidget*widget_5 = chartWidMap.value(5);
	QSplineSeries *series_5 = new QSplineSeries();
	series_5->setName("spline");
	series_5->append(7, 36);
	series_5->append(43, 14);
	series_5->append(53, 48);
	series_5->append(77, 64);
	series_5->append(87, 85);
	QChart *chart_5 = new QChart();
	chart_5->legend()->hide();//隐藏图例
	chart_5->addSeries(series_5);//添加数据
	chart_5->setTitle("Simple example");//标题
	chart_5->createDefaultAxes();//坐标系
	chart_5->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_5->setBackgroundBrush(QBrush(gradient));
	chart_5->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis *xAxis_5 = chart_5->axisX();
	QAbstractAxis *yAxis_5 = chart_5->axisY();
	xAxis_5->setLabelsColor(Qt::white);
	yAxis_5->setLabelsColor(Qt::white);
	QChartView *chartView_5 = new QChartView();
	chartView_5->setChart(chart_5);
	chartView_5->resize(widget_5->size());
	chartView_5->setRenderHint(QPainter::Antialiasing);
	QLayout *layout_5 = widget_5->layout();
	deleteWidget(layout_5);
	widget_5->layout()->addWidget(chartView_5);

	QWidget*widget_6 = chartWidMap.value(6);
	QSplineSeries *series_6 = new QSplineSeries();
	series_6->setName("spline");
	series_6->append(0, 0);
	series_6->append(32, 34);
	series_6->append(53, 48);
	series_6->append(78, 47);
	series_6->append(90, 95);
	QChart *chart_6 = new QChart();
	chart_6->legend()->hide();//隐藏图例
	chart_6->addSeries(series_6);//添加数据
	chart_6->setTitle("Simple example");//标题
	chart_6->createDefaultAxes();//坐标系
	chart_6->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_6->setBackgroundBrush(QBrush(gradient));
	chart_6->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis *xAxis_6 = chart_6->axisX();
	QAbstractAxis *yAxis_6 = chart_6->axisY();
	xAxis_6->setLabelsColor(Qt::white);
	yAxis_6->setLabelsColor(Qt::white);
	QChartView *chartView_6 = new QChartView();
	chartView_6->setChart(chart_6);
	chartView_6->resize(widget_6->size());
	chartView_6->setRenderHint(QPainter::Antialiasing);
	QLayout *layout_6 = widget_6->layout();
	deleteWidget(layout_6);
	widget_6->layout()->addWidget(chartView_6);

	QWidget*widget_7 = chartWidMap.value(7);
	QSplineSeries *series_7 = new QSplineSeries();
	series_7->setName("spline");
	series_7->append(0, 36);
	series_7->append(34, 84);
	series_7->append(45, 48);
	series_7->append(57, 24);
	series_7->append(68, 95);
	QChart *chart_7 = new QChart();
	chart_7->legend()->hide();//隐藏图例
	chart_7->addSeries(series_7);//添加数据
	chart_7->setTitle("Simple example");//标题
	chart_7->createDefaultAxes();//坐标系
	chart_7->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_7->setBackgroundBrush(QBrush(gradient));
	chart_7->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis *xAxis_7 = chart_7->axisX();
	QAbstractAxis *yAxis_7 = chart_7->axisY();
	xAxis_7->setLabelsColor(Qt::white);
	yAxis_7->setLabelsColor(Qt::white);
	QChartView *chartView_7 = new QChartView();
	chartView_7->setChart(chart_7);
	chartView_7->resize(widget_7->size());
	chartView_7->setRenderHint(QPainter::Antialiasing);
	QLayout *layout_7 = widget_7->layout();
	deleteWidget(layout_7);
	widget_7->layout()->addWidget(chartView_7);

	QWidget*widget_8 = chartWidMap.value(8);
	QSplineSeries *series_8 = new QSplineSeries();
	series_8->setName("spline");
	series_8->append(0, 66);
	series_8->append(32, 94);
	series_8->append(43, 18);
	series_8->append(67, 84);
	series_8->append(80, 5);
	QChart *chart_8 = new QChart();
	chart_8->legend()->hide();//隐藏图例
	chart_8->addSeries(series_8);//添加数据
	chart_8->setTitle("Simple example");//标题
	chart_8->createDefaultAxes();//坐标系
	chart_8->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_8->setBackgroundBrush(QBrush(gradient));
	chart_8->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis *xAxis_8 = chart_8->axisX();
	QAbstractAxis *yAxis_8 = chart_8->axisY();
	xAxis_8->setLabelsColor(Qt::white);
	yAxis_8->setLabelsColor(Qt::white);
	QChartView *chartView_8 = new QChartView();
	chartView_8->setChart(chart_8);
	chartView_8->resize(widget_8->size());
	chartView_8->setRenderHint(QPainter::Antialiasing);
	QLayout *layout_8 = widget_8->layout();
	deleteWidget(layout_8);
	widget_8->layout()->addWidget(chartView_8);



	/////////////////////// 中间比例图
	QWidget*widget_0 = chartWidMap.value(0);
	const qreal angularMin = 0;
	const qreal angularMax = 360;
	const qreal radialMin = 0;
	const qreal radialMax = 360;


	qreal ad6 = (angularMax - angularMin) / 8;

	QLineSeries *seriesdata = new QLineSeries();
	seriesdata->append(angularMin, 143);
	seriesdata->append(angularMin + ad6 * 1, 330);
	seriesdata->append(angularMin + ad6 * 2, 290);
	seriesdata->append(angularMin + ad6 * 3, 369);
	seriesdata->append(angularMin + ad6 * 4, 146);
	seriesdata->append(angularMin + ad6 * 5, 262);
	seriesdata->append(angularMin + ad6 * 6, 143);
	seriesdata->append(angularMin + ad6 * 7, 243);
	seriesdata->append(angularMin + ad6 * 8, 143);
	{
		//设置线上的标签可见
		seriesdata->setPointLabelsFormat("@yPoint");
		seriesdata->setPointLabelsClipping(false);
		seriesdata->setPointLabelsVisible(true);
		seriesdata->setPointLabelsColor(Qt::white);
	}



	QAreaSeries *seriesArea = new QAreaSeries();
	seriesArea->setUpperSeries(seriesdata);
	seriesArea->setOpacity(0.5);

	QPolarChart *chart = new QPolarChart();
	chart->setPlotArea(QRectF(0, 2500, 0, 0)); // 上移50个单位


	chart->addSeries(seriesdata);
	chart->addSeries(seriesArea);


	QCategoryAxis *angularAxis = new QCategoryAxis();
	angularAxis->append("A", 0);
	angularAxis->append("B", 45);
	angularAxis->append("C", 90);
	angularAxis->append("D", 134);
	angularAxis->append("E", 180);
	angularAxis->append("F", 225);
	angularAxis->append("G", 270);
	angularAxis->append("H", 315);
	angularAxis->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
	angularAxis->setLabelsColor(Qt::white);

	angularAxis->setShadesBrush(QBrush(QColor(249, 249, 255)));
	chart->addAxis(angularAxis, QPolarChart::PolarOrientationAngular);
	QValueAxis *radialAxis = new QValueAxis();

	radialAxis->setLabelFormat("");
	chart->addAxis(radialAxis, QPolarChart::PolarOrientationRadial);

	angularAxis->setLineVisible(false);
	angularAxis->setLabelsColor(Qt::white);
	chart->axes(QPolarChart::PolarOrientationRadial).at(0)->setVisible(false);
	seriesArea->attachAxis(radialAxis);
	seriesArea->attachAxis(angularAxis);
	for (int i = 0; i <= 2; ++i)
	{
		QLineSeries *seriesLineTemp = new QLineSeries();
		chart->addSeries(seriesLineTemp);
		seriesLineTemp->attachAxis(radialAxis);
		seriesLineTemp->attachAxis(angularAxis);
		seriesLineTemp->setColor(QColor(214, 214, 214));
		int interval = 45;
		seriesLineTemp->append(angularMin, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 1, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 2, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 3, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 4, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 5, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 6, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 7, radialMax - 10 - interval * i);
		seriesLineTemp->append(angularMin + ad6 * 8, radialMax - 10 - interval * i);
	}

	chart->legend()->markers(seriesdata).at(0)->setVisible(false);

	foreach(QLegendMarker* marker, chart->legend()->markers())
	{
		if (marker->type() != QLegendMarker::LegendMarkerTypeArea)
		{
			marker->setVisible(false);
		}
	}


	radialAxis->setRange(radialMin, radialMax);
	angularAxis->setRange(angularMin, angularMax);

	chart->setBackgroundBrush(QBrush(gradient));
	chart->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis *xAxis_0 = chart->axisX();
	QAbstractAxis *yAxis_0 = chart->axisY();
	xAxis_0->setLabelsColor(Qt::white);
	yAxis_0->setLabelsColor(Qt::white);
	QChartView *chartView_0 = new QChartView();
	chartView_0->setChart(chart);
	chartView_0->resize(440, 400);
	chartView_0->move(chartView_0->x(), chartView_0->y() - 100);
	chartView_0->setRenderHint(QPainter::Antialiasing);
	QLayout *layout_0 = widget_0->layout();
	deleteWidget(layout_0);
	widget_0->layout()->addWidget(chartView_0);
}


void mainWidget::deleteWidget(QLayout *layout)
{
	if (layout) {
		for (int i = layout->count() - 1; i >= 0; --i) {
			QLayoutItem *item = layout->itemAt(i);
			QWidget *widget = item->widget();
			if (widget) {
				delete widget;
			}
			else {
				delete item;
			}
		}
	}
}

void mainWidget::refreshMemoryUsage(QLabel *m_statusLabel) {
	// 避免重复创建定时器（防止内存泄漏和多次触发）
	if (timer) {
		timer->stop();
		delete timer;
	}

	timer = new QTimer(this);
	timer->setInterval(5000); // 5秒采样一次（合理间隔，平衡实时性和性能）
	connect(timer, &QTimer::timeout, [this, m_statusLabel]() {
		getMemoryUsage(m_statusLabel);
		});

	// 初始化首次采样的基准时间（关键：提前获取初始时间，避免首次计算异常）
	GetSystemTimes(&prevIdleTime, &prevKernelTime, &prevUserTime);
	isFirstSample = true; // 标记首次采样
	timer->start();
	getMemoryUsage(m_statusLabel); // 首次调用（此时CPU显示为0%，避免异常值）
}

void mainWidget::getMemoryUsage(QLabel *m_statusLabel) {
	QString memoryText = "0.00";
	QString cpuText = "0.00";

	// 内存
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	if (GlobalMemoryStatusEx(&statex)) {
		ULONGLONG totalPhys = statex.ullTotalPhys;
		ULONGLONG availPhys = statex.ullAvailPhys;
		double memoryUsage = ((totalPhys - availPhys) / static_cast<double>(totalPhys)) * 100.0;
		memoryText = QString::number(memoryUsage, 'f', 2);
	}
	else {
		qWarning() << "获取内存信息失败，错误码：" << GetLastError();
		memoryText = "获取失败";
	}

	// CPU
	FILETIME currIdleTime, currKernelTime, currUserTime;
	if (!GetSystemTimes(&currIdleTime, &currKernelTime, &currUserTime)) {
		qWarning() << "获取系统时间失败，错误码：" << GetLastError();
		cpuText = "获取失败";
	}
	else {
		// 首次采样：仅更新基准时间，不计算使用率（避免异常值）
		if (isFirstSample) {
			prevIdleTime = currIdleTime;
			prevKernelTime = currKernelTime;
			prevUserTime = currUserTime;
			isFirstSample = false;
			cpuText = "0.00"; // 首次显示0%
		}
		else {
			// 计算时间差（64位整数，无溢出）
			ULONGLONG idleDiff = fileTimeToULL(currIdleTime) - fileTimeToULL(prevIdleTime);
			ULONGLONG kernelDiff = fileTimeToULL(currKernelTime) - fileTimeToULL(prevKernelTime);
			ULONGLONG userDiff = fileTimeToULL(currUserTime) - fileTimeToULL(prevUserTime);

			// 总系统时间 = 内核时间 + 用户时间（所有CPU核心的总运行时间）
			ULONGLONG totalSysDiff = kernelDiff + userDiff;

			// 避免除零（极端情况，如系统无任何操作）
			if (totalSysDiff == 0) {
				cpuText = "0.00";
			}
			else {
				// 计算CPU使用率：(总时间 - 空闲时间) / 总时间 × 100%
				double cpuUsage = (1.0 - static_cast<double>(idleDiff) / totalSysDiff) * 100.0;
				// 边界限制：确保数值在0%~100%之间（避免计算误差导致的超界）
				cpuUsage = qBound(0.0, cpuUsage, 100.0);
				cpuText = QString::number(cpuUsage, 'f', 2);
			}

			// 更新基准时间（为下一次计算做准备）
			prevIdleTime = currIdleTime;
			prevKernelTime = currKernelTime;
			prevUserTime = currUserTime;
		}
	}

	// 更新QLabel显示
	m_statusLabel->setText(QString("内存使用：%1%, CPU使用：%2%").arg(memoryText).arg(cpuText));

}


// 辅助函数：FILETIME 转 64位整数（核心修复：正确合并高低位）
ULONGLONG mainWidget::fileTimeToULL(const FILETIME& ft)
{
	ULARGE_INTEGER ul;
	ul.LowPart = ft.dwLowDateTime;
	ul.HighPart = ft.dwHighDateTime;
	return ul.QuadPart; // 返回完整的64位时间戳（100纳秒为单位）
}