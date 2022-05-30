#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <vector>

#define PORT 8081

class Server {
    private:
        sockaddr_in address;
        socklen_t addlen;
        int fd;
        ft_socket(void){};
    public:
        ft_socket(int port)
        {
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons( port );
            memset(address.sin_zero, '\0', sizeof address.sin_zero);
            addlen = sizeof(address);
            int opt = 1;
            if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
            {
                perror("In socket");
                exit(EXIT_FAILURE);
            }
            if (bind(fd, (sockaddr *)&address, sizeof(address))<0)
            {
                perror("In bind");
                exit(EXIT_FAILURE);
            }
            if (listen(fd, 1024) < 0)
            {
                perror("In listen");
                exit(EXIT_FAILURE);
            }
            fcntl(fd, F_SETFL, O_NONBLOCK);
        }
        int get_fd()
        {
            return fd;
        }

        sockaddr_in *get_address()
        {
            return &address;
        }

        socklen_t *get_addlen()
        {
            return &addlen;
        }
};

int main(int argc, char const *argv[])
{
    long valread;
    std::vector<int> active;
    std::vector<ft_socket> listen_s;
 
	char a[200000] = "";
    int fd = open("index.html",O_RDONLY);
    int ret = 0;
    size_t size = 0;
    do {
        ret = read(fd, a + size, BUFSIZ);
        size += ret;
    } while (ret == BUFSIZ);

    ft_socket s1(PORT);
    listen_s.push_back(s1.get_fd());
    fd_set master;
    FD_ZERO(&master);
    FD_SET(s1.get_fd(), &master);
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    while(1)
    {

        fd_set copy = master;
        select(0,&copy,nullptr,nullptr,&tv);
        if (FD_ISSET(s1.get_fd(),&copy))
        {
            int new_socket = accept(s1.get_fd(), (sockaddr *)(s1.get_address()), s1.get_addlen());
            if(new_socket > 0)
            {
                active.push_back(new_socket);
                FD_SET(new_socket,&master);
            }
        }
        int i = 0;
        while(i < active.size())
        {
            if(FD_ISSET(active[i],&copy))
            {
                
                char buffer[30000];
                valread = read( active[i] , buffer, 30000);
                std::cout << active[i] << std::endl;
                std::cout << "buffer: " << buffer << std::endl;
                std::string hello = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(size) + "\r\n\r\n";
                char *payload = (char*)malloc(hello.length() + size);
                memset(payload, 0, hello.length() + size);
                memcpy(payload, hello.c_str(), hello.length());
                memcpy(payload + hello.length(), a, size);
                // char *hello = ft
                write(active[i] , payload  , hello.length() + size);
                free(payload);
                active.erase(active.begin() + i);
                FD_CLR(active[i],&copy);
                //std::cout << "after erase " << active.size() << std::endl;
            }
            else
                i++;
        }
    }
    return 0;
}