#ifndef DFONTFILEWATCHER_H
#define DFONTFILEWATCHER_H

#include "dfontpreviewlistdatathread.h"
#include <QObject>
#include <QMap>
#include <QFileSystemWatcher>

class DFontFileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit DFontFileWatcher(QObject *parent = nullptr);
    static void addWatchPath(QString path);

private:
    static DFontFileWatcher *m_pInstance; // 单例
    QFileSystemWatcher *m_pSystemWatcher {nullptr};  // QFileSystemWatcher变量
    QMap<QString, QStringList> m_currentContentsMap; // 当前每个监控的内容目录列表

signals:

public slots:
    void directoryUpdated(const QString &path);  // 目录更新时调用，path是监控的路径
    void fileUpdated(const QString &path);   // 文件被修改时调用，path是监控的路径
};

#endif // DFONTFILEWATCHER_H
