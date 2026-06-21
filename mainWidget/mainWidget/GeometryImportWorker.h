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

class GeometryImportWorker : public QObject
{
    Q_OBJECT

public:
    explicit GeometryImportWorker(const QString& filePath, QObject* parent = nullptr)
        : QObject(parent), m_filePath(filePath), m_interrupted(false) {}

public slots:
    void DoWork();

    void RequestInterruption();

signals:
    void ProgressUpdated(int progress);
    void StatusUpdated(const QString& status);
    void WorkFinished(bool success, const QString& msg, const ModelGeometryInfo& info);

private:
    QString m_filePath;
    std::atomic<bool> m_interrupted;
    TopoDS_Shape m_shape;

    // 解析STEP文件
    bool ImportSTEP(ModelGeometryInfo& info);
    // 解析STL文件
    bool ImportSTL(ModelGeometryInfo& info);
    // 解析IGES文件
    bool ImportIGES(ModelGeometryInfo& info);
    // 计算边界盒
    void CalculateBoundingBox(ModelGeometryInfo& info);


    void AnalyzeNozzleConnection(ModelGeometryInfo& info);
    bool FindConnectionCircle(const TopoDS_Shape& shape, gp_Pnt& center, double& radius);
    void FindBottomEndPoint(ModelGeometryInfo& info);  // 新增

};

#endif // GEOMETRY_IMPORT_WORKER_H

