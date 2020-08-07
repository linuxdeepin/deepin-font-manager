#ifndef ATTRSCROLLWIDGET_H
#define ATTRSCROLLWIDGET_H

#include <map>
#include <DFrame>
#include <DWidget>
#include <DLabel>
#include <QGridLayout>
#include <QTextLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <dfontpreviewitemdef.h>//
#include <DApplication>
#include "dfontbasedialog.h"
#include "dfontbasedialog.h"

#include <QResizeEvent>
#include <QPointF>
#include <DFrame>
#include <DLabel>
#include <QTextEdit>
#include <DWidget>
#include "signalmanager.h"
DWIDGET_USE_NAMESPACE

/**
 * @brief The AttrScrollWidget class
 * @brief   属性显示
 */

/*************************************************************************
 <Class>         dfontinfoscrollarea
 <Description>   字体管理器信息页面详情部分
 <Author>
 <Note>          null
*************************************************************************/
class dfontinfoscrollarea: public DFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(dfontinfoscrollarea)

public:
    explicit dfontinfoscrollarea(DFontPreviewItemData *pData, DWidget *parent  = nullptr);
    //用于信息页面详情信息部分长度判断
    QString elideText(const QString &text, const QFont &font, int nLabelSize);//
    //在字体变化后重绘信息页面
    void updateText();
    //重新计算信息详情页面高度参数
    void autoHeight();
protected:
    //事件过滤器-过滤到字体改变事件后更新详情页字体信息与页面高度
    bool eventFilter(QObject *obj, QEvent *e) override;
    //获取信号管理类的单例对象
    SignalManager *m_signalManager = SignalManager::instance();
    //初始化信息页面中详情页面
    void initUi();
private:
    //初始话信息页面详情中信息部分标签内容
    void createLabel(QGridLayout *layout, const int &index, const QString &objName, const QString &sData);
private:
    //初始话信息页面详情中标题部分标签内容
    DFrame *addTitleFrame(const QString &sData, const QString &objName);
    int m_leftminwidth;

    DFontPreviewItemData *m_fontInfo;//字体信息
    DLabel *basicLabel = nullptr;
    std::map<QLabel *, QString> pTitleMap;//存储信息title键值对
    std::map<QLabel *, QString> pLabelMap;//存储信息info label键值对
    QString elideText(QString &titleName) const;
};

#endif // ATTRSCROLLWIDGET_H
