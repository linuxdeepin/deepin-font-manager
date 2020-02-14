#ifndef DFDELETEDIALOG_H
#define DFDELETEDIALOG_H

#include "dfontbasedialog.h"

#include <DLabel>
#include <DPushButton>
#include <DWarningButton>

class DFontMgrMainWindow;

DWIDGET_USE_NAMESPACE

class DFDeleteDialog : public DFontBaseDialog
{
    Q_OBJECT
public:
    explicit DFDeleteDialog(DFontMgrMainWindow *win, QWidget* parent = nullptr);
    ~DFDeleteDialog();
    void setMainwindow(DFontMgrMainWindow *win);

    static constexpr int DEFAULT_WINDOW_W = 380;
    static constexpr int DEFAULT_WINDOW_H = 160;

protected:
    void initUI();
    void initConnections();

signals:
    void requestDelete();

public slots:

private:
    DLabel* m_fontLogo;
    DLabel* m_messageA;
    DLabel* m_messageB;
    DPushButton* m_cancelBtn;
    DWarningButton* m_confirmBtn;
    DFontMgrMainWindow *m_mainWindow;
    QMetaObject::Connection quitConn;
};

#endif  // DFDELETEDIALOG_H
