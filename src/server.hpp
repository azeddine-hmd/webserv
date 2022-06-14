#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string>
#include <arpa/inet.h>
#include "responseBuilder.hpp"
#include "config/config_model.hpp"

#define PORT 8080

namespace ws {

    class Server {
        ServerBlock&        mServerBlock;
        sockaddr_in         mAddress;
        socklen_t           mAddrlen;
        int                 mSfd;
        std::string         mHost;

        static const int    MAX_LISTENERS = 1024;

        Server();
    public:
        Server( ServerBlock& serverBlock ): mServerBlock(serverBlock) {
            mAddress = getSocketAddress();
            if ((mSfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("In socket");
                exit(EXIT_FAILURE);
            }

            fcntl(mSfd, F_SETFL, O_NONBLOCK);

            if (bind( mSfd, (sockaddr*)&mAddress, sizeof(mAddress) ) < 0) {
                close(mSfd);
                perror("In bind");
                exit(EXIT_FAILURE);
            }

            if (listen(mSfd, MAX_LISTENERS) < 0) {
                perror("In listen");
                exit(EXIT_FAILURE);
            }

            mHost = mServerBlock.serverNames.front() + ":" + std::to_string(mServerBlock.port);
            //TODO: continue

        }

        int getFd() const {
            return mSfd;
        }

        sockaddr_in* getAddress() {
            return &mAddress;
        }

        socklen_t* getAddrlen() {
            return &mAddrlen;
        }

        std::string const& getHost() const {
            return mHost;
        }

    private:
        sockaddr_in getSocketAddress() {
            sockaddr_in socketAddress;
            socketAddress.sin_family = AF_INET;
            socketAddress.sin_addr.s_addr = inet_addr(mServerBlock.host.c_str());
            socketAddress.sin_port = htons(mServerBlock.port);
            memset(socketAddress.sin_zero, 0, sizeof socketAddress.sin_zero);
            return socketAddress;
        }

    };

} // namespace ws