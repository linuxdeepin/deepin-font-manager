// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINLOGGER_H
#define PLUGINLOGGER_H

#include <QObject>
#include <QLoggingCategory>
#include <QStringList>

/**
 * @brief 预览插件动态日志控制器（调试模式适配）
 *
 * 本插件被 dde-file-manager 宿主进程加载，运行在宿主进程内：
 * - 不能调用 QLoggingCategory::setFilterRules（那是进程级全局规则，会覆盖宿主
 *   自身的调试模式日志规则），因此仅通过 QLoggingCategory::setEnabled 调整本
 *   插件声明分类（fmLog / fmPreviewLog）的各级别开关；
 * - 规则来源与主程序一致：DConfig org.deepin.font-manager 的 log_rules 配置项，
 *   监听 valueChanged 实现运行中动态调整。
 *
 * 控制器对象需存活至插件卸载（作为插件实例成员持有即可）。
 */
class PluginLogController : public QObject
{
public:
    explicit PluginLogController(QObject *parent = nullptr);

private:
    void applyRules(const QString &rules);
    void applyCategory(QLoggingCategory &category, const QStringList &items);
};

#endif // PLUGINLOGGER_H
