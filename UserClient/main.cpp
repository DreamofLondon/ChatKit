#include <iostream>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <pthread.h>

#define SERVERPORT 3389

using namespace std;


bool SendMsg(int socket, string msg)
{
    cout << "try to sending msg: " << msg << endl;//////////
    //send length first
    int msgLen = msg.length();
    if (msgLen == 0)
        return true;

    msgLen = htonl(msgLen);

    int sendBytes = send(socket, (void *)&msgLen, sizeof(msgLen), 0);
    if (sendBytes != sizeof(msgLen)) {
        perror("send() msgLen");
        return false;
    }

    //send msg then
    sendBytes = send(socket, msg.c_str(), msg.length(), 0);
    if (sendBytes != msg.length()) {
        perror("send() msg");
        return false;
    }

    return true;
}


bool RecvMsg(int socket, string &msg)
{
    int msgLen;
    //recv msglen
    int recvBytes = recv(socket, (void *)&msgLen, sizeof(msgLen), 0);
    if (recvBytes != sizeof(msgLen)) {
        perror("RecvMsg() recv msgLen");
        return false;
    }
    msgLen = ntohl(msgLen);
    if (msgLen < 0) {
        cout << "msgLen is not a positive integer." << endl;
        return false;
    }
    //recv msg
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    recvBytes = recv(socket, buffer, msgLen, 0);
    if (recvBytes != msgLen) {
        cout << "recv msg with wrong length." << endl;
        return false;
    }

    msg = buffer;
    
    return true;
}


void *Recv(void *arg)
{
    int serverSock = *(int *)arg;

    while(true) {
        string msg;
        if (!RecvMsg(serverSock, msg)) {
            cout << "RecvMsg() failed, closing..." << endl;
            break;
        }
        time_t nowtime;
        time(&nowtime);
        cout << msg << endl;/////////////////
    }
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <server IP> <my name>" << endl;
        return 0;
    }

    int serverSock;
    struct sockaddr_in serverAddr;
    short serverPort = htons(SERVERPORT);

    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) {
        perror("socket()");
        return 0;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = serverPort;
    if (inet_pton(AF_INET, argv[1], &(serverAddr.sin_addr)) < 0) {
        perror("inet_pton()");
        return 0;
    }

    if (connect(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connet()");
        return 0;
    }

    //create reciever thread
    pthread_t pid;
    if (pthread_create(&pid, NULL, Recv, (void *)&serverSock) != 0) {
        perror("pthread_create() Recv failed.");
        close(serverSock);
        return 0;
    }

    //send name first
    string name(argv[2]);
    if (!SendMsg(serverSock, name)) {
        cout << "send name failed. closing..." << endl;
        close(serverSock);
        return 0;
    }

    //input msg and send
    while (true) {
        string msg;
        cin.sync();
        getline(cin, msg);
	cout << "===get line: " << msg << endl;//////////////
        if (msg.length() == 0)
            continue;
        //send msg
        if (!SendMsg(serverSock, msg)) {
            cout << "send msg failed. closing..." << endl;
            close(serverSock);
            break;
        }


    }



    return 0;
}
