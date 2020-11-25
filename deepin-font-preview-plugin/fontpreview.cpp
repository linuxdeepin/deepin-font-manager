/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fontpreview.h"

DFM_BEGIN_NAMESPACE

FontPreview::FontPreview(QObject *parent):
    DFMFilePreview(parent),
    m_previewWidget(new DFontWidget)
{
}

FontPreview::~FontPreview()
{
    if (m_previewWidget)
        m_previewWidget->deleteLater();
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
DUrl FontPreview::fileUrl() const
{
    return m_url;
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
bool FontPreview::setFileUrl(const DUrl &url)
{
    if (m_url == url)
        return true;

    if (!url.isLocalFile())
        return false;

    m_url = url;
    qDebug() << __FUNCTION__ << m_url << "m_url";
    m_previewWidget->setFileUrl(url.toLocalFile());
    m_title = url.toString();
    qDebug() << __FUNCTION__ << m_title << url.toLocalFile() << "m_title";

    if (!m_title.split("/").isEmpty()) {
        m_title = m_title.split("/").last();
        qDebug() << __FUNCTION__ << m_title;
    }

    Q_EMIT titleChanged();

    return true;
}

///*************************************************************************
// <Function>      contentWidget
// <Description>   获取预览控件
// <Author>        null
// <Input>
//    <param1>     null                 Description:null
// <Return>        QWidget *            Description:预览控件的指针
// <Note>          null
//*************************************************************************/
//QWidget *FontPreview::contentWidget() const
//{
//    return m_previewWidget;
//}

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
    return m_title;
}

DFM_END_NAMESPACE
