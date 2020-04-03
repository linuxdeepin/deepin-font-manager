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

class DFontuninstalldialog : public DFontBaseDialog
{
public:
    DFontuninstalldialog(QWidget *parent = nullptr);
    void setValue(const QString &fontName, int index, int totalCount);
private:
    void initUi();


    DLabel *m_progressStepLabel;
    DLabel *m_currentFontLabel;
    DProgressBar *m_progressBar;
    QWidget *m_mainFrame;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // DFONTUNINSTALLDIALOG_H
