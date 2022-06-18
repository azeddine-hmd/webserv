#ifndef REQBUILD_HPP
#define REQBUILD_HPP

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdlib.h>
#include "mimeTypes.hpp"
#include "request.hpp"

namespace ws {

    class ResponseBuilder {
        std::string         _ResponseHeader;
        Request             _req;
        bool                _HeaderSent;
        ServerBlock&        mServerBlock;
        bool                mFinish;

    public:
        ResponseBuilder( Request& request, ServerBlock& serverBlock ): _req(request), mServerBlock(serverBlock) {
            _req = request;
            _HeaderSent = false;
            mFinish = false;
        }

        ResponseBuilder& operator=( ResponseBuilder const& rhs ) {
            if (this != &rhs ) {
                _ResponseHeader = rhs._ResponseHeader;
                _req = rhs._req;
                _HeaderSent = rhs._HeaderSent;
                mServerBlock = rhs.mServerBlock;
            }

            return *this;
        }

        off_t getContentLength(int fd) {
            return (lseek(fd, 0, SEEK_END));
        }
        //HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 11\r\n\r\nhello world

        void sendHeader(int fd) {

        }

        void sendShunk() {
            std::string path = "www";

            path += _req.getHeader("Path");
            int file = open(path.c_str(), O_RDONLY);
            std::string fileS = std::to_string(getContentLength(file));
            //std::cout << path << ": " << file<< std::endl;
            // std::string response = std::string("HTTP/1.1 200 OK\r\nContent-Type: ")
            // + MimeTypes::getType(path.c_str()) + std::string("\r\nContent-Length: ")
            // + std::to_string(getContentLength()) + std::string("\r\n\r\n");
            const char *mime = MimeTypes::getType(path.c_str());
            write(_req.getFd(), _req.getHeader("Version").c_str(), _req.getHeader("Version").size());
            write(_req.getFd(), " 200 OK\r\nContent-Type: ", 23);
            write(_req.getFd(), mime, strlen(mime));
            write(_req.getFd(), "\r\nContent-Length: ", 18);
            write(_req.getFd(), fileS.c_str(), fileS.length());
            write(_req.getFd(), "\r\n\r\n", 4);
            char buffer[1024] = "";
            int ret = 0;
            close(file);
            file = open(path.c_str(), O_RDONLY);
            while ((ret = read(file, buffer, 1024)) > 0) {
                write(_req.getFd(), buffer, 1024);
            }
            write(_req.getFd(), "\r\n\r\n", 4);
            close(file);
            // write(_req.getFd(), )
            // if(!_HeaderSent)
            //     sendHeader(file);
            mFinish = true;
        }

        int getResponseFd() const {
            return _req.getFd();
        }

        bool isFinish() const {
            return mFinish;
        }

        Request& getRequest() {
            return _req;
        }

        void reset() {
            _req.reset();
            _ResponseHeader.clear();
            _HeaderSent = false;
            mFinish = false;
        }

    };

} // namespace ws

#endif