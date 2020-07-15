#include <iostream>

#include <unistd.h>
#include <stdlib.h>

int main(int argc,char* argv[])
{
	//转后台失败返回1 0，0参数 不改变目录
	if(daemon(0,0) == -1){
		std::cout<<"error\n" <<std::endl;
		exit(-1);
	}
		
	while(1){
		sleep(1);
	}
}
