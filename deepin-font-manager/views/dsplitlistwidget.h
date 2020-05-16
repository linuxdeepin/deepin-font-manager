#ifndef DSPLITLISTWIDGET_H
#define DSPLITLISTWIDGET_H

#include <DListView>
#include <QModelIndex>
#include <QMetaType>
#include <DToolTip>
#include <DStyledItemDelegate>
#include <signalmanager.h>
DWIDGET_USE_NAMESPACE

class DNoFocusDelegate : public DStyledItemDelegate
{
public:
    DNoFocusDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    QString adjustLength(QString &titleName, QFont &font) const;
protected:
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)override;

private:
    QAbstractItemView *m_parentView;
    void hideTooltipImmediately();

};

class DSplitListWidget : public DListView
{
    Q_OBJECT
public:
    enum FontGroup {
        AllFont,        //所有字体
        SysFont,        //系统字体
        UserFont,       //用户字体
        CollectFont,    //收藏
        ActiveFont,     //已激活
        ChineseFont,    //中文
        EqualWidthFont  //等宽
    };

    explicit DSplitListWidget(QWidget *parent = nullptr);
    ~DSplitListWidget() override;

    void initListData();

    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

    SignalManager *m_signalManager = SignalManager::instance();
    QStringList m_titleStringList;
    QMap<QString, int> m_titleStringIndexMap;
    QStandardItemModel *m_categoryItemModell;


signals:
    void onListWidgetItemClicked(int index);
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event)override;
};

Q_DECLARE_METATYPE(DSplitListWidget::FontGroup)
#endif  // DSPLITLISTWIDGET_H
