#ifndef DFONTUNINSTALLDIALOG_H
#define DFONTUNINSTALLDIALOG_H


#include "dfontbasedialog.h"
#include "dfmdbmanager.h"
#include "dfinstallerrordialog.h"
#include "dfontinfomanager.h"
#include <QStringList>
#include <QTimer>


#include <DDialog>
#include <DFrame>
#include <DLabel>
#include <DProgressBar>
#include <DMessageManager>
#include <QResizeEvent>

#include <DApplication>
#include <DApplicationHelper>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

class DFontMgrMainWindow;

class DFontuninstalldialog : public DFontBaseDialog
{
public:
    DFontuninstalldialog(DFontMgrMainWindow *win, QWidget *parent = nullptr);
    ~DFontuninstalldialog() override;

    void setValue(const QString &fontName, int index, int totalCount);

    void setMainwindow(DFontMgrMainWindow *win);

private:
    void initUi();


    DLabel *m_progressStepLabel;
    DLabel *m_currentFontLabel;
    DProgressBar *m_progressBar;
    QWidget *m_mainFrame;
    DFontMgrMainWindow *m_mainWindow;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
};

#endif // DFONTUNINSTALLDIALOG_H
