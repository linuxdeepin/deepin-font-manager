#ifndef DFONTPREVIEWITEMDELEGATE_H
#define DFONTPREVIEWITEMDELEGATE_H

#include "dfontpreviewitemdef.h"

#include <QMetaType>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QString>
#include <QStyledItemDelegate>

class DFontPreviewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DFontPreviewItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

    QRect m_checkboxRect;
signals:
};

#endif  // DFONTPREVIEWITEMDELEGATE_H
