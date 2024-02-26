//---------------------------------------------------
// examples commands
// create -a=127.0.0.1 -p=2000                                  -- Создание сервера; [a] - address; [p] - port;
// sendM -a=127.0.0.1 -p=2000 -m=test -c=1                      -- Отправка сообщений; [a] - adress; [p] - port; [c] - need answer (1/0); [m] - massage;
// sendF -a=127.0.0.1 -p=2000 -m=/home/Desktop/1.png -c=1       -- Отправка файлов; [a] - adress; [p] - port; [c] - need answer (1/0); [m] - absolute path file
// exit                                                         -- Выход
//---------------------------------------------------

#include "SimpleProtocol.h"
#include <iostream>
#include <QRegularExpression>
#include <QThread>

enum COMANDS { Exit, Create, SendMassage, SendFile, Error };

void getCommandData(std::string command, std::string* commandName, std::map <char, std::string>* commandArgsMap) {
    QRegularExpression regular("\\w+");
    auto matchData = regular.match(QString(command.data()));
    if (matchData.hasMatch()) {
        *commandName = matchData.captured().toStdString();
    }

    regular = QRegularExpression("-\\w=[A-Za-z0-9\\./]+", QRegularExpression::MultilineOption);
    auto matchArgsData = regular.globalMatch(QString(command.data()));
    while (matchArgsData.hasNext()) {
        std::string parameterValue;
        matchData = matchArgsData.next();
        auto data = matchData.captured().toStdString();
        parameterValue.assign(data, 3, data.size()-3);
        (*commandArgsMap)[data[1]] = parameterValue;
    }
}

void exitCommand(SimpleProtocol **simpleProtocol) {
    if (*simpleProtocol)
        delete *simpleProtocol;
}

void createCommand(SimpleProtocol **simpleProtocol, std::map <char, std::string>* commandArgs) {
    if (commandArgs->find('a')==commandArgs->cend() || commandArgs->find('p')==commandArgs->cend() ) {
        std::cout << "[ERROR] Not found paramrters" << std::endl;
        return;
    }

    exitCommand(simpleProtocol);
    *simpleProtocol = new SimpleProtocol(
            QHostAddress(QString(commandArgs->at('a').data())),
            qint16(std::stoi(commandArgs->at('p')))
    );
    std::cout << "Create simple protocol" << std::endl;
}

void sendMassageCommand(SimpleProtocol **simpleProtocol, std::map <char, std::string>* commandArgs) {
    if (!*simpleProtocol){
        std::cout << "[ERROR] Not create simpleProtocol" << std::endl;
        return;
    }

    if (commandArgs->find('a')==commandArgs->cend() || commandArgs->find('p')==commandArgs->cend()){
        std::cout << "[ERROR] Not found paramrters" << std::endl;
        return;
    }

    std::string massage = commandArgs->find('m')!=commandArgs->cend() ? commandArgs->at('m') : "";
    QHostAddress address = QHostAddress(QString(commandArgs->at('a').data()));
    auto port = qint16(std::stoi(commandArgs->at('p')));
    bool needCheckDelivery = commandArgs->find('c')!=commandArgs->cend() ? bool(std::stoi(commandArgs->at('c'))) : false;

    auto result = (*simpleProtocol)->sendMessage(massage, address, port, needCheckDelivery);
    if (result > 0)
        std::cout << "[ERROR] Command args not valid" << std::endl;
}

void sendFileCommand(SimpleProtocol **simpleProtocol, std::map <char, std::string>* commandArgs) {
    if (!*simpleProtocol){
        std::cout << "[ERROR] Not create simpleProtocol" << std::endl;
        return;
    }

    if (commandArgs->find('a')==commandArgs->cend() || commandArgs->find('p')==commandArgs->cend()){
        std::cout << "[ERROR] Not found paramrters" << std::endl;
        return;
    }

    std::string massage = commandArgs->find('m')!=commandArgs->cend() ? commandArgs->at('m') : "";
    QHostAddress address = QHostAddress(QString(commandArgs->at('a').data()));
    auto port = qint16(std::stoi(commandArgs->at('p')));
    bool needCheckDelivery = commandArgs->find('c')!=commandArgs->cend() ? bool(std::stoi(commandArgs->at('c'))) : false;

    auto result = (*simpleProtocol)->sendFile(massage, address, port, needCheckDelivery);
    if (result > 0)
        std::cout << "[ERROR] Command args not valid" << std::endl;
}

int main() {
    SimpleProtocol *simpleProtocol;
    std::string command;
    std::map <std::string, COMANDS> commandMap =    {{"exit",   Exit},
                                                    {"create",  Create},
                                                    {"sendM",    SendMassage},
                                                    {"sendF",    SendFile}};

    try {
        while (true) {
            std::cout << "Enter command: ";
            getline(std::cin, command);

            std::string commandName = "";
            std::map<char, std::string> commandArgsMap;

            getCommandData(command, &commandName, &commandArgsMap);

            auto typeCommand = commandMap.find(commandName) != commandMap.cend() ? commandMap.at(commandName) : Error;
            switch (typeCommand) {
                case Exit:
                    exitCommand(&simpleProtocol);
                    QThread::sleep(1);
                    return 0;
                case Create:
                    createCommand(&simpleProtocol, &commandArgsMap);
                    QThread::sleep(1);
                    break;
                case SendMassage:
                    sendMassageCommand(&simpleProtocol, &commandArgsMap);
                    QThread::sleep(1);
                    break;
                case SendFile:
                    sendFileCommand(&simpleProtocol, &commandArgsMap);
                    QThread::sleep(1);
                    break;
                default:
                    std::cout << "[ERROR] Not found command" << std::endl;
            }
        }
    }
    catch (...) {
        std::cout << "[ERROR] Stupid user" << std::endl;
        return 0;
    }
}