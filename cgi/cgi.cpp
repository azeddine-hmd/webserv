#include "./cgi.hpp"

// https://www.rfc-editor.org/rfc/rfc3875.pdf

//first is to prepare the  Meta-Variables

using namespace ws;

cgi::cgi( Request ReqData ) {
    std::string root = "/home/mouad/schoolProjects/webserver/cgi/";
    std::string ScriptName = "GET.php";
    this->_ScriptPath = root + ScriptName;
    this->_Buffer = std::string();
    InitMetaVariables(ReqData);
}

cgi::~cgi( void ) {
    
}
/*
	    std::string             _Method;
	    std::string             _ScriptName;
	    std::string             _Query;
	    // std::string             _Path;
	    std::string             _Cookie;
	    std::string             _ContentType;
        int                     _ContentLength;
*/

// https://stackoverflow.com/questions/10847237/how-to-convert-from-int-to-char

void cgi::InitMetaVariables( Request Data ) {
    if (Data._ContentLength)
        setenv("CONTENT_LENGTH", std::to_string(Data._ContentLength).c_str(), 1);
    //if (content type if defined in request header)
    // setenv("CONTENT_TYPE", Data._ContentType.c_str(), 1);
    //else (by default )
    setenv("CONTENT_TYPE", "text/html; charset=UTF-8", 1);
    //cookie is optional need an if statement
    // setenv("HTTP_COOKIE", Data._Cookie.c_str(), 1);
    setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
    setenv("QUERY_STRING", Data._Query.c_str(), 1);
    setenv("REQUEST_METHOD", Data._Method.c_str(), 1);
    setenv("SCRIPT_NAME", this->_ScriptPath.c_str(), 1);
    setenv("SERVER_NAME", "CHEMCH", 1);
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
    setenv("REDIRECT_STATUS", "true", 1);
    if (this->_ScriptPath.find(".php") != std::string::npos)
        this->_ScriptType = "/usr/bin/php-cgi";
    else
        this->_ScriptType = "/usr/bin/python";

}

// https://stackoverflow.com/questions/12790328/how-to-silence-sys-excepthook-is-missing-error

void     cgi::execWithGET( char *args[3], int fd[2]) {
	dup2(fd[1], 1);
    close (fd[1]);
    std::cout << "------> " << args[0] << " " << args[1] << std::endl;
    execve(args[0], args, environ);
    std::cerr << "execve failed | errno: " << strerror(errno) << std::endl;
    exit(errno);
}

int cgi::execWithPOST( char *args[3], int fd[2], int fd2[2], Request Data) {
    struct pollfd   fds;
    int             rc;

    fds.fd = fd2[1];
    fds.events = POLLOUT;
    if ((rc = poll(&fds, 1, 0)) < 0)
        return (0);
    if (fds.revents & POLLOUT) 
        write(fds.fd, Data._Query.c_str(), Data._Query.length());
    dup2(fd2[0], 0);
	dup2(fd[1], 1);
    close (fd2[0]);
    close(fd2[1]);
    close (fd[1]);
    execve(args[0], args, environ);
    std::cerr << "execve failed | errno: " << strerror(errno) << std::endl;
    exit(errno);
}

cgi::ReturnType     cgi::execute(Request Data) {
    int     pid;
    int     fd[2];
    int     fd2[2];
    struct pollfd fds;
    char    *args[3] = {
        (char *)_ScriptType.c_str(),
        (char *)_ScriptPath.c_str(),
        NULL
    };

    if (Data._Method == "GET") {
        pipe(fd);
        pid = fork();
        if (pid < 0)
            return (ReturnType("500", _Error_));
        else if (pid == 0)
            execWithGET(args, fd);
        close(fd[1]);
    }
    else if (Data._Method == "POST") {

        pipe(fd);
        pipe(fd2);
        pid = fork();
        if (pid < 0)
            return (ReturnType("500", _Error_));
        else if (pid == 0) {
            if (!execWithPOST(args, fd, fd2, Data))
                return (ReturnType("500", _Error_));
        }
        close(fd[1]);
        close(fd2[0]);
        close(fd2[1]);
    }
    // need to wait for child process to be terminated
        // - maybe later :)
    // parse executed program output and store it
    fds.fd = fd[0];
    fds.events = POLLIN;
    char val;
    int rc;
    while (1) {
        if ((rc = poll(&fds, 1, 0)) < 0)  // poll ret 0 == timeout || < 0  == error
                return (ReturnType("500", _Error_));
        if (rc == 1 && fds.events & POLLIN ) {
            if (int count = read(fds.fd, &val, 1) < 1) {
                if (count < 0) {
                    close(fd[0]);
                    return (ReturnType("500", _Error_));
                }
                break;
            }
            this->_Buffer += val;
	    }
    }
    close(fd[0]);
    // need to check if the script executed want to redirect (302 | 301) | err 500
    return (ReturnType("201", "HTTP/1.1 200 OK\r\n"));
}

cgi::ValueType   cgi::GetBuffer( void ) {
    return this->_Buffer;
}