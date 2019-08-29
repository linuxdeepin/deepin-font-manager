#include "dfquickinstallwindow.h"
#include "dfontinfomanager.h"
#include "dfontmanager.h"

#include <QResizeEvent>
#include <QVBoxLayout>

#include <DPalette>
#include <DPushButton>
#include <DTitlebar>

DFQuickInstallWindow::DFQuickInstallWindow(QStringList files, QWidget *parent)
    : DDialog(parent)
    , m_fontInfoManager(DFontInfoManager::instance())
    , m_fontManager(DFontManager::instance())
    , m_installFiles(files)
{
    initUI();
    initConnections();

    Q_EMIT fileSelected(m_installFiles);
}

void DFQuickInstallWindow::initUI()
{
    setFixedSize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H));

    // setIcon(QIcon(":/images/deepin-font-manager.svg"), QSize(32, 32));
    // titlebar()->setTitle("思源字体");
    // titlebar()->setT  //   "思源字体");
    // ToDo:
    //    Need localize the string
    setWindowIcon(QIcon(":/images/deepin-font-manager.svg"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    mainLayout->setContentsMargins(10, 0, 10, 10);
    mainLayout->setSpacing(0);

    QHBoxLayout *titleLayout = new QHBoxLayout(this);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    m_titleFrame = new QFrame(this);
    m_titleFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleFrame->setFixedHeight(37);

    m_logoLabel = new DLabel(this);
    m_logoLabel->setObjectName("logoLabel");
    m_logoLabel->setFixedSize(QSize(32, 32));
    m_logoLabel->setFocusPolicy(Qt::NoFocus);
    m_logoLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_logoLabel->setPixmap(QPixmap(":/images/deepin-font-manager.svg"));

    m_titleLabel = new DLabel(this);
    m_titleLabel->setObjectName("tileNameLabel");
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // m_titleLabel->setText("思源字体");

    // titleLayout->addSpacing(10);
    titleLayout->addWidget(m_logoLabel, 0, Qt::AlignLeft | Qt::AlignBottom);
    titleLayout->addStretch();
    titleLayout->addWidget(m_titleLabel, 0, Qt::AlignBottom | Qt::AlignHCenter);
    titleLayout->addStretch();
    titleLayout->addSpacing(32);

    m_titleFrame->setLayout(titleLayout);

    mainLayout->addWidget(m_titleFrame);
    mainLayout->addSpacing(10);

    // Style combox
    m_fontType = new DComboBox(this);
    m_fontType->setFixedSize(QSize(114, 36));

    // Text Preview
    m_fontPreviewTxt = new DTextEdit(this);
    m_fontPreviewTxt->setFixedSize(QSize(381, 216));
    m_fontPreviewTxt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QFont previewFont;
    previewFont.setPixelSize(28);
    m_fontPreviewTxt->setFont(previewFont);
    m_fontPreviewTxt->setText(
        "汉体书写信息技术标准相容\n"
        "档案下载使用界面简单\n"
        "支援服务升级资讯专业制作\n"
        "创意空间快速无线上网\n"
        "㈠㈡㈢㈣㈤㈥㈦㈧㈨㈩\n"
        "AaBbCc ＡａＢｂＣｃ");

    // Action bar
    QHBoxLayout *actionBarLayout = new QHBoxLayout(this);
    actionBarLayout->setContentsMargins(0, 0, 0, 0);
    actionBarLayout->setSpacing(0);

    QFont actionFont;
    actionFont.setPixelSize(14);

    m_stateLabel = new DLabel(this);
    m_stateLabel->setFixedHeight(36);
    m_stateLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_stateLabel->setFont(actionFont);
    QPalette pa = m_stateLabel->palette();
    pa.setBrush(QPalette::Text, DPalette::get(m_stateLabel).textWarning());
    m_stateLabel->setPalette(pa);
    // m_stateLabel->setText("未安装");

    m_actionBtn = new DPushButton(this);
    m_actionBtn->setFixedSize(QSize(120, 36));
    m_actionBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    // m_actionBtn->setFont(actionFont);
    m_actionBtn->setText("安装字体");

    actionBarLayout->addSpacing(20);
    actionBarLayout->addWidget(m_stateLabel);
    actionBarLayout->addStretch();
    actionBarLayout->addWidget(m_actionBtn);
    actionBarLayout->addSpacing(20);

    mainLayout->addWidget(m_fontType, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_fontPreviewTxt, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(16);
    mainLayout->addLayout(actionBarLayout);

    m_mainFrame = new QFrame(this);
    m_mainFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mainFrame->setLayout(mainLayout);

#ifdef FTM_DEBUG_LAYOUT_COLOR
    m_mainFrame->setStyleSheet("background: red");
    m_titleLabel->setStyleSheet("background: silver");
    m_logoLabel->setStyleSheet("background: silver");
    m_titleFrame->setStyleSheet("background: yellow");
    m_fontPreviewTxt->setStyleSheet("background: blue");
    m_fontType->setStyleSheet("background: green");
    m_stateLabel->setStyleSheet("background: green");
#endif
}

void DFQuickInstallWindow::initConnections()
{
    connect(this, &DFQuickInstallWindow::fileSelected, this, &DFQuickInstallWindow::onFileSelected);
    connect(m_actionBtn, &DPushButton::clicked, this, &DFQuickInstallWindow::onInstallBtnClicked);
}

void DFQuickInstallWindow::resizeEvent(QResizeEvent *event)
{
    DDialog::resizeEvent(event);

    m_mainFrame->setFixedSize(event->size().width(), event->size().height());
}

void DFQuickInstallWindow::onFileSelected(QStringList fileList)
{
    if (fileList.size() > 0) {
        QString file = fileList.at(0);

        DFontInfo *pfontInfo = m_fontInfoManager->getFontInfo(file);
        if (pfontInfo->isError) {
            m_stateLabel->setText(QString("文件损坏!"));
            m_actionBtn->setDisabled(true);
            m_fontType->setVisible(false);
        } else {
            if (m_fontInfoManager->isFontInstalled(pfontInfo)) {
                DPalette pa = m_stateLabel->palette();
                QColor color = pa.color(DPalette::TextWarning);
                pa.setColor(DPalette::WindowText, color);
                m_stateLabel->setPalette(pa);

                m_stateLabel->setText(QString("已安装"));

            } else {
                m_stateLabel->setText(QString("未安装"));
            }

            m_titleLabel->setText(pfontInfo->familyName);
            if (pfontInfo->styleName.isEmpty()) {
                m_fontType->addItem("Unknow");
            } else {
                m_fontType->clear();
                m_fontType->addItem(pfontInfo->styleName);
            }

            if (!pfontInfo->familyName.isEmpty()) {
                m_titleLabel->setText(pfontInfo->familyName);
            }
        }
    }
}

void DFQuickInstallWindow::onInstallBtnClicked()
{
    accept();
}
