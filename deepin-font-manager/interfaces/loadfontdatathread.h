#ifndef LOADFONTDATATHREAD_H
#define LOADFONTDATATHREAD_H
#include "dfontinfomanager.h"
#include "dfontpreviewlistdatathread.h"
#include "dfmdbmanager.h"
#include "dcopyfilesmanager.h"

#include <QThread>

class LoadFontDataThread : public QThread
{
    Q_OBJECT
public:
    explicit LoadFontDataThread(QList<QMap<QString, QString>> &list);

protected:
    void run();


signals:
    void dataLoadFinish(QList<DFontPreviewItemData> &);

private:
    QList<DFontPreviewItemData> m_delFontInfoList;
    QList<DFontPreviewItemData> m_startModelList;
    QList<QMap<QString, QString>> m_list;

    QThread mThread;
};



#endif // LOADFONTDATATHREAD_H
