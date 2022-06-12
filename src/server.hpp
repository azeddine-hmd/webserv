#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string>
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