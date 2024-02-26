#include "SimpleProtocol.h"
#include <QFile>
#include <cstring>
#include <iostream>
#include "WorkerThread.h"

const quint64 MAX_MASSAGE_SIZE = 3709551000;// 18446744073709551000;
bool checkMassageSize(quint64 massageSize)
{
    return massageSize <= MAX_MASSAGE_SIZE;
}

SimpleProtocol::SimpleProtocol(QHostAddress address, quint16 port) : QObject()
{
    m_socket = new QUdpSocket(this);
    m_port = port;
    m_socket->bind(address, port);

    WorkerThread *thread = new WorkerThread(this);
    thread->setActivity(true);
    m_workerThread = thread;
    m_workerThread->start();
}

SimpleProtocol::~SimpleProtocol() {
    WorkerThread *thread = (WorkerThread*) m_workerThread;
    thread->setActivity(false);

    m_workerThread->quit();
    m_workerThread->wait();

    delete m_socket;
    delete m_workerThread;
}

// size[8] massage_type[1] user_type[1] flag[1] data[...]
int SimpleProtocol::sendMessage(std::string massage, QHostAddress& address, quint16 port, bool needCheckDelivery, USER_TYPE userType)
{
    if (not checkMassageSize(massage.size())) {
        return 1;
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << quint64(massage.size());
    data.append(MASSAGE_TYPE::String);
    data.append(userType);
    data.append(char(needCheckDelivery));
    data.append(massage.data());

    m_socket->writeDatagram(data, address, port);
    return 0;
}

// size[8] massage_type[1] user_type[1] flag[1] size_type[1] file_type[...] data[...]
int SimpleProtocol::sendFile(std::string filePath, QHostAddress& address, quint16 port, bool needCheckDelivery)
{
    QFile file = QFile(filePath.data());
    if (not file.exists()) {
        return 2;
    }

    quint64 fileSize = file.size();
    if (not checkMassageSize(fileSize)) {
        return 1;
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << fileSize;
    data.append(MASSAGE_TYPE::File);
    data.append(USER_TYPE::Client);
    data.append(char(needCheckDelivery));

    std::string fileType = std::strstr(filePath.data(), ".");
    data.append(qint8(fileType.size()));
    data.append(fileType.data());

    if (!file.open(QIODevice::ReadOnly)) {
        return 3;
    }

    while (!file.atEnd()) {
        data.append(file.readAll());
    }
    file.close();

    m_socket->writeDatagram(data, address, port);
    return 0;
}

void SimpleProtocol::readyRead()
{
    QByteArray buffer;
    buffer.resize(m_socket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    m_socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

    quint64 sizeMassage;
    MASSAGE_TYPE massageType; // 1 byte
    USER_TYPE userType;
    bool needCheckDelivery;

    this->_getDataMassage(&buffer, &sizeMassage, &massageType, &userType, &needCheckDelivery);
    if (needCheckDelivery) {
        this->sendMessage("message received success", sender, senderPort, false, USER_TYPE::Server);
    }

    std::time_t result = std::time(nullptr);
    std::string currentTime = std::asctime(std::localtime(&result));
    currentTime.pop_back();
    std::string message = buffer.data();

    if (MASSAGE_TYPE::File == massageType) {
        qint8 sizeFileType = quint8(buffer.at(0));
        buffer = buffer.remove(0, 1);
        std::string fileType = buffer.first(sizeFileType).toStdString();
        buffer = buffer.remove(0, sizeFileType);
        message = currentTime + fileType;

        QFile file = QFile(message.data());
        file.open(QIODevice::WriteOnly);
        file.write(buffer);
        file.close();
    }

    QString textLog = "From: " + sender.toString() + ":" + QString::number(senderPort) + "; " +
            "User: " + (USER_TYPE::Server == userType ? "server" : "client") + "; " +
            "Time: " + currentTime.data()  + "; " +
            "Massage type: " + (MASSAGE_TYPE::File == massageType ? "File" : "String") + "; " +
            "Massage size: " + QString::number(sizeMassage) + "; " +
            "Message: " + message.data() + "\n" ;

    this->_writeLog(textLog);

    std::cout << "------------------------------" << std::endl;
    std::cout << "From: " << sender.toString().toStdString() << ":" << senderPort << std::endl;
    std::cout << "User: " << (USER_TYPE::Server == userType ? "server" : "client") << std::endl;
    std::cout << "Time: " << currentTime.data() << std::endl;
    std::cout << "Massage type: " << (MASSAGE_TYPE::File == massageType ? "File" : "String") << std::endl;
    std::cout << "Massage size: " << sizeMassage << std::endl;
    std::cout << "Message: " << message.data() << std::endl;
    std::cout << "------------------------------" << std::endl;
}

void SimpleProtocol::_getDataMassage(QByteArray* buffer, quint64* sizeMassage, MASSAGE_TYPE *massageType, USER_TYPE *userType, bool *needCheckDelivery)
{
    QDataStream dataStream(buffer->first(8));
    dataStream >> *sizeMassage;
    *massageType = MASSAGE_TYPE(buffer->at(8));
    *userType = USER_TYPE(buffer->at(9));
    *needCheckDelivery = bool(buffer->at(10));
    *buffer = buffer->remove(0, 11);
}

void SimpleProtocol::_writeLog(QString textLog)
{
    QFile file = QFile("logs.log");
    file.exists() ? file.open(QIODevice::Append) : file.open(QIODevice::ReadWrite);
    file.write(textLog.toStdString().data());
    file.close();
}