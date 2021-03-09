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

#include "signalmanager.h"
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
        m_signalManager = new SignalManager;
    }

    return m_signalManager;
}

SignalManager::~SignalManager()
{
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

}
