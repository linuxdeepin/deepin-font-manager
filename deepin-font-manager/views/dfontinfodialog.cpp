#include "dfontinfodialog.h"
#include "dfontpreviewitemdef.h"
#include "utils.h"
#include "fonticontext.h"

#include <QFileInfo>
#include <QFontMetrics>
#include <QTextBlock>
#include <QVBoxLayout>
#include <QScrollArea>

#include <DApplication>
#include <DApplicationHelper>
#include <DLog>
#include <DFontSizeManager>
#include <DTipLabel>

QString SpliteText(const QString &text, const QFont &font, int nLabelSize)
{
    QFontMetrics fm(font);
    int nTextSize = fm.width(text);
    if (nTextSize > nLabelSize) {
        int nPos = 0;
        long nOffset = 0;
        for (int i = 0; i < text.size(); i++) {
            nOffset += fm.width(text.at(i));
            if (nOffset >= nLabelSize) {
                nPos = i;
                break;
            }
        }

        nPos = (nPos - 1 < 0) ? 0 : nPos - 1;

        QString qstrLeftData = text.left(nPos);
        QString qstrMidData = text.mid(nPos);
        return qstrLeftData + "\n" + SpliteText(qstrMidData, font, nLabelSize);
    }
    return text;
}

DFontInfoDialog::DFontInfoDialog(DFontPreviewItemData *fontInfo, QWidget *parent)
    : DFontBaseDialog(parent)
    , m_fontInfo(fontInfo)
{
    initUI();
    initConnections();
}

void DFontInfoDialog::initUI()
{
    setFixedSize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H));
    setLogoVisable(false);

    //setWindowOpacity(0.5); //Debug

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    mainLayout->setContentsMargins(10, 0, 10, 10);
    mainLayout->setSpacing(0);

    m_mainFrame = new QWidget(this);
    //m_mainFrame->setFrameShape(DFrame::Shape::NoFrame);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    DPalette pa1 = DApplicationHelper::instance()->palette(m_mainFrame);
    //pa1 = m_messageA->palette();
    QColor color = pa1.textLively().color();
    color.setAlphaF(0.7);
    pa1.setColor(DPalette::WindowText, color);
    //pa1.setColor(DPalette::WindowText, "#000000");
    DApplicationHelper::instance()->setPalette(m_mainFrame, pa1);



    // Font logo
    m_fontLogo = new FontIconText(":/images/font-info-logo.svg", this);
    QFileInfo fileInfo(m_fontInfo->fontInfo.filePath);
    QString suffix = fileInfo.suffix().toLower();
    if (!suffix.compare("ttf")) {
        m_fontLogo->setContent(true);
    } else {
        m_fontLogo->setContent(false);
    }
    DFontSizeManager::instance()->bind(m_fontLogo, DFontSizeManager::T4);
    m_fontLogo->setFontName(m_fontInfo->fontInfo.familyName, m_fontInfo->fontInfo.styleName);

    // Font file name
    m_fontFileName = new DLabel(this);
    m_fontFileName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_fontFileName->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    m_fontFileName->setMinimumHeight(m_fontFileName->fontMetrics().height());
//    m_fontFileName->setFixedHeight(18);
//    QFont fileNameFont;
//    fileNameFont.setPixelSize(12);
//    m_fontFileName->setFont(fileNameFont);
    DFontSizeManager::instance()->bind(m_fontFileName, DFontSizeManager::T8);
    m_fontFileName->setText(QFileInfo(m_fontInfo->fontInfo.filePath).fileName());
    // Set color

    DPalette pa = DApplicationHelper::instance()->palette(m_fontFileName);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::ToolTipText));
    m_fontFileName->setPalette(pa);
//    m_fontFileName->setAlignment(Qt::AlignLeft);
    m_fontFileName->setWordWrap(true);

    /**************************Basic info panel****BEGIN*******************************/
    m_basicInfoFrame = new DFrame(this);
    //m_basicInfoFrame->setBackgroundRole(DPalette::Base);
    //m_basicInfoFrame->setFrameShape(DFrame::Shape::NoFrame);
    m_basicInfoFrame->setFixedWidth(380);
    m_basicInfoFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_baseicInfoLayout = new QVBoxLayout();
    m_baseicInfoLayout->setAlignment(Qt::AlignTop /*| Qt::AlignVCenter*/);
    m_baseicInfoLayout->setContentsMargins(10, 5, 0, 30);
    m_baseicInfoLayout->setSpacing(0);

    DLabel *panelName = new DLabel(this);
    panelName->setFixedHeight(panelName->fontMetrics().height());
//    QFont panelNameFont;
//    // panelNameFont.setBold(true);
//    panelNameFont.setPixelSize(14);
//    panelName->setFont(panelNameFont);
    DFontSizeManager::instance()->bind(panelName, DFontSizeManager::T6);
    panelName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    panelName->setText(DApplication::translate("FontDetailDailog", "Basic info"));
    m_baseicInfoLayout->addWidget(panelName);
    m_baseicInfoLayout->addSpacing(8);

    // Style Row
    QString content = m_fontInfo->fontInfo.styleName;
    addLabelContent(DApplication::translate("FontDetailDailog", "Style"), content);
    m_baseicInfoLayout->addSpacing(6);

    // Type Row
    content = m_fontInfo->fontInfo.type;
    if (content.isEmpty())
        content = "True Type";
    addLabelContent(DApplication::translate("FontDetailDailog", "Type"), content);
    m_baseicInfoLayout->addSpacing(6);

    // Version row
    content = m_fontInfo->fontInfo.version;
    if (content.isEmpty())
        content = "Copyright 2014~2015 Adobe Syste-ms Incorporated (http://www.adob.com/), with Reserved "
                  "Font Name cc Source.";
    addLabelContent(DApplication::translate("FontDetailDailog", "Version"), content);
    m_baseicInfoLayout->addSpacing(6);

    // Description row
    content = m_fontInfo->fontInfo.description;
    if (content.isEmpty())
        content = DApplication::translate("FontDetailDailog", "Unknown");
    addLabelContent(DApplication::translate("FontDetailDailog", "Description"), content);
    m_baseicInfoLayout->addSpacing(6);

    //full name
    addLabelContent(DApplication::translate("FontDetailDailog", "Full name"), m_fontInfo->fontInfo.fullname);
    m_baseicInfoLayout->addSpacing(6);

    //ps name
    addLabelContent(DApplication::translate("FontDetailDailog", "Ps name"), m_fontInfo->fontInfo.psname);
    m_baseicInfoLayout->addSpacing(6);

    //trademark
    addLabelContent(DApplication::translate("FontDetailDailog", "Trademark"), m_fontInfo->fontInfo.trademark);

    m_basicInfoFrame->setLayout(m_baseicInfoLayout);

    /**************************Basic info panel****END*******************************/

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setFrameShape(QFrame::Shape::NoFrame);
    scrollArea->setWidget(m_basicInfoFrame);

    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    // Add childs to main layout
    //mainLayout->addSpacing(50);
    mainLayout->addWidget(m_fontLogo, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(6);
    mainLayout->addWidget(m_fontFileName);
    mainLayout->addSpacing(42);
    mainLayout->addWidget(scrollArea);

    m_mainFrame->setLayout(mainLayout);

    addContent(m_mainFrame);

    // Update font info to UI
//    updateFontInfo();
#ifdef FTM_DEBUG_LAYOUT_COLOR
    m_mainFrame->setStyleSheet("background: red");
    m_fontLogo->setStyleSheet("background: blue");
    m_fontSytleName->setStyleSheet("background: blue");
    styleName->setStyleSheet("background: red");
    m_fontTypeName->setStyleSheet("background: blue");
    typeName->setStyleSheet("background: red");
    m_fontFileName->setStyleSheet("background: blue");
    m_basicInfoFrame->setStyleSheet("background: yellow");
    panelName->setStyleSheet("background: blue");
    versionName->setStyleSheet("background: red");
    m_fontVersion->setStyleSheet("background: blue");
    versionName->setStyleSheet("background: red");
    m_fontDescription->setStyleSheet("background: green");
#endif
}

void DFontInfoDialog::initConnections() {}

void DFontInfoDialog::resizeEvent(QResizeEvent *event)
{
    DFontBaseDialog::resizeEvent(event);

    //m_mainFrame->setFixedSize(event->size().width(), event->size().height());
}

void DFontInfoDialog::addLabelContent(const QString &title, const QString &content)
{
    if (content.isEmpty() || title.isEmpty())
        return;
//    qDebug() << __FUNCTION__ << title << " : " << content << ", font = " << m_fontInfo->fontInfo.toString();

    const int TITLE_MAXWIDTH = 72 + 36;
    int m_maxFieldWidth = width() - 10;
    QHBoxLayout *lyout = new QHBoxLayout;

    QLabel *detail = new QLabel();
    detail->setFixedHeight(50);
//    detail->setFixedHeight(detail->fontMetrics().height() * 3);
    detail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    detail->setWordWrap(true);
    DFontSizeManager::instance()->bind(detail, DFontSizeManager::T8);
    DPalette pa1 = DApplicationHelper::instance()->palette(detail);
    pa1.setBrush(DPalette::Text, pa1.color(DPalette::TextTitle));
    detail->setPalette(pa1);
    detail->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    QString outtxt = SpliteText(content, detail->font(), m_maxFieldWidth);
    detail->setText(outtxt);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    titleLabel->setMinimumHeight(detail->minimumHeight());
    titleLabel->setWordWrap(true);
    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::T8);
    DPalette pa2 = DApplicationHelper::instance()->palette(titleLabel);
    pa2.setBrush(DPalette::Text, pa2.color(DPalette::TextTitle));
    titleLabel->setPalette(pa2);
    titleLabel->setText(SpliteText(title, titleLabel->font(), TITLE_MAXWIDTH));
//    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    titleLabel->setFixedWidth(TITLE_MAXWIDTH);

    lyout->addWidget(titleLabel);
    lyout->addWidget(detail);

    m_baseicInfoLayout->addLayout(lyout);
}
