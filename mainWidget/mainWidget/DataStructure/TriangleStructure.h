#pragma once

#include <Standard.hxx>
#include <TColStd_HArray2OfInteger.hxx>
#include <TColStd_HArray2OfReal.hxx>
#include <MeshVS_DataSource.hxx>
#include <Standard_Integer.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <MeshVS_EntityType.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TopoDS_Shape.hxx>
#include <unordered_map>
#include <vector>
#include <utility>

#define MY_PRECISION 1.0e-6

struct GpPntHasher {
    std::size_t operator()(const gp_Pnt& p) const {
        long long x = static_cast<long long>(p.X() * 1000000.0);
        long long y = static_cast<long long>(p.Y() * 1000000.0);
        long long z = static_cast<long long>(p.Z() * 1000000.0);
        return (x * 73856093) ^ (y * 19349663) ^ (z * 83492791);
    }
};

struct GpPntEqual {
    bool operator()(const gp_Pnt& p1, const gp_Pnt& p2) const {
        return (Abs(p1.X() - p2.X()) < MY_PRECISION &&
            Abs(p1.Y() - p2.Y()) < MY_PRECISION &&
            Abs(p1.Z() - p2.Z()) < MY_PRECISION);
    }
};

class TriangleStructure : public MeshVS_DataSource {
public:
    Standard_EXPORT TriangleStructure();

    // 注意：传入非 const 引用以匹配 Ng_OCC_Load_Shape
    Standard_EXPORT TriangleStructure(TopoDS_Shape& shape,
        const Standard_Real linearDeflection = 0.5,
        volatile bool* interrupted = nullptr);

    Standard_EXPORT bool CheckInterruption(volatile bool* interrupted) const;
    Standard_EXPORT void ExtractEdges();
    Standard_EXPORT Handle(TriangleStructure) RotateXZ(const Standard_Real angleDeg,
        const Standard_Real x0,
        const Standard_Real z0) const;

    std::vector<std::pair<Standard_Integer, Standard_Integer>> GetMyEdge();
    Handle(TColStd_HArray2OfReal) GetmyNodeCoords();

    // --- MeshVS_DataSource overrides ---
    Standard_EXPORT Standard_Boolean GetGeom(const Standard_Integer ID,
        const Standard_Boolean IsElement,
        TColStd_Array1OfReal& Coords,
        Standard_Integer& NbNodes,
        MeshVS_EntityType& Type) const Standard_OVERRIDE;

    Standard_EXPORT Standard_Boolean GetGeomType(const Standard_Integer ID,
        const Standard_Boolean IsElement,
        MeshVS_EntityType& Type) const Standard_OVERRIDE;

    Standard_EXPORT Standard_Address GetAddr(const Standard_Integer ID,
        const Standard_Boolean IsElement) const Standard_OVERRIDE;

    Standard_EXPORT virtual Standard_Boolean GetNodesByElement(const Standard_Integer ID,
        TColStd_Array1OfInteger& NodeIDs,
        Standard_Integer& NbNodes) const Standard_OVERRIDE;

    Standard_EXPORT const TColStd_PackedMapOfInteger& GetAllNodes() const Standard_OVERRIDE;
    Standard_EXPORT const TColStd_PackedMapOfInteger& GetAllElements() const Standard_OVERRIDE;

    Standard_EXPORT virtual Standard_Boolean GetNormal(const Standard_Integer Id,
        const Standard_Integer Max,
        Standard_Real& nx,
        Standard_Real& ny,
        Standard_Real& nz) const Standard_OVERRIDE;

private:
    TopoDS_Shape m_shape; // 新增：持有深拷贝后的几何

    Handle(TColStd_HArray2OfReal) myNodeCoords;
    Handle(TColStd_HArray2OfInteger) myElemNodes;
    Handle(TColStd_HArray2OfReal) myElemNormals;
    TColStd_PackedMapOfInteger myNodes;
    TColStd_PackedMapOfInteger myElements;
    std::unordered_map<gp_Pnt, Standard_Integer, GpPntHasher, GpPntEqual> myCoordToNodeMap;
    std::vector<std::pair<Standard_Integer, Standard_Integer>> myEdges;
};