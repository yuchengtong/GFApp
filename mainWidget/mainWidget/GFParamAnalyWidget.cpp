#include "GFParamAnalyWidget.h"
#include "GFTreeParamAnalyWidget.h"

#include <AIS_Shape.hxx>
#include <STEPControl_Reader.hxx>
#include <Prs3d_LineAspect.hxx>
#include "OccView.h"
#include <QSplitter>
#include <QHBoxLayout>
#include <QLabel>
#include "GFLogWidget.h"

GFParamAnalyWidget::GFParamAnalyWidget(QWidget*parent)
	:QWidget(parent)
{
	auto treeParamAnalyWidget = new GFTreeParamAnalyWidget(this);
	m_OccView = new OccView(this);

	m_LogWidget = new GFLogWidget();
	m_LogWidget->setFixedHeight(160);



	QVBoxLayout*vLayout = new QVBoxLayout();
	vLayout->addWidget(m_OccView);
	vLayout->setContentsMargins(0, 0, 0, 0);
	//vLayout->addWidget(m_LogWidget);

	QHBoxLayout*hLayout = new QHBoxLayout();
	hLayout->addWidget(treeParamAnalyWidget);
	hLayout->setSpacing(0);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->addLayout(vLayout);
	this->setLayout(hLayout);


	//QSplitter*splitter = new QSplitter(Qt::Horizontal, this);
	//splitter->addWidget(treeModelWidget);
	//splitter->addWidget(m_OccView);

	//QHBoxLayout*hLayout = new QHBoxLayout();
	//hLayout->addWidget(splitter);
	//this->setLayout(hLayout);


}

GFParamAnalyWidget::~GFParamAnalyWidget()
{
}
