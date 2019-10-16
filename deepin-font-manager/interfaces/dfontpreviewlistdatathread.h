#ifndef DFONTPREVIEWLISTDATATHREAD_H
#define DFONTPREVIEWLISTDATATHREAD_H

#include "dfmdbmanager.h"
#include "dfontinfomanager.h"

#include <QThread>

class DFontPreviewListDataThread : public QObject
{
    Q_OBJECT
public:
    static DFontPreviewListDataThread *instance();

    DFontPreviewListDataThread();
    virtual ~DFontPreviewListDataThread();

    //从fontconfig配置文件同步字体启用/禁用状态数据
    void syncFontEnableDisableStatusData(QStringList disableFontPathList);

    void refreshFontListData(bool isStartup = false);

    void insertFontItemData(QString filePath,
                            int index,
                            QStringList chineseFontPathList,
                            QStringList monoSpaceFontPathList);

    QList<DFontPreviewItemData> getFontModelList();

signals:
    void resultReady();

protected slots:
    void doWork();

protected:
    QThread *mThread;
    DFontPreviewListDataThread *mFunc;

    DFMDBManager *m_dbManager;
    QList<DFontPreviewItemData> m_fontModelList;
};

#endif // DFONTPREVIEWLISTDATATHREAD_H
