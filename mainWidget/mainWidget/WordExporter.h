#include <QObject>
#include <QAxObject>
#include <QWidget>
#include <QMap>
#include <QString>
#include <QVariant>

class WordExporter : public QObject
{
	Q_OBJECT

public:
	explicit WordExporter(QObject* parent = nullptr);
	~WordExporter();

	// 导出Word文档
	bool exportToWord(const QString& templatePath,
		const QString& outputPath,
		const QMap<QString, QVariant>& textData,
		const QMap<QString, QString>& imageWidgets);

	// 单独截图功能
	bool captureWidgetToFile(QWidget* widget, const QString& imagePath);

private:
	QAxObject* wordApp;
	QAxObject* activeDocument;

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

	// 保存临时截图文件
	QString saveTemporaryScreenshot(QWidget* widget);
};