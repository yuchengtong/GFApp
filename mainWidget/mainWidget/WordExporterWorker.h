#ifndef WORD_EXPORTER_WORKER_H
#define WORD_EXPORTER_WORKER_H

#include <QObject>
#include <QAxObject>
#include <QWidget>
#include <QMap>
#include <QString>
#include <QVariant> 

class WordExporterWorker : public QObject
{
    Q_OBJECT

public:
    explicit WordExporterWorker(const QString& templatePath,
        const QString& outputPath,
        const QMap<QString, QVariant>& textData,
        const QMap<QString, QString>& imageWidgets, QObject* parent = nullptr)
        : QObject(parent), m_templateFilePath(templatePath),
        m_outputFilePath(outputPath),
        m_textData(textData),
        m_imageWidgets(imageWidgets),
                m_interrupted(false) {}

public slots:
    void DoWork();

    void RequestInterruption() { m_interrupted = true; }

signals:
    void ProgressUpdated(int progress);
    void StatusUpdated(const QString& status);
    void WorkFinished(bool success, const QString& msg);

private:
    // 初始化Word应用程序
    bool initializeWord();

    // 清理资源
    void cleanup();

    // 替换文本标记
    bool replaceTextMarkers(const QMap<QString, QVariant>& data);

    // 插入图片到标记位置
    bool insertImagesAtMarkers(const QMap<QString, QString>& imagePaths);

    // 查找并替换单个标记
    bool findAndReplace(const QString& findText, const QString& replaceText);

    // 在标记位置插入图片
    bool insertImageAtMarker(const QString& marker, QString imagePath);

private:
    volatile bool m_interrupted;

    QString m_templateFilePath; // 模板文件路径
    QString m_outputFilePath; // 导出文件路径
    QMap<QString, QVariant> m_textData; // 模板文件路径
    QMap<QString, QString> m_imageWidgets; // 模板文件路径

    QAxObject* wordApp;
    QAxObject* activeDocument;

  
};

#endif // WORD_EXPORTER_WORKER_H