#pragma once
#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <QStandardItemModel>
#include <QStyleOptionViewItem>
#include <QMouseEvent>

class GFTreeWidgetItemDelegate :public QStyledItemDelegate
{
	Q_OBJECT
public:
	GFTreeWidgetItemDelegate(QObject* parent = nullptr);
	~GFTreeWidgetItemDelegate();

private:
	virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	
	virtual bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
signals:
	void sigCheckboxToggled(const QModelIndex& index, bool checked);

private:
	QMap<Qt::CheckState, QPixmap> m_checkStatePixmap;
};

