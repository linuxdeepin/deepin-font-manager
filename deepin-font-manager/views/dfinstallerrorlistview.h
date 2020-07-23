#ifndef DFINSTALLERRORLISTVIEW_H
#define DFINSTALLERRORLISTVIEW_H

#include "dfinstallerroritemmodel.h"
#include "dfontinfodialog.h"

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

    QString lengthAutoFeed(QPainter *painter, QString sourceStr, int m_StatusWidth)const;


public:
    DFInstallErrorListDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

class QStandardItemModel;
class DFInstallErrorListView : public DListView
{
    Q_OBJECT

public:
    explicit DFInstallErrorListView(const QList<DFInstallErrorItemModel> &installErrorFontModelList,
                                    QWidget *parent = nullptr);
    ~DFInstallErrorListView() override;

    void mousePressEvent(QMouseEvent *event) override;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;
    void initErrorListData();
    void initDelegate();
    void addErrorListData(const QList<DFInstallErrorItemModel> &installErrorFontModelList);
    void checkScrollToIndex(QStringList &addHalfInstalledFiles, QStringList &oldHalfInstalledFiles, QStringList &errorFileList);
    void scrollToIndex(QString &filePath);
    void setSelectStatus(QStringList &HalfInstalledFiles);
    void updateErrorFontModelList(int index, DFInstallErrorItemModel m_currentItemModel);
    void initModel(bool newOne = true);
    QStandardItemModel *getErrorListSourceModel();

private:

    DFontInfoManager *m_fontInfoManager;
    DFInstallErrorListDelegate *m_errorListItemDelegate;
    QStandardItemModel *m_errorListSourceModel;
    QList<DFInstallErrorItemModel> m_installErrorFontModelList;
    QList<QString> m_errorFontlist;
    int beforeSelectRow = 0;

    bool m_bLeftMouse;

signals:
    void onClickErrorListItem(QModelIndex index);

private slots:
protected:
    void keyPressEvent(QKeyEvent *event);//SP3--安装验证页面，listview上下键自动跳过异常字体
    bool selectNextIndex(int nextIndex);//SP3--安装验证页面，listview上下键自动跳过异常字体
};

#endif // DFINSTALLERRORLISTVIEW_H
