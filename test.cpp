#include <iostream>

int main(int argc,char* argv[])
{
    int port = 8111;
    std::cout<<port<<std::endl;
    std::cout<<htons(port)<<std::endl;
    
    return 0;
}