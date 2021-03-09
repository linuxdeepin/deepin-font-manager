/*
 *
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:
*
* Maintainer:
*
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
