#pragma execution_character_set("utf-8")
#include "GFTreeWidget.h"

GFTreeWidget::GFTreeWidget(QWidget* parent)
	:QTreeWidget(parent)
{
	this->setMouseTracking(true);

	m_TreeWidgetItemDelegate = new GFTreeWidgetItemDelegate(this);
	this->setItemDelegateForColumn(0, m_TreeWidgetItemDelegate);

	connect(m_TreeWidgetItemDelegate, &GFTreeWidgetItemDelegate::sigCheckboxToggled, this,
		[this](const QModelIndex& index, bool checked) {
			QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
			int r = 0;
			QModelIndex cIndex;
			QString itemText0 = index.data(Qt::DisplayRole).toString();
			while (cIndex = index.child(r++, 0), cIndex.isValid())
			{
				QString childItemText = cIndex.data(Qt::DisplayRole).toString();
				if (childItemText != "应力分析" && childItemText != "温度分析" && childItemText != "超压分析")
				{
					model->setData(cIndex, checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);				
				}
				updateCheckBoxFunc(index.parent());
			}
		});
}

GFTreeWidget::~GFTreeWidget()
{
}

void GFTreeWidget::updateCheckBoxFunc(QModelIndex index)
{
	if (!index.isValid())
	{
		return;
	}
	QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
	QModelIndex cIndex;
	Qt::CheckState state = Qt::Unchecked;
	int r = 0;
	int count = 0, totalValue = 0;
	QString itemText0 = index.data(Qt::DisplayRole).toString();

	if (itemText0 != "工程文件")
	{
		while (cIndex = index.child(r++, 0), cIndex.isValid())
		{
			QString itemText1 = cIndex.data(Qt::DisplayRole).toString();
			auto cCheckState = cIndex.data(Qt::CheckStateRole).value<Qt::CheckState>();
			count++;
			totalValue += cCheckState;
		}
		if (count > 0)
		{
			if (totalValue == Qt::Checked * count)
			{
				state = Qt::Checked;
			}
			else if (totalValue != Qt::Unchecked)
			{
				state = Qt::PartiallyChecked;
			}
			model->setData(index, state, Qt::CheckStateRole);
		}
		else
		{
			state = index.data(Qt::CheckStateRole).value<Qt::CheckState>();
		}
	}
}
