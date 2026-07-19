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
#include "mainWidget.h"


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
	geometryNode->setText(0, "固体发动机三维模型");
	geometryNode->setData(0, Qt::UserRole, "Geometry");
	geometryNode->setIcon(0, error_icon);
	{
		QTreeWidgetItem* nozzleShape = new QTreeWidgetItem();
		nozzleShape->setText(0, "喷管");
		nozzleShape->setData(0, Qt::UserRole, "NozzleGeometry");
		nozzleShape->setIcon(0, error_icon);

		QTreeWidgetItem* shellShape = new QTreeWidgetItem();
		shellShape->setText(0, "壳体");
		shellShape->setData(0, Qt::UserRole, "ShellGeometry");
		shellShape->setIcon(0, error_icon);

		QTreeWidgetItem* propellantShape = new QTreeWidgetItem();
		propellantShape->setText(0, "推进剂");
		propellantShape->setData(0, Qt::UserRole, "PropellantGeometry");
		propellantShape->setIcon(0, error_icon);

		QTreeWidgetItem* heatInsulatingLayerShape = new QTreeWidgetItem();
		heatInsulatingLayerShape->setText(0, "绝热层");
		heatInsulatingLayerShape->setData(0, Qt::UserRole, "HeatInsulatingLayerGeometry");
		heatInsulatingLayerShape->setIcon(0, error_icon);

		geometryNode->addChild(nozzleShape);
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
		QTreeWidgetItem* nozzleMesh = new QTreeWidgetItem();
		nozzleMesh->setText(0, "喷管");
		nozzleMesh->setData(0, Qt::UserRole, "NozzleMesh");
		nozzleMesh->setIcon(0, error_icon);

		QTreeWidgetItem* shellMesh = new QTreeWidgetItem();
		shellMesh->setText(0, "壳体");
		shellMesh->setData(0, Qt::UserRole, "ShellMesh");
		shellMesh->setIcon(0, error_icon);

		QTreeWidgetItem* propellantMesh = new QTreeWidgetItem();
		propellantMesh->setText(0, "推进剂");
		propellantMesh->setData(0, Qt::UserRole, "PropellantMesh");
		propellantMesh->setIcon(0, error_icon);

		QTreeWidgetItem* heatInsulatingLayerMesh = new QTreeWidgetItem();
		heatInsulatingLayerMesh->setText(0, "绝热层");
		heatInsulatingLayerMesh->setData(0, Qt::UserRole, "HeatInsulatingLayerMesh");
		heatInsulatingLayerMesh->setIcon(0, error_icon);

		meshItem->addChild(nozzleMesh);
		meshItem->addChild(shellMesh);
		meshItem->addChild(propellantMesh);
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
		stressShellResult->setText(0, "喷管+壳体");
		stressShellResult->setData(0, Qt::UserRole, "FallStressShellResult");
		stressShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* stressPropellantResult = new QTreeWidgetItem();
		stressPropellantResult->setText(0, "推进剂+绝热层");
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
		strainShellResult->setText(0, "喷管+壳体");
		strainShellResult->setData(0, Qt::UserRole, "FallStrainShellResult");
		strainShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* strainPropellantResult = new QTreeWidgetItem();
		strainPropellantResult->setText(0, "推进剂+绝热层");
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
		tempShellResult->setText(0, "喷管+壳体");
		tempShellResult->setData(0, Qt::UserRole, "FallTemperatureShellResult");
		tempShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* tempPropellantResult = new QTreeWidgetItem();
		tempPropellantResult->setText(0, "推进剂+绝热层");
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
		overpressureShellResult->setText(0, "喷管+壳体");
		overpressureShellResult->setData(0, Qt::UserRole, "FallOverpressureShellResult");
		overpressureShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* overpressurePropellantResult = new QTreeWidgetItem();
		overpressurePropellantResult->setText(0, "推进剂+绝热层");
		overpressurePropellantResult->setData(0, Qt::UserRole, "FallOverpressurePropellantResult");
		overpressurePropellantResult->setIcon(0, error_icon);

		overpressureResult->addChild(overpressureShellResult);
		overpressureResult->addChild(overpressurePropellantResult);
	}

	
	QTreeWidgetItem* reactionDegreeResult = new QTreeWidgetItem();
	reactionDegreeResult->setText(0, "反应度");
	reactionDegreeResult->setData(0, Qt::UserRole, "ReactionDegreeResult");
	reactionDegreeResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* reactionDegreePropellantResult = new QTreeWidgetItem();
		reactionDegreePropellantResult->setText(0, "推进剂+绝热层");
		reactionDegreePropellantResult->setData(0, Qt::UserRole, "reactionDegreePropellantResult");
		reactionDegreePropellantResult->setIcon(0, error_icon);

		reactionDegreeResult->addChild(reactionDegreePropellantResult);
	}

	fallAnalysis->addChild(stressResult);
	fallAnalysis->addChild(strainResult);
	fallAnalysis->addChild(temperatureResult);
	fallAnalysis->addChild(overpressureResult);
	fallAnalysis->addChild(reactionDegreeResult);

	QTreeWidgetItem* fastCombustionAnalysis = new QTreeWidgetItem();
	fastCombustionAnalysis->setText(0, "2.快速烤燃安全性分析");
	fastCombustionAnalysis->setData(0, Qt::UserRole, "FastCombustionAnalysis");
	//fastCombustionAnalysis->setCheckState(0, Qt::Unchecked);
	fastCombustionAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* fastCombustionTemperatureResult = new QTreeWidgetItem();
	fastCombustionTemperatureResult->setText(0, "温度分析");
	fastCombustionTemperatureResult->setData(0, Qt::UserRole, "FastCombustionTemperatureResult");
	fastCombustionTemperatureResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* fastTemperatureShellResult = new QTreeWidgetItem();
		fastTemperatureShellResult->setText(0, "喷管+壳体");
		fastTemperatureShellResult->setData(0, Qt::UserRole, "fastTemperatureShellResult");
		fastTemperatureShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* fastTemperaturePropellantResult = new QTreeWidgetItem();
		fastTemperaturePropellantResult->setText(0, "推进剂+绝热层");
		fastTemperaturePropellantResult->setData(0, Qt::UserRole, "fastTemperaturePropellantResult");
		fastTemperaturePropellantResult->setIcon(0, error_icon);

		fastCombustionTemperatureResult->addChild(fastTemperatureShellResult);
		fastCombustionTemperatureResult->addChild(fastTemperaturePropellantResult);
	}
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
	{
		QTreeWidgetItem* slowTemperatureShellResult = new QTreeWidgetItem();
		slowTemperatureShellResult->setText(0, "喷管+壳体");
		slowTemperatureShellResult->setData(0, Qt::UserRole, "slowTemperatureShellResult");
		slowTemperatureShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* slowTemperaturePropellantResult = new QTreeWidgetItem();
		slowTemperaturePropellantResult->setText(0, "推进剂+绝热层");
		slowTemperaturePropellantResult->setData(0, Qt::UserRole, "slowTemperaturePropellantResult");
		slowTemperaturePropellantResult->setIcon(0, error_icon);

		slowCombustionTemperatureResult->addChild(slowTemperatureShellResult);
		slowCombustionTemperatureResult->addChild(slowTemperaturePropellantResult);
	}
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
	{
		QTreeWidgetItem* shootStressShellResult = new QTreeWidgetItem();
		shootStressShellResult->setText(0, "喷管+壳体");
		shootStressShellResult->setData(0, Qt::UserRole, "shootStressShellResult");
		shootStressShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* shootStressPropellantResult = new QTreeWidgetItem();
		shootStressPropellantResult->setText(0, "推进剂+绝热层");
		shootStressPropellantResult->setData(0, Qt::UserRole, "shootStressPropellantResult");
		shootStressPropellantResult->setIcon(0, error_icon);

		shootStressResult->addChild(shootStressShellResult);
		shootStressResult->addChild(shootStressPropellantResult);
	}

	QTreeWidgetItem* shootStrainResult = new QTreeWidgetItem();
	shootStrainResult->setText(0, "应变分析");
	shootStrainResult->setData(0, Qt::UserRole, "ShootStrainResult");
	shootStrainResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* shootStrainShellResult = new QTreeWidgetItem();
		shootStrainShellResult->setText(0, "喷管+壳体");
		shootStrainShellResult->setData(0, Qt::UserRole, "shootStrainShellResult");
		shootStrainShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* shootStrainPropellantResult = new QTreeWidgetItem();
		shootStrainPropellantResult->setText(0, "推进剂+绝热层");
		shootStrainPropellantResult->setData(0, Qt::UserRole, "shootStrainPropellantResult");
		shootStrainPropellantResult->setIcon(0, error_icon);

		shootStrainResult->addChild(shootStrainShellResult);
		shootStrainResult->addChild(shootStrainPropellantResult);
	}

	QTreeWidgetItem* shootTemperatureResult = new QTreeWidgetItem();
	shootTemperatureResult->setText(0, "温度分析");
	shootTemperatureResult->setData(0, Qt::UserRole, "ShootTemperatureResult");
	shootTemperatureResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* shootTempShellResult = new QTreeWidgetItem();
		shootTempShellResult->setText(0, "喷管+壳体");
		shootTempShellResult->setData(0, Qt::UserRole, "shootTempShellResult");
		shootTempShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* shootTempPropellantResult = new QTreeWidgetItem();
		shootTempPropellantResult->setText(0, "推进剂+绝热层");
		shootTempPropellantResult->setData(0, Qt::UserRole, "shootTempPropellantResult");
		shootTempPropellantResult->setIcon(0, error_icon);

		shootTemperatureResult->addChild(shootTempShellResult);
		shootTemperatureResult->addChild(shootTempPropellantResult);
	}

	QTreeWidgetItem* shootOverpressureResult = new QTreeWidgetItem();
	shootOverpressureResult->setText(0, "超压分析");
	shootOverpressureResult->setData(0, Qt::UserRole, "ShootOverpressureResult");
	shootOverpressureResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* shootOverpressureShellResult = new QTreeWidgetItem();
		shootOverpressureShellResult->setText(0, "喷管+壳体");
		shootOverpressureShellResult->setData(0, Qt::UserRole, "shootOverpressureShellResult");
		shootOverpressureShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* shootOverpressurePropellantResult = new QTreeWidgetItem();
		shootOverpressurePropellantResult->setText(0, "推进剂+绝热层");
		shootOverpressurePropellantResult->setData(0, Qt::UserRole, "shootOverpressurePropellantResult");
		shootOverpressurePropellantResult->setIcon(0, error_icon);

		shootOverpressureResult->addChild(shootOverpressureShellResult);
		shootOverpressureResult->addChild(shootOverpressurePropellantResult);
	}

	QTreeWidgetItem* shootReactionDegreeResult = new QTreeWidgetItem();
	shootReactionDegreeResult->setText(0, "反应度");
	shootReactionDegreeResult->setData(0, Qt::UserRole, "ShootReactionDegreeResult");
	shootReactionDegreeResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* shootReactionDegreePropellantResult = new QTreeWidgetItem();
		shootReactionDegreePropellantResult->setText(0, "推进剂+绝热层");
		shootReactionDegreePropellantResult->setData(0, Qt::UserRole, "shootReactionDegreePropellantResult");
		shootReactionDegreePropellantResult->setIcon(0, error_icon);

		shootReactionDegreeResult->addChild(shootReactionDegreePropellantResult);
	}


	shootAnalysis->addChild(shootStressResult);
	shootAnalysis->addChild(shootStrainResult);
	shootAnalysis->addChild(shootTemperatureResult);
	shootAnalysis->addChild(shootOverpressureResult);
	shootAnalysis->addChild(shootReactionDegreeResult);

	QTreeWidgetItem* jetImpactAnalysis = new QTreeWidgetItem();
	jetImpactAnalysis->setText(0, "5.射流冲击安全性分析");
	jetImpactAnalysis->setData(0, Qt::UserRole, "JetImpactAnalysis");
	//jetImpactAnalysis->setCheckState(0, Qt::Unchecked);
	jetImpactAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* jetImpactStressResult = new QTreeWidgetItem();
	jetImpactStressResult->setText(0, "应力分析");
	jetImpactStressResult->setData(0, Qt::UserRole, "JetImpactStressResult");
	jetImpactStressResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* jetStressShellResult = new QTreeWidgetItem();
		jetStressShellResult->setText(0, "喷管+壳体");
		jetStressShellResult->setData(0, Qt::UserRole, "jetStressShellResult");
		jetStressShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* jetStressPropellantResult = new QTreeWidgetItem();
		jetStressPropellantResult->setText(0, "推进剂+绝热层");
		jetStressPropellantResult->setData(0, Qt::UserRole, "jetStressPropellantResult");
		jetStressPropellantResult->setIcon(0, error_icon);

		jetImpactStressResult->addChild(jetStressShellResult);
		jetImpactStressResult->addChild(jetStressPropellantResult);
	}


	QTreeWidgetItem* jetStrainResult = new QTreeWidgetItem();
	jetStrainResult->setText(0, "应变分析");
	jetStrainResult->setData(0, Qt::UserRole, "JetImpactStrainResult");
	jetStrainResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* jetStrainShellResult = new QTreeWidgetItem();
		jetStrainShellResult->setText(0, "喷管+壳体");
		jetStrainShellResult->setData(0, Qt::UserRole, "jetStrainShellResult");
		jetStrainShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* jetStrainPropellantResult = new QTreeWidgetItem();
		jetStrainPropellantResult->setText(0, "推进剂+绝热层");
		jetStrainPropellantResult->setData(0, Qt::UserRole, "jetStrainPropellantResult");
		jetStrainPropellantResult->setIcon(0, error_icon);

		jetStrainResult->addChild(jetStrainShellResult);
		jetStrainResult->addChild(jetStrainPropellantResult);
	}

	QTreeWidgetItem* jetImpactTemperatureResult = new QTreeWidgetItem();
	jetImpactTemperatureResult->setText(0, "温度分析");
	jetImpactTemperatureResult->setData(0, Qt::UserRole, "JetImpactTemperatureResult");
	jetImpactTemperatureResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* jetTempShellResult = new QTreeWidgetItem();
		jetTempShellResult->setText(0, "喷管+壳体");
		jetTempShellResult->setData(0, Qt::UserRole, "jetTempShellResult");
		jetTempShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* jetTempPropellantResult = new QTreeWidgetItem();
		jetTempPropellantResult->setText(0, "推进剂+绝热层");
		jetTempPropellantResult->setData(0, Qt::UserRole, "jetTempPropellantResult");
		jetTempPropellantResult->setIcon(0, error_icon);

		jetImpactTemperatureResult->addChild(jetTempShellResult);
		jetImpactTemperatureResult->addChild(jetTempPropellantResult);
	}

	QTreeWidgetItem* jetImpactOverpressureResult = new QTreeWidgetItem();
	jetImpactOverpressureResult->setText(0, "超压分析");
	jetImpactOverpressureResult->setData(0, Qt::UserRole, "JetImpactOverpressureResult");
	jetImpactOverpressureResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* jetOverpressureShellResult = new QTreeWidgetItem();
		jetOverpressureShellResult->setText(0, "喷管+壳体");
		jetOverpressureShellResult->setData(0, Qt::UserRole, "jetOverpressureShellResult");
		jetOverpressureShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* jetOverpressurePropellantResult = new QTreeWidgetItem();
		jetOverpressurePropellantResult->setText(0, "推进剂+绝热层");
		jetOverpressurePropellantResult->setData(0, Qt::UserRole, "jetOverpressurePropellantResult");
		jetOverpressurePropellantResult->setIcon(0, error_icon);

		jetImpactOverpressureResult->addChild(jetOverpressureShellResult);
		jetImpactOverpressureResult->addChild(jetOverpressurePropellantResult);
	}

	QTreeWidgetItem* jetImpactReactionDegreeResult = new QTreeWidgetItem();
	jetImpactReactionDegreeResult->setText(0, "反应度");
	jetImpactReactionDegreeResult->setData(0, Qt::UserRole, "jetImpactReactionDegreeResult");
	jetImpactReactionDegreeResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* jetImpactReactionDegreePropellantResult = new QTreeWidgetItem();
		jetImpactReactionDegreePropellantResult->setText(0, "推进剂+绝热层");
		jetImpactReactionDegreePropellantResult->setData(0, Qt::UserRole, "jetImpactReactionDegreePropellantResult");
		jetImpactReactionDegreePropellantResult->setIcon(0, error_icon);

		jetImpactReactionDegreeResult->addChild(jetImpactReactionDegreePropellantResult);
	}

	jetImpactAnalysis->addChild(jetImpactStressResult);
	jetImpactAnalysis->addChild(jetStrainResult);
	jetImpactAnalysis->addChild(jetImpactTemperatureResult);
	jetImpactAnalysis->addChild(jetImpactOverpressureResult);
	jetImpactAnalysis->addChild(jetImpactReactionDegreeResult);

	QTreeWidgetItem* fragmentationImpactAnalysis = new QTreeWidgetItem();
	fragmentationImpactAnalysis->setText(0, "6.破片撞击安全性分析");
	fragmentationImpactAnalysis->setData(0, Qt::UserRole, "FragmentationImpactAnalysis");
	//fragmentationImpactAnalysis->setCheckState(0, Qt::Unchecked);
	fragmentationImpactAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* fragmentationImpactStressResult = new QTreeWidgetItem();
	fragmentationImpactStressResult->setText(0, "应力分析");
	fragmentationImpactStressResult->setData(0, Qt::UserRole, "FragmentationImpactStressResult");
	fragmentationImpactStressResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* fragmentationStressShellResult = new QTreeWidgetItem();
		fragmentationStressShellResult->setText(0, "喷管+壳体");
		fragmentationStressShellResult->setData(0, Qt::UserRole, "fragmentationStressShellResult");
		fragmentationStressShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* fragmentationStressPropellantResult = new QTreeWidgetItem();
		fragmentationStressPropellantResult->setText(0, "推进剂+绝热层");
		fragmentationStressPropellantResult->setData(0, Qt::UserRole, "fragmentationStressPropellantResult");
		fragmentationStressPropellantResult->setIcon(0, error_icon);

		fragmentationImpactStressResult->addChild(fragmentationStressShellResult);
		fragmentationImpactStressResult->addChild(fragmentationStressPropellantResult);
	}

	QTreeWidgetItem* fragmentationStrainResult = new QTreeWidgetItem();
	fragmentationStrainResult->setText(0, "应变分析");
	fragmentationStrainResult->setData(0, Qt::UserRole, "FragmentationImpactStrainResult");
	fragmentationStrainResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* fragmentationStrainShellResult = new QTreeWidgetItem();
		fragmentationStrainShellResult->setText(0, "喷管+壳体");
		fragmentationStrainShellResult->setData(0, Qt::UserRole, "fragmentationStrainShellResult");
		fragmentationStrainShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* fragmentationStrainPropellantResult = new QTreeWidgetItem();
		fragmentationStrainPropellantResult->setText(0, "推进剂+绝热层");
		fragmentationStrainPropellantResult->setData(0, Qt::UserRole, "fragmentationStrainPropellantResult");
		fragmentationStrainPropellantResult->setIcon(0, error_icon);

		fragmentationStrainResult->addChild(fragmentationStrainShellResult);
		fragmentationStrainResult->addChild(fragmentationStrainPropellantResult);
	}

	QTreeWidgetItem* fragmentationImpactTemperatureResult = new QTreeWidgetItem();
	fragmentationImpactTemperatureResult->setText(0, "温度分析");
	fragmentationImpactTemperatureResult->setData(0, Qt::UserRole, "FragmentationImpactTemperatureResult");
	fragmentationImpactTemperatureResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* fragmentationTempShellResult = new QTreeWidgetItem();
		fragmentationTempShellResult->setText(0, "喷管+壳体");
		fragmentationTempShellResult->setData(0, Qt::UserRole, "fragmentationTempShellResult");
		fragmentationTempShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* fragmentationTempPropellantResult = new QTreeWidgetItem();
		fragmentationTempPropellantResult->setText(0, "推进剂+绝热层");
		fragmentationTempPropellantResult->setData(0, Qt::UserRole, "fragmentationTempPropellantResult");
		fragmentationTempPropellantResult->setIcon(0, error_icon);

		fragmentationImpactTemperatureResult->addChild(fragmentationTempShellResult);
		fragmentationImpactTemperatureResult->addChild(fragmentationTempPropellantResult);
	}

	QTreeWidgetItem* fragmentationImpactOverpressureResult = new QTreeWidgetItem();
	fragmentationImpactOverpressureResult->setText(0, "超压分析");
	fragmentationImpactOverpressureResult->setData(0, Qt::UserRole, "FragmentationImpactOverpressureResult");
	fragmentationImpactOverpressureResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* fragmentationOverpressureShellResult = new QTreeWidgetItem();
		fragmentationOverpressureShellResult->setText(0, "喷管+壳体");
		fragmentationOverpressureShellResult->setData(0, Qt::UserRole, "fragmentationOverpressureShellResult");
		fragmentationOverpressureShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* fragmentationOverpressurePropellantResult = new QTreeWidgetItem();
		fragmentationOverpressurePropellantResult->setText(0, "推进剂+绝热层");
		fragmentationOverpressurePropellantResult->setData(0, Qt::UserRole, "fragmentationOverpressurePropellantResult");
		fragmentationOverpressurePropellantResult->setIcon(0, error_icon);

		fragmentationImpactOverpressureResult->addChild(fragmentationOverpressureShellResult);
		fragmentationImpactOverpressureResult->addChild(fragmentationOverpressurePropellantResult);
	}

	QTreeWidgetItem* fragmentationImpactReactionDegreeResult = new QTreeWidgetItem();
	fragmentationImpactReactionDegreeResult->setText(0, "反应度");
	fragmentationImpactReactionDegreeResult->setData(0, Qt::UserRole, "fragmentationImpactReactionDegreeResult");
	fragmentationImpactReactionDegreeResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* fragmentationImpactReactionDegreePropellantResult = new QTreeWidgetItem();
		fragmentationImpactReactionDegreePropellantResult->setText(0, "推进剂+绝热层");
		fragmentationImpactReactionDegreePropellantResult->setData(0, Qt::UserRole, "fragmentationImpactReactionDegreePropellantResult");
		fragmentationImpactReactionDegreePropellantResult->setIcon(0, error_icon);

		fragmentationImpactReactionDegreeResult->addChild(fragmentationImpactReactionDegreePropellantResult);
	}

	fragmentationImpactAnalysis->addChild(fragmentationImpactStressResult);
	fragmentationImpactAnalysis->addChild(fragmentationStrainResult);
	fragmentationImpactAnalysis->addChild(fragmentationImpactTemperatureResult);
	fragmentationImpactAnalysis->addChild(fragmentationImpactOverpressureResult);
	fragmentationImpactAnalysis->addChild(fragmentationImpactReactionDegreeResult);

	QTreeWidgetItem* explosiveBlastAnalysis = new QTreeWidgetItem();
	explosiveBlastAnalysis->setText(0, "7.爆炸冲击波安全性分析");
	explosiveBlastAnalysis->setData(0, Qt::UserRole, "ExplosiveBlastAnalysis");
	//explosiveBlastAnalysis->setCheckState(0, Qt::Unchecked);
	explosiveBlastAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* explosiveBlastStressResult = new QTreeWidgetItem();
	explosiveBlastStressResult->setText(0, "应力分析");
	explosiveBlastStressResult->setData(0, Qt::UserRole, "ExplosiveBlastStressResult");
	explosiveBlastStressResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* explosiveStressShellResult = new QTreeWidgetItem();
		explosiveStressShellResult->setText(0, "喷管+壳体");
		explosiveStressShellResult->setData(0, Qt::UserRole, "explosiveStressShellResult");
		explosiveStressShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* explosiveStressPropellantResult = new QTreeWidgetItem();
		explosiveStressPropellantResult->setText(0, "推进剂+绝热层");
		explosiveStressPropellantResult->setData(0, Qt::UserRole, "explosiveStressPropellantResult");
		explosiveStressPropellantResult->setIcon(0, error_icon);

		explosiveBlastStressResult->addChild(explosiveStressShellResult);
		explosiveBlastStressResult->addChild(explosiveStressPropellantResult);
	}


	QTreeWidgetItem* explosiveBlastStrainResult = new QTreeWidgetItem();
	explosiveBlastStrainResult->setText(0, "应变分析");
	explosiveBlastStrainResult->setData(0, Qt::UserRole, "ExplosiveBlastStrainResult");
	explosiveBlastStrainResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* explosiveStrainShellResult = new QTreeWidgetItem();
		explosiveStrainShellResult->setText(0, "喷管+壳体");
		explosiveStrainShellResult->setData(0, Qt::UserRole, "explosiveStrainShellResult");
		explosiveStrainShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* explosiveStrainPropellantResult = new QTreeWidgetItem();
		explosiveStrainPropellantResult->setText(0, "推进剂+绝热层");
		explosiveStrainPropellantResult->setData(0, Qt::UserRole, "explosiveStrainPropellantResult");
		explosiveStrainPropellantResult->setIcon(0, error_icon);

		explosiveBlastStrainResult->addChild(explosiveStrainShellResult);
		explosiveBlastStrainResult->addChild(explosiveStrainPropellantResult);
	}

	QTreeWidgetItem* explosiveBlastTemperatureResult = new QTreeWidgetItem();
	explosiveBlastTemperatureResult->setText(0, "温度分析");
	explosiveBlastTemperatureResult->setData(0, Qt::UserRole, "ExplosiveBlastTemperatureResult");
	explosiveBlastTemperatureResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* explosiveTempShellResult = new QTreeWidgetItem();
		explosiveTempShellResult->setText(0, "喷管+壳体");
		explosiveTempShellResult->setData(0, Qt::UserRole, "explosiveTempShellResult");
		explosiveTempShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* explosiveTempPropellantResult = new QTreeWidgetItem();
		explosiveTempPropellantResult->setText(0, "推进剂+绝热层");
		explosiveTempPropellantResult->setData(0, Qt::UserRole, "explosiveTempPropellantResult");
		explosiveTempPropellantResult->setIcon(0, error_icon);

		explosiveBlastTemperatureResult->addChild(explosiveTempShellResult);
		explosiveBlastTemperatureResult->addChild(explosiveTempPropellantResult);
	}

	QTreeWidgetItem* explosiveBlastOverpressureResult = new QTreeWidgetItem();
	explosiveBlastOverpressureResult->setText(0, "超压分析");
	explosiveBlastOverpressureResult->setData(0, Qt::UserRole, "ExplosiveBlastOverpressureResult");
	explosiveBlastOverpressureResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* explosiveOverpressureShellResult = new QTreeWidgetItem();
		explosiveOverpressureShellResult->setText(0, "喷管+壳体");
		explosiveOverpressureShellResult->setData(0, Qt::UserRole, "explosiveOverpressureShellResult");
		explosiveOverpressureShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* explosiveOverpressurePropellantResult = new QTreeWidgetItem();
		explosiveOverpressurePropellantResult->setText(0, "推进剂+绝热层");
		explosiveOverpressurePropellantResult->setData(0, Qt::UserRole, "explosiveOverpressurePropellantResult");
		explosiveOverpressurePropellantResult->setIcon(0, error_icon);

		explosiveBlastOverpressureResult->addChild(explosiveOverpressureShellResult);
		explosiveBlastOverpressureResult->addChild(explosiveOverpressurePropellantResult);
	}

	QTreeWidgetItem* explosiveBlastReactionDegreeResult = new QTreeWidgetItem();
	explosiveBlastReactionDegreeResult->setText(0, "反应度");
	explosiveBlastReactionDegreeResult->setData(0, Qt::UserRole, "explosiveBlastReactionDegreeResult");
	explosiveBlastReactionDegreeResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* explosiveBlastReactionDegreePropellantResult = new QTreeWidgetItem();
		explosiveBlastReactionDegreePropellantResult->setText(0, "推进剂+绝热层");
		explosiveBlastReactionDegreePropellantResult->setData(0, Qt::UserRole, "explosiveBlastReactionDegreePropellantResult");
		explosiveBlastReactionDegreePropellantResult->setIcon(0, error_icon);

		explosiveBlastReactionDegreeResult->addChild(explosiveBlastReactionDegreePropellantResult);
	}
	explosiveBlastAnalysis->addChild(explosiveBlastStressResult);
	explosiveBlastAnalysis->addChild(explosiveBlastStrainResult);
	explosiveBlastAnalysis->addChild(explosiveBlastTemperatureResult);
	explosiveBlastAnalysis->addChild(explosiveBlastOverpressureResult);
	explosiveBlastAnalysis->addChild(explosiveBlastReactionDegreeResult);

	QTreeWidgetItem* sacrificeExplosionAnalysis = new QTreeWidgetItem();
	sacrificeExplosionAnalysis->setText(0, "8.殉爆安全性分析");
	sacrificeExplosionAnalysis->setData(0, Qt::UserRole, "SacrificeExplosionAnalysis");
	//sacrificeExplosionAnalysis->setCheckState(0, Qt::Unchecked);
	sacrificeExplosionAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* sacrificeExplosionStressResult = new QTreeWidgetItem();
	sacrificeExplosionStressResult->setText(0, "应力分析");
	sacrificeExplosionStressResult->setData(0, Qt::UserRole, "SacrificeExplosioStressResult");
	sacrificeExplosionStressResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* sacrificeStressShellResult = new QTreeWidgetItem();
		sacrificeStressShellResult->setText(0, "喷管+壳体");
		sacrificeStressShellResult->setData(0, Qt::UserRole, "sacrificeStressShellResult");
		sacrificeStressShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* sacrificeStressPropellantResult = new QTreeWidgetItem();
		sacrificeStressPropellantResult->setText(0, "推进剂+绝热层");
		sacrificeStressPropellantResult->setData(0, Qt::UserRole, "sacrificeStressPropellantResult");
		sacrificeStressPropellantResult->setIcon(0, error_icon);

		sacrificeExplosionStressResult->addChild(sacrificeStressShellResult);
		sacrificeExplosionStressResult->addChild(sacrificeStressPropellantResult);
	}


	QTreeWidgetItem* sacrificeExplosionStrainResult = new QTreeWidgetItem();
	sacrificeExplosionStrainResult->setText(0, "应变分析");
	sacrificeExplosionStrainResult->setData(0, Qt::UserRole, "SacrificeExplosioStrainResult");
	sacrificeExplosionStrainResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* sacrificeStrainShellResult = new QTreeWidgetItem();
		sacrificeStrainShellResult->setText(0, "喷管+壳体");
		sacrificeStrainShellResult->setData(0, Qt::UserRole, "sacrificeStrainShellResult");
		sacrificeStrainShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* sacrificeStrainPropellantResult = new QTreeWidgetItem();
		sacrificeStrainPropellantResult->setText(0, "推进剂+绝热层");
		sacrificeStrainPropellantResult->setData(0, Qt::UserRole, "sacrificeStrainPropellantResult");
		sacrificeStrainPropellantResult->setIcon(0, error_icon);

		sacrificeExplosionStrainResult->addChild(sacrificeStrainShellResult);
		sacrificeExplosionStrainResult->addChild(sacrificeStrainPropellantResult);
	}

	QTreeWidgetItem* sacrificeExplosionTemperatureResult = new QTreeWidgetItem();
	sacrificeExplosionTemperatureResult->setText(0, "温度分析");
	sacrificeExplosionTemperatureResult->setData(0, Qt::UserRole, "SacrificeExplosioTemperatureResult");
	sacrificeExplosionTemperatureResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* sacrificeTempShellResult = new QTreeWidgetItem();
		sacrificeTempShellResult->setText(0, "喷管+壳体");
		sacrificeTempShellResult->setData(0, Qt::UserRole, "sacrificeTempShellResult");
		sacrificeTempShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* sacrificeTempPropellantResult = new QTreeWidgetItem();
		sacrificeTempPropellantResult->setText(0, "推进剂+绝热层");
		sacrificeTempPropellantResult->setData(0, Qt::UserRole, "sacrificeTempPropellantResult");
		sacrificeTempPropellantResult->setIcon(0, error_icon);

		sacrificeExplosionTemperatureResult->addChild(sacrificeTempShellResult);
		sacrificeExplosionTemperatureResult->addChild(sacrificeTempPropellantResult);
	}

	QTreeWidgetItem* sacrificeExplosionOverpressureResult = new QTreeWidgetItem();
	sacrificeExplosionOverpressureResult->setText(0, "超压分析");
	sacrificeExplosionOverpressureResult->setData(0, Qt::UserRole, "SacrificeExplosioOverpressureResult");
	sacrificeExplosionOverpressureResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* sacrificeOverpressureShellResult = new QTreeWidgetItem();
		sacrificeOverpressureShellResult->setText(0, "喷管+壳体");
		sacrificeOverpressureShellResult->setData(0, Qt::UserRole, "sacrificeOverpressureShellResult");
		sacrificeOverpressureShellResult->setIcon(0, error_icon);

		QTreeWidgetItem* sacrificeOverpressurePropellantResult = new QTreeWidgetItem();
		sacrificeOverpressurePropellantResult->setText(0, "推进剂+绝热层");
		sacrificeOverpressurePropellantResult->setData(0, Qt::UserRole, "sacrificeOverpressurePropellantResult");
		sacrificeOverpressurePropellantResult->setIcon(0, error_icon);

		sacrificeExplosionOverpressureResult->addChild(sacrificeOverpressureShellResult);
		sacrificeExplosionOverpressureResult->addChild(sacrificeOverpressurePropellantResult);
	}

	QTreeWidgetItem* sacrificeExplosionReactionDegreeResult = new QTreeWidgetItem();
	sacrificeExplosionReactionDegreeResult->setText(0, "反应度");
	sacrificeExplosionReactionDegreeResult->setData(0, Qt::UserRole, "sacrificeExplosionReactionDegreeResult");
	sacrificeExplosionReactionDegreeResult->setIcon(0, error_icon);
	{
		QTreeWidgetItem* sacrificeExplosionDegreePropellantResult = new QTreeWidgetItem();
		sacrificeExplosionDegreePropellantResult->setText(0, "推进剂+绝热层");
		sacrificeExplosionDegreePropellantResult->setData(0, Qt::UserRole, "sacrificeExplosionDegreePropellantResult");
		sacrificeExplosionDegreePropellantResult->setIcon(0, error_icon);

		sacrificeExplosionReactionDegreeResult->addChild(sacrificeExplosionDegreePropellantResult);
	}
	sacrificeExplosionAnalysis->addChild(sacrificeExplosionStressResult);
	sacrificeExplosionAnalysis->addChild(sacrificeExplosionStrainResult);
	sacrificeExplosionAnalysis->addChild(sacrificeExplosionTemperatureResult);
	sacrificeExplosionAnalysis->addChild(sacrificeExplosionOverpressureResult);
	sacrificeExplosionAnalysis->addChild(sacrificeExplosionReactionDegreeResult);

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
	// 1.跌落
	auto fallStressResult = ModelDataManager::GetInstance()->GetFallStressResult();
	auto fallStrainResult = ModelDataManager::GetInstance()->GetFallStrainResult();
	auto fallTemperatureResult = ModelDataManager::GetInstance()->GetFallTemperatureResult();
	auto fallOverpressureResult = ModelDataManager::GetInstance()->GetFallOverpressureResult();
	// 2.快考
	auto fastCombustionTemperatureResult = ModelDataManager::GetInstance()->GetFastCombustionTemperatureResult();
	// 3.慢烤
	auto slowCombustionTemperatureResult = ModelDataManager::GetInstance()->GetSlowCombustionTemperatureResult();
	// 4.枪击
	auto shootStressResult = ModelDataManager::GetInstance()->GetShootStressResult();
	auto shootStrainResult = ModelDataManager::GetInstance()->GetShootStrainResult();
	auto shootTemperatureResult = ModelDataManager::GetInstance()->GetShootTemperatureResult();
	auto shootOverpressureResult = ModelDataManager::GetInstance()->GetShootOverpressureResult();
	// 5.射流冲击
	auto jetImpactStressResult = ModelDataManager::GetInstance()->GetJetImpactStressResult();
	auto jetImpactStrainResult = ModelDataManager::GetInstance()->GetJetImpactStrainResult();
	auto jetImpactTemperatureResult = ModelDataManager::GetInstance()->GetJetImpactTemperatureResult();
	auto jetImpactOverpressureResult = ModelDataManager::GetInstance()->GetJetImpactOverpressureResult();
	// 6.破片撞击
	auto fragmentationImpactStressResult = ModelDataManager::GetInstance()->GetFragmentationImpactStressResult();
	auto fragmentationImpactStrainResult = ModelDataManager::GetInstance()->GetFragmentationImpactStrainResult();
	auto fragmentationImpactTemperatureResult = ModelDataManager::GetInstance()->GetFragmentationImpactTemperatureResult();
	auto fragmentationImpactOverpressureResult = ModelDataManager::GetInstance()->GetFragmentationImpactOverpressureResult();
	// 7.爆炸冲击波
	auto explosiveBlastStressResult = ModelDataManager::GetInstance()->GetExplosiveBlastStressResult();
	auto explosiveBlastStrainResult = ModelDataManager::GetInstance()->GetExplosiveBlastStrainResult();
	auto explosiveBlastTemperatureResult = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult();
	auto explosiveBlastOverpressureResult = ModelDataManager::GetInstance()->GetExplosiveBlastOverpressureResult();
	// 8.殉爆安全性分析
	auto sacrificeExplosionStressResult = ModelDataManager::GetInstance()->GetSacrificeExplosionStressResult();
	auto sacrificeExplosionStrainResult = ModelDataManager::GetInstance()->GetSacrificeExplosionStrainResult();
	auto sacrificeExplosionTemperatureResult = ModelDataManager::GetInstance()->GetSacrificeExplosionTemperatureResult();
	auto sacrificeExplosionOverpressureResult = ModelDataManager::GetInstance()->GetSacrificeExplosionOverpressureResult();


	QString workdir = userinfo.workdir;

	QString itemData = item->data(0, Qt::UserRole).toString();
	emit itemClicked(itemData);

	if (itemData.contains("ShellResult")|| itemData.contains("PropellantResult") || itemData.contains("TemperatureResult"))
	{
		QWidget* parent = parentWidget();
		while (parent) {
			GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
			if (gfParent)
			{
				// 截图结果云图
				QString m_privateDirPath = "";
				if (itemData == "FallStressShellResult" && fallStressResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fall/ShellStress.png";
					fallStressResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetFallStressResult(fallStressResult);
				}
				else if (itemData == "FallStressPropellantResult" && fallStressResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fall/PropellantStress.png";
					fallStressResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetFallStressResult(fallStressResult);
				}
				else if (itemData == "FallStrainShellResult" && fallStrainResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fall/ShellStrain.png";
					fallStrainResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetFallStrainResult(fallStrainResult);
				}
				else if (itemData == "FallStrainPropellantResult" && fallStrainResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fall/PropellantStrain.png";
					fallStrainResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetFallStrainResult(fallStrainResult);
				}
				else if (itemData == "FallTemperatureShellResult" && fallTemperatureResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fall/ShellTemperature.png";
					fallTemperatureResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetFallTemperatureResult(fallTemperatureResult);
				}
				else if (itemData == "FallTemperaturePropellantResult" && fallTemperatureResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fall/PropellantTemperature.png";
					fallTemperatureResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetFallTemperatureResult(fallTemperatureResult);
				}
				else if (itemData == "FallOverpressureShellResult" && fallOverpressureResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fall/ShellOverpressure.png";
					fallOverpressureResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetFallOverpressureResult(fallOverpressureResult);
				}
				else if (itemData == "FallOverpressurePropellantResult" && fallOverpressureResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fall//PropellantOverpressure.png";
					fallOverpressureResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetFallOverpressureResult(fallOverpressureResult);
				}
				// 快烤
				else if (itemData == "FastCombustionTemperatureResult" && fastCombustionTemperatureResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fastCombustion/ShellTemperature.png";
					fastCombustionTemperatureResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetFastCombustionTemperatureResult(fastCombustionTemperatureResult);
				}
				else if (itemData == "fastTemperatureShellResult" && fastCombustionTemperatureResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fastCombustion/PropellantTemperature.png";
					fastCombustionTemperatureResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetFastCombustionTemperatureResult(fastCombustionTemperatureResult);
				}
				// 慢烤
				else if (itemData == "SlowCombustionTemperatureResult" && slowCombustionTemperatureResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/slowCombustion/ShellTemperature.png";
					slowCombustionTemperatureResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetSlowCombustionTemperatureResult(slowCombustionTemperatureResult);
				}
				else if (itemData == "slowTemperatureShellResult" && slowCombustionTemperatureResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/slowCombustion/PropellantTemperature.png";
					slowCombustionTemperatureResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetSlowCombustionTemperatureResult(slowCombustionTemperatureResult);
				}
				// 枪击
				else if (itemData == "shootStressShellResult" && shootStressResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/shoot/ShellStress.png";
					shootStressResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetShootStressResult(shootStressResult);
				}
				else if (itemData == "shootStressPropellantResult" && shootStressResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/shoot/PropellantStress.png";
					shootStressResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetShootStressResult(shootStressResult);
				}
				else if (itemData == "shootStrainShellResult" && shootStrainResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/shoot/ShellStrain.png";
					shootStrainResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetShootStrainResult(shootStrainResult);
				}
				else if (itemData == "shootStrainPropellantResult" && shootStrainResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/shoot/PropellantStrain.png";
					shootStrainResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetShootStrainResult(shootStrainResult);
				}
				else if (itemData == "shootTempShellResult" && shootTemperatureResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/shoot/ShellTemperature.png";
					shootTemperatureResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetShootTemperatureResult(shootTemperatureResult);
				}
				else if (itemData == "shootTempPropellantResult" && shootTemperatureResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/shoot/PropellantTemperature.png";
					shootTemperatureResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetShootTemperatureResult(shootTemperatureResult);
				}
				else if (itemData == "shootOverpressureShellResult" && shootOverpressureResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/shoot/ShellOverpressure.png";
					shootOverpressureResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetShootOverpressureResult(shootOverpressureResult);
				}
				else if (itemData == "shootOverpressurePropellantResult" && shootOverpressureResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/shoot/PropellantOverpressure.png";
					shootOverpressureResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetShootOverpressureResult(shootOverpressureResult);
				}
				// 射流冲击
				else if (itemData == "jetStressShellResult" && jetImpactStressResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/jetImpact/ShellStress.png";
					jetImpactStressResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetJetImpactStressResult(jetImpactStressResult);
				}
				else if (itemData == "jetStressPropellantResult" && jetImpactStressResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/jetImpact/PropellantStress.png";
					jetImpactStressResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetJetImpactStressResult(jetImpactStressResult);
				}
				else if (itemData == "jetStrainShellResult" && jetImpactStrainResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/jetImpact/ShellStrain.png";
					jetImpactStrainResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetJetImpactStrainResult(jetImpactStrainResult);
				}
				else if (itemData == "jetStrainPropellantResult" && jetImpactStrainResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/jetImpact/PropellantStrain.png";
					jetImpactStrainResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetJetImpactStrainResult(jetImpactStrainResult);
				}
				else if (itemData == "jetTempShellResult" && jetImpactTemperatureResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/jetImpact/ShellTemperature.png";
					jetImpactTemperatureResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetJetImpactTemperatureResult(jetImpactTemperatureResult);
				}
				else if (itemData == "jetTempPropellantResult" && jetImpactTemperatureResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/jetImpact/PropellantTemperature.png";
					jetImpactTemperatureResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetJetImpactTemperatureResult(jetImpactTemperatureResult);
				}
				else if (itemData == "jetOverpressureShellResult" && jetImpactOverpressureResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/jetImpact/ShellOverpressure.png";
					jetImpactOverpressureResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetJetImpactOverpressureResult(jetImpactOverpressureResult);
				}
				else if (itemData == "jetOverpressurePropellantResult" && jetImpactOverpressureResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/jetImpact/PropellantOverpressure.png";
					jetImpactOverpressureResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetJetImpactOverpressureResult(jetImpactOverpressureResult);
				}
				// 破片撞击
				else if (itemData == "fragmentationStressShellResult" && fragmentationImpactStressResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fragmentationImpact/ShellStress.png";
					fragmentationImpactStressResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetFragmentationImpactStressResult(fragmentationImpactStressResult);
				}
				else if (itemData == "fragmentationStressPropellantResult" && fragmentationImpactStressResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fragmentationImpact/PropellantStress.png";
					fragmentationImpactStressResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetFragmentationImpactStressResult(fragmentationImpactStressResult);
				}
				else if (itemData == "fragmentationStrainShellResult" && fragmentationImpactStrainResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fragmentationImpact/ShellStrain.png";
					fragmentationImpactStrainResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetFragmentationImpactStrainResult(fragmentationImpactStrainResult);
				}
				else if (itemData == "fragmentationStrainPropellantResult" && fragmentationImpactStrainResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fragmentationImpact/PropellantStrain.png";
					fragmentationImpactStrainResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetFragmentationImpactStrainResult(fragmentationImpactStrainResult);
				}
				else if (itemData == "fragmentationTempShellResult" && fragmentationImpactTemperatureResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fragmentationImpact/ShellTemperature.png";
					fragmentationImpactTemperatureResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetFragmentationImpactTemperatureResult(fragmentationImpactTemperatureResult);
				}
				else if (itemData == "fragmentationTempPropellantResult" && fragmentationImpactTemperatureResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fragmentationImpact/PropellantTemperature.png";
					fragmentationImpactTemperatureResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetFragmentationImpactTemperatureResult(fragmentationImpactTemperatureResult);
				}
				else if (itemData == "fragmentationOverpressureShellResult" && fragmentationImpactOverpressureResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fragmentationImpact/ShellOverpressure.png";
					fragmentationImpactOverpressureResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetFragmentationImpactOverpressureResult(fragmentationImpactOverpressureResult);
				}
				else if (itemData == "fragmentationOverpressurePropellantResult" && fragmentationImpactOverpressureResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/fragmentationImpact/PropellantOverpressure.png";
					fragmentationImpactOverpressureResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetFragmentationImpactOverpressureResult(fragmentationImpactOverpressureResult);
				}
				// 爆炸冲击波
				else if (itemData == "explosiveStressShellResult" && explosiveBlastStressResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/explosiveBlast/ShellStress.png";
					explosiveBlastStressResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetExplosiveBlastStressResult(explosiveBlastStressResult);
				}
				else if (itemData == "explosiveStressPropellantResult" && explosiveBlastStressResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/explosiveBlast/PropellantStress.png";
					explosiveBlastStressResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetExplosiveBlastStressResult(explosiveBlastStressResult);
				}
				else if (itemData == "explosiveStrainShellResult" && explosiveBlastStrainResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/explosiveBlast/ShellStrain.png";
					explosiveBlastStrainResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetExplosiveBlastStrainResult(explosiveBlastStrainResult);
				}
				else if (itemData == "explosiveStrainPropellantResult" && explosiveBlastStrainResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/explosiveBlast/PropellantStrain.png";
					explosiveBlastStrainResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetExplosiveBlastStrainResult(explosiveBlastStrainResult);
				}
				else if (itemData == "explosiveTempShellResult" && explosiveBlastTemperatureResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/explosiveBlast/ShellTemperature.png";
					explosiveBlastTemperatureResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetExplosiveBlastTemperatureResult(explosiveBlastTemperatureResult);
				}
				else if (itemData == "explosiveTempPropellantResult" && explosiveBlastTemperatureResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/explosiveBlast/PropellantTemperature.png";
					explosiveBlastTemperatureResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetExplosiveBlastTemperatureResult(explosiveBlastTemperatureResult);
				}
				else if (itemData == "explosiveOverpressureShellResult" && explosiveBlastOverpressureResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/explosiveBlast/ShellOverpressure.png";
					explosiveBlastOverpressureResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetExplosiveBlastOverpressureResult(explosiveBlastOverpressureResult);
				}
				else if (itemData == "explosiveOverpressurePropellantResult" && explosiveBlastOverpressureResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/explosiveBlast/PropellantOverpressure.png";
					explosiveBlastOverpressureResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetExplosiveBlastOverpressureResult(explosiveBlastOverpressureResult);
				}
				// 殉爆
				else if (itemData == "sacrificeStressShellResult" && sacrificeExplosionStressResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/sacrificeExplosio/ShellStress.png";
					sacrificeExplosionStressResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetSacrificeExplosionStressResult(sacrificeExplosionStressResult);
				}
				if (itemData == "sacrificeStressPropellantResult" && sacrificeExplosionStressResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/sacrificeExplosio/PropellantStress.png";
					sacrificeExplosionStressResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetSacrificeExplosionStressResult(sacrificeExplosionStressResult);
				}
				else if (itemData == "sacrificeStrainShellResult" && sacrificeExplosionStrainResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/sacrificeExplosio/ShellStrain.png";
					sacrificeExplosionStrainResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetSacrificeExplosionStrainResult(sacrificeExplosionStrainResult);
				}
				else if (itemData == "sacrificeStrainPropellantResult" && sacrificeExplosionStrainResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/sacrificeExplosio/PropellantStrain.png";
					sacrificeExplosionStrainResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetSacrificeExplosionStrainResult(sacrificeExplosionStrainResult);
				}
				else if (itemData == "sacrificeTempShellResult" && sacrificeExplosionTemperatureResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/sacrificeExplosio/ShellTemperature.png";
					sacrificeExplosionTemperatureResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetSacrificeExplosionTemperatureResult(sacrificeExplosionTemperatureResult);
				}
				else if (itemData == "sacrificeTempPropellantResult" && sacrificeExplosionTemperatureResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/sacrificeExplosio/PropellantTemperature.png";
					sacrificeExplosionTemperatureResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetSacrificeExplosionTemperatureResult(sacrificeExplosionTemperatureResult);
				}
				else if (itemData == "sacrificeOverpressureShellResult" && sacrificeExplosionOverpressureResult.shellScreenFlag)
				{
					m_privateDirPath = workdir + "/template/sacrificeExplosio/ShellOverpressure.png";
					sacrificeExplosionOverpressureResult.shellScreenFlag = false;
					ModelDataManager::GetInstance()->SetSacrificeExplosionOverpressureResult(sacrificeExplosionOverpressureResult);
				}
				else if (itemData == "sacrificeOverpressurePropellantResult" && sacrificeExplosionOverpressureResult.propellantScreenFlag)
				{
					m_privateDirPath = workdir + "/template/sacrificeExplosio/PropellantOverpressure.png";
					sacrificeExplosionOverpressureResult.propellantScreenFlag = false;
					ModelDataManager::GetInstance()->SetSacrificeExplosionOverpressureResult(sacrificeExplosionOverpressureResult);
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
	auto fallAnalysisResultInfo = ins->GetFallAnalysisResultInfo();
	auto fastCombustionAnalysisResultInfo = ins->GetFastCombustionAnalysisResultInfo();
	auto slowCombustionAnalysisResultInfo = ins->GetSlowCombustionAnalysisResultInfo();
	auto shootAnalysisResultInfo = ins->GetShootAnalysisResultInfo();
	auto jetImpactAnalysisResultInfo = ins->GetJetImpactAnalysisResultInfo();
	auto fragmentationAnalysisResultInfo = ins->GetFragmentationAnalysisResultInfo();
	auto explosiveBlastAnalysisResultInfo = ins->GetExplosiveBlastAnalysisResultInfo();
	auto sacrificeExplosionAnalysisResultInfo = ins->GetSacrificeExplosionAnalysisResultInfo();

	int size = m_treeWidget->topLevelItemCount();
	QTreeWidgetItem *child;
	for (int i = 0; i < size; i++)
	{
		child = m_treeWidget->topLevelItem(i);
		int childCount = child->childCount();
		for (int j = 0; j < childCount; ++j)
		{
			if (child->child(j)->text(0).contains("固体发动机三维模型"))
			{
				QTreeWidgetItem* clChild = child->child(j);
				int clChildCount = clChild->childCount();
				for (int m = 0; m < clChildCount; ++m) 
				{
					if (clChild->child(m)->text(0).contains("喷管"))
					{
						if (geomInfo.nozzleAisShape.IsNull())
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					else if (clChild->child(m)->text(0).contains("壳体"))
					{
						if (geomInfo.shellAisShape.IsNull())
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					else if (clChild->child(m)->text(0).contains("推进剂"))
					{
						if (geomInfo.propellantAisShape.IsNull())
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					else if (clChild->child(m)->text(0).contains("绝热层"))
					{
						if (geomInfo.heatInsulatingLayerAisShape.IsNull())
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
				}
				if (!geomInfo.nozzleAisShape.IsNull() && !geomInfo.shellAisShape.IsNull() && 
					!geomInfo.propellantAisShape.IsNull() && !geomInfo.heatInsulatingLayerAisShape.IsNull())
				{
					clChild->setIcon(0, checked_icon);
				}


				/*if (geomInfo.path.isEmpty())
				{
					child->child(j)->setIcon(0, error_icon);
				}
				else
				{
					child->child(j)->setIcon(0, checked_icon);
				}*/
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
				QTreeWidgetItem* clChild = child->child(j);
				int clChildCount = clChild->childCount();
				for (int m = 0; m < clChildCount; ++m) 
				{
					if (clChild->child(m)->text(0).contains("喷管"))
					{
						if (!meshInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					else if (clChild->child(m)->text(0).contains("壳体"))
					{
						if (!meshInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					else if (clChild->child(m)->text(0).contains("推进剂"))
					{
						if (!meshInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					else if (clChild->child(m)->text(0).contains("绝热层"))
					{
						if (!meshInfo.isChecked)
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
			else if (child->child(j)->text(0).contains("安全特性参数分析"))
			{
				QTreeWidgetItem* clChild = child->child(j);
				int clChildCount = clChild->childCount();
				for (int m = 0; m < clChildCount; ++m) {
					if (clChild->child(m)->text(0).contains("1.跌落安全性分析"))
					{
						QTreeWidgetItem* clclChild = clChild->child(m);
						int clclChildCount = clclChild->childCount();
						for (int o = 0; o < clclChildCount; ++o)
						{
							if (clclChild->child(o)->text(0).contains("应力分析") || clclChild->child(o)->text(0).contains("应变分析") || clclChild->child(o)->text(0).contains("温度分析") || clclChild->child(o)->text(0).contains("超压分析") || clclChild->child(o)->text(0).contains("反应度"))
							{
								if (!fallAnalysisResultInfo.isChecked)
								{
									clclChild->child(o)->setIcon(0, error_icon);
								}
								else
								{
									clclChild->child(o)->setIcon(0, checked_icon);
								}

								QTreeWidgetItem* clclclChild = clclChild->child(o);
								int clclclChildCount = clclclChild->childCount();
								for (int p = 0; p < clclclChildCount; ++p)
								{
									if (!fallAnalysisResultInfo.isChecked)
									{
										clclclChild->child(p)->setIcon(0, error_icon);
									}
									else
									{
										clclclChild->child(p)->setIcon(0, checked_icon);
									}
								}
							}
						}			
					}
					else if (clChild->child(m)->text(0).contains("2.快速烤燃安全性分析"))
					{
						QTreeWidgetItem* clclChild = clChild->child(m);
						int clclChildCount = clclChild->childCount();
						for (int o = 0; o < clclChildCount; ++o)
						{
							if (clclChild->child(o)->text(0).contains("应力分析") || clclChild->child(o)->text(0).contains("应变分析") || clclChild->child(o)->text(0).contains("温度分析") || clclChild->child(o)->text(0).contains("超压分析") || clclChild->child(o)->text(0).contains("反应度"))
							{
								if (!fastCombustionAnalysisResultInfo.isChecked)
								{
									clclChild->child(o)->setIcon(0, error_icon);
								}
								else
								{
									clclChild->child(o)->setIcon(0, checked_icon);
								}
								QTreeWidgetItem* clclclChild = clclChild->child(o);
								int clclclChildCount = clclclChild->childCount();
								for (int p = 0; p < clclclChildCount; ++p)
								{
									if (!fastCombustionAnalysisResultInfo.isChecked)
									{
										clclclChild->child(p)->setIcon(0, error_icon);
									}
									else
									{
										clclclChild->child(p)->setIcon(0, checked_icon);
									}
								}
							}
						}
					}
					else if (clChild->child(m)->text(0).contains("3.慢速烤燃安全性分析"))
					{
						QTreeWidgetItem* clclChild = clChild->child(m);
						int clclChildCount = clclChild->childCount();
						for (int o = 0; o < clclChildCount; ++o)
						{
							if (clclChild->child(o)->text(0).contains("应力分析") || clclChild->child(o)->text(0).contains("应变分析") || clclChild->child(o)->text(0).contains("温度分析") || clclChild->child(o)->text(0).contains("超压分析") || clclChild->child(o)->text(0).contains("反应度"))
							{
								if (!slowCombustionAnalysisResultInfo.isChecked)
								{
									clclChild->child(o)->setIcon(0, error_icon);
								}
								else
								{
									clclChild->child(o)->setIcon(0, checked_icon);
								}
								QTreeWidgetItem* clclclChild = clclChild->child(o);
								int clclclChildCount = clclclChild->childCount();
								for (int p = 0; p < clclclChildCount; ++p)
								{
									if (!slowCombustionAnalysisResultInfo.isChecked)
									{
										clclclChild->child(p)->setIcon(0, error_icon);
									}
									else
									{
										clclclChild->child(p)->setIcon(0, checked_icon);
									}
								}
							}
						}
					}
					else if (clChild->child(m)->text(0).contains("4.枪击安全性分析"))
					{
						QTreeWidgetItem* clclChild = clChild->child(m);
						int clclChildCount = clclChild->childCount();
						for (int o = 0; o < clclChildCount; ++o)
						{
							if (clclChild->child(o)->text(0).contains("应力分析") || clclChild->child(o)->text(0).contains("应变分析") || clclChild->child(o)->text(0).contains("温度分析") || clclChild->child(o)->text(0).contains("超压分析") || clclChild->child(o)->text(0).contains("反应度"))
							{
								if (!shootAnalysisResultInfo.isChecked)
								{
									clclChild->child(o)->setIcon(0, error_icon);
								}
								else
								{
									clclChild->child(o)->setIcon(0, checked_icon);
								}
								QTreeWidgetItem* clclclChild = clclChild->child(o);
								int clclclChildCount = clclclChild->childCount();
								for (int p = 0; p < clclclChildCount; ++p)
								{
									if (!shootAnalysisResultInfo.isChecked)
									{
										clclclChild->child(p)->setIcon(0, error_icon);
									}
									else
									{
										clclclChild->child(p)->setIcon(0, checked_icon);
									}
								}
							}
						}
					}
					else if (clChild->child(m)->text(0).contains("5.射流冲击安全性分析"))
					{
					QTreeWidgetItem* clclChild = clChild->child(m);
					int clclChildCount = clclChild->childCount();
					for (int o = 0; o < clclChildCount; ++o)
					{
						if (clclChild->child(o)->text(0).contains("应力分析") || clclChild->child(o)->text(0).contains("应变分析") || clclChild->child(o)->text(0).contains("温度分析") || clclChild->child(o)->text(0).contains("超压分析") || clclChild->child(o)->text(0).contains("反应度"))
						{
							if (!jetImpactAnalysisResultInfo.isChecked)
							{
								clclChild->child(o)->setIcon(0, error_icon);
							}
							else
							{
								clclChild->child(o)->setIcon(0, checked_icon);
							}
							QTreeWidgetItem* clclclChild = clclChild->child(o);
							int clclclChildCount = clclclChild->childCount();
							for (int p = 0; p < clclclChildCount; ++p)
							{
								if (!jetImpactAnalysisResultInfo.isChecked)
								{
									clclclChild->child(p)->setIcon(0, error_icon);
								}
								else
								{
									clclclChild->child(p)->setIcon(0, checked_icon);
								}
							}
						}
					}
					}
					else if (clChild->child(m)->text(0).contains("6.破片撞击安全性分析"))
					{
					QTreeWidgetItem* clclChild = clChild->child(m);
					int clclChildCount = clclChild->childCount();
					for (int o = 0; o < clclChildCount; ++o)
					{
						if (clclChild->child(o)->text(0).contains("应力分析") || clclChild->child(o)->text(0).contains("应变分析") || clclChild->child(o)->text(0).contains("温度分析") || clclChild->child(o)->text(0).contains("超压分析") || clclChild->child(o)->text(0).contains("反应度"))
						{
							if (!fragmentationAnalysisResultInfo.isChecked)
							{
								clclChild->child(o)->setIcon(0, error_icon);
							}
							else
							{
								clclChild->child(o)->setIcon(0, checked_icon);
							}
							QTreeWidgetItem* clclclChild = clclChild->child(o);
							int clclclChildCount = clclclChild->childCount();
							for (int p = 0; p < clclclChildCount; ++p)
							{
								if (!fragmentationAnalysisResultInfo.isChecked)
								{
									clclclChild->child(p)->setIcon(0, error_icon);
								}
								else
								{
									clclclChild->child(p)->setIcon(0, checked_icon);
								}
							}
						}
					}
					}
					else if (clChild->child(m)->text(0).contains("7.爆炸冲击波安全性分析"))
					{
					QTreeWidgetItem* clclChild = clChild->child(m);
					int clclChildCount = clclChild->childCount();
					for (int o = 0; o < clclChildCount; ++o)
					{
						if (clclChild->child(o)->text(0).contains("应力分析") || clclChild->child(o)->text(0).contains("应变分析") || clclChild->child(o)->text(0).contains("温度分析") || clclChild->child(o)->text(0).contains("超压分析") || clclChild->child(o)->text(0).contains("反应度"))
						{
							if (!explosiveBlastAnalysisResultInfo.isChecked)
							{
								clclChild->child(o)->setIcon(0, error_icon);
							}
							else
							{
								clclChild->child(o)->setIcon(0, checked_icon);
							}
							QTreeWidgetItem* clclclChild = clclChild->child(o);
							int clclclChildCount = clclclChild->childCount();
							for (int p = 0; p < clclclChildCount; ++p)
							{
								if (!explosiveBlastAnalysisResultInfo.isChecked)
								{
									clclclChild->child(p)->setIcon(0, error_icon);
								}
								else
								{
									clclclChild->child(p)->setIcon(0, checked_icon);
								}
							}
						}
					}
					}
					else if (clChild->child(m)->text(0).contains("8.殉爆安全性分析"))
					{
					QTreeWidgetItem* clclChild = clChild->child(m);
					int clclChildCount = clclChild->childCount();
					for (int o = 0; o < clclChildCount; ++o)
					{
						if (clclChild->child(o)->text(0).contains("应力分析") || clclChild->child(o)->text(0).contains("应变分析") || clclChild->child(o)->text(0).contains("温度分析") || clclChild->child(o)->text(0).contains("超压分析") || clclChild->child(o)->text(0).contains("反应度"))
						{
							if (!sacrificeExplosionAnalysisResultInfo.isChecked)
							{
								clclChild->child(o)->setIcon(0, error_icon);
							}
							else
							{
								clclChild->child(o)->setIcon(0, checked_icon);
							}
							QTreeWidgetItem* clclclChild = clclChild->child(o);
							int clclclChildCount = clclclChild->childCount();
							for (int p = 0; p < clclclChildCount; ++p)
							{
								if (!sacrificeExplosionAnalysisResultInfo.isChecked)
								{
									clclclChild->child(p)->setIcon(0, error_icon);
								}
								else
								{
									clclclChild->child(p)->setIcon(0, checked_icon);
								}
							}
						}
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
	if (text == "固体发动机三维模型")
	{
		QMenu menu(this);
		QAction* actShowAll = menu.addAction("显示全部");
		QAction* actHideAll = menu.addAction("隐藏全部");

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

		if (selected == actShowAll)
		{
			auto& geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
			auto occView = importModelWidget->GetOccView();
			Handle(AIS_InteractiveContext) context = occView->getContext();

			if (!geomInfo.nozzleAisShape.IsNull())
				context->Display(geomInfo.nozzleAisShape, Standard_True);
			if (!geomInfo.shellAisShape.IsNull())
				context->Display(geomInfo.shellAisShape, Standard_True);
			if (!geomInfo.propellantAisShape.IsNull())
				context->Display(geomInfo.propellantAisShape, Standard_True);
			if (!geomInfo.heatInsulatingLayerAisShape.IsNull())
				context->Display(geomInfo.heatInsulatingLayerAisShape, Standard_True);
		}
		else if (selected == actHideAll)
		{
			auto& geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
			auto occView = importModelWidget->GetOccView();
			Handle(AIS_InteractiveContext) context = occView->getContext();

			if (!geomInfo.nozzleAisShape.IsNull())
				context->Erase(geomInfo.nozzleAisShape, Standard_True);
			if (!geomInfo.shellAisShape.IsNull())
				context->Erase(geomInfo.shellAisShape, Standard_True);
			if (!geomInfo.propellantAisShape.IsNull())
				context->Erase(geomInfo.propellantAisShape, Standard_True);
			if (!geomInfo.heatInsulatingLayerAisShape.IsNull())
				context->Erase(geomInfo.heatInsulatingLayerAisShape, Standard_True);
		}
	}
	else if (text == "喷管"||text == "壳体" || text == "推进剂" || text == "绝热层")
	{
		QTreeWidgetItem* parentItem = item->parent();
		if (parentItem && parentItem->data(0, Qt::UserRole).toString() == "Geometry")
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
				if (text == "喷管")
				{
					partType = PartType::Nozzle;
				}
				else if (text == "壳体")
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

				connect(workerThread, &QThread::started, worker, &GeometryImportWorker::DoWork);
				connect(worker, &GeometryImportWorker::ProgressUpdated,
					progressDialog, &ProgressDialog::SetProgress);
				connect(worker, &GeometryImportWorker::StatusUpdated,
					progressDialog, &ProgressDialog::SetStatusText);
				connect(progressDialog, &ProgressDialog::Canceled,
					worker, &GeometryImportWorker::RequestInterruption);

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
							if (text == "喷管" && !info.nozzleAisShape.IsNull())
							{
								aisShape = info.nozzleAisShape;
							}
							else if (text == "壳体" && !info.shellAisShape.IsNull())
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

								if (text == "喷管")
								{
									existingInfo.nozzleAisShape = info.nozzleAisShape;
								}
								else if (text == "壳体")
								{
									existingInfo.shellAisShape = info.shellAisShape;
									existingInfo.shellPath = info.shellPath;   // ← 新增

									existingInfo.ptShellLeftBottom = info.ptShellLeftBottom;
									existingInfo.ptShellRightBottom = info.ptShellRightBottom;
									existingInfo.ptNozzleInletBottom = info.ptNozzleInletBottom;
									existingInfo.ptNozzleOutletBottom = info.ptNozzleOutletBottom;
								}
								else if (text == "推进剂")
								{
									existingInfo.propellantAisShape = info.propellantAisShape;
									existingInfo.propellantPath = info.propellantPath;  // ← 新增
								}
								else if (text == "绝热层")
								{
									existingInfo.heatInsulatingLayerAisShape = info.heatInsulatingLayerAisShape;
									existingInfo.heatInsulatingLayerPath = info.heatInsulatingLayerPath;  // ← 新增
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

								//显示所有shape，便于截图
								auto& geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
								if (!geomInfo.nozzleAisShape.IsNull() && !geomInfo.shellAisShape.IsNull() &&
									!geomInfo.propellantAisShape.IsNull() && !geomInfo.heatInsulatingLayerAisShape.IsNull())
								{								
									context->Display(geomInfo.nozzleAisShape, Standard_True);
									context->Display(geomInfo.shellAisShape, Standard_True);
									context->Display(geomInfo.propellantAisShape, Standard_True);
									context->Display(geomInfo.heatInsulatingLayerAisShape, Standard_True);

									Handle(V3d_View) view = occView->getView();
									view->SetProj(V3d_Zneg);
								}

								occView->fitAll();
								occView->update();

								if (auto geomProWid = importModelWidget->findChild<GeomPropertyWidget*>())
								{
									geomProWid->UpdataPropertyInfo();
								}
								// 截图
								QTimer::singleShot(500, this, [=]() {

									auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
									if (!geomInfo.nozzleAisShape.IsNull() && !geomInfo.shellAisShape.IsNull() && !geomInfo.propellantAisShape.IsNull() && !geomInfo.heatInsulatingLayerAisShape.IsNull())
									{
										UserInfo userinfo = ModelDataManager::GetInstance()->GetUserInfo();
										// 截图计算模型
										QString m_privateDirPath = userinfo.workdir + "/template/main.png";
										QDir privateDir(m_privateDirPath);
										wordExporter->captureWidgetToFile(importModelWidget->GetOccView(), m_privateDirPath);
									}
								});
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

					}, Qt::QueuedConnection);
				workerThread->start();
			}
			else if (selected == actShow)
			{
				auto& geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
				Handle(AIS_Shape) aisShape;
				if (text == "喷管")
				{
					aisShape = geomInfo.nozzleAisShape;
				}
				else if (text == "壳体")
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
				if (text == "喷管")
				{
					aisShape = geomInfo.nozzleAisShape;
				}
				else if (text == "壳体")
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
		else if (parentItem && parentItem->data(0, Qt::UserRole).toString() == "Mesh")
		{
			QMenu menu(this);
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

			// 获取对应网格的AIS显示对象
			auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
			Handle(AIS_Shape) aisMesh;
			QString role = item->data(0, Qt::UserRole).toString();

			if (role == "NozzleMesh")
			{
				aisMesh = meshInfo.nozzleAisMesh;
			}
			else if (role == "ShellMesh")
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
				geomInfo.nozzleAisShape,
				geomInfo.shellAisShape,        // 壳体
				geomInfo.propellantAisShape,   // 推进剂
				geomInfo.heatInsulatingLayerAisShape // 绝热层
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

						// 显示四种网格并保存AIS对象
						Handle(AIS_Shape) nozzleAis = displayMesh(info.nozzleMesh, "喷管", QColor(89, 89, 94));   // 深钢灰
						Handle(AIS_Shape) shellAis = displayMesh(info.shellMesh, "壳体", QColor(209, 214, 219));   // 银白金属
						Handle(AIS_Shape) propAis = displayMesh(info.propellantMesh, "推进剂", QColor(230, 97, 38));   // 砖红
						Handle(AIS_Shape) heatAis = displayMesh(info.heatInsulatingLayerMesh, "绝热层", QColor(51, 153, 191));   // 青蓝

						// 保存网格数据及显示对象到数据管理器
						ModelMeshInfo updatedInfo = info;
						updatedInfo.nozzleAisMesh = nozzleAis;
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
		QVector<QString> processedNameList;

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
					processedNameList.push_back("跌落试验计算");
				}
				else if (processedName == "快速烤燃安全性分析")
				{
					processedNameList.push_back("快速烤燃试验计算");
				}
				else if (processedName == "慢速烤燃安全性分析")
				{
					processedNameList.push_back("慢速烤燃试验计算");
				}
				else if (processedName == "枪击安全性分析")
				{
					processedNameList.push_back("枪击试验计算");
				}
				else if (processedName == "射流冲击安全性分析")
				{
					processedNameList.push_back("射流冲击试验计算");
				}
				else if (processedName == "破片撞击安全性分析")
				{
					processedNameList.push_back("破片撞击试验计算");
				}
				else if (processedName == "爆炸冲击波安全性分析")
				{
					processedNameList.push_back("爆炸冲击试验计算");
				}
				else if (processedName == "殉爆安全性分析")
				{
					processedNameList.push_back("殉爆试验计算");
				}
			}
		}


		connect(calAction, &QAction::triggered, this, [item, processedNameList, this]() {

			if (processedNameList.isEmpty())
			{
				QMessageBox::warning(this, "计算", "请先选择安全性分析场景");
				return;
			}

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
					CalculateWorker* worker = new CalculateWorker(processedNameList);
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

												auto fallReactionDegreeResult = ModelDataManager::GetInstance()->GetFallReactionDegreeResult();
												gfParent->GetReactionDegreeResultWidget()->updateData(fallReactionDegreeResult.metalsMaxReactionDegree, fallReactionDegreeResult.metalsMinReactionDegree, fallReactionDegreeResult.metalsAvgReactionDegree, fallReactionDegreeResult.metalsStandardReactionDegree,
													fallReactionDegreeResult.propellantsMaxReactionDegree, fallReactionDegreeResult.propellantsMinReactionDegree, fallReactionDegreeResult.propellantsAvgReactionDegree, fallReactionDegreeResult.propellantsStandardReactionDegree,
													fallReactionDegreeResult.outheatMaxReactionDegree, fallReactionDegreeResult.outheatMinReactionDegree, fallReactionDegreeResult.outheatAvgReactionDegree, fallReactionDegreeResult.outheatStandardReactionDegree,
													fallReactionDegreeResult.insulatingheatMaxReactionDegree, fallReactionDegreeResult.insulatingheatMinReactionDegree, fallReactionDegreeResult.insulatingheatAvgReactionDegree, fallReactionDegreeResult.insulatingheatStandardReactionDegree);


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

												auto reactionDegreeResult = ModelDataManager::GetInstance()->GetShootReactionDegreeResult();
												gfParent->GetShootReactionDegreeResultWidget()->updateData(reactionDegreeResult.metalsMaxReactionDegree, reactionDegreeResult.metalsMinReactionDegree, reactionDegreeResult.metalsAvgReactionDegree, reactionDegreeResult.metalsStandardReactionDegree,
													reactionDegreeResult.propellantsMaxReactionDegree, reactionDegreeResult.propellantsMinReactionDegree, reactionDegreeResult.propellantsAvgReactionDegree, reactionDegreeResult.propellantsStandardReactionDegree,
													reactionDegreeResult.outheatMaxReactionDegree, reactionDegreeResult.outheatMinReactionDegree, reactionDegreeResult.outheatAvgReactionDegree, reactionDegreeResult.outheatStandardReactionDegree,
													reactionDegreeResult.insulatingheatMaxReactionDegree, reactionDegreeResult.insulatingheatMinReactionDegree, reactionDegreeResult.insulatingheatAvgReactionDegree, reactionDegreeResult.insulatingheatStandardReactionDegree);
												
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

												auto reactionDegreeResult = ModelDataManager::GetInstance()->GetJetImpactReactionDegreeResult();
												gfParent->GetJetImpactReactionDegreeResultWidget()->updateData(reactionDegreeResult.metalsMaxReactionDegree, reactionDegreeResult.metalsMinReactionDegree, reactionDegreeResult.metalsAvgReactionDegree, reactionDegreeResult.metalsStandardReactionDegree,
													reactionDegreeResult.propellantsMaxReactionDegree, reactionDegreeResult.propellantsMinReactionDegree, reactionDegreeResult.propellantsAvgReactionDegree, reactionDegreeResult.propellantsStandardReactionDegree,
													reactionDegreeResult.outheatMaxReactionDegree, reactionDegreeResult.outheatMinReactionDegree, reactionDegreeResult.outheatAvgReactionDegree, reactionDegreeResult.outheatStandardReactionDegree,
													reactionDegreeResult.insulatingheatMaxReactionDegree, reactionDegreeResult.insulatingheatMinReactionDegree, reactionDegreeResult.insulatingheatAvgReactionDegree, reactionDegreeResult.insulatingheatStandardReactionDegree);

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

												auto reactionDegreeResult = ModelDataManager::GetInstance()->GetFragmentationImpactReactionDegreeResult();
												gfParent->GetFragmentationImpactReactionDegreeResultWidget()->updateData(reactionDegreeResult.metalsMaxReactionDegree, reactionDegreeResult.metalsMinReactionDegree, reactionDegreeResult.metalsAvgReactionDegree, reactionDegreeResult.metalsStandardReactionDegree,
													reactionDegreeResult.propellantsMaxReactionDegree, reactionDegreeResult.propellantsMinReactionDegree, reactionDegreeResult.propellantsAvgReactionDegree, reactionDegreeResult.propellantsStandardReactionDegree,
													reactionDegreeResult.outheatMaxReactionDegree, reactionDegreeResult.outheatMinReactionDegree, reactionDegreeResult.outheatAvgReactionDegree, reactionDegreeResult.outheatStandardReactionDegree,
													reactionDegreeResult.insulatingheatMaxReactionDegree, reactionDegreeResult.insulatingheatMinReactionDegree, reactionDegreeResult.insulatingheatAvgReactionDegree, reactionDegreeResult.insulatingheatStandardReactionDegree);


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

												auto reactionDegreeResult = ModelDataManager::GetInstance()->GetExplosiveBlastReactionDegreeResult();
												gfParent->GetExplosiveBlastReactionDegreeResultWidget()->updateData(reactionDegreeResult.metalsMaxReactionDegree, reactionDegreeResult.metalsMinReactionDegree, reactionDegreeResult.metalsAvgReactionDegree, reactionDegreeResult.metalsStandardReactionDegree,
													reactionDegreeResult.propellantsMaxReactionDegree, reactionDegreeResult.propellantsMinReactionDegree, reactionDegreeResult.propellantsAvgReactionDegree, reactionDegreeResult.propellantsStandardReactionDegree,
													reactionDegreeResult.outheatMaxReactionDegree, reactionDegreeResult.outheatMinReactionDegree, reactionDegreeResult.outheatAvgReactionDegree, reactionDegreeResult.outheatStandardReactionDegree,
													reactionDegreeResult.insulatingheatMaxReactionDegree, reactionDegreeResult.insulatingheatMinReactionDegree, reactionDegreeResult.insulatingheatAvgReactionDegree, reactionDegreeResult.insulatingheatStandardReactionDegree);


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

												auto reactionDegreeResult = ModelDataManager::GetInstance()->GetSacrificeExplosionReactionDegreeResult();
												gfParent->GetSacrificeExplosionReactionDegreeResultWidget()->updateData(reactionDegreeResult.metalsMaxReactionDegree, reactionDegreeResult.metalsMinReactionDegree, reactionDegreeResult.metalsAvgReactionDegree, reactionDegreeResult.metalsStandardReactionDegree,
													reactionDegreeResult.propellantsMaxReactionDegree, reactionDegreeResult.propellantsMinReactionDegree, reactionDegreeResult.propellantsAvgReactionDegree, reactionDegreeResult.propellantsStandardReactionDegree,
													reactionDegreeResult.outheatMaxReactionDegree, reactionDegreeResult.outheatMinReactionDegree, reactionDegreeResult.outheatAvgReactionDegree, reactionDegreeResult.outheatStandardReactionDegree,
													reactionDegreeResult.insulatingheatMaxReactionDegree, reactionDegreeResult.insulatingheatMinReactionDegree, reactionDegreeResult.insulatingheatAvgReactionDegree, reactionDegreeResult.insulatingheatStandardReactionDegree);


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

							updataIcon();
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
}

void GFTreeModelWidget::exportWord(const QString& directory, QTreeWidgetItem* item)
{
	QString content = calculateParamAnaly();
	UserInfo userinfo = ModelDataManager::GetInstance()->GetUserInfo();
	auto calculationinfo = ModelDataManager::GetInstance()->GetCalculationPropertyInfo();
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

			SteelPropertyWidget* m_steelPropertyWidget = gfParent->GetSteelPropertyWidget();
			QTableWidget* m_steelTableWid = m_databasePropertyWidget->GetQTableWidget();

			PropellantPropertyWidget* m_propellantPropertyWidget = gfParent->GetPropellantPropertyWidget();
			QTableWidget* m_propellantTableWid = m_databasePropertyWidget->GetQTableWidget();

			InsulatingheatPropertyWidget* m_insulatingheatPropertyWidget = gfParent->GetInsulatingheatPropertyWidget();
			QTableWidget* m_insulatingheatTableWid = m_databasePropertyWidget->GetQTableWidget();

			OutheatPropertyWidget* m_outheatPropertyWidget = gfParent->GetOutheatPropertyWidget();
			QTableWidget* m_outheatTableWid = m_databasePropertyWidget->GetQTableWidget();

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
							m_overpressureResultWidge,
							m_steelPropertyWidget,
							m_propellantPropertyWidget,
							m_insulatingheatPropertyWidget,
							m_outheatPropertyWidget);
						
						// 跌落输入数据
						data.insert("测试项目", m_fallTableWid->item(1, 2)->text());
						data.insert("跌落高度", m_fallTableWid->item(2, 2)->text());
						data.insert("跌落姿态", m_fallTableWid->item(3, 2)->text());
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
						data.insert("拟合优度", "92.98");
						data.insert("计算模型公式", calculationinfo.fallStressCalculation.at(0));
						data.insert("结论", m_fallTableWid->item(8, 2)->text() + "，" + m_fallTableWid->item(9, 2)->text() + "，" + m_fallTableWid->item(10, 2)->text());
						data.insert("内容摘要", content);

						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("壳体应力云图", QDir(workdir + "/template/fall/ShellStress.png").absolutePath());
						imagePaths.insert("推进剂应力云图", QDir(workdir + "/template/fall/PropellantStress.png").absolutePath());
						imagePaths.insert("壳体应变云图", QDir(workdir + "/template/fall/ShellStrain.png").absolutePath());
						imagePaths.insert("推进剂应变云图", QDir(workdir + "/template/fall/PropellantStrain.png").absolutePath());
						imagePaths.insert("壳体温度云图", QDir(workdir + "/template/fall/ShellTemperature.png").absolutePath());
						imagePaths.insert("推进剂温度云图", QDir(workdir + "/template/fall/PropellantTemperature.png").absolutePath());
						imagePaths.insert("壳体超压云图", QDir(workdir + "/template/fall/ShellOverpressure.png").absolutePath());
						imagePaths.insert("推进剂超压云图", QDir(workdir + "/template/fall/PropellantOverpressure.png").absolutePath());
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
							nullptr,
							m_steelPropertyWidget,
							m_propellantPropertyWidget,
							m_insulatingheatPropertyWidget,
							m_outheatPropertyWidget);

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

						data.insert("拟合优度", "99.88");
						data.insert("计算模型公式", calculationinfo.fastCombustionCalculation.at(0));
						data.insert("结论", m_fastCombustionTableWid->item(10, 2)->text());
						data.insert("内容摘要", content);

						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("壳体温度云图", QDir(workdir + "/template/fastCombustion/ShellTemperature.png").absolutePath());
						imagePaths.insert("推进剂温度云图", QDir(workdir + "/template/fastCombustion/PropellantTemperature.png").absolutePath());
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
							nullptr,
							m_steelPropertyWidget,
							m_propellantPropertyWidget,
							m_insulatingheatPropertyWidget,
							m_outheatPropertyWidget);

						// 跌落输入数据
						data.insert("测试项目", m_slowCombustionTableWid->item(1, 2)->text());
						data.insert("加热类型", m_slowCombustionTableWid->item(2, 2)->text());
						data.insert("弹药位置", m_slowCombustionTableWid->item(3, 2)->text());
						data.insert("温度传感器数量", m_slowCombustionTableWid->item(4, 2)->text());
						data.insert("冲击波超压传感器数量", m_slowCombustionTableWid->item(5, 2)->text());
						data.insert("风速", m_slowCombustionTableWid->item(6, 2)->text());
						data.insert("平衡时刻", m_slowCombustionTableWid->item(7, 2)->text());
						data.insert("烘箱升温速率", m_slowCombustionTableWid->item(8, 2)->text());
						data.insert("烘箱终止温度", m_slowCombustionTableWid->item(9, 2)->text());

						data.insert("拟合优度", "98.92");
						data.insert("计算模型公式", calculationinfo.slowCombustionCalculation.at(0));
						data.insert("结论", m_slowCombustionTableWid->item(10, 2)->text());
						data.insert("内容摘要", content);

						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("壳体温度云图", QDir(workdir + "/template/slowCombustion/ShellTemperature.png").absolutePath());
						imagePaths.insert("推进剂温度云图", QDir(workdir + "/template/slowCombustion/PropellantTemperature.png").absolutePath());
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
							m_overpressureResultWidge,
							m_steelPropertyWidget,
							m_propellantPropertyWidget,
							m_insulatingheatPropertyWidget,
							m_outheatPropertyWidget);

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

						data.insert("拟合优度", "99.16");
						data.insert("计算模型公式", calculationinfo.shootStressCalculation.at(0));
						data.insert("结论", m_shootTableWid->item(10, 2)->text() + "，" + m_shootTableWid->item(11, 2)->text() + "，" + m_shootTableWid->item(12, 2)->text());
						data.insert("内容摘要", content);

						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("壳体应力云图", QDir(workdir + "/template/shoot/ShellStress.png").absolutePath());
						imagePaths.insert("推进剂应力云图", QDir(workdir + "/template/shoot/PropellantStress.png").absolutePath());
						imagePaths.insert("壳体应变云图", QDir(workdir + "/template/shoot/ShellStrain.png").absolutePath());
						imagePaths.insert("推进剂应变云图", QDir(workdir + "/template/shoot/PropellantStrain.png").absolutePath());
						imagePaths.insert("壳体温度云图", QDir(workdir + "/template/shoot/ShellTemperature.png").absolutePath());
						imagePaths.insert("推进剂温度云图", QDir(workdir + "/template/shoot/PropellantTemperature.png").absolutePath());
						imagePaths.insert("壳体超压云图", QDir(workdir + "/template/shoot/ShellOverpressure.png").absolutePath());
						imagePaths.insert("推进剂超压云图", QDir(workdir + "/template/shoot/PropellantOverpressure.png").absolutePath());
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
							m_overpressureResultWidge,
							m_steelPropertyWidget,
							m_propellantPropertyWidget,
							m_insulatingheatPropertyWidget,
							m_outheatPropertyWidget);

						// 跌落输入数据
						data.insert("测试项目", m_jetImpactTableWid->item(1, 2)->text());
						data.insert("聚能装药口径", m_jetImpactTableWid->item(2, 2)->text());
						data.insert("炸高", m_jetImpactTableWid->item(3, 2)->text());
						data.insert("冲击点角度", m_jetImpactTableWid->item(4, 2)->text());
						data.insert("温度传感器数量", m_jetImpactTableWid->item(5, 2)->text());
						data.insert("超压传感器数量", m_jetImpactTableWid->item(6, 2)->text());
						data.insert("风速", m_jetImpactTableWid->item(7, 2)->text());

						data.insert("拟合优度", "98.53");
						data.insert("计算模型公式", calculationinfo.jetImpactStressCalculation.at(0));
						data.insert("结论", m_jetImpactTableWid->item(8, 2)->text() + "，" + m_jetImpactTableWid->item(9, 2)->text() + "，" + m_jetImpactTableWid->item(10, 2)->text());
						data.insert("内容摘要", content);


						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("壳体应力云图", QDir(workdir + "/template/jetImpact/ShellStress.png").absolutePath());
						imagePaths.insert("推进剂应力云图", QDir(workdir + "/template/jetImpact/PropellantStress.png").absolutePath());
						imagePaths.insert("壳体应变云图", QDir(workdir + "/template/jetImpact/ShellStrain.png").absolutePath());
						imagePaths.insert("推进剂应变云图", QDir(workdir + "/template/jetImpact/PropellantStrain.png").absolutePath());
						imagePaths.insert("壳体温度云图", QDir(workdir + "/template/jetImpact/ShellTemperature.png").absolutePath());
						imagePaths.insert("推进剂温度云图", QDir(workdir + "/template/jetImpact/PropellantTemperature.png").absolutePath());
						imagePaths.insert("壳体超压云图", QDir(workdir + "/template/jetImpact/ShellOverpressure.png").absolutePath());
						imagePaths.insert("推进剂超压云图", QDir(workdir + "/template/jetImpact/PropellantOverpressure.png").absolutePath());
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
							m_overpressureResultWidge,
							m_steelPropertyWidget,
							m_propellantPropertyWidget,
							m_insulatingheatPropertyWidget,
							m_outheatPropertyWidget);

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

						data.insert("拟合优度", "99.72");
						data.insert("计算模型公式", calculationinfo.fragmentationImpactStressCalculation.at(0));
						data.insert("结论", m_fragmentationImpactTableWid->item(11, 2)->text() + "，" + m_fragmentationImpactTableWid->item(12, 2)->text() + "，" + m_fragmentationImpactTableWid->item(13, 2)->text());
						data.insert("内容摘要", content);

						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("壳体应力云图", QDir(workdir + "/template/fragmentationImpact/ShellStress.png").absolutePath());
						imagePaths.insert("推进剂应力云图", QDir(workdir + "/template/fragmentationImpact/PropellantStress.png").absolutePath());
						imagePaths.insert("壳体应变云图", QDir(workdir + "/template/fragmentationImpact/ShellStrain.png").absolutePath());
						imagePaths.insert("推进剂应变云图", QDir(workdir + "/template/fragmentationImpact/PropellantStrain.png").absolutePath());
						imagePaths.insert("壳体温度云图", QDir(workdir + "/template/fragmentationImpact/ShellTemperature.png").absolutePath());
						imagePaths.insert("推进剂温度云图", QDir(workdir + "/template/fragmentationImpact/PropellantTemperature.png").absolutePath());
						imagePaths.insert("壳体超压云图", QDir(workdir + "/template/fragmentationImpact/ShellOverpressure.png").absolutePath());
						imagePaths.insert("推进剂超压云图", QDir(workdir + "/template/fragmentationImpact/PropellantOverpressure.png").absolutePath());
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
							m_overpressureResultWidge,
							m_steelPropertyWidget,
							m_propellantPropertyWidget,
							m_insulatingheatPropertyWidget,
							m_outheatPropertyWidget);

						// 跌落输入数据
						data.insert("测试项目", m_explosiveBlastTableWid->item(1, 2)->text());
						data.insert("TNT当量", m_explosiveBlastTableWid->item(2, 2)->text());
						data.insert("入射角度", m_explosiveBlastTableWid->item(3, 2)->text());
						data.insert("温度传感器数量", m_explosiveBlastTableWid->item(4, 2)->text());
						data.insert("超压传感器数量", m_explosiveBlastTableWid->item(5, 2)->text());
						data.insert("风速", m_explosiveBlastTableWid->item(6, 2)->text());
						
						data.insert("拟合优度", "98.87");
						data.insert("计算模型公式", calculationinfo.explosiveBlastStressCalculation.at(0));
						data.insert("结论", m_explosiveBlastTableWid->item(7, 2)->text() + "，" + m_explosiveBlastTableWid->item(8, 2)->text() + "，" + m_explosiveBlastTableWid->item(9, 2)->text());
						data.insert("内容摘要", content);

						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("壳体应力云图", QDir(workdir + "/template/explosiveBlast/ShellStress.png").absolutePath());
						imagePaths.insert("推进剂应力云图", QDir(workdir + "/template/explosiveBlast/PropellantStress.png").absolutePath());
						imagePaths.insert("壳体应变云图", QDir(workdir + "/template/explosiveBlast/ShellStrain.png").absolutePath());
						imagePaths.insert("推进剂应变云图", QDir(workdir + "/template/explosiveBlast/PropellantStrain.png").absolutePath());
						imagePaths.insert("壳体温度云图", QDir(workdir + "/template/explosiveBlast/ShellTemperature.png").absolutePath());
						imagePaths.insert("推进剂温度云图", QDir(workdir + "/template/explosiveBlast/PropellantTemperature.png").absolutePath());
						imagePaths.insert("壳体超压云图", QDir(workdir + "/template/explosiveBlast/ShellOverpressure.png").absolutePath());
						imagePaths.insert("推进剂超压云图", QDir(workdir + "/template/explosiveBlast/PropellantOverpressure.png").absolutePath());
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
							m_overpressureResultWidge,
							m_steelPropertyWidget,
							m_propellantPropertyWidget,
							m_insulatingheatPropertyWidget,
							m_outheatPropertyWidget);

						// 跌落输入数据
						data.insert("测试项目", m_sacrificeExplosionTableWid->item(1, 2)->text());
						data.insert("殉爆距离", m_sacrificeExplosionTableWid->item(2, 2)->text());
						data.insert("模拟弹药数量", m_sacrificeExplosionTableWid->item(3, 2)->text());
						data.insert("被发弹数量", m_sacrificeExplosionTableWid->item(4, 2)->text());
						data.insert("温度传感器数量", m_sacrificeExplosionTableWid->item(5, 2)->text());
						data.insert("超压传感器数量", m_sacrificeExplosionTableWid->item(6, 2)->text());
						data.insert("风速", m_sacrificeExplosionTableWid->item(7, 2)->text());

						data.insert("拟合优度", "97.94");
						data.insert("计算模型公式", calculationinfo.sacrificeExplosionStressCalculation.at(0));
						data.insert("结论", m_sacrificeExplosionTableWid->item(7, 2)->text() + "，" + m_sacrificeExplosionTableWid->item(8, 2)->text() + "，" + m_sacrificeExplosionTableWid->item(9, 2)->text());
						data.insert("内容摘要", content);

						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir(workdir + "/template/main.png").absolutePath());
						imagePaths.insert("壳体应力云图", QDir(workdir + "/template/sacrificeExplosio/ShellStress.png").absolutePath());
						imagePaths.insert("推进剂应力云图", QDir(workdir + "/template/sacrificeExplosio/PropellantStress.png").absolutePath());
						imagePaths.insert("壳体应变云图", QDir(workdir + "/template/sacrificeExplosio/ShellStrain.png").absolutePath());
						imagePaths.insert("推进剂应变云图", QDir(workdir + "/template/sacrificeExplosio/PropellantStrain.png").absolutePath());
						imagePaths.insert("壳体温度云图", QDir(workdir + "/template/sacrificeExplosio/ShellTemperature.png").absolutePath());
						imagePaths.insert("推进剂温度云图", QDir(workdir + "/template/sacrificeExplosio/PropellantTemperature.png").absolutePath());
						imagePaths.insert("壳体超压云图", QDir(workdir + "/template/sacrificeExplosio/ShellOverpressure.png").absolutePath());
						imagePaths.insert("推进剂超压云图", QDir(workdir + "/template/sacrificeExplosio/PropellantOverpressure.png").absolutePath());
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
	OverpressureResultWidget* overpressureResultWidge,
	SteelPropertyWidget* steelPropertyWidget,
	PropellantPropertyWidget* propellantPropertyWidget,
	InsulatingheatPropertyWidget* insulatingheatPropertyWidget,
	OutheatPropertyWidget* outheatPropertyWidget)
{
	QTableWidget* m_projectTableWid = projectPropertyWidge->GetQTableWidget();

	QTableWidget* m_geomTableWid = geomPropertyWidget->GetQTableWidget();

	QTableWidget* m_materialTableWid = materialPropertyWidget->GetQTableWidget();

	QTableWidget* m_databaseTableWid = databasePropertyWidget->GetQTableWidget();

	QTableWidget* m_steelTableWid = steelPropertyWidget->GetQTableWidget();
	QTableWidget* m_propellantTableWid = propellantPropertyWidget->GetQTableWidget();
	QTableWidget* m_insulatingheatTableWid = insulatingheatPropertyWidget->GetQTableWidget();
	QTableWidget* m_outheatTableWid = outheatPropertyWidget->GetQTableWidget();
	
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
	data.insert("直径", m_geomTableWid->item(3, 2)->text());
	data.insert("总长", m_geomTableWid->item(4, 2)->text());


	// 材料数据
	data.insert("推进剂-材料牌号", m_propellantTableWid->item(1, 2)->text());
	data.insert("推进剂-类别", m_propellantTableWid->item(2, 2)->text());
	data.insert("推进剂-密度", m_propellantTableWid->item(3, 2)->text());
	data.insert("推进剂-热膨胀系数", m_propellantTableWid->item(4, 2)->text());
	data.insert("推进剂-弹性模量", m_propellantTableWid->item(5, 2)->text());
	data.insert("推进剂-切线模量", m_propellantTableWid->item(6, 2)->text());
	data.insert("推进剂-泊松比", m_propellantTableWid->item(7, 2)->text());
	data.insert("推进剂-发火温度", m_propellantTableWid->item(8, 2)->text());
	data.insert("推进剂-发火超压", m_propellantTableWid->item(9, 2)->text());
	data.insert("推进剂-发火概率摩擦感度", m_propellantTableWid->item(10, 2)->text());
	data.insert("推进剂-热导率", m_propellantTableWid->item(11, 2)->text());
	data.insert("推进剂-比热容", m_propellantTableWid->item(12, 2)->text());
	data.insert("l", m_propellantTableWid->item(13, 2)->text());
	data.insert("a", m_propellantTableWid->item(14, 2)->text());
	data.insert("b", m_propellantTableWid->item(15, 2)->text());
	data.insert("c", m_propellantTableWid->item(16, 2)->text());
	data.insert("d", m_propellantTableWid->item(17, 2)->text());
	data.insert("G1", m_propellantTableWid->item(18, 2)->text());
	data.insert("e", m_propellantTableWid->item(19, 2)->text());
	data.insert("g", m_propellantTableWid->item(20, 2)->text());
	data.insert("x", m_propellantTableWid->item(21, 2)->text());
	data.insert("y", m_propellantTableWid->item(22, 2)->text());
	data.insert("z", m_propellantTableWid->item(23, 2)->text());
	data.insert("G2", m_propellantTableWid->item(24, 2)->text());
	data.insert("感度", m_propellantTableWid->item(25, 2)->text());
	double G1 = m_propellantTableWid->item(18, 2)->text().toDouble();
	double G2 = m_propellantTableWid->item(24, 2)->text().toDouble();
	auto figmax = G1 > G2 ? m_propellantTableWid->item(18, 2)->text() : m_propellantTableWid->item(24, 2)->text();
	data.insert("Figmax", figmax);



	data.insert("壳体-材料牌号", m_steelTableWid->item(1, 2)->text());
	data.insert("壳体-密度", m_steelTableWid->item(2, 2)->text());
	data.insert("壳体-热膨胀系数", m_steelTableWid->item(3, 2)->text());
	data.insert("壳体-弹性模量", m_steelTableWid->item(4, 2)->text());
	data.insert("壳体-切线模量", m_steelTableWid->item(5, 2)->text());
	data.insert("壳体-泊松比", m_steelTableWid->item(6, 2)->text());
	data.insert("壳体-屈服强度", m_steelTableWid->item(7, 2)->text());
	data.insert("壳体-抗拉强度", m_steelTableWid->item(8, 2)->text());
	data.insert("壳体-热导率", m_steelTableWid->item(9, 2)->text());
	data.insert("壳体-比热容", m_steelTableWid->item(10, 2)->text());

	data.insert("绝热层-材料牌号", m_insulatingheatTableWid->item(1, 2)->text());
	data.insert("绝热层-密度", m_insulatingheatTableWid->item(2, 2)->text());
	data.insert("绝热层-热膨胀系数", m_insulatingheatTableWid->item(3, 2)->text());
	data.insert("绝热层-弹性模量", m_insulatingheatTableWid->item(4, 2)->text());
	data.insert("绝热层-切线模量", m_insulatingheatTableWid->item(5, 2)->text());
	data.insert("绝热层-泊松比", m_insulatingheatTableWid->item(6, 2)->text());
	data.insert("绝热层-屈服强度", m_insulatingheatTableWid->item(7, 2)->text());
	data.insert("绝热层-抗拉强度", m_insulatingheatTableWid->item(8, 2)->text());
	data.insert("绝热层-热导率", m_insulatingheatTableWid->item(9, 2)->text());
	data.insert("绝热层-比热容", m_insulatingheatTableWid->item(10, 2)->text());

	data.insert("喷管-材料牌号", m_outheatTableWid->item(1, 2)->text());
	data.insert("喷管-密度", m_outheatTableWid->item(2, 2)->text());
	data.insert("喷管-热膨胀系数", m_outheatTableWid->item(3, 2)->text());
	data.insert("喷管-弹性模量", m_outheatTableWid->item(4, 2)->text());
	data.insert("喷管-切线模量", m_outheatTableWid->item(5, 2)->text());
	data.insert("喷管-泊松比", m_outheatTableWid->item(6, 2)->text());
	data.insert("喷管-屈服强度", m_outheatTableWid->item(7, 2)->text());
	data.insert("喷管-抗拉强度", m_outheatTableWid->item(8, 2)->text());
	data.insert("喷管-热导率", m_outheatTableWid->item(9, 2)->text());
	data.insert("喷管-比热容", m_outheatTableWid->item(10, 2)->text());

	
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

// 获取安全性分析评估结果
QString GFTreeModelWidget::calculateParamAnaly()
{
	QString result = "";
	QWidget* parent = parentWidget();
	while (parent)
	{
		mainWidget* paParent = dynamic_cast<mainWidget*>(parent);
		if (paParent)
		{
			QTabWidget* tabWid = paParent->getTabWidget();
			ParamAnalyWidget* analysisEvaluationWid = dynamic_cast<ParamAnalyWidget*>(tabWid->widget(3));
			ParamAnalyTreeWidget* paramAnalyTreeWidget = analysisEvaluationWid->getParamAnalyTreeWidget();
			vector<QString> resultStr = paramAnalyTreeWidget->calculateOnly();
			for (const QString& str : resultStr) {
				result = result + str + "\n";
			}
			break;
		}
		else
		{
			parent = parent->parentWidget();
		}
	}
	return result;
}
