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


namespace ws {


    class Response {
        typedef std::pair<std::string, std::string> pair;

        std::string     _Header;
        std::string     _Body;
        int             _Fd; // the fd of the file that will be sent
        Request        _req;
        cgi             _Cgi;
        // bool         _HeaderSent;
        bool            _isCgi;
        bool            _Error;
        std::string     _Date;
        ServerBlock*    _ServerBlock;
        bool            _Done;

        Response();
    public:

        Response( Request& request, ServerBlock& serverBlock ):  _req(request), _Cgi(&request, "/Users/mel-haya/Desktop/webserv/www/"), _ServerBlock(&serverBlock) {
            _Header = std::string();
            _Body = std::string();
            _Error = false;
            _isCgi = false;
            _Date = "Date: " + setDate() + "\r\n";
            _Fd = request.getFd();
        }

    private:

        /*
            https://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html
            https://www.ibm.com/docs/en/cics-ts/5.2?topic=protocol-http-responses
        */

        std::string setDate(void) {
            char buffer[30];
            struct timeval tv;

            gettimeofday(&tv, NULL);
            strftime(buffer, 29, "%a, %d %b %Y %T %Z", gmtime(&tv.tv_sec));
            return (std::string(buffer));
        }

        int IsFile(std::string &Path) {
            struct stat status;
            if (stat(Path.c_str(), &status) == 0) {
                if (status.st_mode & S_IFDIR)
                    return 0;
                else if (status.st_mode & S_IFREG)
                    return 1;
                else
                    return 0;
            } else
                return 0;
        }

        void    BuildError( std::string  Error) {
            std::string	    StatusCode = Error.substr(0, 3);
            std::string     FilePath =  "resources/error_pages/" + StatusCode + ".html";
            int             fd = open(FilePath.c_str(), O_RDONLY);
            struct stat     FileStatus;

            std::cout << "build Error: " << FilePath << std::endl;
            fstat(fd, &FileStatus);
            _Header += "HTTP/1.1 " + Error + "\r\n" + _Date;
            _Header += "Content-Type: text/html\r\n";
            _Header += "Content-Length: " + ToString(FileStatus.st_size) + "\r\n";
            _Header += "Connection: closed\r\n\r\n";

            write(_Fd, _Header.c_str(), _Header.length());
            char buffer[1024] = "";
            while(read(fd, buffer, 1024) > 0)
                write(_Fd, buffer, 1024);
            write(_Fd, "\r\n\r\n", 4);
            close(fd);
        }

        void Build(void) {
            if (_req.getHeader("Path").find(".php") != std::string::npos
                || _req.getHeader("Path").find(".py") != std::string::npos) {
                std::cout << "in cgi" << std::endl;
                pair status = _Cgi.execute(); // return pair(string status code, string status msg)
                //cgi can eather return content or status codes (400 || 500 || 301 || 302)
                // should build Error
                // else if (redirection for later)
                _Header += status.second + _Date;
                // _Header += "Content-Length:" + this->ToString(status.first.length()) + "\r\n";
                write(_Fd, _Header.c_str(), _Header.length());
                write(_Fd, status.first.c_str(), status.first.length());
                return;
            } else if (_req.getHeader("Method").length()) {
                std::string Method = _req.getHeader("Method");
                std::string FilePath = "www" + _req.getHeader("Path");

                if (Method == "GET") {
                    std::cout << "inside GET" << std::endl;
                    struct stat FileStatus;     //https://linux.die.net/man/2/stat
                    int fd;

                    if (stat(FilePath.c_str(), &FileStatus) < 0) {
                        std::cout << "File Protection | Errno: " << errno << std::endl;
                        if (errno ==
                            EACCES)    //Search permission is denied for one of the directories in the path prefix of path
                            BuildError("403 Forbidden");
                        else if (errno == ENOENT)    //A component of path does not exist, or path is an empty string. 
                            BuildError("404 Not Found");
                    } else {
                        const char *Type = MimeTypes::getType(FilePath.c_str());
                        _Header += "HTTP/1.1 200 OK\r\n" + _Date;
                        if (Type == NULL)
                            _Header += "Content-Type: text/plain\r\n";
                        else
                            _Header += "Content-Type: " + ToString(Type) + "\r\n";
                        _Header += "Content-Length: " + ToString(FileStatus.st_size) + "\r\n";
                        _Header += "Connection: " + _req.getHeader("Connection") + "\r\n\r\n";

                        fd = open(FilePath.c_str(), O_RDONLY);
                        write(_Fd, _Header.c_str(), _Header.length());
                        char buffer[1024] = "";
                        while (read(fd, buffer, 1024) > 0)
                            write(_Fd, buffer, 1024);
                        write(_Fd, "\r\n\r\n", 4);
                        close(fd);
                    }
                    //To Do: Handle autoIndex
                } else if (Method == "POST") {
                    _Header += "HTTP/1.1 201 CREATED\r\n" + _Date;
                    _Header += "Connection: " + _req.getHeader("Connection") + "\r\n\r\n";
                    write(_Fd, _Header.c_str(), _Header.length());
                } else if (Method == "DELETE") {  //https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods/DELETE

                    if (IsFile(FilePath)) {
                        if (remove(FilePath.c_str()) == 0) {
                            _Header += "HTTP/1.1 204 No Content\r\n" + _Date;
                            _Header += "Connection: " + _req.getHeader("Connection") + "\r\n\r\n";
                            write(_Fd, _Header.c_str(), _Header.length());
                        } else
                            BuildError("403 Forbidden");
                    } else
                        BuildError("404 Not Found");
                }
            }
            // else (method not allowed)

        }

        off_t getContentLength(int fd) {
            return (lseek(fd, 0, SEEK_END));
        }

        template<class T>
        std::string ToString(T n) {
            std::ostringstream convert;
            convert << n;
            return (convert.str());
        }

    public:

        /*
         *  main entry.
         *  engine will invoke this everytime socket is ready for writing
         */
        void send() {
            //TODO: implement
        }

        /*
         *  returns socket file descriptor of which response writing to
         */
        int getFd() {
            return _req.getFd();
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
         *  //
         */
        void reset() {
            _req.reset();
            //TODO: add all necessary code for resetting response attribute without modifying on socket fd
            //
        }

    };

} // namespace ws
