#pragma once

#include <ctime>
#include <queue>
#include "config/config.hpp"
#include "server.hpp"
#include "response.hpp"
#include "request.hpp"

namespace ws {

    class Application {
        Config*             _Config;
        std::vector<Server> _Servers;
        static int const    MAX_SOCKET_FD = 1024;
        size_t              _TotalReadFds;
        size_t              _TotalWriteFds;
        std::queue<int>     _SelectReadWaitlist;
        std::queue<int>     _SelectWriteWaitlist;

        Application( Application const& other);
        Application& operator=( Application const& rhs );
    public:
        Application(): _Config(NULL) {
            _TotalReadFds = 0;
            _TotalWriteFds = 0;
        }

        ~Application() {
            delete _Config;
            // stop all server when quitting
            for (size_t i = 0; i < _Servers.size(); i++) {
                _Servers[i].stop();
            }
        }

        /*
         *  Run application and initialize all servers
         */
        void run( Config* config ) {
            _Config = config;
            _Servers = startServers(config->serverBlocks);
            std::cout << "===[Application running...]===" << std::endl;
            startEngine();
        }

        /*
         *  Runs all servers
         */
        int running = 0;
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
                    if ( FD_ISSET(req.getSockFd(), &copy_read) ) {
                        try {
                            req.readChunk();
                        } catch (std::runtime_error& e) {
                            std::cout << e.what() << std::endl;
                            close(req.getSockFd());
                            FD_CLR(req.getSockFd(), &master_read);
                            _TotalReadFds--;
                            requests.erase(requests.begin() + i);
                            i--;
                            continue;
                        }
                        if (req.getStatus()) {
                            FD_CLR(req.getSockFd(), &master_read);
                            _TotalReadFds--;
                            if (_TotalWriteFds > 1024) {
                                _SelectWriteWaitlist.push(req.getSockFd());
                            } else {
                                FD_SET(req.getSockFd(), &master_write);
                                _TotalWriteFds++;
                            }
                            ServerBlock& serverBlock = findServer(req.getHeader("Host"), req.getHost(), req.getPort()).getServerBlock();
                            responses.push_back(Response(req, serverBlock));
                            requests.erase(requests.begin() + i);
                            i--;
                        }
                    }
                }

                // handling active responses
                for (size_t i = 0; i < responses.size(); i++) {
                    Response &response = responses[i];
                    if (FD_ISSET(response.getSockFd(), &copy_write)) {
                        try {

                            if ( response.isCgiActive() ) {
                                if ( FD_ISSET(response.getCgiFd(), &copy_read) ) {
                                    std::cout << "cgi pipe: " << response.getCgiFd() << std::endl;
                                    response.sendChunk(Response::CGI_PIPE_READY);
                                } else {
                                    response.sendChunk(Response::CGI_SUPERVISE);
                                }
                            }
                            else {
                                response.sendChunk(Response::CGI_OFF);
                            }

                        } catch (Response::CgiProcessStarted &e) {
                            std::cout << e.what() << std::endl;
                            response.startCgiTimeout();
                            if (_TotalReadFds > 1024) {
                                _SelectReadWaitlist.push(e.getCgiFd());
                            } else {
                                FD_SET(e.getCgiFd(), &master_read);
                                _TotalReadFds++;
                            }
                        } catch (Response::CgiProcessTerminated &e) {
                            std::cout << e.what() << std::endl;
                            std::cout << "cgi process terminated" << std::endl;
                            FD_CLR(e.getCgiFd(), &master_read);
                            _TotalReadFds--;
                            response.stopCgi();
                        } catch (std::exception &e) {
                            std::cout << "runtime_error caught" << std::endl;
                            std::cout << e.what() << std::endl;
                            close(response.getSockFd());
                            FD_CLR(response.getSockFd(), &master_write);
                            _TotalWriteFds--;
                            if (response.isCgiActive()) {
                                FD_CLR(response.getCgiFd(), &master_read);
                                _TotalReadFds--;
                                response.stopCgi();
                            }
                            responses.erase(responses.begin() + i);
                            i--;
                            continue;
                        }
                        if (response.done()) {
                            FD_CLR(response.getSockFd(), &master_write);
                            _TotalWriteFds--;
                            if (response.getRequest().getHeader("Connection") == "keep-alive") {
                                std::cout << "resetting connection" << std::endl;
                                response.reset();
                                requests.push_back(response.getRequest());
                                if (_TotalReadFds > 1024) {
                                    _SelectReadWaitlist.push(response.getSockFd());
                                } else {
                                    FD_SET(response.getSockFd(), &master_read);
                                    _TotalReadFds++;
                                }
                            } else {
                                close(response.getSockFd());
                            }
                            responses.erase(responses.begin() + i);
                            i--;
                        }
                    }
                }

                // new connection
                for (size_t i = 0; i < _Servers.size(); i++) {
                    Server& server = _Servers[i];
                    if ( FD_ISSET(server.getSocketFD(), &copy_read)) {
                        int new_socket = accept(server.getSocketFD(), (sockaddr *)(server.getAddress()), server.getAddrlen());
                        if (new_socket > 0) {
                            if (_TotalReadFds > 1024) {
                                _SelectReadWaitlist.push(new_socket);
                                continue;
                            } else {
                                _TotalReadFds++;
                                FD_SET(new_socket, &master_read);
                            }
                            fcntl(new_socket, F_SETFL, O_NONBLOCK);
                            std::string host = server.getServerBlock().host;
                            uint16_t    port = server.getServerBlock().port;
                            requests.push_back(Request(new_socket, host, port));
                        }
                    }
                }


                /*
                 *
                 * Active file descriptors in waitlist into select if there's a place for them of curse
                 *
                 */

                while (!_SelectReadWaitlist.empty()) {
                    if (_TotalReadFds > 1024)
                        break;
                    int fd = _SelectReadWaitlist.front();
                    _SelectReadWaitlist.pop();
                    FD_SET(fd, &master_read);
                    _TotalReadFds++;
                }

                while (!_SelectWriteWaitlist.empty()) {
                    if (_TotalWriteFds > 1024)
                        break;
                    int fd = _SelectWriteWaitlist.front();
                    _SelectWriteWaitlist.pop();
                    FD_SET(fd, &master_write);
                    _TotalWriteFds++;
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
            for (size_t i = 0; i < _Servers.size(); i++) {
                Server& server = _Servers[i];
                for (size_t j = 0; j < server.getServerBlock().hosts.size(); j++) {
                    std::string& serverHost = server.getServerBlock().hosts[j];
                    if (serverHost == hostAttribute) {
                        return server;
                    }
                }
            }

            // search by address:port
            for (size_t i = 0; i < _Servers.size(); i++) {
                Server& server = _Servers[i];
                if (server.getServerBlock().host == host && server.getServerBlock().port == port)
                    return server;
            }

            throw std::logic_error(formatMessage("no server host `%s` were found with `%s:%d`", hostAttribute.c_str(), host.c_str(), port));
        }

        void initServersSockets(fd_set *master_read, fd_set *master_write) {
            FD_ZERO(master_read);
            FD_ZERO(master_write);
            for (size_t i = 0; i < _Servers.size(); i++) {
                FD_SET(_Servers[i].getSocketFD(), master_read);
                _TotalReadFds++;
            }
        }

    };

} // namespace ws
