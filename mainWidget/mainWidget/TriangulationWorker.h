#pragma once
#include <QObject>
#include <AIS_Shape.hxx>
#include <QString>
#include "ModelDataManager.h"

class TriangulationWorker : public QObject
{
    Q_OBJECT

public:
    // 修改构造函数：接收三个 Handle(AIS_Shape)
    explicit TriangulationWorker(
        const Handle(AIS_Shape)& shellAisShape,
        const Handle(AIS_Shape)& propellantAisShape,
        const Handle(AIS_Shape)& heatInsulatingAisShape,
        QObject* parent = nullptr)
        : QObject(parent)
        , m_shellAisShape(shellAisShape)
        , m_propellantAisShape(propellantAisShape)
        , m_heatInsulatingAisShape(heatInsulatingAisShape)
        , m_interrupted(false)
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
    Handle(AIS_Shape) m_shellAisShape;
    Handle(AIS_Shape) m_propellantAisShape;
    Handle(AIS_Shape) m_heatInsulatingAisShape;
    volatile bool m_interrupted;

    // 辅助函数：处理单个几何的网格划分
    bool TriangulateSingleShape(
        const Handle(AIS_Shape)& aisShape,
        const QString& name,
        Handle(TriangleStructure)& outMesh,
        double& out_x_min, double& out_x_max,
        double& out_z_min, double& out_z_max,
        int progressStart, int progressEnd);
};
