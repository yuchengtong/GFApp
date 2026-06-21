#pragma execution_character_set("utf-8")
#include "GeometryImportWorker.h"
#include <STEPControl_Reader.hxx>
#include <StlAPI_Reader.hxx>
#include <IGESControl_Reader.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <QThread>

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
            info.path = m_filePath;
            msg = "几何模型导入成功，文件路径：" + m_filePath;
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

    m_shape = reader.OneShape();

    if (m_shape.IsNull())
    {
        return false;
    }

    if (m_interrupted)
    {
        return false;
    }

    info.shape = m_shape;

    emit ProgressUpdated(70);
    return true;
}

bool GeometryImportWorker::ImportSTL(ModelGeometryInfo& info)
{
    emit StatusUpdated("解析STL文件...");
    emit ProgressUpdated(30);

    StlAPI_Reader reader;
    if (!reader.Read(m_shape, m_filePath.toStdString().c_str()))
        return false;

    if (m_interrupted) return false;
    info.shape = m_shape;

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
    m_shape = reader.OneShape();

    if (m_shape.IsNull()) return false;
    info.shape = m_shape;

    emit ProgressUpdated(70);
    return true;
}

void GeometryImportWorker::CalculateBoundingBox(ModelGeometryInfo& info)
{
    if (info.shape.IsNull()) return;

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

    // ===== 新增：分析喷管连接点 =====
    AnalyzeNozzleConnection(info);
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
#include <3rdParty/OpenCASCADE/opencascade-7.4.0/inc/BRep_Tool.hxx>

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

    // 找所有圆柱面-锥面的公共圆边
    // 筛选条件：取 X 最大（最靠右）且半径最大（外壁）的
    bool found = false;
    double bestScore = -1e300;  // 评分：X坐标 * 半径，越大越靠外、越靠右

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

                        // 评分：靠右(X大) + 半径大(外壁)
                        double score = p.X() * 1000.0 + cyl.radius;

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

void GeometryImportWorker::AnalyzeNozzleConnection(ModelGeometryInfo& info)
{
    info.hasNozzle = false;
    info.cylinderRadius = 0.0;
    info.engineLength = 0.0;
    info.nozzleLength = 0.0;

    if (info.shape.IsNull()) {
        return;
    }

    gp_Pnt center;
    double radius = 0.0;

    // 尝试精确找连接圆
    if (FindConnectionCircle(info.shape, center, radius)) {
        info.connectionPoint = center;
        info.cylinderRadius = radius;
        info.hasNozzle = true;
    }
    else {
        // fallback：边界框估算，但优先用最大半径圆柱面
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
        double bestR = info.cylinderRadius;

        if (!radii.empty()) {
            // 找最大半径对应的X位置（外壁）
            size_t maxIdx = 0;
            for (size_t i = 1; i < radii.size(); ++i) {
                if (radii[i] > radii[maxIdx]) maxIdx = i;
            }
            connX = locations[maxIdx].X();
            bestR = radii[maxIdx];
        }

        info.connectionPoint = gp_Pnt(connX, (info.theYmin + info.theYmax) / 2.0, (info.theZmin + info.theZmax) / 2.0);
        info.cylinderRadius = bestR;
        info.hasNozzle = true;
    }

    // 计算长度
    info.engineLength = info.connectionPoint.X() - info.theXmin;
    info.nozzleLength = info.theXmax - info.connectionPoint.X();

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
        info.thickness = radii[0] - radii[1];  // 最大 - 次大
    }

    FindBottomEndPoint(info);
}

void GeometryImportWorker::FindBottomEndPoint(ModelGeometryInfo& info)
{
    if (info.shape.IsNull()) {
        info.bottomEndPoint = gp_Pnt(0, 0, 0);
        info.bottomEndPoint2 = gp_Pnt(0, 0, 0);
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

    // 收集所有圆柱面
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
        info.bottomEndPoint = gp_Pnt(info.theXmin + r * 0.5, info.theYmax, (info.theZmin + info.theZmax) / 2.0);
        info.bottomEndPoint2 = gp_Pnt(info.theXmax - r * 0.5, info.theYmax, (info.theZmin + info.theZmax) / 2.0);
        return;
    }

    // 标记外壁
    for (auto& cyl : cylFaces) {
        double cylYRange = cyl.ymax - cyl.ymin;
        double cylZRange = cyl.zmax - cyl.zmin;
        cyl.isOuter = (cylYRange > totalYRange * 0.85 && cylZRange > totalZRange * 0.85);
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

                // 外壁判断：半径接近整体宽度/2
                double expectedR = std::max(info.width, info.height) / 2.0;
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

        // 左侧：找X最大的（最靠近中心），避免封头边缘
        if (!leftCircles.empty()) {
            // 修正：左侧圆中，X最大的才是最靠近圆柱段的（封头与圆柱交界）
            // X最小的可能是封头最边缘
            auto bestLeft = std::max_element(leftCircles.begin(), leftCircles.end(),
                [](const CircleEdge& a, const CircleEdge& b) { return a.x < b.x; });

            // 但还要排除太靠右的（避免找到右侧的圆）
            // 确保 X < 中心 - 一些余量
            if (bestLeft->x < (info.theXmin + info.theXmax) / 2.0 - info.length * 0.1) {
                info.bottomEndPoint = gp_Pnt(bestLeft->center.X(), bestLeft->center.Y() + bestLeft->radius, bestLeft->center.Z());
            }
            else {
                //  fallback：找X最小的
                auto fallbackLeft = std::min_element(leftCircles.begin(), leftCircles.end(),
                    [](const CircleEdge& a, const CircleEdge& b) { return a.x < b.x; });
                info.bottomEndPoint = gp_Pnt(fallbackLeft->center.X(), fallbackLeft->center.Y() + fallbackLeft->radius, fallbackLeft->center.Z());
            }
        }
        else {
            double r = std::max(info.width, info.height) / 2.0;
            info.bottomEndPoint = gp_Pnt(info.theXmin + r * 0.5, info.theYmax, (info.theZmin + info.theZmax) / 2.0);
        }

        // 右侧：找X最大且不是喷管处的
        if (!rightCircles.empty()) {
            double connX = info.connectionPoint.X();
            std::vector<CircleEdge> validRight;
            for (const auto& c : rightCircles) {
                if (c.x < connX - info.length * 0.05) {
                    validRight.push_back(c);
                }
            }

            if (!validRight.empty()) {
                auto bestRight = std::max_element(validRight.begin(), validRight.end(),
                    [](const CircleEdge& a, const CircleEdge& b) { return a.x < b.x; });
                info.bottomEndPoint2 = gp_Pnt(bestRight->center.X(), bestRight->center.Y() + bestRight->radius, bestRight->center.Z());
            }
            else {
                info.bottomEndPoint2 = gp_Pnt(info.connectionPoint.X(), info.connectionPoint.Y() + info.cylinderRadius, info.connectionPoint.Z());
            }
        }
        else {
            double r = std::max(info.width, info.height) / 2.0;
            info.bottomEndPoint2 = gp_Pnt(info.theXmax - r * 0.5, info.theYmax, (info.theZmin + info.theZmax) / 2.0);
        }
    }
    else {
        double r = std::max(info.width, info.height) / 2.0;
        info.bottomEndPoint = gp_Pnt(info.theXmin + r * 0.5, info.theYmax, (info.theZmin + info.theZmax) / 2.0);
        info.bottomEndPoint2 = gp_Pnt(info.theXmax - r * 0.5, info.theYmax, (info.theZmin + info.theZmax) / 2.0);
    }
}