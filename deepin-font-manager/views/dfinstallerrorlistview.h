#ifndef DFINSTALLERRORLISTVIEW_H
#define DFINSTALLERRORLISTVIEW_H

#include "dfinstallerroritemmodel.h"

#include <DListView>

DWIDGET_USE_NAMESPACE

class DFInstallErrorListDelegate : public DStyledItemDelegate
{
private:
    void drawCheckBox(QPainter *painter, DFInstallErrorItemModel itemModel, QRect bgRect)const;
    void drawCheckBoxIcon(QPainter *painter, QRect bgRect)const;
    void drawFontName(QPainter *painter, const QStyleOptionViewItem &option,
                      DFInstallErrorItemModel itemModel, QRect bgRect, bool bSelectable = true)const;
    void drawFontStyle(QPainter *painter, const QStyleOptionViewItem &option,
                       DFInstallErrorItemModel itemModel, QRect bgRect, bool bSelectable = true)const;
    void drawSelectStatus(QPainter *painter, const QStyleOptionViewItem &option, QRect bgRect)const;

    QString AutoFeed(QPainter *painter, QString sourceStr, int m_StatusWidth)const;


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

    QStandardItemModel *getErrorListSourceModel();

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
