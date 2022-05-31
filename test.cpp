#include "request.hpp"
#include<stdio.h> 
#include<fcntl.h> 
#include<errno.h> 

int main()
{
    int fd = open("test.txt", O_RDONLY); 
    Request ParseRequest(fd);
    ParseRequest.parseFirstLine();

    return (0);
}



// read a big buffer ... then create a stringstream from the result of read now you can getline from it