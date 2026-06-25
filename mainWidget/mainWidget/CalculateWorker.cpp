#pragma execution_character_set("utf-8")
#include "CalculateWorker.h"

#include <QThread>
#include <cmath>


QVector<QString> expandToEightAdjacent(const QVector<QString>& srcList)
{
    QVector<QString> result;
    int srcCount = srcList.size();
    if (srcCount >= 8)
    {
        return srcList.mid(0, 8);
    }

    // 基础每份保底份数、剩余份数分配
    int baseRepeat = 8 / srcCount;
    int remain = 8 % srcCount;

    for (int i = 0; i < srcCount; ++i)
    {
        int repeat = baseRepeat;
        // 前remain个元素多复制一次，凑满8总数
        if (i < remain)
            repeat += 1;

        // 同一个名称连续紧挨填充
        for (int j = 0; j < repeat; ++j)
        {
            result.append(srcList[i]);
        }
    }
    return result;
}


void CalculateWorker::DoWork()
{
    bool success = false;
    QString msg;

    try
    {
        QVector<QString> fullTaskNames = expandToEightAdjacent(m_processedNameList);
        int need = 8 - fullTaskNames.size();
        for (int i = 0; i < need; ++i)
            fullTaskNames.append(m_processedNameList[i % m_processedNameList.size()]);

        emit StatusUpdated("开始计算...");
        emit ProgressUpdated(5);

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }
        emit StatusUpdated(fullTaskNames.at(0));
        emit ProgressUpdated(20);

        

        int currentProgress = 20;
        int startProgress = 20;
        int endProgress = 90;
        int totalStepCnt = fullTaskNames.size();
        int stepDelta = (endProgress - startProgress) / totalStepCnt;


        for (int subStep = 0; subStep < totalStepCnt; subStep++)
        {
            for (int i = 0; i < 1000; ++i)
            {
                if (m_interrupted)
                {
                    emit WorkFinished(false, "计算已取消");
                    return;
                }
                QThread::msleep(m_processedNameList.size());
            }

            if (m_interrupted)
            {
                emit WorkFinished(false, "计算已取消");
                return;
            }
            currentProgress += stepDelta;
            emit StatusUpdated(fullTaskNames.at(subStep));
            emit ProgressUpdated(currentProgress);
        }

        emit StatusUpdated(fullTaskNames.at(totalStepCnt -1));
        emit ProgressUpdated(90);

        //emit StatusUpdated("跌落试验计算");
        //emit ProgressUpdated(20);

        //for (int i = 0; i < 1000; ++i)
        //{
        //    if (m_interrupted)
        //    {
        //        emit WorkFinished(false, "计算已取消");
        //        return;
        //    }
        //    QThread::msleep(10);
        //}

        //if (m_interrupted)
        //{
        //    emit WorkFinished(false, "计算已取消");
        //    return;
        //}

        //emit StatusUpdated("快速烤燃试验计算");
        //emit ProgressUpdated(30);

        //for (int i = 0; i < 1000; ++i)
        //{
        //    if (m_interrupted)
        //    {
        //        emit WorkFinished(false, "计算已取消");
        //        return;
        //    }
        //    QThread::msleep(10);
        //}

        //if (m_interrupted)
        //{
        //    emit WorkFinished(false, "计算已取消");
        //    return;
        //}
        //
        //emit StatusUpdated("慢速烤燃试验计算");
        //emit ProgressUpdated(40);

        //for (int i = 0; i < 1000; ++i)
        //{
        //    if (m_interrupted)
        //    {
        //        emit WorkFinished(false, "计算已取消");
        //        return;
        //    }
        //    QThread::msleep(10);
        //}

        //if (m_interrupted)
        //{
        //    emit WorkFinished(false, "计算已取消");
        //    return;
        //}

        //emit StatusUpdated("枪击试验计算");
        //emit ProgressUpdated(50);

        //for (int i = 0; i < 1000; ++i)
        //{
        //    if (m_interrupted)
        //    {
        //        emit WorkFinished(false, "计算已取消");
        //        return;
        //    }
        //    QThread::msleep(10);
        //}

        //if (m_interrupted)
        //{
        //    emit WorkFinished(false, "计算已取消");
        //    return;
        //}

        //emit StatusUpdated("射流冲击试验计算");
        //emit ProgressUpdated(60);

        ////for (int i = 0; i < 1000; ++i)
        ////{
        ////    if (m_interrupted)
        ////    {
        ////        emit WorkFinished(false, "计算已取消");
        ////        return;
        ////    }
        ////    QThread::msleep(10);
        ////}

        ////if (m_interrupted)
        ////{
        ////    emit WorkFinished(false, "计算已取消");
        ////    return;
        ////}

        //emit StatusUpdated("破片撞击试验计算");
        //emit ProgressUpdated(70);

        //for (int i = 0; i < 1000; ++i)
        //{
        //    if (m_interrupted)
        //    {
        //        emit WorkFinished(false, "计算已取消");
        //        return;
        //    }
        //    QThread::msleep(10);
        //}

        //if (m_interrupted)
        //{
        //    emit WorkFinished(false, "计算已取消");
        //    return;
        //}

        //emit StatusUpdated("爆炸冲击波试验计算");
        //emit ProgressUpdated(80);

        ////for (int i = 0; i < 1000; ++i)
        ////{
        ////    if (m_interrupted)
        ////    {
        ////        emit WorkFinished(false, "计算已取消");
        ////        return;
        ////    }
        ////    QThread::msleep(10);
        ////}

        ////if (m_interrupted)
        ////{
        ////    emit WorkFinished(false, "计算已取消");
        ////    return;
        ////}

        //emit StatusUpdated("殉爆试验计算");
        //emit ProgressUpdated(90);

        ////for (int i = 0; i < 1000; ++i)
        ////{
        ////    if (m_interrupted)
        ////    {
        ////        emit WorkFinished(false, "计算已取消");
        ////        return;
        ////    }
        ////    QThread::msleep(10);
        ////}

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


