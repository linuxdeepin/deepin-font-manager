#ifndef DFINSTALLERRORDIALOG_H
#define DFINSTALLERRORDIALOG_H

#include "dfinstallerroritemmodel.h"
#include "dfontinfomanager.h"
#include "dfinstallerrorlistview.h"

#include <DDialog>
#include <DFrame>
#include <DLabel>
#include <DPushButton>
#include <QResizeEvent>

DWIDGET_USE_NAMESPACE

class DFInstallErrorDialog : public DDialog
{
    Q_OBJECT

public:
    explicit DFInstallErrorDialog(QWidget *parent = nullptr,
                                  QStringList errorInstallFontFileList = QStringList());
    ~DFInstallErrorDialog();

    void initData();
    void initUI();

    void resizeEvent(QResizeEvent *event);

    void initMainFrame();
    void initTitleBar();
    void initInstallErrorFontViews();

    int getErrorFontCheckedCount();

    DFrame *m_mainFrame;
    QVBoxLayout *m_mainLayout;

    DFInstallErrorListView *m_installErrorListView;

    DPushButton *m_quitInstallBtn;
    QPushButton *m_continueInstallBtn;

private:
    void resetContinueInstallBtnStatus();

    DFrame *titleFrame;
    DLabel *logoLabel;
    DLabel *titleLabel;
    DFrame *contentFrame;

    QStringList m_errorInstallFiles;
    QList<DFInstallErrorItemModel> m_installErrorFontModelList;

signals:
    void onCancelInstall();
    void onContinueInstall(QStringList continueInstallFontFilelList);

public slots:
    void onListItemClicked(QModelIndex index);

    void onControlButtonClicked(int btnIndex);
};

#endif  // DFINSTALLERRORDIALOG_H
