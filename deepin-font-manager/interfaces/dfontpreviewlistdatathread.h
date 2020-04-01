#ifndef DFONTPREVIEWLISTDATATHREAD_H
#define DFONTPREVIEWLISTDATATHREAD_H

#include "dfmdbmanager.h"
#include "dfontinfomanager.h"

#include <QThread>
#include <QMutex>
#include <QMutexLocker>

#include <QMetaType>

class QFileSystemWatcher;
class DFontPreviewListView;
class DFontPreviewListDataThread : public QObject
{
    Q_OBJECT
public:
    static DFontPreviewListDataThread *instance(DFontPreviewListView *view);
    static DFontPreviewListDataThread *instance();

    DFontPreviewListDataThread(DFontPreviewListView *view);
    virtual ~DFontPreviewListDataThread();

    //从fontconfig配置文件同步字体启用/禁用状态数据
    void syncFontEnableDisableStatusData(QStringList disableFontPathList);

    void refreshFontListData(bool isStartup = false, QStringList list = QStringList());
    void removeFontData(const DFontPreviewItemData &removeItemData);

    void insertFontItemData(QString filePath,
                            int index,
                            QStringList chineseFontPathList,
                            QStringList monoSpaceFontPathList,
                            bool isStartup = false);

    QList<DFontPreviewItemData> getFontModelList();
    QList<DFontPreviewItemData> getDiffFontModelList() const;
    void setMutex(QMutex *mutex);
    void forceDeleteFiles(const QStringList &files);

signals:
    void resultReady();

protected slots:
    void doWork();
    void initFileSystemWatcher();

public slots:
    void updateChangedFile(const QString &path);
    void updateChangedDir(const QString &path);
    void addPathWatcher(const QString &path);
    void removePathWatcher(const QString &path);
    void onFileChanged(const QStringList &files);

protected:
    QThread mThread;

    DFMDBManager *m_dbManager {nullptr};
    QList<DFontPreviewItemData> m_fontModelList;
    QList<DFontPreviewItemData> m_diffFontModelList;
    DFontPreviewListView *m_view;
    QFileSystemWatcher *m_fsWatcher;
    QMutex *m_mutex;
};

#endif // DFONTPREVIEWLISTDATATHREAD_H
