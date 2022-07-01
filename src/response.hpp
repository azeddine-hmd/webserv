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
#include "StatusCode.hpp"
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

        int             _cgiPip;
        std::string     _cgiFile;
        int             _cgiTmpFile;
        pid_t           _cgiPid;
        bool            _CgiFound;
        bool            _PipHeadersRead;
        struct timeval  _CgiExecDuration;

        Response();
    public:

        Response( Request& request, ServerBlock& serverBlock ): _req(request), _ServerBlock(&serverBlock) {
            _Headers = std::string();
            // _Location();
            _HeadersSent = false;
            _Done = false;
            _BodyFd = -1;
            _cgiPip = -1;
            _cgiFile = std::string();
            _cgiTmpFile = -1;
            _isErr = false;
            _CgiFound = false;
            _PipHeadersRead = false;
            bzero(&_CgiExecDuration, sizeof(_CgiExecDuration));
        }

    private:

        /*
            https://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html
            https://www.ibm.com/docs/en/cics-ts/5.2?topic=protocol-http-responses
        */

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
            if (write(_req.getSockFd(), buffer, readRet) <= 0)
                throw std::runtime_error("error while writing to client");
        }

        int FindLocation( std::string Location ) {
            for (size_t i = 0 ; i < _ServerBlock->locations.size(); i++)
            {
                std::cout << "config: "<< _ServerBlock->locations[i].path << " | ";
                std::cout << "loc: " << Location << std::endl;
                if (_ServerBlock->locations[i].path == Location)
                    return i;
            }
            return -1;
        }

        bool   GetRootLocation() {
            int found = FindLocation("/");
            if (found != -1)
            {
                // std::cout << "found root location" << std::endl;
                _Location = _ServerBlock->locations[found];
                return true;
            }
            return false;
        }

        bool    GetCGILocation(std::string cgiType) {
            std::cout << "CGI Type = " << cgiType << std::endl;
            int found = FindLocation(cgiType);

            if (found != -1)
            {
                _CgiFound = true;
                std::cout << "found CGI location" << std::endl;
                _Location = _ServerBlock->locations[found];
                return true;
            }
            return false;
        }

        bool findCgi(std::string Path) {
            std::vector<std::string> paths = split(Path, "/");
            if (paths.empty())
                return false;

            std::string lastPath = paths.back();
            size_t start = lastPath.rfind(".");
            if (start == std::string::npos)
                return false;

            if (GetCGILocation(lastPath.substr(start, lastPath.size())))
                return true;

            return false;
        }

        bool    ExtractLocation( std::string Path )
        {
            if (findCgi(Path))
                return true;
            std::vector<std::string> Locations = split(Path, "/");
            if (Locations.size() > 0) {
                std::string ToFind = "/" + Locations[0];
                for(size_t i = 0; i < Locations.size(); i++)
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
            return (GetRootLocation());
        }

       void    SendError(int ErrCode) {
            std::string FilePath =  _ServerBlock->errorPages.find(ErrCode)->second;

            std::cout << "Send error called " << ErrCode << std::endl;
            _BodyFd = open(FilePath.c_str(), O_RDONLY);
            _Headers += "HTTP/1.1 " + To_String(ErrCode) + " " + StatusCode::reasonPhrase(ErrCode) + "\r\n";
            _Headers += "Date: " + GetTime();
            if (ErrCode == 405) {
                _Headers += "Allow: ";
                for (size_t i = 0; i < _Location.allowedMethods.size(); i++) {
                    _Headers += _Location.allowedMethods[i];
                    if (i < _Location.allowedMethods.size() -1)
                        _Headers += ", ";

                }
                _Headers += "\r\n";
            }
            _Headers += "Content-Type: text/html; charset=UTF-8\r\n";
            _Headers += "Content-Length: " + To_String(getContentLength(_BodyFd)) + "\r\n";
            if (_req.getHeader("Connection") == "keep-alive")
                _Headers += "Connection: " + _req.getHeader("Connection") + "\r\n";
           interceptResponseHeaders(_Headers);
            _Headers += "\r\n";


            if (write(_req.getSockFd(), _Headers.c_str(), _Headers.length()) < 0)
                throw std::runtime_error("error while writing to client");
            _HeadersSent = true;
            if(_req.getHeader("Method") == "HEAD")
            {
                close(_BodyFd);
                _BodyFd = -1;
                _Done = true;
                return;
            }
       }

        std::string getParent(std::string location)
        {
            std::size_t found = location.find_last_of("/\\");
            if(found == 0)
                return "/";
            else
                return location.substr(0, found);
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
                    if(std::string(d->d_name) == "..") {
						std::string parent = getParent(location);
						//std::cout << "parent: " << parent << std::endl;
                        buffer += makeRow(parent,"..");
					} else if(std::string(d->d_name) != ".") {
						std::string abslPath;
						if (location.size() > 1) {
							abslPath = location + "/" + d->d_name;
						} else {
							abslPath = location + d->d_name;
						}
						std::cout << "abslPath: " << abslPath << std::endl;
						std::cout << "file name: " << d->d_name << std::endl;
                        buffer += makeRow(abslPath,d->d_name);
					}
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
            if (isFileReadable(Path)) {
                SendFile(Path);
                return (true);
            }
            return false;
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

            if (_req.getHeader("Connection") == "keep-alive")
                _Headers += "Connection: " + _req.getHeader("Connection") + "\r\n";
            interceptResponseHeaders(_Headers);
            _Headers += "\r\n";
            if (write(_req.getSockFd(), _Headers.c_str(), _Headers.length()) < 0)
                throw std::runtime_error("error while writing to client");
            _HeadersSent = true;
            if(_req.getHeader("Method") == "HEAD")
            {
                close(_BodyFd);
                _BodyFd = -1;
                _Done = true;
                return;
            }
        }

        void    sendAutoIndex(std::string FilePath) {
            std::cout << "auto index called" << std::endl;
            std::string buffer = generateAutoIndex(_req.getHeader("Path"), FilePath);
            if (buffer == "403")
                SendError(403);
            else {
                _Headers += "HTTP/1.1 200 OK\r\nDate: " + GetTime();
                _Headers += "Content-Type: text/html; charset=UTF-8\r\n";
                _Headers += "Content-Length: " + To_String(buffer.length()) + "\r\n";
                interceptResponseHeaders(_Headers);
                _Headers += "\r\n";
                _Headers += buffer;
                write(_req.getSockFd(), _Headers.c_str(), _Headers.length());
                _HeadersSent = true;
                _Done = true;
            }
        }

        void    CheckPathErrors() {
            std::cout << "Path Error | Errno: " << strerror(errno) << std::endl;
            if (errno == EACCES)    //Search permission is denied for one of the directories in the path prefix of path
                SendError(403);
            else if (errno == ENOENT) //A component of path does not exist, or path is an empty string.
                SendError(404);
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
                        SendError(404);
                }
            }
            else //is File
                SendFile(FilePath);
        }

        void    SendWithPost(std::string FilePath) {
            // need to protect things
            std::vector<std::string> Paths = split(FilePath, "/");
            std::string FileName = Paths[Paths.size() -1];

			std::string path = _req.getHeader("Path");


            std::string targetUpload;
            if (_Location.uploadStore.empty()) {
                if (_Location.root.back() == '/')
                    _Location.root.pop_back();
                targetUpload = _Location.root + "/" + FileName;
            } else {
                if (_Location.uploadStore.back() == '/')
                    _Location.uploadStore.pop_back();
                targetUpload = _Location.uploadStore + "/" + FileName;
            }
            std::cout << "uploading to: " << targetUpload << std::endl;
            std::string cmd = "mv " + _req.getBodyFile().name + ' ' + targetUpload;
            system(cmd.c_str());
            _Headers += "HTTP/1.1 201 CREATED\r\nDate: " + GetTime();
            if (_req.getHeader("Connection") == "keep-alive")
                _Headers += "Connection: " + _req.getHeader("Connection") + "\r\n";
            interceptResponseHeaders(_Headers);
            _Headers += "Content-Length: 0\r\n";
            _Headers += "\r\n";
            if (write(_req.getSockFd(), _Headers.c_str(), _Headers.length()) < 0)
                throw std::runtime_error("error while writing to client");
            _HeadersSent = true;
            _Done = true;
        }


       int  IsFile(std::string &Path) {
            struct stat status;
            if (stat(Path.c_str(), &status) == 0) {
                if (status.st_mode & S_IFREG) // Regular File
                    return (1);
                SendError(403);
                return (0);
            }
            if (errno == EACCES) //Search permission is denied for one of the directories in the path prefix of path
                SendError(403);
            else if (errno == ENOENT) //A component of path does not exist, or path is an empty string.
                SendError(404);
            return (0);
       }

        void    SendWithDelete(std::string FilePath) {
            //Protect root srcs
            if (IsFile(FilePath)) {
                if (remove(FilePath.c_str()) == 0) {
                    _Headers += "HTTP/1.1 204 No Content\r\nDate: " + GetTime();
                    if (_req.getHeader("Connection") == "keep-alive")
                        _Headers += "Connection: " + _req.getHeader("Connection") + "\r\n";
                    interceptResponseHeaders(_Headers);
                    _Headers += "\r\n";
                    if (write(_req.getSockFd(), _Headers.c_str(), _Headers.length()) < 0)
                        throw std::runtime_error("error while writing to client");
                    _HeadersSent = true;
                    _Done = true;              
                }
                else
                    SendError(500);
            }
        }

        std::string     GetFilePath(std::string Path) {
            std::string Root = _Location.root;

            if (Root[Root.length() - 1] != '/')
                Root += "/";
            if (_CgiFound) {
                return Root + split(Path, "/").back();
            }
            std::string NewPath = Path.substr(Path.find(_Location.path) + _Location.path.length()
                                        , Path.length());
            if (NewPath[0] == '/')
                NewPath.erase(0,1);
            return Root + NewPath;
        }

        void    sendWithRedirect() {
            std::string Status = To_String(_Location.redirect.first);
            std::string Phrase = std::string(StatusCode::reasonPhrase(_Location.redirect.first));
            std::string location = _Location.redirect.second;

            _Headers += "HTTP/1.1 " + Status + " " + Phrase + "\r\n";
            _Headers += "Date: " + GetTime();
            _Headers += "Location: " + location + "\r\n";
            if (_req.getHeader("Connection") == "keep-alive")
                _Headers += "Connection: " + _req.getHeader("Connection") + "\r\n";
            interceptResponseHeaders(_Headers);
            _Headers += "\r\n";
            write(_req.getSockFd(), _Headers.c_str(), _Headers.length());
            _HeadersSent = true;
            _Done = true;
            throw std::runtime_error("close connection");
        }

        int     checkMethod(std::string Method) {
            std::vector<std::string> implMethods;

            implMethods.push_back("GET");
            implMethods.push_back("POST");
            implMethods.push_back("DELETE");
            implMethods.push_back("HEAD");
            implMethods.push_back("PUT");

            std::cout << "in check method : ";
            std::cout << _req.getHeader("Method") << std::endl;
            if (std::find(implMethods.begin(),implMethods.end(), Method) == implMethods.end())
            {
                std::cout << "not implemented !" << std::endl;
                return (501);
            }
            std::cout << "implemented !" << std::endl;
            if (std::find(
                _Location.allowedMethods.begin(),
                _Location.allowedMethods.end(),
                Method
            ) == _Location.allowedMethods.end())
                return (405);
            return (0);
        }

        void    SendWithCGI(std::string FilePath) {
            std::cout << "executing cgi ..." << std::endl;
            cgi CGI(_req, FilePath, _Location.cgiPath);
            _cgiPip = CGI.execute();
            _cgiPid = CGI.getCgiPid();
            gettimeofday(&_CgiExecDuration, NULL);
            std::cout << "cgi started ... " << std::endl;
            if (_cgiPip == -1)
                return SendError(500);
            throw CgiProcessStarted(_cgiPip);
        }

        bool     isLessMaxBody( size_t maxBodySize ) {
            size_t ReqLen = std::stoi(_req.getHeader("Content-Length"));
            return ReqLen <= maxBodySize;
        }

        bool checkBodySize( size_t maxBodySize ) {
            if (_req.getHeader("Content-Length").empty() || maxBodySize == 0)
                return false;
            if (!isLessMaxBody(maxBodySize)) {
                return true;
            }
            return false;
        }

        void    sendHeaders() {
            std::string Method = _req.getHeader("Method");
            std::string Path = _req.getHeader("Path");

            int         ErrCode;

            if (checkBodySize(_ServerBlock->maxBodySize))
                return SendError(413);
            if (!ExtractLocation(Path))
                return SendError(404);
            if (checkBodySize(_Location.maxBodySize))
                return SendError(413);
            if ((ErrCode = checkMethod(Method)))
                return SendError(ErrCode);
            std::string FilePath = GetFilePath(Path);
            std::cout << "built path " << FilePath << std::endl;
            if (_Location.redirect != defaults::EMPTY_REDIRECT)
                return sendWithRedirect();
            if (_CgiFound && (Method == "GET" || Method == "POST"))
                return SendWithCGI(FilePath);
            if (Method == "GET" || Method == "HEAD")
                return SendWithGet(FilePath);
            if (Method == "POST" || Method == "PUT")
                return SendWithPost(FilePath);
            if (Method == "DELETE")
                return SendWithDelete(FilePath);
        }

        void readingPipCgi() {
            char buff[1024];
            int readret = read(_cgiPip, buff, 1024);
            // reading from pipe protection
            if (readret < 0) {
                _Headers.clear();
                SendError(500);
                throw CgiProcessTerminated(_cgiPip);
            }
            // reading from cgi pipe until separator were found
            if (!_PipHeadersRead) {
                _Headers += std::string(buff, readret);
                if ( _Headers.find("\r\n\r\n") == std::string::npos )
                    return;
                std::string tmpHeaders;
                if (_Headers.find("Status: ") != std::string::npos) {
                    size_t start = _Headers.find("Status: ");
                    int statusCode = stoi(_Headers.substr(start + 8, start + 11));
                    if (statusCode >= 400) {
                        _Headers.clear();
                        SendError(statusCode);
                        throw CgiProcessTerminated(_cgiPip);
                    }
                    tmpHeaders +=
                            "HTTP/1.1 " + std::to_string(statusCode) + " " + StatusCode::reasonPhrase(statusCode) +
                            "\r\n";
                } else {
                    tmpHeaders += "HTTP/1.1 200 OK\r\n";
                }
                tmpHeaders += "Date: " + GetTime();
                _Headers = tmpHeaders + _Headers;
                interceptResponseHeaders(_Headers);
                if (write(_req.getSockFd(), _Headers.c_str(), _Headers.find("\r\n\r\n") + 2) < 0)
                    throw std::runtime_error("error while writing to client");
                std::cout << "{{{" << _Headers << "}}}" << std::endl;
                _Headers.erase(0, _Headers.find("\r\n\r\n") + 4);
                _PipHeadersRead = true;
                return;
            }
            // creating tmp file if not exist
            if (_cgiTmpFile == -1)  {
                _cgiFile = "/tmp/cgiFile" + GetTime();
                _cgiTmpFile = open(_cgiFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
                if (_cgiTmpFile < 0) {
                    _Headers.clear();
                    SendError(500);
                    throw CgiProcessTerminated(_cgiPip);
                }
                // writing remaining data in _Headers into cgi tmp body file
                if (!_Headers.empty()) {
                    if (write(_cgiTmpFile, _Headers.c_str(), _Headers.size()) < 0) {
                        _Headers.clear();
                        SendError(500);
                        throw CgiProcessTerminated(_cgiPip);
                    }
                    _Headers.clear();
                }
            }
            // at this point cgi have sent everything to us!
            if (readret == 0) {
                std::cout << "reach the end of pipe" << std::endl;
                close(_cgiTmpFile);
                _BodyFd = open(_cgiFile.c_str() , O_RDONLY);
                if (_BodyFd < 0) {
                    _Headers.clear();
                    SendError(500);
                    throw CgiProcessTerminated(_cgiPip);
                }
                _Headers += "Content-Length: " + To_String(getContentLength(_BodyFd)) + "\r\n\r\n";
                if (write(_req.getSockFd(), _Headers.c_str(), _Headers.size()) < 0) {
                    throw std::runtime_error("error while writing to client");
                }
                _HeadersSent = true;
                throw CgiProcessTerminated(_cgiPip);
            } else {
                if (write(_cgiTmpFile, buff, readret) < 0) {
                    _Headers.clear();
                    SendError(500);
                    throw CgiProcessTerminated(_cgiPip);
                }
            }
        }

        void interceptResponseHeaders(std::string& headers) {
            // add more headers
            (void)headers;
        }

        void supervise() {
            int secElapsed = getCgiTimeoutDuration();
            if (secElapsed > 5) {
                throw std::runtime_error("cgi timeout reached");
            }
        }

    public:

        /*
         *  main entry.
         *  engine will invoke this everytime socket is ready for writing
         */
        void sendChunk(bool pipeReady = false) {
            if (_cgiPip > -1) {
                if (pipeReady)
                    readingPipCgi();
                else
                    supervise();
            } else if (!_HeadersSent) {
                sendHeaders();
                // std::cout << _Headers;
            } else if (_BodyFd > -1) {
                sendBody();
            } else {
                throw std::runtime_error("close connection");
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

        bool isCgiActive() const {
            return _cgiPip != -1;
        }

        int getCgiFd() const {
            return _cgiPip;
        }

        /*
         *  stop cgi process for further execution and clean all resources relate to it
         */
        void stopCgi() {
            bzero(&_CgiExecDuration, sizeof(_CgiExecDuration));
            std::cout << "stopping cgi..." << std::endl;
            if (_cgiPip != -1) {
                close(_cgiPip);
                _cgiPip = -1;
            }
            // removing cgi tmp file
            if (isFileReadable(_cgiFile))
                remove(_cgiFile.c_str());
            if (_cgiPid != -1) {
                kill(_cgiPid, SIGKILL);
                bool isChildTerminated = !waitpid(_cgiPid, NULL, 0);
                if (isChildTerminated) {
                    std::cout << "couldn't stop cgi process" << std::endl;
                }
                else
                    std::cout << "cgi terminated" << std::endl;
            }
        }

        /*
         *  reset all parameters to initial state (as if object have just constructed)
         */
        void reset() {
            _req.reset();
        }

        class CgiProcessStarted : std::exception {
            int _cgiFd;

        public:

            CgiProcessStarted(int cgiFd): _cgiFd(cgiFd) {}

            int getCgiFd() const {
                return _cgiFd;
            }

            char const* what() const throw() {
                return "cgi ready to be added to select's read fds list";
            }

        };

        /*
         *  Starts cgi timeout
         */
        void startCgiTimeout() {
            gettimeofday(&_CgiExecDuration, NULL);
        }

        /*
         *  Return elapsed time of seconds since cgi started execution
         */
        int getCgiTimeoutDuration() {
            struct timeval end;
            gettimeofday(&end, NULL);

            return end.tv_sec - _CgiExecDuration.tv_sec;
        }


        class CgiProcessTerminated : std::exception {
            int _cgiFd;

        public:

            CgiProcessTerminated(int cgiFd): _cgiFd(cgiFd) {}

            int getCgiFd() const {
                return _cgiFd;
            }

            char const* what() const throw() {
                return "cgi ready to be removed from select's read fds list";
            }

        };
    };

} // namespace ws
