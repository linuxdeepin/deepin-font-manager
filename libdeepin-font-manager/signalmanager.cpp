#include "signalmanager.h"

SignalManager *SignalManager::m_signalManager = nullptr;

SignalManager *SignalManager::instance()
{
    if (m_signalManager == nullptr) {
        m_signalManager = new SignalManager;
    }

    return m_signalManager;
}

SignalManager::SignalManager(QObject *parent) : QObject(parent)
{

}
