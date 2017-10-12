#ifndef SERVERUTILITY_H
#define SERVERUTILITY_H
#include <string>

using namespace std;


/**
 * @brief CreateListen
 * @param ip
 * @param port
 * @return socket fd or -1 for error
 */
int CreateListen(string ip, short port);

/**
 * @brief ReadExecuteCommand
 * @param sock
 */
void ReadExecuteCommand(int sock);

/**
 * @brief CommandWork wait for commands
 * @param arg not used
 * @return
 */
void *CommandManager(void *arg);

/**
 * @brief DataManager recv new clients and create their VIP thread
 * @param arg not used
 * @return
 */
void *DataManager(void *arg);

/**
 * @brief ClientTalker the VIP thread for every client
 * @param arg the client socket
 * @return
 */
void *ClientTalker(void *arg);

/**
 * @brief BroadCaster the thread send msg to a client connected
 * @param arg
 * @return
 */
void *BroadCaster(void *arg);

/**
 * @brief Read read data from socket
 * @param sock
 * @param buffer
 * @param readLen the number of bytes to recv from socket according to the protocol
 * @return
 */
int Read(int sock, void *buffer, int readLen);

/**
 * @brief Write write data to socket
 * @param sock
 * @param buffer
 * @param writeLen the number of bytes to send to socket according to the protocol
 * @return
 */
int Write(int sock, const void *buffer, int writeLen);
#endif // SERVERUTILITY_H
