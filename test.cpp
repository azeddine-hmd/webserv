#include "src/chunkedDecoder.hpp"

int main()
{
    BodyFile file;
    file.fd = open("test", O_WRONLY | O_CREAT, 0644);
    file.name = "test";

    ChunkedDecoder cd(file);
    std::string buffer = std::string("5\r\nhe\0lo\r\n6\r\n world\r\n",21);
    cd.decode(buffer);
    buffer = std::string("5\r\n 1337\r\n0\r\n\r\n",15);
    cd.decode(buffer);
    std::string test("he\0\0llo",7);
    std::string test2("wo\0\0rld",7);
    test = test + test2;
    
    std::cout << test.size() << std::endl;

}