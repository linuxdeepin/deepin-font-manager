#ifndef DSPLITLISTWIDGET_H
#define DSPLITLISTWIDGET_H

#include <DListWidget>

#include <QMetaType>
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
    ~DSplitListWidget();

    void initListData();
    void initConnections();

    QWidget *initSplitWidget(int widgetWidth, int widgetHeight);
    void insertSplitItem(int row);

    QStringList m_titleStringList;

signals:
    void onListWidgetItemClicked(int index);

protected slots:

    void onListWidgetItemClicked(QListWidgetItem *item);
};

Q_DECLARE_METATYPE(DSplitListWidget::FontGroup)
#endif  // DSPLITLISTWIDGET_H
