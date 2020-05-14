/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#ifndef DFONTPREVIEW_H
#define DFONTPREVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QFontDatabase>

#include <ft2build.h>
#include FT_FREETYPE_H

class DFontPreview : public QWidget
{
    Q_OBJECT

public:
    DFontPreview(QWidget *parent = nullptr);
    ~DFontPreview();

    void setFileUrl(const QString &url);

    QFontDatabase fontDatabase;

protected:
    void paintEvent(QPaintEvent *);

private:
    void initContents();
    QString getSampleString();
    QString getLanguageSampleString(const QString &language);
    bool checkFontContainText(const QString &text);
    QString buildCharlistForFace(int length);

private:
    FT_Library m_library;
    FT_Face m_face;
    FT_Error m_error = 1;
};

#endif  // DFONTPREVIEW_H
