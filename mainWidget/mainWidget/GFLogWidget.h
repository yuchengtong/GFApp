#pragma once
#include <qwidget.h>
#include <QPlainTextEdit>

class GFLogWidget :public QWidget
{
	Q_OBJECT
public:
	GFLogWidget(QWidget*parent = nullptr);
	~GFLogWidget();

	QPlainTextEdit*GetTextEdit() { return m_TextEdit; }

private:
	QPlainTextEdit*m_TextEdit = nullptr;
};

