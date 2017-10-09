#include<iostream>
#include<cstring>
#include<cstdlib>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fstream>

using namespace std;

#define PORT 3389

void process_conn_client(int);

int main()
{
	int server_socket_fd;
	struct sockaddr_in server_addr;

	server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket_fd < 0)
	{
		cout << "socket failed." << endl;
		return -1;
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);
	inet_pton(AF_INET, "47.94.21.114", &server_addr.sin_addr);

	connect(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));
	process_conn_client(server_socket_fd);

	close(server_socket_fd);

	return 0;
}

void process_conn_client(int server_socket_fd)
{
	ssize_t size = 0;
	char buffer[1024];
	write(server_socket_fd, "hi", strlen("hi"));

	size = read(server_socket_fd, buffer, 1024);
	cout << buffer << endl;
	ofstream fout("./abc.txt");
	fout << buffer;
	fout.close();
}
