#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string>
#include <arpa/inet.h>
#include "config/config_model.hpp"

namespace ws {

    class Server {
        ServerBlock&        mServerBlock;
        sockaddr_in         mAddress;
        socklen_t           mAddrlen;
        int                 mSocketFD;
        bool                mHaveBind;

        static const int    MAX_LISTENERS = 1024;

        Server();

    public:

        /*
         *  creates server then binds and listen
         */
        Server( ServerBlock& serverBlock ): mServerBlock(serverBlock) {
            mAddress = getSocketAddress();
            mAddrlen = sizeof(sockaddr_in);
            mHaveBind = false;
            mSocketFD = -1;
        }

        void start() {
            mSocketFD = getSocketFileDescriptor();
        }

        void stop() {
            if (mHaveBind) {
                close(mSocketFD);
            }
        }

        int getSocketFD() const {
            return mSocketFD;
        }

        sockaddr_in* getAddress() {
            return &mAddress;
        }

        socklen_t* getAddrlen() {
            return &mAddrlen;
        }

        ServerBlock& getServerBlock() {
            return mServerBlock;
        }

        void setSocketFD(int socketFD) {
            mSocketFD = socketFD;
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

        int getSocketFileDescriptor() {
            int sfd;
            int opt = 1;

            if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("In socket");
                exit(EXIT_FAILURE);
            }

            if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1)
            {
                close(sfd);
                perror("In setsockopt");
                exit(EXIT_FAILURE);
            }

            if (bind(sfd, (sockaddr*)&mAddress, sizeof(mAddress)) < 0) {
                close(sfd);
                perror("In bind");
                exit(EXIT_FAILURE);
            }

            mHaveBind = true;

            if (listen(sfd, MAX_LISTENERS) < 0) {
                close(sfd);
                perror("In listen");
                exit(EXIT_FAILURE);
            }

            fcntl(sfd, F_SETFL, O_NONBLOCK);

            return sfd;
        }

    };

} // namespace ws