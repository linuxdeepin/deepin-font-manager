// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fontpreview.h"
#include "pluginlogger.h"

#include <QtPlugin>

#ifdef DFM_BASE
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>
#else
#include "dde-file-manager/dfmfilepreviewplugin.h"
#endif

// Qt6 moc（6.8+）解析 Q_PLUGIN_METADATA 的 IID 时只展开本文件可见的宏，
// 不跟随头文件，因此 IID 标识符必须在同文件内可解析，否则 moc 报
// "Parse error at IID"（插件元数据被忽略会导致宿主无法匹配插件）。
// - FilePreviewFactoryInterface_iid：与 dfm-base 头文件中的定义完全一致，
//   重复定义同值无副作用；
// - DFMFilePreviewFactoryInterface_iid：仅保证 moc 能解析（V25/Qt6 走
//   DFM_BASE 分支，legacy 分支不会参与编译；若 legacy 头文件存在，
//   其真实定义通过 #ifndef 优先生效）。
#ifndef FilePreviewFactoryInterface_iid
#define FilePreviewFactoryInterface_iid "com.deepin.filemanager.FilePreviewFactoryInterface_iid"
#endif
#ifndef DFMFilePreviewFactoryInterface_iid
#define DFMFilePreviewFactoryInterface_iid "com.deepin.filemanager.DFMFilePreviewFactoryInterface_iid"
#endif

#ifdef DFM_BASE
DFMBASE_BEGIN_NAMESPACE
#else
DFM_BEGIN_NAMESPACE
#endif

#ifdef DFM_BASE
class TextPreviewPlugin : public AbstractFilePreviewPlugin
#else
class TextPreviewPlugin : public DFMFilePreviewPlugin
#endif
{
    Q_OBJECT
#ifdef DFM_BASE
    Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid FILE "deepin-font-preview-plugin.json")
#else
    Q_PLUGIN_METADATA(IID DFMFilePreviewFactoryInterface_iid FILE "deepin-font-preview-plugin.json")
#endif

public:
#ifdef DFM_BASE
    DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &) override {
        return new FontPreview();
    }
#else
    DFMFilePreview *create(const QString &key) Q_DECL_OVERRIDE {
        Q_UNUSED(key)

        return new FontPreview();
    }
#endif

private:
    // 调试模式适配：插件随宿主进程加载，日志控制器随插件实例存活，
    // 监听 DConfig 变更动态调整本插件分类的日志级别
    PluginLogController m_logController;
};


#ifdef DFM_BASE
DFMBASE_END_NAMESPACE
#else
DFM_END_NAMESPACE
#endif

#include "main.moc"
