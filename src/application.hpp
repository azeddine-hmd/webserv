#pragma once

#include <sys/time.h>
#include "config/config.hpp"
#include "request.hpp"
#include "server.hpp"
#include "Call.hpp"
#include <set>

namespace ws {

    class Application {
        Config*             mConfig;
        std::vector<Server> mServers;
        std::set<int>       mTmpSockets;

        static int const    MAX_SOCKET_FD = 1024;

        Application( Application const& other);
        Application& operator=( Application const& rhs );
    public:
        Application(): mConfig(NULL) {

        }

        ~Application() {
            delete mConfig;
            for (size_t i = 0; i < mServers.size(); i++) {
                mServers[i].stop();
            }

            //TODO: debugging
            std::set<int>::iterator it = mTmpSockets.begin();
            for (; it != mTmpSockets.end(); it++) {
                std::cout << "closing request socket: " << *it << std::endl;
                close(*it);
            }
        }

        /*
         *  Run application and initialize all servers
         */
        void run( Config* config ) {
            mConfig = config;
            mServers = startServers(config->serverBlocks);
            std::cout << "===[Application running...]===" << std::endl;
            startEngine();
        }

        /*
         *  Runs all servers
         */
        void startEngine() {
            std::vector<Request> requests;
            std::vector<ResponseBuilder> responses;
            fd_set master_read, master_write, master_error;
            initServersSockets(&master_read, &master_write, &master_error);

            while (true) {
                fd_set copy_read = master_read;
                fd_set copy_write = master_write;
                fd_set copy_error = master_error;
                int ret = select(MAX_SOCKET_FD, &copy_read, &copy_write, &copy_error, NULL);
                if (ret == -1) {
                    std::cout << "select: " << strerror(errno) << std::endl;
                    sleep(1000);
                    continue;
                }

                // handling active requests
                for (size_t i = 0; i < requests.size(); i++) {
                    Request& req = requests[i];

                    if ( FD_ISSET(req.getFd(), &copy_read) )
                    {
                        req.readChunk();
                        if(req.getStatus())
                        {
                            FD_CLR(req.getFd(), &master_read);
                            FD_SET(req.getFd(), &master_write);

//                            Server * server = findServer(req.getHeader("Host"), req.getHost(), req.getPort());
//                            ServerBlock& serverBlock = server->getServerBlock();

                            responses.push_back(ResponseBuilder(req, /*TODO: remove it*/ mServers[0].getServerBlock()));

                            requests.erase(requests.begin() + i);
                            i--;
                        }
                    }
                }

                // handling active responses
                for (size_t i = 0; i < responses.size(); i++) {
                    ResponseBuilder& response = responses[i];

                    if ( FD_ISSET(response.getResponseFd(), &copy_write) ) {
                        response.sendShunk();
                        if (response.isFinish()) {
                            FD_CLR(response.getResponseFd(), &master_write);
                            // in case of request have keep-alive attribute
                            if (response.getRequest().getHeader("Connection") == "keep-alive") {
                                response.reset();
                            } else {
                                std::cout << std::endl << "response delivered" << std::endl;
                                close(response.getResponseFd());
                                mTmpSockets.erase( response.getResponseFd() );
                                responses.erase(responses.begin() + i);
                                i--;
                            }
                        }

                    }
                }

                // new connection
                for (size_t i = 0; i < mServers.size(); i++) {
                    Server& server = mServers[i];

                    if ( FD_ISSET(server.getSocketFD(), &copy_read)) {
//                        if (socketAlreadyExist())
//                            continue;

                        int new_socket = accept(server.getSocketFD(), (sockaddr *)(server.getAddress()), server.getAddrlen());
                        std::cout << "new_scoket: " << new_socket << std::endl;
                        if ( new_socket > 0 ) {
                            std::string host = server.getServerBlock().host;
                            uint16_t    port = server.getServerBlock().port;
                            requests.push_back(Request(new_socket, host, port));
                            mTmpSockets.insert(new_socket);
                            std::cout << "inserting: " << new_socket << std::endl;
                            FD_SET(new_socket,&master_read);
                        }

                    }

                }

            } // infinity loop

        }


    private:

        std::vector<Server> startServers( std::vector<ServerBlock>& serverBlocks ) const {
            std::vector<Server> servers;

            for (size_t i = 0; i < serverBlocks.size(); i++) {
                Server server(serverBlocks[i]);
                //TODO: don't start server that already have same socket with other server
                server.start();
                servers.push_back(server);
            }

            return servers;
        }

        //TODO: this function still have many bugs right now avoid calling it
        Server* findServer( std::string const& hostAttribute , std::string host, uint16_t port ) {
            for (size_t i = 0; i < mServers.size(); i++) {
                if (mServers[i].getHost() == hostAttribute) {
                    return &mServers[i];
                }
            }

            //TODO: continue
            // if not server name matches select default server
            for (size_t i = 0; i < mServers.size(); i++) {
                if (mServers[i].getHost() == host) {
                    return &mServers[i];
                }
            }

            return NULL;
        }

        void initServersSockets(fd_set *master_read, fd_set *master_write, fd_set *master_error) const {
            FD_ZERO(master_read);
            FD_ZERO(master_write);
            FD_ZERO(master_error);
            for (size_t i = 0; i < mServers.size(); i++) {
                FD_SET(mServers[i].getSocketFD(), master_read);
                FD_SET(mServers[i].getSocketFD(), master_error);
            }
        }

    };

} // namespace ws
