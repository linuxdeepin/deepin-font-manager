#ifndef DINSTALLERRORLISTVIEW_H
#define DINSTALLERRORLISTVIEW_H

#include <DListView>

DWIDGET_USE_NAMESPACE


class DInstallErrorListDelegate : public DStyledItemDelegate
{
public:
    DInstallErrorListDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

};

class DInstallErrorListView : public DListView
{
    Q_OBJECT

public:
    explicit DInstallErrorListView(QWidget *parent = nullptr);
    ~DInstallErrorListView() override;

    void initErrorListData();
    void initDelegate();

    void refreshFontListData(QStandardItemModel *sourceModel, bool isStartup = false);

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void setSelection(const QRect &rect,
                      QItemSelectionModel::SelectionFlags command) Q_DECL_OVERRIDE;

    void setModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;

    void setRightContextMenu(QMenu *rightMenu);

    QModelIndex currModelIndex();

private:
    void initConnections();

    bool m_bLeftMouse;
    QStandardItemModel *m_fontPreviewItemModel;

    QMenu *m_rightMenu;

    QModelIndex m_currModelIndex;

signals:
    void onClickEnableButton(QModelIndex index);
    void onClickCollectionButton(QModelIndex index);

private slots:

};

#endif // DINSTALLERRORLISTVIEW_H
