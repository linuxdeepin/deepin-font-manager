#ifndef DFINSTALLNORMALWINDOW_H
#define DFINSTALLNORMALWINDOW_H

#include <DDialog>
#include <DLabel>
#include <DProgressBar>

DWIDGET_USE_NAMESPACE

class DFontManager;
class DFontInfoManager;

class DFInstallNormalWindow : public DDialog
{
public:
    DFInstallNormalWindow(const QStringList &files = QStringList(), QWidget *parent = nullptr);

protected:
    static constexpr int VERIFY_DELYAY_TIME = 1000;

    void initUI();
    void initConnections();
    void initVerifyTimer();

    int verifyFontFiles();

    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
protected slots:
    void batchInstall();
    void onProgressChanged(const QString &filePath, const double &percent);

private:
    QStringList m_installFiles;
    DFontInfoManager *m_fontInfoManager;
    DFontManager *m_fontManager;

    DLabel *m_logoLabel {nullptr};
    DLabel *m_titleLabel {nullptr};
    QFrame *m_titleFrame {nullptr};
    QFrame *m_mainFrame {nullptr};

    DLabel *m_progressStepLabel {nullptr};
    DLabel *m_currentFontLabel {nullptr};
    DProgressBar *m_progressBar {nullptr};

    QScopedPointer<QTimer> m_verifyTimer {nullptr};
};

#endif  // DFINSTALLNORMALWINDOW_H
