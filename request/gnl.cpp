#include <unistd.h>
#include <string>

//TODO: optimize gnl later
std::string getNextLine(int fd)
{
    std::string line = "";
    char        c[1] = "";
    int         ret = 1;

    while(ret > 0)
    {
        ret = read( fd, c, 1 );
        if(c[0] == '\n')
            break;
        if(c[0] != '\r' && ret)
            line += c;
    }
    return line;
}