// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDLOG_H
#define DDLOG_H

#include <QDebug>
#include <QLoggingCategory>

// 项目统一的日志分类：分类名必须 org.deepin. 前缀且与 desktop 文件名
// （deepin-font-manager）对应，便于 deepin-debug-config（调试模式）按
// *font-manager* 通配规则统一过滤：
// - fmLog         主程序与共享库代码（libdeepin-font-manager 同时编入主程序与预览插件）
// - fmPreviewLog  文件管理器字体预览插件（dde-file-manager 宿主进程内加载）
inline Q_LOGGING_CATEGORY(fmLog, "org.deepin.font-manager")
inline Q_LOGGING_CATEGORY(fmPreviewLog, "org.deepin.font-manager.preview")

#endif // DDLOG_H
