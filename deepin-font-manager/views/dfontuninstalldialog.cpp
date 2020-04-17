#include "dfontuninstalldialog.h"
#include "signalmanager.h"
#include "dfontmgrmainwindow.h"
#include <QApplication>

#include <QDebug>

DFontuninstalldialog::DFontuninstalldialog(DFontMgrMainWindow *win, QWidget *parent)
    : DFontBaseDialog(parent)

{
    initUi();

    repaint();

    setMainwindow(win);
}

DFontuninstalldialog::~DFontuninstalldialog()
{
    if (qApp->activeWindow() == this) {
        qApp->setActiveWindow(m_mainWindow);
        hide();
    }
}

void DFontuninstalldialog::setValue(const QString &fontName, int index, int totalCount)
{
    m_currentFontLabel->setText(fontName);
    m_progressBar->setMaximum(totalCount);
    m_progressBar->setValue(index);
}

void DFontuninstalldialog::setMainwindow(DFontMgrMainWindow *win)
{
    connect(SignalManager::instance(), &SignalManager::updateUninstallDialog, this, [ = ](QString & fontName, int index, int totalCount) {
        setValue(fontName, index, totalCount);
    }, Qt::UniqueConnection);

    connect(SignalManager::instance(), &SignalManager::closeUninstallDialog, this, [ = ] {
        qDebug() << "DFontuninstalldialog closeUninstallDialog" << isVisible();
        setValue(" ", 0, 0);
        if (isVisible())
        {
            accept();
            close();
        }
    }, Qt::UniqueConnection);

    connect(this, &DFontuninstalldialog::closed, this, [ = ]() {
        qDebug() << "DFontuninstalldialog closed ";
        if (m_mainWindow != nullptr)
            m_mainWindow->setDeleteFinish();
    });

    if (m_mainWindow != win && win != nullptr) {
        m_mainWindow = win;
        //start to delete
        m_mainWindow->startToDelete();
    }
}

void DFontuninstalldialog::initUi()
{
    setFixedSize(QSize(380, 136));
//    setTitle(DApplication::translate("Dfuninstalldialog", "Uninstall Font"));

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 0, 0);

    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(10, 0, 0, 0);

    m_progressStepLabel = new DLabel(this);
    QFont pslFont = m_progressStepLabel->font();
    pslFont.setWeight(QFont::Medium);
    //pslFont.setPixelSize(14);
    m_progressStepLabel->setFont(pslFont);
    DFontSizeManager::instance()->bind(m_progressStepLabel, DFontSizeManager::T6);
    m_progressStepLabel->setFixedHeight(m_progressStepLabel->fontMetrics().height());
    m_progressStepLabel->setText(DApplication::translate("Dfuninstalldialog", "Deleting fonts, please wait..."));

    m_currentFontLabel = new DLabel(this);
//    QFont cflFont;
//    cflFont.setPixelSize(12);
//    m_currentFontLabel->setFont(cflFont);
    DFontSizeManager::instance()->bind(m_currentFontLabel, DFontSizeManager::T8);
    m_currentFontLabel->setFixedHeight(m_currentFontLabel->fontMetrics().height());
    m_currentFontLabel->setText("");
    DPalette pa = DApplicationHelper::instance()->palette(m_currentFontLabel);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextTips));
    m_currentFontLabel->setPalette(pa);

    m_progressBar = new DProgressBar();
    m_progressBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_progressBar->setTextVisible(false);
    m_progressBar->setAlignment(Qt::AlignCenter);
    m_progressBar->setFixedSize(380 - 42, 6);

    contentLayout->addSpacing(10);
    contentLayout->addWidget(m_progressStepLabel);
    contentLayout->addSpacing(7);
    contentLayout->addWidget(m_currentFontLabel);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(m_progressBar);
    contentLayout->addSpacing(20);

    mainLayout->addLayout(contentLayout);

    m_mainFrame = new QWidget(this);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mainFrame->setLayout(mainLayout);

    addContent(m_mainFrame);

}
//n卸载时忽略esc快捷键
void DFontuninstalldialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        return ;
    }
}


