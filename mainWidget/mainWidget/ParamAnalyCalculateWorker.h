#ifndef PARAM_ANALY_CALCULATE_WORKER_H
#define PARAM_ANALY_CALCULATE_WORKER_H

#include <QObject>
#include <QString>
#include "ModelDataManager.h"

class ParamAnalyCalculateWorker : public QObject
{
    Q_OBJECT

public:
    explicit ParamAnalyCalculateWorker(QObject* parent = nullptr)
        : QObject(parent), m_interrupted(false) {}

public slots:
    void DoWork();

    void RequestInterruption() { m_interrupted = true; }

signals:
    void ProgressUpdated(int progress);
    void StatusUpdated(const QString& status);
    void WorkFinished(bool success, const QString& msg);

private:
    QString m_filePath;
    volatile bool m_interrupted;
    
};

#endif // PARAM_ANALY_CALCULATE_WORKER_H

