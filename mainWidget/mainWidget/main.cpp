#pragma execution_character_set("utf-8")
#include "mainWidget.h"
#include <QtWidgets/QApplication>
#include "ModelDataManager.h"
#include "LoginDialog.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	// 设置全局字体大小
	QFont font = QFont("Arial", 8);
	QApplication::setFont(font);
	mainWidget w;
	QCoreApplication::setApplicationName(QStringLiteral("固体发动机安全性分析与评估系统"));
	/*w.hide();
	LoginDialog loginDialog;
	if (loginDialog.exec() == QDialog::Accepted) {
		w.show();
		return a.exec();
	}*/
	w.show();
	return a.exec();

}
