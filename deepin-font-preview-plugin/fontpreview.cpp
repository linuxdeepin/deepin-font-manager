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
#include <QFileInfo>

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

DUrl FontPreview::fileUrl() const
{
    return m_url;
}

bool FontPreview::setFileUrl(const DUrl &url)
{
    if (m_url == url)
        return true;

    if (!url.isLocalFile())
        return false;

    m_url = url;
    m_previewWidget->setFileUrl(url.toLocalFile());
    m_title = url.toString();
    qDebug() << __FUNCTION__ << m_title << url.toLocalFile();

    if (!m_title.split("/").isEmpty()) {
        m_title = m_title.split("/").last();
        qDebug() << __FUNCTION__ << m_title;
    }

    Q_EMIT titleChanged();

    return true;
}

QWidget *FontPreview::contentWidget() const
{
    return m_previewWidget;
}

QString FontPreview::title() const
{
    return m_title;
}

bool FontPreview::showStatusBarSeparator() const
{
    return true;
}

DFM_END_NAMESPACE
