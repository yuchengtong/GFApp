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

        if (!CheckGeometryValidity())
        {
            msg = "输入几何模型无效";
            emit WorkFinished(false, msg, meshInfo);
            return;  // 添加立即返回
        }

        // 检查中断
        if (m_interrupted)
        {
            emit WorkFinished(false, "网格划分已取消", meshInfo);
            return;
        }

        emit StatusUpdated("三角化网格划分");
        emit ProgressUpdated(30);

        // 检查中断
        if (m_interrupted)
        {
            emit WorkFinished(false, "网格划分已取消", meshInfo);
            return;
        }

        auto aDataSource = new TriangleStructure(m_originalShape, 0.5, &m_interrupted);

        // 在设置结果前检查中断
        if (m_interrupted)
        {
            delete aDataSource;  // 清理资源
            emit WorkFinished(false, "网格划分已取消", meshInfo);
            return;
        }

        meshInfo.isChecked = true;
        meshInfo.triangleStructure = *aDataSource;

        //创建旋转网格
        auto modelGeometryInfo= ModelDataManager::GetInstance()->GetModelGeometryInfo();
        auto meshData45=aDataSource->RotateXZ(45,(modelGeometryInfo.theXmin + modelGeometryInfo.theXmax)/2.0, 
            (modelGeometryInfo.theZmin + modelGeometryInfo.theZmax) / 2.0);
        auto meshData90 = aDataSource->RotateXZ(90, (modelGeometryInfo.theXmin + modelGeometryInfo.theXmax) / 2.0,
            (modelGeometryInfo.theZmin + modelGeometryInfo.theZmax) / 2.0);
        meshInfo.triangleStructure45 = *meshData45;
        meshInfo.triangleStructure90 = *meshData90;

        //delete aDataSource;  // 清理资源

        // 检查中断
        if (m_interrupted)
        {
            emit WorkFinished(false, "网格划分已取消", meshInfo);
            return;
        }

        emit StatusUpdated("计算网格统计信息");
        emit ProgressUpdated(85);

        // 模拟耗时操作，但需要检查中断
        for (int i = 0; i < 50; ++i)
        {
            if (m_interrupted)
            {
                emit WorkFinished(false, "网格划分已取消", meshInfo);
                return;
            }
            QThread::msleep(10);
        }

        // 最终检查中断
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
    catch (...)
    {
        msg = "网格划分时发生未知错误";
        success = false;
    }

    // 最终检查：如果在中途被取消，覆盖之前的成功状态
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



