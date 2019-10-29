#include "dfontinfodialog.h"
#include "dfontpreviewitemdef.h"
#include "utils.h"

#include <QFileInfo>
#include <QFontMetrics>
#include <QTextBlock>
#include <QVBoxLayout>

#include <DApplication>
#include <DLog>

DFontInfoDialog::DFontInfoDialog(DFontPreviewItemData *fontInfo, QWidget *parent)
    : DDialog(parent)
    , m_fontInfo(fontInfo)
{
    initUI();
    initConnections();
}

void DFontInfoDialog::initUI()
{
    setFixedSize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H));

    //setWindowOpacity(0.5); //Debug

    // ToDo:
    //    Need localize the string

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    mainLayout->setContentsMargins(10, 0, 10, 10);
    mainLayout->setSpacing(0);

    m_mainFrame = new DFrame(this);
    //m_mainFrame->setFrameShape(DFrame::Shape::NoFrame);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Font logo
    m_fontLogo = new DLabel(this);
    m_fontLogo->setFixedSize(QSize(128, 128));
    m_fontLogo->setPixmap(Utils::renderSVG(":/images/font-info-logo.svg", m_fontLogo->size()));

    // Font file name
    m_fontFileName = new DLabel(this);
    m_fontFileName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fontFileName->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    m_fontFileName->setFixedHeight(18);
    QFont fileNameFont;
    fileNameFont.setPixelSize(12);
    m_fontFileName->setFont(fileNameFont);
    m_fontFileName->setText("SourceHanSansSC-ExtraLight");

    /**************************Basic info panel****BEGIN*******************************/
    m_basicInfoFrame = new DFrame(this);
    m_basicInfoFrame->setBackgroundRole(DPalette::Base);
    //m_basicInfoFrame->setFrameShape(DFrame::Shape::NoFrame);
    m_basicInfoFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *baseicInfoLayout = new QVBoxLayout();
    baseicInfoLayout->setAlignment(Qt::AlignTop /*| Qt::AlignVCenter*/);
    baseicInfoLayout->setContentsMargins(10, 5, 0, 0);
    baseicInfoLayout->setSpacing(0);

    DLabel *panelName = new DLabel(this);
    panelName->setFixedHeight(20);
    QFont panelNameFont;
    // panelNameFont.setBold(true);
    panelNameFont.setPixelSize(14);
    panelName->setFont(panelNameFont);
    panelName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    panelName->setText(DApplication::translate("FontDetailDailog", "Basic Information"));

    // Style Row
    QHBoxLayout* styleRowLayout = new QHBoxLayout();
    styleRowLayout->setContentsMargins(0, 0, 0, 0);
    styleRowLayout->setSpacing(0);

    QFont basicInfoFont;
    basicInfoFont.setPixelSize(12);

    DLabel *styleName = new DLabel(this);
    styleName->setFixedSize(QSize(60, 18));
    styleName->setFont(basicInfoFont);
    styleName->setText(DApplication::translate("FontDetailDailog", "Style"));
    styleName->setAlignment(Qt::AlignLeft);

    m_fontSytleName = new DLabel(this);
    m_fontSytleName->setFixedHeight(18);
    m_fontSytleName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fontSytleName->setFont(basicInfoFont);
    m_fontSytleName->setText("Regular");
    m_fontSytleName->setAlignment(Qt::AlignLeft);

    styleRowLayout->addWidget(styleName);
    styleRowLayout->addWidget(m_fontSytleName);

    // Type Row
    QHBoxLayout* typeRowLayout = new QHBoxLayout();
    typeRowLayout->setContentsMargins(0, 0, 0, 0);
    typeRowLayout->setSpacing(0);

    DLabel *typeName = new DLabel(this);
    typeName->setFixedSize(QSize(60, 18));
    typeName->setFont(basicInfoFont);
    typeName->setText(DApplication::translate("FontDetailDailog", "Type"));
    typeName->setAlignment(Qt::AlignLeft);

    m_fontTypeName = new DLabel(this);
    m_fontTypeName->setFixedHeight(18);
    m_fontTypeName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fontTypeName->setFont(basicInfoFont);
    m_fontTypeName->setText("True Type");
    m_fontTypeName->setAlignment(Qt::AlignLeft);

    typeRowLayout->addWidget(typeName);
    typeRowLayout->addWidget(m_fontTypeName,1);

    // Version row
    QHBoxLayout* versionRowLayout = new QHBoxLayout();
    versionRowLayout->setContentsMargins(0, 0, 0, 0);
    versionRowLayout->setSpacing(0);

    DLabel *versionName = new DLabel(this);
    versionName->setFixedSize(QSize(60, 18));
    versionName->setFont(basicInfoFont);
    versionName->setText(DApplication::translate("FontDetailDailog", "Version"));
    versionName->setAlignment(Qt::AlignLeft);

    m_fontVersion = new DLabel(this);
    m_fontVersion->setFixedHeight(72);
    m_fontVersion->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fontVersion->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_fontVersion->setFrameShape(DFrame::Shape::NoFrame);
    m_fontVersion->setContentsMargins(0, 0, 0, 0);
    m_fontVersion->setFont(basicInfoFont);
    m_fontVersion->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_fontVersion->setText(
                "Copyright 2014~2015 Adobe Syste-ms Incorporated (http://www.adob.com/), with Reserved "
                "Font Name cc Source.");
    m_fontVersion->setWordWrap(true);

    versionRowLayout->addWidget(versionName,0,Qt::AlignTop);
    versionRowLayout->addWidget(m_fontVersion);

    // Description row
    QHBoxLayout* despRowLayout = new QHBoxLayout();
    despRowLayout->setContentsMargins(0, 0, 0, 0);
    despRowLayout->setSpacing(0);

    DLabel *despName = new DLabel(this);
    despName->setFixedSize(QSize(60, 18));
    despName->setFont(basicInfoFont);
    despName->setText(DApplication::translate("FontDetailDailog", "Description"));
    despName->setAlignment(Qt::AlignLeft);

    m_fontDescription = new DLabel(this);
    m_fontDescription->setFixedHeight(18);
    m_fontDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fontDescription->setFont(basicInfoFont);
    m_fontDescription->setText(DApplication::translate("FontDetailDailog", "Unkonw"));
    m_fontDescription->setAlignment(Qt::AlignLeft);

    despRowLayout->addWidget(despName);
    despRowLayout->addWidget(m_fontDescription, 1);

    baseicInfoLayout->addWidget(panelName);
    baseicInfoLayout->addSpacing(8);
    baseicInfoLayout->addLayout(styleRowLayout);
    baseicInfoLayout->addSpacing(8);
    baseicInfoLayout->addLayout(typeRowLayout);
    baseicInfoLayout->addSpacing(8);
    baseicInfoLayout->addLayout(versionRowLayout);
    baseicInfoLayout->addLayout(despRowLayout);

    m_basicInfoFrame->setLayout(baseicInfoLayout);
    /**************************Basic info panel****END*******************************/

    // Add childs to main layout
    mainLayout->addSpacing(50);
    mainLayout->addWidget(m_fontLogo, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(9);
    mainLayout->addWidget(m_fontFileName);
    mainLayout->addSpacing(45);
    mainLayout->addWidget(m_basicInfoFrame);

    m_mainFrame->setLayout(mainLayout);

    // Update font info to UI
    updateFontInfo();
#ifdef FTM_DEBUG_LAYOUT_COLOR
    m_mainFrame->setStyleSheet("background: red");
    m_fontLogo->setStyleSheet("background: silver");
    m_fontTypeName->setStyleSheet("background: silver");
    m_fontFileName->setStyleSheet("background: silver");
    m_basicInfoFrame->setStyleSheet("background: yellow");
    panelName->setStyleSheet("background: blue");
    versionName->setStyleSheet("background: green");
    m_fontVersion->setStyleSheet("background: silver");
    m_fontDescription->setStyleSheet("background: green");
#endif
}

void DFontInfoDialog::initConnections() {}

void DFontInfoDialog::resizeEvent(QResizeEvent *event)
{
    DDialog::resizeEvent(event);

    m_mainFrame->setFixedSize(event->size().width(), event->size().height());
}

void DFontInfoDialog::updateFontInfo()
{
    if (nullptr != m_fontInfo) {
        if (!m_fontInfo->fontInfo.filePath.isEmpty()) {
            QFileInfo file(m_fontInfo->fontInfo.filePath);
            m_fontFileName->setText(file.fileName());
        }

        if (!m_fontInfo->fontInfo.styleName.isEmpty()) {
            m_fontSytleName->setText(m_fontInfo->fontInfo.styleName);
        }

        if (!m_fontInfo->fontInfo.type.isEmpty()) {
            m_fontTypeName->setText(m_fontInfo->fontInfo.type);
        }

        if (!m_fontInfo->fontInfo.copyright.isEmpty()) {
            m_fontVersion->setText(Utils::holdTextInRect(
                                       m_fontVersion->font(),
                                       m_fontInfo->fontInfo.copyright,
                                       QSize(200,72) )
                                   );
        }

        if (!m_fontInfo->fontInfo.description.isEmpty()) {
            QString elideString = m_fontDescription->fontMetrics().elidedText(
                        m_fontInfo->fontInfo.description, Qt::ElideRight, 200);
            m_fontDescription->setText(elideString);
        }
    }
}
