// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FONTPREVIEWPLUGIN_H
#define FONTPREVIEWPLUGIN_H

#include <QObject>

#include "dfmfilepreview.h"
#include "durl.h"
#include "dfontwidget.h"

DFM_BEGIN_NAMESPACE

class FontPreview : public DFMFilePreview
{
    Q_OBJECT

public:
    explicit FontPreview(QObject *parent = nullptr);
    ~FontPreview()Q_DECL_OVERRIDE;

    DUrl fileUrl() const Q_DECL_OVERRIDE;
    bool setFileUrl(const DUrl &url) Q_DECL_OVERRIDE;

    QWidget *contentWidget() const Q_DECL_OVERRIDE;

    QString title() const Q_DECL_OVERRIDE;

private:
    DUrl m_url;
    QString m_title;

    DFontWidget *m_previewWidget;
};

DFM_END_NAMESPACE

#endif // FONTPREVIEWPLUGIN_H
