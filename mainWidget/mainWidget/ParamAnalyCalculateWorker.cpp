#pragma execution_character_set("utf-8")
#include "ParamAnalyCalculateWorker.h"

#include <QThread>

void ParamAnalyCalculateWorker::DoWork()
{
    bool success = true;
    QString msg;

    try
    {
        emit StatusUpdated("开始计算...");
        emit ProgressUpdated(10);

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }

        emit StatusUpdated("初始化数据...");
        emit ProgressUpdated(20);

        QThread::msleep(3000);
        emit StatusUpdated("解析数据...");
        emit ProgressUpdated(40);

        QThread::msleep(3000);
        emit StatusUpdated("计算数据...");
        emit ProgressUpdated(60);

        if (success && !m_interrupted)
        {
            QThread::msleep(3000);
            emit StatusUpdated("计算数据...");
            emit ProgressUpdated(80);

            QThread::msleep(3000);
            msg = "计算成功";
            emit ProgressUpdated(100);
        }
        else if (m_interrupted)
        {
            msg = "计算已取消";
            success = false;
        }
        else
        {
            msg = "计算失败";
        }
    }
    catch (const Standard_Failure& e)
    {
        msg = QString("计算错误: %1").arg(e.GetMessageString());
        success = false;
    }
    catch (...)
    {
        msg = "计算时发生未知错误";
        success = false;
    }

    emit WorkFinished(success, msg);
}

