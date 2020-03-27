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

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    QRect adjustPreviewRect(const QString &fontName, const bool &isEnable, const QRect bgRect) const;
    QFont adjustPreviewFont(const QString &fontFamilyName, const QString &fontStyleName, const int &fontSize) const;

    void paintForegroundCheckBox(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintForegroundFontName(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintForegroundCollectIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintForegroundPreviewFont(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QAbstractItemView *m_parentView;

};

#endif  // DFONTPREVIEWITEMDELEGATE_H
