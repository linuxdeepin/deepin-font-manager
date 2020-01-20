#ifndef DFONTPREVIEWITEMDELEGATE_H
#define DFONTPREVIEWITEMDELEGATE_H

#include "dfontpreviewitemdef.h"

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE

class DFontPreviewItemDelegate : public DStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DFontPreviewItemDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

    QRect m_checkboxRect;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    QAbstractItemView *m_parentView;
};

#endif  // DFONTPREVIEWITEMDELEGATE_H
