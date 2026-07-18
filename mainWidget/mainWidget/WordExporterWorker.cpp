#pragma execution_character_set("utf-8")
#include "WordExporterWorker.h"
#include "windows.h"

#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QEventLoop>

void WordExporterWorker::DoWork()
{

    bool success = false;
    QString msg;

    try
    {
        emit StatusUpdated("开始导出报告文件...");
        emit ProgressUpdated(10);

        if (m_interrupted)
        {
            emit WorkFinished(false, "导出已取消");
            return;
        }

        // 获取报告数据
        emit StatusUpdated("初始化报告文件...");
        emit ProgressUpdated(30);


        if (!initializeWord()) {
            msg = "初始化文件失败";
            success = false;
            return;
        }

        // 检查模板文件是否存在
        /*if (!QFile::exists(m_templateFilePath)) {
            msg = "模板文件不存在" + m_templateFilePath;
            success = false;
            return;
        }*/

        // 从资源释放模板到本地临时文件
        QString realLoadTemplate = extractTemplateFromResource(m_templateFilePath);
        if (realLoadTemplate.isEmpty())
        {
            msg = "内置模板释放失败，无法初始化报告";
            success = false;
            cleanup();
            emit WorkFinished(false, msg);
            return;
        }
        
        if (m_interrupted)
        {
            emit WorkFinished(false, "导出已取消");
            return;
        }
        emit StatusUpdated("创建Word应用程序对象");
        emit ProgressUpdated(40);
        // 获取文档集合
        QAxObject* documents = wordApp->querySubObject("Documents");
        if (!documents) {
            msg = "无法获取Documents对象";
            success = false;
            return;
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "导出已取消");
            return;
        }
        emit StatusUpdated("创建Word应用程序对象");
        emit ProgressUpdated(45);
        // 打开模板文档
        /*QVariant varTemplate(m_templateFilePath);
        activeDocument = documents->querySubObject("Open(QVariant)", varTemplate);*/

        activeDocument = documents->querySubObject("Add(const QString&)", realLoadTemplate);
        // 激活文档窗口
        if (activeDocument) {
            QAxObject* docWindow = activeDocument->querySubObject("ActiveWindow");
            if (docWindow) {
                docWindow->dynamicCall("Activate()");
                delete docWindow;
            }
            // 解除文档保护（如果模板本身有保护）
            activeDocument->dynamicCall("Unprotect()");
            // 处理时序延迟
            QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
        }
        //documents->deleteLater();
        if (!activeDocument) {
            msg = "无法打开模板文档";
            success = false;
            return;
        }


        // 解析报告
        emit StatusUpdated("解析数据...");
        emit ProgressUpdated(50);

        if (m_interrupted)
        {
            emit WorkFinished(false, "导出已取消");
            return;
        }

        // 替换文本标记
        if (!replaceTextMarkers(m_textData)) {
            qDebug() << "替换文本标记失败";

            msg = "解析数据失败";
            success = false;
            return;
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "导出已取消");
            return;
        }

        // 插入表格数据（新增）
        if (!insertTables(m_tableData)) {
            qDebug() << "插入表格数据失败";
            msg = "解析数据失败";
            success = false;
            return;
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "导出已取消");
            return;
        }

        // 插入图片
        if (!insertImagesAtMarkers(m_imageWidgets)) {
            qDebug() << "插入图片失败";
            // 继续执行，不直接返回失败
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "导出已取消");
            return;
        }

        // 生成报告
        emit StatusUpdated("开始导出报告...");
        emit ProgressUpdated(80);

        if (m_interrupted)
        {
            emit WorkFinished(false, "导出已取消");
            return;
        }

        // 保存文档
        QVariant varOutputPath(m_outputFilePath);
        activeDocument->dynamicCall("SaveAs(QVariant)", varOutputPath);

        cleanup();

        if (m_interrupted)
        {
            emit WorkFinished(false, "导出已取消");
            return;
        }
		success = true;
		msg = "导出成功";
		emit ProgressUpdated(100);
    }
    catch (...)
    {
        cleanup();
        msg = "生成报告时发生未知错误";
        success = false;
    }

    emit WorkFinished(success, msg);
}


bool WordExporterWorker::initializeWord()
{
	try {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

		// 创建Word应用程序对象
		wordApp = new QAxObject("Word.Application", this);
		if (!wordApp) {
            msg = "无法创建Word应用程序对象";
			return false;
		}
        QString version = wordApp->property("Version").toString();
        emit StatusUpdated("创建Word应用程序对象,版本" + version);
        emit ProgressUpdated(35);

		// 设置Word不可见（后台操作）
		wordApp->setProperty("Visible", false);
        // 显示所有弹窗
        wordApp->setProperty("DisplayAlerts", true);

        // 禁用受保护的视图
        QAxObject* options = wordApp->querySubObject("Options");
        if (options) {
            options->setProperty("ProtectedViewEnableInternetFiles", false);
            options->setProperty("ProtectedViewEnableLocalMachineFiles", false);
            delete options;
        }
		return true;
	}
	catch (...) {
		qDebug() << "初始化Word应用程序时发生异常";
		return false;
	}
}

void WordExporterWorker::cleanup()
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
    // 删除从资源解压的临时模板文件
    if (!m_tempTemplatePath.isEmpty())
    {
        QFile tempFile(m_tempTemplatePath);
        if (tempFile.exists())
        {
            tempFile.remove();
        }
        m_tempTemplatePath.clear();
    }
}

bool WordExporterWorker::replaceTextMarkers(const QMap<QString, QVariant>& data)
{
    if (!activeDocument) return false;

    try {
        QAxObject* content = activeDocument->querySubObject("Content");
        if (content->isNull()) return false;

        QMap<QString, QVariant>::const_iterator it;
        for (it = data.constBegin(); it != data.constEnd(); ++it)
        {
            // 线程中断检测，支持取消导出
            if (m_interrupted)
            {
                content->deleteLater();
                return false;
            }

            QString marker = "{{" + it.key() + "}}";
            QString value = it.value().toString();

         

            bool ret = findAndReplace(marker, value);
            if (!ret)
            {
                // 区分两种失败：找不到标记 / 超长匹配失败
                if (marker.length() > 250 || value.length() > 250)
                {
                    qWarning() << "超长文本替换失败！标记：" << marker
                        << "文本长度：" << value.length();
                }
                else
                {
                    qDebug() << "文档中未找到标记:" << marker;
                }
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

bool WordExporterWorker::findAndReplace(const QString& findText, const QString& replaceText)
{
    if (!activeDocument || findText.isEmpty())
        return false;

    try
    {
        QAxObject* docRange = activeDocument->querySubObject("Content");
        if (!docRange || docRange->isNull())
            return false;

        QAxObject* find = docRange->querySubObject("Find");
        find->setProperty("Text", findText);
        find->setProperty("MatchCase", false);
        find->setProperty("MatchWholeWord", false);
        find->setProperty("MatchWildcards", false);
        find->setProperty("Wrap", 1); // wdFindContinue

        bool hasMatch = false;
        // 循环查找所有匹配项，规避单次Execute超长限制
        while (find->dynamicCall("Execute()").toBool())
        {
            hasMatch = true;
            QAxObject* matchRange = find->querySubObject("Parent");
            if (matchRange && !matchRange->isNull())
            {
                // 清空原有占位符，写入超长替换文本
                matchRange->setProperty("Text", replaceText);
                matchRange->deleteLater();
            }
        }

        find->deleteLater();
        docRange->deleteLater();
        return hasMatch;
    }
    catch (...)
    {
        qDebug() << "查找替换异常，查找内容：" << findText.left(100) << "...";
        return false;
    }
}

bool WordExporterWorker::insertImagesAtMarkers(const QMap<QString, QString>& imagePaths)
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

bool WordExporterWorker::insertImageAtMarker(const QString& marker, QString imagePath)
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


bool WordExporterWorker::insertTables(const QMap<QString, QVector<QVector<QVariant>>>& tableData)
{
    if (!activeDocument || tableData.isEmpty()) return true;

    QMap<QString, QVector<QVector<QVariant>>>::const_iterator it;
    for (it = tableData.constBegin(); it != tableData.constEnd(); ++it) {
        if (!insertTableAtMarker(it.key(), it.value())) {
            qDebug() << "插入表格失败: " << it.key();
        }
    }
    return true;
}

bool WordExporterWorker::insertTableAtMarker(const QString& marker, const QVector<QVector<QVariant>>& tableData)
{
    if (tableData.isEmpty()) return false;

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

        // 表格占位标记格式: {{Table:marker}}
        QString tablePlaceholder = QString("{{Table:%1}}").arg(marker);
        find->setProperty("Text", tablePlaceholder);

        // 执行查找
        bool found = find->dynamicCall("Execute()").toBool();

        if (found) {
            // 获取找到的范围
            QAxObject* range = find->querySubObject("Parent");
            if (range && !range->isNull()) {
                // 删除占位标记
                range->dynamicCall("Delete()");

                // 获取行数和列数
                int rows = tableData.size();
                int cols = tableData.first().size();

                // 获取当前范围的段落
                QAxObject* paragraphs = range->querySubObject("Paragraphs");
                QAxObject* paragraph = paragraphs->querySubObject("Add()");
                QAxObject* paraRange = paragraph->querySubObject("Range");

                // 在段落位置插入表格
                QAxObject* tables = activeDocument->querySubObject("Tables");
                QAxObject* table = tables->querySubObject("Add(const QVariant&, int, int)",
                    paraRange->asVariant(), rows, cols);

                // ========== 设置表格所有框线（核心部分）==========
                QAxObject* borders = table->querySubObject("Borders");
                if (borders && !borders->isNull()) {
                    // 全局边框默认样式（可统一修改）
                    int lineStyle = 1;    // 单实线（wdLineStyleSingle）
                    int lineWidth = 2;    // 0.5磅（wdLineWidth050pt）
                    int color = 0;        // 黑色（wdColorBlack）

                    // 外框：上、下、左、右
                    setBorderStyle(borders, 1, lineStyle, lineWidth, color); // 上边框
                    setBorderStyle(borders, 4, lineStyle, lineWidth, color); // 下边框
                    setBorderStyle(borders, 2, lineStyle, lineWidth, color); // 左边框
                    setBorderStyle(borders, 3, lineStyle, lineWidth, color); // 右边框
                    // 内部框：横线（行之间）、竖线（列之间）
                    setBorderStyle(borders, 5, lineStyle, lineWidth, color); // 内部横线
                    setBorderStyle(borders, 6, lineStyle, lineWidth, color); // 内部竖线

                    borders->deleteLater();
                }

                // 填充表格数据
                for (int i = 0; i < rows; ++i) {
                    for (int j = 0; j < cols; ++j) {
                        // Word表格行索引从1开始
                        QAxObject* cell = table->querySubObject("Cell(int, int)", i + 1, j + 1);
                        if (cell && !cell->isNull()) {
                            QAxObject* cellRange = cell->querySubObject("Range");
                            cellRange->setProperty("Text", tableData[i][j].toString());
                            cell->deleteLater();
                            cellRange->deleteLater();
                        }
                    }
                }

                // 释放资源
                tables->deleteLater();
                table->deleteLater();
                paraRange->deleteLater();
                paragraph->deleteLater();
                paragraphs->deleteLater();
            }
            range->deleteLater();
        }

        // 释放资源
        delete find;
        delete content;

        return found;
    }
    catch (...) {
        qDebug() << "插入表格时发生异常: " << marker;
        return false;
    }
}

void WordExporterWorker::setBorderStyle(QAxObject* borders, int borderType, int lineStyle, int lineWidth, int color)
{
    if (!borders || borders->isNull()) return;

    // 获取指定类型的边框（如上边框、内部横线等）
    QAxObject* border = borders->querySubObject("Item(int)", borderType);
    if (border && !border->isNull()) {
        border->setProperty("LineStyle", lineStyle);  // 线条样式
        border->setProperty("LineWidth", lineWidth);  // 线条粗细
        border->setProperty("Color", color);          // 线条颜色
        border->deleteLater();
    }
}


QString WordExporterWorker::extractTemplateFromResource(QString path)
{
    // 资源内模板路径，和qrc前缀对应
    const QString resTemplatePath = ":/template/" + path;
    // 系统临时目录存放释放后的模板
    QString tempDir = QDir::tempPath() + "/word_export_embed_temp/";
    QDir dir;
    dir.mkpath(tempDir);
    QString tempFile = tempDir + path;

    QFile resFile(resTemplatePath);
    QFile outputTemp(tempFile);

    // 打开资源文件读取
    if (!resFile.open(QIODevice::ReadOnly))
    {
        qCritical() << "资源模板打开失败，资源路径：" << resTemplatePath;
        return "";
    }
    // 打开临时文件写入
    if (!outputTemp.open(QIODevice::WriteOnly))
    {
        qCritical() << "临时模板文件创建失败：" << tempFile;
        resFile.close();
        return "";
    }

    // 二进制完整写入
    QByteArray fileData = resFile.readAll();
    outputTemp.write(fileData);

    resFile.close();
    outputTemp.close();

    m_tempTemplatePath = tempFile;
    return m_tempTemplatePath;
}