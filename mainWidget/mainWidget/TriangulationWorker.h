#ifndef TRIANGULATION_WORKER_H
#define TRIANGULATION_WORKER_H

#include <QObject>
#include <TopoDS_Shape.hxx>
#include <QString>
#include "ModelDataManager.h"

class TriangulationWorker : public QObject
{
    Q_OBJECT

public:
    explicit TriangulationWorker(const TopoDS_Shape& shape,QObject* parent = nullptr)
        : QObject(parent),m_originalShape(shape),m_interrupted(false) 
    {
    }

public slots:
    void DoWork();

    void RequestInterruption();

signals:
    void ProgressUpdated(int progress);

    void StatusUpdated(const QString& status);

    void WorkFinished(bool success, const QString& msg, const ModelMeshInfo& info);

private:
    TopoDS_Shape m_originalShape;  // 待划分网格的原始几何
    volatile bool m_interrupted;

    // 检查输入几何有效性
    bool CheckGeometryValidity();
};

#endif // TRIANGULATION_WORKER_H