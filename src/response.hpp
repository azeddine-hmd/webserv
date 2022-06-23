#pragma once

#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <poll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include<dirent.h>
#include "mimeTypes.hpp"
#include "request.hpp"
#include "config/config_model.hpp"
#include "cgi.hpp"
#include "headers.hpp"
#include <algorithm>


namespace ws {


    class Response {
        typedef std::pair<std::string, std::string>         pair;

        std::string     _Headers;
        Request         _req;
        ServerBlock*    _ServerBlock;
        LocationBlock   _Location;
        bool            _HeadersSent;
        int             _BodyFd;
        bool            _Done;
        bool            _isErr;


        Response();
    public:

        Response( Request& request, ServerBlock& serverBlock ): _req(request), _ServerBlock(&serverBlock) {
            _Headers = std::string();
            // _Location();
            _HeadersSent = false;
            _Done = false;
            _BodyFd = -1;
            _isErr = false;

        }

    private:

        /*
            https://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html
            https://www.ibm.com/docs/en/cics-ts/5.2?topic=protocol-http-responses
        */

       int IsFile(std::string &Path) {
           struct stat status;
           if (stat(Path.c_str(), &status) == 0) {
               if (status.st_mode & S_IFDIR) // Directory
                   return 0;
               else if (status.st_mode & S_IFREG) // Regular File
                   return 1;
               else
                   return 0;
           } else
               return 0;
       }

        int To_Int(std::string stringValue)
        {
            std::stringstream intValue(stringValue);
            int number = 0;
            intValue >> number;
            return number;
        }

        std::string     GetTime( void ) {
            time_t rawtime;

	        time(&rawtime);
            return ctime(&rawtime);
        }

        size_t getContentLength(int fd) {
            size_t length;

            length = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);

            return length;
        }

        template<class T>
        std::string To_String(T n) {
            std::ostringstream convert;
            convert << n;
            return (convert.str());
        }

        // int  getErrorPageFd(std::string path ) {
        //     int fd;

        //     if (fd = open(path.c_str(), O_RDONLY) < 0);
        //         std::cout << "open: " << strerror(errno) << std::endl;
        //     return fd;
        // }

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
            // write(_req.getSockFd(), "\r\n\r\n", 4);
        }

        int FindLocation( std::string Location ) {
            for (int i = 0 ; i< _ServerBlock->locations.size(); i++)
            {
                std::cout << "config: "<< _ServerBlock->locations[i].path << " | ";
                std::cout << "loc: " << Location << std::endl;
                if (_ServerBlock->locations[i].path == Location)
                    return i;
            }
            return -1;
        }

        bool   checkRootLocation() {
            int found = FindLocation("/");
            if (found != -1)
            {
                std::cout << "found root location" << std::endl;
                _Location = _ServerBlock->locations[found];
                return true;
            }
            return false;
        }

        bool    ExtractLocation( std::string Path )
        {
            std::vector<std::string> Locations = split(Path, "/");

            if (Locations.size() > 0)
            {
                std::string ToFind = "/" + Locations[0];
                for(int i = 0; i < Locations.size(); i++)
                {
                    int found = FindLocation(ToFind);
                    if (found != -1)
                    {
                        _Location = _ServerBlock->locations[found];
                        return (true);
                    }
                    if (i < Locations.size() - 1)
                        ToFind += "/" + Locations[i + 1];
                }
            }
            return (checkRootLocation());
        }

       void    SendError( std::string Error) {
            int         ErrorCode = To_Int(Error.substr(0, 3));
            std::string FilePath =  _ServerBlock->errorPages.find(ErrorCode)->second;

            _BodyFd = open(FilePath.c_str(), O_RDONLY);
            _Headers += "HTTP/1.1 " + Error + "\r\n";
            _Headers += "Date: " + GetTime();
            // if (ErrorCode == 405)
                // The Allow header lists the set of methods supported by a resource.
            _Headers += "Content-Type: text/html;charset=UTF-8\r\n";
            _Headers += "Content-Length: " + To_String(getContentLength(_BodyFd)) + "\r\n";
            _Headers += "Connection: close\r\n\r\n";


            if (write(_req.getSockFd(), _Headers.c_str(), _Headers.length()) < 0)
                throw std::runtime_error("error while writing to client");
            _HeadersSent = true;
       }

// auto index
        std::string getParent(std::string location)
        {
            std::size_t found = location.find_last_of("/\\");
            if(found == 0)
                return "/";
            else
                return location.substr(0,found);
        }

        std::string makeRow(std::string location, std::string text)
        {
            return std::string("<tr><td><a href=") + location + ">" + text + std::string("</a></tr></td>");
        }

        std::string generateAutoIndex(std::string location, std::string root)
        {
            dirent *d;
            DIR *dr;
            std::string buffer = std::string("<html><head><title>Index of ") + location + std::string("</title></head><body><h1>Index of ");
            buffer += location + std::string("</h1><table>");
            dr = opendir(root.c_str());
            if (errno == EACCES)
                return ("403");
            else if(dr!=NULL)
            {
                buffer += makeRow(location,".");
                for(d=readdir(dr); d!=NULL; d=readdir(dr))
                {
                    if(std::string(d->d_name) == "..")
                        buffer += makeRow(getParent(location),"..");
                    else if(std::string(d->d_name) != ".")
                        buffer += makeRow(d->d_name,d->d_name);
                }
                closedir(dr);
            }
            else
                std::cout<<"\nError Occurred!" << std::endl;
            buffer += "</table></body></html>";
            return buffer;
        }

        bool    isIndexFile(std::string Path) {
            if (Path[Path.length() - 1] != '/')
                Path += "/";
            Path += _Location.index;
            int fd = open(Path.c_str(), O_RDONLY);
            if (fd > 0)
            {
                SendFile(Path);
                return (true);
            }
            return (false);
        }

        void SendFile(std::string FilePath) {
            int fd = open(FilePath.c_str(), O_RDONLY);
            if (fd < 0)
                throw std::runtime_error("Error Opening a File");
            _BodyFd = fd;
            const char *Type = MimeTypes::getType(FilePath.c_str());
            _Headers += "HTTP/1.1 200 OK\r\nDate: " + GetTime();
            if (Type == NULL)
                _Headers += "Content-Type: text/plain\r\n";
            else
                _Headers += "Content-Type: " + To_String(Type) + "\r\n";
            _Headers += "Content-Length: " + To_String(getContentLength(fd)) + "\r\n";
            _Headers += "Connection: " + _req.getHeader("Connection") + "\r\n\r\n";
            if (write(_req.getSockFd(), _Headers.c_str(), _Headers.length()) < 0)
                throw std::runtime_error("error while writing to client");
            _HeadersSent = true;
        }

        void    sendAutoIndex(std::string FilePath) {
            std::cout << "auto index called" << std::endl;
            std::string buffer = generateAutoIndex(_req.getHeader("Path"), FilePath);
            if (buffer == "403")
                SendError("403 Forbidden");
            else {
                _Headers += "HTTP/1.1 200 OK\r\nDate: " + GetTime();
                _Headers += "Content-Type: text/html;charset=UTF-8\r\n";
                _Headers += "Content-Length: " + To_String(buffer.length()) + "\r\n\r\n";
                _Headers += buffer + "\r\n\r\n";
                write(_req.getSockFd(), _Headers.c_str(), _Headers.length());
                _HeadersSent = true;
            }
        }

        void    CheckPathErrors() {
            std::cout << "Path Error | Errno: " << strerror(errno) << std::endl;
            if (errno == EACCES)    //Search permission is denied for one of the directories in the path prefix of path
                SendError("403 Forbidden");
            else if (errno == ENOENT) //A component of path does not exist, or path is an empty string.
                SendError("404 Not Found");
        }

        void    SendWithGet(std::string FilePath) {

            struct stat FileStatus;     //https://linux.die.net/man/2/stat
            if (stat(FilePath.c_str(), &FileStatus) < 0) //there is a Path Error
                CheckPathErrors();
            else if (S_ISDIR(FileStatus.st_mode)) // is Directory == autoindex
            {
                if (isIndexFile(FilePath) == false)
                {
                    if (_Location.autoindex == true) { //sendAutoindex
                        sendAutoIndex(FilePath);
                    }
                    else
                        SendError("404 Not Found");
                }
            }
            else //is File
                SendFile(FilePath);
        }

        void    SendWithPost() {
            _Headers += "HTTP/1.1 201 CREATED\r\nDate: " + GetTime();
            _Headers += "Connection: " + _req.getHeader("Connection") + "\r\n\r\n";
            // _Headers += "Connection: " + _req.getHeader("Connection") + "\r\n";
            // _Headers += "Content-length: 0\r\n\r\n";
            if (write(_req.getSockFd(), _Headers.c_str(), _Headers.length()) < 0)
                throw std::runtime_error("error while writing to client");
            _HeadersSent = true;
        }

        void    SendWithDelete(std::string FilePath) {
            if (IsFile(FilePath)) {
                if (remove(FilePath.c_str()) == 0) {
                    _Headers += "HTTP/1.1 204 No Content\r\nDate: " + GetTime();
                    _Headers += "Connection: " + _req.getHeader("Connection") + "\r\n\r\n";                
                    if (write(_req.getSockFd(), _Headers.c_str(), _Headers.length()) < 0)
                        throw std::runtime_error("error while writing to client");
                    _HeadersSent = true;                  
                } else
                    SendError("403 Forbidden");
            } else
                SendError("404 Not Found");
        }

        std::string     GetFilePath(std::string Path) {
            std::string Root = _Location.root;
            std::string NewPath = Path.substr(Path.find(_Location.path) + _Location.path.length()
                                        , Path.length());
            if (Root[Root.length() - 1] != '/')
                Root += "/";
            if (NewPath[0] == '/')
                NewPath.erase(0,1);
            return Root + NewPath;
        }

        void    sendHeaders() {
            std::string Method = _req.getHeader("Method");
            std::string Path = _req.getHeader("Path");

            if (ExtractLocation(Path)) {
                if (std::find(_Location.allowedMethods.begin(),_Location.allowedMethods.end(),
                        Method) != _Location.allowedMethods.end())
                {
                    std::string FilePath = GetFilePath(Path);
                    std::cout << "-------- built path " << FilePath << std::endl;
                    if (Method == "GET")
                        SendWithGet(FilePath);
                    else if (Method == "POST")
                    {
                        std::cout << "post request called" << std::endl;
                        SendWithPost();
                    }
                    else if (Method == "DELETE")
                        SendWithDelete(FilePath);
                }
                else
                    SendError("405 Method Not Allowed");
            }
            else
                SendError("404 Not Found");
        }

    public:

        /*
         *  main entry.
         *  engine will invoke this everytime socket is ready for writing
         */
        void send() {
            if (!_HeadersSent) {
                sendHeaders();
            }
            else if (_BodyFd > -1) {
                sendBody();
            }
            else
                _Done = true;
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
