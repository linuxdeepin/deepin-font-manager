#ifndef DFONTPREVIEWITEMDELEGATE_H
#define DFONTPREVIEWITEMDELEGATE_H

#include "dfontpreviewitemdef.h"

#include <QFontDatabase>
#include <QStyledItemDelegate>

#include <dstyleoption.h>

//DWIDGET_USE_NAMESPACE

class DFontPreviewListView;
class DFontPreviewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DFontPreviewItemDelegate(QAbstractItemView *parent = nullptr);
    static void setNoFont(bool noFont);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    enum {
        FontPreviewRole = Dtk::UserRole + 1,
        FontSizeRole = Dtk::UserRole + 2,
    };
protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    QRect adjustPreviewRect(const QRect bgRect) const;
    QPoint adjustPreviewFontBaseLinePoint(const QRect &fontPreviewRect, const QFontMetrics &previewFontMetrics) const;

    void paintForegroundCheckBox(QPainter *painter, const QStyleOptionViewItem &option, const FontData &itemData) const;
    void paintForegroundFontName(QPainter *painter, const QStyleOptionViewItem &option, const FontData &itemData) const;
    void paintForegroundCollectIcon(QPainter *painter, const QStyleOptionViewItem &option, const FontData &itemData) const;
    void paintForegroundPreviewFont(QPainter *painter, const QStyleOptionViewItem &option, const DFontPreviewItemData &data, int fontPixelSize, QString &fontPreviewText) const;
    void paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintTabFocusBackground(QPainter *painter, const QStyleOptionViewItem &option, const QRect &bgRect)const;

    //获取需要绘制区域的路径
    void setPaintPath(const QRect &bgRect, QPainterPath &path, const int xDifference, const int yDifference, const int radius)const;

    DFontPreviewListView *m_parentView;
    DFontPreviewItemData m_fontData;
};

#endif  // DFONTPREVIEWITEMDELEGATE_H
