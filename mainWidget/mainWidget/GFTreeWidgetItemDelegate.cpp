#pragma execution_character_set("utf-8")
#include "GFTreeWidgetItemDelegate.h"
#include <QPainter>
#include <QApplication>
#include <QRect>

GFTreeWidgetItemDelegate::GFTreeWidgetItemDelegate(QObject* parent)
	:QStyledItemDelegate(parent)
{
	m_checkStatePixmap[Qt::Checked] = QPixmap(":/src/ck_checked.svg");
	m_checkStatePixmap[Qt::Unchecked] = QPixmap(":/src/ck_unchecked.svg");
	m_checkStatePixmap[Qt::PartiallyChecked] = QPixmap(":/src/ck_partiallychecked.svg");
}

GFTreeWidgetItemDelegate::~GFTreeWidgetItemDelegate()
{
}

void GFTreeWidgetItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionViewItem opti = option;
	initStyleOption(&opti, index);

	bool isSelected = opti.state & QStyle::State_Selected;
	bool isHovered = opti.state & QStyle::State_MouseOver;


	QString text = index.data(Qt::DisplayRole).toString();
	if (index.column() == 0&&(text== "工况设置"|| text =="1.跌落试验"|| text == "2.快速烤燃试验" || text == "3.慢速烤燃试验" || 
		text == "4.枪击试验"|| text == "5.射流冲击试验"|| text == "6.破片撞击试验" || text == "7.爆炸冲击波试验" || text == "8.殉爆试验"))
	{ // 只对第一列进行自定义绘制
		painter->save();

		// 计算复选框的位置和大小
		int checkboxSize = 20;
		int margin = 4;
		QRect checkboxRect =
			QRect(option.rect.left() + margin, option.rect.center().y() - checkboxSize / 2, checkboxSize, checkboxSize);
		// 绘制复选框
		QStyle* style = qApp->style();
		QStyleOptionButton optBtn;
		optBtn.rect = checkboxRect;

		Qt::CheckState state = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
		auto iter = m_checkStatePixmap.find(state);
		if (iter != m_checkStatePixmap.end())
		{
			// 根据状态选择图片
			const QPixmap& pixmap = iter.value();
			// 计算图片绘制区域（居中）
			int x = checkboxRect.x() + (checkboxRect.width() - pixmap.width()) / 2;
			int y = checkboxRect.y() + (checkboxRect.height() - pixmap.height()) / 2;
			painter->drawPixmap(x, y, pixmap);
		}
		else
		{
			if (state == Qt::Checked)
			{
				optBtn.state = (QStyle::State_On | QStyle::State_Enabled);
			}
			else if (state = Qt::PartiallyChecked)
			{
				optBtn.state = QStyle::State_NoChange | QStyle::State_Enabled;
			}
			else
			{
				optBtn.state = QStyle::State_Off | QStyle::State_Enabled;
				style->drawControl(QStyle::CE_CheckBox, &optBtn, painter);
			}			
		}
		int textX = checkboxRect.right() + margin;
		int textY = option.rect.bottom() - painter->fontMetrics().height() + 12;
		painter->setPen(Qt::black);
		painter->drawText(QPoint(textX, textY), text);

		painter->restore();
	}
	else
	{
		QStyledItemDelegate::paint(painter, option, index);
	}
}

bool GFTreeWidgetItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
	if (index.column() != 0)
	{
		return QStyledItemDelegate::editorEvent(event, model, option, index);
	}
	else
	{
		QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
		if (!mouseEvent)
		{
			return false;
		}
		QString text = index.data(Qt::DisplayRole).toString();
		if (text == "工况设置" || text == "1.跌落试验" || text == "2.快速烤燃试验" || text == "3.慢速烤燃试验" ||
			text == "4.枪击试验" || text == "5.射流冲击试验" || text == "6.破片撞击试验" || text == "7.爆炸冲击波试验" || text == "8.殉爆试验")
		{
			int checkboxSize = 20;
			int margin = 4;;
			QRect checkboxRect =
				QRect(option.rect.left() + margin, option.rect.center().y() - checkboxSize / 2, checkboxSize, checkboxSize);
			if (mouseEvent && mouseEvent->type() == QEvent::MouseButtonRelease && mouseEvent->button() == Qt::LeftButton)
			{
				if (checkboxRect.contains(mouseEvent->pos()))
				{
					Qt::CheckState currentState =
						static_cast<Qt::CheckState>(model->data(index, Qt::CheckStateRole).toInt());
					Qt::CheckState newState = (currentState == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
					model->setData(index, newState, Qt::CheckStateRole);
					emit sigCheckboxToggled(index, newState == Qt::Checked);
					return true;
				}
			}
		}
	}
	return QStyledItemDelegate::editorEvent(event, model, option, index);
}
