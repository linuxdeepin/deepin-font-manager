#ifndef DFINSTALLNORMALWINDOW_H
#define DFINSTALLNORMALWINDOW_H

#include "dfontbasedialog.h"
#include "dfinstallerrordialog.h"
#include "dfontinfomanager.h"

#include <QStringList>
#include <QTimer>

#include <DDialog>
#include <DFrame>
#include <DLabel>
#include <DProgressBar>

DWIDGET_USE_NAMESPACE

class DFontManager;
class DFontInfoManager;
class DFInstallErrorDialog;

class DFInstallNormalWindow : public DFontBaseDialog
{
    Q_OBJECT
public:
    DFInstallNormalWindow(const QStringList &files = QStringList(), QWidget *parent = nullptr);
    ~DFInstallNormalWindow() override;

    void setSkipException(bool skip);
    void breakInstalltion();

protected:
    static constexpr int VERIFY_DELYAY_TIME = 1000;

    void initUI();
    void initConnections();
    void initVerifyTimer();

    void verifyFontFiles();
    bool ifNeedShowExceptionWindow() const;

    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

protected slots:
    void batchInstall();
    void onProgressChanged(const QString &filePath, const double &percent);

    void onCancelInstall();
    void onContinueInstall(QStringList continueInstallFontFileList);

    void showInstallErrDlg();
signals:
    void batchReinstall(QStringList reinstallFiles);
    void finishFontInstall(QStringList fileList);

private:
    enum InstallState { Install, reinstall, damaged };

    QStringList m_installFiles;
    QStringList m_installedFiles;
    QStringList m_newInstallFiles;
    QStringList m_damagedFiles;
    QStringList m_outfileList;

    // Skip popup exception dialog if true
    bool m_isNeedSkipException {false};

    InstallState m_installState {Install};

    QList<DFInstallErrorItemModel *> m_installErrorFontModelList;
    DFontInfoManager *m_fontInfoManager;
    DFontManager *m_fontManager;

    DLabel *m_logoLabel {nullptr};
    DLabel *m_titleLabel {nullptr};
    QWidget *m_titleFrame {nullptr};
    QWidget *m_mainFrame {nullptr};

    DLabel *m_progressStepLabel {nullptr};
    DLabel *m_currentFontLabel {nullptr};
    DProgressBar *m_progressBar {nullptr};

    DFInstallErrorDialog* m_pexceptionDlg {nullptr};

    QScopedPointer<QTimer> m_verifyTimer {nullptr};
};

#endif  // DFINSTALLNORMALWINDOW_H
