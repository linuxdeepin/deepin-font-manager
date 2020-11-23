#include "loadfontdatathread.h"
#include "dfontpreviewlistview.h"
#include <QTimer>

LoadFontDataThread::LoadFontDataThread(QList<QMap<QString, QString> > &list)
{
    m_list = list;
    moveToThread(&mThread);
    QObject::connect(&mThread, &QThread::started, this, &LoadFontDataThread::doWork);

    QTimer::singleShot(3, [this] {
        mThread.start();
    });
}

void LoadFontDataThread::run()
{

}


void LoadFontDataThread::doWork()
{
    qDebug() << QThread::currentThreadId() << __func__ << "------------";
    SignalManager::m_isOnLoad = true;
    DFontPreviewListDataThread *thread = DFontPreviewListDataThread::instance();
    QList<DFontPreviewItemData> fontinfoList = DFMDBManager::instance()->getFontInfo(m_list, &m_delFontInfoList);

    thread->m_fontModelList.append(fontinfoList);

    for (DFontPreviewItemData &itemData : m_delFontInfoList) {
        //如果字体文件已经不存在，则从t_manager表中删除
        //删除字体之前启用字体，防止下次重新安装后就被禁用
        thread->getView()->enableFont(itemData.fontInfo.filePath);
        DFMDBManager::instance()->deleteFontInfo(itemData);
    }

    DFMDBManager::instance()->commitDeleteFontInfo();
    thread->getView()->enableFonts();

    foreach (auto it, m_startModelList) {
        thread->addPathWatcher(it.fontInfo.filePath);
    }
    m_delFontInfoList.clear();
    SignalManager::m_isOnLoad = false;
    SignalManager::m_isDataLoadFinish = true;

    emit dataLoadFinish(fontinfoList);
}
