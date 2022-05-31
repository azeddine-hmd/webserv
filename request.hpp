#ifndef REQ_HPP
#define REQ_HPP

#include <string>
#include <map>
#include <iostream>
#include <fstream>


class Request
{
	private:
		int                                 _SockFd;
		std::string                         _Method;
		std::string                         _Path;
		std::string                         _HttpVersion;
		std::map<std::string, std::string>  _Headers;
		std::string							_Buffer;

		Request() {
		}
	public:
		Request( int fd ) {
			_SockFd = fd;
		}
		~Request( void ) {

		}

		void parseFirstLine( ) {
			std::fstream newfile;
			std::string str;
			FILE *file = fdopen(_SockFd,"r");
			

			
			std::getline(_SockFd, str, '\n');
			// std::cout << << str << std::endl;
			// getline (istream& is, string& str, char delim)
		}

		int parseChunk( ) {
			
			return 0;

		}

		
/*

		/n/r/n/r
		buffer = User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_6)

		str1 = 
		{GET /hello/fsdfdsfdsfdsf 0/1.1\n\r
		Host: 127.0.0.1:8081,
		Connection: keep-alive,
		sec-ch-u/r/n/r/na: " Not A;Brand";v="99", "Chromium";v="101", "Google Chrome";v="101",
		sec-ch-ua-mobile: ?0,
		sec-ch-ua-platform: "macOS",
		Upgrade-Insecure-Requests: 1,
		User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_6)AppleWebKit/537.36 (KHTML, like Gecko) Chrome/101.0.4951.64 Safari/537.36/n/r/n/r/n
		
		str2 = /r/n/rUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_6)AppleWebKit/537.36 (KHTML, like Gecko) Chrome/101.0.4951.64 Safari/537.36 + 
		 
		dfsfdsfdsfddsfdsfdsf\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,;q=0.8,application/signed-exchange;v=b3;q=0.9\n
		Sec-Fetch-Site: none\n
		Sec-Fetch-Mode: navigate\n
		Sec-Fetch-User: ?1\n
		Sec-Fetch-Dest: document\n
		Accept-Encoding: gzip, deflate, br\n

		//if last char is \n 

*/

};

#endif