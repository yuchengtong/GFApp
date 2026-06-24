#pragma execution_character_set("utf-8")
#include "TriangulationWorker.h"
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Face.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_Triangle.hxx>
#include <BRep_Tool.hxx>
#include <gp_Pnt.hxx>
#include <QThread>
#include <cmath>

bool TriangulationWorker::TriangulateSingleShape(
    const Handle(AIS_Shape)& aisShape,
    const QString& name,
    Handle(TriangleStructure)& outMesh,
    double& out_x_min, double& out_x_max,
    double& out_z_min, double& out_z_max,
    int progressStart, int progressEnd)
{
    emit StatusUpdated(QString("正在处理%1网格划分...").arg(name));
    emit ProgressUpdated(progressStart);

    if (m_interrupted)
    {
        return false;
    }

    // 检查 AIS_Shape 是否为空
    if (aisShape.IsNull())
    {
        emit StatusUpdated(QString("%1几何为空，跳过").arg(name));
        return false;
    }

    // 检查内部 Shape 是否为空
    TopoDS_Shape shape = aisShape->Shape();
    if (shape.IsNull())
    {
        emit StatusUpdated(QString("%1几何内部Shape为空，跳过").arg(name));
        return false;
    }

    TopExp_Explorer faceExplorer(shape, TopAbs_FACE);
    if (!faceExplorer.More())
    {
        emit StatusUpdated(QString("%1几何没有面，跳过").arg(name));
        return false;
    }

    emit StatusUpdated(QString("正在计算%1三角网格").arg(name));
    emit ProgressUpdated((progressStart + progressEnd) / 2);

    if (m_interrupted)
    {
        return false;
    }

    // 执行网格划分
    TriangleStructure* rawMesh = new TriangleStructure(aisShape, 10.0, &m_interrupted);
    Handle(TriangleStructure) pMesh(rawMesh);

    if (pMesh.IsNull() || pMesh->GetAllNodes().IsEmpty())
    {
        emit StatusUpdated(QString("%1网格划分失败：未生成有效节点").arg(name));
        return false;
    }

    if (m_interrupted)
    {
        return false;
    }

    // 计算边界框
    auto allNodes = pMesh->GetAllNodes();
    auto nodeCoors = pMesh->GetmyNodeCoords();

    double x_min = DBL_MAX, x_max = -DBL_MAX;
    double z_min = DBL_MAX, z_max = -DBL_MAX;

    if (!allNodes.IsEmpty())
    {
        for (TColStd_PackedMapOfInteger::Iterator it(allNodes); it.More(); it.Next())
        {
            int nodeID = it.Key();
            double x = nodeCoors->Value(nodeID, 1);
            double z = nodeCoors->Value(nodeID, 3);

            x_min = std::min(x_min, x);
            x_max = std::max(x_max, x);
            z_min = std::min(z_min, z);
            z_max = std::max(z_max, z);
        }
    }

    out_x_min = x_min;
    out_x_max = x_max;
    out_z_min = z_min;
    out_z_max = z_max;

    outMesh = pMesh;

    emit ProgressUpdated(progressEnd);
    return true;
}

void TriangulationWorker::DoWork()
{
    ModelMeshInfo meshInfo;
    bool success = false;
    QString msg;

    try
    {
        emit StatusUpdated("开始网格划分准备...");
        emit ProgressUpdated(5);

        if (m_interrupted)
        {
            emit WorkFinished(false, "网格划分已取消", meshInfo);
            return;
        }

        // ========== 1. 壳体网格划分 ==========
        bool shellOK = TriangulateSingleShape(
            m_shellAisShape, "壳体",
            meshInfo.shellMesh,
            meshInfo.shell_x_min, meshInfo.shell_x_max,
            meshInfo.shell_z_min, meshInfo.shell_z_max,
            10, 35);

        if (m_interrupted)
        {
            emit WorkFinished(false, "网格划分已取消", meshInfo);
            return;
        }

        // ========== 2. 推进剂网格划分 ==========
        bool propellantOK = TriangulateSingleShape(
            m_propellantAisShape, "推进剂",
            meshInfo.propellantMesh,
            meshInfo.propellant_x_min, meshInfo.propellant_x_max,
            meshInfo.propellant_z_min, meshInfo.propellant_z_max,
            40, 65);

        if (m_interrupted)
        {
            emit WorkFinished(false, "网格划分已取消", meshInfo);
            return;
        }

        // ========== 3. 隔热层网格划分 ==========
        bool heatInsulatingOK = TriangulateSingleShape(
            m_heatInsulatingAisShape, "隔热层",
            meshInfo.heatInsulatingLayerMesh,
            meshInfo.heatInsulating_x_min, meshInfo.heatInsulating_x_max,
            meshInfo.heatInsulating_z_min, meshInfo.heatInsulating_z_max,
            70, 95);

        if (m_interrupted)
        {
            emit WorkFinished(false, "网格划分已取消", meshInfo);
            return;
        }

        // ========== 判断总体结果 ==========
        // isChecked = true 表示三种都成功
        meshInfo.isChecked = shellOK && propellantOK && heatInsulatingOK;

        if (meshInfo.isChecked)
        {
            success = true;
            msg = "所有几何网格划分完成";
        }
        else
        {
            success = false;
            msg = QString("部分失败：壳体[%1] 推进剂[%2] 隔热层[%3]")
                .arg(shellOK ? "成功" : "失败")
                .arg(propellantOK ? "成功" : "失败")
                .arg(heatInsulatingOK ? "成功" : "失败");
        }

        emit StatusUpdated("网格划分统计信息");
        emit ProgressUpdated(100);
    }
    catch (const Standard_Failure& e)
    {
        msg = QString("网格划分错误: %1").arg(e.GetMessageString());
        success = false;
    }
    catch (const std::exception& e)
    {
        msg = QString("网格划分错误: %1").arg(e.what());
        success = false;
    }
    catch (...)
    {
        msg = "网格划分时发生未知错误";
        success = false;
    }

    if (m_interrupted)
    {
        emit WorkFinished(false, "网格划分已取消", meshInfo);
    }
    else
    {
        emit WorkFinished(success, msg, meshInfo);
    }
}

void TriangulationWorker::RequestInterruption()
{
    m_interrupted = true;
}