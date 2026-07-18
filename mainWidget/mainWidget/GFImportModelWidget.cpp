#pragma execution_character_set("utf-8")
#include "GFImportModelWidget.h"
#include <AIS_Shape.hxx>
#include <AIS_ColorScale.hxx>

#include <STEPControl_Reader.hxx>
#include <Prs3d_LineAspect.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Edge.hxx>
#include <StlAPI_Reader.hxx>
#include <RWStl.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <Quantity_NameOfColor.hxx>
#include <MeshVS_Mesh.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>
#include <V3d_View.hxx>
#include <V3d_TypeOfOrientation.hxx>

#include <QSplitter>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QSplitter>

#include "OccView.h"
#include "GFLogWidget.h"
#include "GFTreeModelWidget.h"
#include "colour_change_algrithm.h"
#include "APISetNodeValue.h"


GFImportModelWidget::GFImportModelWidget(QWidget*parent)
	:QWidget(parent)
{
	m_treeModelWidget = new GFTreeModelWidget();
	m_PropertyStackWidget = new QStackedWidget();
	// 设置m_PropertyStackWidget的背景为白色
	m_PropertyStackWidget->setStyleSheet("background-color: white;");

	// 初始化所有的 PropertyWidget
	m_geomPropertyWidget = new GeomPropertyWidget();
	m_materialPropertyWidget = new MaterialPropertyWidget();
	m_meshPropertyWidget = new MeshPropertyWidget();
	m_settingPropertyWidget = new SettingPropertyWidget();
	m_fallPropertyWidget = new FallPropertyWidget();
	m_fastCombustionPropertyWidget = new FastCombustionPropertyWidget();
	m_slowCombustionPropertyWidget = new SlowCombustionPropertyWidget();
	m_resultsPropertyWidget = new ResultsPropertyWidget();
	m_stressResultWidget = new StressResultWidget();
	m_temperatureResultWidget = new TemperatureResultWidget();
	m_overpressureResultWidge = new OverpressureResultWidget();
	m_steelPropertyWidgett = new SteelPropertyWidget();
	m_propellantPropertyWidget = new PropellantPropertyWidget();
	m_projectPropertyWidge = new ProjectPropertyWidge();
	m_calculationPropertyWidget = new CalculationPropertyWidget();
	m_judgmentPropertyWidget = new JudgmentPropertyWidget();
	m_insulatingheatPropertyWidget = new InsulatingheatPropertyWidget();
	m_outheatPropertyWidget = new OutheatPropertyWidget();
	m_strainResultWidget = new StrainResultWidget();
	m_shootPropertyWidget = new ShootPropertyWidget();
	m_jetImpactPropertyWidget = new JetImpactPropertyWidget();
	m_fragmentationImpactPropertyWidget = new FragmentationImpactPropertyWidget();
	m_explosiveBlastPropertyWidget = new ExplosiveBlastPropertyWidget();
	m_sacrificeExplosionPropertyWidget = new SacrificeExplosionPropertyWidget();
	m_databasePropertyWidget = new DatabasePropertyWidget();
	// 枪击结果
	m_shootStressResultWidget = new StressResultWidget();
	m_shootTemperatureResultWidget = new TemperatureResultWidget();
	m_shootOverpressureResultWidge = new OverpressureResultWidget();
	m_shootStrainResultWidget = new StrainResultWidget();
	// 破片结果
	m_fragmentationImpactStressResultWidget = new StressResultWidget();
	m_fragmentationImpactTemperatureResultWidget = new TemperatureResultWidget();
	m_fragmentationImpactOverpressureResultWidge = new OverpressureResultWidget();
	m_fragmentationImpactStrainResultWidget = new StrainResultWidget();
	
	// 快烤结果
	m_fastCombustionTemperatureResultWidget = new TemperatureResultWidget();

	// 慢烤结果
	m_slowCombustionTemperatureResultWidget = new TemperatureResultWidget();

	// 射流冲击结果
	m_jetImpactStressResultWidget = new StressResultWidget();
	m_jetImpactTemperatureResultWidget = new TemperatureResultWidget();
	m_jetImpactOverpressureResultWidge = new OverpressureResultWidget();
	m_jetImpactStrainResultWidget = new StrainResultWidget();

	// 爆炸冲击波结果
	m_explosiveBlastStressResultWidget = new StressResultWidget();
	m_explosiveBlastTemperatureResultWidget = new TemperatureResultWidget();
	m_explosiveBlastOverpressureResultWidge = new OverpressureResultWidget();
	m_explosiveBlastStrainResultWidget = new StrainResultWidget();

	// 殉爆结果
	m_sacrificeExplosionStressResultWidget = new StressResultWidget();
	m_sacrificeExplosionTemperatureResultWidget = new TemperatureResultWidget();
	m_sacrificeExplosionOverpressureResultWidge = new OverpressureResultWidget();
	m_sacrificeExplosionStrainResultWidget = new StrainResultWidget();


	// 将所有的 PropertyWidget 添加到 QStackedWidget 中
	m_PropertyStackWidget->addWidget(m_geomPropertyWidget);
	m_PropertyStackWidget->addWidget(m_materialPropertyWidget);
	m_PropertyStackWidget->addWidget(m_meshPropertyWidget);
	m_PropertyStackWidget->addWidget(m_settingPropertyWidget);
	m_PropertyStackWidget->addWidget(m_fallPropertyWidget);
	m_PropertyStackWidget->addWidget(m_fastCombustionPropertyWidget);
	m_PropertyStackWidget->addWidget(m_slowCombustionPropertyWidget);
	m_PropertyStackWidget->addWidget(m_resultsPropertyWidget);
	m_PropertyStackWidget->addWidget(m_stressResultWidget);
	m_PropertyStackWidget->addWidget(m_temperatureResultWidget);
	m_PropertyStackWidget->addWidget(m_overpressureResultWidge);
	m_PropertyStackWidget->addWidget(m_steelPropertyWidgett);
	m_PropertyStackWidget->addWidget(m_propellantPropertyWidget);
	m_PropertyStackWidget->addWidget(m_projectPropertyWidge);
	m_PropertyStackWidget->addWidget(m_calculationPropertyWidget);
	m_PropertyStackWidget->addWidget(m_judgmentPropertyWidget);
	m_PropertyStackWidget->addWidget(m_insulatingheatPropertyWidget);
	m_PropertyStackWidget->addWidget(m_outheatPropertyWidget);
	m_PropertyStackWidget->addWidget(m_strainResultWidget);
	m_PropertyStackWidget->addWidget(m_shootPropertyWidget);
	m_PropertyStackWidget->addWidget(m_jetImpactPropertyWidget);
	m_PropertyStackWidget->addWidget(m_fragmentationImpactPropertyWidget);
	m_PropertyStackWidget->addWidget(m_explosiveBlastPropertyWidget);
	m_PropertyStackWidget->addWidget(m_sacrificeExplosionPropertyWidget);
	m_PropertyStackWidget->addWidget(m_databasePropertyWidget);

	m_PropertyStackWidget->addWidget(m_shootStressResultWidget);
	m_PropertyStackWidget->addWidget(m_shootTemperatureResultWidget);
	m_PropertyStackWidget->addWidget(m_shootOverpressureResultWidge);
	m_PropertyStackWidget->addWidget(m_shootStrainResultWidget);
	m_PropertyStackWidget->addWidget(m_fragmentationImpactStressResultWidget);
	m_PropertyStackWidget->addWidget(m_fragmentationImpactTemperatureResultWidget);
	m_PropertyStackWidget->addWidget(m_fragmentationImpactOverpressureResultWidge);
	m_PropertyStackWidget->addWidget(m_fragmentationImpactStrainResultWidget);

	m_PropertyStackWidget->addWidget(m_fastCombustionTemperatureResultWidget);
	m_PropertyStackWidget->addWidget(m_slowCombustionTemperatureResultWidget);

	m_PropertyStackWidget->addWidget(m_jetImpactStressResultWidget);
	m_PropertyStackWidget->addWidget(m_jetImpactTemperatureResultWidget);
	m_PropertyStackWidget->addWidget(m_jetImpactOverpressureResultWidge);
	m_PropertyStackWidget->addWidget(m_jetImpactStrainResultWidget);

	m_PropertyStackWidget->addWidget(m_explosiveBlastStressResultWidget);
	m_PropertyStackWidget->addWidget(m_explosiveBlastTemperatureResultWidget);
	m_PropertyStackWidget->addWidget(m_explosiveBlastOverpressureResultWidge);
	m_PropertyStackWidget->addWidget(m_explosiveBlastStrainResultWidget);

	m_PropertyStackWidget->addWidget(m_sacrificeExplosionStressResultWidget);
	m_PropertyStackWidget->addWidget(m_sacrificeExplosionTemperatureResultWidget);
	m_PropertyStackWidget->addWidget(m_sacrificeExplosionOverpressureResultWidge);
	m_PropertyStackWidget->addWidget(m_sacrificeExplosionStrainResultWidget);


	m_OccView = new OccView(this);
	m_LogWidget = new GFLogWidget();


	// ------ 左侧垂直分割器（树结构与属性表） ------
	auto leftSplitter = new QSplitter(Qt::Vertical);
	leftSplitter->addWidget(m_treeModelWidget);
	leftSplitter->addWidget(m_PropertyStackWidget);
	leftSplitter->setStretchFactor(0, 3);
	leftSplitter->setStretchFactor(1, 1);
	leftSplitter->setContentsMargins(0, 0, 0, 0);
	// 设置分割器的Handle宽度为0（消除视觉间隙）
	leftSplitter->setHandleWidth(1);


	// ------ 右侧垂直分割器（树结构与属性表） ------
	auto rightSplitter = new QSplitter(Qt::Vertical);
	rightSplitter->addWidget(m_OccView);
	rightSplitter->addWidget(m_LogWidget);
	rightSplitter->setStretchFactor(0, 21);
	rightSplitter->setStretchFactor(1, 1);
	rightSplitter->setContentsMargins(0, 0, 0, 0);
	// 设置分割器的Handle宽度为0（消除视觉间隙）
	rightSplitter->setHandleWidth(1);


	// ------ 主水平分割器（左侧与右侧） ------
	auto mainSplitter = new QSplitter(Qt::Horizontal);
	mainSplitter->addWidget(leftSplitter);
	mainSplitter->addWidget(rightSplitter);
	mainSplitter->setContentsMargins(0, 0, 0, 0);
	// 设置分割器的Handle宽度为0（消除视觉间隙）
	mainSplitter->setHandleWidth(1);

	mainSplitter->setStretchFactor(0, 2);
	mainSplitter->setStretchFactor(1, 8);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(mainSplitter);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	// 连接信号和槽
	connect(m_treeModelWidget, &GFTreeModelWidget::itemClicked, this, &GFImportModelWidget::onTreeItemClicked);
}

GFImportModelWidget::~GFImportModelWidget()
{
}


void GFImportModelWidget::onTreeItemClicked(const QString& itemData)
{
	auto occView = GetOccView();
	if (itemData == "Geometry") 
	{
		occView->SetCameraRotationState(true);

		//m_PropertyStackWidget->setCurrentWidget(m_geomPropertyWidget);
		//auto modelInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
		//if (!modelInfo.shape.IsNull())
		//{
		//	Handle(AIS_InteractiveContext) context = occView->getContext();
		//	context->EraseAll(true);
		//	Handle(AIS_Shape) modelPresentation = new AIS_Shape(modelInfo.shape);
		//	context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
		//	context->SetColor(modelPresentation, Quantity_Color(0.0, 1.0, 1.0, Quantity_TOC_RGB), true);
		//	context->Display(modelPresentation, false);
		//	occView->fitAll();
		//}
		m_geomPropertyWidget->UpdataPropertyInfo();
	}
	else if (itemData == "ShellGeometry" || itemData == "PropellantGeometry" || itemData == "HeatInsulatingLayerGeometry" )
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_geomPropertyWidget);
	}
	else if (itemData == "Material") 
	{
		occView->SetCameraRotationState(true);

		m_PropertyStackWidget->setCurrentWidget(m_materialPropertyWidget);
	}
	else if (itemData == "Results") 
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_resultsPropertyWidget);
	}
	else if (itemData == "Steel") 
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_steelPropertyWidgett);
	}
	else if (itemData == "Propellant")
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_propellantPropertyWidget);
	}
	else if (itemData == "Judgment")
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_judgmentPropertyWidget);
	}
	else if (itemData == "Calculation") 
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_calculationPropertyWidget);
	}
	else if (itemData == "Project") 
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_projectPropertyWidge);
	}
	else if (itemData == "Insulatingheat") 
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_insulatingheatPropertyWidget);
	}
	else if (itemData == "Outheat") 
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_outheatPropertyWidget);
	}
	else if (itemData == "Database") 
	{
		occView->SetCameraRotationState(true);

		m_PropertyStackWidget->setCurrentWidget(m_databasePropertyWidget);
	}
	else if (itemData == "Mesh")
	{
		occView->SetCameraRotationState(true);

		m_PropertyStackWidget->setCurrentWidget(m_meshPropertyWidget);

		auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
		//if (meshInfo.isChecked)
		//{
		//	Handle(AIS_InteractiveContext) context = occView->getContext();

		//	BRep_Builder builder;
		//	TopoDS_Compound compound;
		//	builder.MakeCompound(compound);
		//	auto tri = meshInfo.triangleStructure;
		//	auto myEdges = tri.GetMyEdge();

		//	auto myNodeCoords = tri.GetmyNodeCoords();

		//	for (const auto& edge : myEdges)
		//	{
		//		Standard_Integer node1ID = edge.first;
		//		Standard_Integer node2ID = edge.second;

		//		Standard_Real x1 = myNodeCoords->Value(node1ID, 1);
		//		Standard_Real y1 = myNodeCoords->Value(node1ID, 2);
		//		Standard_Real z1 = myNodeCoords->Value(node1ID, 3);

		//		Standard_Real x2 = myNodeCoords->Value(node2ID, 1);
		//		Standard_Real y2 = myNodeCoords->Value(node2ID, 2);
		//		Standard_Real z2 = myNodeCoords->Value(node2ID, 3);

		//		gp_Pnt p1(x1, y1, z1);
		//		gp_Pnt p2(x2, y2, z2);

		//		TopoDS_Vertex v1 = BRepBuilderAPI_MakeVertex(p1);
		//		TopoDS_Vertex v2 = BRepBuilderAPI_MakeVertex(p2);

		//		TopoDS_Edge edgeShape = BRepBuilderAPI_MakeEdge(v1, v2);

		//		builder.Add(compound, edgeShape);
		//	}
		//	Handle(AIS_Shape) aisCompound = new AIS_Shape(compound);
		//	context->EraseAll(true);
		//	context->Display(aisCompound, Standard_True);
		//}

		m_meshPropertyWidget->UpdataPropertyInfo();
	}
	else if (itemData == "ShellMesh" || itemData == "PropellantMesh" || itemData == "HeatInsulatingLayerMesh" )
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_meshPropertyWidget);
	}
	else if (itemData == "Analysis") 
	{
		occView->SetCameraRotationState(true);

		m_PropertyStackWidget->setCurrentWidget(m_settingPropertyWidget);

		//auto modelInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
		//if (!modelInfo.shape.IsNull())
		//{
		//	Handle(AIS_InteractiveContext) context = occView->getContext();
		//	context->EraseAll(true);
		//	Handle(AIS_Shape) modelPresentation = new AIS_Shape(modelInfo.shape);
		//	context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
		//	context->SetColor(modelPresentation, Quantity_Color(0.0, 1.0, 1.0, Quantity_TOC_RGB), true);
		//	context->Display(modelPresentation, false);
		//	occView->fitAll();
		//}
	}
	//跌落
	else if (itemData == "FallAnalysis")
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_fallPropertyWidget);

		/*auto modelInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
		if (!modelInfo.shape.IsNull())
		{
			Handle(AIS_InteractiveContext) context = occView->getContext();
			context->EraseAll(true);
			Handle(AIS_Shape) modelPresentation = new AIS_Shape(modelInfo.shape);
			context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
			context->SetColor(modelPresentation, Quantity_NOC_CYAN, true);
			context->Display(modelPresentation, false);
			occView->fitAll();
		}*/
	}
	else if (itemData == "StressResult")
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_stressResultWidget);

		//Handle(AIS_InteractiveContext) context = occView->getContext();
		//Handle(V3d_View) view = occView->getView();
		//view->SetProj(V3d_Zneg);
		//std::vector<double> nodeValues;
		//APISetNodeValue::SetShellFallStressNephogram(occView, nodeValues);

		//auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
		//auto max_value = fallAnalysisResultInfo.stressMaxValue;
		//auto min_value = fallAnalysisResultInfo.stressMinValue;


		//// 颜色条显示（与原逻辑一致）
		//TCollection_ExtendedString tostr("跌落试验\n应力分析\n单位:MPa", true);
		//Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		//aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		//aColorScale->SetSize(100, 400);
		//aColorScale->SetRange(min_value, max_value);
		//aColorScale->SetNumberOfIntervals(9);
		//aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		//aColorScale->SetTextHeight(14);
		//aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		//aColorScale->SetTitle(tostr);
		//aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		//aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		//aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		//Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		//context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		//context->SetDisplayMode(aColorScale, 1, Standard_False);
		//context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "FallStressShellResult")//跌落应力壳体
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_stressResultWidget);
		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);
		std::vector<double> nodeValues;
		APISetNodeValue::SetShellFallStressNephogram(occView, nodeValues);

		auto fallStressResult = ModelDataManager::GetInstance()->GetFallStressResult();
		auto max_value = fallStressResult.metalsMaxStress;
		auto min_value = fallStressResult.metalsMinStress;

		TCollection_ExtendedString tostr("跌落试验\n应力分析\n单位:MPa", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "FallStressPropellantResult")//跌落应力推进剂
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_stressResultWidget);
		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);
		std::vector<double> nodeValues;
		APISetNodeValue::SetPropellantFallStressNephogram(occView, nodeValues);

		auto fallStressResult = ModelDataManager::GetInstance()->GetFallStressResult();
		auto max_value = fallStressResult.propellantsMaxStress;
		auto min_value = fallStressResult.propellantsMinStress;

		TCollection_ExtendedString tostr("跌落试验\n应力分析\n单位:MPa", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "StrainResult")
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_strainResultWidget);

		//Handle(AIS_InteractiveContext) context = occView->getContext();
		//Handle(V3d_View) view = occView->getView();
		//view->SetProj(V3d_Zneg);
		//std::vector<double> nodeValues;
		////APISetNodeValue::SetFallStrainResult(occView, nodeValues);

		//auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
		//auto max_value = fallAnalysisResultInfo.strainMaxValue;
		//auto min_value = fallAnalysisResultInfo.strainMinValue;


		//// 颜色条显示（与原逻辑一致）
		//TCollection_ExtendedString tostr("跌落试验\n应变分析\n", true);
		//Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		//aColorScale->SetFormat(TCollection_AsciiString("%.6f"));
		//aColorScale->SetSize(100, 400);
		//aColorScale->SetRange(min_value, max_value);
		//aColorScale->SetNumberOfIntervals(9);
		//aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		//aColorScale->SetTextHeight(14);
		//aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		//aColorScale->SetTitle(tostr);
		//aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		//aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		//aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		//Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		//context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		//context->SetDisplayMode(aColorScale, 1, Standard_False);
		//context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "FallStrainShellResult")
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_strainResultWidget);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);
		std::vector<double> nodeValues;
		APISetNodeValue::SetShellFallStrainNephogram(occView, nodeValues);

		auto fallStrainResult = ModelDataManager::GetInstance()->GetFallStrainResult();
		auto max_value = fallStrainResult.metalsMaxStrain;
		auto min_value = fallStrainResult.metalsMinStrain;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("跌落试验\n应变分析\n", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.6f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "FallStrainPropellantResult")
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_strainResultWidget);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);
		std::vector<double> nodeValues;
		APISetNodeValue::SetPropellantFallStrainNephogram(occView, nodeValues);

		auto fallStrainResult = ModelDataManager::GetInstance()->GetFallStrainResult();
		auto max_value = fallStrainResult.propellantsMaxStrain;
		auto min_value = fallStrainResult.propellantsMinStrain;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("跌落试验\n应变分析\n", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.6f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "TemperatureResult")
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_temperatureResultWidget);

		//Handle(AIS_InteractiveContext) context = occView->getContext();
		//Handle(V3d_View) view = occView->getView();
		//view->SetProj(V3d_Zneg);

		//std::vector<double> nodeValues;
		////APISetNodeValue::SetFallTemperatureResult(occView, nodeValues);


		//auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
		//auto max_value = fallAnalysisResultInfo.temperatureMaxValue;
		//auto min_value = fallAnalysisResultInfo.temperatureMinValue;


		//// 颜色条显示（与原逻辑一致）
		//TCollection_ExtendedString tostr("跌落试验\n温度分析\n单位:℃", true);
		//Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		//aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		//aColorScale->SetSize(100, 400);
		//aColorScale->SetRange(min_value, max_value);
		//aColorScale->SetNumberOfIntervals(9);
		//aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		//aColorScale->SetTextHeight(14);
		//aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		//aColorScale->SetTitle(tostr);
		//aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		//aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		//aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		//Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		//context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		//context->SetDisplayMode(aColorScale, 1, Standard_False);
		//context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "FallTemperatureShellResult")
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_temperatureResultWidget);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellFallTempNephogram(occView, nodeValues);

		auto fallTemperatureResult = ModelDataManager::GetInstance()->GetFallTemperatureResult();
		auto max_value = fallTemperatureResult.metalsMaxTemperature;
		auto min_value = fallTemperatureResult.metalsMinTemperature;

		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("跌落试验\n温度分析\n单位:℃", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "FallTemperaturePropellantResult")
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_temperatureResultWidget);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetPropellantFallTempNephogram(occView, nodeValues);

		auto fallTemperatureResult = ModelDataManager::GetInstance()->GetFallTemperatureResult();
		auto max_value = fallTemperatureResult.propellantsMaxTemperature;
		auto min_value = fallTemperatureResult.propellantsMinTemperature;

		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("跌落试验\n温度分析\n单位:℃", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "OverpressureResult")
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_overpressureResultWidge);

		//Handle(AIS_InteractiveContext) context = occView->getContext();
		//Handle(V3d_View) view = occView->getView();
		//view->SetProj(V3d_Zneg);

		//std::vector<double> nodeValues;
		////APISetNodeValue::SetFallOverpressureResult(occView, nodeValues);
		//occView->fitAll();

		//auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
		//auto max_value = fallAnalysisResultInfo.overpressureMaxValue;
		//auto min_value = fallAnalysisResultInfo.overpressureMinValue;


		//// 颜色条显示（与原逻辑一致）
		//TCollection_ExtendedString tostr("跌落试验\n超压分析\n单位:Mpa", true);
		//Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		//aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		//aColorScale->SetSize(100, 400);
		//aColorScale->SetRange(min_value, max_value);
		//aColorScale->SetNumberOfIntervals(9);
		//aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		//aColorScale->SetTextHeight(14);
		//aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		//aColorScale->SetTitle(tostr);
		//aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		//aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		//aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		//Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		//context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		//context->SetDisplayMode(aColorScale, 1, Standard_False);
		//context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "FallOverpressureShellResult")
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_overpressureResultWidge);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellFallPressureNephogram(occView, nodeValues);
		occView->fitAll();

		auto fallOverpressureResult = ModelDataManager::GetInstance()->GetFallOverpressureResult();
		auto max_value = fallOverpressureResult.metalsMaxOverpressure;
		auto min_value = fallOverpressureResult.metalsMinOverpressure;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("跌落试验\n超压分析\n单位:Mpa", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "FallOverpressurePropellantResult")
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_overpressureResultWidge);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetPropellantFallPressureNephogram(occView, nodeValues);
		occView->fitAll();

		auto fallOverpressureResult = ModelDataManager::GetInstance()->GetFallOverpressureResult();
		auto max_value = fallOverpressureResult.propellantsMaxOverpressure;
		auto min_value = fallOverpressureResult.propellantsMinOverpressure;

		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("跌落试验\n超压分析\n单位:Mpa", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	
	//快速烤燃
	else if (itemData == "FastCombustionAnalysis")
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_fastCombustionPropertyWidget);
		//Handle(AIS_InteractiveContext) context = occView->getContext();
		//Handle(V3d_View) view = occView->getView();
		//view->SetProj(V3d_Zneg);

		//std::vector<double> nodeValues;
		//APISetNodeValue::SetFastCombustionTemperatureResult(occView, nodeValues);


		//auto fastCombustionAnalysisResultInfo = ModelDataManager::GetInstance()->GetFastCombustionAnalysisResultInfo();
		//auto max_value = fastCombustionAnalysisResultInfo.temperatureMaxValue;
		//auto min_value = fastCombustionAnalysisResultInfo.temperatureMinValue;


		//// 颜色条显示（与原逻辑一致）
		//TCollection_ExtendedString tostr("快速烤燃\n温度分析\n单位:℃", true);
		//Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		//aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		//aColorScale->SetSize(100, 400);
		//aColorScale->SetRange(min_value, max_value);
		//aColorScale->SetNumberOfIntervals(9);
		//aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		//aColorScale->SetTextHeight(14);
		//aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		//aColorScale->SetTitle(tostr);
		//aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		//aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		//aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		//Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		//context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		//context->SetDisplayMode(aColorScale, 1, Standard_False);
		//context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "fastTemperatureShellResult")
	{
		occView->SetCameraRotationState(false);
		m_PropertyStackWidget->setCurrentWidget(m_fastCombustionTemperatureResultWidget);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellFastCombustionTempNephogram(occView, nodeValues);

		auto fastCombustionTemperatureResult = ModelDataManager::GetInstance()->GetFastCombustionTemperatureResult();
		auto max_value = fastCombustionTemperatureResult.metalsMaxTemperature;
		auto min_value = fastCombustionTemperatureResult.metalsMinTemperature;

		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("快速烤燃\n温度分析\n单位:℃", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "fastTemperaturePropellantResult")
	{
		occView->SetCameraRotationState(false);
		m_PropertyStackWidget->setCurrentWidget(m_fastCombustionTemperatureResultWidget);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetPropellantFastCombustionTempNephogram(occView, nodeValues);

		auto fastCombustionTemperatureResult = ModelDataManager::GetInstance()->GetFastCombustionTemperatureResult();
		auto max_value = fastCombustionTemperatureResult.propellantsMaxTemperature;
		auto min_value = fastCombustionTemperatureResult.propellantsMinTemperature;

		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("快速烤燃\n温度分析\n单位:℃", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	//慢速烤燃
	else if (itemData == "SlowCombustionAnalysis")
	{
		m_PropertyStackWidget->setCurrentWidget(m_slowCombustionPropertyWidget);

		occView->SetCameraRotationState(false);
		//Handle(AIS_InteractiveContext) context = occView->getContext();
		//Handle(V3d_View) view = occView->getView();
		//view->SetProj(V3d_Zneg);

		//std::vector<double> nodeValues;
		//APISetNodeValue::SetSlowCombustionTemperatureResult(occView, nodeValues);

		//auto slowCombustionAnalysisResultInfo = ModelDataManager::GetInstance()->GetSlowCombustionAnalysisResultInfo();
		//auto max_value = slowCombustionAnalysisResultInfo.temperatureMaxValue;
		//auto min_value = slowCombustionAnalysisResultInfo.temperatureMinValue;

		//// 颜色条显示（与原逻辑一致）
		//TCollection_ExtendedString tostr("慢速烤燃\n温度分析\n单位:℃", true);
		//Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		//aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		//aColorScale->SetSize(100, 400);
		//aColorScale->SetRange(min_value, max_value);
		//aColorScale->SetNumberOfIntervals(9);
		//aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		//aColorScale->SetTextHeight(14);
		//aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		//aColorScale->SetTitle(tostr);
		//aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		//aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		//aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		//Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		//context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		//context->SetDisplayMode(aColorScale, 1, Standard_False);
		//context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "slowTemperatureShellResult")
	{
		m_PropertyStackWidget->setCurrentWidget(m_slowCombustionTemperatureResultWidget);

		occView->SetCameraRotationState(false);
		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellSlowCombustionTempNephogram(occView, nodeValues);

		auto slowCombustionTemperatureResult = ModelDataManager::GetInstance()->GetSlowCombustionTemperatureResult();
		auto max_value = slowCombustionTemperatureResult.metalsMaxTemperature;
		auto min_value = slowCombustionTemperatureResult.metalsMinTemperature;

		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("慢速烤燃\n温度分析\n单位:℃", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}	
	else if (itemData == "slowTemperaturePropellantResult")
	{
		m_PropertyStackWidget->setCurrentWidget(m_slowCombustionTemperatureResultWidget);

		occView->SetCameraRotationState(false);
		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetPropellantSlowCombustionTempNephogram(occView, nodeValues);

		auto slowCombustionTemperatureResult = ModelDataManager::GetInstance()->GetSlowCombustionTemperatureResult();
		auto max_value = slowCombustionTemperatureResult.propellantsMaxTemperature;
		auto min_value = slowCombustionTemperatureResult.propellantsMinTemperature;

		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("慢速烤燃\n温度分析\n单位:℃", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	//枪击试验
	else if (itemData == "ShootAnalysis")	
	{
		occView->SetCameraRotationState(false);
		m_PropertyStackWidget->setCurrentWidget(m_shootPropertyWidget);
	}
	else if (itemData == "shootStressShellResult") 
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_shootStressResultWidget);
		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellShootStressResult(occView, nodeValues);

		auto shootStressResult = ModelDataManager::GetInstance()->GetShootStressResult();
		auto max_value = shootStressResult.metalsMaxStress;
		auto min_value = shootStressResult.metalsMinStress;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("枪击试验\n应力分析\n单位:MPa", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "shootStressPropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_shootStressResultWidget);
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantShootStressResult(occView, nodeValues);

	auto shootStressResult = ModelDataManager::GetInstance()->GetShootStressResult();
	auto max_value = shootStressResult.propellantsMaxStress;
	auto min_value = shootStressResult.propellantsMinStress;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("枪击试验\n应力分析\n单位:MPa", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "shootStrainShellResult")  
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_shootStrainResultWidget);
		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellShootStrainResult(occView, nodeValues);

		auto shootStrainResult = ModelDataManager::GetInstance()->GetShootStrainResult();
		auto max_value = shootStrainResult.metalsMaxStrain;
		auto min_value = shootStrainResult.metalsMinStrain;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("枪击试验\n应变分析\n", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.6f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "shootStrainPropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_shootStrainResultWidget);
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantShootStrainResult(occView, nodeValues);

	auto shootStrainResult = ModelDataManager::GetInstance()->GetShootStrainResult();
	auto max_value = shootStrainResult.propellantsMaxStrain;
	auto min_value = shootStrainResult.propellantsMinStrain;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("枪击试验\n应变分析\n", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.6f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "shootTempShellResult") 
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_shootTemperatureResultWidget);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellShootTemperatureResult(occView, nodeValues);

		auto shootTemperatureResult = ModelDataManager::GetInstance()->GetShootTemperatureResult();
		auto max_value = shootTemperatureResult.metalsMaxTemperature;
		auto min_value = shootTemperatureResult.metalsMinTemperature;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("枪击试验\n温度分析\n单位:℃", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "shootTempPropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_shootTemperatureResultWidget);

	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantShootTemperatureResult(occView, nodeValues);

	auto shootTemperatureResult = ModelDataManager::GetInstance()->GetShootTemperatureResult();
	auto max_value = shootTemperatureResult.propellantsMaxTemperature;
	auto min_value = shootTemperatureResult.propellantsMinTemperature;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("枪击试验\n温度分析\n单位:℃", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "shootOverpressureShellResult") 
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_shootOverpressureResultWidge);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellShootOverpressureResult(occView, nodeValues);

		auto shootOverpressureResult = ModelDataManager::GetInstance()->GetShootOverpressureResult();
		auto max_value = shootOverpressureResult.metalsMaxOverpressure;
		auto min_value = shootOverpressureResult.metalsMinOverpressure;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("枪击试验\n超压分析\n单位:MPa", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);

	}
	else if (itemData == "shootOverpressurePropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_shootOverpressureResultWidge);

	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantShootOverpressureResult(occView, nodeValues);

	auto shootOverpressureResult = ModelDataManager::GetInstance()->GetShootOverpressureResult();
	auto max_value = shootOverpressureResult.propellantsMaxOverpressure;
	auto min_value = shootOverpressureResult.propellantsMinOverpressure;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("枪击试验\n超压分析\n单位:MPa", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);

	}
	//射流冲击试验
	else if (itemData == "JetImpactAnalysis") 
	{
		occView->SetCameraRotationState(false);
		m_PropertyStackWidget->setCurrentWidget(m_jetImpactPropertyWidget);
	}
	else if (itemData == "jetStressShellResult") 
	{ 
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_jetImpactStressResultWidget);
		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellJetImpactStressResult(occView, nodeValues);

		auto jetImpactStressResult = ModelDataManager::GetInstance()->GetJetImpactStressResult();
		auto max_value = jetImpactStressResult.metalsMaxStress;
		auto min_value = jetImpactStressResult.metalsMinStress;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("射流冲击试验\n应力分析\n单位:MPa", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "jetStressPropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_jetImpactStressResultWidget);
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantJetImpactStressResult(occView, nodeValues);

	auto jetImpactStressResult = ModelDataManager::GetInstance()->GetJetImpactStressResult();
	auto max_value = jetImpactStressResult.propellantsMaxStress;
	auto min_value = jetImpactStressResult.propellantsMinStress;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("射流冲击试验\n应力分析\n单位:MPa", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "jetStrainShellResult") 
	{ 
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_jetImpactStrainResultWidget);
		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellJetImpactStrainResult(occView, nodeValues);

		auto jetImpactStrainResult = ModelDataManager::GetInstance()->GetJetImpactStrainResult();
		auto max_value = jetImpactStrainResult.metalsMaxStrain;
		auto min_value = jetImpactStrainResult.metalsMinStrain;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("射流冲击试验\n应变分析\n", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.6f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "jetStrainPropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_jetImpactStrainResultWidget);
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantJetImpactStrainResult(occView, nodeValues);

	auto jetImpactStrainResult = ModelDataManager::GetInstance()->GetJetImpactStrainResult();
	auto max_value = jetImpactStrainResult.propellantsMaxStrain;
	auto min_value = jetImpactStrainResult.propellantsMinStrain;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("射流冲击试验\n应变分析\n", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.6f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "jetTempShellResult")
	{ 
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_jetImpactTemperatureResultWidget);
		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellJetImpactTemperatureResult(occView, nodeValues);

		auto jetImpactTemperatureResult = ModelDataManager::GetInstance()->GetJetImpactTemperatureResult();
		auto max_value = jetImpactTemperatureResult.metalsMaxTemperature;
		auto min_value = jetImpactTemperatureResult.metalsMinTemperature;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("射流冲击试验\n温度分析\n单位:℃", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "jetTempPropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_jetImpactTemperatureResultWidget);
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantJetImpactTemperatureResult(occView, nodeValues);

	auto jetImpactTemperatureResult = ModelDataManager::GetInstance()->GetJetImpactTemperatureResult();
	auto max_value = jetImpactTemperatureResult.propellantsMaxTemperature;
	auto min_value = jetImpactTemperatureResult.propellantsMinTemperature;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("射流冲击试验\n温度分析\n单位:℃", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "jetOverpressureShellResult") 
	{ 
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_jetImpactOverpressureResultWidge);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellJetImpactOverpressureResult(occView, nodeValues);

		auto jetImpactOverpressureResult = ModelDataManager::GetInstance()->GetJetImpactOverpressureResult();
		auto max_value = jetImpactOverpressureResult.metalsMaxOverpressure;
		auto min_value = jetImpactOverpressureResult.metalsMinOverpressure;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("枪击试验\n超压分析\n单位:MPa", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "jetOverpressurePropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_jetImpactOverpressureResultWidge);

	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantJetImpactOverpressureResult(occView, nodeValues);

	auto jetImpactOverpressureResult = ModelDataManager::GetInstance()->GetJetImpactOverpressureResult();
	auto max_value = jetImpactOverpressureResult.propellantsMaxOverpressure;
	auto min_value = jetImpactOverpressureResult.propellantsMinOverpressure;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("枪击试验\n超压分析\n单位:MPa", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	//破片试验
	else if (itemData == "FragmentationImpactAnalysis")	
	{
		occView->SetCameraRotationState(false);
		m_PropertyStackWidget->setCurrentWidget(m_fragmentationImpactPropertyWidget);
	}
	else if (itemData == "fragmentationStressShellResult") 
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_fragmentationImpactStressResultWidget);


		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellFragmentationStressResult(occView, nodeValues);

		auto fragmentationImpactStressResult = ModelDataManager::GetInstance()->GetFragmentationImpactStressResult();
		auto max_value = fragmentationImpactStressResult.metalsMaxStress;
		auto min_value = fragmentationImpactStressResult.metalsMinStress;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("破片试验\n应力分析\n单位:MPa", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "fragmentationStressPropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_fragmentationImpactStressResultWidget);


	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantFragmentationStressResult(occView, nodeValues);

	auto fragmentationImpactStressResult = ModelDataManager::GetInstance()->GetFragmentationImpactStressResult();
	auto max_value = fragmentationImpactStressResult.propellantsMaxStress;
	auto min_value = fragmentationImpactStressResult.propellantsMinStress;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("破片试验\n应力分析\n单位:MPa", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "fragmentationStrainShellResult")  
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_fragmentationImpactStrainResultWidget);


		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellFragmentationStrainResult(occView, nodeValues);

		auto fragmentationImpactStrainResult = ModelDataManager::GetInstance()->GetFragmentationImpactStrainResult();
		auto max_value = fragmentationImpactStrainResult.metalsMaxStrain;
		auto min_value = fragmentationImpactStrainResult.metalsMinStrain;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("破片试验\n应变分析\n", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.6f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "fragmentationStrainPropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_fragmentationImpactStrainResultWidget);


	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantFragmentationStrainResult(occView, nodeValues);

	auto fragmentationImpactStrainResult = ModelDataManager::GetInstance()->GetFragmentationImpactStrainResult();
	auto max_value = fragmentationImpactStrainResult.propellantsMaxStrain;
	auto min_value = fragmentationImpactStrainResult.propellantsMinStrain;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("破片试验\n应变分析\n", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.6f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "fragmentationTempShellResult") 
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_fragmentationImpactTemperatureResultWidget);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellFragmentationTemperatureResult(occView, nodeValues);


		auto fastCombustionTemperatureResult = ModelDataManager::GetInstance()->GetFastCombustionTemperatureResult();
		auto max_value = fastCombustionTemperatureResult.metalsMaxTemperature;
		auto min_value = fastCombustionTemperatureResult.metalsMinTemperature;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("破片试验\n温度分析\n单位:℃", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "fragmentationTempPropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_fragmentationImpactTemperatureResultWidget);

	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantFragmentationTemperatureResult(occView, nodeValues);


	auto fastCombustionTemperatureResult = ModelDataManager::GetInstance()->GetFastCombustionTemperatureResult();
	auto max_value = fastCombustionTemperatureResult.propellantsMaxTemperature;
	auto min_value = fastCombustionTemperatureResult.propellantsMinTemperature;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("破片试验\n温度分析\n单位:℃", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "fragmentationOverpressureShellResult") 
	{
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_fragmentationImpactOverpressureResultWidge);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellFragmentationOverpressureResult(occView, nodeValues);

		auto fragmentationImpactOverpressureResult = ModelDataManager::GetInstance()->GetFragmentationImpactOverpressureResult();
		auto max_value = fragmentationImpactOverpressureResult.metalsMaxOverpressure;
		auto min_value = fragmentationImpactOverpressureResult.metalsMinOverpressure;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("破片试验\n超压分析\n单位:Mpa", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "fragmentationOverpressurePropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_fragmentationImpactOverpressureResultWidge);

	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantFragmentationOverpressureResult(occView, nodeValues);

	auto fragmentationImpactOverpressureResult = ModelDataManager::GetInstance()->GetFragmentationImpactOverpressureResult();
	auto max_value = fragmentationImpactOverpressureResult.propellantsMaxOverpressure;
	auto min_value = fragmentationImpactOverpressureResult.propellantsMinOverpressure;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("破片试验\n超压分析\n单位:Mpa", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	//爆炸冲击波试验
	else if (itemData == "ExplosiveBlastAnalysis")
	{ 
		occView->SetCameraRotationState(false);
		m_PropertyStackWidget->setCurrentWidget(m_explosiveBlastPropertyWidget);
	}
	else if (itemData == "explosiveStressShellResult") 
	{ 
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_explosiveBlastStressResultWidget);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellExplosiveBlastStressResult(occView, nodeValues);

		auto explosiveBlastStressResult = ModelDataManager::GetInstance()->GetExplosiveBlastStressResult();
		auto max_value = explosiveBlastStressResult.metalsMaxStress;
		auto min_value = explosiveBlastStressResult.metalsMinStress;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("爆炸冲击波试验\n应力分析\n单位:MPa", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "explosiveStressPropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_explosiveBlastStressResultWidget);

	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantExplosiveBlastStressResult(occView, nodeValues);

	auto explosiveBlastStressResult = ModelDataManager::GetInstance()->GetExplosiveBlastStressResult();
	auto max_value = explosiveBlastStressResult.propellantsMaxStress;
	auto min_value = explosiveBlastStressResult.propellantsMinStress;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("爆炸冲击波试验\n应力分析\n单位:MPa", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "explosiveStrainShellResult")
	{ 
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_explosiveBlastStrainResultWidget);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellExplosiveBlastStrainResult(occView, nodeValues);

		auto explosiveBlastStrainResult = ModelDataManager::GetInstance()->GetExplosiveBlastStrainResult();
		auto max_value = explosiveBlastStrainResult.metalsMaxStrain;
		auto min_value = explosiveBlastStrainResult.metalsMinStrain;

		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("爆炸冲击波试验\n应变分析\n", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.6f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "explosiveStrainPropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_explosiveBlastStrainResultWidget);

	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantExplosiveBlastStrainResult(occView, nodeValues);

	auto explosiveBlastStrainResult = ModelDataManager::GetInstance()->GetExplosiveBlastStrainResult();
	auto max_value = explosiveBlastStrainResult.propellantsMaxStrain;
	auto min_value = explosiveBlastStrainResult.propellantsMinStrain;

	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("爆炸冲击波试验\n应变分析\n", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.6f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "explosiveTempShellResult") 
	{ 
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_explosiveBlastTemperatureResultWidget);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellExplosiveBlastTemperatureResult(occView, nodeValues);

		auto explosiveBlastTemperatureResult = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult();
		auto max_value = explosiveBlastTemperatureResult.metalsMaxTemperature;
		auto min_value = explosiveBlastTemperatureResult.metalsMinTemperature;

		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("爆炸冲击波试验\n温度分析\n单位:℃", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "explosiveTempPropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_explosiveBlastTemperatureResultWidget);

	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantExplosiveBlastTemperatureResult(occView, nodeValues);

	auto explosiveBlastTemperatureResult = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult();
	auto max_value = explosiveBlastTemperatureResult.propellantsMaxTemperature;
	auto min_value = explosiveBlastTemperatureResult.propellantsMinTemperature;

	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("爆炸冲击波试验\n温度分析\n单位:℃", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "explosiveOverpressureShellResult") 
	{ 
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_explosiveBlastOverpressureResultWidge);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellExplosiveBlastOverpressureResult(occView, nodeValues);

		auto explosiveBlastOverpressureResult = ModelDataManager::GetInstance()->GetExplosiveBlastOverpressureResult();
		auto max_value = explosiveBlastOverpressureResult.metalsMaxOverpressure;
		auto min_value = explosiveBlastOverpressureResult.metalsMinOverpressure;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("爆炸冲击波试验\n超压分析\n单位:Mpa", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "explosiveOverpressurePropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_explosiveBlastOverpressureResultWidge);

	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantExplosiveBlastOverpressureResult(occView, nodeValues);

	auto explosiveBlastOverpressureResult = ModelDataManager::GetInstance()->GetExplosiveBlastOverpressureResult();
	auto max_value = explosiveBlastOverpressureResult.propellantsMaxOverpressure;
	auto min_value = explosiveBlastOverpressureResult.propellantsMinOverpressure;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("爆炸冲击波试验\n超压分析\n单位:Mpa", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	// 殉爆试验
	else if (itemData == "SacrificeExplosionAnalysis")
	{ 
		occView->SetCameraRotationState(false);
		m_PropertyStackWidget->setCurrentWidget(m_sacrificeExplosionPropertyWidget);
	}
	else if (itemData == "sacrificeStressShellResult") 
	{ 
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_sacrificeExplosionStressResultWidget);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellSacrificeExplosionStressResult(occView, nodeValues);

		auto sacrificeExplosionStressResult = ModelDataManager::GetInstance()->GetSacrificeExplosionStressResult();
		auto max_value = sacrificeExplosionStressResult.metalsMaxStress;
		auto min_value = sacrificeExplosionStressResult.metalsMinStress;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("殉爆试验\n应力分析\n单位:MPa", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "sacrificeStressPropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_sacrificeExplosionStressResultWidget);

	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantSacrificeExplosionStressResult(occView, nodeValues);

	auto sacrificeExplosionStressResult = ModelDataManager::GetInstance()->GetSacrificeExplosionStressResult();
	auto max_value = sacrificeExplosionStressResult.propellantsMaxStress;
	auto min_value = sacrificeExplosionStressResult.propellantsMinStress;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("殉爆试验\n应力分析\n单位:MPa", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "sacrificeStrainShellResult")
	{
		occView->SetCameraRotationState(false);
		m_PropertyStackWidget->setCurrentWidget(m_sacrificeExplosionStrainResultWidget);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellSacrificeExplosionStrainResult(occView, nodeValues);

		auto sacrificeExplosionStrainResult = ModelDataManager::GetInstance()->GetSacrificeExplosionStrainResult();
		auto max_value = sacrificeExplosionStrainResult.metalsMaxStrain;
		auto min_value = sacrificeExplosionStrainResult.metalsMinStrain;

		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("殉爆试验\n应变分析\n", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.6f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "sacrificeStrainPropellantResult")
	{
	occView->SetCameraRotationState(false);
	m_PropertyStackWidget->setCurrentWidget(m_sacrificeExplosionStrainResultWidget);

	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantSacrificeExplosionStrainResult(occView, nodeValues);

	auto sacrificeExplosionStrainResult = ModelDataManager::GetInstance()->GetSacrificeExplosionStrainResult();
	auto max_value = sacrificeExplosionStrainResult.propellantsMaxStrain;
	auto min_value = sacrificeExplosionStrainResult.propellantsMinStrain;

	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("殉爆试验\n应变分析\n", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.6f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "sacrificeTempShellResult") 
	{ 
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_sacrificeExplosionTemperatureResultWidget);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellSacrificeExplosionTemperatureResult(occView, nodeValues);

		auto sacrificeExplosionTemperatureResult = ModelDataManager::GetInstance()->GetSacrificeExplosionTemperatureResult();
		auto max_value = sacrificeExplosionTemperatureResult.metalsMaxTemperature;
		auto min_value = sacrificeExplosionTemperatureResult.metalsMinTemperature;

		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("殉爆试验\n温度分析\n单位:℃", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "sacrificeTempPropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_sacrificeExplosionTemperatureResultWidget);

	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantSacrificeExplosionTemperatureResult(occView, nodeValues);

	auto sacrificeExplosionTemperatureResult = ModelDataManager::GetInstance()->GetSacrificeExplosionTemperatureResult();
	auto max_value = sacrificeExplosionTemperatureResult.propellantsMaxTemperature;
	auto min_value = sacrificeExplosionTemperatureResult.propellantsMinTemperature;

	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("殉爆试验\n温度分析\n单位:℃", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "sacrificeOverpressureShellResult") 
	{ 
		occView->SetCameraRotationState(false);

		m_PropertyStackWidget->setCurrentWidget(m_sacrificeExplosionOverpressureResultWidge);

		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		view->SetProj(V3d_Zneg);

		std::vector<double> nodeValues;
		APISetNodeValue::SetShellSacrificeExplosionOverpressureResult(occView, nodeValues);

		auto sacrificeExplosionOverpressureResult = ModelDataManager::GetInstance()->GetSacrificeExplosionOverpressureResult();
		auto max_value = sacrificeExplosionOverpressureResult.metalsMaxOverpressure;
		auto min_value = sacrificeExplosionOverpressureResult.metalsMinOverpressure;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("殉爆试验\n超压分析\n单位:Mpa", true);
		Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
		aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
		aColorScale->SetSize(100, 400);
		aColorScale->SetRange(min_value, max_value);
		aColorScale->SetNumberOfIntervals(9);
		aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
		aColorScale->SetTextHeight(14);
		aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
		aColorScale->SetTitle(tostr);
		aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
		aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
		aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
		Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
		context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
		context->SetDisplayMode(aColorScale, 1, Standard_False);
		context->Display(aColorScale, Standard_True);
	}
	else if (itemData == "sacrificeOverpressurePropellantResult")
	{
	occView->SetCameraRotationState(false);

	m_PropertyStackWidget->setCurrentWidget(m_sacrificeExplosionOverpressureResultWidge);

	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();
	view->SetProj(V3d_Zneg);

	std::vector<double> nodeValues;
	APISetNodeValue::SetPropellantSacrificeExplosionOverpressureResult(occView, nodeValues);

	auto sacrificeExplosionOverpressureResult = ModelDataManager::GetInstance()->GetSacrificeExplosionOverpressureResult();
	auto max_value = sacrificeExplosionOverpressureResult.propellantsMaxOverpressure;
	auto min_value = sacrificeExplosionOverpressureResult.propellantsMinOverpressure;


	// 颜色条显示（与原逻辑一致）
	TCollection_ExtendedString tostr("殉爆试验\n超压分析\n单位:Mpa", true);
	Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
	aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
	aColorScale->SetSize(100, 400);
	aColorScale->SetRange(min_value, max_value);
	aColorScale->SetNumberOfIntervals(9);
	aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
	aColorScale->SetTextHeight(14);
	aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
	aColorScale->SetTitle(tostr);
	aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
	aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
	aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
	Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
	context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
	context->SetDisplayMode(aColorScale, 1, Standard_False);
	context->Display(aColorScale, Standard_True);
	}
}


