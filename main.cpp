#include <iostream>
#include <string>
#include <sstream>
#include "cgi/cgi.cpp"

// using namespace ws;

/*
	    std::string             _Method;
	    std::string             _ScriptName;
	    std::string             _Query;
	    // std::string             _Path;
	    std::string             _Cookie;
	    std::string             _ContentType;
        int                     _ContentLength

*/

int main()
{

	try {
    	ws::Request ReqObj("POST","cgi-python", "name=mouad","text/html; charset=UTF-8", 15);
    	ws::cgi CgiObj(ReqObj);
		CgiObj.execute(ReqObj);
		std::cout << CgiObj.GetBuffer() << std::endl;
	}
	catch (std::exception &event) {
		std::cout << event.what() << std::endl;
	}
    return (0);
}