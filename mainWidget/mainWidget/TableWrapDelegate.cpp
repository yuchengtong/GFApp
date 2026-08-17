#include "TableWrapDelegate.h"
#include <QFontMetrics>
#include <QTextDocument>
#include <QTextOption>
#include <QApplication>

TableWrapDelegate::TableWrapDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void TableWrapDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    // 绘制原生背景、选中底色、边框
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const QWidget* widget = opt.widget;
    QStyle* style = widget ? widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);

    QString text = index.data(Qt::DisplayRole).toString();
    if (text.isEmpty())
    {
        painter->restore();
        return;
    }

    QTextDocument doc;
    doc.setPlainText(text);

    doc.setDefaultFont(opt.font);
    QTextOption textOpt = doc.defaultTextOption();
    textOpt.setWrapMode(QTextOption::WrapAnywhere); // 任意字符换行
    textOpt.setAlignment(Qt::AlignLeft | Qt::AlignTop);
    doc.setDefaultTextOption(textOpt);

    int padding = 4;
    doc.setTextWidth(opt.rect.width() - padding * 2);


    painter->translate(opt.rect.left() + padding, opt.rect.top() + padding);
    doc.drawContents(painter);

    painter->restore();
}

QSize TableWrapDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QString text = index.data(Qt::DisplayRole).toString();
    if (text.isEmpty())
        return QStyledItemDelegate::sizeHint(option, index);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QTextDocument doc;
    doc.setPlainText(text);
    doc.setDefaultFont(opt.font); // 尺寸计算也用统一字体
    QTextOption textOpt = doc.defaultTextOption();
    textOpt.setWrapMode(QTextOption::WrapAnywhere);
    doc.setDefaultTextOption(textOpt);

    int padding = 4;
    doc.setTextWidth(opt.rect.width() - padding * 2);
    QSize docSize = doc.size().toSize();
    // 上下补边距
    return QSize(docSize.width(), docSize.height() + padding * 2);
}