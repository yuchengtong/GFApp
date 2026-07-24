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


#include "GFLogWidget.h"
#include "ParamAnalyWidget.h"
#include "GFParamAnalyWidget.h"
#include "OccView.h"
#include "GFTreeModelWidget.h"
#include "IntelligentAnalyWidget.h"



mainWidget::mainWidget(QWidget* parent)
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
	QLabel* m_statusLabel = new QLabel("内存使用：0%，CPU使用：0%");
	statusbar->addPermanentWidget(m_statusLabel);
	refreshMemoryUsage(m_statusLabel);

	QToolBar* topNavToolBar = new QToolBar(this);
	topNavToolBar->setMovable(false);
	topNavToolBar->setFloatable(false);
	topNavToolBar->setIconSize(QSize(40, 40));
	topNavToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); // 图标文字并排

	// 统一按钮样式
	topNavToolBar->setStyleSheet(R"(
		QToolBar {
			border: none;                /* 清除全部边框（含底部横线） */
			border-bottom: 0px solid;
			padding: 2px 4px;
			spacing: 6px;               // 按钮之间间距
		}
		QToolBar::separator {
			width: 0px;                 // 隐藏工具栏内分隔线
			background: transparent;
		}
		QToolBar QToolButton{
			padding: 0 12px;
			height: 48px;
			border: none;
			font-size: 12px;
			min-width: 180px;
		}
		QToolBar QToolButton:hover{
			background:#e6f0ff;
		}
		)");
	//宽度自适应窗口拉伸
	topNavToolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	// 定义Action
	m_ImportModelWidAct = new QAction("安全性特性参数分析", this);
	m_DataBaseWidAct = new QAction("数据管理", this);
	m_IntelligentAnalyWidAct = new QAction("数据智能分析", this);
	m_AnalyEvalWidAct = new QAction("安全性分析与评估", this);
	m_AuxiliaryAnalyWidAct = new QAction("安全性指标预计、权衡和辅助分析", this);
	m_HelpAct = new QAction("帮助", this);

	// 图标
	auto importModelWidIcon = QIcon(":/src/import_model.svg");
	auto dataBaseWidIcon = QIcon(":/src/database.svg");
	auto intelligentAnalyIcon = QIcon(":/src/intelligent_analy.svg");
	auto AnalyEvalWidIcon = QIcon(":/src/analy_eval.svg");
	auto auxiliaryAnalyWidIcon = QIcon(":/src/auxiliary_analy.svg");
	auto helpIcon = QIcon(":/src/help.svg");

	m_ImportModelWidAct->setIcon(importModelWidIcon);
	m_DataBaseWidAct->setIcon(dataBaseWidIcon);
	m_IntelligentAnalyWidAct->setIcon(intelligentAnalyIcon);
	m_AnalyEvalWidAct->setIcon(AnalyEvalWidIcon);
	m_AuxiliaryAnalyWidAct->setIcon(auxiliaryAnalyWidIcon);
	m_HelpAct->setIcon(helpIcon);

	// 把所有Action添加到顶部导航工具栏
	topNavToolBar->addAction(m_DataBaseWidAct);
	topNavToolBar->addAction(m_ImportModelWidAct);
	topNavToolBar->addAction(m_IntelligentAnalyWidAct);
	topNavToolBar->addAction(m_AnalyEvalWidAct);
	topNavToolBar->addAction(m_AuxiliaryAnalyWidAct);
	topNavToolBar->addAction(m_HelpAct);
	topNavToolBar->setFixedHeight(56);


	// 先添加顶部导航
	addToolBar(Qt::TopToolBarArea, topNavToolBar);
	// 插入换行分割线（必须指定区域）
	addToolBarBreak(Qt::TopToolBarArea);
	// 再把ui->mainToolBar加入顶部区域
	addToolBar(Qt::TopToolBarArea, ui->mainToolBar);


	ui->mainToolBar->setMovable(false);
	ui->mainToolBar->setFloatable(false);
	//////////////////////////////////////////////////////////ToolBar



	m_importBtn = new QPushButton();
	m_saveBtn = new QPushButton();
	m_saveAsBtn = new QPushButton();
	m_exportBtn = new QPushButton();
	m_importBtn->setIcon(QIcon(":/src/Import.svg"));
	m_saveBtn->setIcon(QIcon(":/src/Save.svg"));
	m_saveAsBtn->setIcon(QIcon(":/src/Save_as.svg"));
	m_exportBtn->setIcon(QIcon(":/src/Export.svg"));

	const int btnSize = 32;
	QSize iconSize(btnSize, btnSize);
	m_importBtn->setIconSize(iconSize);
	m_saveBtn->setIconSize(iconSize);
	m_saveAsBtn->setIconSize(iconSize);
	m_exportBtn->setIconSize(iconSize);

	auto ImportLabel = new QLabel("导入文件");
	auto SaveLabel = new QLabel("保存文件");
	auto SaveAsLabel = new QLabel("另存为...");
	auto ExportLabel = new QLabel("导出文件");


	auto importVBox = new QVBoxLayout();
	importVBox->addWidget(m_importBtn, 0, Qt::AlignHCenter);
	importVBox->addWidget(ImportLabel, 0, Qt::AlignHCenter);
	importVBox->setSpacing(2);
	importVBox->setContentsMargins(4, 0, 4, 0);

	auto saveAsVBox = new QVBoxLayout();
	saveAsVBox->addWidget(m_saveAsBtn, 0, Qt::AlignHCenter);
	saveAsVBox->addWidget(SaveAsLabel, 0, Qt::AlignHCenter);
	saveAsVBox->setSpacing(2);
	saveAsVBox->setContentsMargins(4, 0, 4, 0);

	auto saveVBox = new QVBoxLayout();
	saveVBox->addWidget(m_saveBtn, 0, Qt::AlignHCenter);
	saveVBox->addWidget(SaveLabel, 0, Qt::AlignHCenter);
	saveVBox->setSpacing(2);
	saveVBox->setContentsMargins(4, 0, 4, 0);

	auto exportVBox = new QVBoxLayout();
	exportVBox->addWidget(m_exportBtn, 0, Qt::AlignHCenter);
	exportVBox->addWidget(ExportLabel, 0, Qt::AlignHCenter);
	exportVBox->setSpacing(2);
	exportVBox->setContentsMargins(4, 0, 4, 0);

	auto hLayout = new QHBoxLayout();
	hLayout->addLayout(importVBox);
	hLayout->addLayout(saveAsVBox);
	hLayout->addLayout(saveVBox);
	hLayout->addLayout(exportVBox);
	hLayout->addStretch();
	hLayout->setSpacing(2);
	hLayout->setContentsMargins(4, 0, 4, 0);

	auto vLayout = new QVBoxLayout();
	vLayout->addLayout(hLayout);
	vLayout->setContentsMargins(0, 0, 0, 0);
	vLayout->setSpacing(1);

	auto geomWidget = new QWidget();
	geomWidget->setMinimumWidth(240);
	geomWidget->setLayout(vLayout);




	m_moveBtn = new QPushButton();
	m_rotateBtn = new QPushButton();
	m_zoomBtn = new QPushButton();
	m_fitAllBtn = new QPushButton();
	m_resetBtn = new QPushButton();
	m_moveBtn->setIcon(QIcon(":/src/Move.svg"));
	m_rotateBtn->setIcon(QIcon(":/src/Rotate.svg"));
	m_zoomBtn->setIcon(QIcon(":/src/Zoom.png"));
	m_fitAllBtn->setIcon(QIcon(":/src/FitAll.png"));
	m_resetBtn->setIcon(QIcon(":/src/Reset.svg"));
	m_moveBtn->setIconSize(iconSize);
	m_rotateBtn->setIconSize(iconSize);
	m_zoomBtn->setIconSize(iconSize);
	m_fitAllBtn->setIconSize(iconSize);
	m_resetBtn->setIconSize(iconSize);
	auto MoveLabel = new QLabel("移动");
	auto RotateLabel = new QLabel("旋转");
	auto ZoomLabel = new QLabel("缩放");
	auto FitAllLabel = new QLabel("聚焦");
	auto ResetLabel = new QLabel("重置");

	auto moveVBox = new QVBoxLayout();
	moveVBox->addWidget(m_moveBtn, 0, Qt::AlignHCenter);
	moveVBox->addWidget(MoveLabel, 0, Qt::AlignHCenter);
	moveVBox->setSpacing(2);
	moveVBox->setContentsMargins(4, 0, 4, 0);

	auto rotateVBox = new QVBoxLayout();
	rotateVBox->addWidget(m_rotateBtn, 0, Qt::AlignHCenter);
	rotateVBox->addWidget(RotateLabel, 0, Qt::AlignHCenter);
	rotateVBox->setSpacing(2);
	rotateVBox->setContentsMargins(4, 0, 4, 0);

	auto zoomVBox = new QVBoxLayout();
	zoomVBox->addWidget(m_zoomBtn, 0, Qt::AlignHCenter);
	zoomVBox->addWidget(ZoomLabel, 0, Qt::AlignHCenter);
	zoomVBox->setSpacing(2);
	zoomVBox->setContentsMargins(4, 0, 4, 0);

	auto fitAllVBox = new QVBoxLayout();
	fitAllVBox->addWidget(m_fitAllBtn, 0, Qt::AlignHCenter);
	fitAllVBox->addWidget(FitAllLabel, 0, Qt::AlignHCenter);
	fitAllVBox->setSpacing(2);
	fitAllVBox->setContentsMargins(4, 0, 4, 0);

	auto resetVBox = new QVBoxLayout();
	resetVBox->addWidget(m_resetBtn, 0, Qt::AlignHCenter);
	resetVBox->addWidget(ResetLabel, 0, Qt::AlignHCenter);
	resetVBox->setSpacing(2);
	resetVBox->setContentsMargins(4, 0, 4, 0);

	auto operationHLayout = new QHBoxLayout();
	operationHLayout->addLayout(moveVBox);
	operationHLayout->addLayout(rotateVBox);
	operationHLayout->addLayout(zoomVBox);
	operationHLayout->addLayout(fitAllVBox);
	operationHLayout->addLayout(resetVBox);
	operationHLayout->addStretch();
	operationHLayout->setSpacing(2);
	operationHLayout->setContentsMargins(4, 0, 4, 0);

	auto operationVLayout = new QVBoxLayout();
	operationVLayout->addLayout(operationHLayout);
	operationVLayout->setContentsMargins(0, 0, 0, 0);
	operationVLayout->setSpacing(1);

	auto operationWidget = new QWidget();
	operationWidget->setMinimumWidth(240);
	operationWidget->setLayout(operationVLayout);


	m_XBtn = new QPushButton();
	m_YBtn = new QPushButton();
	m_ZBtn = new QPushButton();
	m__XBtn = new QPushButton();
	m__YBtn = new QPushButton();
	m__ZBtn = new QPushButton();
	m_XBtn->setIconSize(iconSize);
	m_YBtn->setIconSize(iconSize);
	m_ZBtn->setIconSize(iconSize);
	m__XBtn->setIconSize(iconSize);
	m__YBtn->setIconSize(iconSize);
	m__ZBtn->setIconSize(iconSize);

	m_XBtn->setIcon(QIcon(":/src/View all From +X.png"));
	m_YBtn->setIcon(QIcon(":/src/View all From +Y.png"));
	m_ZBtn->setIcon(QIcon(":/src/View all From +Z.png"));
	m__XBtn->setIcon(QIcon(":/src/View all From -X.png"));
	m__YBtn->setIcon(QIcon(":/src/View all From -Y.png"));
	m__ZBtn->setIcon(QIcon(":/src/View all From -Z.png"));
	auto XLabel = new QLabel("X轴方向");
	auto YLabel = new QLabel("Y轴方向");
	auto ZLabel = new QLabel("Z轴方向");
	auto _XLabel = new QLabel("负X轴方向");
	auto _YLabel = new QLabel("负Y轴方向");
	auto _ZLabel = new QLabel("负Z轴方向");
	auto bottomTitleLab2 = new QLabel("视图");

	auto XVBox = new QVBoxLayout();
	XVBox->addWidget(m_XBtn, 0, Qt::AlignHCenter);
	XVBox->addWidget(XLabel, 0, Qt::AlignHCenter);
	XVBox->setSpacing(2);
	XVBox->setContentsMargins(4, 0, 4, 0);

	auto YVBox = new QVBoxLayout();
	YVBox->addWidget(m_YBtn, 0, Qt::AlignHCenter);
	YVBox->addWidget(YLabel, 0, Qt::AlignHCenter);
	YVBox->setSpacing(2);
	YVBox->setContentsMargins(4, 0, 4, 0);

	auto ZVBox = new QVBoxLayout();
	ZVBox->addWidget(m_ZBtn, 0, Qt::AlignHCenter);
	ZVBox->addWidget(ZLabel, 0, Qt::AlignHCenter);
	ZVBox->setSpacing(2);
	ZVBox->setContentsMargins(4, 0, 4, 0);

	auto _XVBox = new QVBoxLayout();
	_XVBox->addWidget(m__XBtn, 0, Qt::AlignHCenter);
	_XVBox->addWidget(_XLabel, 0, Qt::AlignHCenter);
	_XVBox->setSpacing(2);
	_XVBox->setContentsMargins(4, 0, 4, 0);

	auto _YVBox = new QVBoxLayout();
	_YVBox->addWidget(m__YBtn, 0, Qt::AlignHCenter);
	_YVBox->addWidget(_YLabel, 0, Qt::AlignHCenter);
	_YVBox->setSpacing(2);
	_YVBox->setContentsMargins(4, 0, 4, 0);

	auto _ZVBox = new QVBoxLayout();
	_ZVBox->addWidget(m__ZBtn, 0, Qt::AlignHCenter);
	_ZVBox->addWidget(_ZLabel, 0, Qt::AlignHCenter);
	_ZVBox->setSpacing(2);
	_ZVBox->setContentsMargins(4, 0, 4, 0);

	auto viewHLayout = new QHBoxLayout();
	viewHLayout->addLayout(XVBox);
	viewHLayout->addLayout(YVBox);
	viewHLayout->addLayout(ZVBox);
	viewHLayout->addLayout(_XVBox);
	viewHLayout->addLayout(_YVBox);
	viewHLayout->addLayout(_ZVBox);
	viewHLayout->addStretch();
	viewHLayout->setSpacing(2);
	viewHLayout->setContentsMargins(4, 0, 4, 0);

	auto viewVLayout = new QVBoxLayout();
	viewVLayout->addLayout(viewHLayout);
	viewVLayout->setContentsMargins(0, 0, 0, 0);
	viewVLayout->setSpacing(1);

	auto viewWidget = new QWidget();
	viewWidget->setMinimumWidth(340);
	viewWidget->setLayout(viewVLayout);

	geomWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	operationWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	viewWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	ui->mainToolBar->addWidget(geomWidget);
	ui->mainToolBar->addSeparator();
	ui->mainToolBar->addWidget(operationWidget);
	ui->mainToolBar->addSeparator();
	ui->mainToolBar->addWidget(viewWidget);
	ui->mainToolBar->addSeparator();

	QWidget* rightSpacer = new QWidget;
	rightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	ui->mainToolBar->addWidget(rightSpacer);


	m_TabWidget = new QTabWidget(this);

	m_importModelWid = new GFImportModelWidget(m_TabWidget);
	{
	}

	m_dataBaseWid = new DatabaseWidget(m_TabWidget);
	{
	}

	/*ParamAnalyWidget*paramAnalysisWid = new ParamAnalyWidget(m_TabWidget);
	{
	}*/

	IntelligentAnalyWidget* IntelligenAnalysisWid = new IntelligentAnalyWidget(m_TabWidget);
	{
	}
	m_auxiliaryAnalysisWid = new AuxiliaryAnalysisWidget(m_TabWidget);
	{
	}
	ParamAnalyWidget* analysisEvaluationWid = new ParamAnalyWidget(m_TabWidget);
	{
	}


	m_TabWidget->addTab(m_importModelWid, "importModelWid");
	m_TabWidget->addTab(m_dataBaseWid, "dataBaseWid");
	//m_TabWidget->addTab(paramAnalysisWid, "paramAnalysisWid");
	m_TabWidget->addTab(IntelligenAnalysisWid, "IntelligenAnalysisWid");
	m_TabWidget->addTab(analysisEvaluationWid, "analysisEvaluationWid");
	m_TabWidget->addTab(m_auxiliaryAnalysisWid, "auxiliaryAnalysisWid");
	m_TabWidget->tabBar()->setVisible(false);


	setCentralWidget(m_TabWidget);

	bindConnect();
	
}

mainWidget::~mainWidget()
{
	delete ui;
}

void mainWidget::bindConnect()
{
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
		QTreeWidget* treeWidget = m_dataBaseWid->getQTreeWid();
		auto ins = ModelDataManager::GetInstance();
		UserInfo info = ins->GetUserInfo();
		if (info.username != "admin")
		{
			QTreeWidgetItem* child;
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
		ui->mainToolBar->setVisible(false);
		/*auto occView5 = analysisEvaluationWid->GetOccView();
		auto myview = occView5->getView();
		myview->MustBeResized();*/
		});

	QObject::connect(m_AuxiliaryAnalyWidAct, &QAction::triggered, [=]() {
		m_TabWidget->setCurrentIndex(4);
		// 显示工具栏
		ui->mainToolBar->setVisible(false);
		// 更新echart数据
		m_auxiliaryAnalysisWid->updateAllData();
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

	QObject::connect(m_importBtn, &QPushButton::clicked, [this]() {
		if (m_TabWidget->currentIndex() == 0) {
			// 打开文件对话框
			QString filePath = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(),
				"STEP Files (*.stp *.step);;IGES Files (*.iges *.igs);;VTK Files (*.vtk);;X_T Files (*.x_t);;All Files (*.*)");

			if (filePath.isEmpty())
				return;
			QDateTime currentTime = QDateTime::currentDateTime();
			QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
			auto logWidget = m_importModelWid->GetLogWidget();
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
					bbox.Get(theXmin, theYmin, theZmin, theXmax, theYmax, theZmax); // 获取边界盒最小/最大点(包围盒)
					auto length = double(theXmax - theXmin);
					auto width = double(theYmax - theYmin);
					auto height = double(theZmax - theZmin);

					info.shape = aShape;
					info.path = filePath;
					info.theXmin = theXmin;
					info.theYmin = theYmin;
					info.theZmin = theZmin;
					info.theXmax = theXmax;
					info.theYmax = theYmax;
					info.theZmax = theZmax;

					info.length = length;
					info.width = width;
					info.height = height;
					ModelDataManager::GetInstance()->SetModelGeometryInfo(info);

					m_importModelWid->GetGFTreeModelWidget()->updataIcon();

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

					m_importModelWid->GetGFTreeModelWidget()->updataIcon();

					loadSuccess = true;
				}
			}
			//	else if (filePath.endsWith(".vtk", Qt::CaseInsensitive)) {
			//		vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
			//		reader->SetFileName(filePath.toStdString().c_str());
			//		reader->Update();

			//		vtkPolyData* polyData = reader->GetOutput();
			//		if (!polyData || polyData->GetNumberOfPoints() == 0) {
			//			QMessageBox::warning(this, "Error", "Failed to read VTK file or empty data.");
			//			return;
			//		}

			//		aShape = VtkPolyDataToOCCShape(polyData);
			//		if (aShape.IsNull()) {
			//			QMessageBox::warning(this, "Error", "Failed to convert VTK to OCC shape.");
			//			return;
			//		}

			//		// 计算包围盒等（同前）
			//		Bnd_Box bbox;
			//		BRepBndLib::Add(aShape, bbox);
			//		bbox.SetGap(0.0);
			//		Standard_Real theXmin, theYmin, theZmin, theXmax, theYmax, theZmax;
			//		bbox.Get(theXmin, theYmin, theZmin, theXmax, theYmax, theZmax);

			//		info.shape = aShape;
			//		info.path = filePath;
			//		info.theXmin = theXmin; info.theYmin = theYmin; info.theZmin = theZmin;
			//		info.theXmax = theXmax; info.theYmax = theYmax; info.theZmax = theZmax;
			//		info.length = theXmax - theXmin;
			//		info.width = theYmax - theYmin;
			//		info.height = theZmax - theZmin;

			//		ModelDataManager::GetInstance()->SetModelGeometryInfo(info);
			//		importModelWid->GetGFTreeModelWidget()->updataIcon();
			//		loadSuccess = true;
			//	}
			//	else if (filePath.endsWith(".x_t", Qt::CaseInsensitive)) {
			//	XSControl_Reader aReader_XT;
			//	// 设置为 Parasolid 模式（关键！）
			//	aReader_XT.SetMode("XSTEP"); // 或尝试 "DEFAULT"

			//	IFSelect_ReturnStatus status = aReader_XT.ReadFile(filePath.toStdString().c_str());
			//	if (status == IFSelect_RetDone) {
			//		aReader_XT.PrintCheckLoad(Standard_False, IFSelect_ItemsByEntity);
			//		Standard_Integer nbRoots = aReader_XT.NbRootsForTransfer();
			//		if (nbRoots > 0) {
			//			aReader_XT.TransferRoots();
			//			aShape = aReader_XT.OneShape();

			//			if (!aShape.IsNull()) {
			//				// 计算包围盒
			//				Bnd_Box bbox;
			//				BRepBndLib::Add(aShape, bbox);
			//				bbox.SetGap(0.0);

			//				Standard_Real theXmin, theYmin, theZmin, theXmax, theYmax, theZmax;
			//				bbox.Get(theXmin, theYmin, theZmin, theXmax, theYmax, theZmax);

			//				info.shape = aShape;
			//				info.path = filePath;
			//				info.theXmin = theXmin; info.theYmin = theYmin; info.theZmin = theZmin;
			//				info.theXmax = theXmax; info.theYmax = theYmax; info.theZmax = theZmax;
			//				info.length = double(theXmax - theXmin);
			//				info.width = double(theYmax - theYmin);
			//				info.height = double(theZmax - theZmin);

			//				ModelDataManager::GetInstance()->SetModelGeometryInfo(info);
			//				importModelWid->GetGFTreeModelWidget()->updataIcon();
			//				loadSuccess = true;
			//			}
			//		}
			//	}
			//	if (!loadSuccess || aShape.IsNull()) {
			//		QMessageBox::warning(this, "Error", "Failed to load model");
			//		return;
			//	}

			//	// 获取OCC视图和上下文
			//	auto occView = importModelWid->GetOccView();
			//	Handle(AIS_InteractiveContext) context = occView->getContext();

			//	// 清除之前的显示
			//	context->EraseAll(true);

			//	// 创建模型的AIS表示
			//	Handle(AIS_Shape) modelPresentation = new AIS_Shape(aShape);

			//	// 设置模型显示属性
			//	context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
			//	context->SetColor(modelPresentation, Quantity_Color(0.0, 1.0, 1.0, Quantity_TOC_RGB), true);
			//	context->Display(modelPresentation, false);

			//			
			//	// 调整视图以适应模型
			//	occView->fitAll();


			//	currentTime = QDateTime::currentDateTime();
			//	timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
			//	QString text = timeStr + "[信息]>导入几何模型,路径为：" + filePath;
			//	textEdit->appendPlainText(text);
		}
		else if (m_TabWidget->currentIndex() == 1)
		{
			QString filter = "Image files (*.xlsx *.xlx )";
			QString filePath = QFileDialog::getOpenFileName(nullptr, QObject::tr("Open Excle"),
				QDir::currentPath(), filter);
		}

		});


	auto doSave = [this](const QString& folderPath) -> bool {
		QString baseName = QFileInfo(folderPath).fileName();
		QString gfFilePath = folderPath + "/" + baseName + ".gf";

		QFile file(gfFilePath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QMessageBox::warning(this, tr("保存失败"),
				tr("无法打开项目文件:\n%1").arg(file.errorString()));
			return false;
		}

		// 创建 Model 子文件夹
		QString modelDir = folderPath + "/Model";
		if (!QDir().exists(modelDir) && !QDir().mkpath(modelDir)) {
			QMessageBox::warning(this, tr("保存失败"),
				tr("无法创建 Model 文件夹:\n%1").arg(modelDir));
			file.close();
			return false;
		}

		// 复制几何模型资源文件到 Model 文件夹
		auto geo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
		auto copyIfValid = [](const QString& srcPath, const QString& destDir) {
			if (!srcPath.isEmpty()) {
				QString destPath = destDir + "/" + QFileInfo(srcPath).fileName();
				QFile::copy(srcPath, destPath);
			}
		};
		copyIfValid(geo.nozzlePath, modelDir);
		copyIfValid(geo.shellPath, modelDir);
		copyIfValid(geo.propellantPath, modelDir);
		copyIfValid(geo.heatInsulatingLayerPath, modelDir);

		// 写入 .gf 项目数据
		// QTextStream out(&file);
		// out << yourDataToSave;

		file.close();
		return true;
	};

	// 2. 提取公共的"另存为"对话框：只负责 UI 交互和创建文件夹
	auto saveAsDialog = [this]() -> QString {
		QSettings settings;
		QString lastDir = settings.value("lastSaveDir").toString();

		QString selectedPath = QFileDialog::getSaveFileName(
			this,
			tr("另存为"),
			lastDir,
			tr("GF Files (*.gf);;All Files (*)")
		);

		if (selectedPath.isEmpty())
			return QString();

		QFileInfo fi(selectedPath);
		QString parentDir = fi.absolutePath();
		QString baseName = fi.completeBaseName();
		QString folderPath = parentDir + "/" + baseName;

		if (!QDir().exists(folderPath) && !QDir().mkpath(folderPath)) {
			QMessageBox::warning(this, tr("保存失败"),
				tr("无法创建文件夹:\n%1").arg(folderPath));
			return QString();
		}

		settings.setValue("lastSaveDir", parentDir);
		return folderPath;
	};

	// 3. 保存按钮：有路径直接覆盖，无路径走另存为
	connect(m_saveBtn, &QPushButton::clicked, [this, doSave, saveAsDialog]() {
		auto projectInfo = ModelDataManager::GetInstance()->GetProjectInfo();

		if (projectInfo.projectPath.isEmpty()) 
		{
			QString folderPath = saveAsDialog();
			if (folderPath.isEmpty())
				return;

			if (!doSave(folderPath))
				return;

			projectInfo.projectPath = folderPath;
			ModelDataManager::GetInstance()->SetProjectInfo(projectInfo);
		}
		else 
		{
			doSave(projectInfo.projectPath);  // 直接覆盖原文件
		}
		});

	// 4. 另存为按钮：总是弹对话框，完成后更新项目路径
	connect(m_saveAsBtn, &QPushButton::clicked, [this, doSave, saveAsDialog]() {
		QString folderPath = saveAsDialog();
		if (folderPath.isEmpty())
			return;

		if (!doSave(folderPath))
			return;

		auto projectInfo = ModelDataManager::GetInstance()->GetProjectInfo();
		projectInfo.projectPath = folderPath;
		ModelDataManager::GetInstance()->SetProjectInfo(projectInfo);
		});



	auto occView = m_importModelWid->GetOccView();
	connect(m_moveBtn, &QPushButton::clicked, occView, &OccView::pan);
	connect(m_rotateBtn, &QPushButton::clicked, occView, &OccView::rotate);
	connect(m_zoomBtn , &QPushButton::clicked, occView, &OccView::zoom);
	connect(m_fitAllBtn, &QPushButton::clicked, occView, &OccView::fitAll);
	connect(m_resetBtn, &QPushButton::clicked, occView, &OccView::reset);

	QObject::connect(m_XBtn, &QPushButton::clicked, [occView]() {
		auto state = occView->GetCameraRotationState();
		if (state)
		{
			Handle(V3d_View) view = occView->getView();
			view->SetProj(V3d_Xpos);
			occView->fitAll();
		}
		});
	QObject::connect(m_YBtn, &QPushButton::clicked, [occView]() {
		auto state = occView->GetCameraRotationState();
		if (state)
		{
			Handle(V3d_View) view = occView->getView();
			view->SetProj(V3d_Ypos);
			occView->fitAll();
		}
		});
	QObject::connect(m_ZBtn, &QPushButton::clicked, [occView]() {
		auto state = occView->GetCameraRotationState();
		if (state)
		{
			Handle(V3d_View) view = occView->getView();
			view->SetProj(V3d_Zpos);
			occView->fitAll();
		}
		});
	QObject::connect(m__XBtn, &QPushButton::clicked, [occView]() {
		auto state = occView->GetCameraRotationState();
		if (state)
		{
			Handle(V3d_View) view = occView->getView();
			view->SetProj(V3d_Xneg);
			occView->fitAll();
		}
		});
	QObject::connect(m__YBtn, &QPushButton::clicked, [occView]() {
		auto state = occView->GetCameraRotationState();
		if (state)
		{
			Handle(V3d_View) view = occView->getView();
			view->SetProj(V3d_Yneg);
			occView->fitAll();
		}
		});
	QObject::connect(m__ZBtn, &QPushButton::clicked, [occView]() {
		auto state = occView->GetCameraRotationState();
		if (state)
		{
			Handle(V3d_View) view = occView->getView();
			view->SetProj(V3d_Zneg);
			occView->fitAll();
		}
		});
}


void deleteWidget(QLayout* layout)
{
	if (layout) {
		for (int i = layout->count() - 1; i >= 0; --i) {
			QLayoutItem* item = layout->itemAt(i);
			QWidget* widget = item->widget();
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

	QWidget* widget_1 = chartWidMap.value(1);
	QSplineSeries* series_1 = new QSplineSeries();
	series_1->setName("spline");
	series_1->append(10, 36);
	series_1->append(12, 54);
	series_1->append(13, 78);
	series_1->append(17, 34);
	series_1->append(20, 15);
	QChart* chart_1 = new QChart();
	chart_1->legend()->hide();//隐藏图例
	chart_1->addSeries(series_1);//添加数据
	chart_1->setTitle("Simple example");//标题
	chart_1->createDefaultAxes();//坐标系
	chart_1->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_1->setBackgroundBrush(QBrush(gradient));
	chart_1->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis* xAxis_1 = chart_1->axisX();
	QAbstractAxis* yAxis_1 = chart_1->axisY();
	xAxis_1->setLabelsColor(Qt::white);
	yAxis_1->setLabelsColor(Qt::white);
	QChartView* chartView_1 = new QChartView();
	chartView_1->setChart(chart_1);
	chartView_1->resize(widget_1->size());
	chartView_1->setRenderHint(QPainter::Antialiasing);
	QLayout* layout_1 = widget_1->layout();
	deleteWidget(layout_1);
	widget_1->layout()->addWidget(chartView_1);

	QWidget* widget_2 = chartWidMap.value(2);
	QSplineSeries* series_2 = new QSplineSeries();
	series_2->setName("spline");
	series_2->append(13, 36);
	series_2->append(22, 64);
	series_2->append(36, 88);
	series_2->append(77, 24);
	series_2->append(90, 5);
	QChart* chart_2 = new QChart();
	chart_2->legend()->hide();//隐藏图例
	chart_2->addSeries(series_2);//添加数据
	chart_2->setTitle("Simple example");//标题
	chart_2->createDefaultAxes();//坐标系
	chart_2->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_2->setBackgroundBrush(QBrush(gradient));
	chart_2->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis* xAxis_2 = chart_2->axisX();
	QAbstractAxis* yAxis_2 = chart_2->axisY();
	xAxis_2->setLabelsColor(Qt::white);
	yAxis_2->setLabelsColor(Qt::white);
	QChartView* chartView_2 = new QChartView();
	chartView_2->setChart(chart_2);
	chartView_2->resize(widget_2->size());
	chartView_2->setRenderHint(QPainter::Antialiasing);
	QLayout* layout_2 = widget_2->layout();
	deleteWidget(layout_2);
	widget_2->layout()->addWidget(chartView_2);

	QWidget* widget_3 = chartWidMap.value(3);
	QSplineSeries* series_3 = new QSplineSeries();
	series_3->setName("spline");
	series_3->append(20, 36);
	series_3->append(42, 54);
	series_3->append(63, 18);
	series_3->append(77, 84);
	series_3->append(80, 55);
	QChart* chart_3 = new QChart();
	chart_3->legend()->hide();//隐藏图例
	chart_3->addSeries(series_3);//添加数据
	chart_3->setTitle("Simple example");//标题
	chart_3->createDefaultAxes();//坐标系
	chart_3->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_3->setBackgroundBrush(QBrush(gradient));
	chart_3->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis* xAxis_3 = chart_3->axisX();
	QAbstractAxis* yAxis_3 = chart_3->axisY();
	xAxis_3->setLabelsColor(Qt::white);
	yAxis_3->setLabelsColor(Qt::white);
	QChartView* chartView_3 = new QChartView();
	chartView_3->setChart(chart_3);
	chartView_3->resize(widget_3->size());
	chartView_3->setRenderHint(QPainter::Antialiasing);
	QLayout* layout_3 = widget_3->layout();
	deleteWidget(layout_3);
	widget_3->layout()->addWidget(chartView_3);

	QWidget* widget_4 = chartWidMap.value(4);
	QSplineSeries* series_4 = new QSplineSeries();
	series_4->setName("spline");
	series_4->append(2, 56);
	series_4->append(23, 74);
	series_4->append(43, 28);
	series_4->append(67, 94);
	series_4->append(80, 100);
	QChart* chart_4 = new QChart();
	chart_4->legend()->hide();//隐藏图例
	chart_4->addSeries(series_4);//添加数据
	chart_4->setTitle("Simple example");//标题
	chart_4->createDefaultAxes();//坐标系
	chart_4->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_4->setBackgroundBrush(QBrush(gradient));
	chart_4->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis* xAxis_4 = chart_4->axisX();
	QAbstractAxis* yAxis_4 = chart_4->axisY();
	xAxis_4->setLabelsColor(Qt::white);
	yAxis_4->setLabelsColor(Qt::white);
	QChartView* chartView_4 = new QChartView();
	chartView_4->setChart(chart_4);
	chartView_4->resize(widget_4->size());
	chartView_4->setRenderHint(QPainter::Antialiasing);
	QLayout* layout_4 = widget_4->layout();
	deleteWidget(layout_4);
	widget_4->layout()->addWidget(chartView_4);

	QWidget* widget_5 = chartWidMap.value(5);
	QSplineSeries* series_5 = new QSplineSeries();
	series_5->setName("spline");
	series_5->append(7, 36);
	series_5->append(43, 14);
	series_5->append(53, 48);
	series_5->append(77, 64);
	series_5->append(87, 85);
	QChart* chart_5 = new QChart();
	chart_5->legend()->hide();//隐藏图例
	chart_5->addSeries(series_5);//添加数据
	chart_5->setTitle("Simple example");//标题
	chart_5->createDefaultAxes();//坐标系
	chart_5->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_5->setBackgroundBrush(QBrush(gradient));
	chart_5->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis* xAxis_5 = chart_5->axisX();
	QAbstractAxis* yAxis_5 = chart_5->axisY();
	xAxis_5->setLabelsColor(Qt::white);
	yAxis_5->setLabelsColor(Qt::white);
	QChartView* chartView_5 = new QChartView();
	chartView_5->setChart(chart_5);
	chartView_5->resize(widget_5->size());
	chartView_5->setRenderHint(QPainter::Antialiasing);
	QLayout* layout_5 = widget_5->layout();
	deleteWidget(layout_5);
	widget_5->layout()->addWidget(chartView_5);

	QWidget* widget_6 = chartWidMap.value(6);
	QSplineSeries* series_6 = new QSplineSeries();
	series_6->setName("spline");
	series_6->append(0, 0);
	series_6->append(32, 34);
	series_6->append(53, 48);
	series_6->append(78, 47);
	series_6->append(90, 95);
	QChart* chart_6 = new QChart();
	chart_6->legend()->hide();//隐藏图例
	chart_6->addSeries(series_6);//添加数据
	chart_6->setTitle("Simple example");//标题
	chart_6->createDefaultAxes();//坐标系
	chart_6->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_6->setBackgroundBrush(QBrush(gradient));
	chart_6->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis* xAxis_6 = chart_6->axisX();
	QAbstractAxis* yAxis_6 = chart_6->axisY();
	xAxis_6->setLabelsColor(Qt::white);
	yAxis_6->setLabelsColor(Qt::white);
	QChartView* chartView_6 = new QChartView();
	chartView_6->setChart(chart_6);
	chartView_6->resize(widget_6->size());
	chartView_6->setRenderHint(QPainter::Antialiasing);
	QLayout* layout_6 = widget_6->layout();
	deleteWidget(layout_6);
	widget_6->layout()->addWidget(chartView_6);

	QWidget* widget_7 = chartWidMap.value(7);
	QSplineSeries* series_7 = new QSplineSeries();
	series_7->setName("spline");
	series_7->append(0, 36);
	series_7->append(34, 84);
	series_7->append(45, 48);
	series_7->append(57, 24);
	series_7->append(68, 95);
	QChart* chart_7 = new QChart();
	chart_7->legend()->hide();//隐藏图例
	chart_7->addSeries(series_7);//添加数据
	chart_7->setTitle("Simple example");//标题
	chart_7->createDefaultAxes();//坐标系
	chart_7->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_7->setBackgroundBrush(QBrush(gradient));
	chart_7->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis* xAxis_7 = chart_7->axisX();
	QAbstractAxis* yAxis_7 = chart_7->axisY();
	xAxis_7->setLabelsColor(Qt::white);
	yAxis_7->setLabelsColor(Qt::white);
	QChartView* chartView_7 = new QChartView();
	chartView_7->setChart(chart_7);
	chartView_7->resize(widget_7->size());
	chartView_7->setRenderHint(QPainter::Antialiasing);
	QLayout* layout_7 = widget_7->layout();
	deleteWidget(layout_7);
	widget_7->layout()->addWidget(chartView_7);

	QWidget* widget_8 = chartWidMap.value(8);
	QSplineSeries* series_8 = new QSplineSeries();
	series_8->setName("spline");
	series_8->append(0, 66);
	series_8->append(32, 94);
	series_8->append(43, 18);
	series_8->append(67, 84);
	series_8->append(80, 5);
	QChart* chart_8 = new QChart();
	chart_8->legend()->hide();//隐藏图例
	chart_8->addSeries(series_8);//添加数据
	chart_8->setTitle("Simple example");//标题
	chart_8->createDefaultAxes();//坐标系
	chart_8->axes(Qt::Vertical).first()->setRange(0, 100);//坐标系范围
	chart_8->setBackgroundBrush(QBrush(gradient));
	chart_8->setTitleBrush(Qt::white); // 标题文字颜色
	// 获取X轴和Y轴对象
	QAbstractAxis* xAxis_8 = chart_8->axisX();
	QAbstractAxis* yAxis_8 = chart_8->axisY();
	xAxis_8->setLabelsColor(Qt::white);
	yAxis_8->setLabelsColor(Qt::white);
	QChartView* chartView_8 = new QChartView();
	chartView_8->setChart(chart_8);
	chartView_8->resize(widget_8->size());
	chartView_8->setRenderHint(QPainter::Antialiasing);
	QLayout* layout_8 = widget_8->layout();
	deleteWidget(layout_8);
	widget_8->layout()->addWidget(chartView_8);



	/////////////////////// 中间比例图
	QWidget* widget_0 = chartWidMap.value(0);
	const qreal angularMin = 0;
	const qreal angularMax = 360;
	const qreal radialMin = 0;
	const qreal radialMax = 360;


	qreal ad6 = (angularMax - angularMin) / 8;

	QLineSeries* seriesdata = new QLineSeries();
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



	QAreaSeries* seriesArea = new QAreaSeries();
	seriesArea->setUpperSeries(seriesdata);
	seriesArea->setOpacity(0.5);

	QPolarChart* chart = new QPolarChart();
	chart->setPlotArea(QRectF(0, 2500, 0, 0)); // 上移50个单位


	chart->addSeries(seriesdata);
	chart->addSeries(seriesArea);


	QCategoryAxis* angularAxis = new QCategoryAxis();
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
	QValueAxis* radialAxis = new QValueAxis();

	radialAxis->setLabelFormat("");
	chart->addAxis(radialAxis, QPolarChart::PolarOrientationRadial);

	angularAxis->setLineVisible(false);
	angularAxis->setLabelsColor(Qt::white);
	chart->axes(QPolarChart::PolarOrientationRadial).at(0)->setVisible(false);
	seriesArea->attachAxis(radialAxis);
	seriesArea->attachAxis(angularAxis);
	for (int i = 0; i <= 2; ++i)
	{
		QLineSeries* seriesLineTemp = new QLineSeries();
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

	foreach(QLegendMarker * marker, chart->legend()->markers())
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
	QAbstractAxis* xAxis_0 = chart->axisX();
	QAbstractAxis* yAxis_0 = chart->axisY();
	xAxis_0->setLabelsColor(Qt::white);
	yAxis_0->setLabelsColor(Qt::white);
	QChartView* chartView_0 = new QChartView();
	chartView_0->setChart(chart);
	chartView_0->resize(440, 400);
	chartView_0->move(chartView_0->x(), chartView_0->y() - 100);
	chartView_0->setRenderHint(QPainter::Antialiasing);
	QLayout* layout_0 = widget_0->layout();
	deleteWidget(layout_0);
	widget_0->layout()->addWidget(chartView_0);
}


void mainWidget::deleteWidget(QLayout* layout)
{
	if (layout) {
		for (int i = layout->count() - 1; i >= 0; --i) {
			QLayoutItem* item = layout->itemAt(i);
			QWidget* widget = item->widget();
			if (widget) {
				delete widget;
			}
			else {
				delete item;
			}
		}
	}
}

void mainWidget::refreshMemoryUsage(QLabel* m_statusLabel) {
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

void mainWidget::getMemoryUsage(QLabel* m_statusLabel) {
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