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

TriangleStructure::TriangleStructure(const TopoDS_Shape& shape, const Standard_Real linearDeflection, volatile bool* interrupted)
{
    // 正确初始化空数组
    myNodeCoords = new TColStd_HArray2OfReal(0, 0, 1, 3);
    myElemNodes = new TColStd_HArray2OfInteger(0, 0, 1, 3);
    myElemNormals = new TColStd_HArray2OfReal(0, 0, 1, 3);

    // 检查中断
    if (CheckInterruption(interrupted)) 
    {
        return;
    }

    // 首先对形状进行网格划分
    BRepMesh_IncrementalMesh mesher(shape, 0.08, Standard_False,0.2, Standard_False);
    //BRepMesh_IncrementalMesh mesher(shape, 0.8, Standard_False, 0.8, Standard_False);
    mesher.Perform();

    if (CheckInterruption(interrupted))
    {
        return;
    }

    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(shape, TopAbs_FACE, faceMap);

    // 遍历所有面并提取三角网格
    TopExp_Explorer explorer(shape, TopAbs_FACE);
    int faceCount = 0;
    int totalFaces = faceMap.Extent();
    for (; explorer.More(); explorer.Next())
    {
        // 每处理几个面检查一次中断
        if (faceCount % 10 == 0 && CheckInterruption(interrupted)) 
        {
            return;
        }

        const TopoDS_Face& face = TopoDS::Face(explorer.Current());
        TopLoc_Location loc;
        Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, loc);

        if (!triangulation.IsNull())
        {
            AddTriangulation(triangulation, loc);

            // 检查中断
            if (CheckInterruption(interrupted)) 
            {
                return;
            }
        }

        faceCount++;
    }

    if (CheckInterruption(interrupted))
    {
        return;
    }

	ExtractEdges();
 
}





bool TriangleStructure::CheckInterruption(volatile bool* interrupted) const
{
    return interrupted && *interrupted;
}

void TriangleStructure::AddTriangulation(const Handle(Poly_Triangulation)& triangulation, const TopLoc_Location& loc, volatile bool* interrupted)
{
    if (triangulation.IsNull())
        return;

    // 检查中断
    if (CheckInterruption(interrupted)) 
    {
        return;
    }


    const Standard_Integer nbNodes = triangulation->NbNodes();
    const Standard_Integer nbTriangles = triangulation->NbTriangles();


    // 应用位置变换
    gp_Trsf transformation = loc.Transformation();
    const Standard_Boolean isIdentity = loc.IsIdentity();

    // --- 优化点 1: 预分配内存 ---
    // 为新单元预分配空间
    Standard_Integer newElemCount = nbTriangles;
    Standard_Integer startElemId = myElemNodes->UpperRow() + 1;
    if (startElemId > 1)
    { // 如果不是第一个单元
        myElemNodes->Resize(1, startElemId + newElemCount - 1, 1, 3, Standard_True);
        myElemNormals->Resize(1, startElemId + newElemCount - 1, 1, 3, Standard_True);
    }
    else 
    { // 如果是第一个单元
        myElemNodes->Resize(1, newElemCount, 1, 3, Standard_True);
        myElemNormals->Resize(1, newElemCount, 1, 3, Standard_True);
    }

    // 用于映射局部节点ID到全局节点ID的临时数组
    std::vector<Standard_Integer> localToGlobalNodeMap(nbNodes + 1); // 1-based indexing

    // --- 优化点 2: 批量处理节点，并建立局部到全局的映射 ---
    for (Standard_Integer i = 1; i <= nbNodes; ++i)
    {
        gp_Pnt p = triangulation->Node(i);
        if (!isIdentity) {
            p.Transform(transformation);
        }
        gp_XYZ p_xyz = p.XYZ(); // 使用 gp_XYZ 进行比较和存储，效率更高

        // 检查是否已有相同坐标的节点存在
        auto it = myCoordToNodeMap.find(p_xyz);
        if (it != myCoordToNodeMap.end()) {
            // 重用已有节点
            localToGlobalNodeMap[i] = it->second;
        }
        else {
            // 创建新节点
            Standard_Integer newNodeId = myNodeCoords->UpperRow() + 1;
            myNodeCoords->Resize(1, newNodeId, 1, 3, Standard_True); // Resize一次

            // 存储节点坐标
            myNodeCoords->SetValue(newNodeId, 1, p.X());
            myNodeCoords->SetValue(newNodeId, 2, p.Y());
            myNodeCoords->SetValue(newNodeId, 3, p.Z());

            myNodes.Add(newNodeId);
            localToGlobalNodeMap[i] = newNodeId;
            myCoordToNodeMap[p_xyz] = newNodeId;
        }
    }

    // --- 优化点 3: 批量处理单元和法向量，避免重复变换 ---
    for (Standard_Integer j = 1; j <= nbTriangles; j++)
    {
        auto aTri = triangulation->Triangle(j);
        Standard_Integer V[3];
        aTri.Get(V[0], V[1], V[2]);

        Standard_Integer globalElemId = startElemId + j - 1;

        Standard_Integer n1 = localToGlobalNodeMap[V[0]];
        Standard_Integer n2 = localToGlobalNodeMap[V[1]];
        Standard_Integer n3 = localToGlobalNodeMap[V[2]];

        // 存储单元节点索引
        myElemNodes->SetValue(globalElemId, 1, n1);
        myElemNodes->SetValue(globalElemId, 2, n2);
        myElemNodes->SetValue(globalElemId, 3, n3);

        myElements.Add(globalElemId);

        // --- 优化点 3 (续): 直接使用全局坐标计算法向量 ---
        gp_Pnt p1(myNodeCoords->Value(n1, 1), myNodeCoords->Value(n1, 2), myNodeCoords->Value(n1, 3));
        gp_Pnt p2(myNodeCoords->Value(n2, 1), myNodeCoords->Value(n2, 2), myNodeCoords->Value(n2, 3));
        gp_Pnt p3(myNodeCoords->Value(n3, 1), myNodeCoords->Value(n3, 2), myNodeCoords->Value(n3, 3));

        gp_Vec v1(p1, p2);
        gp_Vec v2(p1, p3); // 通常从第一个点出发，结果一样
        gp_Vec normal = v1.Crossed(v2);

        if (normal.SquareMagnitude() > MY_PRECISION * MY_PRECISION)
        {
            normal.Normalize();
        }

        // 存储法向量
        myElemNormals->SetValue(globalElemId, 1, normal.X());
        myElemNormals->SetValue(globalElemId, 2, normal.Y());
        myElemNormals->SetValue(globalElemId, 3, normal.Z());
    }  
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

Handle(TriangleStructure) TriangleStructure::RotateXZ(const Standard_Real angleDeg, const Standard_Real x0, const Standard_Real z0) const
{
    // 创建新的TriangleStructure对象
    Handle(TriangleStructure) rotatedStructure = new TriangleStructure();

    // 将角度转换为弧度
    Standard_Real angleRad = angleDeg * M_PI / 180.0;
    Standard_Real cosAngle = cos(angleRad);
    Standard_Real sinAngle = sin(angleRad);

    // 复制节点坐标并应用旋转
    Standard_Integer nodeCount = myNodeCoords->UpperRow() - myNodeCoords->LowerRow() + 1;
    rotatedStructure->myNodeCoords = new TColStd_HArray2OfReal(1, nodeCount, 1, 3);

    for (Standard_Integer i = 1; i <= nodeCount; ++i)
    {
        Standard_Real x = myNodeCoords->Value(i, 1);
        Standard_Real y = myNodeCoords->Value(i, 2);
        Standard_Real z = myNodeCoords->Value(i, 3);

        // 平移到旋转中心
        Standard_Real translatedX = x - x0;
        Standard_Real translatedZ = z - z0;

        // 应用旋转矩阵（绕Y轴旋转，在XOZ平面）
        Standard_Real rotatedX = translatedX * cosAngle + translatedZ * sinAngle;
        Standard_Real rotatedZ = -translatedX * sinAngle + translatedZ * cosAngle;

        // 平移回原位置
        rotatedX += x0;
        rotatedZ += z0;

        // 设置旋转后的坐标
        rotatedStructure->myNodeCoords->SetValue(i, 1, rotatedX);
        rotatedStructure->myNodeCoords->SetValue(i, 2, y);  // Y坐标不变
        rotatedStructure->myNodeCoords->SetValue(i, 3, rotatedZ);

        // 添加到节点集合
        rotatedStructure->myNodes.Add(i);
    }

    // 复制单元连接关系（不变）
    Standard_Integer elemCount = myElemNodes->UpperRow() - myElemNodes->LowerRow() + 1;
    rotatedStructure->myElemNodes = new TColStd_HArray2OfInteger(1, elemCount, 1, 3);

    for (Standard_Integer i = 1; i <= elemCount; ++i)
    {
        rotatedStructure->myElemNodes->SetValue(i, 1, myElemNodes->Value(i, 1));
        rotatedStructure->myElemNodes->SetValue(i, 2, myElemNodes->Value(i, 2));
        rotatedStructure->myElemNodes->SetValue(i, 3, myElemNodes->Value(i, 3));
        rotatedStructure->myElements.Add(i);
    }

    // 重新计算法向量（因为旋转后法向量也改变了）
    rotatedStructure->myElemNormals = new TColStd_HArray2OfReal(1, elemCount, 1, 3);

    for (Standard_Integer i = 1; i <= elemCount; ++i)
    {
        Standard_Integer n1 = rotatedStructure->myElemNodes->Value(i, 1);
        Standard_Integer n2 = rotatedStructure->myElemNodes->Value(i, 2);
        Standard_Integer n3 = rotatedStructure->myElemNodes->Value(i, 3);

        // 获取旋转后的节点坐标
        gp_Pnt p1(
            rotatedStructure->myNodeCoords->Value(n1, 1),
            rotatedStructure->myNodeCoords->Value(n1, 2),
            rotatedStructure->myNodeCoords->Value(n1, 3)
        );
        gp_Pnt p2(
            rotatedStructure->myNodeCoords->Value(n2, 1),
            rotatedStructure->myNodeCoords->Value(n2, 2),
            rotatedStructure->myNodeCoords->Value(n2, 3)
        );
        gp_Pnt p3(
            rotatedStructure->myNodeCoords->Value(n3, 1),
            rotatedStructure->myNodeCoords->Value(n3, 2),
            rotatedStructure->myNodeCoords->Value(n3, 3)
        );

        // 计算新的法向量
        gp_Vec v1(p1, p2);
        gp_Vec v2(p1, p3);
        gp_Vec normal = v1.Crossed(v2);

        if (normal.SquareMagnitude() > MY_PRECISION * MY_PRECISION)
        {
            normal.Normalize();
        }

        // 存储新的法向量
        rotatedStructure->myElemNormals->SetValue(i, 1, normal.X());
        rotatedStructure->myElemNormals->SetValue(i, 2, normal.Y());
        rotatedStructure->myElemNormals->SetValue(i, 3, normal.Z());
    }

    // 重新构建坐标到节点的映射
    for (Standard_Integer i = 1; i <= nodeCount; ++i)
    {
        gp_Pnt p(
            rotatedStructure->myNodeCoords->Value(i, 1),
            rotatedStructure->myNodeCoords->Value(i, 2),
            rotatedStructure->myNodeCoords->Value(i, 3)
        );
        rotatedStructure->myCoordToNodeMap[p] = i;
    }

    // 重新提取边信息
    rotatedStructure->ExtractEdges();

    return rotatedStructure;
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


