#ifndef DFONTPREVIEWITEMDEF_H
#define DFONTPREVIEWITEMDEF_H

#include "dfontinfomanager.h"

#include <QMetaType>
#include <QString>
#include <QDebug>
//#include <DApplication>

//#define FTM_DEFAULT_PREVIEW_TEXT QString(tr("Don't let your dreams be dreams"))
#define FTM_DEFAULT_PREVIEW_TEXT  QString(DApplication::translate("Font", "Don't let your dreams be dreams"))
#define FTM_DEFAULT_PREVIEW_CN_TEXT QString("因理想而出生，为责任而成长")
#define FTM_DEFAULT_PREVIEW_EN_TEXT QString("Don't let your dreams be dreams")
#define FTM_DEFAULT_PREVIEW_DIGIT_TEXT QString("0123456789")
#define FTM_DEFAULT_PREVIEW_FONTSIZE 30
#define FTM_PREVIEW_ITEM_HEIGHT (72+2+10)

typedef enum IconStatus {
    IconHover = 0,
    IconPress,
    IconNormal,
} IconStatus;

struct DFontPreviewItemData {
    DFontInfo fontInfo;       //字体信息
    QString strFontName;      //字体名称
    QString strFontFileName;  //字体文件名称
    int iFontSize;            //字体大小
//    QString strFontPreview;   //字体预览内容
    bool isEnabled;           //是否启用
    bool isPreviewEnabled;    //是否启用(用于预览字体delegate,由于启用/禁用字体不是及时生效,需要该字段保存启用历史状态)
    bool isCollected;         //是否收藏
    QString strFontId;        //字体唯一id,对应数据库表中的fontId

    bool isChineseFont;       //是否中文字体
    bool isMonoSpace;         //是否等宽字体
    bool isCanDisable;
    IconStatus collectIconStatus;

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
        fontInfo.specialPreview = false;

        strFontName = "";
        strFontFileName = "";
        iFontSize = 0;
//        strFontPreview = "";
        isEnabled = false;
        isPreviewEnabled = false;
        isCollected = false;
        strFontId = "";
        isCanDisable = true;
        isChineseFont = false;
        isMonoSpace = false;

        collectIconStatus = IconNormal;
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
        fontInfo.fullname = other.fontInfo.fullname;
        fontInfo.psname = other.fontInfo.psname;
        fontInfo.trademark = other.fontInfo.trademark;
        fontInfo.defaultPreview = other.fontInfo.defaultPreview;
        fontInfo.specialPreview = other.fontInfo.specialPreview;

        strFontName = other.strFontName;
        strFontFileName = other.strFontFileName;
        iFontSize = other.iFontSize;
//        strFontPreview = other.strFontPreview;
        isEnabled = other.isEnabled;
        isPreviewEnabled = other.isPreviewEnabled;
        isCollected = other.isCollected;
        strFontId = other.strFontId;

        isChineseFont = other.isChineseFont;
        isMonoSpace = other.isMonoSpace;
        isCanDisable = other.isCanDisable;

        collectIconStatus = other.collectIconStatus;
    }

    ~DFontPreviewItemData()
    {
//        qDebug() << __FUNCTION__ << strFontName;
    }

    bool operator==(const DFontPreviewItemData &info)
    {
        return (fontInfo == info.fontInfo);
    }
};

Q_DECLARE_METATYPE(DFontPreviewItemData)

#endif  // DFONTPREVIEWITEMDEF_H
