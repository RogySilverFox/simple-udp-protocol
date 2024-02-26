#ifndef SIMPLEUDPPROTOCOL_WORKERTHREAD_H
#define SIMPLEUDPPROTOCOL_WORKERTHREAD_H

#include <QThread>
#include "SimpleProtocol.h"

class WorkerThread : public QThread {
    Q_OBJECT

public:
    WorkerThread(SimpleProtocol *protocol);
    void run() override;
    void setActivity(bool active) { m_isActive = active; };
    void setProtocol(SimpleProtocol *protocol) { m_protocol = protocol; };

private:
    bool m_isActive;
    SimpleProtocol *m_protocol;
};

#endif //SIMPLEUDPPROTOCOL_WORKERTHREAD_H
