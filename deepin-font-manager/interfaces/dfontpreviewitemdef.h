#ifndef DFONTPREVIEWITEMDEF_H
#define DFONTPREVIEWITEMDEF_H

#include <QMetaType>
#include <QString>

struct DFontPreviewItemData {
    QString strFontName;     //字体名称
    QString strFontPreview;  //字体预览内容
    bool bEnabled;           //是否启用
    bool bCollected;         //是否收藏
};

Q_DECLARE_METATYPE(DFontPreviewItemData)

#endif  // DFONTPREVIEWITEMDEF_H
