#ifndef DSPLITLISTWIDGET_H
#define DSPLITLISTWIDGET_H

#include <DListWidget>
#include <QStyledItemDelegate>

DWIDGET_USE_NAMESPACE

class DNoFocusDelegate : public QStyledItemDelegate
{
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
};

class DSplitListWidget : public DListWidget
{
    Q_OBJECT
public:
    explicit DSplitListWidget(QWidget *parent = nullptr);
    ~DSplitListWidget();

    QWidget *initSplitWidget(int widgetWidth, int widgetHeight);
    void insertSplitItem(int row);
};

#endif  // DSPLITLISTWIDGET_H
