/**
 * Server class
 *
 * @author yutong
 * @date 2020-07-12
 * @coptleft GPL 2.0
 */

#include <iostream>
#include <unistd.h>

#include "server.h"

namespace avdance{

Server::Server(){
	std::cout << "construct..." <<std::endl;
}

Server::~Server(){
	std::cout<<"destuct..." <<std::endl;
}

void Server::run(){
	while(1)
	{
		std::cout << "running..." <<std::endl; 
		::usleep(1000000);//sleep one second
	}
}

}
