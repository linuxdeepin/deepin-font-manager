#ifndef DFONTINFODIALOG_H
#define DFONTINFODIALOG_H
#include "dfontbasedialog.h"

#include <QResizeEvent>

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
    void addLabelContent(const QString &title, const QString &content);

    void resizeEvent(QResizeEvent *event) override;
signals:

public slots:

private:
    QWidget *m_mainFrame {nullptr};

    FontIconText *m_fontLogo {nullptr};
    DLabel *m_fontFileName {nullptr};

    QFrame *m_basicInfoFrame {nullptr};
    QVBoxLayout *m_baseicInfoLayout;

    DFontPreviewItemData *m_fontInfo;
};

#endif  // DFONTINFODIALOG_H
