#include <iostream>
#include <string>
#include <cstdio>
#include <vector>
#include <arpa/inet.h>
#include <pthread.h>
using namespace std;
#include "msgqueue.h"
#include "serverutility.h"


MsgQueue::MsgQueue():head(0), tail(0)
{
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&reader, NULL);
    pthread_cond_init(&writer, NULL);
}


MsgQueue::~MsgQueue()
{
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&reader);
    pthread_cond_destroy(&writer);
}

void MsgQueue::push(string msg)
{
    pthread_mutex_lock(&mutex);
    while(is_full()) {
        cout << "the msg queue is full, wait..." << endl;
        pthread_cond_wait(&writer, &mutex);
    }

    //write
    buffer[tail] = msg;
    tail = (tail + 1) % SIZE;
    //unlock and tell reader
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&reader);
}

string MsgQueue::pop()
{
    pthread_mutex_lock(&mutex);
    while(is_empty()) {
        cout << "no msg to read, wait..." << endl;
        pthread_cond_wait(&reader, &mutex);
    }
    //read
    string result = buffer[head];
    head = (head + 1) % SIZE;
    //unlock and tell writer
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&writer);

    return result;
}


SetList::SetList()
{
    pthread_mutex_init(&mutex, NULL);
}

SetList::~SetList()
{
    pthread_mutex_destroy(&mutex);
}

void SetList::SendMsg(string msg)
{
    int msgLen = msg.length();
    if (msgLen == 0)
        return;
    msgLen = htonl(msgLen);

    vector<int> failedSocks;//the sockets disconnected
    pthread_mutex_lock(&mutex);
    set<int>::iterator it = clientSocks.begin();

    for (; it != clientSocks.end(); it++) {
        //write length
        if (Write(*it, (void *)&msgLen, sizeof(msgLen)) < 0) {
            cout << "get error while Write() msg length, delete socket." << endl;
            failedSocks.push_back(*it);
            continue;
        }
        //write msg
        if (Write(*it, msg.c_str(), msg.length()) < 0) {
            cout << "get error while Write() msg, delete socket." << endl;
            failedSocks.push_back(*it);
        }
    }
    for (int i = 0; i < failedSocks.size(); i++) {
        clientSocks.erase(failedSocks[i]);
    }
    pthread_mutex_unlock(&mutex);
}

void SetList::AddSocket(int socket)
{
    pthread_mutex_lock(&mutex);
    clientSocks.insert(socket);
    pthread_mutex_unlock(&mutex);
}

void SetList::DeleteSocket(int socket)
{
    pthread_mutex_lock(&mutex);
    clientSocks.erase(socket);
    pthread_mutex_unlock(&mutex);
}
