#pragma execution_character_set("utf-8")
#include "CalculateWorker.h"

#include <QThread>
#include <cmath>

void CalculateWorker::DoWork()
{
    bool success = false;
    QString msg;

    try
    {
        emit StatusUpdated("开始计算...");
        emit ProgressUpdated(5);

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }


        emit StatusUpdated("跌落试验计算");
        emit ProgressUpdated(20);

        for (int i = 0; i < 1000; ++i)
        {
            if (m_interrupted)
            {
                emit WorkFinished(false, "计算已取消");
                return;
            }
            QThread::msleep(10);
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }

        emit StatusUpdated("快速烤燃试验计算");
        emit ProgressUpdated(30);

        for (int i = 0; i < 1000; ++i)
        {
            if (m_interrupted)
            {
                emit WorkFinished(false, "计算已取消");
                return;
            }
            QThread::msleep(10);
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }
        
        emit StatusUpdated("慢速烤燃试验计算");
        emit ProgressUpdated(40);

        for (int i = 0; i < 1000; ++i)
        {
            if (m_interrupted)
            {
                emit WorkFinished(false, "计算已取消");
                return;
            }
            QThread::msleep(10);
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }

        emit StatusUpdated("枪击试验计算");
        emit ProgressUpdated(50);

        for (int i = 0; i < 1000; ++i)
        {
            if (m_interrupted)
            {
                emit WorkFinished(false, "计算已取消");
                return;
            }
            QThread::msleep(10);
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }

        emit StatusUpdated("射流冲击试验计算");
        emit ProgressUpdated(60);

        for (int i = 0; i < 1000; ++i)
        {
            if (m_interrupted)
            {
                emit WorkFinished(false, "计算已取消");
                return;
            }
            QThread::msleep(10);
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }

        emit StatusUpdated("破片撞击试验计算");
        emit ProgressUpdated(70);

        for (int i = 0; i < 1000; ++i)
        {
            if (m_interrupted)
            {
                emit WorkFinished(false, "计算已取消");
                return;
            }
            QThread::msleep(10);
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }

        emit StatusUpdated("爆炸冲击波试验计算");
        emit ProgressUpdated(80);

        for (int i = 0; i < 1000; ++i)
        {
            if (m_interrupted)
            {
                emit WorkFinished(false, "计算已取消");
                return;
            }
            QThread::msleep(10);
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }

        emit StatusUpdated("殉爆试验计算");
        emit ProgressUpdated(90);

        for (int i = 0; i < 1000; ++i)
        {
            if (m_interrupted)
            {
                emit WorkFinished(false, "计算已取消");
                return;
            }
            QThread::msleep(10);
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }

        success = true;
        msg = "计算完成";
        emit ProgressUpdated(100);
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

    // 最终检查：如果在中途被取消，覆盖之前的成功状态
    if (m_interrupted)
    {
        emit WorkFinished(false, "计算已取消");
    }
    else
    {
        emit WorkFinished(success, msg);
    }
}


void CalculateWorker::RequestInterruption() 
{
    m_interrupted = true; 
}


