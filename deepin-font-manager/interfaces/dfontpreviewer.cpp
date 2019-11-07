#include "dfontpreviewer.h"
#include "utils.h"

#include <QPainter>
#include <QPaintEvent>
#include <QRect>

#include <DPalette>
#include <DApplicationHelper>

DFontPreviewer::DFontPreviewer(QWidget *parent)
    : QWidget(parent)
{
    InitData();
    InitConnections();
}

void DFontPreviewer::InitData()
{
    m_previewTexts << "汉体书写信息技术标准相容"
                   << "档案下载使用界面简单"
                   << "支援服务升级资讯专业制作"
                   << "创意空间快速无线上网"
                   << "㈠㈡㈢㈣㈤㈥㈦㈧㈨㈩"
                   << "AaBbCc ＡａＢｂＣｃ";
}

void DFontPreviewer::InitConnections()
{
    connect(this, &DFontPreviewer::previewFontChanged,
            this, &DFontPreviewer::onPreviewFontChanged);
}

void DFontPreviewer::onPreviewFontChanged()
{
    m_previewTexts.clear();
    InitData();
    foreach(auto it, m_previewTexts) {
        QString text = Utils::convertToPreviewString(m_fontPath, it);
        m_previewTexts.replaceInStrings(it, text);
    }
}

void DFontPreviewer::setPreviewFontPath(const QString font)
{
    if (m_fontPath != font) {
        m_fontPath = font;
        Q_EMIT previewFontChanged();
    }
}

void DFontPreviewer::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    //Save pen
    QPen oldPen = painter.pen();

    painter.setRenderHint(QPainter::Antialiasing);
    DPalette pa = DApplicationHelper::instance()->palette(this);
    painter.setBrush(QBrush(pa.color(DPalette::Base)));
    painter.setPen(Qt::transparent);

    QRect rect = this->rect();
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);

    QPainterPath painterPath;
    painterPath.addRoundedRect(rect, 8, 8);
    painter.drawPath(painterPath);

    int topSpace = 0;
    int fontHeight = painter.font().pixelSize();
    int textline = m_previewTexts.size();
    int textSpace = (event->rect().height() - textline*fontHeight-topSpace) / (textline+1);
    int textHeight = fontHeight+textSpace;

    //Restore the pen
    painter.setPen(oldPen);

    QRect startRect(0,topSpace,event->rect().width(),textHeight);
    foreach(auto it, m_previewTexts) {
        painter.drawText(startRect, Qt::AlignCenter, it);

        startRect.setY(startRect.y()+textHeight);
        startRect.setHeight(textHeight);
    }
}
