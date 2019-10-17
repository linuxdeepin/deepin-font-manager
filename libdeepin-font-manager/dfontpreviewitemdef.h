#ifndef DFONTPREVIEWITEMDEF_H
#define DFONTPREVIEWITEMDEF_H

#include "dfontinfomanager.h"

#include <QMetaType>
#include <QString>

#define FTM_DEFAULT_PREVIEW_TEXT QString("Don't let your dreams be dreams")
#define FTM_DEFAULT_PREVIEW_FONTSIZE 30
#define FTM_PREVIEW_ITEM_HEIGHT (72+2)

typedef enum IconStatus
{
    IconHover = 0,
    IconPress,
    IconNormal
} IconStatus;

struct DFontPreviewItemData {
    DFontInfo fontInfo;       //字体信息
    QString strFontName;      //字体名称
    QString strFontFileName;  //字体文件名称
    int iFontSize;            //字体大小
    QString strFontPreview;   //字体预览内容
    bool isEnabled;           //是否启用
    bool isPreviewEnabled;    //是否启用(用于预览字体delegate,由于启用/禁用字体不是及时生效,需要该字段保存启用历史状态)
    bool isCollected;         //是否收藏
    QString strFontId;        //字体唯一id,对应数据库表中的fontId

    bool isChineseFont;       //是否中文字体
    bool isMonoSpace;         //是否等宽字体

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
        fontInfo.isInstalled = false;
        fontInfo.isError = false;
        fontInfo.isSystemFont = true;

        strFontName = "";
        strFontFileName = "";
        iFontSize = 0;
        strFontPreview = "";
        isEnabled = false;
        isPreviewEnabled = false;
        isCollected = false;
        strFontId = "";

        isChineseFont = false;
        isMonoSpace = false;

        collectIconStatus = IconNormal;
    }
};

Q_DECLARE_METATYPE(DFontPreviewItemData)

#endif  // DFONTPREVIEWITEMDEF_H
