#ifndef DFONTPREVIEWLISTDATATHREAD_H
#define DFONTPREVIEWLISTDATATHREAD_H

#include "dfontpreviewitemdef.h"
#include "dfontmanager.h"

#include <QThread>
#include <QMutex>

/*************************************************************************
 <Class>         DFontPreviewListDataThread
 <Description>   description
 <Author>
 <Note>          null
*************************************************************************/

class QFileSystemWatcher;
class DFontPreviewListView;
class DFontManager;
class DFMDBManager;
class DFontInfoManager;
class DFontPreviewListDataThread : public QObject
{
    Q_OBJECT
public:
    static DFontPreviewListDataThread *instance(DFontPreviewListView *view);
    static DFontPreviewListDataThread *instance();

    DFontPreviewListDataThread(DFontPreviewListView *view);
    virtual ~DFontPreviewListDataThread();

    //从fontconfig配置文件同步字体启用/禁用状态数据
    void syncFontEnableDisableStatusData(const QStringList &disableFontPathList);
    //刷新字体列表的数据
    void refreshFontListData(bool isStartup = false, const QStringList &list = QStringList());
    //从字体信息链表中删除需要删除的项
    void removeFontData(const DFontPreviewItemData &removeItemData);
    //将需要添加项的字体数据收集放人list中
    int insertFontItemData(const QString &filePath,
                           int index,
                           const QStringList &chineseFontPathList,
                           const QStringList &monoSpaceFontPathList,
                           bool isStartup = false, bool isEnabled = true);
    //获取当前列表所有项的数据链表
    QList<DFontPreviewItemData> getFontModelList();
    //更新itemDataList的itemData状态
    void updateItemStatus(int index, const DFontPreviewItemData &itemData);
    //更新字体信息中的fontid
    void updateFontId(const DFontPreviewItemData &itemData, int id);
    //通过DFontData信息获取DFontPreviewItemData
    inline static DFontPreviewItemData getFontData(const FontData &fontData)
    {
        DFontPreviewItemData itemdata;
        itemdata.fontData = fontData;
//        qDebug() << __FUNCTION__ << fontData.strFontName;
        int index = m_fontModelList.indexOf(itemdata);
        if (index > -1) {
//            qDebug() << __FUNCTION__ << fontData.strFontName << index;
            return m_fontModelList.at(index);
        } else {
            qDebug() << __FUNCTION__ << " not found " << fontData.getFontType() << fontData.strFontName << DFontPreviewListDataThread::instance()->getFontModelList().size();
        }
        return itemdata;
    }
    //检测是否要弹出字体验证框，存在重复安装字体，系统字体时，损坏字体时弹出字体验证框
    bool ifNeedShowExceptionWindow(bool needSkipException = false);
    //判断当前字体是否为系统字体
    bool isSystemFont(const DFontInfo &f);

private:
    inline QString getFamilyName(const DFontInfo &fontInfo)
    {
        QString familyName = (fontInfo.familyName.isEmpty() || fontInfo.familyName.contains(QChar('?'))) ? fontInfo.fullname : fontInfo.familyName;
        return familyName;
    }
    inline QString getFamilyStyleName(const DFontInfo &fontInfo)
    {
        QString familyName = (fontInfo.familyName.isEmpty() || fontInfo.familyName.contains(QChar('?'))) ? fontInfo.fullname : fontInfo.familyName;
        return (familyName + fontInfo.styleName);
    }
    //获取所有的系统字体信息
    void getAllSysFonts();
    //字体文件过滤器，过滤后得到需要新安装的字体，重复安装字体，损毁字体，系统字体,以及字体验证框弹出时安装的字体
    void verifyFontFiles();
    //批量安装
    void batchInstall();
    //获取新增字体文件
    void updateInstalledFontList(const QStringList &filesList);

signals:
    //发出删除字体文件请求
    void requestDeleted(QStringList &files);
    //发出添加字体文件请求
    void requestAdded(const QStringList &files, bool isFirstInstall = false);
    //请求删除字体文件
    void requestForceDeleteFiles(QStringList &files);
    //请求重装继续
    void requestBatchReInstallContinue();
    //请求批量移除文件监视器
    void requestRemoveFileWatchers(const QStringList &files);
    //添加特定文件夹的文件监视器
    void requestAutoDirWatchers();
    //发出导出字体的请求
    void requestExportFont(QStringList &files);
    //导出字体后，请求提示信息
    void exportFontFinished(int count);
    //请求安装字体
    void requestBatchInstall(const QStringList &fontList);
    //请求重安装字体
    void requestBatchReInstall(const QStringList &fontList);
    //安装字体结束
    void installFinished();
    //重新安装字体结束
    void reInstallFinished();
    //取消重复安装
    void requestCancelReinstall();
    //请求弹“已安装xxx字体”消息框
    void requstShowInstallToast(int fontCnt);

protected slots:
    //线程函数
    void doWork();
    //初始化系统文件监视器
    void initFileSystemWatcher();

public slots:
    //文件修改后触发函数
    void updateChangedFile(const QString &path);
    //文件夹修改后触发函数
    void updateChangedDir();
    //添加文件监视器
    void addPathWatcher(const QString &path);
    //移除特定路径的文件监视器
    void removePathWatcher(const QString &path);
    //文件删除响应函数
    void onFileDeleted(QStringList &files);
    //增加文件响应函数
    void onFileAdded(const QStringList &files);
    //强制删除文件
    void forceDeleteFiles(QStringList &files);
    //批量移除文件监视器
    void onRemoveFileWatchers(const QStringList &files);
    //添加特定文件夹的文件监视器
    void onAutoDirWatchers();
    //导出字体文件
    void onExportFont(QStringList &fontList);
    //字体安装后的处理函数
    void onInstallFinished(int state, const QStringList &fileList);
    //字体重新安装后的处理函数
    void onReInstallFinished(int state, const QStringList &fileList);
    //批量安装处理函数
    void onBatchInstall(const QStringList &fontList);
    //字体验证框弹出时在文件管理器进行安装
    void batchHalfwayInstall(const QStringList &filelist);
    //重装验证页面，继续按钮处理函数-继续批量安装
    void batchReInstallContinue();
    //批量重新安装槽函数
    void onBatchReInstall(const QStringList &fontList);
    //取消安装槽函数
    void onCancelReinstall();

public:
    QStringList m_allFontPathList;
    QStringList m_chineseFontPathList;
    QStringList m_monoSpaceFontPathList;
    static QList<DFontPreviewItemData> m_fontModelList;
    QList<DFontPreviewItemData> m_delFontInfoList;

    //安装/重复安装
    QStringList m_SystemFontsList;

    QStringList m_installFiles;
    QStringList m_installedFiles;
    QStringList m_newInstallFiles;
    QStringList m_damagedFiles;
    QStringList m_systemFiles;
    QStringList m_outfileList;
    QStringList m_errorList;

    QStringList m_installedFontsFamilyname;
    QStringList m_halfInstalledFiles;
    QStringList m_newHalfInstalledFiles;
    QStringList m_oldHalfInstalledFiles;
    DFontManager::Type m_installState {DFontManager::Install};
    int m_installFontCnt;
    bool m_isReinstall;

protected:
    QThread mThread;

    QList<DFontPreviewItemData> m_diffFontModelList;
    DFMDBManager *m_dbManager {nullptr};
    DFontPreviewListView *m_view;
    QFileSystemWatcher *m_fsWatcher;
    QMutex *m_mutex;
    DFontInfoManager *m_fontInfoManager;
    DFontManager *m_fontManager;
};

#endif // DFONTPREVIEWLISTDATATHREAD_H
