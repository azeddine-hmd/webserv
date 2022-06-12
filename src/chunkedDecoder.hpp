#ifndef CD_HPP
#define CD_HPP

#include <string>
#include <stdlib.h>
#include <fcntl.h>
#include <ctime>
#include <iostream>
#include <unistd.h>
#include "utils.hpp"

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

// class ChunkedDecoder
// {
//     private:
//         BodyFile _inFile;
//         BodyFile _outFile;
//     public:
//         ChunkedDecoder(std::string in)
//         {
//             _inFile.name = in;
//             _inFile.fd = open(in.c_str(), O_RDONLY);
//             CreateFile();
//         }

//         void CreateFile()
// 		{
// 			int random = (int)time(nullptr);
// 			std::string n = std::string("USER_") + std::to_string(random);
// 			_outFile.fd = open(n.c_str(), O_CREAT | O_WRONLY, 0644);
// 			_outFile.name = n;
// 		}

//         void decode()
//         {
//             int shunkSize;

//             while(1)
//             {
//                 shunkSize = hexToDec(getNextLine(_inFile.fd));
//                 if(shunkSize == 0)
//                 {
//                     close(_inFile.fd);
//                     close(_outFile.fd);
//                     return;
//                 }
//                 char buffer[shunkSize];
//                 read(_inFile.fd, buffer, shunkSize + 2);
//                 write(_outFile.fd, buffer, shunkSize);
//                 std::cout << shunkSize << " " << buffer << std::endl;
//             }
//         }
// };


class ChunkedDecoder
{
	private:
		std::string	_prvBuffer;
		int			_byteNum;
		BodyFile	_outFile;
		int			_index;
		int			_bufferSize;
	public:
		ChunkedDecoder()
		{
			_outFile = BodyFile();
			_prvBuffer = "";
			_byteNum = -1;
			_index = 0;
		}
		ChunkedDecoder (BodyFile& out)
		{
			_outFile = out;
			_prvBuffer = "";
			_byteNum = -1;
			_index = 0;
		}
		void SetFile(BodyFile& out)
		{
			_outFile = out;
		}
		void getNum(std::string& buffer)
		{
			int pos = buffer.find("\r\n", _index);
			if(pos != -1)
			{
				_byteNum = hexToDec(buffer.substr(_index, pos - _index));
				_index = pos + 2;
			}
			else
			{
				_prvBuffer = buffer.substr(_index, _bufferSize - 1);
				_index = _bufferSize;
			}
		}
		
		void readNBytes(std::string& buffer)
		{
			if(_bufferSize - _index >= _byteNum + 2)
			{
				write(_outFile.fd, buffer.substr(_index, _byteNum).c_str(), _byteNum);
				_index += _byteNum + 2;
				_prvBuffer = "";
				_byteNum = -1;
			}
			else
			{
				std::string to_print = buffer.substr(_index, _byteNum - _index);
				write(_outFile.fd, to_print.c_str(), to_print.size());
				_byteNum -= to_print.size();
				_index += to_print.size();
			}

		}

		int decode (std::string& buffer)
		{
			buffer = _prvBuffer + buffer;
			_prvBuffer = "";
			_index = 0;
			_bufferSize = buffer.size();
			while(_index < _bufferSize)
			{
				if (_byteNum == -1)
				{
					getNum(buffer);
				}
				if (_byteNum == 0)
				{
					return 0;
				}
				if(_byteNum > 0)
				{
					readNBytes(buffer);
				}
			}
			return 1;
		}
};

#endif



