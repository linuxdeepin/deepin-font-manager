#include "dfontinfoscrollarea.h"
#include <QDebug>
#include <DFontSizeManager>
#include <DApplication>
#include <DApplicationHelper>

#define LAEBL_TEXT_WIDTH   165
#define TITLE_VISIBLE_WIDTH 90
#define INFO_VISIBLE_WIDTH 180

dfontinfoscrollarea::dfontinfoscrollarea(DFontPreviewItemData *pData,  DWidget *parent)
    : DFrame(parent)
    , m_fontInfo(pData)
{
    installEventFilter(this);

    setFrameShape(QFrame::NoFrame);

    auto gridLayout = new QGridLayout;
    gridLayout->setContentsMargins(0, 6, 0, 6);
    gridLayout->setSpacing(3);

    QLocale locale;
    QFontMetrics fm(font());

    if (locale.language() == QLocale::English) {
        m_leftminwidth = fm.horizontalAdvance(("Time modified"));
    } else if (locale.language() == QLocale::Chinese) {
        m_leftminwidth = fm.horizontalAdvance("页面大小");
    }


    createLabel(gridLayout, 0, DApplication::translate("FontDetailDailog", "Style"), m_fontInfo->fontInfo.styleName);
    createLabel(gridLayout, 1, DApplication::translate("FontDetailDailog", "Type"), DApplication::translate("FontDetailDailog", m_fontInfo->fontInfo.type.toLatin1()));


    if (m_fontInfo->fontInfo.version.isEmpty()) {
        QString version = "Copyright 2014~2015 Adobe Syste-ms Incorporated (http://www.adob.com/), with Reserved "
                          "Font Name cc Source.";
        createLabel(gridLayout, 2, DApplication::translate("FontDetailDailog", "Version"), version);
    } else {
        createLabel(gridLayout, 2, DApplication::translate("FontDetailDailog", "Version"), m_fontInfo->fontInfo.version);
    }


    if (m_fontInfo->fontInfo.description.isEmpty()) {
        createLabel(gridLayout, 3, DApplication::translate("FontDetailDailog", "Description"), DApplication::translate("FontDetailDailog", "Unknown"));
    } else {
        createLabel(gridLayout, 3, DApplication::translate("FontDetailDailog", "Description"), m_fontInfo->fontInfo.description);
    }


    createLabel(gridLayout, 4, DApplication::translate("FontDetailDailog", "Full name"), m_fontInfo->fontInfo.fullname);
    createLabel(gridLayout, 5, DApplication::translate("FontDetailDailog", "Ps name"), m_fontInfo->fontInfo.psname);
    createLabel(gridLayout, 6, DApplication::translate("FontDetailDailog", "Trademark"), m_fontInfo->fontInfo.trademark);

    auto vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(10, 10, 10, 10);

    basicLabel = new DLabel(DApplication::translate("FontDetailDailog", "Basic info"));
    DFontSizeManager::instance()->bind(basicLabel, DFontSizeManager::T6);
    vLayout->addWidget(basicLabel);

    vLayout->addItem(gridLayout);

    vLayout->addStretch(1);

    this->setLayout(vLayout);
}

bool dfontinfoscrollarea::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::FontChange) {
        qDebug() << __FUNCTION__ << "FontChange";
        updateText();
    }
    return  DFrame::eventFilter(obj, e);
}

void dfontinfoscrollarea::createLabel(QGridLayout *layout, const int &index, const QString &objName, const QString &sData)
{
    QString str = objName;
    DLabel *label = new DLabel(adjustLength(str), this);
    DFontSizeManager::instance()->bind(label, DFontSizeManager::T8);
    label->setAlignment(Qt::AlignTop);

    label->setFixedWidth(100);
    layout->addWidget(label, index, 0);
    if (pTitleMap.find(label) == pTitleMap.end()) {
        pTitleMap.insert(std::pair<QLabel *, QString>(label, objName));
    }
    if (sData == "") {
        DLabel *labelText = new DLabel(this);
        DFontSizeManager::instance()->bind(labelText, DFontSizeManager::T8);
        labelText->setText(DApplication::translate("FontDetailDailog", "Unknown"));
        labelText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        layout->addWidget(labelText, index, 1);
    } else {
        DFrame *widgets = addTitleFrame(sData, objName);
        widgets->setFrameShape(QFrame::NoFrame);
        layout->addWidget(widgets, index, 1);
    }

}

//增加内容信息frame
DFrame *dfontinfoscrollarea::addTitleFrame(const QString &sData, const QString &objName)
{
    DFrame *m_textShowFrame = new DFrame(this);
    QString ts = elideText(sData, this->font(), INFO_VISIBLE_WIDTH);

    QVBoxLayout *vLayout = new QVBoxLayout;

    QLabel *label = new QLabel(ts, m_textShowFrame);
    label->setFixedWidth(INFO_VISIBLE_WIDTH);
    DFontSizeManager::instance()->bind(label, DFontSizeManager::T8);
    if (pLabelMap.find(label) == pLabelMap.end()) {
        pLabelMap.insert(std::pair<QLabel *, QString>(label, sData));
    }

    label->setAlignment(Qt::AlignLeft);
    label->adjustSize();

    vLayout->addWidget(label);

    vLayout->addStretch(1);
    vLayout->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *textShowLayout = new QHBoxLayout;
    textShowLayout->setContentsMargins(0, 0, 0, 0);
    textShowLayout->setSpacing(0);
    textShowLayout->addLayout(vLayout);
    m_textShowFrame->setLayout(textShowLayout);
    vLayout->setContentsMargins(0, 0, 0, 0);

    textShowLayout->addStretch(1);

    m_textShowFrame->setContentsMargins(0, 0, 0, 0);

    return m_textShowFrame;
}

void dfontinfoscrollarea::paintEvent(QPaintEvent *event)
{
    int m_totalHeight = 0;
    for (auto plabeliter : pLabelMap) {
        if (!plabeliter.first) {
            continue;
        }
        m_totalHeight = m_totalHeight + plabeliter.first->height();
    }
    emit m_signalManager->sizeChange(m_totalHeight + 76 + basicLabel->height());
}
QString dfontinfoscrollarea::elideText(const QString &text, const QFont &font, int nLabelSize)
{
    QFontMetrics fm(font);
    QString strText = text;
    int n_TextSize = fm.width(strText);
    int count  = 0;
    if (n_TextSize > nLabelSize) {
        int n_position = 0;
        long n_curSumWidth = 0;
        for (int i = 0; i < strText.size(); i++) {
            n_curSumWidth += fm.width(strText.at(i));
            if (n_curSumWidth > nLabelSize * (count + 1)) {
                n_position = i;
                strText.insert(n_position, "\n");
                count++;
            }
        }
    }
    return strText;
//勿删，函数递归有时异常，需要排查原因，已用上段方法解决
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
//        return qstrLeftData + "\n" + elideText(qstrMidData, font, nLabelSize);
//    }
//    return text;
}

void dfontinfoscrollarea::updateText()
{
    for (auto pTltle : pTitleMap) {
        if (!pTltle.first) {
            continue;
        }
        QString text = pTltle.second;
        QString newtext = adjustLength(text);
        pTltle.first->setText(newtext);
    }
    for (auto plabeliter : pLabelMap) {
        if (!plabeliter.first) {
            continue;
        }
        QString text = plabeliter.second;
        QString newtext = elideText(text, this->font(), INFO_VISIBLE_WIDTH);
        plabeliter.first->setText(newtext);
    }
}

QString dfontinfoscrollarea::adjustLength(QString &titleName) const
{
    QFont font = this->font();
    QFontMetrics fontMetric(font);

    QString finalTitle = "";
    QString m_curTitle = "";

    int curWidth = 0;

    for (auto str : titleName) {
        if (str == "\t") {
            curWidth  += fontMetric.width("a");
        } else {
            curWidth  += fontMetric.width(str);
        }
        m_curTitle += str;

        if (curWidth > TITLE_VISIBLE_WIDTH) {
            if (m_curTitle == titleName) {
                finalTitle = titleName;
                break;
            } else {

                finalTitle =   m_curTitle.append("...");
                break;
            }
        } else {
            finalTitle = titleName;
        }
    }
    return finalTitle;
}
