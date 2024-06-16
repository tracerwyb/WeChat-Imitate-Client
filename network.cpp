#include "network.h"
#include <QApplication>
#include <QDataStream>
#include <QFile>
#include <QPixmap>
#include <QStandardPaths>
#include <QTextStream>
#include <arpa/inet.h>
#include <fstream>
#include <netinet/in.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define IPADDR "192.168.101.210" //10.253.209.148" //"10.253.172.131"
#define PORT 9878
Network::Network() {}

void Network::createSocket()
{
    m_listenfd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    std::string ip=IPADDR;       //
    char * ipaddr=ip.data();
    inet_pton(AF_INET,ipaddr,&servaddr.sin_addr);

    if(connect(m_listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0){
        qDebug()<<"Create socket failed. Errorn info";
    }
    else {
        qDebug()<<"connect succeed!";
        qDebug()<<m_listenfd;
    }
}

void Network::sendTextMessage(char* sendmessage,int size)
{
    write(m_listenfd,&size,sizeof(size));
    int offset=0;
    int n=0;
    while(size>0){
        n=write(m_listenfd,sendmessage+offset,size);
        if(n<0){
            qDebug()<<"write erro!";
            break;
        }
        offset+=n;
        size=size-n;
    }
    qDebug() << "n:" << n;
}

int Network::reciveTextMessage(char* recivemessage)
{
    int size=0;
    int result=read(m_listenfd,&size,sizeof(size));
    if(result<=0){
        return -1;
    }
    qDebug()<<"size :"<<size;
    //char recivemessage[1024]="";
    int n=0;
    int posx=0;
    while((size-n)>0 ){
        n=read(m_listenfd,recivemessage+posx,size); //size=size-n
        posx+=n;
    }
    qDebug()<<"mesg:"<<recivemessage;
    return 0;
}

// QPixmap Network::recImage()
// {
//     char filebuf[202400] = "";
//     int n_read;
//     int posx = 0;
//     std::streamsize size;
//     if ((read(m_listenfd, &size, sizeof(std::streamsize))) <= 0) { //读取消息长度
//         qDebug() << "read image length erro";
//     }
//     qDebug() << "recimage size:" << size;
//     while (size > 0) {
//         qDebug() << "n_read:" << n_read;
//         if (n_read <= 0) {
//             qDebug() << "recimg erro,maybe socket was closed!";
//             break;
//         }
//         size = size - n_read;
//         posx = posx + n_read;
//     }
//     QPixmap tempQPixmap;
//     bool invert = tempQPixmap.loadFromData((const unsigned char *) filebuf, posx);
//     qDebug() << "invert to QPixmap:" << invert;
//     return tempQPixmap;
// }
QPixmap Network::recImage()
{
    char filebuf[502400] = "";
    int posx = 0;
    std::streamsize size;

    // 读取消息长度
    if (read(m_listenfd, &size, sizeof(std::streamsize)) <= 0) {
        qDebug() << "Error reading image length";
        return QPixmap(); // 返回空 QPixmap 表示出错
    }
    qDebug() << "Received image size:" << size;
    int totalsize = size;
    // 循环读取数据
    while (size > 0) {
        int n_read = read(m_listenfd, filebuf + posx, totalsize - posx);
        if (n_read <= 0) {
            qDebug() << "Error reading image data, maybe socket was closed!";
            return QPixmap(); // 返回空 QPixmap 表示出错
        }
        size -= n_read;
        posx += n_read;
    }
    qDebug() << "Print recIMage POSX" << posx;
    QPixmap tempQPixmap;
    bool invert = tempQPixmap.loadFromData(reinterpret_cast<const uchar *>(filebuf), posx);
    qDebug() << "Loaded into QPixmap:" << invert;
    return tempQPixmap;
}

void Network::sendImage(std::string path)
{
    qDebug() << path;
    size_t pos = path.find("file://");
    if (pos != std::string::npos) {
        path.erase(pos, strlen("file://"));
    }
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
        qDebug() << "file open erro!";

    std::streamsize size = file.tellg();
    qDebug() << "file size:" << size;
    qDebug() << "sizeof(size):" << sizeof(size);

    write(m_listenfd, &size, sizeof(size));
    file.seekg(0, std::ios::beg);

    char buf[10240];

    while (size > 0) {
        std::streamsize file_read = file.read(buf, sizeof(buf)).gcount();
        qDebug() << "file_read:" << file_read;
        write(m_listenfd, buf, file_read);
        size = size - file_read;
    }
    qDebug() << "send image ok";
}

void Network::closeSocket()
{
    close(m_listenfd);
}

int Network::Select()
{
    fd_set readfds;
    struct timeval tv;
    int retval;

    FD_ZERO(&readfds);
    FD_SET(m_listenfd,&readfds);

    tv.tv_sec=1;  //seconds
    tv.tv_usec=0; //misroseconds

    retval = select(m_listenfd+1,&readfds,NULL,NULL,&tv);
    if(retval ==1 )
        return 1;
    else {
        return 0;
    }
}


