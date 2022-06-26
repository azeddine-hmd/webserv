#pragma once

#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <poll.h>
#include <sys/stat.h>
#include <sys/wait.h>
// #include "mimeTypes.hpp"
#include "request.hpp"
// #include "config/config_model.hpp"

#define _Error_ "HTTP/1.1 500 Internal Server Error\r\n"
extern char **environ;

namespace ws {

    class cgi {
        typedef std::string String;

        String      _ScriptPath;
        String      _Bin;
        String      _Buffer;
        String      _Query;
        String      _Method;
        Request    &_Data;

        // cgi();
    public:

        cgi(Request &ReqData, String FilePath) : _Data(ReqData) {
            this->_ScriptPath = FilePath;
            // this->_Data = ReqData;
            // std::cout << _ScriptPath << std::endl;
            this->_Method = ReqData.getHeader("Method");
            this->_Buffer = String();
            this->_Query = String();

            String Path = FilePath;
            // Query Parsing

            if (_Method == "POST") {
                // size_t size = std::stoi(ReqData.getHeader("Content-Length"));
                // char *buffer = (char *)malloc(size);
                // int fd = open(ReqData.getFile().c_str(), O_RDONLY);
                // int readRet = -1;
                // this->_Query = std::string();
                // readRet = read(fd,buffer, size);

                // std::string strbuffer = std::string(buffer,readRet);
                // this->_Query = std::string(buffer,readRet);
                // len += readRet;

                // std::cout << "content lenght : " << ReqData->getHeader("Content-Length") << std::endl;
                // int readRet = read(fd,buffer, 1024);
                // close(fd);
                // this->_Query = std::string(buffer,readRet);
            } else if (_Method == "GET" && Path.find('?') != std::string::npos) {
                // To Do: - Query need to be splited with '?'
                this->_Query = Path.substr(Path.find("?") + 1, Path.length());
            }
            InitMetaVariables(ReqData);
        }

        ~cgi(void) {
            remove(_Data.getBody().c_str());
        }

        // el caroto popito
        void InitMetaVariables(Request& Data) {

            std::cout << "Content-Length=> " <<  Data.getHeader("Content-Length") << std::endl;
            std::cout << "QUERY_STRING=> " << this->_Query << std::endl;
            std::cout << "REQUEST_METHOD=> " << this->_Method << std::endl;
            std::cout << "SCRIPT_FILENAME=> " << this->_ScriptPath << std::endl;

            if (Data.getHeader("Content-Length").length()) {
                setenv("CONTENT_LENGTH", Data.getHeader("Content-Length").c_str(), 1);
            }
            // if (Data.getHeader("Content-Type").length())
            //     setenv("CONTENT_TYPE", Data.getHeader("Content-Type").c_str(), 1);
            // else
                setenv("CONTENT_TYPE", "text/html; charset=UTF-8", 1);
            if (_Method == "GET")
                setenv("QUERY_STRING", this->_Query.c_str(), 1);
            else
                setenv("QUERY_STRING", "", 1);
            setenv("REQUEST_METHOD", this->_Method.c_str(), 1);
            setenv("SCRIPT_FILENAME", this->_ScriptPath.c_str(), 1);
            setenv("REDIRECT_STATUS", "true", 1);
            if (this->_ScriptPath.find(".php") != std::string::npos)
                this->_Bin = "/bin/php-cgi";
            else
                this->_Bin = "/usr/bin/python3"; //change it to Python later
            setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
        }

        void execWithGET(char *args[3], int fd[2]) {
            dup2(fd[1], 1);
            close(fd[0]);
            close(fd[1]);
            execve(args[0], args, environ);
            std::cerr << "execve failed | errno: " << strerror(errno) << std::endl;
            exit(errno);
        }

        /*
            HTTP/1.1 200 OK
            Date: Tue, 14 Jun 2022 22:29:18 UTC
            /Users/mel-haya/.brew/bin/php-cgi <---
            X-Powered-By: PHP/8.1.7
            Content-type: text/html; charset=UTF-8

            <html>
            <body>

            <form method="post" action="">
            Name: <input type="text" name="fname">
            <input type="submit">
            </form>

            <br />
            <b>Warning</b>:  Undefined array key "fname" in <b>/Users/mel-haya/Desktop/webserv/www/post.php</b> on line <b>12</b><br />
            Name is empty
            </body>
            </html>
        */
        int execWithPOST(char *args[3], int fd[2], int fd2[2]) {
            // struct pollfd   fds;
            // int             rc;

            // fds.fd = fd2[1];
            // fds.events = POLLOUT;
            // if ((rc = poll(&fds, 1, 0)) < 0)
            //     return (0);
            // if (fds.revents & POLLOUT)
            // {
            // size_t size = std::stoi(_Data.getHeader("Content-Length"));

            // char *buffer = (char *)malloc(size);
            // int y = read(fd,buffer, size);
            // std::cout << "read : " << y << std::endl;
            // write(fd2[1], buffer, size);
            // free(buffer);

            // }
            int fd3 = open(_Data.getBodyFile().name.c_str(), O_RDONLY);
            dup2(fd3, 0);
            dup2(fd[1], 1);
            close(fd2[0]);
            close(fd2[1]);
            close(fd[1]);
            execve(args[0], args, environ);
            std::cerr << "execve failed | errno: " << strerror(errno) << std::endl;

            exit(errno);
        }

        int execute( void ) {
            int pid;
            int fd[2];
            int fd2[2];
            struct pollfd fds;
            char *args[3] = {
                    (char *) this->_Bin.c_str(),
                    (char *) this->_ScriptPath.c_str(),
                    NULL
            };

            if (this->_Method == "GET") {
                pipe(fd);
                pid = fork();
                if (pid < 0)
                    return (-1);
                else if (pid == 0)
                    execWithGET(args, fd);
                close(fd[1]);
            } else if (this->_Method == "POST") {
                pipe(fd);
                pipe(fd2);
                pid = fork();
                if (pid < 0)
                    return (-1);
                else if (pid == 0) {
                    if (!execWithPOST(args, fd, fd2))
                        return (-1);
                }
                close(fd[1]);
                close(fd2[0]);
                close(fd2[1]);
            }
            // wait for child process to exit
            waitpid(pid, NULL, 0);
            // if (_Query.size())
            // {
            //     char buffer1[1024];
            //     int ret = -1;
            //     while (ret != 0)
            //     {
            //         if (ret == -1)
            //             std::cout << "first read " << buffer1 << std::endl;
            //         ret = read(fd[0], buffer1, 1);
            //         std::cout << buffer1;
            //     }
            //     std::cout << std::endl;
            // }
            std::cout << "fd inside cgi: " << fd[0] << std::endl;
            return (fd[0]);
        }
    };

} // namespace ws
