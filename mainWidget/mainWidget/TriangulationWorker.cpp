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
//#include <StlAPI_Reader.hxx>

void TriangulationWorker::DoWork()
{
    ModelMeshInfo meshInfo;
    bool success = false;
    QString msg;

    std::unique_ptr<TriangleStructure> aDataSource;

    try
    {
        emit StatusUpdated("开始网格划分准备...");
        emit ProgressUpdated(5);

        if (m_interrupted)
        {
            emit WorkFinished(false, "网格划分已取消", meshInfo);
            return;
        }

        emit StatusUpdated("三角化网格划分");
        emit ProgressUpdated(30);

        if (m_interrupted)
        {
            emit WorkFinished(false, "网格划分已取消", meshInfo);
            return;
        }

        aDataSource.reset(new TriangleStructure(m_originalShape, 10.0, &m_interrupted));

        if (!aDataSource || aDataSource->GetAllNodes().IsEmpty())
        {
            emit WorkFinished(false, "网格生成失败：未产生有效节点", meshInfo);
            return;
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "网格划分已取消", meshInfo);
            return;
        }

        auto allNodes = aDataSource->GetAllNodes();
        auto nodeCoors = aDataSource->GetmyNodeCoords();

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

        meshInfo.x_min = x_min;
        meshInfo.x_max = x_max;
        meshInfo.z_min = z_min;
        meshInfo.z_max = z_max;

        if (m_interrupted)
        {
            emit WorkFinished(false, "网格划分已取消", meshInfo);
            return;
        }

        meshInfo.isChecked = true;

        meshInfo.triangleStructure = *aDataSource;
        aDataSource.reset();  // 拷贝完成后立即释放

        emit StatusUpdated("计算网格统计信息");
        emit ProgressUpdated(85);

        if (m_interrupted)
        {
            emit WorkFinished(false, "网格划分已取消", meshInfo);
            return;
        }

        success = true;
        msg = "网格划分完成";
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

    // 最终检查：如果中途被取消，覆盖成功状态
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


bool TriangulationWorker::CheckGeometryValidity()
{
    emit StatusUpdated("检查几何模型");
    emit ProgressUpdated(15);

    if (m_originalShape.IsNull())
    {
        return false;
    }

    TopExp_Explorer faceExplorer(m_originalShape, TopAbs_FACE);
    if (!faceExplorer.More())
    {
        return false;
    }

    if (m_interrupted)
    {
        return false;
    }

    emit ProgressUpdated(25);
    return true;
}



