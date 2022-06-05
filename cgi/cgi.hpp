#pragma once

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <map>
#include <cstdlib>
#include <vector>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <istream>
#include <sys/types.h>
#include <signal.h>
#include <string>
#include <algorithm>
#include <poll.h>

#define _Error_ "HTTP/1.1 500 Internal Server Error\r\n"

extern  char** environ;

namespace ws {

	struct Request {
	    std::string     _Method;
	    std::string     _ScriptPath;
	    std::string     _Query;
	    std::string     _Path;
	    std::string     _Cookie;
	    std::string     _ContentType;
	    int    			_ContentLength;

		typedef typename std::string valType;

		Request(valType Method, valType ScriptPath, valType Query, valType ContentType, int ContentLength
		, valType Path = valType(), valType Cookie = valType()) {
			_Method = Method;
	    	_ScriptPath = ScriptPath;
	    	_Query = Query;
	    	_Path = Path;
	    	_Cookie = Cookie;
	    	_ContentType = ContentType;
	    	_ContentLength = ContentLength;
		};

		Request() {};
		~Request() {};
	};

	class cgi {
		private:
			typedef typename std::pair<std::string, std::string>	ReturnValue;
			typedef typename std::string							ValueType;
			ValueType	_ScriptType;
			ValueType	_ScriptPath;
			ValueType	_Buffer;

		public:
			// cgi( void ) {}
			// cgi( Request ReqData, std::string root, std::string scriptName);
			cgi( Request ReqData );
			~cgi( void );
			void InitMetaVariables( Request ReqData );
			ReturnValue	execute( Request Data );
			void		execWithGet( char *args[3], int fd[2]);
			ValueType	GetBuffer( void );


	};

}