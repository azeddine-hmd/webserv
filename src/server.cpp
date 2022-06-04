#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <vector>

#include "request.hpp"
#include "responseBuilder.hpp"

#define PORT 8081

class Server {
    private:
        sockaddr_in _address;
        socklen_t _addrlen;
        int _fd;
        bool _haveBind;
        static const int MAX_LISTENERS = 1024;

    private:
        Server() {

        }

        sockaddr_in getSocketAddress( int port ) {
            sockaddr_in socketAddress;
            socketAddress.sin_family = AF_INET;
            socketAddress.sin_addr.s_addr = INADDR_ANY;
            socketAddress.sin_port = htons( port );
            memset(socketAddress.sin_zero, 0, sizeof socketAddress.sin_zero);
            return socketAddress;
        }

    public:
        Server( int port ): _haveBind(false) {
            _address = getSocketAddress(port);
            _addrlen = sizeof(_address);

            if ((_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("In socket");
                exit(EXIT_FAILURE);
            }

            if (bind(_fd, (sockaddr *)&_address, sizeof(_address)) < 0)
            {
                close(_fd);
                perror("In bind");
                exit(EXIT_FAILURE);
            }
            _haveBind = true;

            if (listen(_fd, MAX_LISTENERS) < 0)
            {
                perror("In listen");
                exit(EXIT_FAILURE);
            }

            fcntl(_fd, F_SETFL, O_NONBLOCK);
        }

        int getFd() {
            return _fd;
        }

        sockaddr_in* getAddress() {
            return &_address;
        }

        socklen_t* getAddrlen() {
            return &_addrlen;
        }
};

void closePort(int sig)
{
    close(PORT);
    exit(0);
}


int main(int argc, char const *argv[])
{
    long valread;
    std::vector<Request> active;
    std::vector<Server> listen_s;

    signal(SIGINT, closePort);
    Server s1(PORT);
    listen_s.push_back(s1.getFd());
    fd_set master_read, master_write;
    FD_ZERO(&master_read);
    FD_ZERO(&master_write);
    FD_SET(s1.getFd(), &master_read);
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    while(1)
    {
        fd_set copy_read = master_read;
        fd_set copy_write = master_write;
        select(1024, &copy_read, &copy_write, nullptr, &tv);
        int i = 0;
        while(i < active.size())
        {
            if(FD_ISSET( active[i].getFd(), &copy_read ))
            {
                active[i].readChunk();
                if(active[i].getStatus())
                {
                    FD_CLR(active[i].getFd(),&master_read);
                    FD_SET(active[i].getFd(), &master_write);
                }
            }
            if (FD_ISSET( active[i].getFd(), &copy_write))
            {
                ResponseBuilder rb(active[i]);
                rb.sendShunk();
                FD_CLR(active[i].getFd(),&master_write);
                close(active[i].getFd());
                active.erase(active.begin() + i);
                i--;
            }
            i++;
        }
        if (FD_ISSET(s1.getFd(),&copy_read))
        {
            int new_socket = accept(s1.getFd(), (sockaddr *)(s1.getAddress()), s1.getAddrlen());
            if(new_socket > 0)
            {
                active.push_back(Request(new_socket));
                FD_SET(new_socket,&master_read);
            }
        }
    }
    return 0;
}