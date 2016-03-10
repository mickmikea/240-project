#ifndef READ_WRITE_FILE_H
#define READ_WRITE_FILE_H
#include <vector>
#include <iostream>

// header file for read and write functions
namespace read_write_file
{
    std::vector<std::string> read_file(const std::string& name); // takes a file name returns a vector of the file info
    void write_file(std::vector<std::string>, std::string); // takes a vector and a file name writes vector to filename
};

#endif // READ_WRITE_FILE_H
