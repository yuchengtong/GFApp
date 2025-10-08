#include "TriangleStructure.h"

#include <Precision.hxx>
#include <Standard_Type.hxx>
#include <TColStd_DataMapOfIntegerReal.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Tool.hxx>
#include <gp_Trsf.hxx>
#include <TopoDS.hxx>

#include <map>
#include <set>
#include <utility> // for std::pair
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>

TriangleStructure::TriangleStructure()
{
	myNodeCoords = new TColStd_HArray2OfReal(0, 0, 1, 3);
	myElemNodes = new TColStd_HArray2OfInteger(0, 0, 1, 3);
	myElemNormals = new TColStd_HArray2OfReal(0, 0, 1, 3);
}

TriangleStructure::TriangleStructure(const TopoDS_Shape& shape, const Standard_Real linearDeflection)
{
    // 正确初始化空数组
    myNodeCoords = new TColStd_HArray2OfReal(0, 0, 1, 3);
    myElemNodes = new TColStd_HArray2OfInteger(0, 0, 1, 3);
    myElemNormals = new TColStd_HArray2OfReal(0, 0, 1, 3);

    // 首先对形状进行网格划分
    BRepMesh_IncrementalMesh mesher(shape, linearDeflection);
    mesher.Perform();


    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(shape, TopAbs_FACE, faceMap);

    // 遍历所有面并提取三角网格
    TopExp_Explorer explorer(shape, TopAbs_FACE);
    for (; explorer.More(); explorer.Next()) {
        const TopoDS_Face& face = TopoDS::Face(explorer.Current());
        TopLoc_Location loc;
        Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, loc);

        if (!triangulation.IsNull()) {
            AddTriangulation(triangulation, loc);
        }
    }

	ExtractEdges();
 
}

void TriangleStructure::AddTriangulation(const Handle(Poly_Triangulation)& triangulation, const TopLoc_Location& loc)
{
    if (triangulation.IsNull())
        return;

    const Standard_Integer nbNodes = triangulation->NbNodes();
    const Standard_Integer nbTriangles = triangulation->NbTriangles();

    // 应用位置变换
    gp_Trsf transformation = loc.Transformation();

    // 用于映射局部节点ID到全局节点ID的临时数组
    TColStd_Array1OfInteger localToGlobalNodeMap(1, nbNodes);

    // 遍历所有局部节点，检查是否需要创建新节点或重用已有节点
    for (Standard_Integer i = 1; i <= nbNodes; ++i)
    {
        gp_Pnt p = triangulation->Node(i);
        if (!loc.IsIdentity()) {
            p.Transform(transformation);
        }

        // 检查是否已有相同坐标的节点存在
        auto it = myCoordToNodeMap.find(p);
        if (it != myCoordToNodeMap.end()) {
            // 重用已有节点
            localToGlobalNodeMap(i) = it->second;
        }
        else {
            // 创建新节点
            Standard_Integer newNodeId = myNodeCoords->UpperRow() + 1;
            if (newNodeId == 1) {
                myNodeCoords->Resize(1, 1, 1, 3, Standard_True);
            }
            else {
                myNodeCoords->Resize(1, newNodeId, 1, 3, Standard_True);
            }

            // 存储节点坐标
            myNodeCoords->SetValue(newNodeId, 1, p.X());
            myNodeCoords->SetValue(newNodeId, 2, p.Y());
            myNodeCoords->SetValue(newNodeId, 3, p.Z());

            // 添加到节点ID集合
            myNodes.Add(newNodeId);

            // 记录局部节点到全局节点的映射
            localToGlobalNodeMap(i) = newNodeId;

            // 将新节点添加到坐标到ID的映射中
            myCoordToNodeMap[p] = newNodeId;
        }
    }

    // 计算单元偏移量
    Standard_Integer elemOffset = myElemNodes->UpperRow();
    if (elemOffset == 0)
        elemOffset = 1;
    else
        elemOffset += 1;

    // 调整数组大小以容纳新单元
    myElemNodes->Resize(1, elemOffset + nbTriangles - 1, 1, 3, Standard_True);
    myElemNormals->Resize(1, elemOffset + nbTriangles - 1, 1, 3, Standard_True);

    // 添加所有单元
    for (Standard_Integer j = 1; j <= nbTriangles; j++)
    {
        auto aTri = triangulation->Triangle(j);
        Standard_Integer V[3];
        aTri.Get(V[0], V[1], V[2]);

        // 当前单元的全局ID
        Standard_Integer globalElemId = elemOffset + j - 1;

        // 使用映射的全局节点ID
        Standard_Integer n1 = localToGlobalNodeMap(V[0]);
        Standard_Integer n2 = localToGlobalNodeMap(V[1]);
        Standard_Integer n3 = localToGlobalNodeMap(V[2]);

        // 存储单元节点索引
        myElemNodes->SetValue(globalElemId, 1, n1);
        myElemNodes->SetValue(globalElemId, 2, n2);
        myElemNodes->SetValue(globalElemId, 3, n3);

        // 添加到单元ID集合
        myElements.Add(globalElemId);

        // 计算法向量
        gp_Pnt p1 = triangulation->Node(V[0]);
        gp_Pnt p2 = triangulation->Node(V[1]);
        gp_Pnt p3 = triangulation->Node(V[2]);
        if (!loc.IsIdentity())
        {
            p1.Transform(transformation);
            p2.Transform(transformation);
            p3.Transform(transformation);
        }

        gp_Vec  v1(p1, p2);
        gp_Vec  v2(p2, p3);
        gp_Vec  normal = v1.Crossed(v2);

        if (normal.SquareMagnitude() > MY_PRECISION * MY_PRECISION)
        {
            normal.Normalize();
        }

        // 存储法向量
        myElemNormals->SetValue(globalElemId, 1, normal.X());
        myElemNormals->SetValue(globalElemId, 2, normal.Y());
        myElemNormals->SetValue(globalElemId, 3, normal.Z());
    }


    //Standard_Integer nodeIdCount = myNodes.Extent();  // 节点ID的数量（即节点总数）
    //std::cout << "myNodes 包含的节点ID数量: " << nodeIdCount << std::endl;

    //Standard_Integer elementIdCount = myElements.Extent();  // 元素ID的数量（即元素总数）
    //std::cout << "myElements 包含的元素ID数量: " << elementIdCount << std::endl;

    //// 遍历所有元素
    //Standard_Integer elemMin = myElemNodes->LowerRow();
    //Standard_Integer elemMax = myElemNodes->UpperRow();
    //for (Standard_Integer elemID = elemMin; elemID <= elemMax; ++elemID)
    //{
    //    std::cout << "=== 元素ID: " << elemID << " ===" << std::endl;

    //    // 1. 获取元素的法向量
    //    Standard_Real nx = myElemNormals->Value(elemID, 1);
    //    Standard_Real ny = myElemNormals->Value(elemID, 2);
    //    Standard_Real nz = myElemNormals->Value(elemID, 3);
    //    std::cout << "法向量: (" << nx << ", " << ny << ", " << nz << ")" << std::endl;

    //    // 2. 获取元素包含的节点ID及坐标
    //    for (Standard_Integer i = 1; i <= 3; ++i)  // 三角形有3个节点
    //    {
    //        Standard_Integer nodeID = myElemNodes->Value(elemID, i);
    //        // 从节点坐标数组中获取该节点的坐标
    //        Standard_Real x = myNodeCoords->Value(nodeID, 1);
    //        Standard_Real y = myNodeCoords->Value(nodeID, 2);
    //        Standard_Real z = myNodeCoords->Value(nodeID, 3);
    //        std::cout << "节点" << i << " (ID=" << nodeID << "): "
    //            << "(" << x << ", " << y << ", " << z << ")" << std::endl;
    //    }
    //    std::cout << std::endl;
    //}
}

void TriangleStructure::ExtractEdges()
{
	std::set<std::pair<Standard_Integer, Standard_Integer>> edgeSet;

	Standard_Integer elemMin = myElemNodes->LowerRow();
	Standard_Integer elemMax = myElemNodes->UpperRow();
	for (Standard_Integer elemID = elemMin; elemID <= elemMax; ++elemID)
	{
		Standard_Integer n1 = myElemNodes->Value(elemID, 1);
		Standard_Integer n2 = myElemNodes->Value(elemID, 2);
		Standard_Integer n3 = myElemNodes->Value(elemID, 3);

		// 检查是否有退化的三角形（两个顶点相同
		if (n1 == n2 || n1 == n3 || n3 == n2)
		{
			continue;
		}

		// 确保边线的节点 ID 按升序排列
		auto edge1 = std::make_pair(std::min(n1, n2), std::max(n1, n2));
		auto edge2 = std::make_pair(std::min(n2, n3), std::max(n2, n3));
		auto edge3 = std::make_pair(std::min(n3, n1), std::max(n3, n1));

		edgeSet.insert(edge1);
		edgeSet.insert(edge2);
		edgeSet.insert(edge3);
	}
	myEdges.assign(edgeSet.begin(), edgeSet.end());
}

std::vector<std::pair<Standard_Integer, Standard_Integer>> TriangleStructure::GetMyEdge()
{
	return myEdges;
}

Handle(TColStd_HArray2OfReal) TriangleStructure::GetmyNodeCoords()
{
	return myNodeCoords;
}


//================================================================
// Function : GetGeom
// Purpose  :
//================================================================
Standard_Boolean TriangleStructure::GetGeom
    ( const Standard_Integer ID, const Standard_Boolean IsElement,
     TColStd_Array1OfReal& Coords, Standard_Integer& NbNodes,
     MeshVS_EntityType& Type ) const
{
    // if( myMesh.IsNull() )
    //     return Standard_False;

    if( IsElement )
    {
        if( ID>=1 && ID<=myElements.Extent() )
        {
            Type = MeshVS_ET_Face;
            NbNodes = 3;

            for( Standard_Integer i = 1, k = 1; i <= 3; i++ )
            {
                Standard_Integer IdxNode = myElemNodes->Value(ID, i);
                for(Standard_Integer j = 1; j <= 3; j++, k++ )
                    Coords(k) = myNodeCoords->Value(IdxNode, j);
            }

            return Standard_True;
        }
        else
            return Standard_False;
    }
    else
        if( ID>=1 && ID<=myNodes.Extent() )
        {
            Type = MeshVS_ET_Node;
            NbNodes = 1;

            Coords( 1 ) = myNodeCoords->Value(ID, 1);
            Coords( 2 ) = myNodeCoords->Value(ID, 2);
            Coords( 3 ) = myNodeCoords->Value(ID, 3);
            return Standard_True;
        }
        else
            return Standard_False;
}

//================================================================
// Function : GetGeomType
// Purpose  :
//================================================================
Standard_Boolean TriangleStructure::GetGeomType
    ( const Standard_Integer,
     const Standard_Boolean IsElement,
     MeshVS_EntityType& Type ) const
{
    if( IsElement )
    {
        Type = MeshVS_ET_Face;
        return Standard_True;
    }
    else
    {
        Type = MeshVS_ET_Node;
        return Standard_True;
    }
}

//================================================================
// Function : GetAddr
// Purpose  :
//================================================================
Standard_Address TriangleStructure::GetAddr
    ( const Standard_Integer, const Standard_Boolean ) const
{
    return NULL;
}

//================================================================
// Function : GetNodesByElement
// Purpose  :
//================================================================
Standard_Boolean TriangleStructure::GetNodesByElement
    ( const Standard_Integer ID,
     TColStd_Array1OfInteger& theNodeIDs,
     Standard_Integer& /*theNbNodes*/ ) const
{
    // if( myMesh.IsNull() )
    //     return Standard_False;

    if( ID>=1 && ID<=myElements.Extent() && theNodeIDs.Length() >= 3 )
    {
        Standard_Integer aLow = theNodeIDs.Lower();
        theNodeIDs (aLow)     = myElemNodes->Value(ID, 1 );
        theNodeIDs (aLow + 1) = myElemNodes->Value(ID, 2 );
        theNodeIDs (aLow + 2) = myElemNodes->Value(ID, 3 );
        return Standard_True;
    }
    return Standard_False;
}

//================================================================
// Function : GetAllNodes
// Purpose  :
//================================================================
const TColStd_PackedMapOfInteger& TriangleStructure::GetAllNodes() const
{
    return myNodes;
}

//================================================================
// Function : GetAllElements
// Purpose  :
//================================================================
const TColStd_PackedMapOfInteger& TriangleStructure::GetAllElements() const
{
    return myElements;
}

//================================================================
// Function : GetNormal
// Purpose  :
//================================================================
Standard_Boolean TriangleStructure::GetNormal
    ( const Standard_Integer Id, const Standard_Integer Max,
     Standard_Real& nx, Standard_Real& ny,Standard_Real& nz ) const
{
    // if( myMesh.IsNull() )
    //     return Standard_False;

    if( Id>=1 && Id<=myElements.Extent() && Max>=3 )
    {
        nx = myElemNormals->Value(Id, 1);
        ny = myElemNormals->Value(Id, 2);
        nz = myElemNormals->Value(Id, 3);
        return Standard_True;
    }
    else
        return Standard_False;
}


