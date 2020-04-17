#ifndef DFDELETEDIALOG_H
#define DFDELETEDIALOG_H

#include "dfontbasedialog.h"
#include "signalmanager.h"


#include <DLabel>
#include <DPushButton>
#include <DWarningButton>

class DFontMgrMainWindow;


DWIDGET_USE_NAMESPACE

class DFDeleteDialog : public DFontBaseDialog
{
    Q_OBJECT
public:
    explicit DFDeleteDialog(DFontMgrMainWindow *win, int deleteCnt = 0, int systemCnt = 0, QWidget *parent = nullptr);

    void setMainwindow(DFontMgrMainWindow *win);

    static constexpr int DEFAULT_WINDOW_W = 380;
    static constexpr int DEFAULT_WINDOW_H = 160;

public slots:
    void onFontChanged(const QFont &font);

protected:
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void requestDelete();

private slots:
    void setTheme();

private:
    void initUI();
    void initConnections();

    void initMessageTitle();
    void initMessageDetail();
    QLayout *initBottomButtons();

    DLabel *m_fontLogo;
    DLabel *messageTitle;
    DLabel *messageDetail;
    DScrollArea *messageDetailScrollArea;
    DPushButton *m_cancelBtn;
    DWarningButton *m_confirmBtn;
    int m_deleteCnt;
    int m_systemCnt;
    int m_old_width {0};
    int m_old_height {0};
    int m_w_wd {0};
    int m_w_ht {0};
    int m_count {0};
    bool m_deleting;
};

#endif  // DFDELETEDIALOG_H
