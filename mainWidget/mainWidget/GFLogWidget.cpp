#pragma execution_character_set("utf-8")
#include "GFLogWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QDateTime>

GFLogWidget::GFLogWidget(QWidget*parent) 
	:QWidget(parent)
{
	auto titlelabel = new QLabel("日志区");
	m_TextEdit = new QPlainTextEdit();
	m_TextEdit->setReadOnly(true);
	auto vLayout = new QVBoxLayout();
	vLayout->addWidget(titlelabel);
	vLayout->addWidget(m_TextEdit);
	vLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(vLayout);
}

GFLogWidget::~GFLogWidget()
{
}

void GFLogWidget::PrintInfo(QString info, bool valid)
{
	QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	QString prefix = valid ? "[信息]>" : "[错误]>";
	QColor color = valid ? Qt::black : Qt::red;

	QTextCharFormat oldFormat = m_TextEdit->currentCharFormat();
	QTextCharFormat newFormat = oldFormat;
	newFormat.setForeground(color);

	m_TextEdit->setCurrentCharFormat(newFormat);
	m_TextEdit->appendPlainText(timeStr + prefix + info);
	m_TextEdit->setCurrentCharFormat(oldFormat);

	update();
}



