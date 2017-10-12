#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "serverutility.h"

using namespace std;



int main(int argc, char *argv[])
{

    //command thread
    pthread_t ctid;
    if (pthread_create(&ctid, NULL, CommandManager, NULL) !=0) {
        perror("pthread_create() CommandWork");
        return 0;
    }

    //client manager thread
    pthread_t dtid;
    if (pthread_create(&dtid, NULL, DataManager, NULL) !=0) {
        perror("pthread_create() CommandWork");
        return 0;
    }

    //msg broadcast thread
    pthread_t btid;
    if (pthread_create(&btid, NULL, BroadCaster, NULL) !=0) {
        perror("pthread_create() BroadCaster");
        return 0;
    }

    while(true) {
        sleep(5);
    }

    return 0;
}
