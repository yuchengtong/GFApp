#ifndef TRIANGULATION_WORKER_H
#define TRIANGULATION_WORKER_H

#include <QObject>
#include <Poly_Triangulation.hxx>
#include <TopoDS_Shape.hxx>
#include <TopLoc_Location.hxx>
#include "TriangleStructure.h"

class TriangulationWorker : public QObject
{
    Q_OBJECT

public:
    // 构造函数1：处理STL三角化（输入STL网格）
    explicit TriangulationWorker(Handle(Poly_Triangulation) stlMesh,
        const TopLoc_Location& loc,
        Handle(TriangleStructure) dataSource,
        QObject* parent = nullptr);

    // 构造函数2：处理OCC形状三角化（输入OCC形状）
    explicit TriangulationWorker(const TopoDS_Shape& shape,
        const Standard_Real linearDeflection,
        Handle(TriangleStructure) dataSource,
        QObject* parent = nullptr);

public slots:
    // 线程核心执行函数（后台线程运行）
    void DoWork();

    // 接收取消请求
    void RequestInterruption();

signals:
    // 进度转发（0~100）
    void ProgressUpdated(int progress);

    // 状态转发（如“处理节点1000/5000”）
    void StatusUpdated(const QString& status);

    // 任务完成（success：是否成功，msg：结果信息，dataSource：三角化结果）
    void WorkFinished(bool success, const QString& msg, Handle(TriangleStructure) dataSource);

private:
    // 处理STL网格的参数
    Handle(Poly_Triangulation) m_stlMesh;
    TopLoc_Location m_loc;

    // 处理OCC形状的参数
    TopoDS_Shape m_shape;
    Standard_Real m_linearDeflection;

    // 三角化结果数据源
    Handle(TriangleStructure) m_dataSource;

    // 任务类型（区分STL和OCC形状）
    enum TaskType { Task_STL, Task_Shape };
    TaskType m_taskType;

    // 中断标志
    volatile bool m_interrupted = false;
};

#endif // TRIANGULATION_WORKER_H