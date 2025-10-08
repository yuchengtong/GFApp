#include "ParamAnalyWidget.h"

#include "OccView.h"
#include <QHBoxLayout>



ParamAnalyWidget::ParamAnalyWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.widget->update();
	// tableWidge随窗口大小变化
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	/*m_OccView = new OccView(ui.widget);
	QHBoxLayout*hLayout = new QHBoxLayout();
	hLayout->addWidget(m_OccView);
	ui.widget->setLayout(hLayout);*/

	// 树默认展开
	ui.m_SafeParamTtreeWid->expandAll();



	// 连接信号槽以处理复选框点击事件
	connect(ui.m_SafeParamTtreeWid, &QTreeWidget::itemChanged, [](QTreeWidgetItem* item, int column)    {
			if (item->childCount() > 0) {
				// 如果父节点被选中，则选中所有子节点
				if (item->checkState(0) == Qt::Checked) {
					for (int i = 0; i < item->childCount(); ++i) {
						item->child(i)->setCheckState(0, Qt::Checked);
					}
				}
				// 如果父节点被取消选中，则取消选中所有子节点
				else if (item->checkState(0) == Qt::Unchecked) {
					for (int i = 0; i < item->childCount(); ++i) {
						item->child(i)->setCheckState(0, Qt::Unchecked);
					}
				}
			}
	});
}

ParamAnalyWidget::~ParamAnalyWidget()
{}







