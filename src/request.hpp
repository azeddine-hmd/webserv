#ifndef REQ_HPP
#define REQ_HPP

#include <map>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <cstdlib>
#include <fcntl.h>
#include "utils.hpp"

struct BodyFile
{
	public:
		int 		fd;
		std::string name;
};

// #include "chunkedDecoder.hpp"

#define BUFFER_SIZE 1024

// std::string getNextLine(std::string& buffer);


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
        std::string                         _Host;
        uint16_t                            _Port;
        bool                                _Chunked; // true if chunked encoding is used
        int                                 _ChunkSize; // size of the current chunk

    public:
        Request( int fd, std::string host, uint16_t port ): _Host(host), _Port(port) {
            _SockFd = fd;
            _HeaderDone = false;
            _RequestDone = false;
            _Chunked = false;
            _BodyBuffer = "";
            _BodyFile.fd = -1;
            _ChunkSize = -1;
            _bodySize = 0;
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
        int parseParam ( void ) {
			std::string str = getNextLine(_BodyBuffer);
			if(str.size() == 0)
				return -1;
			int pos = str.find(":");
			std::string key = str.substr(0, pos);
			str.erase(0, pos + 2);
			_Headers[key] = str;
			return 0;
		}

        void ChunkedDecode()
        {
            int delimPos;
            if(_ChunkSize >= 0 && (int)_BodyBuffer.size() < _ChunkSize + 2)
            {
                return;
            }
            while(((delimPos = _BodyBuffer.find("\r\n")) != -1 && _ChunkSize == -1) || _ChunkSize > 0)
            {
                if(_ChunkSize == -1)
                {
                    
                    _ChunkSize = hexToDec(_BodyBuffer.substr(0,delimPos));
                    _bodySize += _ChunkSize;
                    _BodyBuffer.erase(0, delimPos + 2);
                    if(_ChunkSize == 0 && _BodyBuffer.size() >= 2)
                    {
                        _RequestDone = true;
                        _Headers["Content-Length"] = std::to_string(_bodySize);
                        _BodyBuffer.erase(0, 2); 
                        return;
                    }
                }
                else
                {
                    if((int)_BodyBuffer.size() >= _ChunkSize + 2)
                    {
                        write(_BodyFile.fd, _BodyBuffer.c_str(), _ChunkSize);
                        _BodyBuffer.erase(0, _ChunkSize + 2);
                        _ChunkSize = -1;
                    }
                    else
                        return;
                }
            }
        }

        // parse the whole header part of a request
        void parseRequestHeader ( void ) {
			char buf[BUFFER_SIZE];
			int ret = read(_SockFd, buf, BUFFER_SIZE);
			if (ret <= 0) 
                	throw std::runtime_error("error while reading");
			_BodyBuffer += std::string(buf, ret);
            if (_BodyBuffer.find("\r\n\r\n") == std::string::npos)
                return;
			parseFirstLine();
			while(parseParam() != -1);
			_HeaderDone = true;
            if(_Headers.find("Transfer-Encoding") != _Headers.end() && _Headers["Transfer-Encoding"] == "chunked")
            {
                _Chunked = true;
            }
			else if(_Headers.find("Content-Length") != _Headers.end())
			{
				std::istringstream iss(_Headers["Content-Length"]);
				iss >> _targetSize;
			}
			else
			{
				_RequestDone = true;
				return;
			}
			CreateFile();
            if(_Chunked)
                ChunkedDecode();
            else
            {
			    write(_BodyFile.fd, _BodyBuffer.c_str(), _BodyBuffer.size());
			    _bodySize = _BodyBuffer.size();
                _BodyBuffer = "";
                if(_bodySize == _targetSize)
                {
                    _RequestDone = true;
                    std::cout << "done" << std::endl;
                    return;
                }
            }
		}

        // get a parameter by key
        std::string getHeader( std::string key ) {
            if(_Headers.find(key) == _Headers.end())
                return "";
            return _Headers[key];
        }

        bool checkHeader( std::string key )
        {
            return (_Headers.find(key) != _Headers.end());
        }

        // get a parameter by key
        std::string getBody() {
            return _BodyBuffer;
        }

        // true if the request is done else false;
        bool getStatus() {
            return _RequestDone;
        }

        int getSockFd() const {
            return _SockFd;
        }

        void CreateFile() {
            int random = (int) time(nullptr);
            std::string n = std::string("tmp/USER_") + std::to_string(random);
            _BodyFile.fd = open(n.c_str(), O_CREAT | O_WRONLY, 0644);
            _BodyFile.name = n;
        }

        void readChunk ( void ) {
			if(_HeaderDone)
			{
				char buf[BUFFER_SIZE];
				int ret = read(_SockFd, buf, BUFFER_SIZE);
				if (ret <= 0) 
                	throw std::runtime_error("error while reading");
                if(_Chunked)
                {
                    _BodyBuffer += std::string(buf, ret);
                    ChunkedDecode();
                }
                else
                {
                    _bodySize += ret;
                    write(_BodyFile.fd, buf, ret);
                    if(_bodySize == _targetSize)
                    {
                        _RequestDone = true;
                        std::cout << "done" << std::endl;
                        return;
                    }
                }
			}
			else
				parseRequestHeader();
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
            _ChunkSize = -1;
            _Chunked = false;
        }

        std::string getHost() const {
            return _Host;
        }

        uint16_t getPort() const {
            return _Port;
        }

        BodyFile const &getBodyFile() const {
            return _BodyFile;
        }

    };
} // namespace ws
#endif