#ifndef REQ_HPP
#define REQ_HPP

#include <map>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <cstdlib>


std::string getNextLine(std::string& buffer);

struct BodyFile
{
	public:
		int 		fd;
		std::string name;
};

#define BUFFER_SIZE 1024

class Request
{
	private:
		int                                 _SockFd; // socket file descriptor
		std::map<std::string, std::string>  _Headers; // map contains all the headers sent by the client including the method, path, http version
		std::string							_BodyBuffer; // body :)
		bool								_HeaderDone; // true after parsing the headers
		bool								_RequestDone; // true when req is done
		BodyFile							_BodyFile;
		size_t								_bodySize;	
		size_t								_targetSize;				

	public:
		Request( int fd ) {
			_SockFd 		= fd;
			_HeaderDone 	= false;
			_RequestDone 	= false;
			_BodyBuffer 	= "";
			_BodyFile.fd 	= -1;
		}
		~Request( void ) {

		}
		// parse the first line of a request example: ()
		void parseFirstLine ( void ) {
			std::string str = getNextLine(_BodyBuffer);
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
		// parse the whole header part of a request
		void parseRequestHeader ( void ) {
			char buf[BUFFER_SIZE * 10];
			int ret = read(_SockFd, buf, BUFFER_SIZE * 10);
			if (ret <= 0) 
                	throw std::runtime_error("error while reading");
			_BodyBuffer = std::string(buf, ret);
			parseFirstLine();
			
			while(parseParam() != -1);
			_HeaderDone = true;
			if(_Headers.find("Content-Length") != _Headers.end())
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
			write(_BodyFile.fd, _BodyBuffer.c_str(), _BodyBuffer.size());
			_bodySize = _BodyBuffer.size();
			if(_bodySize == _targetSize)
			{
				_RequestDone = true;
				std::cout << "done" << std::endl;
				return;
			}
		}
		// get a parameter by key
		std::string getHeader( std::string key ) {
			return _Headers[key];
		}

		// get a parameter by key
		std::string getBody( void ) {
			return _BodyBuffer;
		}
		
		// true if the request is done else false;
		bool getStatus( void ) {
			return _RequestDone;
		}

		int getFd( void ) {
			return _SockFd;
		}

		std::string getFile()
		{
			return _BodyFile.name;
		}

		void CreateFile()
		{
			int random = (int)time(nullptr);
			std::string n = "www/uploads/upload_file"; //std::string("USER_") + std::to_string(random);
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
				std::string buffer = std::string(buf, ret);
				_bodySize += ret;
				if(ret > 0)
					write(_BodyFile.fd, buf, ret);
				if(_bodySize == _targetSize)
				{
					_RequestDone = true;
					std::cout << "done" << std::endl;
					return;
				}
			}
			else
			{
				parseRequestHeader();
			}
		}
};
#endif