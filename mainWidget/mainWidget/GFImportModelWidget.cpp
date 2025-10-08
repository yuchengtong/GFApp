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

#include "OccView.h"
#include "GFLogWidget.h"
#include "GFTreeModelWidget.h"
#include "colour_change_algrithm.h"

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
	else if (itemData == "FallAnalysis") {
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
	else if (itemData == "StressResult") {
		m_PropertyStackWidget->setCurrentWidget(m_stressResultWidget);

		auto occView = GetOccView();
		Handle(AIS_InteractiveContext) context = occView->getContext();
		auto view = occView->getView();
		context->RemoveAll(true);

		// 仅处理 double 类型的 clamp 函数
		auto my_clamp =[](double value, double low, double high) {
			if (value < low) 
				return low;
			if (value > high) 
				return high;
			return value;
		};

		auto fallAnalysisResultInfo=ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
		if (fallAnalysisResultInfo.isChecked)
		{
			auto allnode = fallAnalysisResultInfo.triangleStructure->GetAllNodes();
			auto nodecoords = fallAnalysisResultInfo.triangleStructure->GetmyNodeCoords();
			auto max_value = fallAnalysisResultInfo.maxValue;
			auto min_value = fallAnalysisResultInfo.minValue;

			std::vector<double> nodeValues;

			Handle(MeshVS_Mesh) aMesh = new MeshVS_Mesh();
			aMesh->SetDataSource(fallAnalysisResultInfo.triangleStructure);

			// 椭圆参数
			const double f1x = 20.0, f1z = -200.0;   // 焦点1
			const double f2x = 820.0, f2z = -200.0; // 焦点2
			const double twoA = 900.0;              // 长轴长度
			const double twoB = 140.0;              // 短轴长度

			// 数值范围参数
			const double focusValue = min_value+(max_value- min_value)*0.5;         // 焦点处的值
			const double minValue = min_value;           // 椭圆内最小值

			// 计算两焦点之间的距离
			const double focusDistance = sqrt(pow(f2x - f1x, 2) + pow(f2z - f1z, 2));

			// lambda表达式：判断点是否在椭圆内并计算对应值
			auto getEllipseValue = [&](double x, double z) {
				// 计算点到两焦点的距离之和
				double d1 = sqrt(pow(x - f1x, 2) + pow(z - f1z, 2));
				double d2 = sqrt(pow(x - f2x, 2) + pow(z - f2z, 2));
				double sumD = d1 + d2;

				// 判断是否在椭圆内
				if (sumD < twoA) {
					// 归一化距离（0表示在焦点附近，1表示在椭圆边缘）
					// 最小距离和为两焦点距离（焦点处），最大为2a（椭圆边缘）
					double t = (sumD - focusDistance) / (twoA - focusDistance);
					t = my_clamp(t, 0.0, 1.0);

					// 计算值：从焦点值线性过渡到最小值
					return focusValue - (focusValue - minValue) * t;
				}
				else {
					// 椭圆外的点：返回自定义值（示例用z坐标）
					return minValue;
				}
			};

			// 遍历所有节点计算值
			for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) {
				int nodeID = it.Key();
				double x = nodecoords->Value(nodeID, 1); // 节点x坐标
				double z = nodecoords->Value(nodeID, 3); // 节点z坐标

				// 调整z阈值为-235，同时保持缩小后的区域范围
				bool isOriginalArea = (z < -235 &&  // 从-230改为-235
					((f1x + 35 < x && x < f1x + 45) ||  // 维持10单位宽度
					(f2x - 45 < x && x < f2x - 35)));   // 维持10单位宽度

				// 周围一圈的条件（同步调整z阈值）
				double offset = 15.0;
				bool isSurroundingArea =
					// 上侧边界区域（z阈值同步调整为-235）
					(z < -235 + offset &&
					((f1x + 35 < x && x < f1x + 45) ||
						(f2x - 45 < x && x < f2x - 35))) ||
					// 左侧边界区域
						(z < -235 + offset &&
					((f1x + 35 - offset < x && x <= f1x + 35) ||
							(f2x - 45 - offset < x && x <= f2x - 45))) ||
					// 右侧边界区域
						(z < -235 + offset &&
					((f1x + 45 <= x && x < f1x + 45 + offset) ||
							(f2x - 35 <= x && x < f2x - 35 + offset)));

				bool isSurroundingArea_box =
					// 上侧边界区域（z阈值同步调整）
					(z < -235 + offset + 10 &&
					((f1x + 37 < x && x < f1x + 43) ||
						(f2x - 43 < x && x < f2x - 37)));

				if (isOriginalArea) {
					nodeValues.push_back(max_value);
				}
				else if (isSurroundingArea) {
					nodeValues.push_back(0.6 * max_value);
				}
				else if (isSurroundingArea_box) {
					nodeValues.push_back(0.55 * max_value);
				}
				else
				{
					// 获取椭圆内的计算值
					double value = getEllipseValue(x, z);
					nodeValues.push_back(value);
				}
				
			}


			// 设置颜色映射和显示（与原逻辑一致）
			MeshVS_DataMapOfIntegerColor colormap = getMeshDataMap(nodeValues, min_value, max_value);
			Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
			nodal->SetColors(colormap);
			aMesh->AddBuilder(nodal);
			aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);

			// 显示模型
			context->Display(aMesh, Standard_True);
			occView->fitAll();

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
	}
	else if (itemData == "TemperatureResult") {
		m_PropertyStackWidget->setCurrentWidget(m_temperatureResultWidget);

	}
	else if (itemData == "OverpressureResult") {
		m_PropertyStackWidget->setCurrentWidget(m_overpressureResultWidge);

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
	
}






