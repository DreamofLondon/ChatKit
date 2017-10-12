#include <iostream>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SERVERPORT 7760

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <server IP> <command string>" << endl;
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

    //send length first
    int msgLen = strlen(argv[2]);
    msgLen = htonl(msgLen);

    int sendBytes = send(serverSock, (void *)&msgLen, sizeof(msgLen), 0);
    if (sendBytes != sizeof(msgLen)) {
        perror("send() msgLen");
        close(serverSock);
        return 0;
    }

    //send msg then
    sendBytes = send(serverSock, argv[2], strlen(argv[2]), 0);
    if (sendBytes != strlen(argv[2])) {
        perror("send() msg");
    }

    close(serverSock);

    return 0;
}
