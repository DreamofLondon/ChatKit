#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#include <string>
#include <set>
#include <pthread.h>

using namespace std;

const int SIZE = 100;

class MsgQueue
{
public:
    MsgQueue();
    void push(string msg);
    string pop();
    ~MsgQueue();


private:
    string buffer[SIZE];
    int head;
    int tail;
    pthread_mutex_t mutex;
    pthread_cond_t reader;
    pthread_cond_t writer;

    bool is_full() {
        return (tail + 1) % SIZE == head;
    }

    bool is_empty() {
        return tail == head;
    }
};


class SetList
{
public:
    SetList();
    ~SetList();
    void SendMsg(string msg);
    void AddSocket(int socket);
    void DeleteSocket(int socket);

private:
    set<int> clientSocks;//the client socket fds
    pthread_mutex_t mutex;

};

#endif // MSGQUEUE_H
