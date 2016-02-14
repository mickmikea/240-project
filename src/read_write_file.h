#ifndef READ_WRITE_FILE_H
#define READ_WRITE_FILE_H
#include <vector>
#include <iostream>

// header file for read and write functions
class read_write_file
{
    public:
        read_write_file();
        std::vector<std::string> read_file(std::string name); // takes a file name returns a vector of the file info
        void write_file(std::vector<std::string>, std::string); // takes a vector and a file name writes vector to filename

    protected:
    private:
};

#endif // READ_WRITE_FILE_H
