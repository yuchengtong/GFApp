#include "TriangleStructure.h"
#include <Precision.hxx>
#include <Standard_Type.hxx>
#include <BRep_Tool.hxx>
#include <gp_Trsf.hxx>
#include <TopoDS.hxx>
#include <gp_Vec.hxx>
#include <cmath>
#include <set>
#include <cassert>
#include <iostream>
#include <mutex>
#include <BRepBuilderAPI_Copy.hxx>

namespace nglib {
#include <nglib.h>
}
using namespace nglib;

// 线程安全的 Netgen 初始化
void EnsureNgInit() {
    static std::once_flag flag;
    std::call_once(flag, []() {
        Ng_Init();
        });
}

// 中断检查辅助函数
bool TriangleStructure::CheckInterruption(volatile bool* interrupted) const {
    return interrupted && *interrupted;
}

// 默认构造函数
TriangleStructure::TriangleStructure() {
    myNodeCoords = new TColStd_HArray2OfReal(0, 0, 1, 3);
    myElemNodes = new TColStd_HArray2OfInteger(0, 0, 1, 3);
    myElemNormals = new TColStd_HArray2OfReal(0, 0, 1, 3);
}

// 主构造函数（修复版）
TriangleStructure::TriangleStructure(TopoDS_Shape& shape,
    const Standard_Real linearDeflection,
    volatile bool* interrupted)
{
    // === Step 1: 深拷贝输入几何（关键！）===
    BRepBuilderAPI_Copy copier(shape);
    if (!copier.IsDone() || copier.Shape().IsNull()) {
        std::cerr << "TriangleStructure: Failed to deep-copy input shape." << std::endl;
        return;
    }
    m_shape = copier.Shape();

    if (CheckInterruption(interrupted)) return;

    // === Step 2: 初始化 Netgen ===
    EnsureNgInit();

    // === Step 3: 加载几何 ===
    assert(!m_shape.IsNull());
    Ng_OCC_Geometry* occ_geom = Ng_OCC_Load_Shape(m_shape);
    if (!occ_geom) {
        std::cerr << "TriangleStructure: Failed to load shape into Netgen." << std::endl;
        return;
    }

    // === Step 4: 创建网格 ===
    Ng_Mesh* mesh = Ng_NewMesh();
    if (!mesh) {
        std::cerr << "TriangleStructure: Ng_NewMesh failed." << std::endl;
        return;
    }

    // === Step 5: 设置参数（零初始化！）===
    Ng_Meshing_Parameters mp = {};
    mp.uselocalh = 1;
    mp.maxh = 5.0;
    mp.minh = 0.1;
    mp.elementsperedge = 3.0;
    mp.elementspercurve = 4.0;
    mp.grading = 0.25;
    mp.closeedgeenable = 0;
    mp.optsurfmeshenable = 1;

    Ng_OCC_SetLocalMeshSize(occ_geom, mesh, &mp);

    // === Step 6: 生成边网格 ===
    Ng_Result res = Ng_OCC_GenerateEdgeMesh(occ_geom, mesh, &mp);
    if (res != NG_OK || CheckInterruption(interrupted)) {
        Ng_DeleteMesh(mesh);
        return;
    }

    // === Step 7: 生成面网格 ===
    res = Ng_OCC_GenerateSurfaceMesh(occ_geom, mesh, &mp);
    if (res != NG_OK || CheckInterruption(interrupted)) {
        Ng_DeleteMesh(mesh);
        return;
    }

    int np = Ng_GetNP(mesh);
    int ne = Ng_GetNSE(mesh);
    if (np <= 0 || ne <= 0) {
        Ng_DeleteMesh(mesh);
        return;
    }

    // === Step 8: 分配数组 ===
    myNodeCoords = new TColStd_HArray2OfReal(1, np, 1, 3);
    myElemNodes = new TColStd_HArray2OfInteger(1, ne, 1, 3);
    myElemNormals = new TColStd_HArray2OfReal(1, ne, 1, 3);

    // === Step 9: 加载节点 ===
    double pt[3];
    for (int i = 1; i <= np; ++i) {
        Ng_GetPoint(mesh, i, pt);
        myNodeCoords->SetValue(i, 1, pt[0]);
        myNodeCoords->SetValue(i, 2, pt[1]);
        myNodeCoords->SetValue(i, 3, pt[2]);
        myNodes.Add(i);
        myCoordToNodeMap[gp_Pnt(pt[0], pt[1], pt[2])] = i;
    }

    // === Step 10: 加载单元并计算法向 ===
    int tri[3];
    for (int i = 1; i <= ne; ++i) {
        Ng_GetSurfaceElement(mesh, i, tri);
        Standard_Integer n1 = tri[0], n2 = tri[1], n3 = tri[2];
        myElemNodes->SetValue(i, 1, n1);
        myElemNodes->SetValue(i, 2, n2);
        myElemNodes->SetValue(i, 3, n3);
        myElements.Add(i);

        gp_Pnt p1(myNodeCoords->Value(n1, 1), myNodeCoords->Value(n1, 2), myNodeCoords->Value(n1, 3));
        gp_Pnt p2(myNodeCoords->Value(n2, 1), myNodeCoords->Value(n2, 2), myNodeCoords->Value(n2, 3));
        gp_Pnt p3(myNodeCoords->Value(n3, 1), myNodeCoords->Value(n3, 2), myNodeCoords->Value(n3, 3));

        gp_Vec v1(p1, p2);
        gp_Vec v2(p1, p3);
        gp_Vec normal = v1.Crossed(v2);
        if (normal.SquareMagnitude() > MY_PRECISION * MY_PRECISION) {
            normal.Normalize();
        }
        myElemNormals->SetValue(i, 1, normal.X());
        myElemNormals->SetValue(i, 2, normal.Y());
        myElemNormals->SetValue(i, 3, normal.Z());
    }

    Ng_DeleteMesh(mesh); // 只需删除 mesh

    if (CheckInterruption(interrupted)) return;

    ExtractEdges(); // 从三角形重建边
}

// ========== 以下为原有功能（保持不变）==========

void TriangleStructure::ExtractEdges() {
    std::set<std::pair<Standard_Integer, Standard_Integer>> edgeSet;
    Standard_Integer elemMin = myElemNodes->LowerRow();
    Standard_Integer elemMax = myElemNodes->UpperRow();
    for (Standard_Integer elemID = elemMin; elemID <= elemMax; ++elemID) {
        Standard_Integer n1 = myElemNodes->Value(elemID, 1);
        Standard_Integer n2 = myElemNodes->Value(elemID, 2);
        Standard_Integer n3 = myElemNodes->Value(elemID, 3);
        if (n1 == n2 || n1 == n3 || n2 == n3) continue;
        auto edge1 = std::make_pair(std::min(n1, n2), std::max(n1, n2));
        auto edge2 = std::make_pair(std::min(n2, n3), std::max(n2, n3));
        auto edge3 = std::make_pair(std::min(n3, n1), std::max(n3, n1));
        edgeSet.insert(edge1);
        edgeSet.insert(edge2);
        edgeSet.insert(edge3);
    }
    myEdges.assign(edgeSet.begin(), edgeSet.end());
}

Handle(TriangleStructure) TriangleStructure::RotateXZ(const Standard_Real angleDeg,
    const Standard_Real x0,
    const Standard_Real z0) const {
    Handle(TriangleStructure) rotatedStructure = new TriangleStructure();
    Standard_Real angleRad = angleDeg * M_PI / 180.0;
    Standard_Real cosAngle = cos(angleRad);
    Standard_Real sinAngle = sin(angleRad);
    Standard_Integer nodeCount = myNodeCoords->UpperRow();
    rotatedStructure->myNodeCoords = new TColStd_HArray2OfReal(1, nodeCount, 1, 3);
    for (Standard_Integer i = 1; i <= nodeCount; ++i) {
        Standard_Real x = myNodeCoords->Value(i, 1);
        Standard_Real y = myNodeCoords->Value(i, 2);
        Standard_Real z = myNodeCoords->Value(i, 3);
        Standard_Real tx = x - x0;
        Standard_Real tz = z - z0;
        Standard_Real rx = tx * cosAngle + tz * sinAngle + x0;
        Standard_Real rz = -tx * sinAngle + tz * cosAngle + z0;
        rotatedStructure->myNodeCoords->SetValue(i, 1, rx);
        rotatedStructure->myNodeCoords->SetValue(i, 2, y);
        rotatedStructure->myNodeCoords->SetValue(i, 3, rz);
        rotatedStructure->myNodes.Add(i);
    }
    Standard_Integer elemCount = myElemNodes->UpperRow();
    rotatedStructure->myElemNodes = new TColStd_HArray2OfInteger(1, elemCount, 1, 3);
    for (Standard_Integer i = 1; i <= elemCount; ++i) {
        rotatedStructure->myElemNodes->SetValue(i, 1, myElemNodes->Value(i, 1));
        rotatedStructure->myElemNodes->SetValue(i, 2, myElemNodes->Value(i, 2));
        rotatedStructure->myElemNodes->SetValue(i, 3, myElemNodes->Value(i, 3));
        rotatedStructure->myElements.Add(i);
    }
    rotatedStructure->myElemNormals = new TColStd_HArray2OfReal(1, elemCount, 1, 3);
    for (Standard_Integer i = 1; i <= elemCount; ++i) {
        Standard_Integer n1 = rotatedStructure->myElemNodes->Value(i, 1);
        Standard_Integer n2 = rotatedStructure->myElemNodes->Value(i, 2);
        Standard_Integer n3 = rotatedStructure->myElemNodes->Value(i, 3);
        gp_Pnt p1(rotatedStructure->myNodeCoords->Value(n1, 1),
            rotatedStructure->myNodeCoords->Value(n1, 2),
            rotatedStructure->myNodeCoords->Value(n1, 3));
        gp_Pnt p2(rotatedStructure->myNodeCoords->Value(n2, 1),
            rotatedStructure->myNodeCoords->Value(n2, 2),
            rotatedStructure->myNodeCoords->Value(n2, 3));
        gp_Pnt p3(rotatedStructure->myNodeCoords->Value(n3, 1),
            rotatedStructure->myNodeCoords->Value(n3, 2),
            rotatedStructure->myNodeCoords->Value(n3, 3));
        gp_Vec v1(p1, p2);
        gp_Vec v2(p1, p3);
        gp_Vec normal = v1.Crossed(v2);
        if (normal.SquareMagnitude() > MY_PRECISION * MY_PRECISION) {
            normal.Normalize();
        }
        rotatedStructure->myElemNormals->SetValue(i, 1, normal.X());
        rotatedStructure->myElemNormals->SetValue(i, 2, normal.Y());
        rotatedStructure->myElemNormals->SetValue(i, 3, normal.Z());
    }
    for (Standard_Integer i = 1; i <= nodeCount; ++i) {
        gp_Pnt p(rotatedStructure->myNodeCoords->Value(i, 1),
            rotatedStructure->myNodeCoords->Value(i, 2),
            rotatedStructure->myNodeCoords->Value(i, 3));
        rotatedStructure->myCoordToNodeMap[p] = i;
    }
    rotatedStructure->ExtractEdges();
    return rotatedStructure;
}

std::vector<std::pair<Standard_Integer, Standard_Integer>> TriangleStructure::GetMyEdge() {
    return myEdges;
}

Handle(TColStd_HArray2OfReal) TriangleStructure::GetmyNodeCoords() {
    return myNodeCoords;
}

// --- MeshVS_DataSource implementations ---
Standard_Boolean TriangleStructure::GetGeom(const Standard_Integer ID,
    const Standard_Boolean IsElement,
    TColStd_Array1OfReal& Coords,
    Standard_Integer& NbNodes,
    MeshVS_EntityType& Type) const {
    if (IsElement) {
        if (ID >= 1 && ID <= myElements.Extent()) {
            Type = MeshVS_ET_Face;
            NbNodes = 3;
            for (Standard_Integer i = 1, k = 1; i <= 3; ++i) {
                Standard_Integer nodeID = myElemNodes->Value(ID, i);
                for (Standard_Integer j = 1; j <= 3; ++j, ++k) {
                    Coords(k) = myNodeCoords->Value(nodeID, j);
                }
            }
            return Standard_True;
        }
    }
    else {
        if (ID >= 1 && ID <= myNodes.Extent()) {
            Type = MeshVS_ET_Node;
            NbNodes = 1;
            Coords(1) = myNodeCoords->Value(ID, 1);
            Coords(2) = myNodeCoords->Value(ID, 2);
            Coords(3) = myNodeCoords->Value(ID, 3);
            return Standard_True;
        }
    }
    return Standard_False;
}

Standard_Boolean TriangleStructure::GetGeomType(const Standard_Integer,
    const Standard_Boolean IsElement,
    MeshVS_EntityType& Type) const {
    Type = IsElement ? MeshVS_ET_Face : MeshVS_ET_Node;
    return Standard_True;
}

Standard_Address TriangleStructure::GetAddr(const Standard_Integer,
    const Standard_Boolean) const {
    return nullptr;
}

Standard_Boolean TriangleStructure::GetNodesByElement(const Standard_Integer ID,
    TColStd_Array1OfInteger& theNodeIDs,
    Standard_Integer& /*theNbNodes*/) const {
    if (ID >= 1 && ID <= myElements.Extent() && theNodeIDs.Length() >= 3) {
        Standard_Integer aLow = theNodeIDs.Lower();
        theNodeIDs(aLow) = myElemNodes->Value(ID, 1);
        theNodeIDs(aLow + 1) = myElemNodes->Value(ID, 2);
        theNodeIDs(aLow + 2) = myElemNodes->Value(ID, 3);
        return Standard_True;
    }
    return Standard_False;
}

const TColStd_PackedMapOfInteger& TriangleStructure::GetAllNodes() const {
    return myNodes;
}

const TColStd_PackedMapOfInteger& TriangleStructure::GetAllElements() const {
    return myElements;
}

Standard_Boolean TriangleStructure::GetNormal(const Standard_Integer Id,
    const Standard_Integer Max,
    Standard_Real& nx,
    Standard_Real& ny,
    Standard_Real& nz) const {
    if (Id >= 1 && Id <= myElements.Extent() && Max >= 3) {
        nx = myElemNormals->Value(Id, 1);
        ny = myElemNormals->Value(Id, 2);
        nz = myElemNormals->Value(Id, 3);
        return Standard_True;
    }
    return Standard_False;
}