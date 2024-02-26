#include "WorkerThread.h"

WorkerThread::WorkerThread(SimpleProtocol *protocol) : QThread()
{
    m_isActive = false;
    m_protocol = protocol;
}

void WorkerThread::run()
{
    auto soket = m_protocol->getSocet();
    while (m_isActive) {
        while (soket->hasPendingDatagrams()){
            m_protocol->readyRead();
        }
    }
}