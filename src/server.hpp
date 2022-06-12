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

namespace ws {

    class Server {
        ServerBlock&        mServerBlock;
        sockaddr_in         mAddress;
        socklen_t           mAddrlen;
        int                 mSfd;
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
            mSfd = -1;
        }

        Server& operator=( Server const& rhs ) {
            if (this != &rhs) {
                mServerBlock = rhs.mServerBlock;
                mAddress = rhs.mAddress;
                mAddrlen = rhs.mAddrlen;
                mSfd = rhs.mSfd;
            }

            return *this;
        }

        void start() {
            mSfd = getSocketFileDescriptor();
        }

        void stop() {
            if (mHaveBind) {
                close(mSfd);
            }
        }

        int getSocketFD() const {
            return mSfd;
        }

        sockaddr_in* getAddress() {
            return &mAddress;
        }

        socklen_t* getAddrlen() {
            return &mAddrlen;
        }

        //TODO: return vector instead
        std::string getHost() const {
            return mServerBlock.getHost();
        }

        ServerBlock& getServerBlock() {
            return mServerBlock;
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

            if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("In socket");
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