#ifndef GEOMETRY_IMPORT_WORKER_H
#define GEOMETRY_IMPORT_WORKER_H

#include <QObject>
#include <TopoDS_Shape.hxx>
#include <QString>
#include <TDocStd_Document.hxx>
#include <TDF_Label.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TopLoc_Location.hxx>
#include "ModelDataManager.h"

enum class PartType {
    Nozzle,
    Shell,           // 壳体
    Propellant,      // 推进剂
    HeatInsulatingLayer, // 绝热层
    Unknown          // 未知
};

class GeometryImportWorker : public QObject
{
    Q_OBJECT

public:
    explicit GeometryImportWorker(const QString& filePath, QObject* parent = nullptr)
        : QObject(parent), m_filePath(filePath), m_interrupted(false) {}

    void SetPartType(PartType type);

    bool FindConnectionCircle(const TopoDS_Shape& shape, gp_Pnt& center, double& radius);

    void AnalyzeGeometry(ModelGeometryInfo& info);

    void FindShellBottomPoints(ModelGeometryInfo& info);

    void FindNozzleOutletPoint(ModelGeometryInfo& info);


public slots:
    void DoWork();

    void RequestInterruption();

signals:
    void ProgressUpdated(int progress);
    void StatusUpdated(const QString& status);
    void WorkFinished(bool success, QString msg, ModelGeometryInfo info);

private:
    // 解析STEP文件
    bool ImportSTEP(ModelGeometryInfo& info);
    // 解析STL文件
    bool ImportSTL(ModelGeometryInfo& info);
    // 解析IGES文件
    bool ImportIGES(ModelGeometryInfo& info);

private:


private:
    QString m_filePath;
    std::atomic<bool> m_interrupted;

    PartType m_partType = PartType::Unknown;
};

#endif // GEOMETRY_IMPORT_WORKER_H

