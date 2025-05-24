// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fontpreview.h"

#ifdef DFM_BASE
DFMBASE_BEGIN_NAMESPACE
#else
DFM_BEGIN_NAMESPACE
#endif

#ifdef DFM_BASE
FontPreview::FontPreview(QObject *parent):
    AbstractBasePreview(parent),
#else
FontPreview::FontPreview(QObject *parent):
    DFMFilePreview(parent),
#endif
    m_previewWidget(new DFontWidget)
{
    qDebug() << "FontPreview constructor";
}

FontPreview::~FontPreview()
{
    qDebug() << "FontPreview destructor";
//    bug 142781
//    插件中不需要释放。释放会导致文管异常退出。
//    if (m_previewWidget)
//        m_previewWidget->deleteLater();
}

/*************************************************************************
 <Function>      setFileUrl
 <Description>   设置url路径
 <Author>        null
 <Input>
    <param1>     url             Description:需要设置的路径
 <Return>        bool                Description:是否设置成功
 <Note>          null
*************************************************************************/
#ifdef DFM_BASE
bool FontPreview::setFileUrl(const QUrl &url)
#else
bool FontPreview::setFileUrl(const DUrl &url)
#endif
{
    qDebug() << "setFileUrl called with url:" << url;
    
    if (m_url == url) {
        qDebug() << "URL unchanged, skipping";
        return true;
    }

    if (!url.isLocalFile()) {
        qWarning() << "Non-local file URL not supported:" << url;
        return false;
    }

    m_url = url;
    qInfo() << "Setting new file URL:" << url;
    
    m_previewWidget->setFileUrl(url.toLocalFile());
    qDebug() << "Preview widget URL set to:" << url.toLocalFile();
    
    m_title = url.toString();
    qDebug() << "Initial title:" << m_title;

    if (!m_title.split("/").isEmpty()) {
        m_title = m_title.split("/").last();
        qDebug() << "Extracted title:" << m_title;
    }

    Q_EMIT titleChanged();

    return true;
}

/*************************************************************************
 <Function>      fileUrl
 <Description>   获取文件的url路径
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        DUrl            Description:返回字体文件的url路径
 <Note>          null
*************************************************************************/
#ifdef DFM_BASE
QUrl FontPreview::fileUrl() const
#else
DUrl FontPreview::fileUrl() const
#endif
{
    return m_url;
}

/*************************************************************************
 <Function>      contentWidget
 <Description>   获取预览控件
 <Author>        null
 <Input>
    <param1>     null                 Description:null
 <Return>        QWidget *            Description:预览控件的指针
 <Note>          null
*************************************************************************/
QWidget *FontPreview::contentWidget() const
{
    qDebug() << "Getting content widget";
    return m_previewWidget;
}

/*************************************************************************
 <Function>      title
 <Description>   获取标题label显示内容
 <Author>        null
 <Input>
    <param1>     null               Description:null
 <Return>        QString            Description:label显示内容
 <Note>          null
*************************************************************************/
QString FontPreview::title() const
{
    qDebug() << "Getting title:" << m_title;
    return m_title;
}

#ifdef DFM_BASE
DFMBASE_END_NAMESPACE
#else
DFM_END_NAMESPACE
#endif

