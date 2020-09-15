#ifndef DSPLITLISTWIDGET_H
#define DSPLITLISTWIDGET_H

#include <DListView>
#include <QModelIndex>
#include <QMetaType>
#include <DToolTip>
#include <DStyledItemDelegate>
#include <signalmanager.h>
DWIDGET_USE_NAMESPACE


struct FocusStatus {
    bool m_IsMouseClicked {false};
    bool m_IsFirstFocus{true};
    bool m_IsHalfWayFocus{false};
};

class DSplitListWidget;
class DNoFocusDelegate : public DStyledItemDelegate
{
public:
    DNoFocusDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    // 绘制tab选中之后的背景
    void paintTabFocusBackground(QPainter *painter, const QStyleOptionViewItem &option,
                                 const QRect &backgroundRect, const DPalette::ColorGroup cg)const;
    //获取需要绘制区域的路径
    void setPaintPath(const QRect &bgRect, QPainterPath &path, const int xDifference, const int yDifference, const int radius)const;

    QString adjustLength(QString &titleName, QFont &font) const;
protected:
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)override;

private:
    DSplitListWidget *m_parentView;
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


    void setIsHalfWayFocus(bool IsHalfWayFocus);


    bool IsTabFocus() const;

    FocusStatus &getStatus();

    void setCurrentStatus(const FocusStatus &currentStatus);

private:
    bool m_refreshFinished = true;
    bool m_isFocusFromFontListView{false};
    //判断鼠标有没有点击
    bool m_IsMouseClicked = false;

    //判断是否通过tab获取的焦点
    bool m_IsTabFocus = false;

    //判断是否是通过键盘左键获取到的焦点，这种情况下选中效果与tab效果一致
    bool m_IsLeftFocus = false;

    //判断是否为第一次打开应用设置的焦点
    bool m_IsFirstFocus = true;

    //判断是否为删除等过程中设置的焦点
    bool m_IsHalfWayFocus = false;

    FocusStatus m_currentStatus;

signals:
    void onListWidgetItemClicked(int index);
//    void leftListviewHasFocus();
public slots:
    void setRefreshFinished(bool isInstalling);
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event)override;
    bool eventFilter(QObject *obj, QEvent *event)override;
};

Q_DECLARE_METATYPE(DSplitListWidget::FontGroup)
#endif  // DSPLITLISTWIDGET_H
