#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <string>
#include <iostream>
#include <vector>



class ft_socket
{
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
    int fd = open("test.jpg",O_RDONLY);
    int ret = 0;
    size_t size = 0;
    do {
        ret = read(fd, a + size, BUFSIZ);
        size += ret;
    } while (ret == BUFSIZ);


    ft_socket s1(8080);
    listen_s.push_back(s1.get_fd());
    fd_set master;
    FD_ZERO(&master);
    FD_SET(s1.get_fd(), &master);
    timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    while(1)
    {
        // std::cerr<<"\n+++++++ Waiting for new connection ++++++++\n\n";
        // if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        // {
        //     perror("In accept");
        //     exit(EXIT_FAILURE);
        // }
        fd_set copy = master;
        //std::cout << copy.fds_bits << std::endl;
        select(0,&copy,nullptr,nullptr,&tv);

        for(int i = 0; i < listen_s.size();i++)
        {
            ft_socket &tmp = listen_s[i];

            if (FD_ISSET(tmp.get_fd(),&copy))
            {
                std::cerr << "add client added" << std::endl;
                active.push_back(accept(tmp.get_fd(), (sockaddr *)(tmp.get_address()), tmp.get_addlen()));
            }
        }
        for(int i = 0; i < active.size();i++)
        {
            std::cout << "hello" << std::endl;
            if(FD_ISSET(active[i],&copy))
            {
                char buffer[30000] = {0};
                valread = read( active[i] , buffer, 30000);
                if(valread <= 0)
                {
                    close(active[i]);
                    active.erase(active.begin() + i);
                    break;
                }
                std::string hello = "HTTP/1.1 200 OK\r\nContent-Type: image/jpg\r\nContent-Length: " + std::to_string(size) + "\r\n\r\n";
                char *payload = (char*)malloc(hello.length() + size);
                memset(payload, 0, hello.length() + size);
                memcpy(payload, hello.c_str(), hello.length());
                memcpy(payload + hello.length(), a, size);
                // char *hello = ft
                write(active[i] , payload  , hello.length() + size);
            }
        }




        // char buffer[30000] = {0};
        // valread = read( new_socket , buffer, 30000);
        // std::cerr << buffer <<std::endl;
        // std::string hello = "HTTP/1.1 200 OK\r\nContent-Type: image/jpg\r\nContent-Length: " + std::to_string(size) + "\r\n\r\n";
        // char *payload = (char*)malloc(hello.length() + size);
        // memset(payload, 0, hello.length() + size);
        // memcpy(payload, hello.c_str(), hello.length());
        // memcpy(payload + hello.length(), a, size);
        // // char *hello = ft
        // write(new_socket , payload  , hello.length() + size);

        // std::cerr<<"------------------Hello message sent-------------------"<<std::endl;
        // close(new_socket);
    }
    return 0;
}