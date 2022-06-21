#pragma once

#include <sys/time.h>
#include "config/config.hpp"
#include "server.hpp"
#include "response.hpp"
#include "request.hpp"

namespace ws {

    class Application {
        Config*             mConfig;
        std::vector<Server> mServers;
        static int const    MAX_SOCKET_FD = 1024;

        Application( Application const& other);
        Application& operator=( Application const& rhs );
    public:
        Application(): mConfig(NULL) {

        }

        ~Application() {
            delete mConfig;
            // stop all server when quitting
            for (size_t i = 0; i < mServers.size(); i++) {
                mServers[i].stop();
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
            std::vector<Response> responses;
            fd_set master_read, master_write;
            initServersSockets(&master_read, &master_write);

            while (true) {
                fd_set copy_read = master_read;
                fd_set copy_write = master_write;
                int ret = select(MAX_SOCKET_FD, &copy_read, &copy_write, NULL, NULL);
                if (ret == -1) {
                    std::cout << "select: " << strerror(errno) << std::endl;
                    sleep(1);
                    continue;
                }

                // handling active requests
                for (size_t i = 0; i < requests.size(); i++) {
                    Request& req = requests[i];

                    if ( FD_ISSET(req.getSockFd(), &copy_read) )
                    {
                        try {
                            req.readChunk();
                        } catch (std::runtime_error& e) {
                            std::cout << e.what() << std::endl;
                            close(req.getSockFd());
                            FD_CLR(req.getSockFd(), &master_read);
                            requests.erase(requests.begin() + i);
                            i--;
                            continue;
                        }

                        if(req.getStatus())
                        {
                            FD_CLR(req.getSockFd(), &master_read);
                            FD_SET(req.getSockFd(), &master_write);
                            ServerBlock& serverBlock = findServer(req.getHeader("Host"), req.getHost(), req.getPort()).getServerBlock();
                            responses.push_back(Response(req, serverBlock));
                            requests.erase(requests.begin() + i);
                            i--;
                        }
                    }
                }

                // handling active responses
                for (size_t i = 0; i < responses.size(); i++) {
                    Response& response = responses[i];

                    if ( FD_ISSET(response.getSockFd(), &copy_write) ) {
                        try {
                            response.send();
                        } catch (std::runtime_error& e) {
                            std::cout << e.what() << std::endl;
                            close(response.getSockFd());
                            FD_CLR(response.getSockFd(), &master_write);
                            responses.erase(responses.begin() + i);
                            i--;
                            continue;
                        }
                        if (response.done()) {
                            FD_CLR(response.getSockFd(), &master_write);
                            if (response.getRequest().getHeader("Connection") == "keep-alive") {
                                std::cout << "resetting connection" << std::endl;
                                response.reset();
                                requests.push_back(response.getRequest());
                                FD_SET(response.getSockFd(), &master_read);
                            } else {
                                close(response.getSockFd());
                            }
                            responses.erase(responses.begin() + i);
                            i--;
                        }

                    }
                }

                // new connection
                for (size_t i = 0; i < mServers.size(); i++) {
                    Server& server = mServers[i];

                    if ( FD_ISSET(server.getSocketFD(), &copy_read)) {
                        int new_socket = accept(server.getSocketFD(), (sockaddr *)(server.getAddress()), server.getAddrlen());
                        fcntl(new_socket, F_SETFL, O_NONBLOCK); //TODO: maybe useless ?
                        if ( new_socket > 0 ) {
                            std::string host = server.getServerBlock().host;
                            uint16_t    port = server.getServerBlock().port;
                            requests.push_back(Request(new_socket, host, port));
                            FD_SET(new_socket,&master_read);
                        }

                    }

                }

            } // infinity loop

        }


    private:

        std::vector<Server> startServers( std::vector<ServerBlock>& serverBlocks ) const {
            std::vector<Server> servers;
            std::map<std::pair<std::string, uint16_t>, int> socketPool;

            for (size_t i = 0; i < serverBlocks.size(); i++) {
                ServerBlock& serverBlock = serverBlocks[i];
                std::pair<std::string, uint16_t> addressPort = std::make_pair(serverBlock.host, serverBlock.port);
                std::map<std::pair<std::string, uint16_t>, int>::const_iterator cit = socketPool.find(addressPort);
                if (cit == socketPool.end()) {
                    Server server(serverBlock);
                    server.start();
                    servers.push_back(server);
                    socketPool.insert(std::make_pair(addressPort, server.getSocketFD()));
                } else {
                    Server server(serverBlock);
                    server.setSocketFD((*cit).second);
                    servers.push_back(server);
                }
            }

            return servers;
        }

        Server& findServer( std::string const& hostAttribute , std::string host, uint16_t port ) {
            // search by server_name:port
            for (size_t i = 0; i < mServers.size(); i++) {
                Server& server = mServers[i];
                for (size_t j = 0; j < server.getServerBlock().hosts.size(); j++) {
                    std::string& serverHost = server.getServerBlock().hosts[i];
                    if (serverHost == hostAttribute) {
                        return server;
                    }
                }
            }

            // search by address:port
            for (size_t i = 0; i < mServers.size(); i++) {
                Server& server = mServers[i];
                if (server.getServerBlock().host == host && server.getServerBlock().port == port)
                    return server;
            }

            throw std::logic_error(formatMessage("no server host `%s` were found with `%s:%d`", hostAttribute.c_str(), host.c_str(), port));
        }

        void initServersSockets(fd_set *master_read, fd_set *master_write) const {
            FD_ZERO(master_read);
            FD_ZERO(master_write);
            for (size_t i = 0; i < mServers.size(); i++) {
                FD_SET(mServers[i].getSocketFD(), master_read);
            }
        }

    };

} // namespace ws
