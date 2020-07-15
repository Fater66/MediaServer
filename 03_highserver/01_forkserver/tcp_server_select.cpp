#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8111
#define MESSAGE_LEN 1024
#define FD_SIZE 1024
int main(int argc,char* argv[])
{
	int ret = -1;
	int socket_fd,accept_fd;
	int on =1;
	int backlog = 10;
	
	int flags;
	int events = 0;
	//
	int max_fd = -1;
	//fd 文件描述符 
	//fd_set 文件描述符集
	fd_set fd_sets;
	// 同时有fd_size(1024)个连接
	int accept_fds[FD_SIZE] = {-1,};
	int curpos = -1;

	struct sockaddr_in localaddr,remoteaddr;
	
	char in_buff[MESSAGE_LEN] = {0,};
	socket_fd = socket(AF_INET,SOCK_STREAM,0);
	if(socket_fd ==-1){
		std::cout<<"Failed to create socket!"<<std::endl;
		exit(-1);
	}
	//file control
	//fcntl(int fildes, int cmd, ...);
	//F_GETFL            Get descriptor status flags, as described below 
	flags = fcntl(socket_fd,F_GETFL,0);
	//F_SETFL:Set descriptor status flags to arg
	//设置成非阻塞
	fcntl(socket_fd,F_SETFL,flags | O_NONBLOCK);
     	max_fd = socket_fd;

	for(int i = 0;i<FD_SIZE;i++)
	{
		accept_fds[i] = -1;
	}

	ret = setsockopt(socket_fd, SOL_SOCKET,
         SO_REUSEADDR,
         &on,
         sizeof(on));

	if(ret == -1){
		std::cout<<"Failed to set socket options!"<<std::endl;
	}

	localaddr.sin_family = AF_INET;
	localaddr.sin_port = PORT;
     	localaddr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(localaddr.sin_zero),8);
	
	ret = bind(socket_fd,
         (struct sockaddr *)&localaddr,
         sizeof(struct sockaddr));
	if(ret == -1)
	{
		std::cout<<"Failed to bind addr!"<<std::endl;
		exit(-1);
	}
	
	ret = listen(socket_fd,backlog);
	if(ret == -1)
	{
		std::cout<<"Failed to listen socket!" <<std::endl;
		exit(-1);
	}
	for(;;)
	{	
		//清空set
		FD_ZERO(&fd_sets);
		//将侦听的socket加入文件描述符集
		FD_SET(socket_fd,&fd_sets);
		for(int i = 0;i<FD_SIZE;i++)
		{
			if(accept_fds[i] != -1)
			{
				//遍历找到不为-1的文件描述符 加入set
				if(accept_fds[i] > max_fd)
				{	
					max_fd = accept_fds[i];
				}
				FD_SET(accept_fds[i],&fd_sets);
			}
		}
		events = select(max_fd+1,&fd_sets,NULL,NULL,NULL);
		if(events < 0 )
		{
			std::cout<<"Failed to use select" << std::endl;
			break;
		}else if(events == 0)// 超时
		{
			std::cout<<"timeout!..." << std::endl;
			continue;
		}else if(events){
			//如果是侦听的socket触发events,找到set中空槽，创建新的socket
			if(FD_ISSET(socket_fd,&fd_sets)){
				for(int i = 0;i < FD_SIZE;i++)
				{
					if(accept_fds[i] == -1)
					{
						curpos = i;
						break;
					}
				}
			
			socklen_t addr_len = sizeof(struct sockaddr);
			accept_fd = accept(socket_fd,(struct sockaddr *)&remoteaddr,&addr_len);
			//将accept_fd设置成非阻塞
			flags = fcntl(accept_fd,F_GETFL,0);
			fcntl(accept_fd,F_SETFL,flags | O_NONBLOCK);
     			//将当前fd插入文件描述符集对应位置
			accept_fds[curpos] = accept_fd;
			}
			
			for(int i = 0;i<FD_SIZE;i++)
			{
				if(accept_fds[i]!= -1 && FD_ISSET(accept_fds[i],&fd_sets))		
				{
					memset(in_buff,0,MESSAGE_LEN);			
					ret = recv(accept_fds[i], (void *)in_buff, MESSAGE_LEN,0);
				if(ret == 0){
					close(accept_fds[i]);
					break;
				}
	
				std::cout<<"recv:"<<in_buff<<std::endl;
				send(accept_fds[i],(void*)in_buff,MESSAGE_LEN,0);
				}
			}
		}
		
	}

	close(socket_fd);
	return 0;
}
