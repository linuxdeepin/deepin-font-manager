#ifndef DFINSTALLERRORDIALOG_H
#define DFINSTALLERRORDIALOG_H

#include "dfinstallerroritemmodel.h"
#include "dfontinfomanager.h"
#include "dfinstallerrorlistview.h"

#include <DDialog>
#include <DFrame>
#include <DLabel>
#include <DPushButton>
//#include <DSuggestButton>
#include <QResizeEvent>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

class DFMSuggestButton;
typedef DFMSuggestButton DSuggestButton;

class DFMSuggestButton : public QPushButton, public DObject
{
    Q_OBJECT

public:
    explicit DFMSuggestButton(QWidget *parent = nullptr);
    explicit DFMSuggestButton(const QString &text, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
};

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

    QWidget *m_mainFrame;
    QVBoxLayout *m_mainLayout;

    DFInstallErrorListView *m_installErrorListView;

    DPushButton *m_quitInstallBtn;
    DSuggestButton *m_continueInstallBtn;

private:
    void resetContinueInstallBtnStatus();

    QWidget *titleFrame;
    DLabel *logoLabel;
    DLabel *titleLabel;
    QWidget *contentFrame;

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
