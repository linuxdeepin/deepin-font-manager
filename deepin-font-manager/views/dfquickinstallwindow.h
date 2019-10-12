#ifndef DFQUICKINSTALL_H
#define DFQUICKINSTALL_H

#include <DApplication>
#include <DComboBox>
#include <DFrame>
#include <DLabel>
#include <DMainWindow>
#include <DTextEdit>

DWIDGET_USE_NAMESPACE

class DFontInfoManager;
class DFontManager;
struct DFontInfo;
class DFontPreviewer;

class DFQuickInstallWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit DFQuickInstallWindow(QStringList files = QStringList(), QWidget* parent = nullptr);

    ~DFQuickInstallWindow() override;

    static constexpr int DEFAULT_WINDOW_W = 480;
    static constexpr int DEFAULT_WINDOW_H = 380;

protected:
    void initUI();
    void initConnections();
    void InitPreviewFont(DFontInfo fontInfo);

    void resizeEvent(QResizeEvent* event) override;
signals:
    void fileSelected(QStringList fileList);
    void quickInstall();

public slots:
    void onFileSelected(QStringList fileList);
    void onInstallBtnClicked();

private:
    DLabel* m_titleLabel {nullptr};
    DFrame* m_mainFrame {nullptr};

    DComboBox* m_fontType {nullptr};
    DFontPreviewer* m_fontPreviewTxt {nullptr};

    DLabel* m_stateLabel {nullptr};
    DPushButton* m_actionBtn {nullptr};

    DFontInfoManager* m_fontInfoManager;
    DFontManager* m_fontManager;

    QStringList m_installFiles;

    DPalette m_oldPaStateLbl;
};

#endif  // DFQUICKINSTALL_H
