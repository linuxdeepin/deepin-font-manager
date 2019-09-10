#include "dfquickinstallwindow.h"
#include "dfontinfomanager.h"
#include "dfontmanager.h"
#include "utils.h"

#include <QFontDatabase>
#include <QResizeEvent>
#include <QVBoxLayout>

#include <DLog>
#include <DPalette>
#include <DPushButton>
#include <DTitlebar>
#include <DApplicationHelper>

DFQuickInstallWindow::DFQuickInstallWindow(QStringList files, QWidget *parent)
    : DMainWindow(parent)
    , m_fontInfoManager(DFontInfoManager::instance())
    , m_fontManager(DFontManager::instance())
    , m_installFiles(files)
{
    initUI();
    initConnections();

    // Q_EMIT fileSelected(m_installFiles);
}

DFQuickInstallWindow::~DFQuickInstallWindow()
{
    qDebug() << __FUNCTION__ << "destructor";
}

void DFQuickInstallWindow::initUI()
{
    setFixedSize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H));
    // titlebar()->setIcon(QIcon(":/images/deepin-font-manager.svg"));
    // titlebar()->setTitle("思源字体");
    // titlebar()->setT  //   "思源字体");
    // ToDo:
    //    Need localize the string
    setWindowIcon(QIcon(":/images/deepin-font-manager.svg"));

    m_logoLabel = new DLabel(this);
    m_logoLabel->setObjectName("LogoLabel");
    m_logoLabel->setFixedSize(QSize(32, 32));
    m_logoLabel->setFocusPolicy(Qt::NoFocus);
    m_logoLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_logoLabel->setPixmap(Utils::renderSVG(":/images/deepin-font-manager.svg", m_logoLabel->size()));

    m_titleLabel = new DLabel(this);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_titleLabel->setText("思源字体");

    m_titleFrame = new DFrame(this);
    m_titleFrame->setObjectName("TitleBar");
    // d->titleFrame->setStyleSheet("background: yellow");  // debug
    m_titleFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);

    titleLayout->addSpacing(7);
    titleLayout->addWidget(m_logoLabel);
    titleLayout->addStretch(2);
    titleLayout->addWidget(m_titleLabel, 0, Qt::AlignBottom);
    titleLayout->addStretch(1);

    m_titleFrame->setLayout(titleLayout);

    titlebar()->setCustomWidget(m_titleFrame, false);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(20, 0, 20, 10);
    mainLayout->setSpacing(0);

    // Style combox
    m_fontType = new DComboBox(this);
    m_fontType->setFixedSize(QSize(114, 36));

    // Text Preview
    m_fontPreviewTxt = new DTextEdit(this);
    // m_fontPreviewTxt->setFixedSize(QSize(381, 216));

    // m_fontPreviewTxt->setFixedHeight(216);
    m_fontPreviewTxt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //    QFont previewFont;
    //    previewFont.setPixelSize(28);
    //    m_fontPreviewTxt->setFont(previewFont);

    m_fontPreviewTxt->setText(
        "汉体书写信息技术标准相容\n"
        "档案下载使用界面简单\n"
        "支援服务升级资讯专业制作\n"
        "创意空间快速无线上网\n"
        "㈠㈡㈢㈣㈤㈥㈦㈧㈨㈩\n"
        "AaBbCc ＡａＢｂＣｃ");

    // Action bar
    QHBoxLayout *actionBarLayout = new QHBoxLayout();
    actionBarLayout->setContentsMargins(0, 0, 0, 0);
    actionBarLayout->setSpacing(0);

    //    QFont actionFont;
    //    actionFont.setPixelSize(14);

    m_stateLabel = new DLabel(this);
    m_stateLabel->setFixedHeight(36);
    m_stateLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // m_stateLabel->setFont(actionFont);
    DPalette pa = DApplicationHelper::instance()->palette(m_stateLabel);
    pa.setBrush(QPalette::Text,  pa.brush(DPalette::ColorType::TextWarning));
    m_stateLabel->setPalette(pa);

    m_actionBtn = new DPushButton(this);
    m_actionBtn->setFixedSize(QSize(120, 36));
    m_actionBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    // m_actionBtn->setFont(actionFont);
    m_actionBtn->setText(DApplication::translate("QuickInstallWindow", "Install Font"));

    // actionBarLayout->addSpacing(20);
    actionBarLayout->addWidget(m_stateLabel);
    actionBarLayout->addStretch();
    actionBarLayout->addWidget(m_actionBtn);
    actionBarLayout->addSpacing(20);

    mainLayout->addWidget(m_fontType, 0, Qt::AlignCenter);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_fontPreviewTxt);
    mainLayout->addSpacing(16);
    mainLayout->addLayout(actionBarLayout);

    m_mainFrame = new DFrame(this);
    m_mainFrame->setFrameShape(DFrame::Shape::NoFrame);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mainFrame->setLayout(mainLayout);

    setCentralWidget(m_mainFrame);

#ifdef FTM_DEBUG_LAYOUT_COLOR
    m_mainFrame->setStyleSheet("background: red");
    m_fontPreviewTxt->setStyleSheet("background: blue");
    m_fontType->setStyleSheet("background: green");
    m_stateLabel->setStyleSheet("background: green");
    m_titleFrame->setStyleSheet("background: green");
    m_stateLabel->setStyleSheet("background: blue");
    m_titleLabel->setStyleSheet("background: yellow");
#endif
}

void DFQuickInstallWindow::initConnections()
{
    connect(this, &DFQuickInstallWindow::fileSelected, this, &DFQuickInstallWindow::onFileSelected);
    connect(m_actionBtn, &DPushButton::clicked, this, &DFQuickInstallWindow::onInstallBtnClicked);
}

void DFQuickInstallWindow::resizeEvent(QResizeEvent *event)
{
    DMainWindow::resizeEvent(event);

    // m_mainFrame->setFixedSize(event->size().width() - 150, event->size().height());
}

void DFQuickInstallWindow::onFileSelected(QStringList fileList)
{
    if (fileList.size() > 0) {
        QString file = fileList.at(0);

        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(file);
        if (fontInfo.isError) {
            m_stateLabel->setText(DApplication::translate("QuickInstallWindow", "File Error"));
            m_actionBtn->setDisabled(true);
            m_fontType->setVisible(false);
        } else {
            if (fontInfo.isInstalled) {
                DPalette pa = DApplicationHelper::instance()->palette(m_stateLabel);
                pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextWarning));
                m_stateLabel->setPalette(pa);
                m_stateLabel->setText(DApplication::translate("QuickInstallWindow", "Installed"));

            } else {
                m_stateLabel->setText(
                    DApplication::translate("QuickInstallWindow", "Not Installed"));
            }

            m_titleLabel->setText(fontInfo.familyName);

            if (fontInfo.styleName.isEmpty()) {
                m_fontType->addItem(DApplication::translate("QuickInstallWindow", "Unknow"));
            } else {
                m_fontType->clear();
                m_fontType->addItem(fontInfo.styleName);
            }

            if (!fontInfo.familyName.isEmpty()) {
                m_titleLabel->setText(fontInfo.familyName);
            }

            InitPreviewFont(fontInfo);
        }
    }
}

void DFQuickInstallWindow::onInstallBtnClicked()
{
    // Close Quick install first
    close();
    Q_EMIT quickInstall();
}

void DFQuickInstallWindow::InitPreviewFont(DFontInfo fontInfo)
{
    qDebug() << __FUNCTION__ << "enter";

    if (!fontInfo.isError) {
        fontInfo = m_fontInfoManager->getFontInfo(fontInfo.filePath);
        if (!fontInfo.isInstalled) {
            int fontId = QFontDatabase::addApplicationFont(fontInfo.filePath);
            QStringList familys = QFontDatabase::applicationFontFamilies(fontId);

            if (familys.size() > 0) {
                fontInfo.familyName = familys.at(0);
            }
        }

        qDebug() << __FUNCTION__ << fontInfo.familyName;

        QFont preivewFont;
        preivewFont.setFamily(fontInfo.familyName);
        preivewFont.setPixelSize(28);
        QString styleName = fontInfo.styleName;

        if (styleName.contains("Italic")) {
            preivewFont.setItalic(true);
        }

        if (styleName.contains("Regular")) {
            preivewFont.setWeight(QFont::Normal);
        } else if (styleName.contains("Bold")) {
            preivewFont.setWeight(QFont::Bold);
        } else if (styleName.contains("Light")) {
            preivewFont.setWeight(QFont::Light);
        } else if (styleName.contains("Thin")) {
            preivewFont.setWeight(QFont::Thin);
        } else if (styleName.contains("ExtraLight")) {
            preivewFont.setWeight(QFont::ExtraLight);
        } else if (styleName.contains("ExtraBold")) {
            preivewFont.setWeight(QFont::ExtraBold);
        } else if (styleName.contains("Medium")) {
            preivewFont.setWeight(QFont::Medium);
        } else if (styleName.contains("DemiBold")) {
            preivewFont.setWeight(QFont::DemiBold);
        } else if (styleName.contains("Black")) {
            preivewFont.setWeight(QFont::Black);
        }

        m_fontPreviewTxt->setFont(preivewFont);

        m_titleLabel->setText(fontInfo.familyName);
    }
}
