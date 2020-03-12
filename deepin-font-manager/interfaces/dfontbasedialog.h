#ifndef DFONTBASEDIALOG_H
#define DFONTBASEDIALOG_H

#include <dabstractdialog.h>

#include <DLabel>
#include <DWindowCloseButton>

DWIDGET_USE_NAMESPACE

//class DWindowCloseButton;
//class DLabel;
class QVBoxLayout;

class DFontBaseDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit DFontBaseDialog(QWidget *parent = nullptr);

    void addContent(QWidget *content);
    void setIconPixmap(const QPixmap &iconPixmap);

    QWidget *getContent() const;

protected:
    void initUI();
    void InitConnections();
    void setLogoVisable(bool visible = true);
    void setTitle(const QString &title);
    QLayout *getContentLayout();

    //Overrides
    void closeEvent(QCloseEvent *event) override;
signals:
    void closed();
public slots:
private:
    QWidget *m_titleBar {nullptr};
    DLabel  *m_logoIcon {nullptr};
    DLabel  *m_tileText {nullptr};
    DWindowCloseButton *m_closeButton {nullptr};

    QWidget *m_content {nullptr};
    QVBoxLayout *m_contentLayout {nullptr};
};

#endif // DFONTBASEDIALOG_H
