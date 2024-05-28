#ifndef CLIENT_H
#define CLIENT_H
#include "network.h"
#include <string>
#include <iostream>
#include <QDateTime>
#include <QQmlEngine>
#define MAX 1024

class Client
{

private:
    Client();
    static Client* m_instance;

    Network m_network;

    char buf[MAX];
    int id;
    int acid;
    std::string request_type;
    QDateTime time;

    QString m_name;

signals:
    void  netnameChanged(const QString netname);
public:
    static Client *getInstance();

    void send(char *buf, int size);

    bool receive(char *buf);

    void start();

    int select();

    //void reconnect();

    //std::string receiveFile();

    void closeSocket();

    
    char *Messagedata();
    void comversionJson(char* json_buf);
    void setId();
    void setAcceptId();
    void setRequestType();

};

#endif
