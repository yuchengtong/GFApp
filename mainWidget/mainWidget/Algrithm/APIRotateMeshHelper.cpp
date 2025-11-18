#include "APIRotateMeshHelper.h"
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Ax1.hxx>
bool APIRotateMeshHelper::RotateAroundCenter(TriangleStructure& mesh,
    const TopoDS_Shape& shape,
    const gp_Dir& rotationAxis,
    Standard_Real angleRadians)
{
    /*
    // 检查网格是否包含节点
    const TColStd_PackedMapOfInteger& nodes = mesh.GetAllNodes();
    if (nodes.IsEmpty())
        return false;

    // 检查模型是否为空
    if (shape.IsNull())
        return false;

    // 获取节点坐标数组（用于修改节点位置）
    Handle(TColStd_HArray2OfReal) nodeCoords = mesh.GetmyNodeCoords();
    if (nodeCoords.IsNull())
        return false;

    // 1. 计算模型（Shape）的中心：通过包围盒
    Bnd_Box bbox;
    // 计算Shape的包围盒（自动处理位置变换）
    BRepBndLib::Add(shape, bbox);
    // 获取包围盒的最小/最大坐标
    Standard_Real xMin, yMin, zMin;
    Standard_Real xMax, yMax, zMax;
    bbox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
    // 计算包围盒中心点（模型中心）
    gp_Pnt center(
        (xMin + xMax) / 2.0,
        (yMin + yMax) / 2.0,
        (zMin + zMax) / 2.0
    );

    // 2. 构建旋转变换矩阵
    gp_Trsf transform;
    // 步骤1：将模型平移到原点（减去中心点坐标）
    transform.SetTranslation(gp_Vec(center.XYZ().Reversed()));

    // 步骤2：绕原点的指定轴旋转
    gp_Trsf rotation;
    rotation.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), rotationAxis), angleRadians);

    // 组合变换：先平移到原点，再旋转
    transform.Multiply(rotation);

    // 步骤3：将模型平移回原中心点
    transform.SetTranslationPart(transform.TranslationPart() + center.XYZ());

    // 3. 应用变换到所有节点
    for (Standard_Integer i = 1; i <= nodes.Extent(); ++i)
    {
        gp_Pnt node(
            nodeCoords->Value(i, 1),
            nodeCoords->Value(i, 2),
            nodeCoords->Value(i, 3)
        );
        node.Transform(transform); // 应用完整变换

        // 更新节点坐标
        nodeCoords->SetValue(i, 1, node.X());
        nodeCoords->SetValue(i, 2, node.Y());
        nodeCoords->SetValue(i, 3, node.Z());
    }

    // 4. 旋转所有三角形的法线（向量无需平移，只需旋转）
    const TColStd_PackedMapOfInteger& elements = mesh.GetAllElements();
    if (!elements.IsEmpty())
    {
        // 注意：需在TriangleStructure中添加GetElemNormals()方法（参考之前的回复）
        Handle(TColStd_HArray2OfReal) elemNormals = mesh.GetElemNormals();
        if (!elemNormals.IsNull())
        {
            for (Standard_Integer j = 1; j <= elements.Extent(); ++j)
            {
                gp_Vec normal(
                    elemNormals->Value(j, 1),
                    elemNormals->Value(j, 2),
                    elemNormals->Value(j, 3)
                );
                normal.Transform(rotation); // 仅应用旋转变换

                // 保持法线单位长度
                if (normal.SquareMagnitude() > MY_PRECISION * MY_PRECISION)
                {
                    normal.Normalize();
                }

                // 更新法线坐标
                elemNormals->SetValue(j, 1, normal.X());
                elemNormals->SetValue(j, 2, normal.Y());
                elemNormals->SetValue(j, 3, normal.Z());
            }
        }
    }

    // 5. 重建坐标到节点ID的映射（因坐标已改变）
    mesh.RebuildCoordToNodeMap(); // 需在TriangleStructure中实现该方法
    */
    return true;
}

