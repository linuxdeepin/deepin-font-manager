// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddlog.h"
#include "signalmanager.h"

#include <QDebug>

bool SignalManager::m_isOnLoad = false;
bool SignalManager::m_isOnStartupLoad = false;
bool SignalManager::m_isDataLoadFinish = false;

SignalManager *SignalManager::m_signalManager = nullptr;


/*************************************************************************
 <Function>      instance
 <Description>   信号处理类对象单例构造函数
 <Author>
 <Input>         null
 <Return>        SignalManager::instance()  Description:返回信号管理类对象的单例
 <Note>          null
*************************************************************************/
SignalManager *SignalManager::instance()
{
    if (m_signalManager == nullptr) {
        qCDebug(fmLog) << "Creating new SignalManager instance";
        m_signalManager = new SignalManager;
    } else {
        qCDebug(fmLog) << "Using existing SignalManager instance";
    }

    return m_signalManager;
}

SignalManager::~SignalManager()
{
    qCDebug(fmLog) << "Destroying SignalManager instance";
    m_signalManager = nullptr;
}

/*************************************************************************
 <Function>      SignalManager
 <Description>   构造函数
 <Author>
 <Input>
    <param1>     parent          Description:父对象
 <Return>        SignalManager   Description:返回信号管理类对象
 <Note>          null
*************************************************************************/
SignalManager::SignalManager(QObject *parent) : QObject(parent)
{
    qCDebug(fmLog) << "SignalManager initialized";
}
