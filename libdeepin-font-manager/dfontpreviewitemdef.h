#ifndef DFONTPREVIEWITEMDEF_H
#define DFONTPREVIEWITEMDEF_H

#include "dfontinfomanager.h"

#include <QMetaType>
#include <QString>
#include <QDebug>

//#define FTM_DEFAULT_PREVIEW_TEXT QString(tr("Don't let your dreams be dreams"))
#define FTM_DEFAULT_PREVIEW_TEXT QString(DTK_WIDGET_NAMESPACE::DApplication::translate("Font", "Don't let your dreams be dreams"))
#define FTM_DEFAULT_PREVIEW_CN_TEXT QString("因理想而出生，为责任而成长")
#define FTM_DEFAULT_PREVIEW_EN_TEXT QString("Don't let your dreams be dreams")
#define FTM_DEFAULT_PREVIEW_LENGTH (15)
#define FTM_DEFAULT_PREVIEW_EN_LENGTH (FTM_DEFAULT_PREVIEW_EN_TEXT.size())
#define FTM_DEFAULT_PREVIEW_DIGIT_TEXT QString("0123456789")
#define FTM_DEFAULT_PREVIEW_FONTSIZE 30
#define FTM_PREVIEW_ITEM_HEIGHT (72+2+10)

typedef enum IconStatus {
    IconNormal = 0,
    IconPress,
    IconHover,
} IconStatus;

struct DFontPreviewItemData {
    DFontInfo fontInfo;       //字体信息
    QString strFontName;      //字体名称
//    QString strFontFileName;  //字体文件名称
//    qint8 iFontSize;          //字体大小
//    QString strFontPreview;   //字体预览内容
    bool isEnabled;           //是否启用
//    bool isPreviewEnabled;    //是否启用(用于预览字体delegate,由于启用/禁用字体不是及时生效,需要该字段保存启用历史状态)
    bool isCollected;         //是否收藏
    bool isChineseFont;       //是否中文字体
    bool isMonoSpace;         //是否等宽字体
    QString strFontId;        //字体唯一id,对应数据库表中的fontId


//    IconStatus collectIconStatus;
    int appFontId;

    DFontPreviewItemData()
    {
        fontInfo.filePath = "";
        fontInfo.familyName = "";
        fontInfo.styleName = "";
        fontInfo.type = "";
        fontInfo.version = "";
        fontInfo.copyright = "";
        fontInfo.description = "";
        fontInfo.sysVersion = "";
        fontInfo.fullname = "";
        fontInfo.psname = "";
        fontInfo.trademark = "";
        fontInfo.isInstalled = false;
        fontInfo.isError = false;
        fontInfo.isSystemFont = true;
        fontInfo.defaultPreview = "";
        fontInfo.previewLang = FONT_LANG_NONE;

        strFontName = "";
//        strFontFileName = "";
//        iFontSize = 0;
//        strFontPreview = "";
        isEnabled = false;
//        isPreviewEnabled = false;
        isCollected = false;
        strFontId = "";
        isChineseFont = false;
        isMonoSpace = false;
        appFontId = -1;

//        collectIconStatus = IconNormal;

    }

    DFontPreviewItemData(const DFontPreviewItemData &other)
    {
        fontInfo.filePath = other.fontInfo.filePath;
        fontInfo.familyName = other.fontInfo.familyName;
        fontInfo.styleName = other.fontInfo.styleName;
        fontInfo.type = other.fontInfo.type;
        fontInfo.version = other.fontInfo.version;
        fontInfo.copyright = other.fontInfo.copyright;
        fontInfo.description = other.fontInfo.description;
        fontInfo.sysVersion = other.fontInfo.sysVersion;
        fontInfo.fullname = other.fontInfo.fullname;
        fontInfo.psname = other.fontInfo.psname;
        fontInfo.trademark = other.fontInfo.trademark;
        fontInfo.isInstalled = other.fontInfo.isInstalled;
        fontInfo.isError = other.fontInfo.isError;
        fontInfo.isSystemFont = other.fontInfo.isSystemFont;
        fontInfo.defaultPreview = other.fontInfo.defaultPreview;
        fontInfo.previewLang = other.fontInfo.previewLang;

        strFontName = other.strFontName;
//        strFontFileName = other.strFontFileName;
//        iFontSize = other.iFontSize;
//        strFontPreview = other.strFontPreview;
        isEnabled = other.isEnabled;
//        isPreviewEnabled = other.isPreviewEnabled;
        isCollected = other.isCollected;
        strFontId = other.strFontId;

        isChineseFont = other.isChineseFont;
        isMonoSpace = other.isMonoSpace;
        appFontId = other.appFontId;

//        collectIconStatus = other.collectIconStatus;
    }

    ~DFontPreviewItemData()
    {
//        qDebug() << __FUNCTION__ << strFontName;
    }

    bool operator==(const DFontPreviewItemData &info)
    {
        return (strFontName == info.strFontName);
        //return (fontInfo == info.fontInfo);
    }
};

Q_DECLARE_METATYPE(DFontPreviewItemData)

struct FontData {
    QString strFontName;     //字体名称 familyName-StyleName
    int fontState;           // ENABLED : 是否启用isEnabled; COLLECTED : 是否收藏isCollected; CHINESED : 是否中文字体isChineseFont; MONOSPACE : 是否等宽字体isMonoSpace; NORMAL/PRESS/HOVER: hover state

    FontData()
    {
        strFontName = "";
        fontState = 0;
    }

    FontData(const FontData &other)
    {
        strFontName = other.strFontName;
        fontState = other.fontState;
    }

    FontData(const QString &_strFontName, bool isEnabled, bool isCollected)
    {
        strFontName = _strFontName;
        setEnabled(isEnabled);
        setCollected(isCollected);
    }

    enum {
        CLEAR = 0,
        ENABLED = 0x01,
        COLLECTED = 0x02,
        CHINESED = 0x04,
        MONOSPACE = 0x08,
        NORMAL = 0x10,
        PRESS = 0x20,
        HOVEER = 0x40,
        HOVERSTATE = 0x30,
    };

    void setEnabled(bool isEnabled)
    {
        fontState = isEnabled ? (fontState | ENABLED) : (fontState & ~ENABLED);
    }

    bool isEnabled()
    {
        return (fontState & ENABLED);
    }

    void setCollected(bool collected)
    {
        fontState = (collected ? (fontState | COLLECTED) : (fontState & ~COLLECTED));
    }

    bool isCollected()
    {
        return (fontState & COLLECTED);
    }

    void setChinese(bool chinese)
    {
        fontState = (chinese ? (fontState | CHINESED) : (fontState & ~CHINESED));
    }

    bool isChinese()
    {
        return (fontState & CHINESED);
    }

    void setMonoSpace(bool isMonospace)
    {
        fontState = (isMonospace ? (fontState | MONOSPACE) : (fontState & ~MONOSPACE));
    }

    bool isMonoSpace()
    {
        return (fontState & MONOSPACE);
    }

    void setHoverState(IconStatus state)
    {
        fontState &= ~HOVERSTATE;
        fontState |= ((state) << 4);
    }

    IconStatus getHoverState()
    {
        return static_cast<IconStatus>((fontState & HOVERSTATE) >> 4);
    }
};

Q_DECLARE_METATYPE(FontData)
#endif  // DFONTPREVIEWITEMDEF_H
