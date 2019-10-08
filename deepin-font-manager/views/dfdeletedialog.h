#ifndef DFDELETEDIALOG_H
#define DFDELETEDIALOG_H

#include <DDialog>
#include <DLabel>
#include <DPushButton>
#include <DWarningButton>

DWIDGET_USE_NAMESPACE

class DFDeleteDialog : public DDialog
{
    Q_OBJECT
public:
    explicit DFDeleteDialog(QWidget* parent = nullptr);

    static constexpr int DEFAULT_WINDOW_W = 380;
    static constexpr int DEFAULT_WINDOW_H = 160;

protected:
    void initUI();
    void initConnections();
signals:

public slots:

private:
    DLabel* m_fontLogo;
    DLabel* m_messageA;
    DLabel* m_messageB;
    DPushButton* m_cancelBtn;
    DWarningButton* m_confirmBtn;
};

#endif  // DFDELETEDIALOG_H
