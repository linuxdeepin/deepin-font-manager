#ifndef DFONTPREVIEWITEMDEF_H
#define DFONTPREVIEWITEMDEF_H

#include "dfontinfomanager.h"

#include <QMetaType>
#include <QString>

#define FTM_DEFAULT_PREVIEW_TEXT QString("Don't let your dreams be dreams")

struct DFontPreviewItemData {
    DFontInfo *pFontInfo;    //字体信息
    QString strFontName;     //字体名称
    QString strFontPreview;  //字体预览内容
    bool bEnabled;           //是否启用
    bool bCollected;         //是否收藏
};

Q_DECLARE_METATYPE(DFontPreviewItemData)

#endif  // DFONTPREVIEWITEMDEF_H
