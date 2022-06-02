#ifndef REQ_HPP
#define REQ_HPP

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <cstdlib>
#include <ctime>

#include <fcntl.h>


std::string getNextLine(int fd);
#define BUFFER_SIZE 10000


struct BodyFile
{
	public:
		int 		fd;
		std::string name;
};


class Request
{
	private:
		int                                 _SockFd; // socket file descriptor
		std::map<std::string, std::string>  _Headers; // map contains all the headers sent by the client including the method, path, http version
		std::string							_BodyBuffer; // body :)
		bool								_HeaderDone; // true after parsing the headers
		bool								_RequestDone; // true when req is done
		BodyFile							_BodyFile;

		Request( void ) {

		}
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
			std::string str = getNextLine(_SockFd);
			std::stringstream ss(str);
			ss >> _Headers["Method"];
			ss >> _Headers["Path"];
			ss >> _Headers["Version"];
		}
		// parse one line that contains one header example: (Content-Length: 531)
		int parseParam ( void ) {
			std::string str = getNextLine(_SockFd);
			if(str.size() == 0)
				return -1;
			std::stringstream ss(str);
			std::string key;
			ss >> key;
			key.pop_back();
			ss >> _Headers[key];
			return 0;
		}
		// parse the whole header part of a request
		void parseRequestHeader ( void ) {
			parseFirstLine();
			while(parseParam() != -1);
			_HeaderDone = true;
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

		void CreateFile()
		{
			int random = (int)time(nullptr);
			std::string n = std::string("USER_") + std::to_string(random);
			_BodyFile.fd = open(n.c_str(), O_CREAT | O_WRONLY, 0644);
			_BodyFile.name = n;
		}

		// parse the header in the first call then reads a chunk of BUFFER_SIZE bits
		void readChunk ( void ) {
			if(_HeaderDone)
			{
				char buf[BUFFER_SIZE];
				int ret = read(_SockFd, buf, BUFFER_SIZE);
				
				if(ret <= 0)
				{
					_RequestDone = true;
					return;
				}
				if(ret > 0)
				{
					if(_BodyFile.fd == -1)
						CreateFile();
					write(_BodyFile.fd, buf, ret);
				}	
			}
			else
			{
				parseRequestHeader();
			}
		}
};

#endif