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

int main(int argc, char const *argv[])
{
    long valread;
    std::vector<int> active;
    std::vector<Server> listen_s;
 
	char a[200000] = "";
    int fd = open("index.html",O_RDONLY);
    int ret = 0;
    size_t size = 0;
    do {
        ret = read(fd, a + size, BUFSIZ);
        size += ret;
    } while (ret == BUFSIZ);

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
                std::cout << buffer << std::endl;
                std::string hello = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(size) + "\r\n\r\n";
                char *payload = (char*)malloc(hello.length() + size);
                bzero(payload, hello.length() + size);
                memset(payload, 0, hello.length() + size);
                memcpy(payload, hello.c_str(), hello.length());
                memcpy(payload + hello.length(), a, size);
                // char *hello = ft
                write(active[i] , payload  , hello.length() + size);
                free(payload);
                active.erase(active.begin() + i);
                FD_CLR(active[i],&copy);
            }
            else
                i++;
        }
    }
    return 0;
}


//file b random name copy system(mv 566 /uploads/)

/*

GET /hello/fsdfdsfdsfdsf HTTP/1.1
Host: 127.0.0.1:8081
Connection: keep-alive
sec-ch-ua: " Not A;Brand";v="99", "Chromium";v="101", "Google Chrome";v="101"
sec-ch-ua-mobile: ?0
sec-ch-ua-platform: "macOS"
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/101.0.4951.64 Safari/537.36\r\n
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,imag

e/apng,;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n
Sec-Fetch-Site: none
Sec-Fetch-Mode: navigate
Sec-Fetch-User: ?1
Sec-Fetch-Dest: document
Accept-Encoding: gzip, deflate, br

*/