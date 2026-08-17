#ifndef TABLEWRAPDELEGATE_H
#define TABLEWRAPDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>

class TableWrapDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TableWrapDelegate(QObject* parent = nullptr);

    // 重写绘制：强制长文本任意字符换行
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    // 重写尺寸计算：自动算出换行后需要的行高
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif // TABLEWRAPDELEGATE_H