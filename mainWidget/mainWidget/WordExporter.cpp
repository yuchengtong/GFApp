#include "WordExporter.h"
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QApplication>
#include <QThread>
#include <QDebug>
#include <QPixmap>
#include <QScreen>

WordExporter::WordExporter(QObject* parent) : QObject(parent)
{
	wordApp = nullptr;
	activeDocument = nullptr;
}

WordExporter::~WordExporter()
{
	cleanup();
}

bool WordExporter::initializeWord()
{
	try {
		// 创建Word应用程序对象
		wordApp = new QAxObject("Word.Application", this);
		if (wordApp->isNull()) {
			qDebug() << "无法创建Word应用程序对象";
			return false;
		}

		// 设置Word不可见（后台操作）
		wordApp->setProperty("Visible", false);
		return true;
	}
	catch (...) {
		qDebug() << "初始化Word应用程序时发生异常";
		return false;
	}
}

void WordExporter::cleanup()
{
	if (activeDocument) {
		activeDocument->dynamicCall("Close(Boolean)", false);
		activeDocument->deleteLater();
		activeDocument = nullptr;
	}
	if (wordApp) {
		wordApp->dynamicCall("Quit()");
		wordApp->deleteLater();
		wordApp = nullptr;
	}
}

bool WordExporter::exportToWord(const QString& templatePath,
	const QString& outputPath,
	const QMap<QString, QVariant>& textData,
	const QMap<QString, QString>& imageWidgets)
{
	if (!initializeWord()) {
		return false;
	}

	try {
		// 检查模板文件是否存在
		if (!QFile::exists(templatePath)) {
			qDebug() << "模板文件不存在:" << templatePath;
			return false;
		}

		// 获取文档集合
		QAxObject* documents = wordApp->querySubObject("Documents");
		if (documents->isNull()) {
			qDebug() << "无法获取Documents对象";
			return false;
		}

		// 打开模板文档
		QVariant varTemplate(templatePath);
		activeDocument = documents->querySubObject("Open(QVariant)", varTemplate);
		documents->deleteLater();

		if (activeDocument->isNull()) {
			qDebug() << "无法打开模板文档";
			return false;
		}

		// 替换文本标记
		if (!replaceTextMarkers(textData)) {
			qDebug() << "替换文本标记失败";
			return false;
		}

		// 插入图片
		if (!insertImagesAtMarkers(imageWidgets)) {
			qDebug() << "插入图片失败";
			// 继续执行，不直接返回失败
		}

		// 保存文档
		QVariant varOutputPath(outputPath);
		activeDocument->dynamicCall("SaveAs(QVariant)", varOutputPath);

		cleanup();
		return true;
	}
	catch (...) {
		qDebug() << "导出Word文档时发生异常";
		cleanup();
		return false;
	}
}

bool WordExporter::replaceTextMarkers(const QMap<QString, QVariant>& data)
{
	if (!activeDocument) return false;

	try {
		// 获取文档内容
		QAxObject* content = activeDocument->querySubObject("Content");
		if (content->isNull()) return false;

		// 遍历数据并替换所有标记
		QMap<QString, QVariant>::const_iterator it;
		for (it = data.constBegin(); it != data.constEnd(); ++it) {
			QString marker = "{{" + it.key() + "}}";
			QString value = it.value().toString();

			if (!findAndReplace(marker, value)) {
				qDebug() << "替换标记失败:" << marker;
			}
		}

		content->deleteLater();
		return true;
	}
	catch (...) {
		qDebug() << "替换文本标记时发生异常";
		return false;
	}
}

bool WordExporter::findAndReplace(const QString& findText, const QString& replaceText)
{
	if (!activeDocument) return false;

	try {
		// 获取查找对象
		QAxObject* findObj = activeDocument->querySubObject("Range()")->querySubObject("Find");
		if (findObj->isNull()) return false;

		// 设置查找参数
		QVariantList params;
		params << findText;    // FindText
		params << false;       // MatchCase
		params << false;       // MatchWholeWord
		params << false;       // MatchWildcards
		params << false;       // MatchSoundsLike
		params << false;       // MatchAllWordForms
		params << true;        // Forward
		params << 1;           // Wrap (1 = wdFindContinue)
		params << false;       // Format
		params << replaceText; // ReplaceWith
		params << 2;           // Replace (2 = wdReplaceAll)
		params << false;       // MatchKashida
		params << false;       // MatchDiacritics
		params << false;       // MatchAlefHamza
		params << false;       // MatchControl

		// 执行查找替换
		QVariant result = findObj->dynamicCall("Execute(const QVariant&, const QVariant&, "
			"const QVariant&, const QVariant&, const QVariant&, "
			"const QVariant&, const QVariant&, const QVariant&, "
			"const QVariant&, const QVariant&, const QVariant&, "
			"const QVariant&, const QVariant&, const QVariant&)",
			params);

		findObj->deleteLater();
		return result.toBool();
	}
	catch (...) {
		qDebug() << "查找替换时发生异常";
		return false;
	}
}

bool WordExporter::insertImagesAtMarkers(const QMap<QString, QString>& imagePaths)
{
	if (!activeDocument || imagePaths.isEmpty()) return true;

	QMap<QString, QString>::const_iterator it;
	for (it = imagePaths.constBegin(); it != imagePaths.constEnd(); ++it) {
		if (!insertImageAtMarker(it.key(), it.value())) {
			qDebug() << "插入图片失败，标记:" << it.key();
		}
	}

	return true;
}

bool WordExporter::insertImageAtMarker(const QString& marker, QString imagePath)
{
	if (imagePath.isEmpty()) return false;

	try {
		// 获取文档内容范围
		QAxObject* content = activeDocument->querySubObject("Content");
		if (!content || content->isNull()) {
			qWarning() << "无法获取文档内容";
			return false;
		}

		// 获取查找对象
		QAxObject* find = content->querySubObject("Find");
		if (!find || find->isNull()) {
			qWarning() << "无法获取Find对象";
			delete content;
			return false;
		}

		// 查找图片占位符
		QString imagePlaceholder = QString("{{%1}}").arg(marker);
		find->setProperty("Text", imagePlaceholder);

		// 执行查找
		bool found = find->dynamicCall("Execute()").toBool();

		if (found) {
			// 获取找到的范围
			QAxObject* range = find->querySubObject("Parent");
			if (range && !range->isNull()) {
				// 替换占位符为图片
				range->dynamicCall("Delete()"); // 先删除占位符文本

				// 获取InlineShapes集合并添加图片
				QAxObject* inlineShapes = range->querySubObject("InlineShapes");
				if (inlineShapes && !inlineShapes->isNull()) {
					inlineShapes->dynamicCall("AddPicture(const QString&)", imagePath);
					delete inlineShapes;
				}

				delete range;
			}
		}

		// 释放资源
		delete find;
		delete content;

		return found;
	}
	catch (...) {
		qDebug() << "插入图片时发生异常，标记:" << marker;
		return false;
	}
}

QString WordExporter::saveTemporaryScreenshot(QWidget* widget)
{
	if (!widget) return "";

	// 确保widget已经显示并更新
	widget->show();
	widget->update();
	QApplication::processEvents();
	QThread::msleep(100);

	// 截图
	QPixmap pixmap = widget->grab();
	if (pixmap.isNull()) {
		qDebug() << "截图失败";
		return "";
	}

	// 生成临时文件路径
	QString tempDir = QDir::tempPath();
	QString tempFile = tempDir + "/qt_screenshot_" +
		QString::number(QDateTime::currentSecsSinceEpoch()) + ".png";

	// 保存图片
	if (!pixmap.save(tempFile, "PNG")) {
		qDebug() << "保存截图文件失败";
		return "";
	}

	return tempFile;
}

bool WordExporter::captureWidgetToFile(QWidget* widget, const QString& imagePath)
{
	static int cnt = 0;
	QScreen* screen = QGuiApplication::primaryScreen();

	QRect rect;
	rect.setX(widget->mapToGlobal(widget->pos()).x());
	rect.setY(widget->mapToGlobal(widget->pos()).y());
	rect.setWidth(widget->width());
	rect.setHeight(widget->height());

	QThread::msleep(1000);
	return screen->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height()).save(imagePath);
}