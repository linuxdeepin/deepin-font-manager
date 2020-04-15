#include "dfontinfodialog.h"
#include "dfontpreviewitemdef.h"
#include "utils.h"
#include "fonticontext.h"
#include "dfontinfoscrollarea.h"

#include <QFileInfo>
#include <QFontMetrics>
#include <QTextBlock>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>

#include <DApplication>
#include <DApplicationHelper>
#include <DLog>
#include <DFontSizeManager>
#include <DTipLabel>
#include <QBitmap>

DFontInfoDialog::DFontInfoDialog(DFontPreviewItemData *fontInfo, QWidget *parent)
    : DFontBaseDialog(parent)
    , m_fontInfo(fontInfo)
    , fontinfoArea(nullptr)
{
    m_faCenter = parent->geometry().center();
    initUI();
    initConnections();
    this->move(m_faCenter - this->rect().center());
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
                if (strText.length() - 52 > 21) {
                    count = 0;
                    QString str = strText.mid(52, 17).append("...").append(strText.right(5));
                    strText = strText.remove(52, strText.size());
                    strText = strText.append(str);
                    break;
                }
            }
        }
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

    DFontSizeManager::instance()->bind(m_fontFileName, DFontSizeManager::T8);
    QString str = QFileInfo(m_fontInfo->fontInfo.filePath).fileName();
    QString text = AutoFeed(str);


    QFontMetrics elideFont(this->font());
//    m_fontFileName->setText(elideFont.elidedText(text, Qt::ElideRight, 140));
    m_fontFileName->setText(text);
    // Set color
    DPalette pa = DApplicationHelper::instance()->palette(m_fontFileName);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::ToolTipText));
    m_fontFileName->setPalette(pa);

    /**************************Basic info panel****BEGIN*******************************/
    m_basicInfoFrame = new DFrame(this);
//    initConnections();//themeChanged


    m_basicInfoFrame->setFrameShape(DFrame::Shape::NoFrame);
    m_basicInfoFrame->setFixedWidth(275);
    m_basicInfoFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_baseicInfoLayout = new QVBoxLayout();
    m_baseicInfoLayout->setAlignment(Qt::AlignTop /*| Qt::AlignVCenter*/);
    m_baseicInfoLayout->setContentsMargins(10, 10, 40, 10);
    m_baseicInfoLayout->setSpacing(0);

    DLabel *panelName = new DLabel(this);
    panelName->setWordWrap(true);
    panelName->setFixedHeight(panelName->fontMetrics().height() + 2);
//    panelName->setFixedHeight(40);

    DFontSizeManager::instance()->bind(panelName, DFontSizeManager::T6);
    panelName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//    panelName->setText(DApplication::translate("FontDetailDailog", "Basic info"));
    m_baseicInfoLayout->addWidget(panelName);
    m_baseicInfoLayout->addSpacing(4);


//    insertContents();
    m_baseicInfoLayout->addStretch(2);
    m_basicInfoFrame->setLayout(m_baseicInfoLayout);

    /**************************Basic info panel****END*******************************/

    scrollArea = new QScrollArea();
//    scrollArea->setLineWidth(120);
    scrollArea->setFixedSize(QSize(290, 375));
//    scrollArea->setContentsMargins(0,0,30,0);

    QPixmap bmp(QSize(280, 375));
    bmp.fill();
    QPainter p(&bmp);
    bmp.setDevicePixelRatio(0);
    p.setBrush(Qt::black);
    p.drawRoundedRect(bmp.rect(), 12, 12);
    p.setRenderHint(QPainter::Antialiasing);
    scrollArea->viewport()->setMask(bmp);

    m_basicInfoFrame->setAttribute(Qt::WA_TranslucentBackground);

    scrollArea->setFrameShape(QFrame::Shape::NoFrame);

    fontinfoArea = new dfontinfoscrollarea(m_fontInfo);
    scrollArea->setWidget(fontinfoArea);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    // Add childs to main layout
    mainLayout->addWidget(m_fontLogo, 0, Qt::AlignHCenter);
//    mainLayout->addSpacing(6);
    mainLayout->addWidget(m_fontFileName);
//    mainLayout->addSpacing(42);
    mainLayout->addStretch(8);
    mainLayout->addWidget(scrollArea);
    m_mainFrame->setLayout(mainLayout);
    addContent(m_mainFrame);


    if (DApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        DPalette paFrame = DApplicationHelper::instance()->palette(scrollArea->viewport());
        QColor colorFrame = paFrame.textLively().color();
        colorFrame.setAlphaF(0.05);
        paFrame.setColor(DPalette::Base, colorFrame);
        DApplicationHelper::instance()->setPalette(scrollArea->viewport(), paFrame);

    } else if (DApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        DPalette paFrame = DApplicationHelper::instance()->palette(scrollArea->viewport());
        QColor colorFrame = paFrame.textLively().color();
        colorFrame.setAlphaF(0.70);
        paFrame.setColor(DPalette::Base, colorFrame);
        DApplicationHelper::instance()->setPalette(scrollArea->viewport(), paFrame);
    }
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
void DFontInfoDialog::initConnections()
{
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, [ = ] {
        DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();

        if (DApplicationHelper::DarkType == themeType)
        {
            DPalette paFrame = DApplicationHelper::instance()->palette(scrollArea->viewport());
            QColor colorFrame = paFrame.textLively().color();
            colorFrame.setAlphaF(0.05);
            paFrame.setColor(DPalette::Base, colorFrame);
            DApplicationHelper::instance()->setPalette(scrollArea->viewport(), paFrame);
        } else if (DApplicationHelper::LightType == themeType)
        {
            DPalette paFrame = DApplicationHelper::instance()->palette(scrollArea->viewport());
            QColor colorFrame = paFrame.textLively().color();
            colorFrame.setAlphaF(0.70);
            paFrame.setColor(DPalette::Base, colorFrame);
            DApplicationHelper::instance()->setPalette(scrollArea->viewport(), paFrame);
        }
    });

    connect(m_signalManager, &SignalManager::sizeChange, this, [ = ](int height) {
//        fontinfoArea->setFixedHeight(height);
//        scrollArea->viewport()->setFixedHeight(height + 20);
//        m_height = height;
        if (height * 1.3 + 280 < DEFAULT_WINDOW_H) {
            this->setFixedHeight(height * 1.3 + 280);
//            this->move(m_faCenter - this->rect().center());
            QPixmap bmp(QSize(280, height * 1.3));
            bmp.fill();
            QPainter p(&bmp);
            bmp.setDevicePixelRatio(0);
            p.setBrush(Qt::black);
            p.drawRoundedRect(bmp.rect(), 12, 12);
            p.setRenderHint(QPainter::Antialiasing);
            scrollArea->viewport()->setMask(bmp);
            scrollArea->viewport()->setFixedHeight(height * 1.3);
            scrollArea->setFixedHeight(height * 1.3);
        } else {
            this->setFixedHeight(DEFAULT_WINDOW_H);
//            this->move(m_faCenter - this->rect().center());
            QPixmap bmp(QSize(280, 375));
            bmp.fill();
            QPainter p(&bmp);
            bmp.setDevicePixelRatio(0);
            p.setBrush(Qt::black);
            p.drawRoundedRect(bmp.rect(), 12, 12);
            p.setRenderHint(QPainter::Antialiasing);
            scrollArea->viewport()->setMask(bmp);
            scrollArea->viewport()->setFixedHeight(375);
            scrollArea->setFixedHeight(375);
        }
        //        this->setFixedSize(QSize(DEFAULT_WINDOW_W, height + 300));
    });
}
void DFontInfoDialog::resizeEvent(QResizeEvent *event)
{
    DFontBaseDialog::resizeEvent(event);

}




