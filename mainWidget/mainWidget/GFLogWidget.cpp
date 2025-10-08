#pragma execution_character_set("utf-8")
#include "GFLogWidget.h"
#include <QLabel>
#include <QVBoxLayout>
GFLogWidget::GFLogWidget(QWidget*parent) 
	:QWidget(parent)
{
	auto titlelabel = new QLabel("ÈÕÖ¾Çø");
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
