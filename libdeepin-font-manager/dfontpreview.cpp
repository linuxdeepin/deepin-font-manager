/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dfontpreview.h"
#include <QApplication>
#include <QGuiApplication>
#include <QDesktopWidget>
#include <QTextStream>
#include <QFileInfo>
#include <QPainter>
#include <QScreen>
#include <QDebug>
#include <QFile>
#include "dfontwidget.h"

#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>

static const QString lowerTextStock = "abcdefghijklmnopqrstuvwxyz";
static const QString upperTextStock = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const QString punctuationTextStock = "0123456789.:,;(*!?')";
static const int textWidth = 1204;
static QString sampleString = nullptr;
static QString styleName = nullptr;
static QHash<QString, QString> contents = {};

DFontPreview::DFontPreview(QWidget *parent)
    : QWidget(parent),
      m_library(nullptr),
      m_face(nullptr)
{
    initContents();

    setFixedSize(qApp->primaryScreen()->geometry().width() / 1.5,
                 qApp->primaryScreen()->geometry().height() / 1.5);
}

DFontPreview::~DFontPreview()
{
    FT_Done_Face(m_face);
    FT_Done_FreeType(m_library);
}

void DFontPreview::setFileUrl(const QString &url)
{
    // fontDatabase->removeAllApplicationFonts();
    // fontDatabase->addApplicationFont(url);

    FT_Init_FreeType(&m_library);
    m_error = FT_New_Face(m_library, url.toUtf8().constData(), 0, &m_face);

    if (m_error != 0 && QFileInfo(url).completeSuffix() != "pcf.gz")
        return;

    sampleString = getSampleString().simplified();
    styleName = (char *) m_face->style_name;

    repaint();
}

void DFontPreview::paintEvent(QPaintEvent *e)
{
    currentMaxWidth = 1;
    if (m_error != 0)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QFont font(fontDatabase.applicationFontFamilies(0).first());
    painter.setPen(Qt::black);

    if (styleName.contains("Italic")) {
        font.setItalic(true);
    }

    if (styleName.contains("Regular")) {
        font.setWeight(QFont::Normal);
    } else if (styleName.contains("Bold")) {
        font.setWeight(QFont::Bold);
    } else if (styleName.contains("Light")) {
        font.setWeight(QFont::Light);
    } else if (styleName.contains("Thin")) {
        font.setWeight(QFont::Thin);
    } else if (styleName.contains("ExtraLight")) {
        font.setWeight(QFont::ExtraLight);
    } else if (styleName.contains("ExtraBold")) {
        font.setWeight(QFont::ExtraBold);
    } else if (styleName.contains("Medium")) {
        font.setWeight(QFont::Medium);
    } else if (styleName.contains("DemiBold")) {
        font.setWeight(QFont::DemiBold);
    } else if (styleName.contains("Black")) {
        font.setWeight(QFont::Black);
    }

    const int padding = 30;
    const int x = 35;
    int y = 10;
    int fontSize = 25;

    font.setPointSize(fontSize);
    painter.setFont(font);

    const QFontMetrics metrics(font);

    // if we don't have lowercase/uppercase/punctuation text in the face
    // we omit it directly, and render a random text below.

    /*根据获取的新point进行绘制 UT000539 fix bug 27030*/
    if (checkFontContainText(m_face, lowerTextStock)) {
        const int lowerWidth = metrics.width(lowerTextStock);
        isNeedScroll(lowerWidth);
        const int lowerHeight = metrics.height();
        QPoint baseLinePoint = adjustPreviewFontBaseLinePoint(QRect(x, y + padding, lowerWidth, lowerHeight), metrics);
        painter.drawText(baseLinePoint.x(), baseLinePoint.y(), lowerTextStock);
        y += lowerHeight;
    }

    if (checkFontContainText(m_face, upperTextStock)) {
        const int upperWidth = metrics.width(upperTextStock);
        isNeedScroll(upperWidth);
        const int upperHeight = metrics.height();
        QPoint baseLinePoint = adjustPreviewFontBaseLinePoint(QRect(x, y + padding, upperWidth, upperHeight), metrics);
        painter.drawText(baseLinePoint.x(), baseLinePoint.y(), upperTextStock);
        y += upperHeight;
    }

    if (checkFontContainText(m_face, punctuationTextStock)) {
        const int punWidth = metrics.width(punctuationTextStock);
        isNeedScroll(punWidth);
        int punHeight = metrics.height();
        QPoint baseLinePoint = adjustPreviewFontBaseLinePoint(QRect(x, y + padding, punWidth, punHeight), metrics);
        painter.drawText(baseLinePoint.x(), baseLinePoint.y(), punctuationTextStock);
        y += punHeight;
    }
    for (int i = 0; i < 20; ++i) {
        fontSize += 3;
        font.setPointSize(fontSize);
        painter.setFont(font);

        QFontMetrics met(font);
        int sampleWidth = met.width(sampleString);
        isNeedScroll(sampleWidth);
        int sampleHeight = met.height();

        if (y + sampleHeight >= rect().height() - padding * 2)
            break;

        QPoint baseLinePoint = adjustPreviewFontBaseLinePoint(QRect(x, y + padding * 2, sampleWidth, sampleHeight), met);
        painter.drawText(baseLinePoint.x(), baseLinePoint.y(), sampleString);
        y += sampleHeight + padding;
    }
    /*判断是否需要滚动条 UT000539*/
    if (currentMaxWidth > viewWidth) {
        setFixedWidth(currentMaxWidth);
        m_needScroll = true;
    } else {
        setFixedWidth(qApp->primaryScreen()->geometry().width() / 1.5);
        m_needScroll = false;
    }
    QWidget::paintEvent(e);
}

/*判断text宽度是否超过当前宽度 UT000539*/
void DFontPreview::isNeedScroll(const int width)
{
    if (m_needScroll == false) {
        if (width > textWidth) {
            m_needScroll = true;
        }
    }
    if (width > currentMaxWidth) {
        currentMaxWidth = width;
    }
}

void DFontPreview::initContents()
{
    QFile file("/usr/share/deepin-font-manager/CONTENTS.txt");

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray content = file.readAll();
    QTextStream stream(&content, QIODevice::ReadOnly);

    file.close();

    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const QStringList items = line.split(QChar(':'));

        contents.insert(items.at(0), items.at(1));
    }
}

QString DFontPreview::getSampleString()
{
    QString sampleString = nullptr;
    bool isAvailable = false;

    // check the current system language sample string.
    sampleString = getLanguageSampleString(QLocale::system().name());
    if (checkFontContainText(m_face, sampleString) && !sampleString.isEmpty()) {
        isAvailable = true;
    }

    // check english sample string.
    if (!isAvailable) {
        sampleString = getLanguageSampleString("en");
        if (checkFontContainText(m_face, sampleString)) {
            isAvailable = true;
        }
    }

    // random string from available chars.
    if (!isAvailable) {
        sampleString = buildCharlistForFace(m_face, 36);
    }

    return sampleString;
}

QString DFontPreview::getLanguageSampleString(const QString &language)
{
    QString result = nullptr;
    QString key = nullptr;

    if (contents.contains(language)) {
        key = language;
    } else {
        const QStringList parseList = language.split("_", QString::SkipEmptyParts);
        if (parseList.length() > 0 &&
                contents.contains(parseList.first())) {
            key = parseList.first();
        }
    }

    if (contents.contains(key)) {
        auto findResult = contents.find(key);
        result.append(findResult.value());
    }

    return result;
}

bool DFontPreview::checkFontContainText(FT_Face face, const QString &text)
{
    if (face == nullptr || face->num_charmaps == 0)
        return false;

    bool retval = true;

    int err = 0;
    if (face->charmap == nullptr)
        err = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
    if (err != 0) {
        for (int i = 0; i < face->num_charmaps; ++i) {
            err = FT_Select_Charmap(face, face->charmaps[i]->encoding);
            if (err == 0) {
                break;
            }
        }
    }

    for (auto ch : text) {
        if (!FT_Get_Char_Index(face, ch.unicode())) {
            retval = false;
            break;
        }
    }

    return retval;
}

QString DFontPreview::buildCharlistForFace(FT_Face face, int length)
{
    QString retval;
    if (face == nullptr)
        return retval;

    FcCharSet *fcs = nullptr;
    FcChar32 count = 0;

    fcs = FcFreeTypeCharSet(face, nullptr);

    count = FcCharSetCount(fcs);
    qDebug() << __FUNCTION__ << " total count = " << count;

    QList<uint> ucs4List;
    if (count > 0) {
        FcChar32 ucs4, pos, map[FC_CHARSET_MAP_SIZE];
        int retCount = 0;
        uint unicode = 0;

        for (ucs4 = FcCharSetFirstPage(fcs, map, &pos);
                ucs4 != FC_CHARSET_DONE;
                ucs4 = FcCharSetNextPage(fcs, map, &pos)) {
            for (uint i = 0; i < FC_CHARSET_MAP_SIZE; i++) {
                if (map[i] == 0)
                    continue;
                for (uint j = 0; j < 32; j++) {
                    if ((map[i] & (1 << j)) == 0)
                        continue;

                    unicode = ucs4 + i * 32 + j;
                    ucs4List << unicode;
                    retCount++;
                }
            }
        }

        int len = (length > retCount) ? retCount : length;

        uint firstChar = ucs4List[len - 1];
        int index = 0;
        for (int i = len - 2; i >= 0; i--) {
            if (firstChar - ucs4List[i] > static_cast<uint>((len - 1 - i) + len - 1)) {
                index = i + 1;
                break;
            }
        }

        for (int i = index; i < retCount && i < index + length; i++) {
            retval += QString::fromUcs4(&ucs4List[i], 1);
        }
    }

    FcCharSetDestroy(fcs);

    return retval;
}

/*返回绘制起始point UT000539 fix bug 27030*/
QPoint DFontPreview::adjustPreviewFontBaseLinePoint(const QRect &fontPreviewRect, const QFontMetrics &previewFontMetrics) const
{
    Q_UNUSED(previewFontMetrics);
    int commonFontDescent = fontPreviewRect.height() / 4;
    int baseLineX = fontPreviewRect.x();
    int baseLineY = fontPreviewRect.bottom() - commonFontDescent;

    return QPoint(baseLineX, baseLineY);
}
