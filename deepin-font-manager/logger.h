// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FONTMANAGER_LOGGER_H
#define FONTMANAGER_LOGGER_H

#include <QObject>

#include <dtkcore_global.h>

#ifdef DTKCORE_CLASS_DConfigFile
DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE
#endif

/**
 * @brief 主程序动态日志规则管理器（调试模式适配）
 *
 * 参考 deepin-reader 的 MLogger 实现：读取 DConfig org.deepin.font-manager 的
 * log_rules 配置项并应用到 Qt 日志过滤规则，同时监听 valueChanged，配合
 * deepin-debug-config 调试模式实现日志级别运行中动态切换。DConfig 不可用时
 * 退回 QT_LOGGING_RULES 环境变量规则兜底。
 *
 * 注意：MLogger 实例必须先于任何日志打印构造（qunsetenv 之前不能有日志输出），
 * 且实例需存活整个 main 生命周期，否则 DConfig 变更监听会随对象析构而失效。
 */
class MLogger : public QObject
{
    Q_OBJECT
public:
    explicit MLogger(QObject *parent = nullptr);
    ~MLogger() override;

    inline QString rules() const { return m_rules; }
    void setRules(const QString &rules);

private:
    void appendRules(const QString &rules);

private:
    QString m_rules;
#ifdef DTKCORE_CLASS_DConfigFile
    Dtk::Core::DConfig *m_config = nullptr;
#endif
};

#endif // FONTMANAGER_LOGGER_H
