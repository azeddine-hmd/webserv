#pragma once

#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <poll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "mimeTypes.hpp"
#include "request.hpp"
#include "config/config_model.hpp"
#include "cgi.hpp"
#include "headers.hpp"


namespace ws {


    class Response {
        typedef std::pair<std::string, std::string>         pair;
        typedef std::map<int, std::string>::const_iterator  ErrPgIter;

        std::string     _Body;
        std::string     _Date;

        Request        _req;
        bool           _HeadersSent;
        int             _BodyFd;
        bool            _Done;
        ServerBlock*    _ServerBlock;

        Response();
    public:

        Response( Request& request, ServerBlock& serverBlock ): _req(request), _ServerBlock(&serverBlock) {
            _HeadersSent = false;
            _BodyFd = -1;
            _Done = false;
        }

    private:

        /*
            https://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html
            https://www.ibm.com/docs/en/cics-ts/5.2?topic=protocol-http-responses
        */

//        std::string setDate(void) {
//            char buffer[30];
//            struct timeval tv;
//
//            gettimeofday(&tv, NULL);
//            strftime(buffer, 29, "%a, %d %b %Y %T %Z", gmtime(&tv.tv_sec));
//            return (std::string(buffer));
//        }

//        int IsFile(std::string &Path) {
//            struct stat status;
//            if (stat(Path.c_str(), &status) == 0) {
//                if (status.st_mode & S_IFDIR)
//                    return 0;
//                else if (status.st_mode & S_IFREG)
//                    return 1;
//                else
//                    return 0;
//            } else
//                return 0;
//        }

//        void    BuildError( std::string  Error) {
//            std::string	    StatusCode = Error.substr(0, 3);
//            std::string     FilePath =  "resources/error_pages/" + StatusCode + ".html";
//            int             fd = open(FilePath.c_str(), O_RDONLY);
//            struct stat     FileStatus;
//
//            std::cout << "build Error: " << FilePath << std::endl;
//            fstat(fd, &FileStatus);
//            _Headers += "HTTP/1.1 " + Error + "\r\n" + _Date;
//            _Headers += "Content-Type: text/html\r\n";
//            _Headers += "Content-Length: " + ToString(FileStatus.st_size) + "\r\n";
//            _Headers += "Connection: closed\r\n\r\n";
//
//            write(_Fd, _Headers.c_str(), _Headers.length());
//            char buffer[1024] = "";
//            while(read(fd, buffer, 1024) > 0)
//                write(_Fd, buffer, 1024);
//            write(_Fd, "\r\n\r\n", 4);
//            close(fd);
//        }

//        void Build(void) {
//            if (_req.getHeader("Path").find(".php") != std::string::npos
//                || _req.getHeader("Path").find(".py") != std::string::npos) {
//                std::cout << "in cgi" << std::endl;
//                pair status = _Cgi.execute(); // return pair(string status code, string status msg)
//                //cgi can eather return content or status codes (400 || 500 || 301 || 302)
//                // should build Error
//                // else if (redirection for later)
//                _Headers += status.second + _Date;
//                // _Headers += "Content-Length:" + this->ToString(status.first.length()) + "\r\n";
//                write(_Fd, _Headers.c_str(), _Headers.length());
//                write(_Fd, status.first.c_str(), status.first.length());
//                return;
//            } else if (_req.getHeader("Method").length()) {
//                std::string Method = _req.getHeader("Method");
//                std::string FilePath = "www" + _req.getHeader("Path");
//
//                if (Method == "GET") {
//                    std::cout << "inside GET" << std::endl;
//                    struct stat FileStatus;     //https://linux.die.net/man/2/stat
//                    int fd;
//
//                    if (stat(FilePath.c_str(), &FileStatus) < 0) {
//                        std::cout << "File Protection | Errno: " << errno << std::endl;
//                        if (errno ==
//                            EACCES)    //Search permission is denied for one of the directories in the path prefix of path
//                            BuildError("403 Forbidden");
//                        else if (errno == ENOENT)    //A component of path does not exist, or path is an empty string.
//                            BuildError("404 Not Found");
//                    } else {
//                        const char *Type = MimeTypes::getType(FilePath.c_str());
//                        _Headers += "HTTP/1.1 200 OK\r\n" + _Date;
//                        if (Type == NULL)
//                            _Headers += "Content-Type: text/plain\r\n";
//                        else
//                            _Headers += "Content-Type: " + ToString(Type) + "\r\n";
//                        _Headers += "Content-Length: " + ToString(FileStatus.st_size) + "\r\n";
//                        _Headers += "Connection: " + _req.getHeader("Connection") + "\r\n\r\n";
//
//                        fd = open(FilePath.c_str(), O_RDONLY);
//                        write(_Fd, _Headers.c_str(), _Headers.length());
//                        char buffer[1024] = "";
//                        while (read(fd, buffer, 1024) > 0)
//                            write(_Fd, buffer, 1024);
//                        write(_Fd, "\r\n\r\n", 4);
//                        close(fd);
//                    }
//                    //To Do: Handle autoIndex
//                } else if (Method == "POST") {
//                    _Headers += "HTTP/1.1 201 CREATED\r\n" + _Date;
//                    _Headers += "Connection: " + _req.getHeader("Connection") + "\r\n\r\n";
//                    write(_Fd, _Headers.c_str(), _Headers.length());
//                } else if (Method == "DELETE") {  //https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods/DELETE
//
//                    if (IsFile(FilePath)) {
//                        if (remove(FilePath.c_str()) == 0) {
//                            _Headers += "HTTP/1.1 204 No Content\r\n" + _Date;
//                            _Headers += "Connection: " + _req.getHeader("Connection") + "\r\n\r\n";
//                            write(_Fd, _Headers.c_str(), _Headers.length());
//                        } else
//                            BuildError("403 Forbidden");
//                    } else
//                        BuildError("404 Not Found");
//                }
//            }
//            // else (method not allowed)
//
//        }

        size_t getContentLength(int fd) {
            size_t length;

            length = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);

            return length;
        }

        template<class T>
        std::string ToString(T n) {
            std::ostringstream convert;
            convert << n;
            return (convert.str());
        }

        int  getErrorPageFd( char const* path ) {
            int fd;

            fd = open(path, O_RDONLY);
            if (fd < 0)
                std::cout << "open: " << strerror(errno) << std::endl;

            return fd;
        }

        void sendError(int error) {
            char const* errorPagePath = _ServerBlock->errorPages.find(error)->second.c_str();
            _BodyFd = getErrorPageFd(errorPagePath);

            Headers headers(error);
            headers.add<3>("Content-Type", MimeTypes::getType(errorPagePath), "charset=utf-8");
            if (_req.getHeader("Connection") == "keep-alive")
                headers.add("Connection", "Keep-Alive");
            headers.add("Content-Length", std::to_string(getContentLength(_BodyFd)));
            headers.addBodySeparator();

            if (write(_req.getSockFd(), headers.content().c_str(), headers.content().size()) < 0)
                throw std::runtime_error("error while writing to client");

            std::cout << "[Response Headers]" << std::endl;
            std::cout << headers.content() << std::endl;
        }

        void sendFile( const char* path ) {
            _BodyFd = open(path, O_RDONLY);
            if (_BodyFd < 0)
                throw std::runtime_error(strerror(errno));

            Headers headers(StatusCode::ok);
            headers.add<3>("Content-Type", MimeTypes::getType(path), "charset=utf-8");
            if (_req.getHeader("Connection") == "keep-alive")
                headers.add("Connection", "keep-alive");
            headers.add("Content-Length", std::to_string(getContentLength(_BodyFd)));
            headers.addBodySeparator();

            if (write(_req.getSockFd(), headers.content().c_str(), headers.content().size()) < 0)
                throw std::runtime_error("error while writing to client");

            std::cout << "[Response Headers]" << std::endl;
            std::cout << headers.content() << std::endl;
        }


        void sendBody() {
            char buffer[1024];
            int readRet = read(_BodyFd, buffer, 1024);
            if (readRet < 0)
                throw std::runtime_error("error while reading from response body file");
            if (readRet == 0) {
                close(_BodyFd);
                _Done = true;
                return;
            }
            if (write(_req.getSockFd(), buffer, readRet) < 0)
                throw std::runtime_error("error while writing to client");
        }

        void sendHeaders() {

            std::string path = _req.getHeader("Path");
            std::vector<std::string> paths = split(path, "/");

            if (paths.empty())
                sendFile("www/index.html");
            else {
                std::string filePath = "www/" + paths[0];
                sendFile(filePath.c_str());
                sendError(StatusCode::notFound);
            }


            _HeadersSent = true;
        }

    public:

        /*
         *  main entry.
         *  engine will invoke this everytime socket is ready for writing
         */
        void send() {
            if (!_HeadersSent) {
                sendHeaders();
            } else {
                sendBody();
            }
        }

        /*
         *  returns socket file descriptor of which response writing to
         */
        int getSockFd() {
            return _req.getSockFd();
        }

        /*
         *  return true if response completed otherwise false
         */
        bool done() const {
            return _Done;
        }

        /*
         *  request getter
         */
        Request& getRequest() {
            return _req;
        }

        /*
         *  reset all parameters to initial state (as if object have just constructed)
         */
        void reset() {
            _req.reset();
            _HeadersSent = false;
            _BodyFd = -1;
            _Done = false;
        }

    };

} // namespace ws
