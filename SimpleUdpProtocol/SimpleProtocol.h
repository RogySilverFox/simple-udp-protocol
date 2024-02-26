#ifndef SIMPLEUDPPROTOCOL_SIMPLEPROTOCOL_H
#define SIMPLEUDPPROTOCOL_SIMPLEPROTOCOL_H

#include <QObject>
#include <QUdpSocket>
#include <QThread>

enum MASSAGE_TYPE { String, File }; // 1 byte
enum USER_TYPE { Server, Client }; // 1 byte

class SimpleProtocol : public QObject {
    Q_OBJECT

public:
    SimpleProtocol(QHostAddress address, quint16 port);
    ~SimpleProtocol();
    int sendMessage(std::string massage, QHostAddress& address, quint16 port, bool needCheckDelivery=false, USER_TYPE userType=USER_TYPE::Client);
    int sendFile(std::string filePath, QHostAddress& address, quint16 port, bool needCheckDelivery=false);
    QUdpSocket* getSocet() {return m_socket;};
    void readyRead();

private:
    void _getDataMassage(QByteArray* buffer, quint64* sizeMassage, MASSAGE_TYPE *massageType, USER_TYPE *userType, bool *needCheckDelivery);
    void _writeLog(QString textLog);

    QUdpSocket *m_socket;
    quint16 m_port;
    QThread *m_workerThread;
};

#endif //SIMPLEUDPPROTOCOL_SIMPLEPROTOCOL_H