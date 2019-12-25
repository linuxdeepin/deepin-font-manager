#include "dfontfilewatcher.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>

DFontFileWatcher *DFontFileWatcher::m_pInstance = nullptr;

DFontFileWatcher::DFontFileWatcher(QObject *parent) : QObject(parent)
{

}

// 监控文件或目录
void DFontFileWatcher::addWatchPath(QString path)
{
    qDebug() << QString("Add to watch: %1").arg(path);

    if (m_pInstance == nullptr) {
        m_pInstance = new DFontFileWatcher();
        m_pInstance->m_pSystemWatcher = new QFileSystemWatcher();

        // 连接QFileSystemWatcher的directoryChanged和fileChanged信号到相应的槽
        connect(m_pInstance->m_pSystemWatcher, SIGNAL(directoryChanged(QString)), m_pInstance, SLOT(directoryUpdated(QString)));
        connect(m_pInstance->m_pSystemWatcher, SIGNAL(fileChanged(QString)), m_pInstance, SLOT(fileUpdated(QString)));
    }

    // 添加监控路径
    m_pInstance->m_pSystemWatcher->addPath(path);

    // 如果添加路径是一个目录，保存当前内容列表
    QFileInfo file(path);
    if (file.isDir()) {
        const QDir dirw(path);
        m_pInstance->m_currentContentsMap[path] = dirw.entryList(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
    }
}

void DFontFileWatcher::directoryUpdated(const QString &path)
{
    qDebug() << QString("Directory updated: %1").arg(path);

    // 比较最新的内容和保存的内容找出区别(变化)
    QStringList currEntryList = m_currentContentsMap[path];
    const QDir dir(path);

    QStringList newEntryList = dir.entryList(QDir::NoDotAndDotDot  | QDir::AllDirs | QDir::Files, QDir::DirsFirst);

    QSet<QString> newDirSet = QSet<QString>::fromList(newEntryList);
    QSet<QString> currentDirSet = QSet<QString>::fromList(currEntryList);

    // 添加了文件
    QSet<QString> newFiles = newDirSet - currentDirSet;
    QStringList newFile = newFiles.toList();

    // 文件已被移除
    QSet<QString> deletedFiles = currentDirSet - newDirSet;
    QStringList deleteFile = deletedFiles.toList();

    // 更新当前设置
    m_currentContentsMap[path] = newEntryList;

    if (!newFile.isEmpty() && !deleteFile.isEmpty()) {
        // 文件/目录重命名
        if ((newFile.count() == 1) && (deleteFile.count() == 1)) {
            qDebug() << QString("File Renamed from %1 to %2").arg(deleteFile.first()).arg(newFile.first());
        }
    } else {
        // 添加新文件/目录至Dir
        if (!newFile.isEmpty()) {
            qDebug() << "New Files/Dirs added: " << newFile;

            foreach (QString file, newFile) {
                // 处理操作每个新文件....
            }
        }

        // 从Dir中删除文件/目录
        if (!deleteFile.isEmpty()) {
            qDebug() << "Files/Dirs deleted: " << deleteFile;

            foreach (QString file, deleteFile) {
                // 处理操作每个被删除的文件....
            }
        }
    }
}

void DFontFileWatcher::fileUpdated(const QString &path)
{
    QFileInfo file(path);
    QString strPath = file.absolutePath();
    QString strName = file.fileName();

    qDebug() << QString("The file %1 at path %2 is updated").arg(strName).arg(strPath);
}
