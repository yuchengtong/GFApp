#pragma execution_character_set("utf-8")
#include "GeometryImportWorker.h"
#include <STEPControl_Reader.hxx>
#include <StlAPI_Reader.hxx>
#include <IGESControl_Reader.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <QThread>
#include <STEPCAFControl_Reader.hxx>
#include <BRep_Builder.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFApp_Application.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TDataStd_Name.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRepAdaptor_Curve.hxx>



void GeometryImportWorker::DoWork()
{
    ModelGeometryInfo info;
    bool success = false;
    QString msg;

    try
    {
        emit StatusUpdated("开始解析文件...");
        emit ProgressUpdated(10);

        if (m_interrupted)
        {
            emit WorkFinished(false, "导入已取消", info);
            return;
        }

        // 根据文件后缀选择导入方式
        if (m_filePath.endsWith(".stp", Qt::CaseInsensitive) ||
            m_filePath.endsWith(".step", Qt::CaseInsensitive))
        {
            success = ImportSTEP(info);
        }
        else if (m_filePath.endsWith(".stl", Qt::CaseInsensitive))
        {
            success = ImportSTL(info);
        }
        else if (m_filePath.endsWith(".iges", Qt::CaseInsensitive) ||
            m_filePath.endsWith(".igs", Qt::CaseInsensitive))
        {
            success = ImportIGES(info);
        }
        else
        {
            msg = "不支持的文件格式";
            success = false;
        }

        // 在判断成功之前检查中断
        if (m_interrupted)
        {
            emit WorkFinished(false, "导入已取消", info);
            return;
        }

        if (success)
        {
            emit StatusUpdated("计算模型边界盒...");
            emit ProgressUpdated(80);

            // 在计算边界框前检查中断
            if (m_interrupted)
            {
                emit WorkFinished(false, "导入已取消", info);
                return;
            }

            CalculateBoundingBox(info);

            if (m_partType == PartType::Shell)
            {
                AnalyzeGeometry(info);
            }


            info.path = m_filePath;
            msg = "几何模型导入成功";
            emit ProgressUpdated(100);
        }
        else
        {
            msg = "几何模型导入失败";
        }
    }
    catch (const Standard_Failure& e)
    {
        if (m_interrupted)
        {
            msg = "导入已取消";
            success = false;
        }
        else
        {
            msg = QString("导入错误: %1").arg(e.GetMessageString());
            success = false;
        }
    }
    catch (...)
    {
        if (m_interrupted)
        {
            msg = "导入已取消";
            success = false;
        }
        else
        {
            msg = "导入时发生未知错误";
            success = false;
        }
    }

    emit WorkFinished(success, msg, info);
}

void GeometryImportWorker::RequestInterruption()
{
    m_interrupted = true;
}

bool GeometryImportWorker::ImportSTEP(ModelGeometryInfo& info)
{
    emit StatusUpdated("解析STEP文件...");
    emit ProgressUpdated(30);

    if (m_interrupted)
    {
        return false;
    }

    STEPControl_Reader reader;
    QByteArray utf8Bytes = m_filePath.toUtf8();
    const char* cStr = utf8Bytes.constData();
    if (reader.ReadFile(cStr) != IFSelect_RetDone)
    {
        return false;
    }

    if (m_interrupted)
    {
        return false;
    }

    emit ProgressUpdated(50);
    emit StatusUpdated("转换STEP模型...");

    if (m_interrupted)
    {
        return false;
    }

    reader.TransferRoots();

    if (m_interrupted)
    {
        return false;
    }

    auto shape = reader.OneShape();

    if (shape.IsNull())
    {
        return false;
    }

    if (m_interrupted)
    {
        return false;
    }

    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);

    switch (m_partType)
    {
    case PartType::Shell:
        // 壳体 - 灰色
        aisShape->SetColor(Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB));
        info.shellAisShape = aisShape;
        break;
    case PartType::Propellant:
        // 推进剂 - 橙色
        aisShape->SetColor(Quantity_Color(1.0, 0.5, 0.0, Quantity_TOC_RGB));
        info.propellantAisShape = aisShape;
        break;
    case PartType::HeatInsulatingLayer:
        // 绝热层 - 蓝色
        aisShape->SetColor(Quantity_Color(0.0, 0.5, 1.0, Quantity_TOC_RGB));
        info.heatInsulatingLayerAisShape = aisShape;
        break;
    default:
        // 未知类型，不设置颜色
        break;
    }

    info.shape = shape;

    emit ProgressUpdated(70);
    return true;
}

bool GeometryImportWorker::ImportSTL(ModelGeometryInfo& info)
{
    emit StatusUpdated("解析STL文件...");
    emit ProgressUpdated(30);

    StlAPI_Reader reader;
    TopoDS_Shape shape;
    if (!reader.Read(shape, m_filePath.toStdString().c_str()))
        return false;

    if (m_interrupted) return false;
    info.shape = shape;

    emit ProgressUpdated(70);
    return true;
}

bool GeometryImportWorker::ImportIGES(ModelGeometryInfo& info)
{
    emit StatusUpdated("解析IGES文件...");
    emit ProgressUpdated(30);

    IGESControl_Reader reader;
    if (reader.ReadFile(m_filePath.toStdString().c_str()) != IFSelect_RetDone)
        return false;

    if (m_interrupted) return false;
    emit ProgressUpdated(50);
    emit StatusUpdated("转换IGES模型...");

    reader.TransferRoots();
    TopoDS_Shape shape;
    shape = reader.OneShape();

    if (shape.IsNull()) return false;

    // IGES 同样处理部件类型
    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);

    switch (m_partType)
    {
    case PartType::Shell:
        aisShape->SetColor(Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB));
        info.shellAisShape = aisShape;
        break;
    case PartType::Propellant:
        aisShape->SetColor(Quantity_Color(1.0, 0.5, 0.0, Quantity_TOC_RGB));
        info.propellantAisShape = aisShape;
        break;
    case PartType::HeatInsulatingLayer:
        aisShape->SetColor(Quantity_Color(0.0, 0.5, 1.0, Quantity_TOC_RGB));
        info.heatInsulatingLayerAisShape = aisShape;
        break;
    default:
        break;
    }

    info.shape = shape;

    emit ProgressUpdated(70);
    return true;
}

void GeometryImportWorker::CalculateBoundingBox(ModelGeometryInfo& info)
{
    // 先清空原有整体 shape
    info.shape = TopoDS_Shape();

    std::vector<TopoDS_Shape> shapes;

    if (!info.shellAisShape.IsNull())
        shapes.push_back(info.shellAisShape->Shape());

    if (!info.propellantAisShape.IsNull())
        shapes.push_back(info.propellantAisShape->Shape());

    if (!info.heatInsulatingLayerAisShape.IsNull())
        shapes.push_back(info.heatInsulatingLayerAisShape->Shape());

    if (shapes.empty())
        return;

    // 只有一个部件时，直接用该部件的 shape
    if (shapes.size() == 1)
    {
        info.shape = shapes[0];

        Bnd_Box bbox;
        BRepBndLib::Add(info.shape, bbox);
        bbox.SetGap(0.0);

        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
        bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        info.theXmin = xmin;
        info.theYmin = ymin;
        info.theZmin = zmin;
        info.theXmax = xmax;
        info.theYmax = ymax;
        info.theZmax = zmax;

        info.length = xmax - xmin;
        info.width = ymax - ymin;
        info.height = zmax - zmin;
        return;
    }

    // 多个部件时，合并成 Compound
    BRep_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);

    for (const auto& s : shapes)
    {
        builder.Add(compound, s);
    }

    Bnd_Box bbox;
    BRepBndLib::Add(compound, bbox);
    bbox.SetGap(0.0);

    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    info.theXmin = xmin;
    info.theYmin = ymin;
    info.theZmin = zmin;
    info.theXmax = xmax;
    info.theYmax = ymax;
    info.theZmax = zmax;

    info.length = xmax - xmin;
    info.width = ymax - ymin;
    info.height = zmax - zmin;

    info.shape = compound;
}

void GeometryImportWorker::SetPartType(PartType type)
{
    m_partType = type;
}



// ========== 在 GeometryImportWorker.cpp 末尾添加 ==========

#include <BRepAdaptor_Surface.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <GeomAbs_CurveType.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Cone.hxx>
#include <gp_Circ.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <algorithm>
#include <BRep_Tool.hxx>

bool GeometryImportWorker::FindConnectionCircle(const TopoDS_Shape& shape, gp_Pnt& center, double& radius)
{
    struct CylFace {
        TopoDS_Face face;
        double radius;
        gp_Pnt location;
    };
    struct ConeFace {
        TopoDS_Face face;
        gp_Pnt location;
    };

    std::vector<CylFace> cylFaces;
    std::vector<ConeFace> coneFaces;

    TopExp_Explorer faceExp(shape, TopAbs_FACE);
    while (faceExp.More()) {
        TopoDS_Face face = TopoDS::Face(faceExp.Current());
        BRepAdaptor_Surface surf(face);

        if (surf.GetType() == GeomAbs_Cylinder) {
            gp_Cylinder cyl = surf.Cylinder();
            cylFaces.push_back({ face, cyl.Radius(), cyl.Location() });
        }
        else if (surf.GetType() == GeomAbs_Cone) {
            gp_Cone cone = surf.Cone();
            coneFaces.push_back({ face, cone.Location() });
        }
        faceExp.Next();
    }

    if (cylFaces.empty() || coneFaces.empty()) {
        return false;
    }

    bool found = false;
    double bestScore = -1e300;

    for (const auto& cyl : cylFaces) {
        TopTools_IndexedMapOfShape cylEdges;
        TopExp_Explorer cylEdgeExp(cyl.face, TopAbs_EDGE);
        while (cylEdgeExp.More()) {
            cylEdges.Add(cylEdgeExp.Current());
            cylEdgeExp.Next();
        }

        for (const auto& cone : coneFaces) {
            TopExp_Explorer coneEdgeExp(cone.face, TopAbs_EDGE);
            while (coneEdgeExp.More()) {
                TopoDS_Edge edge = TopoDS::Edge(coneEdgeExp.Current());

                if (cylEdges.Contains(edge)) {
                    BRepAdaptor_Curve curve(edge);
                    if (curve.GetType() == GeomAbs_Circle) {
                        gp_Circ circle = curve.Circle();
                        gp_Pnt p = circle.Location();

                        // 评分：X 主导，radius 辅助
                        double score = p.X() + cyl.radius * 0.001;

                        if (score > bestScore) {
                            bestScore = score;
                            center = p;
                            radius = circle.Radius();
                            found = true;
                        }
                    }
                }
                coneEdgeExp.Next();
            }
        }
    }

    return found;
}

void GeometryImportWorker::AnalyzeGeometry(ModelGeometryInfo& info)
{
    // 初始化所有点
    info.ptShellLeftBottom = gp_Pnt(0, 0, 0);
    info.ptShellRightBottom = gp_Pnt(0, 0, 0);
    info.ptNozzleInletBottom = gp_Pnt(0, 0, 0);
    info.ptNozzleOutletBottom = gp_Pnt(0, 0, 0);

    if (info.shape.IsNull()) {
        return;
    }

    gp_Pnt center;
    double radius = 0.0;
    bool hasNozzle = false;

    // 尝试精确找连接圆（圆柱-锥面交界）
    if (FindConnectionCircle(info.shape, center, radius)) {
        // 喷管入口底部：连接圆中心向下偏移半径
        info.ptNozzleInletBottom = gp_Pnt(center.X(), center.Y() + radius, center.Z());
        hasNozzle = true;
    }
    else {
        // fallback：用最大半径圆柱面的位置估算
        std::vector<double> radii;
        std::vector<gp_Pnt> locations;

        TopExp_Explorer faceExp(info.shape, TopAbs_FACE);
        while (faceExp.More()) {
            TopoDS_Face face = TopoDS::Face(faceExp.Current());
            BRepAdaptor_Surface surf(face);
            if (surf.GetType() == GeomAbs_Cylinder) {
                radii.push_back(surf.Cylinder().Radius());
                locations.push_back(surf.Cylinder().Location());
            }
            faceExp.Next();
        }

        double connX = info.theXmin + info.length * 0.72;
        double bestR = 0.0;

        if (!radii.empty()) {
            size_t maxIdx = 0;
            for (size_t i = 1; i < radii.size(); ++i) {
                if (radii[i] > radii[maxIdx]) maxIdx = i;
            }
            connX = locations[maxIdx].X();
            bestR = radii[maxIdx];
            hasNozzle = true;
        }

        info.ptNozzleInletBottom = gp_Pnt(
            connX,
            (info.theYmin + info.theYmax) / 2.0 + bestR,
            (info.theZmin + info.theZmax) / 2.0
        );
    }

    // 计算厚度：最大半径 - 次大半径（外壁 - 内壁）
    std::vector<double> radii;
    TopExp_Explorer faceExp(info.shape, TopAbs_FACE);
    while (faceExp.More()) {
        TopoDS_Face face = TopoDS::Face(faceExp.Current());
        BRepAdaptor_Surface surf(face);
        if (surf.GetType() == GeomAbs_Cylinder) {
            radii.push_back(surf.Cylinder().Radius());
        }
        faceExp.Next();
    }

    if (radii.size() >= 2) {
        std::sort(radii.begin(), radii.end(), std::greater<double>());
        info.thickness = radii[0] - radii[1];
    }

    // 找筒体两端底部点
    FindShellBottomPoints(info);

    // 找喷管出口底部点
    if (hasNozzle) {
        FindNozzleOutletPoint(info);
    }
}

void GeometryImportWorker::FindShellBottomPoints(ModelGeometryInfo& info)
{
    if (info.shape.IsNull()) {
        info.ptShellLeftBottom = gp_Pnt(0, 0, 0);
        info.ptShellRightBottom = gp_Pnt(0, 0, 0);
        return;
    }

    double totalYRange = info.theYmax - info.theYmin;
    double totalZRange = info.theZmax - info.theZmin;

    struct CylData {
        TopoDS_Face face;
        double radius;
        gp_Pnt location;
        gp_Dir axis;
        double ymin, ymax, zmin, zmax;
        bool isOuter;
    };

    std::vector<CylData> cylFaces;

    TopExp_Explorer faceExp(info.shape, TopAbs_FACE);
    while (faceExp.More()) {
        TopoDS_Face face = TopoDS::Face(faceExp.Current());
        BRepAdaptor_Surface surf(face);

        if (surf.GetType() != GeomAbs_Cylinder) {
            faceExp.Next();
            continue;
        }

        Bnd_Box box;
        BRepBndLib::Add(face, box);
        Standard_Real x1, y1, z1, x2, y2, z2;
        box.Get(x1, y1, z1, x2, y2, z2);

        gp_Cylinder cyl = surf.Cylinder();
        cylFaces.push_back({
            face, cyl.Radius(), cyl.Location(), cyl.Axis().Direction(),
            y1, y2, z1, z2, false
            });

        faceExp.Next();
    }

    if (cylFaces.empty()) {
        double r = std::max(info.width, info.height) / 2.0;
        info.ptShellLeftBottom = gp_Pnt(info.theXmin + r * 0.5, info.theYmax, (info.theZmin + info.theZmax) / 2.0);
        info.ptShellRightBottom = gp_Pnt(info.theXmax - r * 0.5, info.theYmax, (info.theZmin + info.theZmax) / 2.0);
        return;
    }

    // 标记外壁：按范围 + 半径辅助判断
    double expectedR = std::max(info.width, info.height) / 2.0;
    for (auto& cyl : cylFaces) {
        double cylYRange = cyl.ymax - cyl.ymin;
        double cylZRange = cyl.zmax - cyl.zmin;
        bool byRange = (cylYRange > totalYRange * 0.85 && cylZRange > totalZRange * 0.85);
        bool byRadius = (cyl.radius > expectedR * 0.7);
        cyl.isOuter = byRange || byRadius;
    }

    struct CircleEdge {
        gp_Pnt center;
        double radius;
        double x;
        bool isLeft;
    };

    std::vector<CircleEdge> connectionCircles;

    for (const auto& cyl : cylFaces) {
        if (!cyl.isOuter) continue;

        TopTools_IndexedMapOfShape cylEdges;
        TopExp_Explorer edgeExp(cyl.face, TopAbs_EDGE);
        while (edgeExp.More()) {
            cylEdges.Add(edgeExp.Current());
            edgeExp.Next();
        }

        TopExp_Explorer faceExp2(info.shape, TopAbs_FACE);
        while (faceExp2.More()) {
            TopoDS_Face face2 = TopoDS::Face(faceExp2.Current());
            if (face2.IsSame(cyl.face)) { faceExp2.Next(); continue; }

            BRepAdaptor_Surface surf2(face2);
            GeomAbs_SurfaceType type2 = surf2.GetType();

            bool isEndCap = (type2 == GeomAbs_Sphere || type2 == GeomAbs_Torus ||
                type2 == GeomAbs_SurfaceOfRevolution || type2 == GeomAbs_Plane);

            if (!isEndCap) { faceExp2.Next(); continue; }

            TopExp_Explorer edgeExp2(face2, TopAbs_EDGE);
            while (edgeExp2.More()) {
                TopoDS_Edge edge = TopoDS::Edge(edgeExp2.Current());

                if (!cylEdges.Contains(edge)) { edgeExp2.Next(); continue; }

                BRepAdaptor_Curve curve(edge);
                if (curve.GetType() != GeomAbs_Circle) { edgeExp2.Next(); continue; }

                gp_Circ circle = curve.Circle();
                gp_Pnt p = circle.Location();

                if (circle.Radius() < expectedR * 0.7) { edgeExp2.Next(); continue; }

                double xCenter = (info.theXmin + info.theXmax) / 2.0;
                bool isLeft = (p.X() < xCenter);

                connectionCircles.push_back({ p, circle.Radius(), p.X(), isLeft });

                edgeExp2.Next();
            }
            faceExp2.Next();
        }
    }

    if (!connectionCircles.empty()) {
        std::vector<CircleEdge> leftCircles;
        std::vector<CircleEdge> rightCircles;

        for (const auto& c : connectionCircles) {
            if (c.isLeft) leftCircles.push_back(c);
            else rightCircles.push_back(c);
        }

        // 左侧：找X最大的（最靠近中心，即封头与圆柱交界）
        if (!leftCircles.empty()) {
            auto bestLeft = std::max_element(leftCircles.begin(), leftCircles.end(),
                [](const CircleEdge& a, const CircleEdge& b) { return a.x < b.x; });

            if (bestLeft->x < (info.theXmin + info.theXmax) / 2.0 - info.length * 0.1) {
                info.ptShellLeftBottom = gp_Pnt(
                    bestLeft->center.X(),
                    bestLeft->center.Y() + bestLeft->radius,  // 上边缘作为底部？根据坐标系确认
                    bestLeft->center.Z()
                );
            }
            else {
                auto fallbackLeft = std::min_element(leftCircles.begin(), leftCircles.end(),
                    [](const CircleEdge& a, const CircleEdge& b) { return a.x < b.x; });
                info.ptShellLeftBottom = gp_Pnt(
                    fallbackLeft->center.X(),
                    fallbackLeft->center.Y() + fallbackLeft->radius,
                    fallbackLeft->center.Z()
                );
            }
        }
        else {
            double r = std::max(info.width, info.height) / 2.0;
            info.ptShellLeftBottom = gp_Pnt(info.theXmin + r * 0.5, info.theYmax, (info.theZmin + info.theZmax) / 2.0);
        }

        // 右侧：找X最大但排除喷管处（必须在喷管入口左侧）
        if (!rightCircles.empty()) {
            double connX = info.ptNozzleInletBottom.X();
            std::vector<CircleEdge> validRight;
            for (const auto& c : rightCircles) {
                if (c.x < connX - info.length * 0.05) {
                    validRight.push_back(c);
                }
            }

            if (!validRight.empty()) {
                auto bestRight = std::max_element(validRight.begin(), validRight.end(),
                    [](const CircleEdge& a, const CircleEdge& b) { return a.x < b.x; });
                info.ptShellRightBottom = gp_Pnt(
                    bestRight->center.X(),
                    bestRight->center.Y() + bestRight->radius,
                    bestRight->center.Z()
                );
            }
            else {
                // fallback：用喷管入口点
                double cylR = std::max(info.width, info.height) / 2.0;
                info.ptShellRightBottom = gp_Pnt(
                    info.ptNozzleInletBottom.X(),
                    info.ptNozzleInletBottom.Y() + cylR,
                    info.ptNozzleInletBottom.Z()
                );
            }
        }
        else {
            double r = std::max(info.width, info.height) / 2.0;
            info.ptShellRightBottom = gp_Pnt(info.theXmax - r * 0.5, info.theYmax, (info.theZmin + info.theZmax) / 2.0);
        }
    }
    else {
        double r = std::max(info.width, info.height) / 2.0;
        info.ptShellLeftBottom = gp_Pnt(info.theXmin + r * 0.5, info.theYmax, (info.theZmin + info.theZmax) / 2.0);
        info.ptShellRightBottom = gp_Pnt(info.theXmax - r * 0.5, info.theYmax, (info.theZmin + info.theZmax) / 2.0);
    }
}

void GeometryImportWorker::FindNozzleOutletPoint(ModelGeometryInfo& info)
{
    if (info.shape.IsNull()) {
        info.ptNozzleOutletBottom = gp_Pnt(0, 0, 0);
        return;
    }

    struct CircleCandidate {
        gp_Pnt center;
        double radius;
        gp_Pnt bottom;
    };

    std::vector<CircleCandidate> candidates;

    // 遍历所有面，收集所有圆边
    TopExp_Explorer faceExp(info.shape, TopAbs_FACE);
    while (faceExp.More()) {
        TopoDS_Face face = TopoDS::Face(faceExp.Current());

        TopExp_Explorer edgeExp(face, TopAbs_EDGE);
        while (edgeExp.More()) {
            TopoDS_Edge edge = TopoDS::Edge(edgeExp.Current());
            BRepAdaptor_Curve curve(edge);

            if (curve.GetType() != GeomAbs_Circle) {
                edgeExp.Next();
                continue;
            }

            gp_Circ circle = curve.Circle();
            gp_Pnt c = circle.Location();
            double r = circle.Radius();

            // 底部点
            gp_Pnt bottom(c.X(), c.Y() + r, c.Z());

            candidates.push_back({ c, r, bottom });

            edgeExp.Next();
        }
        faceExp.Next();
    }

    if (candidates.empty()) {
        info.ptNozzleOutletBottom = gp_Pnt(0, 0, 0);
        return;
    }

    // 严格按你的规则排序：X 最大 → Y 最大 → Z 最小
    std::sort(candidates.begin(), candidates.end(),
        [](const CircleCandidate& a, const CircleCandidate& b) {
            // X 降序
            if (std::abs(a.bottom.X() - b.bottom.X()) > 1.0e-6)
                return a.bottom.X() > b.bottom.X();
            // X 相同，Y 降序
            if (std::abs(a.bottom.Y() - b.bottom.Y()) > 1.0e-6)
                return a.bottom.Y() > b.bottom.Y();
            // Y 相同，Z 升序（最小）
            return a.bottom.Z() < b.bottom.Z();
        });

    // 取第一个（最优）
    info.ptNozzleOutletBottom = candidates.front().bottom;
}