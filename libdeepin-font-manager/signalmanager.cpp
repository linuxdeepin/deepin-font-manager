#include "signalmanager.h"

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
