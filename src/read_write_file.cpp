#include "read_write_file.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <vector>

// takes a file name returns a vector of the file info
std::vector<std::string> read_write_file::read_file(const std::string& name)
{
    std::ifstream infile(name.c_str());
    std::vector<std::string> text;

    if(!infile.good()) {
        return text;
    }

    std::copy(std::istream_iterator<std::string>(infile),
              std::istream_iterator<std::string>(),
              std::back_inserter(text));

    return text;
}

// takes a vector and a file name writes vector to filename
void read_write_file::write_file(std::vector<std::string> text, std::string name)
{
    remove(name.c_str());
    std::ofstream myfile;
    myfile.open (name.c_str(), std::ios::app);
    for (int i =0; i < text.size(); i++)
    {
        myfile << text[i];
        myfile << "\n";
    }
}
