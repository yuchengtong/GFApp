#pragma once

#include <Standard.hxx>

#include <TColStd_HArray2OfInteger.hxx>
#include <TColStd_HArray2OfReal.hxx>
#include <MeshVS_DataSource.hxx>
#include <Standard_Integer.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <MeshVS_EntityType.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <Poly_Triangulation.hxx>
#include <TopoDS_Shape.hxx>
#include <TopLoc_Location.hxx>
#include <unordered_map>
// 自定义容差值 (1e-6)
#define MY_PRECISION 1.0e-6


struct GpPntHasher {
    std::size_t operator()(const gp_Pnt& p) const {
        // 将坐标乘以1e6并转换为整数，以处理1e-6的容差
        long long x = static_cast<long long>(p.X() * 1000000.0);
        long long y = static_cast<long long>(p.Y() * 1000000.0);
        long long z = static_cast<long long>(p.Z() * 1000000.0);

        // 使用组合哈希值
        return (x * 73856093) ^ (y * 19349663) ^ (z * 83492791);
    }
};

// 比较函数：使用容差值比较两个点
struct GpPntEqual {
    bool operator()(const gp_Pnt& p1, const gp_Pnt& p2) const {
        return (Abs(p1.X() - p2.X()) < MY_PRECISION &&
            Abs(p1.Y() - p2.Y()) < MY_PRECISION &&
            Abs(p1.Z() - p2.Z()) < MY_PRECISION);
    }
};

//! The sample DataSource for working with STLMesh_Mesh
class TriangleStructure : public MeshVS_DataSource
{
public:
	Standard_EXPORT TriangleStructure();
    //! Constructor
    Standard_EXPORT TriangleStructure(const TopoDS_Shape& shape, const Standard_Real linearDeflection = 0.5);

    Standard_EXPORT void AddTriangulation(const Handle(Poly_Triangulation)& triangulation, const TopLoc_Location& loc);

	void ExtractEdges();

	std::vector<std::pair<Standard_Integer, Standard_Integer>> GetMyEdge();
	Handle(TColStd_HArray2OfReal) GetmyNodeCoords();


    //! Returns geometry information about node (if IsElement is False) or element (IsElement is True) by coordinates.
    //! For element this method must return all its nodes coordinates in the strict order: X, Y, Z and
    //! with nodes order is the same as in wire bounding the face or link. NbNodes is number of nodes of element.
    //! It is recommended to return 1 for node. Type is an element type.
    Standard_EXPORT Standard_Boolean GetGeom (const Standard_Integer ID, const Standard_Boolean IsElement, TColStd_Array1OfReal& Coords, Standard_Integer& NbNodes, MeshVS_EntityType& Type) const Standard_OVERRIDE;

    //! This method is similar to GetGeom, but returns only element or node type. This method is provided for
    //! a fine performance.
    Standard_EXPORT Standard_Boolean GetGeomType (const Standard_Integer ID, const Standard_Boolean IsElement, MeshVS_EntityType& Type) const Standard_OVERRIDE;

    //! This method returns by number an address of any entity which represents element or node data structure.
    Standard_EXPORT Standard_Address GetAddr (const Standard_Integer ID, const Standard_Boolean IsElement) const Standard_OVERRIDE;

    //! This method returns information about what node this element consist of.
    Standard_EXPORT virtual Standard_Boolean GetNodesByElement (const Standard_Integer ID, TColStd_Array1OfInteger& NodeIDs, Standard_Integer& NbNodes) const Standard_OVERRIDE;

    //! This method returns map of all nodes the object consist of.
    Standard_EXPORT const TColStd_PackedMapOfInteger& GetAllNodes() const Standard_OVERRIDE;

    //! This method returns map of all elements the object consist of.
    Standard_EXPORT const TColStd_PackedMapOfInteger& GetAllElements() const Standard_OVERRIDE;

    //! This method calculates normal of face, which is using for correct reflection presentation.
    //! There is default method, for advance reflection this method can be redefined.
    Standard_EXPORT virtual Standard_Boolean GetNormal (const Standard_Integer Id, const Standard_Integer Max, Standard_Real& nx, Standard_Real& ny, Standard_Real& nz) const Standard_OVERRIDE;


private:
    Handle(Poly_Triangulation) myMesh;         

    Handle(TColStd_HArray2OfReal) myNodeCoords;    // 节点坐标 [节点ID][X,Y,Z]
    Handle(TColStd_HArray2OfInteger) myElemNodes;  // 单元-节点连接关系 [单元ID][节点1,节点2,节点3]
    Handle(TColStd_HArray2OfReal) myElemNormals;   // 单元法向量 [单元ID][nx,ny,nz]
    TColStd_PackedMapOfInteger myNodes;             // 节点ID集合
    TColStd_PackedMapOfInteger myElements;          // 单元ID集合

    // 使用标准库的unordered_map替代NCollection_DataMap
    std::unordered_map<gp_Pnt, Standard_Integer, GpPntHasher, GpPntEqual> myCoordToNodeMap;

	// 存储边线信息
	std::vector<std::pair<Standard_Integer, Standard_Integer>> myEdges;

};



