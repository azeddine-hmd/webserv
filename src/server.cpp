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

#define PORT 8080

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
    fd_set master;
    FD_ZERO(&master);
    FD_SET(s1.getFd(), &master);
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    while(1)
    {
        fd_set copy = master;
        select(0,&copy,nullptr,nullptr,&tv);
        if (FD_ISSET(s1.getFd(),&copy))
        {
            int new_socket = accept(s1.getFd(), (sockaddr *)(s1.getAddress()), s1.getAddrlen());
            if(new_socket > 0)
            {
                active.push_back(Request(new_socket));
                FD_SET(new_socket,&master);
            }
        }
        int i = 0;
        while(i < active.size())
        {
            if(FD_ISSET( active[i].getFd(), &copy ))
            {
                active[i].readChunk();
                if(active[i].getStatus())
                {
                    ResponseBuilder rb(active[i]);
                    rb.sendShunk();
                    active.erase(active.begin() + i);
                    FD_CLR(active[i].getFd(),&master);
                    close(active[i].getFd()); 
                }
            }
            i++;
        }
    }
    return 0;
}