#ifndef DFONTINFODIALOG_H
#define DFONTINFODIALOG_H
#include "dfontbasedialog.h"
#include "dfontinfoscrollarea.h"
#include "signalmanager.h"

#include <QResizeEvent>
#include <QPointF>
#include <DFrame>
#include <DLabel>
#include <QTextEdit>
#include <DWidget>
class QHBoxLayout;
DWIDGET_USE_NAMESPACE

struct DFontPreviewItemData;
class FontIconText;
class DFontInfoDialog : public DFontBaseDialog
{
    Q_OBJECT
public:
    explicit DFontInfoDialog(DFontPreviewItemData *fontInfo, QWidget *parent = nullptr);

    static constexpr int DEFAULT_WINDOW_W = 300;
    static constexpr int DEFAULT_WINDOW_H = 640;

protected:
    void initUI();
    void initConnections();
    void resizeEvent(QResizeEvent *event) override;
    void insertContents();
signals:

public slots:

private:
    int m_height;
    QWidget *m_mainFrame {nullptr};
    QString m_FileName;
    QPoint m_faCenter;
    SignalManager *m_signalManager = SignalManager::instance();

    FontIconText *m_fontLogo {nullptr};
    DLabel *m_fontFileName {nullptr};

    QFrame *m_basicInfoFrame {nullptr};
    QString AutoFeed(QString text);
    QVBoxLayout *m_baseicInfoLayout;

    DFontPreviewItemData *m_fontInfo;

    dfontinfoscrollarea *fontinfoArea;

    DScrollArea *scrollArea;
};

#endif  // DFONTINFODIALOG_H
