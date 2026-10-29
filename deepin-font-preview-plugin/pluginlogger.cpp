// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginlogger.h"

#include "ddlog.h"

#include <QDebug>

#ifdef DTKCORE_CLASS_DConfigFile
#include <DConfig>
#include <dtkcore_global.h>
#endif

PluginLogController::PluginLogController(QObject *parent)
    : QObject(parent)
{
#ifdef DTKCORE_CLASS_DConfigFile
    // 插件运行在 dde-file-manager 宿主进程内，日志规则来源与主程序保持一致：
    // DConfig org.deepin.font-manager 的 log_rules，监听变更实现运行中动态调整
    Dtk::Core::DConfig *config =
        Dtk::Core::DConfig::create("org.deepin.font-manager", "org.deepin.font-manager", this);
    if (config) {
        applyRules(config->value("log_rules").toString());
        connect(config, &Dtk::Core::DConfig::valueChanged, this, [this, config](const QString &key) {
            if (key == "log_rules") {
                applyRules(config->value(key).toString());
            }
        });
    }
#endif
}

void PluginLogController::applyRules(const QString &rules)
{
    // 只调整本插件声明分类的级别开关（fmLog 供编入插件的共享库代码使用）
    const QStringList items = rules.split(';', Qt::SkipEmptyParts);
    applyCategory(const_cast<QLoggingCategory &>(fmLog()), items);
    applyCategory(const_cast<QLoggingCategory &>(fmPreviewLog()), items);
}

void PluginLogController::applyCategory(QLoggingCategory &category, const QStringList &items)
{
    struct LevelSpec
    {
        QtMsgType type;
        const char *key;
    };
    const LevelSpec levels[] = {
        { QtDebugMsg, "debug" },
        { QtInfoMsg, "info" },
        { QtWarningMsg, "warning" }
    };

    const QString name = QString::fromLatin1(category.categoryName());
    for (const LevelSpec &level : levels) {
        // 与 Qt 过滤规则语义一致：取最后一条命中本分类的对应级别规则
        int enable = -1;
        for (const QString &item : items) {
            const QString rule = item.trimmed();
            const int eq = rule.lastIndexOf('=');
            if (eq <= 0)
                continue;
            const QString value = rule.mid(eq + 1);
            if (value != "true" && value != "false")
                continue;
            QString pattern = rule.left(eq);
            const int dot = pattern.lastIndexOf('.');
            if (dot < 0)
                continue;
            if (pattern.mid(dot + 1) != level.key)
                continue;
            pattern = pattern.left(dot);
            // 通配模式（如 *font-manager*）按"包含"匹配，与 QLoggingRule 行为一致
            if (pattern.contains('*')) {
                pattern.remove('*');
                if (!pattern.isEmpty() && !name.contains(pattern))
                    continue;
            } else if (name != pattern) {
                continue;
            }
            enable = (value == "true") ? 1 : 0;
        }
        if (enable >= 0)
            category.setEnabled(level.type, enable == 1);
    }
}
