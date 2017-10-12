#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
//#include <set>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "serverutility.h"
#include "msgqueue.h"

#define LISTENQUE 5
#define COMMANDPORT 7760
#define DATAPORT 3389

using namespace std;

MsgQueue mq;
SetList sl;

/**
 * @brief CreateListen
 * @param ip
 * @param port
 * @return socket fd or -1 for error
 */
int CreateListen(string ip, short port)
{
    if (port <= 0)
        return -1;

    int serverSock;
    struct sockaddr_in serverAddr;
    short serverPort = htons(port);
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = serverPort;

    //ge addr
    if (ip.length() == 0) {
        //defaut addr
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        //get binary type of ip addr
        int ret = inet_pton(AF_INET, ip.c_str(), (void *)&(serverAddr.sin_addr));
        if (ret == 0) {
            cout << "wrong IP format." << endl;
            return -1;
        } else if (ret < 0) {
            cout << "inet_pton() failed with ip: " << ip << endl;
            perror("inet_pton()");
            return -1;
        }
    }

    serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSock < 0) {
        perror("socket()");
        return serverSock;
    }

    //set addr reuseful
    int optValue = SO_REUSEADDR;
    if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, (void *)&optValue, sizeof(optValue)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
        close(serverSock);
        return -1;
    }

    //bind and listen
    if (bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind()");
        close(serverSock);
        return -1;
    }

    if (listen(serverSock, LISTENQUE) < 0) {
        perror("listen()");
        close(serverSock);
        return -1;
    }

    return serverSock;
}


/**
 * @brief CommandWork wait for commands
 * @param arg not used
 * @return
 */
void *CommandManager(void *arg)
{
    //get socket
    int commandSock = CreateListen("", COMMANDPORT);
    if (commandSock < 0) {
        cout << "CreateListen() command failed." << endl;
        return 0;
    }

    //accept
    struct sockaddr_in clientAddr;
    memset(&clientAddr, 0, sizeof(clientAddr));
    socklen_t addrLen = sizeof(clientAddr);
    while(true) {
        struct sockaddr_in clientAddr;
        memset(&clientAddr, 0, addrLen);
        int clientSock = accept(commandSock, (struct sockaddr *)&clientAddr, &addrLen);
        if (clientSock < 0) {
            perror("accept()");
            continue;
        }

        //read and execute command
        ReadExecuteCommand(clientSock);
    }
}

/**
 * @brief ReadExecuteCommand
 * @param sock
 */
void ReadExecuteCommand(int sock)
{
    /*
     * int - command buffer length int network order
     * char* - command buffer
    */

    //read int
    int commandLen;
    if (Read(sock, (void *)&commandLen, sizeof(commandLen)) < 0){
        cout << "Read() commandLen failed." << endl;
        return;
    }
    commandLen = ntohl(commandLen);

    //read command
    if (commandLen <= 0) {
        cout << "commandLen is not a positive integer." << endl;
        return;
    }
    char commandLine[256];
    memset(commandLine, 0, sizeof(commandLine));
    if (Read(sock, (void *)commandLine, commandLen) < 0) {
        cout << "Read() commandLine failed." << endl;
        return;
    }

    //execute command
    cout << "now we execute command: " << commandLine << endl;

    //close socket
    close(sock);
}


/**
 * @brief DataManager recv new clients and create their VIP thread
 * @param arg not used
 * @return
 */
void *DataManager(void *arg)
{
    int dataSock = CreateListen("", DATAPORT);
    if (dataSock < 0) {
        cout << "CreateListen() data failed." << endl;
        return 0;
    }

    //accept
    struct sockaddr_in clientAddr;
    memset(&clientAddr, 0, sizeof(clientAddr));
    socklen_t addrLen = sizeof(clientAddr);
    while(true) {
        struct sockaddr_in clientAddr;
        memset(&clientAddr, 0, addrLen);
        int *clientSock = new int;
        *clientSock = accept(dataSock, (struct sockaddr *)&clientAddr, &addrLen);
        if (*clientSock < 0) {
            perror("accept()");
            delete clientSock;
            continue;
        }

        //add to the client sockets list
        sl.AddSocket(*clientSock);

        //create client thread
        pthread_t tid;
        if (pthread_create(&tid, NULL, ClientTalker, (void *)clientSock) != 0) {
            perror("pthread_create() clientTalker");
            delete clientSock;
        }
    }
}

/**
 * @brief ClientTalker the VIP thread for every client
 * @param arg the client socket
 * @return
 */
void *ClientTalker(void *arg)
{
    //get socket
    int clientSock = *(int *)arg;
    delete (int *)arg;

    /*
     * first read client's name then begin talking
     * int - name buffer length, int network order
     * char* - name buffer
    */
    //read int
    int nameLen;
    if (Read(clientSock, (void *)&nameLen, sizeof(nameLen)) < 0) {
        cout << "Read() nameLen failed." << endl;
        sl.DeleteSocket(clientSock);
        return NULL;
    }
    nameLen = ntohl(nameLen);//change to local order

    //read name
    if (nameLen <= 0) {
        cout << "nameLen is not a positive integer." << endl;
        sl.DeleteSocket(clientSock);
        close(clientSock);
        return NULL;
    }
    char nameBuffer[32];
    memset(nameBuffer, 0, sizeof(nameBuffer));
    if (Read(clientSock, (void *)nameBuffer, nameLen) < 0) {
        cout << "Read() nameBuffer failed." << endl;
        sl.DeleteSocket(clientSock);
        return NULL;
    }
    cout << "recv a new client named: " << nameBuffer << endl;

    //now begin talking
    while(true) {
        int bufferLen;
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        if (Read(clientSock, (void *)&bufferLen, sizeof(bufferLen)) < 0) {
            cout << "Read() bufferLen failed." << endl;
            sl.DeleteSocket(clientSock);
            break;
        }
        bufferLen = ntohl(bufferLen);
        if (bufferLen <= 0 || bufferLen >= 256) {
            cout << "recv bufferLen not positive integer or larger than 255." << endl;
            sl.DeleteSocket(clientSock);
            close(clientSock);
            break;
        }

        //recv buffer
        if (Read(clientSock, (void *)buffer, bufferLen) < 0) {
            cout << "Read() buffer failed." << endl;
            sl.DeleteSocket(clientSock);
            break;
        }

        //send msg to msg queue
        string theMsg(nameBuffer);
        theMsg = theMsg + " : " + buffer;
        mq.push(theMsg);

    }

}

/**
 * @brief BroadCaster the thread send msg to a client connected
 * @param arg
 * @return
 */
void *BroadCaster(void *arg)
{
    while(true) {
        string msg = mq.pop();
        sl.SendMsg(msg);
    }
}

/**
 * @brief Read read data from socket
 * @param sock
 * @param buffer
 * @param readLen the number of bytes to recv from socket according to the protocol
 * @return
 */
int Read(int sock, void *buffer, int readLen)
{
    if (readLen == 0)
        return 0;
    int recvLen = recv(sock, buffer, readLen, 0);
    if (recvLen != readLen) {
        perror("recv()");
        close(sock);
        return -1;
    }

    return recvLen;
}


/**
 * @brief Write write data to socket
 * @param sock
 * @param buffer
 * @param writeLen the number of bytes to send to socket according to the protocol
 * @return
 */
int Write(int sock, const void *buffer, int writeLen)
{
    if (writeLen == 0)
        return 0;
    int sendLen = send(sock, buffer, writeLen, 0);
    if (sendLen != writeLen) {
        perror("send()");
        close(sock);
        return -1;
    }

    return sendLen;
}

