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
    // ========== 1. 重复点击拦截 ==========
    if (itemData == m_lastClickedItemKey) {
        return;
    }
    m_lastClickedItemKey = itemData;

    auto occView = GetOccView();
    if (!occView) return;
    Handle(AIS_InteractiveContext) context = occView->getContext();
    Handle(V3d_View) view = occView->getView();

    // ========== 2. 通用 Lambda 工具 ==========
    auto clearSceneAuxObjects = [&]() {
        AIS_ListOfInteractive displayedList;
        context->DisplayedObjects(displayedList);
        for (AIS_ListIteratorOfListOfInteractive it(displayedList); it.More(); it.Next()) {
            Handle(AIS_InteractiveObject) obj = it.Value();
            if (obj->IsKind(STANDARD_TYPE(MeshVS_Mesh)) ||
                obj->IsKind(STANDARD_TYPE(AIS_ColorScale)) ||
                (obj->IsKind(STANDARD_TYPE(AIS_Shape)) &&
                    !Handle(AIS_Shape)::DownCast(obj).IsNull() &&
                    std::abs(Handle(AIS_Shape)::DownCast(obj)->Transparency() - 0.9) < 1e-6)) {
                context->Erase(obj, Standard_False);
            }
        }
    };

    auto createColorScale = [&](const QString& title, double minVal, double maxVal, const char* format) {
        QByteArray utf8 = title.toUtf8();
        Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
        aColorScale->SetFormat(TCollection_AsciiString(format));
        aColorScale->SetSize(100, 400);
        aColorScale->SetRange(minVal, maxVal);
        aColorScale->SetNumberOfIntervals(9);
        aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
        aColorScale->SetTextHeight(14);
        aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
        aColorScale->SetTitle(TCollection_ExtendedString(utf8.constData(), Standard_True));
        aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
        aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
        aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
        Graphic3d_Vec2i offset(0, 450);
        context->SetTransformPersistence(aColorScale,
            new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, offset));
        context->SetDisplayMode(aColorScale, 1, Standard_False);
        context->Display(aColorScale, Standard_True);
    };

    auto setupResultView = [&](QWidget* page) {
        occView->SetCameraRotationState(false);
        m_PropertyStackWidget->setCurrentWidget(page);
        clearSceneAuxObjects();
        view->SetProj(V3d_Zneg);
    };

    // ========== 3. 辅助网格构建函数（消除重复代码） ==========
    auto buildShellNozzlePropellantMesh = [&](const std::vector<double>& shellVals,
        const std::vector<double>& nozzleVals,
        const std::vector<double>& propVals,
        double minVal, double maxVal,
        double angle,
        Handle(MeshVS_Mesh)& shellMesh,
        Handle(MeshVS_Mesh)& nozzleMesh,
        Handle(MeshVS_Mesh)& propellantMesh) {
            auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
            auto geo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
            double cx = (geo.theXmin + geo.theXmax) / 2.0;
            double cy = (geo.theYmin + geo.theYmax) / 2.0;

            // 壳体
            if (!meshInfo.shellMesh.IsNull() && !shellVals.empty()) {
                shellMesh = new MeshVS_Mesh();
                shellMesh->SetDataSource(meshInfo.shellMesh->RotateXY(angle, cx, cy));
                auto colorMap = APISetNodeValue::GetMeshDataMap(shellVals, minVal, maxVal);
                Handle(MeshVS_NodalColorPrsBuilder) builder = new MeshVS_NodalColorPrsBuilder(shellMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
                builder->SetColors(colorMap);
                shellMesh->AddBuilder(builder);
                shellMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
            }

            // 喷嘴
            if (!meshInfo.nozzleMesh.IsNull() && !nozzleVals.empty()) {
                nozzleMesh = new MeshVS_Mesh();
                nozzleMesh->SetDataSource(meshInfo.nozzleMesh->RotateXY(angle, cx, cy));
                auto colorMap = APISetNodeValue::GetMeshDataMap(nozzleVals, minVal, maxVal);
                Handle(MeshVS_NodalColorPrsBuilder) builder = new MeshVS_NodalColorPrsBuilder(nozzleMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
                builder->SetColors(colorMap);
                nozzleMesh->AddBuilder(builder);
                nozzleMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
            }

            // 推进剂（填充 -1）
            if (!meshInfo.propellantMesh.IsNull()) {
                propellantMesh = new MeshVS_Mesh();
                propellantMesh->SetDataSource(meshInfo.propellantMesh->RotateXY(angle, cx, cy));
                std::vector<double> fillVals(propVals.size(), -1.0);
                auto colorMap = APISetNodeValue::GetMeshDataMap(fillVals, minVal, maxVal);
                Handle(MeshVS_NodalColorPrsBuilder) builder = new MeshVS_NodalColorPrsBuilder(propellantMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
                builder->SetColors(colorMap);
                propellantMesh->AddBuilder(builder);
                propellantMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
            }
    };

    auto buildPropellantOnlyMesh = [&](const std::vector<double>& propVals,
        double minVal, double maxVal,
        double angle,
        Handle(MeshVS_Mesh)& propellantMesh) {
            auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
            auto geo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
            if (meshInfo.propellantMesh.IsNull() || propVals.empty()) return;
            double cx = (geo.theXmin + geo.theXmax) / 2.0;
            double cy = (geo.theYmin + geo.theYmax) / 2.0;

            propellantMesh = new MeshVS_Mesh();
            propellantMesh->SetDataSource(meshInfo.propellantMesh->RotateXY(angle, cx, cy));
            auto colorMap = APISetNodeValue::GetMeshDataMap(propVals, minVal, maxVal);
            Handle(MeshVS_NodalColorPrsBuilder) builder = new MeshVS_NodalColorPrsBuilder(propellantMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
            builder->SetColors(colorMap);
            propellantMesh->AddBuilder(builder);
            propellantMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
    };

    // ========== 4. 页面切换映射表 ==========
    struct PageSwitchConfig { QWidget* page; bool enableRotation; };
    static const QMap<QString, PageSwitchConfig> pageConfigMap = {
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
        {"FallAnalysis", {m_fallPropertyWidget, false}},
        {"FastCombustionAnalysis", {m_fastCombustionPropertyWidget, false}},
        {"SlowCombustionAnalysis", {m_slowCombustionPropertyWidget, false}},
        {"ShootAnalysis", {m_shootPropertyWidget, false}},
        {"JetImpactAnalysis", {m_jetImpactPropertyWidget, false}},
        {"FragmentationImpactAnalysis", {m_fragmentationImpactPropertyWidget, false}},
        {"ExplosiveBlastAnalysis", {m_explosiveBlastPropertyWidget, false}},
        {"SacrificeExplosionAnalysis", {m_sacrificeExplosionPropertyWidget, false}},
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

    // ========== 5. 几何与网格节点 ==========
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

    // ========== 6. 云图结果节点（全部使用辅助函数，统一角度旋转） ==========
    // ----- 跌落试验 -----
    if (itemData == "FallStressShellResult") {
        setupResultView(m_stressResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFallStressResult();
        auto angle = ModelDataManager::GetInstance()->GetFallSettingInfo().angle;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellStressNodeValues, info.nozzleStressNodeValues,
            info.propellantStressNodeValues,
            res.metalsMinStress, res.metalsMaxStress,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("跌落试验\n应力分析\n单位:MPa", res.metalsMinStress, res.metalsMaxStress, "%.2f");
    }
    else if (itemData == "FallStressPropellantResult") {
        setupResultView(m_stressResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFallStressResult();

        double maxValue = std::max(res.propellantsMaxStress, res.insulatingheatMaxStress);

        auto angle = ModelDataManager::GetInstance()->GetFallSettingInfo().angle;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantStressNodeValues,
            res.propellantsMinStress, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("跌落试验\n应力分析\n单位:MPa", res.propellantsMinStress, maxValue, "%.2f");
    }
    else if (itemData == "FallStrainShellResult") {
        setupResultView(m_strainResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFallStrainResult();
        auto angle = ModelDataManager::GetInstance()->GetFallSettingInfo().angle;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellStrainNodeValues, info.nozzleStrainNodeValues,
            info.propellantStrainNodeValues,
            res.metalsMinStrain, res.metalsMaxStrain,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("跌落试验\n应变分析", res.metalsMinStrain, res.metalsMaxStrain, "%.6f");
    }
    else if (itemData == "FallStrainPropellantResult") {
        setupResultView(m_strainResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFallStrainResult();

        double maxValue = std::max(res.propellantsMaxStrain, res.insulatingheatMaxStrain);

        auto angle = ModelDataManager::GetInstance()->GetFallSettingInfo().angle;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantStrainNodeValues,
            res.propellantsMinStrain, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("跌落试验\n应变分析", res.propellantsMinStrain, maxValue, "%.6f");
    }
    else if (itemData == "FallTemperatureShellResult") {
        setupResultView(m_temperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFallTemperatureResult();
        auto angle = ModelDataManager::GetInstance()->GetFallSettingInfo().angle;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellTemperatureNodeValues, info.nozzleTemperatureNodeValues,
            info.propellantTemperatureNodeValues,
            res.metalsMinTemperature, res.metalsMaxTemperature,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("跌落试验\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "FallTemperaturePropellantResult") {
        setupResultView(m_temperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFallTemperatureResult();
        auto angle = ModelDataManager::GetInstance()->GetFallSettingInfo().angle;

        double maxTemperature = std::max(res.propellantsMaxTemperature, res.insulatingheatMaxTemperature);

        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantTemperatureNodeValues,
            res.propellantsMinTemperature, maxTemperature,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("跌落试验\n温度分析\n单位:℃", res.propellantsMinTemperature, maxTemperature, "%.2f");
    }
    else if (itemData == "FallOverpressureShellResult") {
        setupResultView(m_overpressureResultWidge);
        auto info = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFallOverpressureResult();
        auto angle = ModelDataManager::GetInstance()->GetFallSettingInfo().angle;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellOverpressureNodeValues, info.nozzleOverpressureNodeValues,
            info.propellantOverpressureNodeValues,
            res.metalsMinOverpressure, res.metalsMaxOverpressure,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("跌落试验\n超压分析\n单位:MPa", res.metalsMinOverpressure, res.metalsMaxOverpressure, "%.2f");
    }
    else if (itemData == "FallOverpressurePropellantResult") {
        setupResultView(m_overpressureResultWidge);
        auto info = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFallOverpressureResult();

        double maxValue = std::max(res.propellantsMaxOverpressure, res.insulatingheatMaxOverpressure);

        auto angle = ModelDataManager::GetInstance()->GetFallSettingInfo().angle;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantOverpressureNodeValues,
            res.propellantsMinOverpressure, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("跌落试验\n超压分析\n单位:MPa", res.propellantsMinOverpressure, maxValue, "%.2f");
    }
    else if (itemData == "reactionDegreePropellantResult") {
        setupResultView(m_reactionDegreeResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFallReactionDegreeResult();
        auto angle = ModelDataManager::GetInstance()->GetFallSettingInfo().angle;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantReactionDegreeNodeValues,
            res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("跌落试验\n反应度分析", res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree, "%.2f");
    }

    // ----- 快速烤燃 -----
    else if (itemData == "fastTemperatureShellResult") {
        setupResultView(m_fastCombustionTemperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFastCombustionAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFastCombustionTemperatureResult();
        //auto angle = ModelDataManager::GetInstance()->GetFastCombustionSettingInfo().angle;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellTemperatureNodeValues, info.nozzleTemperatureNodeValues,
            info.propellantTemperatureNodeValues,
            res.metalsMinTemperature, res.metalsMaxTemperature,
            90, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("快速烤燃\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "fastTemperaturePropellantResult") {
        setupResultView(m_fastCombustionTemperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFastCombustionAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFastCombustionTemperatureResult();
        //auto angle = ModelDataManager::GetInstance()->GetFastCombustionSettingInfo().angle;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantTemperatureNodeValues,
            res.propellantsMinTemperature, res.propellantsMaxTemperature,
            90, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("快速烤燃\n温度分析\n单位:℃", res.propellantsMinTemperature, res.propellantsMaxTemperature, "%.2f");
    }

    // ----- 慢速烤燃 -----
    else if (itemData == "slowTemperatureShellResult") {
        setupResultView(m_slowCombustionTemperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetSlowCombustionAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetSlowCombustionTemperatureResult();
        //auto angle = ModelDataManager::GetInstance()->GetSlowCombustionSettingInfo().angle;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellTemperatureNodeValues, info.nozzleTemperatureNodeValues,
            info.propellantTemperatureNodeValues,
            res.metalsMinTemperature, res.metalsMaxTemperature,
            90, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("慢速烤燃\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "slowTemperaturePropellantResult") {
        setupResultView(m_slowCombustionTemperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetSlowCombustionAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetSlowCombustionTemperatureResult();
        //auto angle = ModelDataManager::GetInstance()->GetSlowCombustionSettingInfo().angle;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantTemperatureNodeValues,
            res.propellantsMinTemperature, res.propellantsMaxTemperature,
            90, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("慢速烤燃\n温度分析\n单位:℃", res.propellantsMinTemperature, res.propellantsMaxTemperature, "%.2f");
    }

    // ----- 枪击试验 -----
    else if (itemData == "shootStressShellResult") {
        setupResultView(m_shootStressResultWidget);
        auto info = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetShootStressResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellStressNodeValues, info.nozzleStressNodeValues,
            info.propellantStressNodeValues,
            res.metalsMinStress, res.metalsMaxStress,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("枪击试验\n应力分析\n单位:MPa", res.metalsMinStress, res.metalsMaxStress, "%.2f");
    }
    else if (itemData == "shootStressPropellantResult") {
        setupResultView(m_shootStressResultWidget);
        auto info = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetShootStressResult();

        double maxValue = std::max(res.propellantsMaxStress, res.insulatingheatMaxStress);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantStressNodeValues,
            res.propellantsMinStress, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("枪击试验\n应力分析\n单位:MPa", res.propellantsMinStress, maxValue, "%.2f");
    }
    else if (itemData == "shootStrainShellResult") {
        setupResultView(m_shootStrainResultWidget);
        auto info = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetShootStrainResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellStrainNodeValues, info.nozzleStrainNodeValues,
            info.propellantStrainNodeValues,
            res.metalsMinStrain, res.metalsMaxStrain,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("枪击试验\n应变分析", res.metalsMinStrain, res.metalsMaxStrain, "%.6f");
    }
    else if (itemData == "shootStrainPropellantResult") {
        setupResultView(m_shootStrainResultWidget);
        auto info = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetShootStrainResult();

        double maxValue = std::max(res.propellantsMaxStrain, res.insulatingheatMaxStrain);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantStrainNodeValues,
            res.propellantsMinStrain, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("枪击试验\n应变分析", res.propellantsMinStrain, maxValue, "%.6f");
    }
    else if (itemData == "shootTempShellResult") {
        setupResultView(m_shootTemperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetShootTemperatureResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellTemperatureNodeValues, info.nozzleTemperatureNodeValues,
            info.propellantTemperatureNodeValues,
            res.metalsMinTemperature, res.metalsMaxTemperature,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("枪击试验\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "shootTempPropellantResult") {
        setupResultView(m_shootTemperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetShootTemperatureResult();

        double maxTemperature = std::max(res.propellantsMaxTemperature, res.insulatingheatMaxTemperature);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantTemperatureNodeValues,
            res.propellantsMinTemperature, maxTemperature,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("枪击试验\n温度分析\n单位:℃", res.propellantsMinTemperature, maxTemperature, "%.2f");
    }
    else if (itemData == "shootOverpressureShellResult") {
        setupResultView(m_shootOverpressureResultWidge);
        auto info = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetShootOverpressureResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellOverpressureNodeValues, info.nozzleOverpressureNodeValues,
            info.propellantOverpressureNodeValues,
            res.metalsMinOverpressure, res.metalsMaxOverpressure,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("枪击试验\n超压分析\n单位:MPa", res.metalsMinOverpressure, res.metalsMaxOverpressure, "%.2f");
    }
    else if (itemData == "shootOverpressurePropellantResult") {
        setupResultView(m_shootOverpressureResultWidge);
        auto info = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetShootOverpressureResult();

        double maxValue = std::max(res.propellantsMaxOverpressure, res.insulatingheatMaxOverpressure);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantOverpressureNodeValues,
            res.propellantsMinOverpressure, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("枪击试验\n超压分析\n单位:MPa", res.propellantsMinOverpressure, maxValue, "%.2f");
    }
    else if (itemData == "shootReactionDegreePropellantResult") {
        setupResultView(m_shootReactionDegreeResultWidget);
        auto info = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetShootReactionDegreeResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantReactionDegreeNodeValues,
            res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("枪击试验\n反应度分析", res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree, "%.2f");
    }

    // ----- 射流冲击 -----
    else if (itemData == "jetStressShellResult") {
        setupResultView(m_jetImpactStressResultWidget);
        auto info = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetJetImpactStressResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellStressNodeValues, info.nozzleStressNodeValues,
            info.propellantStressNodeValues,
            res.metalsMinStress, res.metalsMaxStress,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("射流冲击试验\n应力分析\n单位:MPa", res.metalsMinStress, res.metalsMaxStress, "%.2f");
    }
    else if (itemData == "jetStressPropellantResult") {
        setupResultView(m_jetImpactStressResultWidget);
        auto info = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetJetImpactStressResult();

        double maxValue = std::max(res.propellantsMaxStress, res.insulatingheatMaxStress);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantStressNodeValues,
            res.propellantsMinStress, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("射流冲击试验\n应力分析\n单位:MPa", res.propellantsMinStress, maxValue, "%.2f");
    }
    else if (itemData == "jetStrainShellResult") {
        setupResultView(m_jetImpactStrainResultWidget);
        auto info = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetJetImpactStrainResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellStrainNodeValues, info.nozzleStrainNodeValues,
            info.propellantStrainNodeValues,
            res.metalsMinStrain, res.metalsMaxStrain,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("射流冲击试验\n应变分析", res.metalsMinStrain, res.metalsMaxStrain, "%.6f");
    }
    else if (itemData == "jetStrainPropellantResult") {
        setupResultView(m_jetImpactStrainResultWidget);
        auto info = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetJetImpactStrainResult();

        double maxValue = std::max(res.propellantsMaxStrain, res.insulatingheatMaxStrain);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantStrainNodeValues,
            res.propellantsMinStrain, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("射流冲击试验\n应变分析", res.propellantsMinStrain, maxValue, "%.6f");
    }
    else if (itemData == "jetTempShellResult") {
        setupResultView(m_jetImpactTemperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetJetImpactTemperatureResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellTemperatureNodeValues, info.nozzleTemperatureNodeValues,
            info.propellantTemperatureNodeValues,
            res.metalsMinTemperature, res.metalsMaxTemperature,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("射流冲击试验\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "jetTempPropellantResult") {
        setupResultView(m_jetImpactTemperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetJetImpactTemperatureResult();

        double maxTemperature = std::max(res.propellantsMaxTemperature, res.insulatingheatMaxTemperature);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantTemperatureNodeValues,
            res.propellantsMinTemperature, maxTemperature,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("射流冲击试验\n温度分析\n单位:℃", res.propellantsMinTemperature, maxTemperature, "%.2f");
    }
    else if (itemData == "jetOverpressureShellResult") {
        setupResultView(m_jetImpactOverpressureResultWidge);
        auto info = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetJetImpactOverpressureResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellOverpressureNodeValues, info.nozzleOverpressureNodeValues,
            info.propellantOverpressureNodeValues,
            res.metalsMinOverpressure, res.metalsMaxOverpressure,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("射流冲击试验\n超压分析\n单位:MPa", res.metalsMinOverpressure, res.metalsMaxOverpressure, "%.2f");
    }
    else if (itemData == "jetOverpressurePropellantResult") {
        setupResultView(m_jetImpactOverpressureResultWidge);
        auto info = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetJetImpactOverpressureResult();

        double maxValue = std::max(res.propellantsMaxOverpressure, res.insulatingheatMaxOverpressure);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantOverpressureNodeValues,
            res.propellantsMinOverpressure, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("射流冲击试验\n超压分析\n单位:MPa", res.propellantsMinOverpressure, maxValue, "%.2f");
    }
    else if (itemData == "jetImpactReactionDegreePropellantResult") {
        setupResultView(m_jetImpactReactionDegreeResultWidget);
        auto info = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetJetImpactReactionDegreeResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantReactionDegreeNodeValues,
            res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("射流冲击试验\n反应度分析", res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree, "%.2f");
    }

    // ----- 破片试验 -----
    else if (itemData == "fragmentationStressShellResult") {
        setupResultView(m_fragmentationImpactStressResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactStressResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellStressNodeValues, info.nozzleStressNodeValues,
            info.propellantStressNodeValues,
            res.metalsMinStress, res.metalsMaxStress,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("破片试验\n应力分析\n单位:MPa", res.metalsMinStress, res.metalsMaxStress, "%.2f");
    }
    else if (itemData == "fragmentationStressPropellantResult") {
        setupResultView(m_fragmentationImpactStressResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactStressResult();

        double maxValue = std::max(res.propellantsMaxStress, res.insulatingheatMaxStress);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantStressNodeValues,
            res.propellantsMinStress, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("破片试验\n应力分析\n单位:MPa", res.propellantsMinStress, maxValue, "%.2f");
    }
    else if (itemData == "fragmentationStrainShellResult") {
        setupResultView(m_fragmentationImpactStrainResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactStrainResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellStrainNodeValues, info.nozzleStrainNodeValues,
            info.propellantStrainNodeValues,
            res.metalsMinStrain, res.metalsMaxStrain,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("破片试验\n应变分析", res.metalsMinStrain, res.metalsMaxStrain, "%.6f");
    }
    else if (itemData == "fragmentationStrainPropellantResult") {
        setupResultView(m_fragmentationImpactStrainResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactStrainResult();

        double maxValue = std::max(res.propellantsMaxStrain, res.insulatingheatMaxStrain);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantStrainNodeValues,
            res.propellantsMinStrain, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("破片试验\n应变分析", res.propellantsMinStrain, maxValue, "%.6f");
    }
    else if (itemData == "fragmentationTempShellResult") {
        setupResultView(m_fragmentationImpactTemperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactTemperatureResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellTemperatureNodeValues, info.nozzleTemperatureNodeValues,
            info.propellantTemperatureNodeValues,
            res.metalsMinTemperature, res.metalsMaxTemperature,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("破片试验\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "fragmentationTempPropellantResult") {
        setupResultView(m_fragmentationImpactTemperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactTemperatureResult();

        double maxTemperature = std::max(res.propellantsMaxTemperature, res.insulatingheatMaxTemperature);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantTemperatureNodeValues,
            res.propellantsMinTemperature, maxTemperature,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("破片试验\n温度分析\n单位:℃", res.propellantsMinTemperature, maxTemperature, "%.2f");
    }
    else if (itemData == "fragmentationOverpressureShellResult") {
        setupResultView(m_fragmentationImpactOverpressureResultWidge);
        auto info = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactOverpressureResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellOverpressureNodeValues, info.nozzleOverpressureNodeValues,
            info.propellantOverpressureNodeValues,
            res.metalsMinOverpressure, res.metalsMaxOverpressure,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("破片试验\n超压分析\n单位:MPa", res.metalsMinOverpressure, res.metalsMaxOverpressure, "%.2f");
    }
    else if (itemData == "fragmentationOverpressurePropellantResult") {
        setupResultView(m_fragmentationImpactOverpressureResultWidge);
        auto info = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactOverpressureResult();

        double maxValue = std::max(res.propellantsMaxOverpressure, res.insulatingheatMaxOverpressure);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantOverpressureNodeValues,
            res.propellantsMinOverpressure, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("破片试验\n超压分析\n单位:MPa", res.propellantsMinOverpressure, maxValue, "%.2f");
    }
    else if (itemData == "fragmentationImpactReactionDegreePropellantResult") {
        setupResultView(m_fragmentationImpactReactionDegreeResultWidget);
        auto info = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetFragmentationImpactReactionDegreeResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantReactionDegreeNodeValues,
            res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("破片试验\n反应度分析", res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree, "%.2f");
    }

    // ----- 爆炸冲击波 -----
    else if (itemData == "explosiveStressShellResult") {
        setupResultView(m_explosiveBlastStressResultWidget);
        auto info = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastStressResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellStressNodeValues, info.nozzleStressNodeValues,
            info.propellantStressNodeValues,
            res.metalsMinStress, res.metalsMaxStress,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("爆炸冲击波试验\n应力分析\n单位:MPa", res.metalsMinStress, res.metalsMaxStress, "%.2f");
    }
    else if (itemData == "explosiveStressPropellantResult") {
        setupResultView(m_explosiveBlastStressResultWidget);
        auto info = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastStressResult();

        double maxValue = std::max(res.propellantsMaxStress, res.insulatingheatMaxStress);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantStressNodeValues,
            res.propellantsMinStress, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("爆炸冲击波试验\n应力分析\n单位:MPa", res.propellantsMinStress, maxValue, "%.2f");
    }
    else if (itemData == "explosiveStrainShellResult") {
        setupResultView(m_explosiveBlastStrainResultWidget);
        auto info = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastStrainResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellStrainNodeValues, info.nozzleStrainNodeValues,
            info.propellantStrainNodeValues,
            res.metalsMinStrain, res.metalsMaxStrain,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("爆炸冲击波试验\n应变分析", res.metalsMinStrain, res.metalsMaxStrain, "%.6f");
    }
    else if (itemData == "explosiveStrainPropellantResult") {
        setupResultView(m_explosiveBlastStrainResultWidget);
        auto info = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastStrainResult();

        double maxValue = std::max(res.propellantsMaxStrain, res.insulatingheatMaxStrain);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantStrainNodeValues,
            res.propellantsMinStrain, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("爆炸冲击波试验\n应变分析", res.propellantsMinStrain, maxValue, "%.6f");
    }
    else if (itemData == "explosiveTempShellResult") {
        setupResultView(m_explosiveBlastTemperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellTemperatureNodeValues, info.nozzleTemperatureNodeValues,
            info.propellantTemperatureNodeValues,
            res.metalsMinTemperature, res.metalsMaxTemperature,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("爆炸冲击波试验\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "explosiveTempPropellantResult") {
        setupResultView(m_explosiveBlastTemperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult();

        double maxTemperature = std::max(res.propellantsMaxTemperature, res.insulatingheatMaxTemperature);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantTemperatureNodeValues,
            res.propellantsMinTemperature, maxTemperature,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("爆炸冲击波试验\n温度分析\n单位:℃", res.propellantsMinTemperature, maxTemperature, "%.2f");
    }
    else if (itemData == "explosiveOverpressureShellResult") {
        setupResultView(m_explosiveBlastOverpressureResultWidge);
        auto info = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastOverpressureResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellOverpressureNodeValues, info.nozzleOverpressureNodeValues,
            info.propellantOverpressureNodeValues,
            res.metalsMinOverpressure, res.metalsMaxOverpressure,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("爆炸冲击波试验\n超压分析\n单位:MPa", res.metalsMinOverpressure, res.metalsMaxOverpressure, "%.2f");
    }
    else if (itemData == "explosiveOverpressurePropellantResult") {
        setupResultView(m_explosiveBlastOverpressureResultWidge);
        auto info = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastOverpressureResult();

        double maxValue = std::max(res.propellantsMaxOverpressure, res.insulatingheatMaxOverpressure);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantOverpressureNodeValues,
            res.propellantsMinOverpressure, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("爆炸冲击波试验\n超压分析\n单位:MPa", res.propellantsMinOverpressure, maxValue, "%.2f");
    }
    else if (itemData == "explosiveBlastReactionDegreePropellantResult") {
        setupResultView(m_explosiveBlastReactionDegreeResultWidget);
        auto info = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetExplosiveBlastReactionDegreeResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantReactionDegreeNodeValues,
            res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("爆炸冲击波试验\n反应度分析", res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree, "%.2f");
    }

    // ----- 殉爆试验 -----
    else if (itemData == "sacrificeStressShellResult") {
        setupResultView(m_sacrificeExplosionStressResultWidget);
        auto info = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionStressResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellStressNodeValues, info.nozzleStressNodeValues,
            info.propellantStressNodeValues,
            res.metalsMinStress, res.metalsMaxStress,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("殉爆试验\n应力分析\n单位:MPa", res.metalsMinStress, res.metalsMaxStress, "%.2f");
    }
    else if (itemData == "sacrificeStressPropellantResult") {
        setupResultView(m_sacrificeExplosionStressResultWidget);
        auto info = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionStressResult();

        double maxValue = std::max(res.propellantsMaxStress, res.insulatingheatMaxStress);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantStressNodeValues,
            res.propellantsMinStress, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("殉爆试验\n应力分析\n单位:MPa", res.propellantsMinStress, maxValue, "%.2f");
    }
    else if (itemData == "sacrificeStrainShellResult") {
        setupResultView(m_sacrificeExplosionStrainResultWidget);
        auto info = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionStrainResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellStrainNodeValues, info.nozzleStrainNodeValues,
            info.propellantStrainNodeValues,
            res.metalsMinStrain, res.metalsMaxStrain,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("殉爆试验\n应变分析", res.metalsMinStrain, res.metalsMaxStrain, "%.6f");
    }
    else if (itemData == "sacrificeStrainPropellantResult") {
        setupResultView(m_sacrificeExplosionStrainResultWidget);
        auto info = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionStrainResult();

        double maxValue = std::max(res.propellantsMaxStrain, res.insulatingheatMaxStrain);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantStrainNodeValues,
            res.propellantsMinStrain, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("殉爆试验\n应变分析", res.propellantsMinStrain, maxValue, "%.6f");
    }
    else if (itemData == "sacrificeTempShellResult") {
        setupResultView(m_sacrificeExplosionTemperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionTemperatureResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellTemperatureNodeValues, info.nozzleTemperatureNodeValues,
            info.propellantTemperatureNodeValues,
            res.metalsMinTemperature, res.metalsMaxTemperature,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("殉爆试验\n温度分析\n单位:℃", res.metalsMinTemperature, res.metalsMaxTemperature, "%.2f");
    }
    else if (itemData == "sacrificeTempPropellantResult") {
        setupResultView(m_sacrificeExplosionTemperatureResultWidget);
        auto info = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionTemperatureResult();

        double maxTemperature = std::max(res.propellantsMaxTemperature, res.insulatingheatMaxTemperature);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantTemperatureNodeValues,
            res.propellantsMinTemperature, maxTemperature,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("殉爆试验\n温度分析\n单位:℃", res.propellantsMinTemperature, maxTemperature, "%.2f");
    }
    else if (itemData == "sacrificeOverpressureShellResult") {
        setupResultView(m_sacrificeExplosionOverpressureResultWidge);
        auto info = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionOverpressureResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) shellMesh, nozzleMesh, propellantMesh;
        buildShellNozzlePropellantMesh(info.shellOverpressureNodeValues, info.nozzleOverpressureNodeValues,
            info.propellantOverpressureNodeValues,
            res.metalsMinOverpressure, res.metalsMaxOverpressure,
            angle, shellMesh, nozzleMesh, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        context->Display(shellMesh, Standard_True);
        context->Display(nozzleMesh, Standard_True);
        occView->fitAll();
        createColorScale("殉爆试验\n超压分析\n单位:MPa", res.metalsMinOverpressure, res.metalsMaxOverpressure, "%.2f");
    }
    else if (itemData == "sacrificeOverpressurePropellantResult") {
        setupResultView(m_sacrificeExplosionOverpressureResultWidge);
        auto info = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionOverpressureResult();

        double maxValue = std::max(res.propellantsMaxOverpressure, res.insulatingheatMaxOverpressure);

        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantOverpressureNodeValues,
            res.propellantsMinOverpressure, maxValue,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("殉爆试验\n超压分析\n单位:MPa", res.propellantsMinOverpressure, maxValue, "%.2f");
    }
    else if (itemData == "sacrificeExplosionDegreePropellantResult") {
        setupResultView(m_sacrificeExplosionReactionDegreeResultWidget);
        auto info = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();
        auto res = ModelDataManager::GetInstance()->GetSacrificeExplosionReactionDegreeResult();
        auto angle = 90;
        Handle(MeshVS_Mesh) propellantMesh;
        buildPropellantOnlyMesh(info.propellantReactionDegreeNodeValues,
            res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree,
            angle, propellantMesh);
        context->EraseAll(true);
        context->Display(propellantMesh, Standard_True);
        occView->fitAll();
        createColorScale("殉爆试验\n反应度分析", res.propellantsMinReactionDegree, res.propellantsMaxReactionDegree, "%.2f");
    }

    // ========== 7. 统一重绘 ==========
    view->Redraw();
}


