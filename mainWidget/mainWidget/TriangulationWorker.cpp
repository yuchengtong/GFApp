#include "TriangulationWorker.h"
#include <QThread>

// 构造函数1：处理STL网格三角化
TriangulationWorker::TriangulationWorker(Handle(Poly_Triangulation) stlMesh,
    const TopLoc_Location& loc,
    Handle(TriangleStructure) dataSource,
    QObject* parent)
    : QObject(parent),
    m_stlMesh(stlMesh),
    m_loc(loc),
    m_dataSource(dataSource),
    m_taskType(Task_STL),
    m_interrupted(false)
{
    // 连接TriangleStructure的信号到自身（转发给UI）
    if (!m_dataSource.IsNull())
    {
        connect(m_dataSource.get(), &TriangleStructure::ProgressUpdated,
            this, &TriangulationWorker::ProgressUpdated);
        connect(m_dataSource.get(), &TriangleStructure::StatusUpdated,
            this, &TriangulationWorker::StatusUpdated);
    }
}

// 构造函数2：处理OCC形状三角化
TriangulationWorker::TriangulationWorker(const TopoDS_Shape& shape,
    const Standard_Real linearDeflection,
    Handle(TriangleStructure) dataSource,
    QObject* parent)
    : QObject(parent),
    m_shape(shape),
    m_linearDeflection(linearDeflection),
    m_dataSource(dataSource),
    m_taskType(Task_Shape),
    m_interrupted(false)
{
    // 连接TriangleStructure的信号到自身（转发给UI）
    if (!m_dataSource.IsNull())
    {
        connect(m_dataSource.get(), &TriangleStructure::ProgressUpdated,
            this, &TriangulationWorker::ProgressUpdated);
        connect(m_dataSource.get(), &TriangleStructure::StatusUpdated,
            this, &TriangulationWorker::StatusUpdated);
    }
}

// 线程核心执行函数
void TriangulationWorker::DoWork()
{
    // 1. 检查核心参数有效性
    if (m_dataSource.IsNull())
    {
        emit WorkFinished(false, "错误：三角化数据源为空", m_dataSource);
        return;
    }

    // 2. 根据任务类型执行三角化
    bool success = true;
    QString msg = "三角化成功";

    try
    {
        switch (m_taskType)
        {
        case Task_STL:
            // 处理STL网格
            if (m_stlMesh.IsNull())
            {
                success = false;
                msg = "错误：STL网格为空";
            }
            else
            {
                m_dataSource->AddTriangulation(m_stlMesh, m_loc);
                if (m_dataSource->IsInterrupted())
                {
                    success = false;
                    msg = "三角化已取消（STL处理）";
                }
                else
                {
                    msg = QString("STL三角化成功：节点%1个，单元%2个")
                        .arg(m_dataSource->GetAllNodes()->Length())
                        .arg(m_dataSource->GetElemNodes()->UpperRow());
                }
            }
            break;

        case Task_Shape:
            // 处理OCC形状
            if (m_shape.IsNull())
            {
                success = false;
                msg = "错误：OCC形状为空";
            }
            else
            {
                m_dataSource->TriangulateShape(m_shape, m_linearDeflection);
                if (m_dataSource->IsInterrupted())
                {
                    success = false;
                    msg = "三角化已取消（OCC形状处理）";
                }
                else
                {
                    msg = QString("OCC形状三角化成功：节点%1个，单元%2个")
                        .arg(m_dataSource->GetAllNodes()->Length())
                        .arg(m_dataSource->GetElemNodes()->UpperRow());
                }
            }
            break;

        default:
            success = false;
            msg = "错误：未知任务类型";
            break;
        }
    }
    catch (const Standard_Failure& e)
    {
        // 捕获OCC异常
        success = false;
        msg = QString("三角化异常：%1").arg(e.GetMessageString());
    }
    catch (...)
    {
        // 捕获其他异常
        success = false;
        msg = "三角化异常：未知错误";
    }

    // 3. 发送完成信号（携带结果）
    emit WorkFinished(success, msg, m_dataSource);
}

// 接收取消请求
void TriangulationWorker::RequestInterruption()
{
    m_interrupted = true;
    if (!m_dataSource.IsNull())
    {
        m_dataSource->RequestInterruption(); // 通知TriangleStructure中断
    }
}