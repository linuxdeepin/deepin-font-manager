#ifndef DFINSTALLERRORLISTVIEW_H
#define DFINSTALLERRORLISTVIEW_H

#include "dfinstallerroritemwidget.h"

#include <DListView>

DWIDGET_USE_NAMESPACE

class DFInstallErrorListDelegate : public DStyledItemDelegate
{
public:
    DFInstallErrorListDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    QAbstractItemView *m_parentView;
};

class DFInstallErrorListView : public DListView
{
    Q_OBJECT

public:
    explicit DFInstallErrorListView(QList<DFInstallErrorItemModel> installErrorFontModelList,
                                    QWidget *parent = nullptr);
    ~DFInstallErrorListView() override;

    void mousePressEvent(QMouseEvent *event) override;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;
    void initErrorListData();
    void initDelegate();

    QStandardItemModel* getErrorListSourceModel();

private:

    DFInstallErrorListDelegate *m_errorListItemDelegate;
    QStandardItemModel *m_errorListSourceModel;
    QList<DFInstallErrorItemModel> m_installErrorFontModelList;

    bool m_bLeftMouse;

signals:
    void onClickErrorListItem(QModelIndex index);

private slots:

};

#endif // DFINSTALLERRORLISTVIEW_H
