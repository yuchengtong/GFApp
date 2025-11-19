#pragma execution_character_set("utf-8")
#include "GeometryImportWorker.h"
#include <STEPControl_Reader.hxx>
#include <StlAPI_Reader.hxx>
#include <IGESControl_Reader.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <QThread>

void GeometryImportWorker::DoWork()
{
    ModelGeometryInfo info;
    bool success = false;
    QString msg;

    try
    {
        emit StatusUpdated("开始解析文件...");
        emit ProgressUpdated(10);

        if (m_interrupted)
        {
            emit WorkFinished(false, "导入已取消", info);
            return;
        }

        // 根据文件后缀选择导入方式
        if (m_filePath.endsWith(".stp", Qt::CaseInsensitive) ||
            m_filePath.endsWith(".step", Qt::CaseInsensitive))
        {
            success = ImportSTEP(info);
        }
        else if (m_filePath.endsWith(".stl", Qt::CaseInsensitive))
        {
            success = ImportSTL(info);
        }
        else if (m_filePath.endsWith(".iges", Qt::CaseInsensitive) ||
            m_filePath.endsWith(".igs", Qt::CaseInsensitive))
        {
            success = ImportIGES(info);
        }
        else
        {
            msg = "不支持的文件格式";
            success = false;
        }

        // 在判断成功之前检查中断
        if (m_interrupted)
        {
            emit WorkFinished(false, "导入已取消", info);
            return;
        }

        if (success)
        {
            emit StatusUpdated("计算模型边界盒...");
            emit ProgressUpdated(80);

            // 在计算边界框前检查中断
            if (m_interrupted)
            {
                emit WorkFinished(false, "导入已取消", info);
                return;
            }

            CalculateBoundingBox(info);
            info.path = m_filePath;
            msg = "几何模型导入成功";
            emit ProgressUpdated(100);
        }
        else
        {
            msg = "几何模型导入失败";
        }
    }
    catch (const Standard_Failure& e)
    {
        if (m_interrupted)
        {
            msg = "导入已取消";
            success = false;
        }
        else 
        {
            msg = QString("导入错误: %1").arg(e.GetMessageString());
            success = false;
        }
    }
    catch (...)
    {
        if (m_interrupted) 
        {
            msg = "导入已取消";
            success = false;
        }
        else 
        {
            msg = "导入时发生未知错误";
            success = false;
        }
    }

    emit WorkFinished(success, msg, info);
}

void GeometryImportWorker::RequestInterruption()
{
    m_interrupted = true;
}

bool GeometryImportWorker::ImportSTEP(ModelGeometryInfo& info)
{
    emit StatusUpdated("解析STEP文件...");
    emit ProgressUpdated(30);

    if (m_interrupted)
    {
        return false;
    }

    STEPControl_Reader reader;
    QByteArray utf8Bytes = m_filePath.toUtf8();
    const char* cStr = utf8Bytes.constData();
    if (reader.ReadFile(cStr) != IFSelect_RetDone)
    {
        return false;
    }

    if (m_interrupted)
    {
        return false;
    }

    emit ProgressUpdated(50);
    emit StatusUpdated("转换STEP模型...");

    if (m_interrupted)
    {
        return false;
    }

    reader.TransferRoots();

    if (m_interrupted)
    {
        return false;
    }

    m_shape = reader.OneShape();

    if (m_shape.IsNull())
    {
        return false;
    }

    if (m_interrupted)
    {
        return false;
    }

    info.shape = m_shape;

    emit ProgressUpdated(70);
    return true;
}

bool GeometryImportWorker::ImportSTL(ModelGeometryInfo& info)
{
    emit StatusUpdated("解析STL文件...");
    emit ProgressUpdated(30);

    StlAPI_Reader reader;
    if (!reader.Read(m_shape, m_filePath.toStdString().c_str()))
        return false;

    if (m_interrupted) return false;
    info.shape = m_shape;

    emit ProgressUpdated(70);
    return true;
}

bool GeometryImportWorker::ImportIGES(ModelGeometryInfo& info)
{
    emit StatusUpdated("解析IGES文件...");
    emit ProgressUpdated(30);

    IGESControl_Reader reader;
    if (reader.ReadFile(m_filePath.toStdString().c_str()) != IFSelect_RetDone)
        return false;

    if (m_interrupted) return false;
    emit ProgressUpdated(50);
    emit StatusUpdated("转换IGES模型...");

    reader.TransferRoots();
    m_shape = reader.OneShape();

    if (m_shape.IsNull()) return false;
    info.shape = m_shape;

    emit ProgressUpdated(70);
    return true;
}

void GeometryImportWorker::CalculateBoundingBox(ModelGeometryInfo& info)
{
    if (info.shape.IsNull()) return;

    Bnd_Box bbox;
    BRepBndLib::Add(info.shape, bbox);
    bbox.SetGap(0.0);

    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    info.theXmin = xmin;
    info.theYmin = ymin;
    info.theZmin = zmin;
    info.theXmax = xmax;
    info.theYmax = ymax;
    info.theZmax = zmax;

    info.length = xmax - xmin;
    info.width = ymax - ymin;
    info.height = zmax - zmin;
}