// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontloadthread.h"
#include <QFile>
#include <QDebug>

DFontLoadThread::DFontLoadThread(QObject *parent)
    : QThread(parent)
{
    qDebug() << "DFontLoadThread created";
}

DFontLoadThread::~DFontLoadThread()
{
    // qDebug() << "DFontLoadThread destroyed";
}

void DFontLoadThread::open(const QString &filepath)
{
    qDebug() << "Preparing to load font file:" << filepath;
    m_filePath = filepath;
}

void DFontLoadThread::run()
{
    qDebug() << "Starting to load font file:" << m_filePath;
    QFile file(m_filePath);

    if (file.open(QIODevice::ReadOnly)) {
        qDebug() << "Successfully opened font file:" << m_filePath;
        QByteArray fileContent = file.readAll();
        // qDebug() << "Read" << fileContent.size() << "bytes from font file";
        
        emit loadFinished(fileContent);
    } else {
        qWarning() << "Failed to open font file:" << m_filePath << "Error:" << file.errorString();
        emit loadFinished(QByteArray());
    }

    file.close();
    qDebug() << "Font loading thread finished";
}
