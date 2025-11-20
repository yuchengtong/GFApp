#ifndef CALCULATE_WORKER_H
#define CALCULATE_WORKER_H

#include <QObject>
#include <QString>
#include "ModelDataManager.h"

class CalculateWorker : public QObject
{
    Q_OBJECT

public:
    explicit CalculateWorker(QObject* parent = nullptr)
        : QObject(parent),m_interrupted(false) 
    {
    }

public slots:
    void DoWork();

    void RequestInterruption();

signals:
    void ProgressUpdated(int progress);

    void StatusUpdated(const QString& status);

    void WorkFinished(bool success, const QString& msg);

private:

    volatile bool m_interrupted;

};

#endif // TRIANGULATION_WORKER_H