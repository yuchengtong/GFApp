#ifndef CALCULATE_WORKER_H
#define CALCULATE_WORKER_H

#include <QObject>
#include <QString>
#include "ModelDataManager.h"

class CalculateWorker : public QObject
{
    Q_OBJECT

public:
    explicit CalculateWorker(QVector<QString> processedNameList, QObject* parent = nullptr)
        : QObject(parent), m_processedNameList(processedNameList), m_interrupted(false)
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

    QVector<QString> m_processedNameList;

};

#endif // TRIANGULATION_WORKER_H