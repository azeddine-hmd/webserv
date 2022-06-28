#pragma once

#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <poll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "request.hpp"

#define _Error_ "HTTP/1.1 500 Internal Server Error\r\n"
extern char **environ;

namespace ws {

    class cgi {
        typedef std::string String;

        String      _ScriptPath;
        String      _Bin;
        String      _Query;
        String      _Method;
        Request    &_Data;

        pid_t       _cgiPid;

        // cgi();
    public:

        cgi(Request &ReqData, String FilePath, String cgiBin) : _Data(ReqData) {
            std::vector<std::string> Path = split(FilePath, "?");

            _cgiPid = -1;
            this->_ScriptPath = Path[0];
            if (Path.size() > 1)
                this->_Query += Path[1];
            this->_Method = ReqData.getHeader("Method");
            this->_Bin = cgiBin;
            InitMetaVariables(ReqData);
        }

        ~cgi(void) {
            remove(_Data.getBody().c_str());
        }

        // el caroto popito
        void InitMetaVariables(Request& Data) {

            // std::cout << "Content-Length=> " <<  Data.getHeader("Content-Length") << std::endl;
            // std::cout << "QUERY_STRING=> " << this->_Query << std::endl;
            // std::cout << "REQUEST_METHOD=> " << this->_Method << std::endl;
            // std::cout << "SCRIPT_FILENAME=> " << this->_ScriptPath << std::endl;

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
            setenv("HTTP_COOKIE", Data.getHeader("Cookie").c_str() , 1);
            setenv("REDIRECT_STATUS", "true", 1);
            setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
        }

        void execWithGET(char *args[3], int fd[2]) {
            std::cout << "cgi executing get ..." << std::endl;
            dup2(fd[1], 1);
            close(0);
            close(fd[1]);
            close(fd[0]);
            execve(args[0], args, environ);
            std::cerr << "execve failed | errno: " << strerror(errno) << std::endl;
            exit(errno);
        }

        int execWithPOST(char *args[3], int fd[2]) {
            std::cout << "cgi executing post ..." << std::endl;
            int fd3 = open(_Data.getBodyFile().name.c_str(), O_RDONLY);
            dup2(fd3, 0);
            dup2(fd[1], 1);
            close(fd[1]);
            execve(args[0], args, environ);
            std::cerr << "execve failed | errno: " << strerror(errno) << std::endl;
            exit(errno);
        }

        int execute( void ) {
            int pid;
            int fd[2];
            char *args[3] = {
                (char *) this->_Bin.c_str(),
                (char *) this->_ScriptPath.c_str(),
                NULL
            };

            pipe(fd);
            pid = fork();
            if (pid < 0)
                return (-1);
            if (pid == 0)
            {
                if (this->_Method == "GET") 
                    execWithGET(args, fd);
                else if (this->_Method == "POST")
                    execWithPOST(args, fd);
            } else {
                _cgiPid = pid;
            }
            close(fd[1]);
            return (fd[0]);
        }

        pid_t   getCgiPid() {
            return _cgiPid;
        }
    };
}
