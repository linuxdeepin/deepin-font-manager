#ifndef DFONTPREVIEWITEMDELEGATE_H
#define DFONTPREVIEWITEMDELEGATE_H

#include "dfontpreviewitemdef.h"

#include <QFontDatabase>

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE

class DFontPreviewItemDelegate : public DStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DFontPreviewItemDelegate(QAbstractItemView *parent = nullptr);
    void setNoFont(bool noFont);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    QRect adjustPreviewRect(const QRect bgRect) const;
    QFont adjustPreviewFont(const QString &fontFamilyName, const QString &fontStyleName, const int &fontSize) const;
    QPoint adjustPreviewFontBaseLinePoint(const QRect &fontPreviewRect, const QFontMetrics &previewFontMetrics) const;

    void paintForegroundCheckBox(QPainter *painter, const QStyleOptionViewItem &option, const DFontPreviewItemData &itemData) const;
    void paintForegroundFontName(QPainter *painter, const QStyleOptionViewItem &option, const DFontPreviewItemData &itemData) const;
    void paintForegroundCollectIcon(QPainter *painter, const QStyleOptionViewItem &option, const DFontPreviewItemData &itemData) const;
    void paintForegroundPreviewFont(QPainter *painter, const QStyleOptionViewItem &option, const DFontPreviewItemData &data, int fontPixelSize, QString &fontPreviewText) const;
    void paintForegroundPreviewContent(QPainter *painter, const QString &content, const QRect &fontPreviewRect, const QFont &previewFont) const;
    void paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QAbstractItemView *m_parentView;
    bool m_hasFont;
    QFontDatabase m_fdb;
};

#endif  // DFONTPREVIEWITEMDELEGATE_H
