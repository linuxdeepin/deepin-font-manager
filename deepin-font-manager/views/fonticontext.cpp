#include "fonticontext.h"

#include <DStyle>
#include <DFontSizeManager>

#include <QBitmap>
#include <QPainter>
#include <QPainterPath>
#include <QImage>
#include <QDebug>

DWIDGET_USE_NAMESPACE
using DTK_GUI_NAMESPACE::DSvgRenderer;

FontIconText::FontIconText(const QString picPath, QWidget *parent)
    : QWidget(parent)
    , m_picPath(picPath)
    , render(new DSvgRenderer())
    , m_text("ttf")
{
    render->load(picPath);
    QSize defaultSize = render->defaultSize();
    setFixedSize(defaultSize.width(), defaultSize.height());
}

void FontIconText::setFontName(const QString &familyName, const QString &styleName)
{
    m_font = font();
    m_font.setFamily(familyName);

    if (styleName.contains("Italic")) {
        m_font.setItalic(true);
    }

    if (styleName.contains("Regular")) {
        m_font.setWeight(QFont::Normal);
    } else if (styleName.contains("Bold")) {
        m_font.setWeight(QFont::Bold);
    } else if (styleName.contains("Light")) {
        m_font.setWeight(QFont::Light);
    } else if (styleName.contains("Thin")) {
        m_font.setWeight(QFont::Thin);
    } else if (styleName.contains("ExtraLight")) {
        m_font.setWeight(QFont::ExtraLight);
    } else if (styleName.contains("ExtraBold")) {
        m_font.setWeight(QFont::ExtraBold);
    } else if (styleName.contains("Medium")) {
        m_font.setWeight(QFont::Medium);
    } else if (styleName.contains("DemiBold")) {
        m_font.setWeight(QFont::DemiBold);
    } else if (styleName.contains("Black")) {
        m_font.setWeight(QFont::Black);
    }
}

void FontIconText::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    //first draw image

    const auto ratio = devicePixelRatioF();
    QSize defaultSize = render->defaultSize() * ratio;
    QImage img = render->toImage(defaultSize);
    QRect picRect = rect();
    if (m_isTtf == true) {
        //then draw text
        QIcon m_fontIcon = QIcon("/usr/share/icons/bloom/mimetypes/256/font-ttf.svg");// QIcon::fromTheme("deepin-font-manager");
        QPixmap p = m_fontIcon.pixmap(defaultSize);
        painter.drawPixmap(picRect, p);
    } else {
        QIcon m_fontIcon = QIcon("/usr/share/icons/bloom/mimetypes/256/font-x-generic.svg");// QIcon::fromTheme("deepin-font-manager");
        QPixmap p = m_fontIcon.pixmap(defaultSize);
        painter.drawPixmap(picRect, p);
    }

}

void FontIconText::setContent(bool isTtf)
{
    m_isTtf = isTtf;
}
