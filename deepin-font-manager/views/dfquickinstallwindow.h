#ifndef DFQUICKINSTALL_H
#define DFQUICKINSTALL_H
#include <QFrame>

#include <DComboBox>
#include <DDialog>
#include <DLabel>
#include <DMainWindow>
#include <DTextEdit>

DWIDGET_USE_NAMESPACE

class DFontInfoManager;
class DFontManager;

class DFQuickInstallWindow : public DDialog
{
    Q_OBJECT
public:
    explicit DFQuickInstallWindow(QStringList files, QWidget* parent = nullptr);

    static constexpr int DEFAULT_WINDOW_W = 480;
    static constexpr int DEFAULT_WINDOW_H = 380;

protected:
    void initUI();
    void initConnections();

    void resizeEvent(QResizeEvent* event) override;
signals:
    void fileSelected(QStringList fileList);

public slots:
    void onFileSelected(QStringList fileList);
    void onInstallBtnClicked();

private:
    DLabel* m_logoLabel {nullptr};
    DLabel* m_titleLabel {nullptr};
    QFrame* m_titleFrame {nullptr};
    QFrame* m_mainFrame {nullptr};

    DComboBox* m_fontType {nullptr};
    DTextEdit* m_fontPreviewTxt {nullptr};

    DLabel* m_stateLabel {nullptr};
    DPushButton* m_actionBtn {nullptr};

    DFontInfoManager* m_fontInfoManager;
    DFontManager* m_fontManager;

    QStringList m_installFiles;
};

#endif  // DFQUICKINSTALL_H
