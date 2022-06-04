#include <unistd.h>
#include <string>
#include <iostream>
//TODO: optimize gnl later
// std::string getNextLine(int fd)
// {
//     std::string line = "";
//     char        c[2] = "\0";
//     int         ret = 1;
//     while(ret > 0)
//     {
//         ret = read( fd, c, 1 );
//         if(c[0] == '\n')
//             break;
//         if(c[0] != '\r' && ret)
//             line += c;
//     }
//     return line;
// }

std::string getNextLine(std::string& buffer)
{
    int delimPos = buffer.find("\r\n");
    std::string ret = buffer.substr(0, delimPos);
    buffer = buffer.substr(delimPos + 2, buffer.size() - 1);
    //std::cout << ret << std::endl;
    return ret;
}
