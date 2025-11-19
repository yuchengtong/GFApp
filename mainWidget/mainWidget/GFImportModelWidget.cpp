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
#include <MeshVS_Mesh.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>

#include <QSplitter>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QSplitter>

#include <V3d_View.hxx>
#include <V3d_TypeOfOrientation.hxx>


#include "OccView.h"
#include "GFLogWidget.h"
#include "GFTreeModelWidget.h"
#include "colour_change_algrithm.h"
#include "APISetNodeValue.h"

void HSVtoRGB(double h, double s, double v, double& r, double& g, double& b)
{
	if (s <= 0.0) {
		// 无饱和度时为灰度
		r = v;
		g = v;
		b = v;
		return;
	}

	// HSV转RGB核心计算
	double c = v * s;                  // 色度
	double x = c * (1.0 - std::fabs(std::fmod(h * 6.0, 2.0) - 1.0));
	double m = v - c;                  // 明度偏移值

	// 根据Hue值确定RGB分量
	double r_temp, g_temp, b_temp;
	if (h < 1.0 / 6.0) {
		r_temp = c;
		g_temp = x;
		b_temp = 0.0;
	}
	else if (h < 2.0 / 6.0) {
		r_temp = x;
		g_temp = c;
		b_temp = 0.0;
	}
	else if (h < 3.0 / 6.0) {
		r_temp = 0.0;
		g_temp = c;
		b_temp = x;
	}
	else if (h < 4.0 / 6.0) {
		r_temp = 0.0;
		g_temp = x;
		b_temp = c;
	}
	else if (h < 5.0 / 6.0) {
		r_temp = x;
		g_temp = 0.0;
		b_temp = c;
	}
	else {
		r_temp = c;
		g_temp = 0.0;
		b_temp = x;
	}

	// 应用明度偏移并限制范围
	r = std::clamp(r_temp + m, 0.0, 1.0);
	g = std::clamp(g_temp + m, 0.0, 1.0);
	b = std::clamp(b_temp + m, 0.0, 1.0);
}

MeshVS_DataMapOfIntegerColor getMeshDataMap(std::vector<double> tt, double min, double max)
{
	double a, r, g, b;
	MeshVS_DataMapOfIntegerColor colormap;
	int index = 0;

	// 处理特殊情况：避免除零
	if (max <= min) {
		Quantity_Color defaultColor(0.5, 0.5, 0.5, Quantity_TOC_RGB); // 灰色
		for (size_t i = 0; i < tt.size(); ++i) {
			colormap.Bind(i + 1, defaultColor);
		}
		return colormap;
	}

	for (double t : tt)
	{
		// 1. 归一化到[0,1]范围
		a = (t - min) / (max - min);
		a = std::clamp(a, 0.0, 1.0); // 确保值在有效范围

		// 2. 定义HSV参数（保持蓝→绿→红趋势）
		// H: 0.666(蓝) → 0.333(绿) → 0(红)，对应HSV色轮
		double h = 0.666 - a * 0.666; // 从蓝色(0.666)过渡到红色(0)
		double s = 1.0;               // 最大饱和度
		double v = 1.0;               // 最大明度

		// 3. 手动转换HSV到RGB
		HSVtoRGB(h, s, v, r, g, b);

		// 4. 绑定颜色到索引
		colormap.Bind(index + 1, Quantity_Color(r, g, b, Quantity_TOC_RGB));
		index++;
	}
	return colormap;
}


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

	mainSplitter->setStretchFactor(0, 1);
	mainSplitter->setStretchFactor(1, 9);

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
	if (itemData == "Geometry") {
		m_PropertyStackWidget->setCurrentWidget(m_geomPropertyWidget);
		auto modelInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
		if (!modelInfo.shape.IsNull())
		{
			auto occView = GetOccView();
			Handle(AIS_InteractiveContext) context = occView->getContext();
			context->RemoveAll(true);
			Handle(AIS_Shape) modelPresentation = new AIS_Shape(modelInfo.shape);
			context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
			context->SetColor(modelPresentation, Quantity_NOC_CYAN, true);
			context->Display(modelPresentation, false);
			occView->fitAll();
		}
		m_geomPropertyWidget->UpdataPropertyInfo();
	}
	else if (itemData == "Material") {
		m_PropertyStackWidget->setCurrentWidget(m_materialPropertyWidget);
	}
	else if (itemData == "Database") {
		m_PropertyStackWidget->setCurrentWidget(m_databasePropertyWidget);
	}
	else if (itemData == "Mesh") {
		m_PropertyStackWidget->setCurrentWidget(m_meshPropertyWidget);

		auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
		if (meshInfo.isChecked)
		{
			auto occView = GetOccView();
			Handle(AIS_InteractiveContext) context = occView->getContext();

			BRep_Builder builder;
			TopoDS_Compound compound;
			builder.MakeCompound(compound);
			auto tri = meshInfo.triangleStructure;
			auto myEdges = tri.GetMyEdge();

			auto myNodeCoords = tri.GetmyNodeCoords();

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
			context->RemoveAll(true);
			context->Display(aisCompound, Standard_True);
		}

		m_meshPropertyWidget->UpdataPropertyInfo();
	}
	else if (itemData == "Analysis") {
		m_PropertyStackWidget->setCurrentWidget(m_settingPropertyWidget);

		auto modelInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
		if (!modelInfo.shape.IsNull())
		{
			auto occView = GetOccView();
			Handle(AIS_InteractiveContext) context = occView->getContext();
			context->RemoveAll(true);
			Handle(AIS_Shape) modelPresentation = new AIS_Shape(modelInfo.shape);
			context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
			context->SetColor(modelPresentation, Quantity_NOC_CYAN, true);
			context->Display(modelPresentation, false);
			occView->fitAll();
		}
	}
	else if (itemData == "FallAnalysis")
	{
		m_PropertyStackWidget->setCurrentWidget(m_fallPropertyWidget);

		auto modelInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
		if (!modelInfo.shape.IsNull())
		{
			auto occView = GetOccView();
			Handle(AIS_InteractiveContext) context = occView->getContext();
			context->RemoveAll(true);
			Handle(AIS_Shape) modelPresentation = new AIS_Shape(modelInfo.shape);
			context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
			context->SetColor(modelPresentation, Quantity_NOC_CYAN, true);
			context->Display(modelPresentation, false);
			occView->fitAll();
		}

	}
	else if (itemData == "StressResult")
	{
		m_PropertyStackWidget->setCurrentWidget(m_stressResultWidget);

		auto occView = GetOccView();
		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		context->RemoveAll(true);

		std::vector<double> nodeValues;
		APISetNodeValue::SetFallStressResult(occView, nodeValues);
		occView->fitAll();

		auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
		auto max_value = fallAnalysisResultInfo.stressMaxValue;
		auto min_value = fallAnalysisResultInfo.stressMinValue;


		// 颜色条显示（与原逻辑一致）
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
	else if (itemData == "StrainResult") {
		m_PropertyStackWidget->setCurrentWidget(m_strainResultWidget);

	}
	else if (itemData == "TemperatureResult") {
		m_PropertyStackWidget->setCurrentWidget(m_temperatureResultWidget);
		auto occView = GetOccView();
		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		context->RemoveAll(true);

		std::vector<double> nodeValues;
		APISetNodeValue::SetFallTemperatureResult(occView, nodeValues);
		occView->fitAll();

		auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
		auto max_value = fallAnalysisResultInfo.temperatureMaxValue;
		auto min_value = fallAnalysisResultInfo.temperatureMinValue;


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
	else if (itemData == "OverpressureResult") {
		m_PropertyStackWidget->setCurrentWidget(m_overpressureResultWidge);
		auto occView = GetOccView();
		Handle(AIS_InteractiveContext) context = occView->getContext();
		Handle(V3d_View) view = occView->getView();
		context->RemoveAll(true);

		std::vector<double> nodeValues;
		APISetNodeValue::SetFallOverpressureResult(occView, nodeValues);
		occView->fitAll();

		auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
		auto max_value = fallAnalysisResultInfo.overpressureMaxValue;
		auto min_value = fallAnalysisResultInfo.overpressureMinValue;


		// 颜色条显示（与原逻辑一致）
		TCollection_ExtendedString tostr("跌落试验\n超压分析\n单位:℃", true);
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
	else if (itemData == "FastCombustionAnalysis") {
		m_PropertyStackWidget->setCurrentWidget(m_fastCombustionPropertyWidget);
	}
	else if (itemData == "SlowCombustionAnalysis") {
		m_PropertyStackWidget->setCurrentWidget(m_slowCombustionPropertyWidget);
	}
	else if (itemData == "Results") {
		m_PropertyStackWidget->setCurrentWidget(m_resultsPropertyWidget);
	}
	else if (itemData == "Steel") {
		m_PropertyStackWidget->setCurrentWidget(m_steelPropertyWidgett);
	}
	else if (itemData == "Propellant") {
		m_PropertyStackWidget->setCurrentWidget(m_propellantPropertyWidget);
	}
	else if (itemData == "Judgment") {
		m_PropertyStackWidget->setCurrentWidget(m_judgmentPropertyWidget);
	}
	else if (itemData == "Calculation") {
		m_PropertyStackWidget->setCurrentWidget(m_calculationPropertyWidget);
	}
	else if (itemData == "Project") {
		m_PropertyStackWidget->setCurrentWidget(m_projectPropertyWidge);
	}
	else if (itemData == "Insulatingheat") {
		m_PropertyStackWidget->setCurrentWidget(m_insulatingheatPropertyWidget);
	}
	else if (itemData == "Outheat") {
		m_PropertyStackWidget->setCurrentWidget(m_outheatPropertyWidget);
	}
	else if (itemData == "ShootAnalysis") {
		m_PropertyStackWidget->setCurrentWidget(m_shootPropertyWidget);
	}
	else if (itemData == "JetImpactAnalysis") {
		m_PropertyStackWidget->setCurrentWidget(m_jetImpactPropertyWidget);
	}
	else if (itemData == "FragmentationImpactAnalysis") {
		m_PropertyStackWidget->setCurrentWidget(m_fragmentationImpactPropertyWidget);
	}
	else if (itemData == "ExplosiveBlastAnalysis") {
		m_PropertyStackWidget->setCurrentWidget(m_explosiveBlastPropertyWidget);
	}
	else if (itemData == "SacrificeExplosionAnalysis") {
		m_PropertyStackWidget->setCurrentWidget(m_sacrificeExplosionPropertyWidget);
	}
	
}






