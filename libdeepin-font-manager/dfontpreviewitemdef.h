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

/*************************************************************************
 <Enum>          IconStatus
 <Description>   收藏图标状态
 <Author>
 <Value>
    <Value1>     IconNormal=0       Description:常规状态
    <Value2>     IconPress          Description:press状态
    <Value2>     IconHover          Description:hover状态
 <Note>          null
*************************************************************************/
typedef enum IconStatus {
    IconNormal = 0,
    IconPress,
    IconHover,
} IconStatus;

/*************************************************************************
 <Struct>        DFInstallErrorItemModel
 <Description>   ItemModel信息结构体
 <Attribution>
    <Attr1>      fontInfo         Description:字体信息
    <Attr2>      strFontName      Description:字体名称
    <Attr3>      isEnabled        Description:是否启用
    <Attr4>      isCollected      Description:是否收藏
    <Attr5>      isChineseFont    Description:是否中文字体
    <Attr6>      isMonoSpace      Description:是否等宽字体
    <Attr7>      appFontId        Description:字体唯一id,对应数据库表中的fontId

    <Attr8>      filePath         Description:字体路径
    <Attr9>      familyName       Description:字体组名
    <Attr10>      styleName       Description:字体风格
    <Attr11>      type            Description:字体类型
    <Attr12>      version         Description:版本信息
    <Attr13>      copyright       Description:版权信息
    <Attr14>      description     Description:字体描述内容
    <Attr15>      sysVersion      Description:系统版本
    <Attr16>      fullname        Description:字体全名
    <Attr17>      psname          Description:字体PS名称
    <Attr18>      trademark       Description:字体商标信息
    <Attr19>      isInstalled     Description:是否已安装
    <Attr20>      isSystemFont    Description:是否为系统字体
    <Attr21>      defaultPreview  Description:默认预览内容
    <Attr22>      previewLang     Description:预览语言
 <Note>          null
*************************************************************************/
struct DFontPreviewItemData {
    DFontInfo fontInfo;
    QString strFontName;
    bool isEnabled;
    bool isCollected;
    bool isChineseFont;
    bool isMonoSpace;
    QString strFontId;
    int appFontId;

    //初始化
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
        isEnabled = false;
        isCollected = false;
        strFontId = "";
        isChineseFont = false;
        isMonoSpace = false;
        appFontId = -1;
    }

    //拷贝构造函数
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
        isEnabled = other.isEnabled;
        isCollected = other.isCollected;
        strFontId = other.strFontId;

        isChineseFont = other.isChineseFont;
        isMonoSpace = other.isMonoSpace;
        appFontId = other.appFontId;
    }
    //结构体析构函数
    ~DFontPreviewItemData()
    {

    }
    //strFontName字段赋值
    bool operator==(const DFontPreviewItemData &info)
    {
        return (strFontName == info.strFontName);
    }
};

Q_DECLARE_METATYPE(DFontPreviewItemData)

/*************************************************************************
 <Struct>        FontData
 <Description>   字体信息结构体
 <Attribution>
    <Attr1>     strFontName           Description:字体名称 familyName-StyleName
    <Attr2>     fontState             Description:字体状态
 <Note>          null
*************************************************************************/
struct FontData {
    QString strFontName;
    int fontState;
    FontData()
    {
        strFontName = "";
        fontState = 0;
    }
    //拷贝构造
    FontData(const FontData &other)
    {
        strFontName = other.strFontName;
        fontState = other.fontState;
    }
    //重载拷贝构造
    FontData(const QString &_strFontName, bool isEnabled, bool isCollected)
    {
        strFontName = _strFontName;
        setEnabled(isEnabled);
        setCollected(isCollected);
    }
    /*************************************************************************
     <Enum>          枚举
     <Description>   字体状态枚举
     <Author>
     <Value>
        <Value1>     CLEAR               Description:无状态
        <Value2>     ENABLED             Description:是否启用
        <Value3>     COLLECTED           Description:是否收藏
        <Value4>     CHINESED            Description:是否中文字体
        <Value5>     MONOSPACE           Description:是否等宽字体

        <Value6>     NORMAL              Description:常规
        <Value7>     PRESS               Description:press状态
        <Value8>     HOVEER              Description:hover状态
        <Value9>     HOVERSTATE          Description:press&hover状态
     <Note>          null
    *************************************************************************/
    //构造
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
    //更新启用状态
    void setEnabled(bool isEnabled)
    {
        fontState = isEnabled ? (fontState | ENABLED) : (fontState & ~ENABLED);
    }
    //返回启用状态
    bool isEnabled()
    {
        return (fontState & ENABLED);
    }
    //更新收藏状态
    void setCollected(bool collected)
    {
        fontState = (collected ? (fontState | COLLECTED) : (fontState & ~COLLECTED));
    }
    //返回收藏状态
    bool isCollected()
    {
        return (fontState & COLLECTED);
    }
    //更新是否为中文字体状态
    void setChinese(bool chinese)
    {
        fontState = (chinese ? (fontState | CHINESED) : (fontState & ~CHINESED));
    }
    //返回是否为中文字体状态
    bool isChinese()
    {
        return (fontState & CHINESED);
    }
    //更新是否为等宽字体状态
    void setMonoSpace(bool isMonospace)
    {
        fontState = (isMonospace ? (fontState | MONOSPACE) : (fontState & ~MONOSPACE));
    }
    //返回是否为等宽字体状态
    bool isMonoSpace()
    {
        return (fontState & MONOSPACE);
    }
    //更新hover或press状态
    void setHoverState(IconStatus state)
    {
        fontState &= ~HOVERSTATE;
        fontState |= ((state) << 4);
    }
    //返回hover或press状态
    IconStatus getHoverState()
    {
        return static_cast<IconStatus>((fontState & HOVERSTATE) >> 4);
    }
};

Q_DECLARE_METATYPE(FontData)
#endif  // DFONTPREVIEWITEMDEF_H
