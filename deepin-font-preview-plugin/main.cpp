// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//#include "dde-file-manager/dfmfilepreviewplugin.h"

#include "fontpreview.h"

//DFM_BEGIN_NAMESPACE

class TextPreviewPlugin /*: public DFMFilePreviewPlugin*/: public QObject
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID DFMFilePreviewFactoryInterface_iid FILE "deepin-font-preview-plugin.json")

public:
//    DFMFilePreview *create(const QString &key) Q_DECL_OVERRIDE {
//        Q_UNUSED(key)

//        return new FontPreview();
//        return NULL;
//    }
};


//DFM_END_NAMESPACE

#include "main.moc"
