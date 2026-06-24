#pragma execution_character_set("utf-8")
#include "GFTreeModelWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QApplication>
#include <QIcon>
#include <QFileDialog>
#include <QDateTime>
#include <QRegExp>
#include <QRegularExpression> 
#include <QValidator>
#include <QThread>
#include <QTimer>
#include <algorithm>

#include <AIS_Shape.hxx>
#include <AIS_ColorScale.hxx>

#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Builder.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAlgo_FaceRestrictor.hxx>
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <BRepProj_Projection.hxx>
#include <BRepGProp.hxx>


#include <GProp_GProps.hxx>

#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>

#include <MeshVS_Mesh.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>

#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Drawer.hxx>
#include <Quantity_ColorRGBA.hxx>

#include <RWStl.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI_Reader.hxx>

#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopExp_Explorer.hxx>
#include <TColStd_HArray2OfInteger.hxx>
#include <TColStd_HArray2OfReal.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>


#include <V3d_View.hxx>
#include <V3d_Viewer.hxx> 




#include "GFImportModelWidget.h"
#include "TriangleStructure.h"
#include "occView.h"
#include "ModelDataManager.h"
#include "ProgressDialog.h"
#include "GeometryImportWorker.h"
#include "WordExporterWorker.h"
#include "APICreateMidSurfaceHelper.h"


#include <QScreen>
#include <QtCore/qstandardpaths.h>
#include "TriangulationWorker.h"
#include "APICalculateHepler.h"
#include "CalculateWorker.h"
#include <BRepPrimAPI_MakeSphere.hxx>




GFTreeModelWidget::GFTreeModelWidget(QWidget*parent)
	:QWidget(parent)
{
	qRegisterMetaType<ModelGeometryInfo>("ModelGeometryInfo");
	qRegisterMetaType<ModelMeshInfo>("ModelMeshInfo");

	init();
	bindConnect();
}

GFTreeModelWidget::~GFTreeModelWidget()
{
}

void GFTreeModelWidget::init()
{
	wordExporter = new WordExporter(this);

	QIcon error_icon(":/src/Error.svg");
	QIcon checked_icon(":/src/Checked.svg");

	m_treeWidget = new GFTreeWidget(this);
	//treeWidget->setStyleSheet(R"(
	// QTreeWidget::branch:has-children:!has-siblings:closed,
	// QTreeWidget::branch:closed:has-children:has-siblings {
	//         border-image: none;
	//         image: url(:/src/treeclose.svg);
	// }
	//
	// QTreeWidget::branch:open:has-children:!has-siblings,
	// QTreeWidget::branch:open:has-children:has-siblings  {
	//         border-image: none;
	//         image: url(:/src/treeopen.svg);
	// }
	//)");

	m_treeWidget->setColumnCount(1);
	m_treeWidget->setHeaderLabels({ "项目结构" });
	m_treeWidget->setHeaderHidden(true);

	// 创建根节点
	QTreeWidgetItem* rootItem = new QTreeWidgetItem(m_treeWidget);
	rootItem->setText(0, "工程文件");
	rootItem->setData(0, Qt::UserRole, "Project");
	rootItem->setExpanded(true);
	//rootItem->setIcon(0, icon);

	// 几何模型节点
	QTreeWidgetItem* geometryNode = new QTreeWidgetItem(rootItem);
	geometryNode->setText(0, "固体发动机三维模型导入");
	geometryNode->setData(0, Qt::UserRole, "Geometry");
	geometryNode->setIcon(0, error_icon);
	{
		QTreeWidgetItem* shellShape = new QTreeWidgetItem();
		shellShape->setText(0, "壳体");
		shellShape->setData(0, Qt::UserRole, "Shell");
		shellShape->setIcon(0, error_icon);

		QTreeWidgetItem* propellantShape = new QTreeWidgetItem();
		propellantShape->setText(0, "推进剂");
		propellantShape->setData(0, Qt::UserRole, "Propellant");
		propellantShape->setIcon(0, error_icon);

		QTreeWidgetItem* heatInsulatingLayerShape = new QTreeWidgetItem();
		heatInsulatingLayerShape->setText(0, "绝热层");
		heatInsulatingLayerShape->setData(0, Qt::UserRole, "HeatInsulatingLayer");
		heatInsulatingLayerShape->setIcon(0, error_icon);

		geometryNode->addChild(shellShape);
		geometryNode->addChild(propellantShape);
		geometryNode->addChild(heatInsulatingLayerShape);
		geometryNode->setExpanded(true);
	}
	// 材料节点
	QTreeWidgetItem* databaseNode = new QTreeWidgetItem(rootItem);
	databaseNode->setText(0, "数据库");
	databaseNode->setData(0, Qt::UserRole, "Database");
	databaseNode->setIcon(0, error_icon);
	databaseNode->setExpanded(true);

	QTreeWidgetItem* materialNode = new QTreeWidgetItem();
	materialNode->setText(0, "材料数据库");
	materialNode->setData(0, Qt::UserRole, "Material");
	materialNode->setIcon(0, error_icon);

	databaseNode->addChild(materialNode);


	QTreeWidgetItem* steel = new QTreeWidgetItem();
	steel->setText(0, "壳体材料");
	steel->setData(0, Qt::UserRole, "Steel");
	steel->setIcon(0, error_icon);

	QTreeWidgetItem* propellant = new QTreeWidgetItem();
	propellant->setText(0, "含能材料");
	propellant->setData(0, Qt::UserRole, "Propellant");
	propellant->setIcon(0, error_icon);

	QTreeWidgetItem* outheat = new QTreeWidgetItem();
	outheat->setText(0, "外防热材料");
	outheat->setData(0, Qt::UserRole, "Outheat");
	outheat->setIcon(0, error_icon);

	QTreeWidgetItem* insulatingheat = new QTreeWidgetItem();
	insulatingheat->setText(0, "绝热层材料");
	insulatingheat->setData(0, Qt::UserRole, "Insulatingheat");
	insulatingheat->setIcon(0, error_icon);

	materialNode->addChild(steel);
	materialNode->addChild(propellant);
	materialNode->addChild(outheat);
	materialNode->addChild(insulatingheat);

	materialNode->setExpanded(true);

	QTreeWidgetItem* judgment = new QTreeWidgetItem();
	judgment->setText(0, "标准数据库");
	judgment->setData(0, Qt::UserRole, "Judgment");
	judgment->setIcon(0, error_icon);

	QTreeWidgetItem* calculation = new QTreeWidgetItem();
	calculation->setText(0, "计算模型数据库");
	calculation->setData(0, Qt::UserRole, "Calculation");
	calculation->setIcon(0, checked_icon);


	databaseNode->addChild(judgment);
	databaseNode->addChild(calculation);

	//网格节点
	QTreeWidgetItem* meshItem = new QTreeWidgetItem(rootItem);
	meshItem->setText(0, "网格");
	meshItem->setData(0, Qt::UserRole, "Mesh");
	meshItem->setIcon(0, error_icon);
	{
		QTreeWidgetItem* stressShellMesh = new QTreeWidgetItem();
		stressShellMesh->setText(0, "壳体");
		stressShellMesh->setData(0, Qt::UserRole, "ShellMesh");
		stressShellMesh->setIcon(0, error_icon);

		QTreeWidgetItem* stressPropellantMesh = new QTreeWidgetItem();
		stressPropellantMesh->setText(0, "推进剂");
		stressPropellantMesh->setData(0, Qt::UserRole, "Propellant");
		stressPropellantMesh->setIcon(0, error_icon);

		QTreeWidgetItem* heatInsulatingLayerMesh = new QTreeWidgetItem();
		heatInsulatingLayerMesh->setText(0, "绝热层");
		heatInsulatingLayerMesh->setData(0, Qt::UserRole, "HeatInsulatingLayer");
		heatInsulatingLayerMesh->setIcon(0, error_icon);

		meshItem->addChild(stressShellMesh);
		meshItem->addChild(stressPropellantMesh);
		meshItem->addChild(heatInsulatingLayerMesh);
		meshItem->setExpanded(true);
	}

	// 分析设置节点
	QTreeWidgetItem* analysisNode = new QTreeWidgetItem(rootItem);
	analysisNode->setText(0, "安全特性参数分析");
	analysisNode->setData(0, Qt::UserRole, "Analysis");
	analysisNode->setIcon(0, error_icon);
	analysisNode->setExpanded(true);

	QTreeWidgetItem* fallAnalysis = new QTreeWidgetItem();
	fallAnalysis->setText(0, "1.跌落安全性分析");
	fallAnalysis->setData(0, Qt::UserRole, "FallAnalysis");
	//fallAnalysis->setCheckState(0, Qt::Unchecked);
	fallAnalysis->setIcon(0, error_icon);


	QTreeWidgetItem* stressResult = new QTreeWidgetItem();
	stressResult->setText(0, "应力分析");
	stressResult->setData(0, Qt::UserRole, "StressResult");
	stressResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* stressShellResult = new QTreeWidgetItem();
		stressShellResult->setText(0, "壳体");
		stressShellResult->setData(0, Qt::UserRole, "FallStressShellResult");
		stressShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* stressPropellantResult = new QTreeWidgetItem();
		stressPropellantResult->setText(0, "推进剂");
		stressPropellantResult->setData(0, Qt::UserRole, "FallStressPropellantResult");
		stressPropellantResult->setIcon(0, error_icon);

		stressResult->addChild(stressShellResult);
		stressResult->addChild(stressPropellantResult);
	}

	QTreeWidgetItem* strainResult = new QTreeWidgetItem();
	strainResult->setText(0, "应变分析");
	strainResult->setData(0, Qt::UserRole, "StrainResult");
	strainResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* strainShellResult = new QTreeWidgetItem();
		strainShellResult->setText(0, "壳体");
		strainShellResult->setData(0, Qt::UserRole, "FallStrainShellResult");
		strainShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* strainPropellantResult = new QTreeWidgetItem();
		strainPropellantResult->setText(0, "推进剂");
		strainPropellantResult->setData(0, Qt::UserRole, "FallStrainPropellantResult");
		strainPropellantResult->setIcon(0, error_icon);

		strainResult->addChild(strainShellResult);
		strainResult->addChild(strainPropellantResult);
	}

	QTreeWidgetItem* temperatureResult = new QTreeWidgetItem();
	temperatureResult->setText(0, "温度分析");
	temperatureResult->setData(0, Qt::UserRole, "TemperatureResult");
	temperatureResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* tempShellResult = new QTreeWidgetItem();
		tempShellResult->setText(0, "壳体");
		tempShellResult->setData(0, Qt::UserRole, "FallTemperatureShellResult");
		tempShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* tempPropellantResult = new QTreeWidgetItem();
		tempPropellantResult->setText(0, "推进剂");
		tempPropellantResult->setData(0, Qt::UserRole, "FallTemperaturePropellantResult");
		tempPropellantResult->setIcon(0, error_icon);

		temperatureResult->addChild(tempShellResult);
		temperatureResult->addChild(tempPropellantResult);
	}

	QTreeWidgetItem* overpressureResult = new QTreeWidgetItem();
	overpressureResult->setText(0, "超压分析");
	overpressureResult->setData(0, Qt::UserRole, "OverpressureResult");
	overpressureResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* overpressureShellResult = new QTreeWidgetItem();
		overpressureShellResult->setText(0, "壳体");
		overpressureShellResult->setData(0, Qt::UserRole, "FallOverpressureShellResult");
		overpressureShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* overpressurePropellantResult = new QTreeWidgetItem();
		overpressurePropellantResult->setText(0, "推进剂");
		overpressurePropellantResult->setData(0, Qt::UserRole, "FallOverpressurePropellantResult");
		overpressurePropellantResult->setIcon(0, error_icon);

		overpressureResult->addChild(overpressureShellResult);
		overpressureResult->addChild(overpressurePropellantResult);
	}

	fallAnalysis->addChild(stressResult);
	fallAnalysis->addChild(strainResult);
	fallAnalysis->addChild(temperatureResult);
	fallAnalysis->addChild(overpressureResult);

	QTreeWidgetItem* fastCombustionAnalysis = new QTreeWidgetItem();
	fastCombustionAnalysis->setText(0, "2.快速烤燃安全性分析");
	fastCombustionAnalysis->setData(0, Qt::UserRole, "FastCombustionAnalysis");
	//fastCombustionAnalysis->setCheckState(0, Qt::Unchecked);
	fastCombustionAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* fastCombustionTemperatureResult = new QTreeWidgetItem();
	fastCombustionTemperatureResult->setText(0, "温度分析");
	fastCombustionTemperatureResult->setData(0, Qt::UserRole, "FastCombustionTemperatureResult");
	fastCombustionTemperatureResult->setIcon(0, error_icon);

	fastCombustionAnalysis->addChild(fastCombustionTemperatureResult);


	QTreeWidgetItem* slowCombustionAnalysis = new QTreeWidgetItem();
	slowCombustionAnalysis->setText(0, "3.慢速烤燃安全性分析");
	slowCombustionAnalysis->setData(0, Qt::UserRole, "SlowCombustionAnalysis");
	//slowCombustionAnalysis->setCheckState(0, Qt::Unchecked);
	slowCombustionAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* slowCombustionTemperatureResult = new QTreeWidgetItem();
	slowCombustionTemperatureResult->setText(0, "温度分析");
	slowCombustionTemperatureResult->setData(0, Qt::UserRole, "SlowCombustionTemperatureResult");
	slowCombustionTemperatureResult->setIcon(0, error_icon);

	slowCombustionAnalysis->addChild(slowCombustionTemperatureResult);


	QTreeWidgetItem* shootAnalysis = new QTreeWidgetItem();
	shootAnalysis->setText(0, "4.枪击安全性分析");
	shootAnalysis->setData(0, Qt::UserRole, "ShootAnalysis");
	//shootAnalysis->setCheckState(0, Qt::Unchecked);
	shootAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* shootStressResult = new QTreeWidgetItem();
	shootStressResult->setText(0, "应力分析");
	shootStressResult->setData(0, Qt::UserRole, "ShootStressResult");
	shootStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* shootStrainResult = new QTreeWidgetItem();
	shootStrainResult->setText(0, "应变分析");
	shootStrainResult->setData(0, Qt::UserRole, "ShootStrainResult");
	shootStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* shootTemperatureResult = new QTreeWidgetItem();
	shootTemperatureResult->setText(0, "温度分析");
	shootTemperatureResult->setData(0, Qt::UserRole, "ShootTemperatureResult");
	shootTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* shootOverpressureResult = new QTreeWidgetItem();
	shootOverpressureResult->setText(0, "超压分析");
	shootOverpressureResult->setData(0, Qt::UserRole, "ShootOverpressureResult");
	shootOverpressureResult->setIcon(0, error_icon);

	shootAnalysis->addChild(shootStressResult);
	shootAnalysis->addChild(shootStrainResult);
	shootAnalysis->addChild(shootTemperatureResult);
	shootAnalysis->addChild(shootOverpressureResult);


	QTreeWidgetItem* jetImpactAnalysis = new QTreeWidgetItem();
	jetImpactAnalysis->setText(0, "5.射流冲击安全性分析");
	jetImpactAnalysis->setData(0, Qt::UserRole, "JetImpactAnalysis");
	//jetImpactAnalysis->setCheckState(0, Qt::Unchecked);
	jetImpactAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* jetImpactStressResult = new QTreeWidgetItem();
	jetImpactStressResult->setText(0, "应力分析");
	jetImpactStressResult->setData(0, Qt::UserRole, "JetImpactStressResult");
	jetImpactStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* jetStrainResult = new QTreeWidgetItem();
	jetStrainResult->setText(0, "应变分析");
	jetStrainResult->setData(0, Qt::UserRole, "JetImpactStrainResult");
	jetStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* jetImpactTemperatureResult = new QTreeWidgetItem();
	jetImpactTemperatureResult->setText(0, "温度分析");
	jetImpactTemperatureResult->setData(0, Qt::UserRole, "JetImpactTemperatureResult");
	jetImpactTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* jetImpactOverpressureResult = new QTreeWidgetItem();
	jetImpactOverpressureResult->setText(0, "超压分析");
	jetImpactOverpressureResult->setData(0, Qt::UserRole, "JetImpactOverpressureResult");
	jetImpactOverpressureResult->setIcon(0, error_icon);

	jetImpactAnalysis->addChild(jetImpactStressResult);
	jetImpactAnalysis->addChild(jetStrainResult);
	jetImpactAnalysis->addChild(jetImpactTemperatureResult);
	jetImpactAnalysis->addChild(jetImpactOverpressureResult);


	QTreeWidgetItem* fragmentationImpactAnalysis = new QTreeWidgetItem();
	fragmentationImpactAnalysis->setText(0, "6.破片撞击安全性分析");
	fragmentationImpactAnalysis->setData(0, Qt::UserRole, "FragmentationImpactAnalysis");
	//fragmentationImpactAnalysis->setCheckState(0, Qt::Unchecked);
	fragmentationImpactAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* fragmentationImpactStressResult = new QTreeWidgetItem();
	fragmentationImpactStressResult->setText(0, "应力分析");
	fragmentationImpactStressResult->setData(0, Qt::UserRole, "FragmentationImpactStressResult");
	fragmentationImpactStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* fragmentationStrainResult = new QTreeWidgetItem();
	fragmentationStrainResult->setText(0, "应变分析");
	fragmentationStrainResult->setData(0, Qt::UserRole, "FragmentationImpactStrainResult");
	fragmentationStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* fragmentationImpactTemperatureResult = new QTreeWidgetItem();
	fragmentationImpactTemperatureResult->setText(0, "温度分析");
	fragmentationImpactTemperatureResult->setData(0, Qt::UserRole, "FragmentationImpactTemperatureResult");
	fragmentationImpactTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* fragmentationImpactOverpressureResult = new QTreeWidgetItem();
	fragmentationImpactOverpressureResult->setText(0, "超压分析");
	fragmentationImpactOverpressureResult->setData(0, Qt::UserRole, "FragmentationImpactOverpressureResult");
	fragmentationImpactOverpressureResult->setIcon(0, error_icon);

	fragmentationImpactAnalysis->addChild(fragmentationImpactStressResult);
	fragmentationImpactAnalysis->addChild(fragmentationStrainResult);
	fragmentationImpactAnalysis->addChild(fragmentationImpactTemperatureResult);
	fragmentationImpactAnalysis->addChild(fragmentationImpactOverpressureResult);


	QTreeWidgetItem* explosiveBlastAnalysis = new QTreeWidgetItem();
	explosiveBlastAnalysis->setText(0, "7.爆炸冲击波安全性分析");
	explosiveBlastAnalysis->setData(0, Qt::UserRole, "ExplosiveBlastAnalysis");
	//explosiveBlastAnalysis->setCheckState(0, Qt::Unchecked);
	explosiveBlastAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* explosiveBlastStressResult = new QTreeWidgetItem();
	explosiveBlastStressResult->setText(0, "应力分析");
	explosiveBlastStressResult->setData(0, Qt::UserRole, "ExplosiveBlastStressResult");
	explosiveBlastStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* explosiveBlastStrainResult = new QTreeWidgetItem();
	explosiveBlastStrainResult->setText(0, "应变分析");
	explosiveBlastStrainResult->setData(0, Qt::UserRole, "ExplosiveBlastStrainResult");
	explosiveBlastStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* explosiveBlastTemperatureResult = new QTreeWidgetItem();
	explosiveBlastTemperatureResult->setText(0, "温度分析");
	explosiveBlastTemperatureResult->setData(0, Qt::UserRole, "ExplosiveBlastTemperatureResult");
	explosiveBlastTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* explosiveBlastOverpressureResult = new QTreeWidgetItem();
	explosiveBlastOverpressureResult->setText(0, "超压分析");
	explosiveBlastOverpressureResult->setData(0, Qt::UserRole, "ExplosiveBlastOverpressureResult");
	explosiveBlastOverpressureResult->setIcon(0, error_icon);

	explosiveBlastAnalysis->addChild(explosiveBlastStressResult);
	explosiveBlastAnalysis->addChild(explosiveBlastStrainResult);
	explosiveBlastAnalysis->addChild(explosiveBlastTemperatureResult);
	explosiveBlastAnalysis->addChild(explosiveBlastOverpressureResult);


	QTreeWidgetItem* sacrificeExplosionAnalysis = new QTreeWidgetItem();
	sacrificeExplosionAnalysis->setText(0, "8.殉爆安全性分析");
	sacrificeExplosionAnalysis->setData(0, Qt::UserRole, "SacrificeExplosionAnalysis");
	//sacrificeExplosionAnalysis->setCheckState(0, Qt::Unchecked);
	sacrificeExplosionAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* sacrificeExplosionStressResult = new QTreeWidgetItem();
	sacrificeExplosionStressResult->setText(0, "应力分析");
	sacrificeExplosionStressResult->setData(0, Qt::UserRole, "SacrificeExplosioStressResult");
	sacrificeExplosionStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* sacrificeExplosionStrainResult = new QTreeWidgetItem();
	sacrificeExplosionStrainResult->setText(0, "应变分析");
	sacrificeExplosionStrainResult->setData(0, Qt::UserRole, "SacrificeExplosioStrainResult");
	sacrificeExplosionStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* sacrificeExplosionTemperatureResult = new QTreeWidgetItem();
	sacrificeExplosionTemperatureResult->setText(0, "温度分析");
	sacrificeExplosionTemperatureResult->setData(0, Qt::UserRole, "SacrificeExplosioTemperatureResult");
	sacrificeExplosionTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* sacrificeExplosionOverpressureResult = new QTreeWidgetItem();
	sacrificeExplosionOverpressureResult->setText(0, "超压分析");
	sacrificeExplosionOverpressureResult->setData(0, Qt::UserRole, "SacrificeExplosioOverpressureResult");
	sacrificeExplosionOverpressureResult->setIcon(0, error_icon);

	sacrificeExplosionAnalysis->addChild(sacrificeExplosionStressResult);
	sacrificeExplosionAnalysis->addChild(sacrificeExplosionStrainResult);
	sacrificeExplosionAnalysis->addChild(sacrificeExplosionTemperatureResult);
	sacrificeExplosionAnalysis->addChild(sacrificeExplosionOverpressureResult);


	analysisNode->addChild(fallAnalysis);
	analysisNode->addChild(fastCombustionAnalysis);
	analysisNode->addChild(slowCombustionAnalysis);

	analysisNode->addChild(shootAnalysis);
	analysisNode->addChild(jetImpactAnalysis);
	analysisNode->addChild(fragmentationImpactAnalysis);
	analysisNode->addChild(explosiveBlastAnalysis);
	analysisNode->addChild(sacrificeExplosionAnalysis);


	//// 安全特性分析
	//QTreeWidgetItem* paramAnalyNode = new QTreeWidgetItem(rootItem);
	//paramAnalyNode->setText(0, "安全特性分析");
	//paramAnalyNode->setData(0, Qt::UserRole, "Results");
	//paramAnalyNode->setIcon(0, error_icon);

	//QTreeWidgetItem* paramAnalyResult = new QTreeWidgetItem();
	//paramAnalyResult->setText(0, "分析报告");
	//paramAnalyResult->setData(0, Qt::UserRole, "paramAnalyResult");
	//paramAnalyResult->setIcon(0, error_icon);

	//paramAnalyNode->addChild(paramAnalyResult);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(m_treeWidget);
	layout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(layout);
}

void GFTreeModelWidget::bindConnect()
{
	connect(m_treeWidget, &QTreeWidget::itemClicked, this, &GFTreeModelWidget::onTreeItemClicked);
}

void GFTreeModelWidget::onTreeItemClicked(QTreeWidgetItem* item, int column)
{
	UserInfo userinfo = ModelDataManager::GetInstance()->GetUserInfo();
	QString workdir = userinfo.workdir;

	QString itemData = item->data(0, Qt::UserRole).toString();
	emit itemClicked(itemData);

	if (itemData.contains("StressResult")|| itemData.contains("StrainResult") || itemData.contains("TemperatureResult") || itemData.contains("OverpressureResult") )
	{
		QWidget* parent = parentWidget();
		while (parent) {
			GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
			if (gfParent)
			{
				// 截图结果云图
				QString m_privateDirPath = "";
				if (itemData == "StressResult")
				{
					m_privateDirPath = workdir + "/template/fall/Stress.png";
				}
				else if (itemData == "StrainResult")
				{
					m_privateDirPath = workdir + "/template/fall/Strain.png";
				}
				else if (itemData == "TemperatureResult")
				{
					m_privateDirPath = workdir + "/template/fall/Temperature.png";
				}
				else if (itemData == "OverpressureResult")
				{
					m_privateDirPath = workdir + "/template/fall/Overpressure.png";
				}

				else if (itemData == "FastCombustionTemperatureResult")
				{
					m_privateDirPath = workdir + "/template/fastCombustion/Temperature.png";
				}

				else if (itemData == "SlowCombustionTemperatureResult")
				{
					m_privateDirPath = workdir + "/template/slowCombustion/Temperature.png";
				}

				if (itemData == "ShootStressResult")
				{
					m_privateDirPath = workdir + "/template/shoot/Stress.png";
				}
				else if (itemData == "ShootStrainResult")
				{
					m_privateDirPath = workdir + "/template/shoot/Strain.png";
				}
				else if (itemData == "ShootTemperatureResult")
				{
					m_privateDirPath = workdir + "/template/shoot/Temperature.png";
				}
				else if (itemData == "ShootOverpressureResult")
				{
					m_privateDirPath = workdir + "/template/shoot/Overpressure.png";
				}

				if (itemData == "JetImpactStressResult")
				{
					m_privateDirPath = workdir + "/template/jetImpact/Stress.png";
				}
				else if (itemData == "JetImpactStrainResult")
				{
					m_privateDirPath = workdir + "/template/jetImpact/Strain.png";
				}
				else if (itemData == "JetImpactTemperatureResult")
				{
					m_privateDirPath = workdir + "/template/jetImpact/Temperature.png";
				}
				else if (itemData == "JetImpactOverpressureResult")
				{
					m_privateDirPath = workdir + "/template/jetImpact/Overpressure.png";
				}

				if (itemData == "FragmentationImpactStressResult")
				{
					m_privateDirPath = workdir + "/template/fragmentationImpact/Stress.png";
				}
				else if (itemData == "FragmentationImpactStrainResult")
				{
					m_privateDirPath = workdir + "/template/fragmentationImpact/Strain.png";
				}
				else if (itemData == "FragmentationImpactTemperatureResult")
				{
					m_privateDirPath = workdir + "/template/fragmentationImpact/Temperature.png";
				}
				else if (itemData == "FragmentationImpactOverpressureResult")
				{
					m_privateDirPath = workdir + "/template/fragmentationImpact/Overpressure.png";
				}

				if (itemData == "ExplosiveBlastStressResult")
				{
					m_privateDirPath = workdir + "/template/explosiveBlast/Stress.png";
				}
				else if (itemData == "ExplosiveBlastStrainResult")
				{
					m_privateDirPath = workdir + "/template/explosiveBlast/Strain.png";
				}
				else if (itemData == "ExplosiveBlastTemperatureResult")
				{
					m_privateDirPath = workdir + "/template/explosiveBlast/Temperature.png";
				}
				else if (itemData == "ExplosiveBlastOverpressureResult")
				{
					m_privateDirPath = workdir + "/template/explosiveBlast/Overpressure.png";
				}

				if (itemData == "SacrificeExplosioStressResult")
				{
					m_privateDirPath = workdir + "/template/sacrificeExplosio/Stress.png";
				}
				else if (itemData == "SacrificeExplosioStrainResult")
				{
					m_privateDirPath = workdir + "/template/sacrificeExplosio/Strain.png";
				}
				else if (itemData == "SacrificeExplosioTemperatureResult")
				{
					m_privateDirPath = workdir + "/template/sacrificeExplosio/Temperature.png";
				}
				else if (itemData == "SacrificeExplosioOverpressureResult")
				{
					m_privateDirPath = workdir + "/template/sacrificeExplosio/Overpressure.png";
				}
				
				QDir privateDir(m_privateDirPath);
				wordExporter->captureWidgetToFile(gfParent->GetOccView(), m_privateDirPath);
				break;
			}
			else
			{
				parent = parent->parentWidget();
			}
		}
	}
}

void GFTreeModelWidget::updataIcon()
{
	QIcon error_icon(":/src/Error.svg");
	QIcon checked_icon(":/src/Checked.svg");

	auto ins=ModelDataManager::GetInstance();
	auto geomInfo = ins->GetModelGeometryInfo();
	auto meshInfo = ins->GetModelMeshInfo();

	auto steelInfo = ins->GetSteelPropertyInfo();
	auto propellantInfo = ins->GetPropellantPropertyInfo();
	auto calculationInfo = ins->GetCalculationPropertyInfo();
	auto judgementPropertyInfo = ins->GetJudgementPropertyInfo();
	auto insulatingheatPropertyInfo = ins->GetInsulatingheatPropertyInfo();
	auto outheatPropertyInfo = ins->GetOutheatPropertyInfo();

	int size = m_treeWidget->topLevelItemCount();
	QTreeWidgetItem *child;
	for (int i = 0; i < size; i++)
	{
		child = m_treeWidget->topLevelItem(i);
		int childCount = child->childCount();
		for (int j = 0; j < childCount; ++j)
		{
			if (child->child(j)->text(0).contains("固体发动机三维模型导入"))
			{
				if (geomInfo.path.isEmpty())
				{
					child->child(j)->setIcon(0, error_icon);
				}
				else
				{
					child->child(j)->setIcon(0, checked_icon);
				}
			}
			else if (child->child(j)->text(0).contains("网格"))
			{
				if (!meshInfo.isChecked)
				{
					child->child(j)->setIcon(0, error_icon);
				}
				else
				{
					child->child(j)->setIcon(0, checked_icon);
				}
			}
			else if (child->child(j)->text(0).contains("数据库"))
			{
				QTreeWidgetItem *clChild = child->child(j);
				int clChildCount = clChild->childCount();
				for (int m = 0; m < clChildCount; ++m) {

					if (clChild->child(m)->text(0).contains("标准数据库"))
					{
						if (!judgementPropertyInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					else if (clChild->child(m)->text(0).contains("材料数据库"))
					{
						QTreeWidgetItem *clChild_child = clChild->child(m);
						int clChildCount = clChild_child->childCount();
						for (int n = 0; n < clChildCount; ++n) {
							if (clChild_child->child(n)->text(0).contains("壳体材料"))
							{
								if (!steelInfo.isChecked)
								{
									clChild_child->child(n)->setIcon(0, error_icon);
								}
								else
								{
									clChild_child->child(n)->setIcon(0, checked_icon);
								}
							}
							if (clChild_child->child(n)->text(0).contains("含能材料"))
							{
								if (!propellantInfo.isChecked)
								{
									clChild_child->child(n)->setIcon(0, error_icon);
								}
								else
								{
									clChild_child->child(n)->setIcon(0, checked_icon);
								}
							}
							if (clChild_child->child(n)->text(0).contains("绝热层材料"))
							{
								if (!insulatingheatPropertyInfo.isChecked)
								{
									clChild_child->child(n)->setIcon(0, error_icon);
								}
								else
								{
									clChild_child->child(n)->setIcon(0, checked_icon);
								}
							}
							if (clChild_child->child(n)->text(0).contains("外防热材料"))
							{
								if (!outheatPropertyInfo.isChecked)
								{
									clChild_child->child(n)->setIcon(0, error_icon);
								}
								else
								{
									clChild_child->child(n)->setIcon(0, checked_icon);
								}
							}
						}
						if (outheatPropertyInfo.isChecked && insulatingheatPropertyInfo.isChecked && propellantInfo.isChecked && steelInfo.isChecked)
						{
							clChild_child->setIcon(0, checked_icon);
						}
					}
				}
				if (judgementPropertyInfo.isChecked && outheatPropertyInfo.isChecked && insulatingheatPropertyInfo.isChecked && propellantInfo.isChecked && steelInfo.isChecked)
				{
					clChild->setIcon(0, checked_icon);
				}
			}
			else if (child->child(j)->text(0).contains("材料数据库"))
			{
				QTreeWidgetItem *clChild = child->child(j);
				int clChildCount = clChild->childCount();
				for (int m = 0; m < clChildCount; ++m) {
					if (clChild->child(m)->text(0).contains("壳体材料"))
					{
						if (!steelInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					if (clChild->child(m)->text(0).contains("含能材料"))
					{
						if (!propellantInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					if (clChild->child(m)->text(0).contains("绝热层材料"))
					{
						if (!insulatingheatPropertyInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					if (clChild->child(m)->text(0).contains("外防热材料"))
					{
						if (!outheatPropertyInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					if (clChild->child(m)->text(0).contains("标准数据库"))
					{
						if (!judgementPropertyInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
				}
				
			}
		}
	}
}

void GFTreeModelWidget::contextMenuEvent(QContextMenuEvent *event)
{
	QTreeWidgetItem *item = m_treeWidget->itemAt(event->pos());
	if (!item) 
	{
		return;
	}

	QString text = item->text(0);
	if (text == "壳体" || text == "推进剂" || text == "绝热层")
	{
		// ========== 新增：判断是否为网格子节点 ==========
		QTreeWidgetItem* parentItem = item->parent();
		if (parentItem && parentItem->data(0, Qt::UserRole).toString() == "Mesh")
		{
			QMenu menu(this);
			QAction* actShow = menu.addAction("显示");
			QAction* actHide = menu.addAction("隐藏");

			QAction* selected = menu.exec(m_treeWidget->viewport()->mapToGlobal(event->pos()));
			if (!selected)
			{
				return;
			}

			// 查找父窗口 GFImportModelWidget
			QWidget* parent = parentWidget();
			GFImportModelWidget* importModelWidget = nullptr;
			while (parent)
			{
				importModelWidget = qobject_cast<GFImportModelWidget*>(parent);
				if (importModelWidget)
				{
					break;
				}
				parent = parent->parentWidget();
			}

			if (!importModelWidget)
			{
				return;
			}

			// 获取对应网格的AIS显示对象
			auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
			Handle(AIS_Shape) aisMesh;
			QString role = item->data(0, Qt::UserRole).toString();

			if (role == "ShellMesh")
			{
				aisMesh = meshInfo.shellAisMesh;
			}
			else if (role == "Propellant")
			{
				aisMesh = meshInfo.propellantAisMesh;
			}
			else if (role == "HeatInsulatingLayer")
			{
				aisMesh = meshInfo.heatInsulatingLayerAisMesh;
			}

			if (aisMesh.IsNull())
			{
				QMessageBox::warning(this, "提示", "该部件网格尚未划分或显示对象不存在");
				return;
			}

			auto occView = importModelWidget->GetOccView();
			Handle(AIS_InteractiveContext) context = occView->getContext();

			if (selected == actShow)
			{
				context->Display(aisMesh, Standard_True);
			}
			else if (selected == actHide)
			{
				context->Erase(aisMesh, Standard_True);
			}
			return;  // 处理完毕，直接返回
		}
		else if (parentItem && parentItem->data(0, Qt::UserRole).toString() == "Geometry")
		{
			QMenu menu(this);
			QAction* actImport = menu.addAction("导入");
			QAction* actShow = menu.addAction("显示");
			QAction* actHide = menu.addAction("隐藏");

			QAction* selected = menu.exec(m_treeWidget->viewport()->mapToGlobal(event->pos()));
			if (!selected)
			{
				return;
			}

			QWidget* parent = parentWidget();
			GFImportModelWidget* importModelWidget = nullptr;
			while (parent)
			{
				importModelWidget = qobject_cast<GFImportModelWidget*>(parent);
				if (importModelWidget)
				{
					break;
				}

				parent = parent->parentWidget();
			}

			if (!importModelWidget)
			{
				return;
			}

			if (selected == actImport)
			{
				QString filePath = QFileDialog::getOpenFileName(
					importModelWidget,
					"Open File",
					QDir::homePath(),
					"STEP Files (*.stp *.step);;"
					"IGES Files (*.iges *.igs);;"
					"VTK Files (*.vtk);;"
					"X_T Files (*.x_t);;"
					"All Files (*.*)");

				if (filePath.isEmpty())
				{
					return;
				}

				// 写日志
				auto logWidget = importModelWidget->GetLogWidget();
				auto textEdit = logWidget->GetTextEdit();
				QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
				QString partName = text;
				textEdit->appendPlainText(timeStr + "[信息]>开始导入" + partName + "几何模型");
				logWidget->update();
				QApplication::processEvents();

				// 创建进度对话框
				ProgressDialog* progressDialog = new ProgressDialog(partName + "导入", importModelWidget);
				progressDialog->setAttribute(Qt::WA_DeleteOnClose);
				progressDialog->show();

				// 创建工作线程和对象
				QThread* workerThread = new QThread(this);
				GeometryImportWorker* worker = new GeometryImportWorker(filePath);

				// 根据部件名称设置部件类型
				PartType partType = PartType::Unknown;
				if (text == "壳体")
				{
					partType = PartType::Shell;
				}
				else if (text == "推进剂")
				{
					partType = PartType::Propellant;
				}
				else if (text == "绝热层")
				{
					partType = PartType::HeatInsulatingLayer;
				}
				worker->SetPartType(partType);

				worker->moveToThread(workerThread);

				// 连接工作信号
				connect(workerThread, &QThread::started, worker, &GeometryImportWorker::DoWork);
				connect(worker, &GeometryImportWorker::ProgressUpdated,
					progressDialog, &ProgressDialog::SetProgress);
				connect(worker, &GeometryImportWorker::StatusUpdated,
					progressDialog, &ProgressDialog::SetStatusText);
				connect(progressDialog, &ProgressDialog::Canceled,
					worker, &GeometryImportWorker::RequestInterruption);

				// 使用 QueuedConnection 确保在主线程处理结果
				connect(worker, &GeometryImportWorker::WorkFinished, this,
					[=](bool success, QString msg, ModelGeometryInfo info) {
						QString finishTimeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
						QString level = success ? "信息" : "错误";
						textEdit->appendPlainText(finishTimeStr + "[" + level + "]>" + msg);

						if (success && !info.shape.IsNull())
						{
							auto occView = importModelWidget->GetOccView();
							Handle(AIS_InteractiveContext) context = occView->getContext();

							Handle(AIS_Shape) aisShape;
							if (text == "壳体" && !info.shellAisShape.IsNull())
							{
								aisShape = info.shellAisShape;
							}
							else if (text == "推进剂" && !info.propellantAisShape.IsNull())
							{
								aisShape = info.propellantAisShape;
							}
							else if (text == "绝热层" && !info.heatInsulatingLayerAisShape.IsNull())
							{
								aisShape = info.heatInsulatingLayerAisShape;
							}

							if (!aisShape.IsNull())
							{
								context->EraseAll(true);
								context->ClearCurrents(true);
								context->SetDisplayMode(aisShape, AIS_Shaded, false);
								context->Display(aisShape, true);

								auto* manager = ModelDataManager::GetInstance();
								auto existingInfo = manager->GetModelGeometryInfo();

								if (text == "壳体")
								{
									existingInfo.shellAisShape = info.shellAisShape;

									existingInfo.ptShellLeftBottom = info.ptShellLeftBottom;
									existingInfo.ptShellRightBottom = info.ptShellRightBottom;
									existingInfo.ptNozzleInletBottom = info.ptNozzleInletBottom;
									existingInfo.ptNozzleOutletBottom = info.ptNozzleOutletBottom;
								}
								else if (text == "推进剂")
								{
									existingInfo.propellantAisShape = info.propellantAisShape;
								}
								else if (text == "绝热层")
								{
									existingInfo.heatInsulatingLayerAisShape = info.heatInsulatingLayerAisShape;
								}

								existingInfo.path = info.path;
								existingInfo.shape = info.shape;

								existingInfo.theXmin = info.theXmin;
								existingInfo.theYmin = info.theYmin;
								existingInfo.theZmin = info.theZmin;
								existingInfo.theXmax = info.theXmax;
								existingInfo.theYmax = info.theYmax;
								existingInfo.theZmax = info.theZmax;
								existingInfo.length = info.length;
								existingInfo.width = info.width;
								existingInfo.height = info.height;

								manager->SetModelGeometryInfo(std::move(existingInfo));

								updataIcon();

								occView->fitAll();
								occView->update();

								if (auto geomProWid = importModelWidget->findChild<GeomPropertyWidget*>())
								{
									geomProWid->UpdataPropertyInfo();
								}
							}
						}
						else if (!success)
						{
							QMessageBox::warning(importModelWidget, "导入失败", msg);
						}

						// 安全关闭线程
						progressDialog->close();

						workerThread->quit();
						QTimer::singleShot(1000, this, [=]() {
							if (workerThread->isRunning())
							{
								workerThread->terminate();
							}
							worker->deleteLater();
							workerThread->deleteLater();
							});

						QTimer::singleShot(500, this, [=]() {
							QString m_privateDirPath = "src/template/main.png";
							if (wordExporter) {
								wordExporter->captureWidgetToFile(importModelWidget->GetOccView(), m_privateDirPath);
							}
							});
					}, Qt::QueuedConnection);
				workerThread->start();
			}
			else if (selected == actShow)
			{
				auto& geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
				Handle(AIS_Shape) aisShape;
				if (text == "壳体")
				{
					aisShape = geomInfo.shellAisShape;
				}
				else if (text == "推进剂")
				{
					aisShape = geomInfo.propellantAisShape;
				}
				else if (text == "绝热层")
				{
					aisShape = geomInfo.heatInsulatingLayerAisShape;
				}

				if (!aisShape.IsNull())
				{
					auto occView = importModelWidget->GetOccView();
					Handle(AIS_InteractiveContext) context = occView->getContext();
					context->Display(aisShape, Standard_True);
				}
			}
			else if (selected == actHide)
			{
				auto& geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
				Handle(AIS_Shape) aisShape;
				if (text == "壳体")
				{
					aisShape = geomInfo.shellAisShape;
				}
				else if (text == "推进剂")
				{
					aisShape = geomInfo.propellantAisShape;
				}
				else if (text == "绝热层")
				{
					aisShape = geomInfo.heatInsulatingLayerAisShape;
				}

				if (!aisShape.IsNull())
				{
					auto occView = importModelWidget->GetOccView();
					Handle(AIS_InteractiveContext) context = occView->getContext();
					context->Erase(aisShape, Standard_True);
				}
			}
		}
	}
	else if (text == "网格")
	{
		contextMenu = new QMenu(this);
		QAction* meshAction = new QAction("网格划分", this);
		connect(meshAction, &QAction::triggered, this, [item, this]() {
		QWidget* parent = parentWidget();
		GFImportModelWidget* importModelWidget = nullptr;
		while (parent)
		{
			importModelWidget = qobject_cast<GFImportModelWidget*>(parent);
			if (importModelWidget)
			{
				break;
			}

			parent = parent->parentWidget();
		}

		if (!importModelWidget)
		{
			return;
		}

		QDateTime currentTime = QDateTime::currentDateTime();
		QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
		auto logWidget = importModelWidget->GetLogWidget();
		auto textEdit = logWidget->GetTextEdit();
		QString text = timeStr + "[信息]>启动网格划分引擎，采用自适应尺寸控制算法";
		textEdit->appendPlainText(text);
		logWidget->update();

		// 创建进度对话框
		ProgressDialog* progressDialog = new ProgressDialog("网格划分", importModelWidget);
		progressDialog->show();

		// 从数据管理器获取三种几何的 AIS_Shape
		auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();

		// 创建工作线程和工作对象（传入三个 Handle(AIS_Shape)）
		TriangulationWorker* worker = new TriangulationWorker(
			geomInfo.shellAisShape,        // 壳体
			geomInfo.propellantAisShape,   // 推进剂
			geomInfo.heatInsulatingLayerAisShape // 隔热层
		);

		QThread* workerThread = new QThread();
		worker->moveToThread(workerThread);

		// 连接信号槽
		connect(workerThread, &QThread::started, worker, &TriangulationWorker::DoWork);
		connect(worker, &TriangulationWorker::ProgressUpdated,
			progressDialog, &ProgressDialog::SetProgress);
		connect(worker, &TriangulationWorker::StatusUpdated,
			progressDialog, &ProgressDialog::SetStatusText);
		connect(progressDialog, &ProgressDialog::Canceled,
			worker, &TriangulationWorker::RequestInterruption,
			Qt::DirectConnection);

		// 处理导入结果
		connect(worker, &TriangulationWorker::WorkFinished, this,
			[=](bool success, const QString& msg, const ModelMeshInfo& info) {
				// 更新日志
				QDateTime finishTime = QDateTime::currentDateTime();
				QString finishTimeStr = finishTime.toString("yyyy-MM-dd hh:mm:ss");
				textEdit->appendPlainText(finishTimeStr + "[" + (success ? "信息" : "错误") + "]>" + msg);

				if (success)
				{
					auto occView = importModelWidget->GetOccView();
					Handle(AIS_InteractiveContext) context = occView->getContext();
					auto view = occView->getView();
					context->EraseAll(true);

					// ========== 显示三种网格结果（修改lambda返回AIS句柄） ==========
					auto displayMesh = [&](const Handle(TriangleStructure)& meshData,
						const QString& name,
						const QColor& color)->Handle(AIS_Shape) {
						if (meshData.IsNull() || meshData->GetAllNodes().IsEmpty())
						{
							return Handle(AIS_Shape)();  // 返回空句柄
						}

						BRep_Builder builder;
						TopoDS_Compound compound;
						builder.MakeCompound(compound);

						auto myEdges = meshData->GetMyEdge();
						auto myNodeCoords = meshData->GetmyNodeCoords();

						for (const auto& edge : myEdges)
						{
							Standard_Integer node1ID = edge.first;
							Standard_Integer node2ID = edge.second;

							Standard_Real x1 = myNodeCoords->Value(node1ID, 1);
							Standard_Real y1 = myNodeCoords->Value(node1ID, 2);
							Standard_Real z1 = myNodeCoords->Value(node1ID, 3);

							Standard_Real x2 = myNodeCoords->Value(node2ID, 1);
							Standard_Real y2 = myNodeCoords->Value(node2ID, 2);
							Standard_Real z2 = myNodeCoords->Value(node2ID, 3);

							gp_Pnt p1(x1, y1, z1);
							gp_Pnt p2(x2, y2, z2);

							TopoDS_Vertex v1 = BRepBuilderAPI_MakeVertex(p1);
							TopoDS_Vertex v2 = BRepBuilderAPI_MakeVertex(p2);

							TopoDS_Edge edgeShape = BRepBuilderAPI_MakeEdge(v1, v2);
							builder.Add(compound, edgeShape);
						}

						Handle(AIS_Shape) aisCompound = new AIS_Shape(compound);
						aisCompound->SetColor(Quantity_Color(color.redF(), color.greenF(), color.blueF(), Quantity_TOC_RGB));
						context->Display(aisCompound, Standard_True);
						return aisCompound;  // 返回创建的AIS对象
					};

					// 显示三种网格并保存AIS对象
					Handle(AIS_Shape) shellAis = displayMesh(info.shellMesh, "壳体", QColor(255, 0, 0));
					Handle(AIS_Shape) propAis = displayMesh(info.propellantMesh, "推进剂", QColor(0, 255, 0));
					Handle(AIS_Shape) heatAis = displayMesh(info.heatInsulatingLayerMesh, "隔热层", QColor(0, 0, 255));

					// 保存网格数据及显示对象到数据管理器
					ModelMeshInfo updatedInfo = info;  // info 是 const&，这里拷贝一份
					updatedInfo.shellAisMesh = shellAis;
					updatedInfo.propellantAisMesh = propAis;
					updatedInfo.heatInsulatingLayerAisMesh = heatAis;
					ModelDataManager::GetInstance()->SetModelMeshInfo(updatedInfo);

					updataIcon();

					auto meshProWid = importModelWidget->findChild<MeshPropertyWidget*>();
					if (meshProWid)
					{
						meshProWid->UpdataPropertyInfo();
					}
				}
				else
				{
					QMessageBox::warning(this, "网格划分失败", msg);
				}

				// 清理资源
				progressDialog->close();
				workerThread->quit();
				workerThread->wait();
				worker->deleteLater();
				workerThread->deleteLater();
				progressDialog->deleteLater();
			});

		// 启动线程
		workerThread->start();

		});
	contextMenu->addAction(meshAction);
	contextMenu->exec(event->globalPos());
	}
	else if (text == "安全特性参数分析")
	{
		contextMenu = new QMenu(this);
		QAction* calAction = new QAction("计算", this);
		QAction* exportAction = new QAction("导出报告", this);

		int childCount = item->childCount();
		QList<QTreeWidgetItem*> checkedChildItems;
		for (int i = 0; i < childCount; ++i) {
			QTreeWidgetItem* childItem = item->child(i);
			if (childItem->checkState(0) == Qt::Checked)
			{
				checkedChildItems.append(childItem);
			}
		}

		connect(calAction, &QAction::triggered, this, [item, this]() {
			QWidget* parent = parentWidget();
			while (parent)
			{
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{
					QDateTime currentTime = QDateTime::currentDateTime();
					QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
					auto logWidget = gfParent->GetLogWidget();
					auto textEdit = logWidget->GetTextEdit();

					auto occView = gfParent->GetOccView();
					Handle(AIS_InteractiveContext) context = occView->getContext();
					Handle(V3d_View) view = occView->getView();

					// 创建进度对话框
					ProgressDialog* progressDialog = new ProgressDialog("计算", gfParent);
					progressDialog->show();

					// 创建工作线程和工作对象
					auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
					CalculateWorker* worker = new CalculateWorker();
					QThread* workerThread = new QThread();
					worker->moveToThread(workerThread);

					// 连接信号槽
					connect(workerThread, &QThread::started, worker, &CalculateWorker::DoWork);
					connect(worker, &CalculateWorker::ProgressUpdated,
						progressDialog, &ProgressDialog::SetProgress);
					connect(worker, &CalculateWorker::StatusUpdated,
						progressDialog, &ProgressDialog::SetStatusText);
					connect(progressDialog, &ProgressDialog::Canceled,
						worker, &CalculateWorker::RequestInterruption,
						Qt::DirectConnection);

					// 处理导入结果
					connect(worker, &CalculateWorker::WorkFinished, this,
						[=](bool success, const QString& msg) {
							// 更新日志
							QDateTime finishTime = QDateTime::currentDateTime();
							QString finishTimeStr = finishTime.toString("yyyy-MM-dd hh:mm:ss");
							textEdit->appendPlainText(finishTimeStr + "[" + (success ? "信息" : "错误") + "]>" + msg);
							if (success)
							{
								auto tensileStrength = ModelDataManager::GetInstance()->GetSteelPropertyInfo().tensileStrength;	// 壳体抗拉强度
								auto ignitionTemperature = ModelDataManager::GetInstance()->GetPropellantPropertyInfo().ignitionTemperature; // 推进剂发火温度
								auto fireOverpressure = ModelDataManager::GetInstance()->GetPropellantPropertyInfo().fireOverpressure; // 推进剂发火超压
								for (int i = 0; i < item->childCount(); ++i) {
									QTreeWidgetItem* childItem = item->child(i);
									auto originalName = childItem->text(0);
									int dotIndex = originalName.indexOf('.');
									QString processedName;
									if (dotIndex != -1)
									{
										processedName = originalName.mid(dotIndex + 1).trimmed();
									}
									else {
										processedName = originalName;
									}

									bool isChecked = (childItem->checkState(0) == Qt::Checked);
									if (isChecked)
									{
										QString text = timeStr + "[信息]>开始进行" + processedName;
										textEdit->appendPlainText(text);

										if (processedName == "跌落安全性分析")
										{
											std::vector<double> resultValue;
											resultValue.reserve(8);
											bool success = APICalculateHepler::CalculateFallAnalysisResult(occView, resultValue);

											QDateTime currentTime = QDateTime::currentDateTime();
											QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
											if (success)
											{
												QString text = timeStr + "[信息]>跌落安全性分析计算完成";
												textEdit->appendPlainText(text);

												context->EraseAll(true);
												view->SetProj(V3d_Yneg);
												view->Redraw();

												auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
												auto oriShape = geomInfo.shape;

												auto fallStressResult = ModelDataManager::GetInstance()->GetFallStressResult();
												gfParent->GetStressResultWidget()->updateData(fallStressResult.metalsMaxStress, fallStressResult.metalsMinStress, fallStressResult.metalsAvgStress, fallStressResult.metalsStandardStress,
													fallStressResult.propellantsMaxStress, fallStressResult.propellantsMinStress, fallStressResult.propellantsAvgStress, fallStressResult.propellantsStandardStress,
													fallStressResult.outheatMaxStress, fallStressResult.outheatMinStress, fallStressResult.outheatAvgStress, fallStressResult.outheatStandardStress,
													fallStressResult.insulatingheatMaxStress, fallStressResult.insulatingheatMinStress, fallStressResult.insulatingheatAvgStress, fallStressResult.insulatingheatStandardStress);

												auto fallStrainResult = ModelDataManager::GetInstance()->GetFallStrainResult();
												gfParent->GetStrainResultWidget()->updateData(fallStrainResult.metalsMaxStrain, fallStrainResult.metalsMinStrain, fallStrainResult.metalsAvgStrain, fallStrainResult.metalsStandardStrain,
													fallStrainResult.propellantsMaxStrain, fallStrainResult.propellantsMinStrain, fallStrainResult.mpropellantsAvgStrain, fallStrainResult.propellantsStandardStrain,
													fallStrainResult.outheatMaxStrain, fallStrainResult.outheatMinStrain, fallStrainResult.outheatAvgStrain, fallStrainResult.outheatStandardStrain,
													fallStrainResult.insulatingheatMaxStrain, fallStrainResult.insulatingheatMinStrain, fallStrainResult.insulatingheatAvgStrain, fallStrainResult.insulatingheatStandardStrain);

												auto fallTemperatureResult = ModelDataManager::GetInstance()->GetFallTemperatureResult();
												gfParent->GetTemperatureResultWidget()->updateData(fallTemperatureResult.metalsMaxTemperature, fallTemperatureResult.metalsMinTemperature, fallTemperatureResult.metalsAvgTemperature, fallTemperatureResult.metalsStandardTemperature,
													fallTemperatureResult.propellantsMaxTemperature, fallTemperatureResult.propellantsMinTemperature, fallTemperatureResult.mpropellantsAvgTemperature, fallTemperatureResult.propellantsStandardTemperature,
													fallTemperatureResult.outheatMaxTemperature, fallTemperatureResult.outheatMinTemperature, fallTemperatureResult.outheatAvgTemperature, fallTemperatureResult.outheatStandardTemperature,
													fallTemperatureResult.insulatingheatMaxTemperature, fallTemperatureResult.insulatingheatMinTemperature, fallTemperatureResult.insulatingheatAvgTemperature, fallTemperatureResult.insulatingheatStandardTemperature);

												auto fallOverpressureResult = ModelDataManager::GetInstance()->GetFallOverpressureResult();
												gfParent->GetOverpressureResultWidget()->updateData(fallOverpressureResult.metalsMaxOverpressure, fallOverpressureResult.metalsMinOverpressure, fallOverpressureResult.metalsAvgOverpressure, fallOverpressureResult.metalsStandardOverpressure,
													fallOverpressureResult.propellantsMaxOverpressure, fallOverpressureResult.propellantsMinOverpressure, fallOverpressureResult.mpropellantsAvgOverpressure, fallOverpressureResult.propellantsStandardOverpressure,
													fallOverpressureResult.outheatMaxOverpressure, fallOverpressureResult.outheatMinOverpressure, fallOverpressureResult.outheatAvgOverpressure, fallOverpressureResult.outheatStandardOverpressure,
													fallOverpressureResult.insulatingheatMaxOverpressure, fallOverpressureResult.insulatingheatMinOverpressure, fallOverpressureResult.insulatingheatAvgOverpressure, fallOverpressureResult.insulatingheatStandardOverpressure);

												// 更新判断结果
												auto tableWidget = gfParent->GetFallPropertyWidget()->GetQTableWidget();
												if (resultValue[0]> tensileStrength)
												{
													tableWidget->item(8, 2)->setText("应力超过壳体最大抗拉强度，有燃爆风险");
												}
												else
												{
													tableWidget->item(8, 2)->setText("应力未超过壳体最大抗拉强度");
												}
												if (fallTemperatureResult.metalsMaxTemperature > ignitionTemperature)
												{
													tableWidget->item(9, 2)->setText("温度超过推进剂最大发火温度，有燃爆风险");
												}
												else
												{
													tableWidget->item(9, 2)->setText("温度超过推进剂最大发火温度");
												}
												if (fallOverpressureResult.metalsMaxOverpressure > fireOverpressure)
												{
													tableWidget->item(10, 2)->setText("超压超过推进剂最大发火超压，有燃爆风险");
												}
												else
												{
													tableWidget->item(10, 2)->setText("超压超过推进剂最大发火超压");
												}
											}
											else
											{
												QString text = timeStr + "[信息]>跌落安全性分析计算失败";
												textEdit->appendPlainText(text);
											}
										}
										else if (processedName == "快速烤燃安全性分析")
										{
											std::vector<double> resultValue;
											resultValue.reserve(8);
											bool success = APICalculateHepler::CalculateFastCombustionAnalysisResult(occView, resultValue);

											QDateTime currentTime = QDateTime::currentDateTime();
											QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
											if (success)
											{
												QString text = timeStr + "[信息]>快速烤燃安全性分析计算完成";
												textEdit->appendPlainText(text);

												auto temperatureResult = ModelDataManager::GetInstance()->GetFastCombustionTemperatureResult();
												gfParent->GetFastCombustionTemperatureResultWidget()->updateData(temperatureResult.metalsMaxTemperature, temperatureResult.metalsMinTemperature, temperatureResult.metalsAvgTemperature, temperatureResult.metalsStandardTemperature,
													temperatureResult.propellantsMaxTemperature, temperatureResult.propellantsMinTemperature, temperatureResult.mpropellantsAvgTemperature, temperatureResult.propellantsStandardTemperature,
													temperatureResult.outheatMaxTemperature, temperatureResult.outheatMinTemperature, temperatureResult.outheatAvgTemperature, temperatureResult.outheatStandardTemperature,
													temperatureResult.insulatingheatMaxTemperature, temperatureResult.insulatingheatMinTemperature, temperatureResult.insulatingheatAvgTemperature, temperatureResult.insulatingheatStandardTemperature);

												// 更新判断结果
												auto tableWidget = gfParent->GetFastCombustionPropertyWidget()->GetQTableWidget();
												if (temperatureResult.metalsMaxTemperature > ignitionTemperature)
												{
													tableWidget->item(10, 2)->setText("温度超过推进剂最大发火温度，有燃爆风险");
												}
												else
												{
													tableWidget->item(10, 2)->setText("温度超过推进剂最大发火温度");
												}
											}
											else
											{
												QString text = timeStr + "[信息]>快速烤燃安全性分析计算失败";
												textEdit->appendPlainText(text);
											}
										}
										else if (processedName == "慢速烤燃安全性分析")
										{
											std::vector<double> resultValue;
											resultValue.reserve(8);
											bool success = APICalculateHepler::CalculateSlowCombustionAnalysisResult(occView, resultValue);


											QDateTime currentTime = QDateTime::currentDateTime();
											QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
											if (success)
											{
												QString text = timeStr + "[信息]>慢速烤燃安全性分析计算完成";
												textEdit->appendPlainText(text);

												auto temperatureResult = ModelDataManager::GetInstance()->GetSlowCombustionTemperatureResult();
												gfParent->GetSlowCombustionTemperatureResultWidget()->updateData(temperatureResult.metalsMaxTemperature, temperatureResult.metalsMinTemperature, temperatureResult.metalsAvgTemperature, temperatureResult.metalsStandardTemperature,
													temperatureResult.propellantsMaxTemperature, temperatureResult.propellantsMinTemperature, temperatureResult.mpropellantsAvgTemperature, temperatureResult.propellantsStandardTemperature,
													temperatureResult.outheatMaxTemperature, temperatureResult.outheatMinTemperature, temperatureResult.outheatAvgTemperature, temperatureResult.outheatStandardTemperature,
													temperatureResult.insulatingheatMaxTemperature, temperatureResult.insulatingheatMinTemperature, temperatureResult.insulatingheatAvgTemperature, temperatureResult.insulatingheatStandardTemperature);

												// 更新判断结果
												auto tableWidget = gfParent->GetSlowCombustionPropertyWidget()->GetQTableWidget();
												if (temperatureResult.metalsMaxTemperature > ignitionTemperature)
												{
													tableWidget->item(10, 2)->setText("温度超过推进剂最大发火温度，有燃爆风险");
												}
												else
												{
													tableWidget->item(10, 2)->setText("温度超过推进剂最大发火温度");
												}
											}
											else
											{
												QString text = timeStr + "[信息]>慢速烤燃安全性分析计算失败";
												textEdit->appendPlainText(text);
											}
										}
										else if (processedName == "枪击安全性分析")
										{
											std::vector<double> resultValue;
											resultValue.reserve(8);
											bool success = APICalculateHepler::CalculateShootingAnalysisResult(occView, resultValue);

											QDateTime currentTime = QDateTime::currentDateTime();
											QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
											if (success)
											{
												QString text = timeStr + "[信息]>枪击安全性分析计算完成";
												textEdit->appendPlainText(text);

												context->EraseAll(true);
												view->SetProj(V3d_Yneg);
												view->Redraw();

												auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
												auto oriShape = geomInfo.shape;

												
												auto stressResult = ModelDataManager::GetInstance()->GetShootStressResult();
												gfParent->GetShootStressResultWidget()->updateData(stressResult.metalsMaxStress, stressResult.metalsMinStress, stressResult.metalsAvgStress, stressResult.metalsStandardStress,
													stressResult.propellantsMaxStress, stressResult.propellantsMinStress, stressResult.propellantsAvgStress, stressResult.propellantsStandardStress,
													stressResult.outheatMaxStress, stressResult.outheatMinStress, stressResult.outheatAvgStress, stressResult.outheatStandardStress,
													stressResult.insulatingheatMaxStress, stressResult.insulatingheatMinStress, stressResult.insulatingheatAvgStress, stressResult.insulatingheatStandardStress);

												auto strainResult = ModelDataManager::GetInstance()->GetShootStrainResult();
												gfParent->GetShootStrainResultWidget()->updateData(strainResult.metalsMaxStrain, strainResult.metalsMinStrain, strainResult.metalsAvgStrain, strainResult.metalsStandardStrain,
													strainResult.propellantsMaxStrain, strainResult.propellantsMinStrain, strainResult.mpropellantsAvgStrain, strainResult.propellantsStandardStrain,
													strainResult.outheatMaxStrain, strainResult.outheatMinStrain, strainResult.outheatAvgStrain, strainResult.outheatStandardStrain,
													strainResult.insulatingheatMaxStrain, strainResult.insulatingheatMinStrain, strainResult.insulatingheatAvgStrain, strainResult.insulatingheatStandardStrain);

												auto temperatureResult = ModelDataManager::GetInstance()->GetShootTemperatureResult();
												gfParent->GetShootTemperatureResultWidget()->updateData(temperatureResult.metalsMaxTemperature, temperatureResult.metalsMinTemperature, temperatureResult.metalsAvgTemperature, temperatureResult.metalsStandardTemperature,
													temperatureResult.propellantsMaxTemperature, temperatureResult.propellantsMinTemperature, temperatureResult.mpropellantsAvgTemperature, temperatureResult.propellantsStandardTemperature,
													temperatureResult.outheatMaxTemperature, temperatureResult.outheatMinTemperature, temperatureResult.outheatAvgTemperature, temperatureResult.outheatStandardTemperature,
													temperatureResult.insulatingheatMaxTemperature, temperatureResult.insulatingheatMinTemperature, temperatureResult.insulatingheatAvgTemperature, temperatureResult.insulatingheatStandardTemperature);

												auto overpressureResult = ModelDataManager::GetInstance()->GetShootOverpressureResult();
												gfParent->GetShootOverpressureResultWidget()->updateData(overpressureResult.metalsMaxOverpressure, overpressureResult.metalsMinOverpressure, overpressureResult.metalsAvgOverpressure, overpressureResult.metalsStandardOverpressure,
													overpressureResult.propellantsMaxOverpressure, overpressureResult.propellantsMinOverpressure, overpressureResult.mpropellantsAvgOverpressure, overpressureResult.propellantsStandardOverpressure,
													overpressureResult.outheatMaxOverpressure, overpressureResult.outheatMinOverpressure, overpressureResult.outheatAvgOverpressure, overpressureResult.outheatStandardOverpressure,
													overpressureResult.insulatingheatMaxOverpressure, overpressureResult.insulatingheatMinOverpressure, overpressureResult.insulatingheatAvgOverpressure, overpressureResult.insulatingheatStandardOverpressure);

												// 更新判断结果
												auto tableWidget = gfParent->GetShootPropertyWidget()->GetQTableWidget();
												if (resultValue[0] > tensileStrength)
												{
													tableWidget->item(10, 2)->setText("应力超过壳体最大抗拉强度，有燃爆风险");
												}
												else
												{
													tableWidget->item(10, 2)->setText("应力未超过壳体最大抗拉强度");
												}
												if (temperatureResult.metalsMaxTemperature > ignitionTemperature)
												{
													tableWidget->item(11, 2)->setText("温度超过推进剂最大发火温度，有燃爆风险");
												}
												else
												{
													tableWidget->item(11, 2)->setText("温度超过推进剂最大发火温度");
												}
												if (overpressureResult.metalsMaxOverpressure > fireOverpressure)
												{
													tableWidget->item(12, 2)->setText("超压超过推进剂最大发火超压，有燃爆风险");
												}
												else
												{
													tableWidget->item(12, 2)->setText("超压超过推进剂最大发火超压");
												}
}
											else
											{
												QString text = timeStr + "[信息]>枪击安全性分析计算失败";
												textEdit->appendPlainText(text);
											}
										}
										else if (processedName == "射流冲击安全性分析")
										{

											std::vector<double> resultValue;
											resultValue.reserve(8);
											bool success = APICalculateHepler::CalculateJetImpactingAnalysisResult(occView, resultValue);


											QDateTime currentTime = QDateTime::currentDateTime();
											QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
											if (success)
											{
												QString text = timeStr + "[信息]>射流冲击安全性分析计算完成";
												textEdit->appendPlainText(text);

										

												auto stressResult = ModelDataManager::GetInstance()->GetJetImpactStressResult();
												gfParent->GetJetImpactStressResultWidget()->updateData(stressResult.metalsMaxStress, stressResult.metalsMinStress, stressResult.metalsAvgStress, stressResult.metalsStandardStress,
													stressResult.propellantsMaxStress, stressResult.propellantsMinStress, stressResult.propellantsAvgStress, stressResult.propellantsStandardStress,
													stressResult.outheatMaxStress, stressResult.outheatMinStress, stressResult.outheatAvgStress, stressResult.outheatStandardStress,
													stressResult.insulatingheatMaxStress, stressResult.insulatingheatMinStress, stressResult.insulatingheatAvgStress, stressResult.insulatingheatStandardStress);

												auto strainResult = ModelDataManager::GetInstance()->GetJetImpactStrainResult();
												gfParent->GetJetImpactStrainResultWidget()->updateData(strainResult.metalsMaxStrain, strainResult.metalsMinStrain, strainResult.metalsAvgStrain, strainResult.metalsStandardStrain,
													strainResult.propellantsMaxStrain, strainResult.propellantsMinStrain, strainResult.mpropellantsAvgStrain, strainResult.propellantsStandardStrain,
													strainResult.outheatMaxStrain, strainResult.outheatMinStrain, strainResult.outheatAvgStrain, strainResult.outheatStandardStrain,
													strainResult.insulatingheatMaxStrain, strainResult.insulatingheatMinStrain, strainResult.insulatingheatAvgStrain, strainResult.insulatingheatStandardStrain);

												auto temperatureResult = ModelDataManager::GetInstance()->GetJetImpactTemperatureResult();
												gfParent->GetJetImpactTemperatureResultWidget()->updateData(temperatureResult.metalsMaxTemperature, temperatureResult.metalsMinTemperature, temperatureResult.metalsAvgTemperature, temperatureResult.metalsStandardTemperature,
													temperatureResult.propellantsMaxTemperature, temperatureResult.propellantsMinTemperature, temperatureResult.mpropellantsAvgTemperature, temperatureResult.propellantsStandardTemperature,
													temperatureResult.outheatMaxTemperature, temperatureResult.outheatMinTemperature, temperatureResult.outheatAvgTemperature, temperatureResult.outheatStandardTemperature,
													temperatureResult.insulatingheatMaxTemperature, temperatureResult.insulatingheatMinTemperature, temperatureResult.insulatingheatAvgTemperature, temperatureResult.insulatingheatStandardTemperature);

												auto overpressureResult = ModelDataManager::GetInstance()->GetJetImpactOverpressureResult();
												gfParent->GetJetImpactOverpressureResultWidget()->updateData(overpressureResult.metalsMaxOverpressure, overpressureResult.metalsMinOverpressure, overpressureResult.metalsAvgOverpressure, overpressureResult.metalsStandardOverpressure,
													overpressureResult.propellantsMaxOverpressure, overpressureResult.propellantsMinOverpressure, overpressureResult.mpropellantsAvgOverpressure, overpressureResult.propellantsStandardOverpressure,
													overpressureResult.outheatMaxOverpressure, overpressureResult.outheatMinOverpressure, overpressureResult.outheatAvgOverpressure, overpressureResult.outheatStandardOverpressure,
													overpressureResult.insulatingheatMaxOverpressure, overpressureResult.insulatingheatMinOverpressure, overpressureResult.insulatingheatAvgOverpressure, overpressureResult.insulatingheatStandardOverpressure);

												// 更新判断结果
												auto tableWidget = gfParent->GetJetImpactPropertyWidget()->GetQTableWidget();
												if (stressResult.metalsMaxStress > tensileStrength)
												{
													tableWidget->item(8, 2)->setText("应力超过壳体最大抗拉强度，有燃爆风险");
												}
												else
												{
													tableWidget->item(8, 2)->setText("应力未超过壳体最大抗拉强度");
												}
												if (temperatureResult.metalsMaxTemperature > ignitionTemperature)
												{
													tableWidget->item(9, 2)->setText("温度超过推进剂最大发火温度，有燃爆风险");
												}
												else
												{
													tableWidget->item(9, 2)->setText("温度超过推进剂最大发火温度");
												}
												if (overpressureResult.metalsMaxOverpressure > fireOverpressure)
												{
													tableWidget->item(10, 2)->setText("超压超过推进剂最大发火超压，有燃爆风险");
												}
												else
												{
													tableWidget->item(10, 2)->setText("超压超过推进剂最大发火超压");
												}
											}
											else
											{
												QString text = timeStr + "[信息]>射流冲击安全性分析计算失败";
												textEdit->appendPlainText(text);
											}
										}
										else if (processedName == "破片撞击安全性分析")
										{
											std::vector<double> resultValue;
											resultValue.reserve(8);
											bool success = APICalculateHepler::CalculateFragmentationAnalysisResult(occView, resultValue);

											QDateTime currentTime = QDateTime::currentDateTime();
											QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
											if (success)
											{
												QString text = timeStr + "[信息]>破片安全性分析计算完成";
												textEdit->appendPlainText(text);

												context->EraseAll(true);
												view->SetProj(V3d_Yneg);
												view->Redraw();

												auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
												auto oriShape = geomInfo.shape;

												auto stressResult = ModelDataManager::GetInstance()->GetFragmentationImpactStressResult();
												gfParent->GetFragmentationImpactStressResultWidget()->updateData(stressResult.metalsMaxStress, stressResult.metalsMinStress, stressResult.metalsAvgStress, stressResult.metalsStandardStress,
													stressResult.propellantsMaxStress, stressResult.propellantsMinStress, stressResult.propellantsAvgStress, stressResult.propellantsStandardStress,
													stressResult.outheatMaxStress, stressResult.outheatMinStress, stressResult.outheatAvgStress, stressResult.outheatStandardStress,
													stressResult.insulatingheatMaxStress, stressResult.insulatingheatMinStress, stressResult.insulatingheatAvgStress, stressResult.insulatingheatStandardStress);

												auto strainResult = ModelDataManager::GetInstance()->GetFragmentationImpactStrainResult();
												gfParent->GetFragmentationImpactStrainResultWidget()->updateData(strainResult.metalsMaxStrain, strainResult.metalsMinStrain, strainResult.metalsAvgStrain, strainResult.metalsStandardStrain,
													strainResult.propellantsMaxStrain, strainResult.propellantsMinStrain, strainResult.mpropellantsAvgStrain, strainResult.propellantsStandardStrain,
													strainResult.outheatMaxStrain, strainResult.outheatMinStrain, strainResult.outheatAvgStrain, strainResult.outheatStandardStrain,
													strainResult.insulatingheatMaxStrain, strainResult.insulatingheatMinStrain, strainResult.insulatingheatAvgStrain, strainResult.insulatingheatStandardStrain);

												auto temperatureResult = ModelDataManager::GetInstance()->GetFragmentationImpactTemperatureResult();
												gfParent->GetFragmentationImpactTemperatureResultWidget()->updateData(temperatureResult.metalsMaxTemperature, temperatureResult.metalsMinTemperature, temperatureResult.metalsAvgTemperature, temperatureResult.metalsStandardTemperature,
													temperatureResult.propellantsMaxTemperature, temperatureResult.propellantsMinTemperature, temperatureResult.mpropellantsAvgTemperature, temperatureResult.propellantsStandardTemperature,
													temperatureResult.outheatMaxTemperature, temperatureResult.outheatMinTemperature, temperatureResult.outheatAvgTemperature, temperatureResult.outheatStandardTemperature,
													temperatureResult.insulatingheatMaxTemperature, temperatureResult.insulatingheatMinTemperature, temperatureResult.insulatingheatAvgTemperature, temperatureResult.insulatingheatStandardTemperature);

												auto overpressureResult = ModelDataManager::GetInstance()->GetFragmentationImpactOverpressureResult();
												gfParent->GetFragmentationImpactOverpressureResultWidget()->updateData(overpressureResult.metalsMaxOverpressure, overpressureResult.metalsMinOverpressure, overpressureResult.metalsAvgOverpressure, overpressureResult.metalsStandardOverpressure,
													overpressureResult.propellantsMaxOverpressure, overpressureResult.propellantsMinOverpressure, overpressureResult.mpropellantsAvgOverpressure, overpressureResult.propellantsStandardOverpressure,
													overpressureResult.outheatMaxOverpressure, overpressureResult.outheatMinOverpressure, overpressureResult.outheatAvgOverpressure, overpressureResult.outheatStandardOverpressure,
													overpressureResult.insulatingheatMaxOverpressure, overpressureResult.insulatingheatMinOverpressure, overpressureResult.insulatingheatAvgOverpressure, overpressureResult.insulatingheatStandardOverpressure);

												// 更新判断结果
												auto tableWidget = gfParent->GetFragmentationImpactPropertyWidget()->GetQTableWidget();
												if (resultValue[0] > tensileStrength)
												{
													tableWidget->item(11, 2)->setText("应力超过壳体最大抗拉强度，有燃爆风险");
												}
												else
												{
													tableWidget->item(11, 2)->setText("应力未超过壳体最大抗拉强度");
												}
												if (temperatureResult.metalsMaxTemperature > ignitionTemperature)
												{
													tableWidget->item(12, 2)->setText("温度超过推进剂最大发火温度，有燃爆风险");
												}
												else
												{
													tableWidget->item(12, 2)->setText("温度超过推进剂最大发火温度");
												}
												if (overpressureResult.metalsMaxOverpressure > fireOverpressure)
												{
													tableWidget->item(13, 2)->setText("超压超过推进剂最大发火超压，有燃爆风险");
												}
												else
												{
													tableWidget->item(13, 2)->setText("超压超过推进剂最大发火超压");
												}
											}
											else
											{
												QString text = timeStr + "[信息]>破片安全性分析计算失败";
												textEdit->appendPlainText(text);
											}
										}
										else if (processedName == "爆炸冲击波安全性分析")
										{

											std::vector<double> resultValue;
											resultValue.reserve(8);
											bool success = APICalculateHepler::CalculateExplosiveBlastAnalysisResult(occView, resultValue);


											QDateTime currentTime = QDateTime::currentDateTime();
											QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
											if (success)
											{
												QString text = timeStr + "[信息]>爆炸冲击波安全性分析计算完成";
												textEdit->appendPlainText(text);



												auto stressResult = ModelDataManager::GetInstance()->GetExplosiveBlastStressResult();
												gfParent->GetExplosiveBlastStressResultWidget()->updateData(stressResult.metalsMaxStress, stressResult.metalsMinStress, stressResult.metalsAvgStress, stressResult.metalsStandardStress,
													stressResult.propellantsMaxStress, stressResult.propellantsMinStress, stressResult.propellantsAvgStress, stressResult.propellantsStandardStress,
													stressResult.outheatMaxStress, stressResult.outheatMinStress, stressResult.outheatAvgStress, stressResult.outheatStandardStress,
													stressResult.insulatingheatMaxStress, stressResult.insulatingheatMinStress, stressResult.insulatingheatAvgStress, stressResult.insulatingheatStandardStress);

												auto strainResult = ModelDataManager::GetInstance()->GetExplosiveBlastStrainResult();
												gfParent->GetExplosiveBlastStrainResultWidget()->updateData(strainResult.metalsMaxStrain, strainResult.metalsMinStrain, strainResult.metalsAvgStrain, strainResult.metalsStandardStrain,
													strainResult.propellantsMaxStrain, strainResult.propellantsMinStrain, strainResult.mpropellantsAvgStrain, strainResult.propellantsStandardStrain,
													strainResult.outheatMaxStrain, strainResult.outheatMinStrain, strainResult.outheatAvgStrain, strainResult.outheatStandardStrain,
													strainResult.insulatingheatMaxStrain, strainResult.insulatingheatMinStrain, strainResult.insulatingheatAvgStrain, strainResult.insulatingheatStandardStrain);

												auto temperatureResult = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult();
												gfParent->GetExplosiveBlastTemperatureResultWidget()->updateData(temperatureResult.metalsMaxTemperature, temperatureResult.metalsMinTemperature, temperatureResult.metalsAvgTemperature, temperatureResult.metalsStandardTemperature,
													temperatureResult.propellantsMaxTemperature, temperatureResult.propellantsMinTemperature, temperatureResult.mpropellantsAvgTemperature, temperatureResult.propellantsStandardTemperature,
													temperatureResult.outheatMaxTemperature, temperatureResult.outheatMinTemperature, temperatureResult.outheatAvgTemperature, temperatureResult.outheatStandardTemperature,
													temperatureResult.insulatingheatMaxTemperature, temperatureResult.insulatingheatMinTemperature, temperatureResult.insulatingheatAvgTemperature, temperatureResult.insulatingheatStandardTemperature);

												auto overpressureResult = ModelDataManager::GetInstance()->GetExplosiveBlastOverpressureResult();
												gfParent->GetExplosiveBlastOverpressureResultWidget()->updateData(overpressureResult.metalsMaxOverpressure, overpressureResult.metalsMinOverpressure, overpressureResult.metalsAvgOverpressure, overpressureResult.metalsStandardOverpressure,
													overpressureResult.propellantsMaxOverpressure, overpressureResult.propellantsMinOverpressure, overpressureResult.mpropellantsAvgOverpressure, overpressureResult.propellantsStandardOverpressure,
													overpressureResult.outheatMaxOverpressure, overpressureResult.outheatMinOverpressure, overpressureResult.outheatAvgOverpressure, overpressureResult.outheatStandardOverpressure,
													overpressureResult.insulatingheatMaxOverpressure, overpressureResult.insulatingheatMinOverpressure, overpressureResult.insulatingheatAvgOverpressure, overpressureResult.insulatingheatStandardOverpressure);

												// 更新判断结果
												auto tableWidget = gfParent->GetExplosiveBlastPropertyWidget()->GetQTableWidget();
												if (stressResult.metalsMaxStress > tensileStrength)
												{
													tableWidget->item(7, 2)->setText("应力超过壳体最大抗拉强度，有燃爆风险");
												}
												else
												{
													tableWidget->item(7, 2)->setText("应力未超过壳体最大抗拉强度");
												}
												if (temperatureResult.metalsMaxTemperature > ignitionTemperature)
												{
													tableWidget->item(8, 2)->setText("温度超过推进剂最大发火温度，有燃爆风险");
												}
												else
												{
													tableWidget->item(8, 2)->setText("温度超过推进剂最大发火温度");
												}
												if (overpressureResult.metalsMaxOverpressure > fireOverpressure)
												{
													tableWidget->item(9, 2)->setText("超压超过推进剂最大发火超压，有燃爆风险");
												}
												else
												{
													tableWidget->item(9, 2)->setText("超压超过推进剂最大发火超压");
												}
											}
											else
											{
												QString text = timeStr + "[信息]>爆炸冲击波安全性分析计算失败";
												textEdit->appendPlainText(text);
											}
										}
										else if (processedName == "殉爆安全性分析")
										{
											
											std::vector<double> resultValue;
											resultValue.reserve(8);
											bool success = APICalculateHepler::CalculateSacrificeExplosionAnalysisResult(occView, resultValue);


											QDateTime currentTime = QDateTime::currentDateTime();
											QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
											if (success)
											{
												QString text = timeStr + "[信息]>殉爆安全性分析计算完成";
												textEdit->appendPlainText(text);



												auto stressResult = ModelDataManager::GetInstance()->GetSacrificeExplosionStressResult();
												gfParent->GetSacrificeExplosionStressResultWidget()->updateData(stressResult.metalsMaxStress, stressResult.metalsMinStress, stressResult.metalsAvgStress, stressResult.metalsStandardStress,
													stressResult.propellantsMaxStress, stressResult.propellantsMinStress, stressResult.propellantsAvgStress, stressResult.propellantsStandardStress,
													stressResult.outheatMaxStress, stressResult.outheatMinStress, stressResult.outheatAvgStress, stressResult.outheatStandardStress,
													stressResult.insulatingheatMaxStress, stressResult.insulatingheatMinStress, stressResult.insulatingheatAvgStress, stressResult.insulatingheatStandardStress);

												auto strainResult = ModelDataManager::GetInstance()->GetSacrificeExplosionStrainResult();
												gfParent->GetSacrificeExplosionStrainResultWidget()->updateData(strainResult.metalsMaxStrain, strainResult.metalsMinStrain, strainResult.metalsAvgStrain, strainResult.metalsStandardStrain,
													strainResult.propellantsMaxStrain, strainResult.propellantsMinStrain, strainResult.mpropellantsAvgStrain, strainResult.propellantsStandardStrain,
													strainResult.outheatMaxStrain, strainResult.outheatMinStrain, strainResult.outheatAvgStrain, strainResult.outheatStandardStrain,
													strainResult.insulatingheatMaxStrain, strainResult.insulatingheatMinStrain, strainResult.insulatingheatAvgStrain, strainResult.insulatingheatStandardStrain);

												auto temperatureResult = ModelDataManager::GetInstance()->GetSacrificeExplosionTemperatureResult();
												gfParent->GetSacrificeExplosionTemperatureResultWidget()->updateData(temperatureResult.metalsMaxTemperature, temperatureResult.metalsMinTemperature, temperatureResult.metalsAvgTemperature, temperatureResult.metalsStandardTemperature,
													temperatureResult.propellantsMaxTemperature, temperatureResult.propellantsMinTemperature, temperatureResult.mpropellantsAvgTemperature, temperatureResult.propellantsStandardTemperature,
													temperatureResult.outheatMaxTemperature, temperatureResult.outheatMinTemperature, temperatureResult.outheatAvgTemperature, temperatureResult.outheatStandardTemperature,
													temperatureResult.insulatingheatMaxTemperature, temperatureResult.insulatingheatMinTemperature, temperatureResult.insulatingheatAvgTemperature, temperatureResult.insulatingheatStandardTemperature);

												auto overpressureResult = ModelDataManager::GetInstance()->GetSacrificeExplosionOverpressureResult();
												gfParent->GetSacrificeExplosionOverpressureResultWidget()->updateData(overpressureResult.metalsMaxOverpressure, overpressureResult.metalsMinOverpressure, overpressureResult.metalsAvgOverpressure, overpressureResult.metalsStandardOverpressure,
													overpressureResult.propellantsMaxOverpressure, overpressureResult.propellantsMinOverpressure, overpressureResult.mpropellantsAvgOverpressure, overpressureResult.propellantsStandardOverpressure,
													overpressureResult.outheatMaxOverpressure, overpressureResult.outheatMinOverpressure, overpressureResult.outheatAvgOverpressure, overpressureResult.outheatStandardOverpressure,
													overpressureResult.insulatingheatMaxOverpressure, overpressureResult.insulatingheatMinOverpressure, overpressureResult.insulatingheatAvgOverpressure, overpressureResult.insulatingheatStandardOverpressure);

												// 更新判断结果
												auto tableWidget = gfParent->GetSacrificeExplosionPropertyWidget()->GetQTableWidget();
												if (stressResult.metalsMaxStress > tensileStrength)
												{
													tableWidget->item(8, 2)->setText("应力超过壳体最大抗拉强度，有燃爆风险");
												}
												else
												{
													tableWidget->item(8, 2)->setText("应力未超过壳体最大抗拉强度");
												}
												if (temperatureResult.metalsMaxTemperature > ignitionTemperature)
												{
													tableWidget->item(9, 2)->setText("温度超过推进剂最大发火温度，有燃爆风险");
												}
												else
												{
													tableWidget->item(9, 2)->setText("温度超过推进剂最大发火温度");
												}
												if (overpressureResult.metalsMaxOverpressure > fireOverpressure)
												{
													tableWidget->item(10, 2)->setText("超压超过推进剂最大发火超压，有燃爆风险");
												}
												else
												{
													tableWidget->item(10, 2)->setText("超压超过推进剂最大发火超压");
												}
											}
											else
											{
												QString text = timeStr + "[信息]>殉爆安全性分析计算失败";
												textEdit->appendPlainText(text);
											}
										}
									}
								}
								logWidget->update();
							}
							else if (!success)
							{
								QMessageBox::warning(this, "计算", msg);
							}

							// 清理资源
							progressDialog->close();
							workerThread->quit();
							workerThread->wait();
							worker->deleteLater();
							workerThread->deleteLater();
							progressDialog->deleteLater();
						});

					// 启动线程
					workerThread->start();
					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
			});
			
	
		connect(exportAction, &QAction::triggered, [this, item]() {
			QString directory = QFileDialog::getExistingDirectory(nullptr,
				tr("选择文件夹"),
				"/home", // 默认的起始目录
				QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks); // 选项
			if (!directory.isEmpty()) {
				exportWord(directory, item); // 直接在Lambda中传递参数
			}
		});
		contextMenu->addAction(calAction); // 将动作添加到菜单中
		contextMenu->addAction(exportAction);
		contextMenu->exec(event->globalPos()); // 在鼠标位置显示菜单
	}
	else if (text == "固体发动机三维模型导入")
	{
		contextMenu = new QMenu(this); // 创建菜单对象
		QAction *customAction = new QAction("导入", this); // 创建动作对象并添加到菜单中
		connect(customAction, &QAction::triggered, this, [item, this]() {
			QWidget* parent = parentWidget();
			while (parent) {
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{
					QString filePath = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(),
						"STEP Files (*.stp *.step);;IGES Files (*.iges *.igs);;VTK Files (*.vtk);;X_T Files (*.x_t);;All Files (*.*)");

					if (filePath.isEmpty())
					{
						return;
					}
				
					QDateTime currentTime = QDateTime::currentDateTime();
					QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
					auto logWidget = gfParent->GetLogWidget();
					auto textEdit = logWidget->GetTextEdit();
					QString text = timeStr + "[信息]>开始导入几何模型";
					textEdit->appendPlainText(text);
					logWidget->update();

					// 关键：强制刷新UI，确保日志立即显示
					QApplication::processEvents();
					

					// 创建进度对话框
					ProgressDialog* progressDialog = new ProgressDialog("固体发动机三维模型导入", gfParent);
					progressDialog->show();

					// 创建工作线程和工作对象
					GeometryImportWorker* worker = new GeometryImportWorker(filePath);
					QThread* workerThread = new QThread();
					worker->moveToThread(workerThread);

					// 连接信号槽
					connect(workerThread, &QThread::started, worker, &GeometryImportWorker::DoWork);
					connect(worker, &GeometryImportWorker::ProgressUpdated,
						progressDialog, &ProgressDialog::SetProgress);
					connect(worker, &GeometryImportWorker::StatusUpdated,
						progressDialog, &ProgressDialog::SetStatusText);
					connect(progressDialog, &ProgressDialog::Canceled,
						worker, &GeometryImportWorker::RequestInterruption,
						Qt::DirectConnection); 

					// 处理导入结果
					connect(worker, &GeometryImportWorker::WorkFinished, this,
						[=](bool success, const QString& msg, const ModelGeometryInfo& info) {
							// 更新日志
							QDateTime finishTime = QDateTime::currentDateTime();
							QString finishTimeStr = finishTime.toString("yyyy-MM-dd hh:mm:ss");
							textEdit->appendPlainText(finishTimeStr + "[" + (success ? "信息" : "错误") + "]>" + msg);

							if (success && !info.shape.IsNull())
							{
								// 保存模型信息
								ModelDataManager::GetInstance()->SetModelGeometryInfo(info);
								updataIcon();

								// 更新显示
								auto occView = gfParent->GetOccView();
								Handle(AIS_InteractiveContext) context = occView->getContext();
								context->EraseAll(true);

								Handle(AIS_Shape) modelPresentation = new AIS_Shape(info.shape);
								context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
								context->SetColor(modelPresentation, Quantity_Color(0.0, 1.0, 1.0, Quantity_TOC_RGB), true);
								context->Display(modelPresentation, false);

								/*
								if (info.hasNozzle) {
									QString nozzleInfo = QString(
										"[喷管分析] 连接点: (%.3f, %.3f, %.3f), 半径: %.3f, 发动机长度: %.3f, 喷管长度: %.3f, 厚度: %.3f"
									)
										.arg(info.connectionPoint.X())
										.arg(info.connectionPoint.Y())
										.arg(info.connectionPoint.Z())
										.arg(info.cylinderRadius)
										.arg(info.engineLength)
										.arg(info.nozzleLength)
										.arg(info.thickness);

									textEdit->appendPlainText(nozzleInfo);
								}

								// 可选：3D视图中标记红色连接点
								if (info.hasNozzle) {
									gp_Ax2 sphereAxis(info.connectionPoint, gp_Dir(0, 0, 1));
									TopoDS_Shape marker = BRepPrimAPI_MakeSphere(
										sphereAxis,
										info.cylinderRadius * 0.3
									).Shape();
									Handle(AIS_Shape) markerAIS = new AIS_Shape(marker);
									context->SetColor(markerAIS, Quantity_Color(1.0, 0.0, 0.0, Quantity_TOC_RGB), true);
									context->SetDisplayMode(markerAIS, AIS_Shaded, true);
									context->Display(markerAIS, false);
								}
								*/
								// 在导入成功回调中，新增底部端点标记
								//if (info.hasNozzle) {
								//	gp_Ax2 sphereAxis1(info.bottomEndPoint, gp_Dir(0, 0, 1));
								//	TopoDS_Shape marker1 = BRepPrimAPI_MakeSphere(sphereAxis1, info.cylinderRadius * 0.3).Shape();
								//	Handle(AIS_Shape) markerAIS1 = new AIS_Shape(marker1);
								//	context->SetColor(markerAIS1, Quantity_Color(0.0, 1.0, 0.0, Quantity_TOC_RGB), true); // 绿色
								//	context->Display(markerAIS1, false);

								//	// 后封头底部（右侧）- 蓝色
								//	gp_Ax2 sphereAxis2(info.bottomEndPoint2, gp_Dir(0, 0, 1));
								//	TopoDS_Shape marker2 = BRepPrimAPI_MakeSphere(sphereAxis2, info.cylinderRadius * 0.3).Shape();
								//	Handle(AIS_Shape) markerAIS2 = new AIS_Shape(marker2);
								//	context->SetColor(markerAIS2, Quantity_Color(0.0, 0.0, 1.0, Quantity_TOC_RGB), true); // 蓝色
								//	context->Display(markerAIS2, false);
								//}




								occView->fitAll();

								// 更新属性窗口
								auto geomProWid = gfParent->findChild<GeomPropertyWidget*>();
								geomProWid->UpdataPropertyInfo();
							}
							else if (!success)
							{
								QMessageBox::warning(this, "导入失败", msg);
							}

							// 清理资源
							progressDialog->close();
							workerThread->quit();
							if (!workerThread->wait(500)) 
							{  
								workerThread->terminate();
							}
							worker->deleteLater();
							workerThread->deleteLater();
							progressDialog->deleteLater();

							UserInfo userinfo = ModelDataManager::GetInstance()->GetUserInfo();
							// 截图计算模型
							QString m_privateDirPath = userinfo.workdir + "/template/main.png";
							QDir privateDir(m_privateDirPath);
							wordExporter->captureWidgetToFile(gfParent->GetOccView(), m_privateDirPath);
						});

					// 启动线程
					workerThread->start();
					break;					
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
		});
		contextMenu->addAction(customAction); // 将动作添加到菜单中
		contextMenu->exec(event->globalPos()); // 在鼠标位置显示菜单
	}
	else if (text == "网格")
	{
		contextMenu = new QMenu(this);
		QAction *meshAction = new QAction("网格划分", this);
		connect(meshAction, &QAction::triggered, this, [item, this]() {
			QWidget* parent = parentWidget();
			while (parent)
			{
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{								
					QDateTime currentTime = QDateTime::currentDateTime();
					QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
					auto logWidget = gfParent->GetLogWidget();
					auto textEdit = logWidget->GetTextEdit();
					QString text = timeStr + "[信息]>启动网格划分引擎，采用自适应尺寸控制算法";
					textEdit->appendPlainText(text);
					logWidget->update();
					
					auto occView = gfParent->GetOccView();
					Handle(AIS_InteractiveContext) context = occView->getContext();
					auto view = occView->getView();
					context->EraseAll(true);

					// 创建进度对话框
					ProgressDialog* progressDialog = new ProgressDialog("网格划分", gfParent);
					progressDialog->show();

					// 创建工作线程和工作对象
					auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
					TriangulationWorker* worker = new TriangulationWorker(geomInfo.shape);
					QThread* workerThread = new QThread();
					worker->moveToThread(workerThread);

					// 连接信号槽
					connect(workerThread, &QThread::started, worker, &TriangulationWorker::DoWork);
					connect(worker, &TriangulationWorker::ProgressUpdated,
						progressDialog, &ProgressDialog::SetProgress);
					connect(worker, &TriangulationWorker::StatusUpdated,
						progressDialog, &ProgressDialog::SetStatusText);
					connect(progressDialog, &ProgressDialog::Canceled,
						worker, &TriangulationWorker::RequestInterruption,
						Qt::DirectConnection);

					// 处理导入结果
					connect(worker, &TriangulationWorker::WorkFinished, this,
						[=](bool success, const QString& msg, const ModelMeshInfo& info) {
							// 更新日志
							QDateTime finishTime = QDateTime::currentDateTime();
							QString finishTimeStr = finishTime.toString("yyyy-MM-dd hh:mm:ss");
							textEdit->appendPlainText(finishTimeStr + "[" + (success ? "信息" : "错误") + "]>" + msg);
							if (success)
							{
								ModelDataManager::GetInstance()->SetModelMeshInfo(info);
								BRep_Builder builder;
								TopoDS_Compound compound;
								builder.MakeCompound(compound);

								auto aDataSource = info.triangleStructure;
								auto myEdges = aDataSource.GetMyEdge();
								auto myNodeCoords = aDataSource.GetmyNodeCoords();
								for (const auto& edge : myEdges)
								{
									Standard_Integer node1ID = edge.first;
									Standard_Integer node2ID = edge.second;

									Standard_Real x1 = myNodeCoords->Value(node1ID, 1);
									Standard_Real y1 = myNodeCoords->Value(node1ID, 2);
									Standard_Real z1 = myNodeCoords->Value(node1ID, 3);

									Standard_Real x2 = myNodeCoords->Value(node2ID, 1);
									Standard_Real y2 = myNodeCoords->Value(node2ID, 2);
									Standard_Real z2 = myNodeCoords->Value(node2ID, 3);

									gp_Pnt p1(x1, y1, z1);
									gp_Pnt p2(x2, y2, z2);

									TopoDS_Vertex v1 = BRepBuilderAPI_MakeVertex(p1);
									TopoDS_Vertex v2 = BRepBuilderAPI_MakeVertex(p2);

									TopoDS_Edge edgeShape = BRepBuilderAPI_MakeEdge(v1, v2);

									builder.Add(compound, edgeShape);
								}
								Handle(AIS_Shape) aisCompound = new AIS_Shape(compound);
								context->Display(aisCompound, Standard_True);

								updataIcon();

								auto meshProWid = gfParent->findChild<MeshPropertyWidget*>();
								meshProWid->UpdataPropertyInfo();
							}
							else if (!success)
							{
								QMessageBox::warning(this, "导入失败", msg);
							}

							// 清理资源
							progressDialog->close();
							workerThread->quit();
							workerThread->wait();
							worker->deleteLater();
							workerThread->deleteLater();
							progressDialog->deleteLater();
						});

					// 启动线程
					workerThread->start();

					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
		});
		contextMenu->addAction(meshAction);
		contextMenu->exec(event->globalPos());
	}
}

void GFTreeModelWidget::exportWord(const QString& directory, QTreeWidgetItem* item)
{
	UserInfo userinfo = ModelDataManager::GetInstance()->GetUserInfo();
	QString workdir = userinfo.workdir;

	QWidget* parent = parentWidget();
	while (parent) {
		GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
		if (gfParent)
		{
			auto logWidget = gfParent->GetLogWidget();
			auto textEdit = logWidget->GetTextEdit();

			ProjectPropertyWidge* m_projectPropertyWidge = gfParent->GetProjectPropertyWidget();
			QTableWidget* m_projectTableWid = m_projectPropertyWidge->GetQTableWidget();

			GeomPropertyWidget* m_geomPropertyWidget = gfParent->GetGeomPropertyWidget();
			QTableWidget* m_geomTableWid = m_geomPropertyWidget->GetQTableWidget();

			MaterialPropertyWidget* m_materialPropertyWidget = gfParent->GetMaterialPropertyWidget();
			QTableWidget* m_materialTableWid = m_materialPropertyWidget->GetQTableWidget();

			DatabasePropertyWidget* m_databasePropertyWidget = gfParent->GetDatabasePropertyWidget();
			QTableWidget* m_databaseTableWid = m_databasePropertyWidget->GetQTableWidget();

			for (int i = 0; i < item->childCount(); ++i) {
				QTreeWidgetItem* childItem = item->child(i);
				auto originalName = childItem->text(0);
				int dotIndex = originalName.indexOf('.');
				QString processedName;
				if (dotIndex != -1)
				{
					processedName = originalName.mid(dotIndex + 1).trimmed();
				}
				else {
					processedName = originalName;
				}

				bool isChecked = (childItem->checkState(0) == Qt::Checked);
				if (isChecked)
				{
					if (processedName == "跌落安全性分析")
					{
						{
							QDateTime currentTime = QDateTime::currentDateTime();
							QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
							QString text = timeStr + "[信息]>开始导出跌落安全性分析报告";
							textEdit->appendPlainText(text);
							logWidget->update();
							// 关键：强制刷新UI，确保日志立即显示
							QApplication::processEvents();
						}

						FallPropertyWidget* m_fallPropertyWidget = gfParent->GetFallPropertyWidget();
						QTableWidget* m_fallTableWid = m_fallPropertyWidget->GetQTableWidget();

						StressResultWidget* m_stressResultWidget = gfParent->GetStressResultWidget();
						QTableWidget* m_stressTableWid = m_stressResultWidget->GetQTableWidget();

						StrainResultWidget* m_strainResultWidget = gfParent->GetStrainResultWidget();
						QTableWidget* m_strainTableWid = m_strainResultWidget->GetQTableWidget();

						TemperatureResultWidget* m_temperatureResultWidget = gfParent->GetTemperatureResultWidget();
						QTableWidget* m_temperatureTableWid = m_temperatureResultWidget->GetQTableWidget();

						OverpressureResultWidget* m_overpressureResultWidge = gfParent->GetOverpressureResultWidget();
						QTableWidget* m_overpressureTableWid = m_overpressureResultWidge->GetQTableWidget();

						QMap<QString, QVariant> data = convertTextData(m_projectPropertyWidge,
							m_geomPropertyWidget,
							m_materialPropertyWidget,
							m_databasePropertyWidget,
							m_stressResultWidget,
							m_strainResultWidget,
							m_temperatureResultWidget,
							m_overpressureResultWidge);
						
						// 跌落输入数据
						data.insert("测试项目", m_fallTableWid->item(1, 2)->text());
						data.insert("跌落高度", m_fallTableWid->item(2, 2)->text());
						QComboBox* comboBox = qobject_cast<QComboBox*>(m_fallTableWid->cellWidget(3, 2));
						if (comboBox)
						{
							QString selectedText = comboBox->currentText();
							data.insert("跌落姿态", selectedText);
						}
						else
						{
							data.insert("跌落姿态", "");
						}
						data.insert("跌落钢板硬度", m_fallTableWid->item(4, 2)->text());
						data.insert("温度传感器数量", m_fallTableWid->item(5, 2)->text());
						if (m_fallTableWid->item(6, 2))
						{
							data.insert("冲击波超压传感器数量", m_fallTableWid->item(6, 2)->text());
						}
						else
						{
							data.insert("冲击波超压传感器数量", "");
						}
						data.insert("风速", m_fallTableWid->item(7, 2)->text());

						

						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("应力云图", QDir(workdir + "/template/fall/Stress.png").absolutePath());
						imagePaths.insert("应变云图", QDir(workdir + "/template/fall/Strain.png").absolutePath());
						imagePaths.insert("温度云图", QDir(workdir + "/template/fall/Temperature.png").absolutePath());
						imagePaths.insert("超压云图", QDir(workdir + "/template/fall/Overpressure.png").absolutePath());
						QMap<QString, QVector<QVector<QVariant>>> tableData;

						// 创建进度对话框
						ProgressDialog* progressDialog = new ProgressDialog("导出跌落仿真报告进度", gfParent);
						progressDialog->show();

						// 创建工作线程和工作对象
						//WordExporterWorker* wordExporterWorker = new WordExporterWorker(QDir("src/template/跌落仿真计算数据表.docx").absolutePath(), directory + "/跌落仿真计算数据表.docx", data, imagePaths, tableData);
						WordExporterWorker* wordExporterWorker = new WordExporterWorker("跌落仿真计算数据表.docx", directory + "/跌落仿真计算数据表.docx", data, imagePaths, tableData);
						QThread* wordExporterThread = new QThread();
						wordExporterWorker->moveToThread(wordExporterThread);

						// 连接信号槽
						connect(wordExporterThread, &QThread::started, wordExporterWorker, &WordExporterWorker::DoWork);
						connect(wordExporterWorker, &WordExporterWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
						connect(wordExporterWorker, &WordExporterWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
						connect(progressDialog, &ProgressDialog::Canceled, wordExporterWorker, &WordExporterWorker::RequestInterruption, Qt::DirectConnection);

						// 处理导入结果
						connect(wordExporterWorker, &WordExporterWorker::WorkFinished, this,
							[=](bool success, const QString& msg)
							{
								if (success)
								{
									// 更新日志
									{
										QDateTime currentTime = QDateTime::currentDateTime();
										QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
										QString text = timeStr + "[信息]>成功导出跌落安全性分析报告";
										text = text + "\n" + timeStr + "[信息]>跌落安全性分析报告：" + directory + "/跌落仿真计算数据表.docx";
										textEdit->appendPlainText(text);
										logWidget->update();

										// 关键：强制刷新UI，确保日志立即显示
										QApplication::processEvents();
									}

								}
								else if (!success)
								{
									QMessageBox::warning(this, "导出失败", msg);
								}
								// 清理资源
								progressDialog->close();
								wordExporterThread->quit();
								wordExporterThread->wait();
								wordExporterWorker->deleteLater();
								wordExporterThread->deleteLater();
								progressDialog->deleteLater();
							});
						// 启动线程
						wordExporterThread->start();

					}
					else if (processedName == "快速烤燃安全性分析")
					{
						{
							QDateTime currentTime = QDateTime::currentDateTime();
							QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
							QString text = timeStr + "[信息]>开始导出快速烤燃安全性分析报告";
							textEdit->appendPlainText(text);
							logWidget->update();
							// 关键：强制刷新UI，确保日志立即显示
							QApplication::processEvents();
						}

						FastCombustionPropertyWidget* m_fastCombustionPropertyWidget = gfParent->GetFastCombustionPropertyWidget();
						QTableWidget* m_fastCombustionTableWid = m_fastCombustionPropertyWidget->GetQTableWidget();

						TemperatureResultWidget* m_temperatureResultWidget = gfParent->GetFastCombustionTemperatureResultWidget();
						QTableWidget* m_temperatureTableWid = m_temperatureResultWidget->GetQTableWidget();

						QMap<QString, QVariant> data = convertTextData(m_projectPropertyWidge,
							m_geomPropertyWidget,
							m_materialPropertyWidget,
							m_databasePropertyWidget,
							nullptr,
							nullptr,
							m_temperatureResultWidget,
							nullptr);

						// 跌落输入数据
						data.insert("测试项目", m_fastCombustionTableWid->item(1, 2)->text());
						data.insert("燃油类型", m_fastCombustionTableWid->item(2, 2)->text());
						data.insert("弹药位置", m_fastCombustionTableWid->item(3, 2)->text());
						data.insert("温度传感器数量", m_fastCombustionTableWid->item(4, 2)->text());
						data.insert("冲击波超压传感器数量", m_fastCombustionTableWid->item(5, 2)->text());
						data.insert("风速", m_fastCombustionTableWid->item(6, 2)->text());
						data.insert("火焰温度达到时间", m_fastCombustionTableWid->item(7, 2)->text());
						data.insert("结束时间", m_fastCombustionTableWid->item(8, 2)->text());
						data.insert("平均温度", m_fastCombustionTableWid->item(9, 2)->text());


						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("温度云图", QDir(workdir + "/template/fastCombustion/Temperature.png").absolutePath());
						QMap<QString, QVector<QVector<QVariant>>> tableData;

						// 创建进度对话框
						ProgressDialog* progressDialog = new ProgressDialog("导出快速烤燃仿真计算报告进度", gfParent);
						progressDialog->show();

						// 创建工作线程和工作对象
						//WordExporterWorker* wordExporterWorker = new WordExporterWorker(QDir("src/template/快速烤燃仿真计算数据表.docx").absolutePath(), directory + "/快速烤燃仿真计算数据表.docx", data, imagePaths, tableData);
						WordExporterWorker* wordExporterWorker = new WordExporterWorker("快速烤燃仿真计算数据表.docx", directory + "/快速烤燃仿真计算数据表.docx", data, imagePaths, tableData);
						QThread* wordExporterThread = new QThread();
						wordExporterWorker->moveToThread(wordExporterThread);

						// 连接信号槽
						connect(wordExporterThread, &QThread::started, wordExporterWorker, &WordExporterWorker::DoWork);
						connect(wordExporterWorker, &WordExporterWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
						connect(wordExporterWorker, &WordExporterWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
						connect(progressDialog, &ProgressDialog::Canceled, wordExporterWorker, &WordExporterWorker::RequestInterruption, Qt::DirectConnection);

						// 处理导入结果
						connect(wordExporterWorker, &WordExporterWorker::WorkFinished, this,
							[=](bool success, const QString& msg)
							{
								if (success)
								{
									// 更新日志
									{
										QDateTime currentTime = QDateTime::currentDateTime();
										QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
										QString text = timeStr + "[信息]>成功导出快速烤燃试安全性分析报告";
										text = text + "\n" + timeStr + "[信息]>快速烤燃安全性分析报告：" + directory + "/快速烤燃仿真计算数据表.docx";
										textEdit->appendPlainText(text);
										logWidget->update();

										// 关键：强制刷新UI，确保日志立即显示
										QApplication::processEvents();
									}

								}
								else if (!success)
								{
									QMessageBox::warning(this, "导出失败", msg);
								}
								// 清理资源
								progressDialog->close();
								wordExporterThread->quit();
								wordExporterThread->wait();
								wordExporterWorker->deleteLater();
								wordExporterThread->deleteLater();
								progressDialog->deleteLater();
							});
						// 启动线程
						wordExporterThread->start();
					}
					else if (processedName == "慢速烤燃安全性分析")
					{
						{
							QDateTime currentTime = QDateTime::currentDateTime();
							QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
							QString text = timeStr + "[信息]>开始导出慢速烤燃安全性分析报告";
							textEdit->appendPlainText(text);
							logWidget->update();
							// 关键：强制刷新UI，确保日志立即显示
							QApplication::processEvents();
						}

						SlowCombustionPropertyWidget* m_slowCombustionPropertyWidget = gfParent->GetSlowCombustionPropertyWidget();
						QTableWidget* m_slowCombustionTableWid = m_slowCombustionPropertyWidget->GetQTableWidget();

						TemperatureResultWidget* m_temperatureResultWidget = gfParent->GetSlowCombustionTemperatureResultWidget();
						QTableWidget* m_temperatureTableWid = m_temperatureResultWidget->GetQTableWidget();

						QMap<QString, QVariant> data = convertTextData(m_projectPropertyWidge,
							m_geomPropertyWidget,
							m_materialPropertyWidget,
							m_databasePropertyWidget,
							nullptr,
							nullptr,
							m_temperatureResultWidget,
							nullptr);

						// 跌落输入数据
						data.insert("测试项目", m_slowCombustionTableWid->item(1, 2)->text());
						data.insert("加热类型", m_slowCombustionTableWid->item(2, 2)->text());
						data.insert("弹药位置", m_slowCombustionTableWid->item(3, 2)->text());
						data.insert("温度传感器数量", m_slowCombustionTableWid->item(4, 2)->text());
						data.insert("冲击波超压传感器数量", m_slowCombustionTableWid->item(5, 2)->text());
						data.insert("风速", m_slowCombustionTableWid->item(6, 2)->text());
						data.insert("平衡时刻", m_slowCombustionTableWid->item(7, 2)->text());
						data.insert("烘箱升温速率", m_slowCombustionTableWid->item(8, 2)->text());
						data.insert("烘箱终止温度}", m_slowCombustionTableWid->item(9, 2)->text());


						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("温度云图", QDir(workdir + "/template/slowCombustion/Temperature.png").absolutePath());
						QMap<QString, QVector<QVector<QVariant>>> tableData;

						// 创建进度对话框
						ProgressDialog* progressDialog = new ProgressDialog("导出慢速烤燃仿真计算报告进度", gfParent);
						progressDialog->show();

						// 创建工作线程和工作对象
						//WordExporterWorker* wordExporterWorker = new WordExporterWorker(QDir("src/template/慢速烤燃仿真计算数据表.docx").absolutePath(), directory + "/慢速烤燃仿真计算数据表.docx", data, imagePaths, tableData);
						WordExporterWorker* wordExporterWorker = new WordExporterWorker("慢速烤燃仿真计算数据表.docx", directory + "/慢速烤燃仿真计算数据表.docx", data, imagePaths, tableData);
						QThread* wordExporterThread = new QThread();
						wordExporterWorker->moveToThread(wordExporterThread);

						// 连接信号槽
						connect(wordExporterThread, &QThread::started, wordExporterWorker, &WordExporterWorker::DoWork);
						connect(wordExporterWorker, &WordExporterWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
						connect(wordExporterWorker, &WordExporterWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
						connect(progressDialog, &ProgressDialog::Canceled, wordExporterWorker, &WordExporterWorker::RequestInterruption, Qt::DirectConnection);

						// 处理导入结果
						connect(wordExporterWorker, &WordExporterWorker::WorkFinished, this,
							[=](bool success, const QString& msg)
							{
								if (success)
								{
									// 更新日志
									{
										QDateTime currentTime = QDateTime::currentDateTime();
										QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
										QString text = timeStr + "[信息]>成功导出慢速烤燃安全性分析报告";
										text = text + "\n" + timeStr + "[信息]>慢速烤燃安全性分析报告：" + directory + "/慢速烤燃仿真计算数据表.docx";
										textEdit->appendPlainText(text);
										logWidget->update();

										// 关键：强制刷新UI，确保日志立即显示
										QApplication::processEvents();
									}

								}
								else if (!success)
								{
									QMessageBox::warning(this, "导出失败", msg);
								}
								// 清理资源
								progressDialog->close();
								wordExporterThread->quit();
								wordExporterThread->wait();
								wordExporterWorker->deleteLater();
								wordExporterThread->deleteLater();
								progressDialog->deleteLater();
							});
						// 启动线程
						wordExporterThread->start();
					}
					else if (processedName == "枪击安全性分析")
					{
						{
							QDateTime currentTime = QDateTime::currentDateTime();
							QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
							QString text = timeStr + "[信息]>开始导出枪击安全性分析报告";
							textEdit->appendPlainText(text);
							logWidget->update();
							// 关键：强制刷新UI，确保日志立即显示
							QApplication::processEvents();
						}

						ShootPropertyWidget* m_shootPropertyWidget = gfParent->GetShootPropertyWidget();
						QTableWidget* m_shootTableWid = m_shootPropertyWidget->GetQTableWidget();

						StressResultWidget* m_stressResultWidget = gfParent->GetShootStressResultWidget();
						QTableWidget* m_stressTableWid = m_stressResultWidget->GetQTableWidget();

						StrainResultWidget* m_strainResultWidget = gfParent->GetShootStrainResultWidget();
						QTableWidget* m_strainTableWid = m_strainResultWidget->GetQTableWidget();

						TemperatureResultWidget* m_temperatureResultWidget = gfParent->GetShootTemperatureResultWidget();
						QTableWidget* m_temperatureTableWid = m_temperatureResultWidget->GetQTableWidget();

						OverpressureResultWidget* m_overpressureResultWidge = gfParent->GetShootOverpressureResultWidget();
						QTableWidget* m_overpressureTableWid = m_overpressureResultWidge->GetQTableWidget();

						QMap<QString, QVariant> data = convertTextData(m_projectPropertyWidge,
							m_geomPropertyWidget,
							m_materialPropertyWidget,
							m_databasePropertyWidget,
							m_stressResultWidget,
							m_strainResultWidget,
							m_temperatureResultWidget,
							m_overpressureResultWidge);

						// 跌落输入数据
						data.insert("测试项目", m_shootTableWid->item(1, 2)->text());
						data.insert("撞击速度", m_shootTableWid->item(2, 2)->text());
						data.insert("撞击角度", m_shootTableWid->item(3, 2)->text());
						data.insert("子弹型式", m_shootTableWid->item(4, 2)->text());
						data.insert("子弹直径", m_shootTableWid->item(5, 2)->text());
						data.insert("子弹硬度", m_shootTableWid->item(6, 2)->text());
						data.insert("温度传感器数量", m_shootTableWid->item(7, 2)->text());
						data.insert("超压传感器数量", m_shootTableWid->item(8, 2)->text());
						data.insert("风速", m_shootTableWid->item(9, 2)->text());


						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("应力云图", QDir(workdir + "/template/shoot/Stress.png").absolutePath());
						imagePaths.insert("应变云图", QDir(workdir + "/template/shoot/Strain.png").absolutePath());
						imagePaths.insert("温度云图", QDir(workdir + "/template/shoot/Temperature.png").absolutePath());
						imagePaths.insert("超压云图", QDir(workdir + "/template/shoot/Overpressure.png").absolutePath());
						QMap<QString, QVector<QVector<QVariant>>> tableData;

						// 创建进度对话框
						ProgressDialog* progressDialog = new ProgressDialog("导出枪击仿真计算报告进度", gfParent);
						progressDialog->show();

						// 创建工作线程和工作对象
						//WordExporterWorker* wordExporterWorker = new WordExporterWorker(QDir("src/template/枪击仿真计算数据表.docx").absolutePath(), directory + "/枪击仿真计算数据表.docx", data, imagePaths, tableData);
						WordExporterWorker* wordExporterWorker = new WordExporterWorker("枪击仿真计算数据表.docx", directory + "/枪击仿真计算数据表.docx", data, imagePaths, tableData);
						QThread* wordExporterThread = new QThread();
						wordExporterWorker->moveToThread(wordExporterThread);

						// 连接信号槽
						connect(wordExporterThread, &QThread::started, wordExporterWorker, &WordExporterWorker::DoWork);
						connect(wordExporterWorker, &WordExporterWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
						connect(wordExporterWorker, &WordExporterWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
						connect(progressDialog, &ProgressDialog::Canceled, wordExporterWorker, &WordExporterWorker::RequestInterruption, Qt::DirectConnection);

						// 处理导入结果
						connect(wordExporterWorker, &WordExporterWorker::WorkFinished, this,
							[=](bool success, const QString& msg)
							{
								if (success)
								{
									// 更新日志
									{
										QDateTime currentTime = QDateTime::currentDateTime();
										QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
										QString text = timeStr + "[信息]>成功导出枪击安全性分析报告";
										text = text + "\n" + timeStr + "[信息]>枪击安全性分析报告：" + directory + "/枪击仿真计算数据表.docx";
										textEdit->appendPlainText(text);
										logWidget->update();

										// 关键：强制刷新UI，确保日志立即显示
										QApplication::processEvents();
									}

								}
								else if (!success)
								{
									QMessageBox::warning(this, "导出失败", msg);
								}
								// 清理资源
								progressDialog->close();
								wordExporterThread->quit();
								wordExporterThread->wait();
								wordExporterWorker->deleteLater();
								wordExporterThread->deleteLater();
								progressDialog->deleteLater();
							});
						// 启动线程
						wordExporterThread->start();
					}
					else if (processedName == "射流冲击安全性分析")
					{
						{
							QDateTime currentTime = QDateTime::currentDateTime();
							QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
							QString text = timeStr + "[信息]>开始导出射流冲击安全性分析报告";
							textEdit->appendPlainText(text);
							logWidget->update();
							// 关键：强制刷新UI，确保日志立即显示
							QApplication::processEvents();
						}

						JetImpactPropertyWidget* m_jetImpactPropertyWidget = gfParent->GetJetImpactPropertyWidget();
						QTableWidget* m_jetImpactTableWid = m_jetImpactPropertyWidget->GetQTableWidget();

						StressResultWidget* m_stressResultWidget = gfParent->GetJetImpactStressResultWidget();
						QTableWidget* m_stressTableWid = m_stressResultWidget->GetQTableWidget();

						StrainResultWidget* m_strainResultWidget = gfParent->GetJetImpactStrainResultWidget();
						QTableWidget* m_strainTableWid = m_strainResultWidget->GetQTableWidget();

						TemperatureResultWidget* m_temperatureResultWidget = gfParent->GetJetImpactTemperatureResultWidget();
						QTableWidget* m_temperatureTableWid = m_temperatureResultWidget->GetQTableWidget();

						OverpressureResultWidget* m_overpressureResultWidge = gfParent->GetJetImpactOverpressureResultWidget();
						QTableWidget* m_overpressureTableWid = m_overpressureResultWidge->GetQTableWidget();

						QMap<QString, QVariant> data = convertTextData(m_projectPropertyWidge,
							m_geomPropertyWidget,
							m_materialPropertyWidget,
							m_databasePropertyWidget,
							m_stressResultWidget,
							m_strainResultWidget,
							m_temperatureResultWidget,
							m_overpressureResultWidge);

						// 跌落输入数据
						data.insert("测试项目", m_jetImpactTableWid->item(1, 2)->text());
						data.insert("聚能装药口径", m_jetImpactTableWid->item(2, 2)->text());
						data.insert("炸高", m_jetImpactTableWid->item(3, 2)->text());
						data.insert("冲击点角度", m_jetImpactTableWid->item(4, 2)->text());
						data.insert("温度传感器数量", m_jetImpactTableWid->item(5, 2)->text());
						data.insert("超压传感器数量", m_jetImpactTableWid->item(6, 2)->text());
						data.insert("风速", m_jetImpactTableWid->item(7, 2)->text());
						


						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("应力云图", QDir(workdir + "/template/jetImpact/Stress.png").absolutePath());
						imagePaths.insert("应变云图", QDir(workdir + "/template/jetImpact/Strain.png").absolutePath());
						imagePaths.insert("温度云图", QDir(workdir + "/template/jetImpact/Temperature.png").absolutePath());
						imagePaths.insert("超压云图", QDir(workdir + "/template/jetImpact/Overpressure.png").absolutePath());
						QMap<QString, QVector<QVector<QVariant>>> tableData;

						// 创建进度对话框
						ProgressDialog* progressDialog = new ProgressDialog("导出射流冲击仿真计算报告进度", gfParent);
						progressDialog->show();

						// 创建工作线程和工作对象
						//WordExporterWorker* wordExporterWorker = new WordExporterWorker(QDir("src/template/射流冲击仿真计算数据表.docx").absolutePath(), directory + "/射流冲击仿真计算数据表.docx", data, imagePaths, tableData);
						WordExporterWorker* wordExporterWorker = new WordExporterWorker("射流冲击仿真计算数据表.docx", directory + "/射流冲击仿真计算数据表.docx", data, imagePaths, tableData);
						QThread* wordExporterThread = new QThread();
						wordExporterWorker->moveToThread(wordExporterThread);

						// 连接信号槽
						connect(wordExporterThread, &QThread::started, wordExporterWorker, &WordExporterWorker::DoWork);
						connect(wordExporterWorker, &WordExporterWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
						connect(wordExporterWorker, &WordExporterWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
						connect(progressDialog, &ProgressDialog::Canceled, wordExporterWorker, &WordExporterWorker::RequestInterruption, Qt::DirectConnection);

						// 处理导入结果
						connect(wordExporterWorker, &WordExporterWorker::WorkFinished, this,
							[=](bool success, const QString& msg)
							{
								if (success)
								{
									// 更新日志
									{
										QDateTime currentTime = QDateTime::currentDateTime();
										QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
										QString text = timeStr + "[信息]>成功导出射流冲击安全性分析报告";
										text = text + "\n" + timeStr + "[信息]>射流冲击安全性分析报告：" + directory + "/射流冲击仿真计算数据表.docx";
										textEdit->appendPlainText(text);
										logWidget->update();

										// 关键：强制刷新UI，确保日志立即显示
										QApplication::processEvents();
									}

								}
								else if (!success)
								{
									QMessageBox::warning(this, "导出失败", msg);
								}
								// 清理资源
								progressDialog->close();
								wordExporterThread->quit();
								wordExporterThread->wait();
								wordExporterWorker->deleteLater();
								wordExporterThread->deleteLater();
								progressDialog->deleteLater();
							});
						// 启动线程
						wordExporterThread->start();
					}
					else if (processedName == "破片撞击安全性分析")
					{
						{
							QDateTime currentTime = QDateTime::currentDateTime();
							QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
							QString text = timeStr + "[信息]>开始导出破片撞击安全性分析报告";
							textEdit->appendPlainText(text);
							logWidget->update();
							// 关键：强制刷新UI，确保日志立即显示
							QApplication::processEvents();
						}

						FragmentationImpactPropertyWidget* m_fragmentationImpactPropertyWidget = gfParent->GetFragmentationImpactPropertyWidget();
						QTableWidget* m_fragmentationImpactTableWid = m_fragmentationImpactPropertyWidget->GetQTableWidget();

						StressResultWidget* m_stressResultWidget = gfParent->GetFragmentationImpactStressResultWidget();

						StrainResultWidget* m_strainResultWidget = gfParent->GetFragmentationImpactStrainResultWidget();

						TemperatureResultWidget* m_temperatureResultWidget = gfParent->GetFragmentationImpactTemperatureResultWidget();

						OverpressureResultWidget* m_overpressureResultWidge = gfParent->GetFragmentationImpactOverpressureResultWidget();

						QMap<QString, QVariant> data = convertTextData(m_projectPropertyWidge,
							m_geomPropertyWidget,
							m_materialPropertyWidget,
							m_databasePropertyWidget,
							m_stressResultWidget,
							m_strainResultWidget,
							m_temperatureResultWidget,
							m_overpressureResultWidge);

						// 跌落输入数据
						data.insert("测试项目", m_fragmentationImpactTableWid->item(1, 2)->text());
						data.insert("撞击速度", m_fragmentationImpactTableWid->item(2, 2)->text());
						data.insert("撞击角度", m_fragmentationImpactTableWid->item(3, 2)->text());
						data.insert("破片形状", m_fragmentationImpactTableWid->item(4, 2)->text());
						data.insert("破片直径", m_fragmentationImpactTableWid->item(5, 2)->text());
						data.insert("破片质量", m_fragmentationImpactTableWid->item(6, 2)->text());
						data.insert("破片硬度", m_fragmentationImpactTableWid->item(7, 2)->text());
						data.insert("温度传感器数量", m_fragmentationImpactTableWid->item(8, 2)->text());
						data.insert("超压传感器数量", m_fragmentationImpactTableWid->item(9, 2)->text());
						data.insert("风速", m_fragmentationImpactTableWid->item(10, 2)->text());


						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("应力云图", QDir(workdir + "/template/fragmentationImpact/Stress.png").absolutePath());
						imagePaths.insert("应变云图", QDir(workdir + "/template/fragmentationImpact/Strain.png").absolutePath());
						imagePaths.insert("温度云图", QDir(workdir + "/template/fragmentationImpact/Temperature.png").absolutePath());
						imagePaths.insert("超压云图", QDir(workdir + "/template/fragmentationImpact/Overpressure.png").absolutePath());
						QMap<QString, QVector<QVector<QVariant>>> tableData;

						// 创建进度对话框
						ProgressDialog* progressDialog = new ProgressDialog("导出破片撞击仿真计算报告进度", gfParent);
						progressDialog->show();

						// 创建工作线程和工作对象
						//WordExporterWorker* wordExporterWorker = new WordExporterWorker(QDir("src/template/破片撞击仿真计算数据表.docx").absolutePath(), directory + "/破片撞击仿真计算数据表.docx", data, imagePaths, tableData);
						WordExporterWorker* wordExporterWorker = new WordExporterWorker("破片撞击仿真计算数据表.docx", directory + "/破片撞击仿真计算数据表.docx", data, imagePaths, tableData);
						QThread* wordExporterThread = new QThread();
						wordExporterWorker->moveToThread(wordExporterThread);

						// 连接信号槽
						connect(wordExporterThread, &QThread::started, wordExporterWorker, &WordExporterWorker::DoWork);
						connect(wordExporterWorker, &WordExporterWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
						connect(wordExporterWorker, &WordExporterWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
						connect(progressDialog, &ProgressDialog::Canceled, wordExporterWorker, &WordExporterWorker::RequestInterruption, Qt::DirectConnection);

						// 处理导入结果
						connect(wordExporterWorker, &WordExporterWorker::WorkFinished, this,
							[=](bool success, const QString& msg)
							{
								if (success)
								{
									// 更新日志
									{
										QDateTime currentTime = QDateTime::currentDateTime();
										QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
										QString text = timeStr + "[信息]>成功导出破片撞击安全性分析报告";
										text = text + "\n" + timeStr + "[信息]>破片撞击安全性分析报告：" + directory + "/破片撞击仿真计算数据表.docx";
										textEdit->appendPlainText(text);
										logWidget->update();

										// 关键：强制刷新UI，确保日志立即显示
										QApplication::processEvents();
									}

								}
								else if (!success)
								{
									QMessageBox::warning(this, "导出失败", msg);
								}
								// 清理资源
								progressDialog->close();
								wordExporterThread->quit();
								wordExporterThread->wait();
								wordExporterWorker->deleteLater();
								wordExporterThread->deleteLater();
								progressDialog->deleteLater();
							});
						// 启动线程
						wordExporterThread->start();
					}
					else if (processedName == "爆炸冲击波安全性分析")
					{
						{
							QDateTime currentTime = QDateTime::currentDateTime();
							QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
							QString text = timeStr + "[信息]>开始导出爆炸冲击波安全性分析报告";
							textEdit->appendPlainText(text);
							logWidget->update();
							// 关键：强制刷新UI，确保日志立即显示
							QApplication::processEvents();
						}

						ExplosiveBlastPropertyWidget* m_explosiveBlastPropertyWidget = gfParent->GetExplosiveBlastPropertyWidget();
						QTableWidget* m_explosiveBlastTableWid = m_explosiveBlastPropertyWidget->GetQTableWidget();

						StressResultWidget* m_stressResultWidget = gfParent->GetExplosiveBlastStressResultWidget();
						QTableWidget* m_stressTableWid = m_stressResultWidget->GetQTableWidget();

						StrainResultWidget* m_strainResultWidget = gfParent->GetExplosiveBlastStrainResultWidget();
						QTableWidget* m_strainTableWid = m_strainResultWidget->GetQTableWidget();

						TemperatureResultWidget* m_temperatureResultWidget = gfParent->GetExplosiveBlastTemperatureResultWidget();
						QTableWidget* m_temperatureTableWid = m_temperatureResultWidget->GetQTableWidget();

						OverpressureResultWidget* m_overpressureResultWidge = gfParent->GetExplosiveBlastOverpressureResultWidget();
						QTableWidget* m_overpressureTableWid = m_overpressureResultWidge->GetQTableWidget();

						QMap<QString, QVariant> data = convertTextData(m_projectPropertyWidge,
							m_geomPropertyWidget,
							m_materialPropertyWidget,
							m_databasePropertyWidget,
							m_stressResultWidget,
							m_strainResultWidget,
							m_temperatureResultWidget,
							m_overpressureResultWidge);

						// 跌落输入数据
						data.insert("测试项目", m_explosiveBlastTableWid->item(1, 2)->text());
						data.insert("TNT当量", m_explosiveBlastTableWid->item(2, 2)->text());
						data.insert("入射角度", m_explosiveBlastTableWid->item(3, 2)->text());
						data.insert("温度传感器数量", m_explosiveBlastTableWid->item(4, 2)->text());
						data.insert("超压传感器数量", m_explosiveBlastTableWid->item(5, 2)->text());
						data.insert("风速", m_explosiveBlastTableWid->item(6, 2)->text());
						


						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("应力云图", QDir(workdir + "/template/explosiveBlast/Stress.png").absolutePath());
						imagePaths.insert("应变云图", QDir(workdir + "/template/explosiveBlast/Strain.png").absolutePath());
						imagePaths.insert("温度云图", QDir(workdir + "/template/explosiveBlast/Temperature.png").absolutePath());
						imagePaths.insert("超压云图", QDir(workdir + "/template/explosiveBlast/Overpressure.png").absolutePath());
						QMap<QString, QVector<QVector<QVariant>>> tableData;

						// 创建进度对话框
						ProgressDialog* progressDialog = new ProgressDialog("导出射流冲击仿真计算报告进度", gfParent);
						progressDialog->show();

						// 创建工作线程和工作对象
						//WordExporterWorker* wordExporterWorker = new WordExporterWorker(QDir("src/template/爆炸冲击波仿真计算数据表.docx").absolutePath(), directory + "/爆炸冲击波仿真计算数据表.docx", data, imagePaths, tableData);
						WordExporterWorker* wordExporterWorker = new WordExporterWorker("爆炸冲击波仿真计算数据表.docx", directory + "/爆炸冲击波仿真计算数据表.docx", data, imagePaths, tableData);
						QThread* wordExporterThread = new QThread();
						wordExporterWorker->moveToThread(wordExporterThread);

						// 连接信号槽
						connect(wordExporterThread, &QThread::started, wordExporterWorker, &WordExporterWorker::DoWork);
						connect(wordExporterWorker, &WordExporterWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
						connect(wordExporterWorker, &WordExporterWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
						connect(progressDialog, &ProgressDialog::Canceled, wordExporterWorker, &WordExporterWorker::RequestInterruption, Qt::DirectConnection);

						// 处理导入结果
						connect(wordExporterWorker, &WordExporterWorker::WorkFinished, this,
							[=](bool success, const QString& msg)
							{
								if (success)
								{
									// 更新日志
									{
										QDateTime currentTime = QDateTime::currentDateTime();
										QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
										QString text = timeStr + "[信息]>成功导出爆炸冲击波安全性分析报告";
										text = text + "\n" + timeStr + "[信息]>爆炸冲击波安全性分析报告：" + directory + "/爆炸冲击波仿真计算数据表.docx";
										textEdit->appendPlainText(text);
										logWidget->update();

										// 关键：强制刷新UI，确保日志立即显示
										QApplication::processEvents();
									}

								}
								else if (!success)
								{
									QMessageBox::warning(this, "导出失败", msg);
								}
								// 清理资源
								progressDialog->close();
								wordExporterThread->quit();
								wordExporterThread->wait();
								wordExporterWorker->deleteLater();
								wordExporterThread->deleteLater();
								progressDialog->deleteLater();
							});
						// 启动线程
						wordExporterThread->start();
					}
					else if (processedName == "殉爆安全性分析")
					{
						{
							QDateTime currentTime = QDateTime::currentDateTime();
							QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
							QString text = timeStr + "[信息]>开始导出殉爆安全性分析报告";
							textEdit->appendPlainText(text);
							logWidget->update();
							// 关键：强制刷新UI，确保日志立即显示
							QApplication::processEvents();
						}

						SacrificeExplosionPropertyWidget* m_sacrificeExplosionPropertyWidget = gfParent->GetSacrificeExplosionPropertyWidget();
						QTableWidget* m_sacrificeExplosionTableWid = m_sacrificeExplosionPropertyWidget->GetQTableWidget();

						StressResultWidget* m_stressResultWidget = gfParent->GetSacrificeExplosionStressResultWidget();
						QTableWidget* m_stressTableWid = m_stressResultWidget->GetQTableWidget();

						StrainResultWidget* m_strainResultWidget = gfParent->GetSacrificeExplosionStrainResultWidget();
						QTableWidget* m_strainTableWid = m_strainResultWidget->GetQTableWidget();

						TemperatureResultWidget* m_temperatureResultWidget = gfParent->GetSacrificeExplosionTemperatureResultWidget();
						QTableWidget* m_temperatureTableWid = m_temperatureResultWidget->GetQTableWidget();

						OverpressureResultWidget* m_overpressureResultWidge = gfParent->GetSacrificeExplosionOverpressureResultWidget();
						QTableWidget* m_overpressureTableWid = m_overpressureResultWidge->GetQTableWidget();

						QMap<QString, QVariant> data = convertTextData(m_projectPropertyWidge,
							m_geomPropertyWidget,
							m_materialPropertyWidget,
							m_databasePropertyWidget,
							m_stressResultWidget,
							m_strainResultWidget,
							m_temperatureResultWidget,
							m_overpressureResultWidge);

						// 跌落输入数据
						data.insert("测试项目", m_sacrificeExplosionTableWid->item(1, 2)->text());
						data.insert("殉爆距离", m_sacrificeExplosionTableWid->item(2, 2)->text());
						data.insert("模拟弹药数量", m_sacrificeExplosionTableWid->item(3, 2)->text());
						data.insert("被发弹数量", m_sacrificeExplosionTableWid->item(4, 2)->text());
						data.insert("温度传感器数量", m_sacrificeExplosionTableWid->item(5, 2)->text());
						data.insert("超压传感器数量", m_sacrificeExplosionTableWid->item(6, 2)->text());
						data.insert("风速", m_sacrificeExplosionTableWid->item(7, 2)->text());


						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("应力云图", QDir(workdir + "/template/sacrificeExplosio/Stress.png").absolutePath());
						imagePaths.insert("应变云图", QDir(workdir + "/template/sacrificeExplosio/Strain.png").absolutePath());
						imagePaths.insert("温度云图", QDir(workdir + "/template/sacrificeExplosio/Temperature.png").absolutePath());
						imagePaths.insert("超压云图", QDir(workdir + "/template/sacrificeExplosio/Overpressure.png").absolutePath());
						QMap<QString, QVector<QVector<QVariant>>> tableData;

						// 创建进度对话框
						ProgressDialog* progressDialog = new ProgressDialog("导出殉爆仿真计算报告进度", gfParent);
						progressDialog->show();

						// 创建工作线程和工作对象
						//WordExporterWorker* wordExporterWorker = new WordExporterWorker(QDir("src/template/殉爆仿真计算数据表.docx").absolutePath(), directory + "/殉爆仿真计算数据表.docx", data, imagePaths, tableData);
						WordExporterWorker* wordExporterWorker = new WordExporterWorker("殉爆仿真计算数据表.docx", directory + "/殉爆仿真计算数据表.docx", data, imagePaths, tableData);
						QThread* wordExporterThread = new QThread();
						wordExporterWorker->moveToThread(wordExporterThread);

						// 连接信号槽
						connect(wordExporterThread, &QThread::started, wordExporterWorker, &WordExporterWorker::DoWork);
						connect(wordExporterWorker, &WordExporterWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
						connect(wordExporterWorker, &WordExporterWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
						connect(progressDialog, &ProgressDialog::Canceled, wordExporterWorker, &WordExporterWorker::RequestInterruption, Qt::DirectConnection);

						// 处理导入结果
						connect(wordExporterWorker, &WordExporterWorker::WorkFinished, this,
							[=](bool success, const QString& msg)
							{
								if (success)
								{
									// 更新日志
									{
										QDateTime currentTime = QDateTime::currentDateTime();
										QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
										QString text = timeStr + "[信息]>成功导出殉爆安全性分析报告";
										text = text + "\n" + timeStr + "[信息]>殉爆安全性分析报告：" + directory + "/殉爆仿真计算数据表.docx";
										textEdit->appendPlainText(text);
										logWidget->update();

										// 关键：强制刷新UI，确保日志立即显示
										QApplication::processEvents();
									}

								}
								else if (!success)
								{
									QMessageBox::warning(this, "导出失败", msg);
								}
								// 清理资源
								progressDialog->close();
								wordExporterThread->quit();
								wordExporterThread->wait();
								wordExporterWorker->deleteLater();
								wordExporterThread->deleteLater();
								progressDialog->deleteLater();
							});
						// 启动线程
						wordExporterThread->start();
					}
				}
			}
			break;
		}
		else
		{
			parent = parent->parentWidget();
		}
	}
}




QMap<QString, QVariant> GFTreeModelWidget::convertTextData(ProjectPropertyWidge* projectPropertyWidge,
	GeomPropertyWidget* geomPropertyWidget,
	MaterialPropertyWidget* materialPropertyWidget,
	DatabasePropertyWidget* databasePropertyWidget,
	StressResultWidget* stressResultWidget,
	StrainResultWidget* strainResultWidget,
	TemperatureResultWidget* temperatureResultWidget,
	OverpressureResultWidget* overpressureResultWidge)
{
	QTableWidget* m_projectTableWid = projectPropertyWidge->GetQTableWidget();

	QTableWidget* m_geomTableWid = geomPropertyWidget->GetQTableWidget();

	QTableWidget* m_materialTableWid = materialPropertyWidget->GetQTableWidget();

	QTableWidget* m_databaseTableWid = databasePropertyWidget->GetQTableWidget();

	

	



	QMap<QString, QVariant> data;
	// 标题数据
	data.insert("工程名称", m_projectTableWid->item(1, 2)->text());
	data.insert("工程地点", m_projectTableWid->item(2, 2)->text());
	data.insert("测试设备", m_projectTableWid->item(4, 2)->text());
	data.insert("发动机型号", m_geomTableWid->item(1, 2)->text());
	data.insert("工程时间", m_projectTableWid->item(3, 2)->text());
	data.insert("测试标准", m_databaseTableWid->item(1, 2)->text());
	data.insert("壳体材料", m_materialTableWid->item(1, 2)->text());
	data.insert("含能材料", m_materialTableWid->item(2, 2)->text());
	data.insert("外防热材料", m_materialTableWid->item(3, 2)->text());
	data.insert("绝热层材料", m_materialTableWid->item(4, 2)->text());


	// 计算输出数据
	if (stressResultWidget != nullptr)
	{
		QTableWidget* m_stressTableWid = stressResultWidget->GetQTableWidget();
		data.insert("发动机壳体最大应力", m_stressTableWid->item(1, 2)->text());
		data.insert("发动机壳体最小应力", m_stressTableWid->item(2, 2)->text());
		data.insert("发动机壳体平均应力", m_stressTableWid->item(3, 2)->text());
		data.insert("发动机壳体应力标准差", m_stressTableWid->item(4, 2)->text());
		data.insert("固体推进剂最大应力", m_stressTableWid->item(5, 2)->text());
		data.insert("固体推进剂最小应力", m_stressTableWid->item(6, 2)->text());
		data.insert("固体推进剂平均应力", m_stressTableWid->item(7, 2)->text());
		data.insert("固体推进剂应力标准差", m_stressTableWid->item(8, 2)->text());
		data.insert("隔绝热最大应力", m_stressTableWid->item(9, 2)->text());
		data.insert("隔绝热最小应力", m_stressTableWid->item(10, 2)->text());
		data.insert("隔绝热平均应力", m_stressTableWid->item(11, 2)->text());
		data.insert("隔绝热应力标准差", m_stressTableWid->item(12, 2)->text());
		data.insert("外防热最大应力", m_stressTableWid->item(13, 2)->text());
		data.insert("外防热最小应力", m_stressTableWid->item(14, 2)->text());
		data.insert("外防热平均应力", m_stressTableWid->item(15, 2)->text());
		data.insert("外防热应力标准差", m_stressTableWid->item(16, 2)->text());
	}
	
	if (strainResultWidget != nullptr)
	{
		QTableWidget* m_strainTableWid = strainResultWidget->GetQTableWidget();
		data.insert("发动机壳体最大应变", m_strainTableWid->item(1, 2)->text());
		data.insert("发动机壳体最小应变", m_strainTableWid->item(2, 2)->text());
		data.insert("发动机壳体平均应变", m_strainTableWid->item(3, 2)->text());
		data.insert("发动机壳体应变标准差", m_strainTableWid->item(4, 2)->text());
		data.insert("固体推进剂最大应变", m_strainTableWid->item(5, 2)->text());
		data.insert("固体推进剂最小应变", m_strainTableWid->item(6, 2)->text());
		data.insert("固体推进剂平均应变", m_strainTableWid->item(7, 2)->text());
		data.insert("固体推进剂应变标准差", m_strainTableWid->item(8, 2)->text());
		data.insert("隔绝热最大应变", m_strainTableWid->item(9, 2)->text());
		data.insert("隔绝热最小应变", m_strainTableWid->item(10, 2)->text());
		data.insert("隔绝热平均应变", m_strainTableWid->item(11, 2)->text());
		data.insert("隔绝热应变标准差", m_strainTableWid->item(12, 2)->text());
		data.insert("外防热最大应变", m_strainTableWid->item(13, 2)->text());
		data.insert("外防热最小应变", m_strainTableWid->item(14, 2)->text());
		data.insert("外防热平均应变", m_strainTableWid->item(15, 2)->text());
		data.insert("外防热应变标准差", m_strainTableWid->item(16, 2)->text());
	}
	
	if (temperatureResultWidget != nullptr)
	{
		QTableWidget* m_temperatureTableWid = temperatureResultWidget->GetQTableWidget();
		data.insert("发动机壳体最高温度", m_temperatureTableWid->item(1, 2)->text());
		data.insert("发动机壳体最低温度", m_temperatureTableWid->item(2, 2)->text());
		data.insert("发动机壳体平均温度", m_temperatureTableWid->item(3, 2)->text());
		data.insert("发动机壳体温度标准差", m_temperatureTableWid->item(4, 2)->text());
		data.insert("固体推进剂最高温度", m_temperatureTableWid->item(5, 2)->text());
		data.insert("固体推进剂最低温度", m_temperatureTableWid->item(6, 2)->text());
		data.insert("固体推进剂平均温度", m_temperatureTableWid->item(7, 2)->text());
		data.insert("固体推进剂温度标准差", m_temperatureTableWid->item(8, 2)->text());
		data.insert("隔绝热最高温度", m_temperatureTableWid->item(9, 2)->text());
		data.insert("隔绝热最低温度", m_temperatureTableWid->item(10, 2)->text());
		data.insert("隔绝热平均温度", m_temperatureTableWid->item(11, 2)->text());
		data.insert("隔绝热温度标准差", m_temperatureTableWid->item(12, 2)->text());
		data.insert("外防热最高温度", m_temperatureTableWid->item(13, 2)->text());
		data.insert("外防热最低温度", m_temperatureTableWid->item(14, 2)->text());
		data.insert("外防热平均温度", m_temperatureTableWid->item(15, 2)->text());
		data.insert("外防热温度标准差", m_temperatureTableWid->item(16, 2)->text());
	}
	
	if (overpressureResultWidge != nullptr)
	{
		QTableWidget* m_overpressureTableWid = overpressureResultWidge->GetQTableWidget();
		data.insert("发动机壳体最大超压", m_overpressureTableWid->item(1, 2)->text());
		data.insert("发动机壳体最小超压", m_overpressureTableWid->item(2, 2)->text());
		data.insert("发动机壳体平均超压", m_overpressureTableWid->item(3, 2)->text());
		data.insert("发动机壳体超压标准差", m_overpressureTableWid->item(4, 2)->text());
		data.insert("固体推进剂最大超压", m_overpressureTableWid->item(5, 2)->text());
		data.insert("固体推进剂最小超压", m_overpressureTableWid->item(6, 2)->text());
		data.insert("固体推进剂平均超压", m_overpressureTableWid->item(7, 2)->text());
		data.insert("固体推进剂超压标准差", m_overpressureTableWid->item(8, 2)->text());
		data.insert("隔绝热最大超压", m_overpressureTableWid->item(9, 2)->text());
		data.insert("隔绝热最小超压", m_overpressureTableWid->item(10, 2)->text());
		data.insert("隔绝热平均超压", m_overpressureTableWid->item(11, 2)->text());
		data.insert("隔绝热超压标准差", m_overpressureTableWid->item(12, 2)->text());
		data.insert("外防热最大超压", m_overpressureTableWid->item(13, 2)->text());
		data.insert("外防热最小超压", m_overpressureTableWid->item(14, 2)->text());
		data.insert("外防热平均超压", m_overpressureTableWid->item(15, 2)->text());
		data.insert("外防热超压标准差", m_overpressureTableWid->item(16, 2)->text());
	}
	

	return data;
}