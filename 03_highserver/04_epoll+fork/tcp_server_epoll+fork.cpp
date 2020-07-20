#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#define MAX_PROCESS 4

#define PORT 8111
#define MESSAGE_SIZE 1024
#define MAX_EVENTS 20
#define TIMEOUT 500
int main(int argc, char *argv[])
{
	int ret = -1;
	int socket_fd = -1, accept_fd = -1;
	int on = 1;
	int backlog = 10;
	int epoll_fd;
	int flags = 1;
	struct epoll_event ev, events[MAX_EVENTS];
	int event_number;

	pid_t pid = -1;
	struct sockaddr_in localaddr, remoteaddr;

	char in_buff[MESSAGE_SIZE] = {
		0,
	};
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1)
	{
		std::cout << "Failed to create socket!" << std::endl;
		exit(-1);
	}
	flags = fcntl(socket_fd, F_GETFL, 0);
	fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
	ret = setsockopt(socket_fd, SOL_SOCKET,
					 SO_REUSEADDR,
					 &on,
					 sizeof(on));

	if (ret == -1)
	{
		std::cout << "Failed to set socket options!" << std::endl;
	}

	localaddr.sin_family = AF_INET;
	localaddr.sin_port = PORT;
	localaddr.sin_addr.s_addr = INADDR_ANY;
	ret = bind(socket_fd,
			   (struct sockaddr *)&localaddr,
			   sizeof(struct sockaddr));
	if (ret == -1)
	{
		std::cout << "Failed to bind addr!" << std::endl;
		exit(-1);
	}

	ret = listen(socket_fd, backlog);
	if (ret == -1)
	{
		std::cout << "Failed to listen socket!" << std::endl;
		exit(-1);
	}

	for (int i = 0; i < MAX_PROCESS; i++)
	{
		//父进程不等于0
		if (pid != 0)
		{
			pid = fork();
		}
	}

	if (pid == 0)
	{
		//create epoll 述符
		epoll_fd = epoll_create(256);
		ev.events = EPOLLIN;
		ev.data.fd = socket_fd;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev);
		for (;;)
		{
			event_number = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
			//std::cout<<"event_number="<<event_number<<std::endl;
			for (int i = 0; i < event_number; i++)
			{
				if (events[i].data.fd == socket_fd)
				{
					std::cout << "listen event" << std::endl;
					socklen_t addr_len = sizeof(struct sockaddr);
					accept_fd = accept(socket_fd, (struct sockaddr *)&remoteaddr, &addr_len);
					//�set nonblock mode
					flags = fcntl(accept_fd, F_GETFL, 0);
					fcntl(accept_fd, F_SETFL, flags | O_NONBLOCK);
					ev.events = EPOLLIN | EPOLLET;
					ev.data.fd = accept_fd;
					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, accept_fd, &ev);
				}
				else if (events[i].events & EPOLLIN)
				{
					std::cout << "enter no-socket port." << std::endl;
					do
					{
						memset(in_buff, 0, MESSAGE_SIZE);
						std::cout << "after clear inbuff:" << in_buff << std::endl;
						ret = recv(events[i].data.fd, &in_buff, MESSAGE_SIZE, 0);
						if (ret == 0)
						{
							//client close its connection
							std::cout << "client close its connection" << std::endl;
							close(events[i].data.fd);
						}
						if (ret == MESSAGE_SIZE)
						{
							std::cout << "More data to process.." << std::endl;
						}
					} while (ret < -1 && errno == EINTR);
					if (ret < 0)
					{
						switch (errno)
						{
						case EAGAIN:
							break;
						default:
							break;
						}
					}
					if (ret > 0)
					{
						std::cout << "recv:" << in_buff << std::endl;
						send(events[i].data.fd, (void *)in_buff, MESSAGE_SIZE, 0);
					}
				}
			}
		}

		close(socket_fd);
	}
	else
	{
		//父进程 pid != 0
		do
		{
			waitpid(-1, NULL, 0)
		} while (pid != -1)
	}
	return 0;
}
