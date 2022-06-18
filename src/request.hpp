#ifndef REQ_HPP
#define REQ_HPP

#include <map>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <cstdlib>
#include "chunkedDecoder.hpp"
#include "utils.hpp"

#define BUFFER_SIZE 1024

namespace ws {

    class Request {
        int                                 _SockFd; // socket file descriptor
        std::map<std::string, std::string>  _Headers; // map contains all the headers sent by the client including the method, path, http version
        std::string                         _BodyBuffer; // body :)
        bool                                _HeaderDone; // true after parsing the headers
        bool                                _RequestDone; // true when req is done
        BodyFile                            _BodyFile;
        size_t                              _bodySize;
        size_t                              _targetSize;
        ChunkedDecoder                      _chunkedDecoder;

        //TODO: is there other way to identify host and port
        std::string                         mHost;
        uint16_t                            mPort;

    public:
        Request( int fd, std::string host, uint16_t port ): mHost(host), mPort(port) {
            _SockFd = fd;
            _HeaderDone = false;
            _RequestDone = false;
            _BodyBuffer = "";
            _BodyFile.fd = -1;
        }

        ~Request() {

        }

        // parse the first line of a request example: ()
        void parseFirstLine() {
            std::string str = ws::getNextLine(_BodyBuffer);
            std::stringstream ss(str);
            ss >> _Headers["Method"];
            ss >> _Headers["Path"];
            ss >> _Headers["Version"];
        }

        // parse one line that contains one header example: (Content-Length: 531)
        int parseParam() {
            std::string str = ws::getNextLine(_BodyBuffer);
            if (str.size() == 0)
                return -1;
            std::stringstream ss(str);
            std::string key;
            ss >> key;
            key.pop_back();
            ss >> _Headers[key];
            return 0;
        }

        // parse the whole header part of a request
        void parseRequestHeader() {
            char buf[BUFFER_SIZE * 10];
            int ret = read(_SockFd, buf, BUFFER_SIZE * 10);
            if (ret <= 0) {
                throw std::runtime_error("error while reading");
            }

            _BodyBuffer = std::string(buf, ret);
            parseFirstLine();

            while (parseParam() != -1);
            if (_Headers.find("Content-Length") != _Headers.end()) {
                std::istringstream iss(_Headers["Content-Length"]);
                iss >> _targetSize;
            }
            _HeaderDone = true;
            if (_Headers["Method"] != "POST") {
                _RequestDone = true;
                lseek(_SockFd, 0, SEEK_END); // ignore body if method is get or delete
            } else {
                CreateFile();
                if (_Headers["Transfer-Encoding"] == "chunked") {
                    _chunkedDecoder.SetFile(_BodyFile);
                    _chunkedDecoder.decode(_BodyBuffer);
                }
                write(_BodyFile.fd, _BodyBuffer.c_str(), _BodyBuffer.size());
                _bodySize = _BodyBuffer.size();
            }
        }

        // get a parameter by key
        std::string getHeader( std::string key ) {
            return _Headers[key];
        }

        // get a parameter by key
        std::string getBody() {
            return _BodyBuffer;
        }

        // true if the request is done else false;
        bool getStatus() {
            return _RequestDone;
        }

        int getFd() const {
            return _SockFd;
        }

        void CreateFile() {
            int random = (int) time(nullptr);
            std::string n = std::string("USER_") + std::to_string(random);
            _BodyFile.fd = open(n.c_str(), O_CREAT | O_WRONLY, 0644);
            _BodyFile.name = n;
        }

        void readChunk() {
            if (_HeaderDone) {
                char buf[BUFFER_SIZE];
                int ret = read(_SockFd, buf, BUFFER_SIZE);
                if (ret <= 0)
                    throw std::runtime_error("error while reading");
                std::string buffer = std::string(buf, ret);
                _bodySize += ret;
                // if(ret > 0)
                // {
                // 	if(_BodyFile.fd == -1)
                // 		CreateFile();
                // 	write(_BodyFile.fd, buf, ret);
                // }
                if (_Headers["Transfer-Encoding"] == "chunked") {
                    if (!_chunkedDecoder.decode(buffer)) {
                        _RequestDone = true;
                        std::cout << "done" << std::endl;
                        return;
                    }
                } else {
                    if (ret > 0)
                        write(_BodyFile.fd, buf, ret);
                    if (_bodySize == _targetSize) {
                        _RequestDone = true;
                        std::cout << "done" << std::endl;
                        return;
                    }
                }
            } else {
                parseRequestHeader();
            }
        }

        void reset() {
            _HeaderDone = false;
            _RequestDone = false;
            _BodyBuffer = "";
            _BodyFile.fd = -1;
            _Headers.clear();
            _BodyBuffer.clear();
            _bodySize = 0;
            _targetSize = 0;
        }

        std::string getHost() const {
            return mHost;
        }

        uint16_t getPort() const {
            return mPort;
        }

    };

} // namespace ws

#endif