#ifndef CD_HPP
#define CD_HPP

#include <string>
#include <stdlib.h>
#include <fcntl.h>
#include <ctime>
#include <iostream>
#include <unistd.h>

std::string getNextLine(int fd);
struct BodyFile
{
	public:
		int 		fd;
		std::string name;
};

int hexToDec(std::string num)
{
    std::string set     = "0123456789ABCDEF";
    int         ret     = 0;
    int         pow     = 1;
    int         index   = num.size() - 1;

    while(index >= 0)
    {
        ret += set.find(num[index--]) * pow;
        pow *= 16;
    }
    return ret;
}

class ChunkedDecoder
{
    private:
        BodyFile _inFile;
        BodyFile _outFile;
    public:
        ChunkedDecoder(std::string in)
        {
            _inFile.name = in;
            _inFile.fd = open(in.c_str(), O_RDONLY);
            CreateFile();
        }

        void CreateFile()
		{
			int random = (int)time(nullptr);
			std::string n = std::string("USER_") + std::to_string(random);
			_outFile.fd = open(n.c_str(), O_CREAT | O_WRONLY, 0644);
			_outFile.name = n;
		}

        void decode()
        {
            int shunkSize;

            while(1)
            {
                shunkSize = hexToDec(getNextLine(_inFile.fd));
                if(shunkSize == 0)
                {
                    close(_inFile.fd);
                    close(_outFile.fd);
                    return;
                }
                char buffer[shunkSize];
                read(_inFile.fd, buffer, shunkSize + 2);
                write(_outFile.fd, buffer, shunkSize);
            }
        }
};


#endif