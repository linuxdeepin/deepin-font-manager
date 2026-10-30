// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logger.h"

#include "ddlog.h"

#include <QLoggingCategory>

#ifdef DTKCORE_CLASS_DConfigFile
#include <DConfig>
#endif

DCORE_USE_NAMESPACE

MLogger::MLogger(QObject *parent)
    : QObject(parent)
    , m_rules("")
#ifdef DTKCORE_CLASS_DConfigFile
    , m_config(nullptr)
#endif
{
    QByteArray logRules = qgetenv("QT_LOGGING_RULES");
    // qunsetenv 之前一定不要有任何日志打印，否则取消环境变量设置不会生效（大坑）
    qunsetenv("QT_LOGGING_RULES");

    // 环境变量规则兜底（DConfig 不可用时仍可通过环境变量手动调整级别）
    m_rules = logRules;

#ifdef DTKCORE_CLASS_DConfigFile
    // dconfig 规则：与 deepin-debug-config 调试模式脚本写入的 log_rules 联动
    m_config = DConfig::create("org.deepin.font-manager", "org.deepin.font-manager");
    logRules = m_config->value("log_rules").toByteArray();
    appendRules(logRules);
    setRules(m_rules);

    // 监听 dconfig 变更，实现运行中动态调整（对象需存活整个 main 生命周期）
    connect(m_config, &DConfig::valueChanged, this, [this](const QString &key) {
        if (key == "log_rules") {
            setRules(m_config->value(key).toByteArray());
        }
    });
#else
    setRules(m_rules);
#endif
}

MLogger::~MLogger()
{
#ifdef DTKCORE_CLASS_DConfigFile
    if (m_config) {
        m_config->deleteLater();
    }
#endif
}

void MLogger::setRules(const QString &rules)
{
    auto tmpRules = rules;
    // Qt 限制：setFilterRules 的分隔符必须是 \n，不能用 ;
    m_rules = tmpRules.replace(";", "\n");
    QLoggingCategory::setFilterRules(m_rules);
}

void MLogger::appendRules(const QString &rules)
{
    QString tmpRules = rules;
    tmpRules = tmpRules.replace(";", "\n");
    auto tmplist = tmpRules.split('\n');
    for (int i = 0; i < tmplist.count(); i++)
        if (m_rules.contains(tmplist.at(i))) {
            tmplist.removeAt(i);
            i--;
        }
    if (tmplist.isEmpty())
        return;
    m_rules.isEmpty() ? m_rules = tmplist.join("\n")
                      : m_rules += "\n" + tmplist.join("\n");
}
