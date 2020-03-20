#include "dfontinfodialog.h"
#include "dfontpreviewitemdef.h"
#include "utils.h"
#include "fonticontext.h"

#include <QFileInfo>
#include <QFontMetrics>
#include <QTextBlock>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <DDLabel.h>

#include <DApplication>
#include <DApplicationHelper>
#include <DLog>
#include <DFontSizeManager>
#include <DTipLabel>
#include <QBitmap>

//QString SpliteText(const QString &text, const QFont &font, int nLabelSize)
//{
//    QFontMetrics fm(font);
//    int nTextSize = fm.width(text);
//    if (nTextSize > nLabelSize) {
//        int nPos = 0;
//        long nOffset = 0;
//        for (int i = 0; i < text.size(); i++) {
//            nOffset += fm.width(text.at(i));
//            if (nOffset >= nLabelSize) {
//                nPos = i;
//                break;
//            }
//        }

//        nPos = (nPos - 1 < 0) ? 0 : nPos - 1;

//        QString qstrLeftData = text.left(nPos);
//        QString qstrMidData = text.mid(nPos);
//        return qstrLeftData + "\n" + SpliteText(qstrMidData, font, nLabelSize);
//    }
//    return text;
//}
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

QString DFontInfoDialog::AutoFeed(QString &text)
{
    QString strText = text;
    int AntoIndex = 1;
    int count = 0;
    if (!strText.isEmpty()) {

        for (int i = 1; i < strText.size() + 1; i++) { //25个字符换一行
            if (i == 25 * AntoIndex + AntoIndex - 1) {
                strText.insert(i, "\n");
                count++;
                AntoIndex ++;
            }
            if (count == 2) {
                count = 0;
                break;
            }
        }
        QString str = strText.mid(52, 17).append("...").append(strText.right(5));
        strText = strText.remove(52, strText.size());
        strText = strText.append(str);

    }
    return strText;
}

void DFontInfoDialog::initUI()
{
    setFixedSize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H));
    setLogoVisable(false);

    //setWindowOpacity(0.5); //Debug

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    mainLayout->setContentsMargins(10, 0, 0, 10);
    mainLayout->setSpacing(0);

    m_mainFrame = new QWidget(this);
    //m_mainFrame->setFrameShape(DFrame::Shape::NoFrame);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

//    DPalette pa1 = DApplicationHelper::instance()->palette(m_mainFrame);
//    //pa1 = m_messageA->palette();
//    QColor color = pa1.textLively().color();
//    color.setAlphaF(0.7);
//    pa1.setColor(DPalette::WindowText, color);
//    //pa1.setColor(DPalette::WindowText, "#000000");
//    DApplicationHelper::instance()->setPalette(m_mainFrame, pa1);


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

    m_fontFileName = new DLabel(this);
    m_fontFileName->adjustSize();
    m_fontFileName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_fontFileName->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    m_fontFileName->setMinimumHeight(m_fontFileName->fontMetrics().height());
    m_fontFileName->setWordWrap(true);



//    m_fontFileName->setFixedHeight(18);
//    QFont fileNameFont;
//    fileNameFont.setPixelSize(12);
//    m_fontFileName->setFont(fileNameFont);
    DFontSizeManager::instance()->bind(m_fontFileName, DFontSizeManager::T8);
    QString str = QFileInfo(m_fontInfo->fontInfo.filePath).fileName();
    QString text = AutoFeed(str);

    m_fontFileName->setText(text);
    // Set color
    DPalette pa = DApplicationHelper::instance()->palette(m_fontFileName);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::ToolTipText));
    m_fontFileName->setPalette(pa);

    /**************************Basic info panel****BEGIN*******************************/
    m_basicInfoFrame = new DFrame(this);
    //m_basicInfoFrame->setBackgroundRole(DPalette::Base);
    m_basicInfoFrame->setFrameShape(DFrame::Shape::NoFrame);
    m_basicInfoFrame->setFixedWidth(275);
    m_basicInfoFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_baseicInfoLayout = new QVBoxLayout();
    m_baseicInfoLayout->setAlignment(Qt::AlignTop /*| Qt::AlignVCenter*/);
    m_baseicInfoLayout->setContentsMargins(10, 10, 40, 10);
    m_baseicInfoLayout->setSpacing(0);

    DLabel *panelName = new DLabel(this);
    panelName->setFixedHeight(panelName->fontMetrics().height()+2);
//    QFont panelNameFont;
//    // panelNameFont.setBold(true);
//    panelNameFont.setPixelSize(14);
//    panelName->setFont(panelNameFont);
    DFontSizeManager::instance()->bind(panelName, DFontSizeManager::T6);
    panelName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    panelName->setText(DApplication::translate("FontDetailDailog", "Basic info"));
    m_baseicInfoLayout->addWidget(panelName);
    m_baseicInfoLayout->addSpacing(8);


    insertContents();
    m_baseicInfoLayout->addStretch();
    m_basicInfoFrame->setLayout(m_baseicInfoLayout);

    /**************************Basic info panel****END*******************************/

    QScrollArea *scrollArea = new QScrollArea;

    scrollArea = new QScrollArea();
//    DPalette pa2 = scrollArea->palette();
//    pa2.setBrush(DPalette::Base, Qt::red);
//    scrollArea->setPalette(pa2);

    //DPalette pa2 = DApplicationHelper::instance()->palette(scrollArea);
    //pa1 = m_messageA->palette();
    //QColor color2 = pa2.textLively().color();
    //color2.setAlphaF(0.95);
    //pa2.setColor(DPalette::Base, color2);
    //DApplicationHelper::instance()->setPalette(scrollArea, pa2);

    scrollArea->setLineWidth(120);
    scrollArea->setFixedSize(QSize(290, 375));

    QPixmap bmp(QSize(280, 375));
    bmp.fill();
    QPainter p(&bmp);
    bmp.setDevicePixelRatio(0);
    p.setBrush(Qt::black);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawRoundedRect(bmp.rect(), 12, 12);
    scrollArea->viewport()->setMask(bmp);



    DPalette paFrame = DApplicationHelper::instance()->palette(m_basicInfoFrame);
    QColor colorFrame = paFrame.textLively().color();
    colorFrame.setAlphaF(0.2);
    paFrame.setColor(DPalette::Base, colorFrame);
    DApplicationHelper::instance()->setPalette(m_basicInfoFrame, paFrame);

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

    const int TITLE_MAXWIDTH = 110;
//    int m_maxFieldWidth = width() - 40;

    DDLabel *titles= new DDLabel();
    DDLabel *details = new DDLabel();

    details->setMinimumHeight(0);
    titles->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    titles->setObjectName("commom");
    titles->adjustSize();
    titles->setFixedWidth(TITLE_MAXWIDTH);
    DFontSizeManager::instance()->bind(titles, DFontSizeManager::T8);
    DPalette paTitles = DApplicationHelper::instance()->palette(titles);
    paTitles.setBrush(DPalette::WindowText, paTitles.color(DPalette::TextTitle));
    titles->setPalette(paTitles);
    QString title_content = SpliteText(title, titles->font(), TITLE_MAXWIDTH);
    titles->Settext(title_content);
    titles->setAlignment(Qt::AlignLeft | Qt::AlignTop);
//    titles->setWordWrap(true);

    details->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    details->setObjectName("commom");
    details->adjustSize();
    details->setFixedWidth(150);
    DPalette paDetails = DApplicationHelper::instance()->palette(details);
    paDetails.setBrush(DPalette::WindowText, paDetails.color(DPalette::TextTitle));
    details->setPalette(paDetails);
    details->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    QString details_content = SpliteText(content, details->font(), 220);


    details->Settext(details_content);
    details->setWordWrap(true);
    DFontSizeManager::instance()->bind(details, DFontSizeManager::T8);

    QHBoxLayout *hlyout = new QHBoxLayout;
    hlyout->addWidget(titles);
//       hlyout->addSpacing(30);
    hlyout->addWidget(details);
//        hlyout->addSpacing(15);
        hlyout->addStretch();
    m_baseicInfoLayout->addLayout(hlyout);

}
void DFontInfoDialog::addLabelContent_VersionAndDescription(const QString &title, const QString &content)
{


    if (content.isEmpty() || title.isEmpty())
        return;

//    const int TITLE_MAXWIDTH = 72 + 36;
    const int TITLE_MAXWIDTH = 110;
//    int m_maxFieldWidth = width() - 40;
    DDLabel *titles= new DDLabel();
    DLabel *details = new DLabel();
    details->setMinimumHeight(0);
    titles->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    titles->adjustSize();
//    titles->setWordWrap(true);
    titles->setFixedWidth(TITLE_MAXWIDTH);
    DFontSizeManager::instance()->bind(titles, DFontSizeManager::T8);
    DPalette paTitles = DApplicationHelper::instance()->palette(titles);
    paTitles.setBrush(DPalette::WindowText, paTitles.color(DPalette::TextTitle));
    titles->setPalette(paTitles);
    titles->Settext(SpliteText(title, titles->font(), TITLE_MAXWIDTH));
    titles->setAlignment(Qt::AlignLeft | Qt::AlignTop);
//    titles->setWordWrap(true);

    details->setObjectName("high");

    details->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    details->adjustSize();
    details->setFixedWidth(150);
    DPalette paDetails = DApplicationHelper::instance()->palette(details);
    paDetails.setBrush(DPalette::WindowText, paDetails.color(DPalette::TextTitle));
    details->setPalette(paDetails);
    details->setAlignment(Qt::AlignLeft | Qt::AlignTop);
//    details->Settext(SpliteText(content, details->font(),220));
    QString detail_content =SpliteText(content, details->font(), 220);
    details->setText(detail_content);
    details->setWordWrap(true);
    DFontSizeManager::instance()->bind(details, DFontSizeManager::T8);

    QHBoxLayout *hlyout = new QHBoxLayout;

    hlyout->addWidget(titles);
    hlyout->addWidget(details);
    hlyout->addStretch();

    m_baseicInfoLayout->addLayout(hlyout);
}
void DFontInfoDialog::insertContents()
{

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
    addLabelContent_VersionAndDescription(DApplication::translate("FontDetailDailog", "Version"), content);
    m_baseicInfoLayout->addSpacing(6);

    // Description row
    content = m_fontInfo->fontInfo.description;
    if (content.isEmpty())
        content = DApplication::translate("FontDetailDailog", "Unknown");
    addLabelContent_VersionAndDescription(DApplication::translate("FontDetailDailog", "Description"), content);
    m_baseicInfoLayout->addSpacing(6);

    //full name
    addLabelContent(DApplication::translate("FontDetailDailog", "Full name"), m_fontInfo->fontInfo.fullname);
    m_baseicInfoLayout->addSpacing(6);

    //ps name
    addLabelContent(DApplication::translate("FontDetailDailog", "Ps name"), m_fontInfo->fontInfo.psname);
    m_baseicInfoLayout->addSpacing(6);

    //trademark
    addLabelContent(DApplication::translate("FontDetailDailog", "Trademark"), m_fontInfo->fontInfo.trademark);
}
//void DFontInfoDialog::paintEvent(QPaintEvent *event)
//{
//    QList<DDLabel *> labels = m_basicInfoFrame->findChildren<DDLabel *>();
//    foreach (DDLabel * label, labels) {
//       if(label->objectName()=="high"){
//           label->setFixedHeight(label->fontMetrics().height()*4);
//       }else if(label->objectName()=="commom"){
//           label->setFixedHeight(label->fontMetrics().height());
//       }
//    }
//}
