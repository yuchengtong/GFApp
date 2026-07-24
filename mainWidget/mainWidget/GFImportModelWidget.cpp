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
	m_steelPropertyWidgett = new SteelPropertyWidget();
	m_propellantPropertyWidget = new PropellantPropertyWidget();
	m_projectPropertyWidge = new ProjectPropertyWidge();
	m_calculationPropertyWidget = new CalculationPropertyWidget();
	m_judgmentPropertyWidget = new JudgmentPropertyWidget();
	m_insulatingheatPropertyWidget = new InsulatingheatPropertyWidget();
	m_outheatPropertyWidget = new OutheatPropertyWidget();
	m_shootPropertyWidget = new ShootPropertyWidget();
	m_jetImpactPropertyWidget = new JetImpactPropertyWidget();
	m_fragmentationImpactPropertyWidget = new FragmentationImpactPropertyWidget();
	m_explosiveBlastPropertyWidget = new ExplosiveBlastPropertyWidget();
	m_sacrificeExplosionPropertyWidget = new SacrificeExplosionPropertyWidget();
	m_databasePropertyWidget = new DatabasePropertyWidget();
	m_nozzlePropertyWidget = new NozzlePropertyWidget();
	// 跌落
	m_stressResultWidget = new StressResultWidget();
	m_temperatureResultWidget = new TemperatureResultWidget();
	m_overpressureResultWidge = new OverpressureResultWidget();
	m_strainResultWidget = new StrainResultWidget();
	m_reactionDegreeResultWidget = new ReactionDegreeResultWidget();

	// 枪击结果
	m_shootStressResultWidget = new StressResultWidget();
	m_shootTemperatureResultWidget = new TemperatureResultWidget();
	m_shootOverpressureResultWidge = new OverpressureResultWidget();
	m_shootStrainResultWidget = new StrainResultWidget();
	m_shootReactionDegreeResultWidget = new ReactionDegreeResultWidget();
	// 破片结果
	m_fragmentationImpactStressResultWidget = new StressResultWidget();
	m_fragmentationImpactTemperatureResultWidget = new TemperatureResultWidget();
	m_fragmentationImpactOverpressureResultWidge = new OverpressureResultWidget();
	m_fragmentationImpactStrainResultWidget = new StrainResultWidget();
	m_fragmentationImpactReactionDegreeResultWidget = new ReactionDegreeResultWidget();
	
	// 快烤结果
	m_fastCombustionTemperatureResultWidget = new TemperatureResultWidget();

	// 慢烤结果
	m_slowCombustionTemperatureResultWidget = new TemperatureResultWidget();

	// 射流冲击结果
	m_jetImpactStressResultWidget = new StressResultWidget();
	m_jetImpactTemperatureResultWidget = new TemperatureResultWidget();
	m_jetImpactOverpressureResultWidge = new OverpressureResultWidget();
	m_jetImpactStrainResultWidget = new StrainResultWidget();
	m_jetImpactReactionDegreeResultWidget = new ReactionDegreeResultWidget();

	// 爆炸冲击波结果
	m_explosiveBlastStressResultWidget = new StressResultWidget();
	m_explosiveBlastTemperatureResultWidget = new TemperatureResultWidget();
	m_explosiveBlastOverpressureResultWidge = new OverpressureResultWidget();
	m_explosiveBlastStrainResultWidget = new StrainResultWidget();
	m_explosiveBlastReactionDegreeResultWidget = new ReactionDegreeResultWidget();

	// 殉爆结果
	m_sacrificeExplosionStressResultWidget = new StressResultWidget();
	m_sacrificeExplosionTemperatureResultWidget = new TemperatureResultWidget();
	m_sacrificeExplosionOverpressureResultWidge = new OverpressureResultWidget();
	m_sacrificeExplosionStrainResultWidget = new StrainResultWidget();
	m_sacrificeExplosionReactionDegreeResultWidget = new ReactionDegreeResultWidget();


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
	m_PropertyStackWidget->addWidget(m_nozzlePropertyWidget);
	

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

	m_PropertyStackWidget->addWidget(m_reactionDegreeResultWidget);
	m_PropertyStackWidget->addWidget(m_shootReactionDegreeResultWidget);
	m_PropertyStackWidget->addWidget(m_jetImpactReactionDegreeResultWidget);
	m_PropertyStackWidget->addWidget(m_fragmentationImpactReactionDegreeResultWidget);
	m_PropertyStackWidget->addWidget(m_explosiveBlastReactionDegreeResultWidget);
	m_PropertyStackWidget->addWidget(m_sacrificeExplosionReactionDegreeResultWidget);


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
    // ========== 1. 重复点击拦截：相同节点直接返回 ==========
    if (itemData == m_lastClickedItemKey) {
        return;
    }
    m_lastClickedItemKey = itemData;

    auto occView = GetOccView();
    if (!occView) {
        return;
    }
    Handle(AIS_InteractiveContext) context = occView->getContext();
    Handle(V3d_View) view = occView->getView();
    std::vector<double> nodeValues; // 复用节点值容器，避免重复构造

    // ========== 内部工具Lambda：公共逻辑全量提取，不新增类函数 ==========
    // 清理场景旧对象：网格、色标、半透明辅助形状（原代码重复10+次）
    auto clearSceneAuxObjects = [&]() {
        AIS_ListOfInteractive displayedList;
        context->DisplayedObjects(displayedList);

        for (AIS_ListIteratorOfListOfInteractive it(displayedList); it.More(); it.Next()) {
            Handle(AIS_InteractiveObject) obj = it.Value();
            if (obj->IsKind(STANDARD_TYPE(MeshVS_Mesh))) {
                context->Erase(obj, Standard_False);
                continue;
            }
            if (obj->IsKind(STANDARD_TYPE(AIS_ColorScale))) {
                context->Erase(obj, Standard_False);
                continue;
            }
            if (obj->IsKind(STANDARD_TYPE(AIS_Shape))) {
                Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(obj);
                if (!aisShape.IsNull() && std::abs(aisShape->Transparency() - 0.9) < 1e-6) {
                    context->Erase(obj, Standard_False);
                }
            }
        }
    };

    // 统一创建颜色标尺：公共参数集中配置，消除几十行重复代码
    auto createColorScale = [&](const QString& title, double minVal, double maxVal, const char* format) {
        QByteArray utf8 = title.toUtf8();
        TCollection_ExtendedString tostr(utf8.constData(), Standard_True);

        Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();

        aColorScale->SetFormat(TCollection_AsciiString(format));
        aColorScale->SetSize(100, 400);
        aColorScale->SetRange(minVal, maxVal);
        aColorScale->SetNumberOfIntervals(9);
        aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
        aColorScale->SetTextHeight(14);
        aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
        aColorScale->SetTitle(tostr);
        aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
        aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
        aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);

        Graphic3d_Vec2i offset(0, 450);
        context->SetTransformPersistence(aColorScale,
            new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, offset));
        context->SetDisplayMode(aColorScale, 1, Standard_False);
        context->Display(aColorScale, Standard_True);
    };

    // 结果页统一前置操作：关闭旋转、切换页面、清理场景、设置俯视视角
    auto setupResultView = [&](QWidget* page) {
        occView->SetCameraRotationState(false);
        m_PropertyStackWidget->setCurrentWidget(page);
        clearSceneAuxObjects();
        view->SetProj(V3d_Zneg);
    };

    // ========== 2. 纯页面切换：数据驱动映射表，消除30+个重复if ==========
    struct PageSwitchConfig {
        QWidget* page;
        bool enableRotation;
    };
    static const QMap<QString, PageSwitchConfig> pageConfigMap = {
        // 基础属性
        {"Material", {m_materialPropertyWidget, true}},
        {"Results", {m_resultsPropertyWidget, true}},
        {"Steel", {m_steelPropertyWidgett, true}},
        {"Propellant", {m_propellantPropertyWidget, true}},
        {"Nozzle", {m_nozzlePropertyWidget, true}},
        {"Judgment", {m_judgmentPropertyWidget, true}},
        {"Calculation", {m_calculationPropertyWidget, true}},
        {"Project", {m_projectPropertyWidge, true}},
        {"Insulatingheat", {m_insulatingheatPropertyWidget, true}},
        {"Outheat", {m_outheatPropertyWidget, true}},
        {"Database", {m_databasePropertyWidget, true}},
        {"Analysis", {m_settingPropertyWidget, true}},

        // 试验根节点
        {"FallAnalysis", {m_fallPropertyWidget, false}},
        {"FastCombustionAnalysis", {m_fastCombustionPropertyWidget, false}},
        {"SlowCombustionAnalysis", {m_slowCombustionPropertyWidget, false}},
        {"ShootAnalysis", {m_shootPropertyWidget, false}},
        {"JetImpactAnalysis", {m_jetImpactPropertyWidget, false}},
        {"FragmentationImpactAnalysis", {m_fragmentationImpactPropertyWidget, false}},
        {"ExplosiveBlastAnalysis", {m_explosiveBlastPropertyWidget, false}},
        {"SacrificeExplosionAnalysis", {m_sacrificeExplosionPropertyWidget, false}},

        // 结果根节点
        {"StressResult", {m_stressResultWidget, false}},
        {"StrainResult", {m_strainResultWidget, false}},
        {"TemperatureResult", {m_temperatureResultWidget, false}},
        {"OverpressureResult", {m_overpressureResultWidge, false}},
        {"ReactionDegreeResult", {m_reactionDegreeResultWidget, false}},
        {"ShootReactionDegreeResult", {m_shootReactionDegreeResultWidget, false}},
        {"jetImpactReactionDegreeResult", {m_jetImpactReactionDegreeResultWidget, false}},
        {"fragmentationImpactReactionDegreeResult", {m_fragmentationImpactReactionDegreeResultWidget, false}},
        {"explosiveBlastReactionDegreeResult", {m_explosiveBlastReactionDegreeResultWidget, false}},
        {"sacrificeExplosionReactionDegreeResult", {m_sacrificeExplosionReactionDegreeResultWidget, false}},
    };

    auto itPage = pageConfigMap.constFind(itemData);
    if (itPage != pageConfigMap.constEnd()) {
        occView->SetCameraRotationState(itPage.value().enableRotation);
        m_PropertyStackWidget->setCurrentWidget(itPage.value().page);
        return;
    }

    // ========== 3. 几何与网格节点处理 ==========
    static const QStringList geomItems = {
        "NozzleGeometry", "ShellGeometry",
        "PropellantGeometry", "HeatInsulatingLayerGeometry"
    };
    static const QStringList meshItems = {
        "NozzleMesh", "ShellMesh",
        "PropellantMesh", "HeatInsulatingLayerMesh"
    };

    if (itemData == "Geometry") {
        occView->SetCameraRotationState(true);
        m_PropertyStackWidget->setCurrentWidget(m_geomPropertyWidget);
        m_geomPropertyWidget->UpdataPropertyInfo();
        return;
    }
    if (geomItems.contains(itemData)) {
        occView->SetCameraRotationState(true);
        m_PropertyStackWidget->setCurrentWidget(m_geomPropertyWidget);
        clearSceneAuxObjects();
        view->Redraw();
        return;
    }
    if (itemData == "Mesh") {
        occView->SetCameraRotationState(true);
        m_PropertyStackWidget->setCurrentWidget(m_meshPropertyWidget);
        m_meshPropertyWidget->UpdataPropertyInfo();
        clearSceneAuxObjects();
        view->Redraw();
        return;
    }
    if (meshItems.contains(itemData)) {
        occView->SetCameraRotationState(true);
        m_PropertyStackWidget->setCurrentWidget(m_meshPropertyWidget);
        clearSceneAuxObjects();
        view->Redraw();
        return;
    }

    // ========== 4. 云图结果节点（按试验分类，逻辑清晰易维护） ==========
    // ----- 跌落试验 -----
    if (itemData == "FallStressShellResult") {
        setupResultView(m_stressResultWidget);
        APISetNodeValue::SetShellFallStressNephogram(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetFallStressResult();
        createColorScale("跌落试验\n应力分析\n单位:MPa", res.metalsMinStress, res.metalsMaxStress, "%.2f");
    }
    else if (itemData == "FallStressPropellantResult") {
        setupResultView(m_stressResultWidget);
        APISetNodeValue::SetPropellantFallStressNephogram(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetFallStressResult();
        createColorScale("跌落试验\n应力分析\n单位:MPa", res.propellantsMinStress, res.propellantsMaxStress, "%.2f");
    }
    else if (itemData == "FallStrainShellResult") {
        setupResultView(m_strainResultWidget);
        APISetNodeValue::SetShellFallStrainNephogram(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetFallStrainResult();
        createColorScale("跌落试验\n应变分析\n单位:mm", res.metalsMinStrain, res.metalsMaxStrain, "%.6f");
    }
    else if (itemData == "FallStrainPropellantResult") {
        setupResultView(m_strainResultWidget);
        APISetNodeValue::SetPropellantFallStrainNephogram(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetFallStrainResult();
        createColorScale("跌落试验\n应变分析\n单位:mm", res.propellantsMinStrain, res.propellantsMaxStrain, "%.6f");
    }
    else if (itemData == "FallTemperatureShellResult") {
        setupResultView(m_temperatureResultWidget);
        APISetNodeValue::SetShellFallTempNephogram(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetFallTemperatureResult();
        createColorScale("跌落试验\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "FallTemperaturePropellantResult") {
        setupResultView(m_temperatureResultWidget);
        APISetNodeValue::SetPropellantFallTempNephogram(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetFallTemperatureResult();
        createColorScale("跌落试验\n温度分析\n单位:℃", res.propellantsMinTemperature, res.propellantsMaxTemperature, "%.2f");
    }
    else if (itemData == "FallOverpressureShellResult") {
        setupResultView(m_overpressureResultWidge);
        APISetNodeValue::SetShellFallPressureNephogram(occView, nodeValues);
        occView->fitAll();
        auto res = ModelDataManager::GetInstance()->GetFallOverpressureResult();
        createColorScale("跌落试验\n超压分析\n单位:MPa", res.metalsMinOverpressure, res.metalsMaxOverpressure, "%.2f");
    }
    else if (itemData == "FallOverpressurePropellantResult") {
        setupResultView(m_overpressureResultWidge);
        APISetNodeValue::SetPropellantFallPressureNephogram(occView, nodeValues);
        occView->fitAll();
        auto res = ModelDataManager::GetInstance()->GetFallOverpressureResult();
        createColorScale("跌落试验\n超压分析\n单位:MPa", res.propellantsMinOverpressure, res.propellantsMaxOverpressure, "%.2f");
    }
    else if (itemData == "reactionDegreePropellantResult") {
        setupResultView(m_reactionDegreeResultWidget);
        APISetNodeValue::SetPropellantFallReactionDegreeNephogram(occView, nodeValues);
        occView->fitAll();
        auto res = ModelDataManager::GetInstance()->GetFallReactionDegreeResult();
        createColorScale("跌落试验\n反应度分析", res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree, "%.2f");
    }

    // ----- 快速烤燃 -----
    else if (itemData == "fastTemperatureShellResult") {
        setupResultView(m_fastCombustionTemperatureResultWidget);
        APISetNodeValue::SetShellFastCombustionTempNephogram(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetFastCombustionTemperatureResult();
        createColorScale("快速烤燃\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "fastTemperaturePropellantResult") {
        setupResultView(m_fastCombustionTemperatureResultWidget);
        APISetNodeValue::SetPropellantFastCombustionTempNephogram(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetFastCombustionTemperatureResult();
        createColorScale("快速烤燃\n温度分析\n单位:℃", res.propellantsMinTemperature, res.propellantsMaxTemperature, "%.2f");
    }

    // ----- 慢速烤燃 -----
    else if (itemData == "slowTemperatureShellResult") {
        setupResultView(m_slowCombustionTemperatureResultWidget);
        APISetNodeValue::SetShellSlowCombustionTempNephogram(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetSlowCombustionTemperatureResult();
        createColorScale("慢速烤燃\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "slowTemperaturePropellantResult") {
        setupResultView(m_slowCombustionTemperatureResultWidget);
        APISetNodeValue::SetPropellantSlowCombustionTempNephogram(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetSlowCombustionTemperatureResult();
        createColorScale("慢速烤燃\n温度分析\n单位:℃", res.propellantsMinTemperature, res.propellantsMaxTemperature, "%.2f");
    }

    // ----- 枪击试验 -----
    else if (itemData == "shootStressShellResult") {
        setupResultView(m_shootStressResultWidget);
        APISetNodeValue::SetShellShootStressResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetShootStressResult();
        createColorScale("枪击试验\n应力分析\n单位:MPa", res.metalsMinStress, res.metalsMaxStress, "%.2f");
    }
    else if (itemData == "shootStressPropellantResult") {
        setupResultView(m_shootStressResultWidget);
        APISetNodeValue::SetPropellantShootStressResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetShootStressResult();
        createColorScale("枪击试验\n应力分析\n单位:MPa", res.propellantsMinStress, res.propellantsMaxStress, "%.2f");
    }
    else if (itemData == "shootStrainShellResult") {
        setupResultView(m_shootStrainResultWidget);
        APISetNodeValue::SetShellShootStrainResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetShootStrainResult();
        createColorScale("枪击试验\n应变分析\n单位:mm", res.metalsMinStrain, res.metalsMaxStrain, "%.6f");
    }
    else if (itemData == "shootStrainPropellantResult") {
        setupResultView(m_shootStrainResultWidget);
        APISetNodeValue::SetPropellantShootStrainResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetShootStrainResult();
        createColorScale("枪击试验\n应变分析\n单位:mm", res.propellantsMinStrain, res.propellantsMaxStrain, "%.6f");
    }
    else if (itemData == "shootTempShellResult") {
        setupResultView(m_shootTemperatureResultWidget);
        APISetNodeValue::SetShellShootTemperatureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetShootTemperatureResult();
        createColorScale("枪击试验\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "shootTempPropellantResult") {
        setupResultView(m_shootTemperatureResultWidget);
        APISetNodeValue::SetPropellantShootTemperatureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetShootTemperatureResult();
        createColorScale("枪击试验\n温度分析\n单位:℃", res.propellantsMinTemperature, res.propellantsMaxTemperature, "%.2f");
    }
    else if (itemData == "shootOverpressureShellResult") {
        setupResultView(m_shootOverpressureResultWidge);
        APISetNodeValue::SetShellShootOverpressureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetShootOverpressureResult();
        createColorScale("枪击试验\n超压分析\n单位:MPa", res.metalsMinOverpressure, res.metalsMaxOverpressure, "%.2f");
    }
    else if (itemData == "shootOverpressurePropellantResult") {
        setupResultView(m_shootOverpressureResultWidge);
        APISetNodeValue::SetPropellantShootOverpressureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetShootOverpressureResult();
        createColorScale("枪击试验\n超压分析\n单位:MPa", res.propellantsMinOverpressure, res.propellantsMaxOverpressure, "%.2f");
    }
    else if (itemData == "shootReactionDegreePropellantResult") {
        setupResultView(m_shootReactionDegreeResultWidget);
        APISetNodeValue::SetPropellantShootReactionDegreeNephogram(occView, nodeValues);
        occView->fitAll();
        auto res = ModelDataManager::GetInstance()->GetShootReactionDegreeResult();
        createColorScale("枪击试验\n反应度分析", res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree, "%.2f");
    }

    // ----- 射流冲击 -----
    else if (itemData == "jetStressShellResult") {
        setupResultView(m_jetImpactStressResultWidget);
        APISetNodeValue::SetShellJetImpactStressResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetJetImpactStressResult();
        createColorScale("射流冲击试验\n应力分析\n单位:MPa", res.metalsMinStress, res.metalsMaxStress, "%.2f");
    }
    else if (itemData == "jetStressPropellantResult") {
        setupResultView(m_jetImpactStressResultWidget);
        APISetNodeValue::SetPropellantJetImpactStressResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetJetImpactStressResult();
        createColorScale("射流冲击试验\n应力分析\n单位:MPa", res.propellantsMinStress, res.propellantsMaxStress, "%.2f");
    }
    else if (itemData == "jetStrainShellResult") {
        setupResultView(m_jetImpactStrainResultWidget);
        APISetNodeValue::SetShellJetImpactStrainResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetJetImpactStrainResult();
        createColorScale("射流冲击试验\n应变分析\n单位:mm", res.metalsMinStrain, res.metalsMaxStrain, "%.6f");
    }
    else if (itemData == "jetStrainPropellantResult") {
        setupResultView(m_jetImpactStrainResultWidget);
        APISetNodeValue::SetPropellantJetImpactStrainResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetJetImpactStrainResult();
        createColorScale("射流冲击试验\n应变分析\n单位:mm", res.propellantsMinStrain, res.propellantsMaxStrain, "%.6f");
    }
    else if (itemData == "jetTempShellResult") {
        setupResultView(m_jetImpactTemperatureResultWidget);
        APISetNodeValue::SetShellJetImpactTemperatureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetJetImpactTemperatureResult();
        createColorScale("射流冲击试验\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "jetTempPropellantResult") {
        setupResultView(m_jetImpactTemperatureResultWidget);
        APISetNodeValue::SetPropellantJetImpactTemperatureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetJetImpactTemperatureResult();
        createColorScale("射流冲击试验\n温度分析\n单位:℃", res.propellantsMinTemperature, res.propellantsMaxTemperature, "%.2f");
    }
    else if (itemData == "jetOverpressureShellResult") {
        setupResultView(m_jetImpactOverpressureResultWidge);
        APISetNodeValue::SetShellJetImpactOverpressureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetJetImpactOverpressureResult();
        createColorScale("射流冲击试验\n超压分析\n单位:MPa", res.metalsMinOverpressure, res.metalsMaxOverpressure, "%.2f");
    }
    else if (itemData == "jetOverpressurePropellantResult") {
        setupResultView(m_jetImpactOverpressureResultWidge);
        APISetNodeValue::SetPropellantJetImpactOverpressureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetJetImpactOverpressureResult();
        createColorScale("射流冲击试验\n超压分析\n单位:MPa", res.propellantsMinOverpressure, res.propellantsMaxOverpressure, "%.2f");
    }
    else if (itemData == "jetImpactReactionDegreePropellantResult") {
        setupResultView(m_jetImpactReactionDegreeResultWidget);
        APISetNodeValue::SetPropellantJetImpactReactionDegreeNephogram(occView, nodeValues);
        occView->fitAll();
        auto res = ModelDataManager::GetInstance()->GetJetImpactReactionDegreeResult();
        createColorScale("射流冲击试验\n反应度分析", res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree, "%.2f");
    }

    // ----- 破片试验 -----
    else if (itemData == "fragmentationStressShellResult") {
        setupResultView(m_fragmentationImpactStressResultWidget);
        APISetNodeValue::SetShellFragmentationStressResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactStressResult();
        createColorScale("破片试验\n应力分析\n单位:MPa", res.metalsMinStress, res.metalsMaxStress, "%.2f");
    }
    else if (itemData == "fragmentationStressPropellantResult") {
        setupResultView(m_fragmentationImpactStressResultWidget);
        APISetNodeValue::SetPropellantFragmentationStressResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactStressResult();
        createColorScale("破片试验\n应力分析\n单位:MPa", res.propellantsMinStress, res.propellantsMaxStress, "%.2f");
    }
    else if (itemData == "fragmentationStrainShellResult") {
        setupResultView(m_fragmentationImpactStrainResultWidget);
        APISetNodeValue::SetShellFragmentationStrainResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactStrainResult();
        createColorScale("破片试验\n应变分析\n单位:mm", res.metalsMinStrain, res.metalsMaxStrain, "%.6f");
    }
    else if (itemData == "fragmentationStrainPropellantResult") {
        setupResultView(m_fragmentationImpactStrainResultWidget);
        APISetNodeValue::SetPropellantFragmentationStrainResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactStrainResult();
        createColorScale("破片试验\n应变分析\n单位:mm", res.propellantsMinStrain, res.propellantsMaxStrain, "%.6f");
    }
    else if (itemData == "fragmentationTempShellResult") {
        setupResultView(m_fragmentationImpactTemperatureResultWidget);
        APISetNodeValue::SetShellFragmentationTemperatureResult(occView, nodeValues);
        // 修复原代码BUG：原代码错误调用了快速烤燃的温度数据
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactTemperatureResult();
        createColorScale("破片试验\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "fragmentationTempPropellantResult") {
        setupResultView(m_fragmentationImpactTemperatureResultWidget);
        APISetNodeValue::SetPropellantFragmentationTemperatureResult(occView, nodeValues);
        // 修复原代码BUG：原代码错误调用了快速烤燃的温度数据
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactTemperatureResult();
        createColorScale("破片试验\n温度分析\n单位:℃", res.propellantsMinTemperature, res.propellantsMaxTemperature, "%.2f");
    }
    else if (itemData == "fragmentationOverpressureShellResult") {
        setupResultView(m_fragmentationImpactOverpressureResultWidge);
        APISetNodeValue::SetShellFragmentationOverpressureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactOverpressureResult();
        createColorScale("破片试验\n超压分析\n单位:MPa", res.metalsMinOverpressure, res.metalsMaxOverpressure, "%.2f");
    }
    else if (itemData == "fragmentationOverpressurePropellantResult") {
        setupResultView(m_fragmentationImpactOverpressureResultWidge);
        APISetNodeValue::SetPropellantFragmentationOverpressureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactOverpressureResult();
        createColorScale("破片试验\n超压分析\n单位:MPa", res.propellantsMinOverpressure, res.propellantsMaxOverpressure, "%.2f");
    }
    else if (itemData == "fragmentationImpactReactionDegreePropellantResult") {
        setupResultView(m_fragmentationImpactReactionDegreeResultWidget);
        APISetNodeValue::SetPropellantFragmentationReactionDegreeNephogram(occView, nodeValues);
        occView->fitAll();
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactReactionDegreeResult();
        createColorScale("破片试验\n反应度分析", res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree, "%.2f");
    }

    // ----- 爆炸冲击波 -----
    else if (itemData == "explosiveStressShellResult") {
        setupResultView(m_explosiveBlastStressResultWidget);
        APISetNodeValue::SetShellExplosiveBlastStressResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastStressResult();
        createColorScale("爆炸冲击波试验\n应力分析\n单位:MPa", res.metalsMinStress, res.metalsMaxStress, "%.2f");
    }
    else if (itemData == "explosiveStressPropellantResult") {
        setupResultView(m_explosiveBlastStressResultWidget);
        APISetNodeValue::SetPropellantExplosiveBlastStressResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastStressResult();
        createColorScale("爆炸冲击波试验\n应力分析\n单位:MPa", res.propellantsMinStress, res.propellantsMaxStress, "%.2f");
    }
    else if (itemData == "explosiveStrainShellResult") {
        setupResultView(m_explosiveBlastStrainResultWidget);
        APISetNodeValue::SetShellExplosiveBlastStrainResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastStrainResult();
        createColorScale("爆炸冲击波试验\n应变分析\n单位:mm", res.metalsMinStrain, res.metalsMaxStrain, "%.6f");
    }
    else if (itemData == "explosiveStrainPropellantResult") {
        setupResultView(m_explosiveBlastStrainResultWidget);
        APISetNodeValue::SetPropellantExplosiveBlastStrainResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastStrainResult();
        createColorScale("爆炸冲击波试验\n应变分析\n单位:mm", res.propellantsMinStrain, res.propellantsMaxStrain, "%.6f");
    }
    else if (itemData == "explosiveTempShellResult") {
        setupResultView(m_explosiveBlastTemperatureResultWidget);
        APISetNodeValue::SetShellExplosiveBlastTemperatureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult();
        createColorScale("爆炸冲击波试验\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "explosiveTempPropellantResult") {
        setupResultView(m_explosiveBlastTemperatureResultWidget);
        APISetNodeValue::SetPropellantExplosiveBlastTemperatureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult();
        createColorScale("爆炸冲击波试验\n温度分析\n单位:℃", res.propellantsMinTemperature, res.propellantsMaxTemperature, "%.2f");
    }
    else if (itemData == "explosiveOverpressureShellResult") {
        setupResultView(m_explosiveBlastOverpressureResultWidge);
        APISetNodeValue::SetShellExplosiveBlastOverpressureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastOverpressureResult();
        createColorScale("爆炸冲击波试验\n超压分析\n单位:MPa", res.metalsMinOverpressure, res.metalsMaxOverpressure, "%.2f");
    }
    else if (itemData == "explosiveOverpressurePropellantResult") {
        setupResultView(m_explosiveBlastOverpressureResultWidge);
        APISetNodeValue::SetPropellantExplosiveBlastOverpressureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastOverpressureResult();
        createColorScale("爆炸冲击波试验\n超压分析\n单位:MPa", res.propellantsMinOverpressure, res.propellantsMaxOverpressure, "%.2f");
    }
    else if (itemData == "explosiveBlastReactionDegreePropellantResult") {
        setupResultView(m_explosiveBlastReactionDegreeResultWidget);
        APISetNodeValue::SetPropellantExplosiveBlastReactionDegreeNephogram(occView, nodeValues);
        occView->fitAll();
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastReactionDegreeResult();
        createColorScale("爆炸冲击波试验\n反应度分析", res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree, "%.2f");
    }

    // ----- 殉爆试验 -----
    else if (itemData == "sacrificeStressShellResult") {
        setupResultView(m_sacrificeExplosionStressResultWidget);
        APISetNodeValue::SetShellSacrificeExplosionStressResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionStressResult();
        createColorScale("殉爆试验\n应力分析\n单位:MPa", res.metalsMinStress, res.metalsMaxStress, "%.2f");
    }
    else if (itemData == "sacrificeStressPropellantResult") {
        setupResultView(m_sacrificeExplosionStressResultWidget);
        APISetNodeValue::SetPropellantSacrificeExplosionStressResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionStressResult();
        createColorScale("殉爆试验\n应力分析\n单位:MPa", res.propellantsMinStress, res.propellantsMaxStress, "%.2f");
    }
    else if (itemData == "sacrificeStrainShellResult") {
        setupResultView(m_sacrificeExplosionStrainResultWidget);
        APISetNodeValue::SetShellSacrificeExplosionStrainResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionStrainResult();
        createColorScale("殉爆试验\n应变分析\n单位:mm", res.metalsMinStrain, res.metalsMaxStrain, "%.6f");
    }
    else if (itemData == "sacrificeStrainPropellantResult") {
        setupResultView(m_sacrificeExplosionStrainResultWidget);
        APISetNodeValue::SetPropellantSacrificeExplosionStrainResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionStrainResult();
        createColorScale("殉爆试验\n应变分析\n单位:mm", res.propellantsMinStrain, res.propellantsMaxStrain, "%.6f");
    }
    else if (itemData == "sacrificeTempShellResult") {
        setupResultView(m_sacrificeExplosionTemperatureResultWidget);
        APISetNodeValue::SetShellSacrificeExplosionTemperatureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionTemperatureResult();
        createColorScale("殉爆试验\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "sacrificeTempPropellantResult") {
        setupResultView(m_sacrificeExplosionTemperatureResultWidget);
        APISetNodeValue::SetPropellantSacrificeExplosionTemperatureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionTemperatureResult();
        createColorScale("殉爆试验\n温度分析\n单位:℃", res.propellantsMinTemperature, res.propellantsMaxTemperature, "%.2f");
    }
    else if (itemData == "sacrificeOverpressureShellResult") {
        setupResultView(m_sacrificeExplosionOverpressureResultWidge);
        APISetNodeValue::SetShellSacrificeExplosionOverpressureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionOverpressureResult();
        createColorScale("殉爆试验\n超压分析\n单位:MPa", res.metalsMinOverpressure, res.metalsMaxOverpressure, "%.2f");
    }
    else if (itemData == "sacrificeOverpressurePropellantResult") {
        setupResultView(m_sacrificeExplosionOverpressureResultWidge);
        APISetNodeValue::SetPropellantSacrificeExplosionOverpressureResult(occView, nodeValues);
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionOverpressureResult();
        createColorScale("殉爆试验\n超压分析\n单位:MPa", res.propellantsMinOverpressure, res.propellantsMaxOverpressure, "%.2f");
    }
    else if (itemData == "sacrificeExplosionDegreePropellantResult") {
        setupResultView(m_sacrificeExplosionReactionDegreeResultWidget);
        APISetNodeValue::SetPropellantSacrificeExplosionReactionDegreeNephogram(occView, nodeValues);
        occView->fitAll();
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionReactionDegreeResult();
        createColorScale("殉爆试验\n反应度分析", res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree, "%.2f");
    }

    // 统一重绘视图
    view->Redraw();
}


