#ifndef REQBUILD_HPP
#define REQBUILD_HPP

#include <stdio.h>
#include <string>
#include <unistd.h>
#include "request.hpp"
#include <fcntl.h>
#include "mimeTypes.hpp"
#include <cstring>
#include <stdlib.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#define _Error_ "HTTP/1.1 500 Internal Server Error\r\n"
extern char **environ;

class cgi {
	private:
		typedef  std::pair<std::string, std::string>	pair;
		typedef  std::string							String;

		String	_ScriptPath;
        String  _Bin;
		String	_Buffer;
        String  _Query;
        String  _Method;
        Request _Data;

	public:
		// cgi( void ): {}
		cgi( Request ReqData, String root) : _Data(ReqData) {
            this->_ScriptPath = root + "upload.php";
            this->_Data = ReqData;
            // std::cout << _ScriptPath << std::endl;
            this->_Method = ReqData.getHeader("Method");
            this->_Buffer = String();
            this->_Query = String();


            String Path = ReqData.getHeader("Path");
            // Query Parsing
            
            if (_Method == "POST")
            {
                // size_t size = std::stoi(ReqData.getHeader("Content-Length"));
                // char *buffer = (char *)malloc(size);
                // int fd = open(ReqData.getFile().c_str(), O_RDONLY);
                // int readRet = -1;
                // this->_Query = std::string();
                // readRet = read(fd,buffer, size);
                
                    // std::string strbuffer = std::string(buffer,readRet);
                // this->_Query = std::string(buffer,readRet);
                    // len += readRet;
                
                std::cout << "content lenght : " << ReqData.getHeader("Content-Length") << std::endl;
                // int readRet = read(fd,buffer, 1024);
                // close(fd);
                // this->_Query = std::string(buffer,readRet);            
            }
            else if (_Method == "GET" && Path.find('?') != std::string::npos) {
                    this->_Query = Path.substr(Path.find("?")+1, Path.length());
            }
            InitMetaVariables(ReqData);
        }
		~cgi( void ) {
            remove(_Data.getFile().c_str());
        }
        // el caroto popito
		void    InitMetaVariables( Request Data ) {
            if (Data.getHeader("Content-Length").length())
            {
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
        
        void    execWithGET( char *args[3], int fd[2]) {
        	dup2(fd[1], 1);
            close (fd[1]);
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
        int execWithPOST( char *args[3], int fd[2], int fd2[2]) {
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
            int fd3 = open(_Data.getFile().c_str(), O_RDONLY);
            dup2(fd3, 0);
        	dup2(fd[1], 1);
            close (fd2[0]);
            close(fd2[1]);
            close (fd[1]);
            execve(args[0], args, environ);
            std::cerr << "execve failed | errno: " << strerror(errno) << std::endl;

            exit(errno);
        }

		pair	execute( void ) {
            int             pid;
            int             fd[2];
            int             fd2[2];
            struct pollfd   fds;
            char    *args[3] = {
                (char *)this->_Bin.c_str(),
                (char *)this->_ScriptPath.c_str(),
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
            }
            else if (this->_Method == "POST") {
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
            String  buffer;
            while (1) {
                if ((rc = poll(&fds, 1, 0)) < 0)  // poll ret 0 == timeout || < 0  == error 
                {
                        std::cout << "nari poll cgi error 500" << std::endl;
                        return (std::make_pair("500", _Error_));
                }
                if (rc == 1 && fds.events & POLLIN ) {
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
// }
};

class Response
{
    private:

        typedef std::pair<std::string, std::string>     pair;


        std::string _Header;
        std::string _Body;
        int         _Fd; // the fd of the file that will be sent
        Request    _req;
        cgi         _Cgi;
        // bool        _HeaderSent;
        bool        _isCgi;
        bool        _Error;
        std::string _Date;

    public:

        Response (Request& request) : _req(request), _Cgi(request, "/Users/mel-haya/Desktop/webserv/www/")
        {
            _Header = std::string();
            _Body = std::string();
            _Error = false;
            _isCgi = false;
            _Date = "Date: " + setDate() + "\r\n";
            _Fd = request.getFd();
        }

        /*
            https://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html
            https://www.ibm.com/docs/en/cics-ts/5.2?topic=protocol-http-responses
        */

        std::string     setDate(void)
        {
        	char    buffer[30];
        	struct timeval	tv;

        	gettimeofday(&tv, NULL);
        	strftime(buffer, 29, "%a, %d %b %Y %T %Z", gmtime(&tv.tv_sec));
        	return (std::string(buffer));
        }

        int     IsFile(std::string& Path)
        {
        	struct stat status;
        	if (stat(Path.c_str(), &status) == 0 )
        	{
        		if (status.st_mode & S_IFDIR)
        			return 0;
        		else if (status.st_mode & S_IFREG)
        			return 1;
        		else
        			return 0;
        	}
        	else
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

        void    Build( void ) {
            if (_req.getHeader("Path").find(".php") != std::string::npos 
                || _req.getHeader("Path").find(".py") != std::string::npos)
            {
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
            }
            else if (_req.getHeader("Method").length()) {
                std::string Method = _req.getHeader("Method");
                std::string FilePath = "www" + _req.getHeader("Path");

                if (Method == "GET") {
                    std::cout << "inside GET" << std::endl;
		            struct stat FileStatus;     //https://linux.die.net/man/2/stat
                    int         fd;

		            if (stat(FilePath.c_str(), &FileStatus) < 0) {
                        std::cout << "File Protection | Errno: " << errno << std::endl;
                        if (errno == EACCES)    //Search permission is denied for one of the directories in the path prefix of path
                            BuildError("403 Forbidden");
                        else if (errno == ENOENT)    //A component of path does not exist, or path is an empty string. 
                            BuildError("404 Not Found");
                    }
                    else {
                        const char* Type = MimeTypes::getType(FilePath.c_str());
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
                        while(read(fd, buffer, 1024) > 0)
                            write(_Fd, buffer, 1024);
                        write(_Fd, "\r\n\r\n", 4);
                        close(fd);
                    }
                    //To Do: Handle autoIndex
                }
                else if (Method == "POST") {
                    _Header += "HTTP/1.1 201 CREATED\r\n" + _Date;
                    _Header += "Connection: " + _req.getHeader("Connection") + "\r\n\r\n";
                    write(_Fd, _Header.c_str(), _Header.length());
                }
                else if (Method == "DELETE") {  //https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods/DELETE

                    if (IsFile(FilePath))
	                {
	                	if (remove(FilePath.c_str()) == 0) {
                            _Header += "HTTP/1.1 204 No Content\r\n" + _Date;
                            _Header += "Connection: " + _req.getHeader("Connection") + "\r\n\r\n";
                            write(_Fd, _Header.c_str(), _Header.length());
                        }
	                	else
	                		BuildError("403 Forbidden");
	                }
	                else
                        BuildError("404 Not Found");
                }
            }
            // else (method not allowed)
        
        }

        off_t getContentLength( int fd ) {
            return (lseek(fd, 0, SEEK_END));
        }

        template<class T>
        std::string ToString(T n){
            std::ostringstream convert;
            convert << n;
            return(convert.str());
        }
};


#endif