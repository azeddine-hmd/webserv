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

#define _Error_ "HTTP/1.1 500 Internal Server Error\r\n"
extern char **environ;

namespace ws {

    class cgi {
        typedef std::pair<std::string, std::string> pair;
        typedef std::string String;

        String      _ScriptPath;
        String      _Bin;
        String      _Buffer;
        String      _Query;
        String      _Method;
        Request*    _Data;

        cgi();
    public:

        cgi(Request* ReqData, String root) : _Data(ReqData) {
            this->_ScriptPath = root + "upload.php";
            this->_Data = ReqData;
            // std::cout << _ScriptPath << std::endl;
            this->_Method = ReqData->getHeader("Method");
            this->_Buffer = String();
            this->_Query = String();

            String Path = ReqData->getHeader("Path");
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

                std::cout << "content lenght : " << ReqData->getHeader("Content-Length") << std::endl;
                // int readRet = read(fd,buffer, 1024);
                // close(fd);
                // this->_Query = std::string(buffer,readRet);
            } else if (_Method == "GET" && Path.find('?') != std::string::npos) {
                this->_Query = Path.substr(Path.find("?") + 1, Path.length());
            }
            InitMetaVariables(*ReqData);
        }

        ~cgi(void) {
            remove(_Data->getBody().c_str());
        }

        // el caroto popito
        void InitMetaVariables(Request& Data) {
            if (Data.getHeader("Content-Length").length()) {
                setenv("CONTENT_LENGTH", Data.getHeader("Content-Length").c_str(), 1);
            }
            if (Data.getHeader("Content-Type").length())
                setenv("CONTENT_TYPE", Data.getHeader("Content-Type").c_str(), 1);
            else
                setenv("CONTENT_TYPE", "text/html; charset=UTF-8", 1);
            if (_Method == "GET")
                setenv("QUERY_STRING", this->_Query.c_str(), 1);
            else
                setenv("QUERY_STRING", "", 1);
            setenv("REQUEST_METHOD", this->_Method.c_str(), 1);
            setenv("SCRIPT_FILENAME", this->_ScriptPath.c_str(), 1);
            setenv("REDIRECT_STATUS", "true", 1);
            if (this->_ScriptPath.find(".php") != std::string::npos)
                this->_Bin = "/goinfre/mel-haya/.brew/Cellar/php/8.1.7/bin/php-cgi";
            else
                this->_Bin = "/usr/bin/python";
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
            int fd3 = open(_Data->getBodyFile().name.c_str(), O_RDONLY);
            dup2(fd3, 0);
            dup2(fd[1], 1);
            close(fd2[0]);
            close(fd2[1]);
            close(fd[1]);
            execve(args[0], args, environ);
            std::cerr << "execve failed | errno: " << strerror(errno) << std::endl;

            exit(errno);
        }

        pair execute(void) {
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
                    return (std::make_pair("500", _Error_));
                else if (pid == 0)
                    execWithGET(args, fd);
                close(fd[1]);
            } else if (this->_Method == "POST") {
                pipe(fd);
                pipe(fd2);
                pid = fork();
                if (pid < 0)
                    return (std::make_pair("500", _Error_));
                else if (pid == 0) {
                    if (!execWithPOST(args, fd, fd2))
                        return (std::make_pair("500", _Error_));
                }
                close(fd[1]);
                close(fd2[0]);
                close(fd2[1]);
            }
            // wait for child process to exit
            waitpid(pid, NULL, 0);

            // int status;
            // pid_t popito;
            // time_t t = time(NULL);
            // while ((time(NULL) - t) < 3) {
            // 	if ((popito = waitpid(pid, &status, WNOHANG)) > 0)
            // 		break ;
            //     std::cout << "wait pid return: " << popito << std::endl;
            // }
            // while (1)
            // {
            // 	popito = waitpid(pid, &status, WNOHANG);
            //     if (popito > 0)
            //         break;
            // std::cout << "wait pid return: " << popito << std::endl;
            // }
            // if (WEXITSTATUS(status) || kill(pid, SIGKILL) == 0)
            // {
            //     close(fd[0]);
            //     std::cout << "nari wait exitstatus rabak error 500" << std::endl;
            //     return (std::make_pair("500", _Error_));
            // }

            // parse executed program output and store it
            fds.fd = fd[0];
            fds.events = POLLIN;
            char val;
            int rc;
            String buffer;
            while (1) {
                if ((rc = poll(&fds, 1, 0)) < 0)  // poll ret 0 == timeout || < 0  == error
                {
                    std::cout << "nari poll cgi error 500" << std::endl;
                    return (std::make_pair("500", _Error_));
                }
                if (rc == 1 && fds.events & POLLIN) {
                    if (int count = read(fds.fd, &val, 1) < 1) {
                        if (count < 0) {
                            std::cout << "nari cgi error 500" << std::endl;
                            close(fd[0]);
                            return (std::make_pair("500", _Error_));
                        }
                        break;
                    }
                    buffer += val;
                }
            }
            close(fd[0]);
            // need to check if the script executed want to redirect (302 | 301) | err 500
            return (std::make_pair(buffer, "HTTP/1.1 200 OK\r\n"));
        }

    };

} // namespace ws
