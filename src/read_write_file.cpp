#include "read_write_file.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

read_write_file::read_write_file(){}

// takes a file name returns a vector of the file info
std::vector<std::string> read_write_file::read_file(std::string name)
{
    std::ifstream infile(name.c_str());
    std::vector<std::string> text;
    while (infile)
    {
        int x =0;
        std::string Str;
        while (getline(infile, Str))
        {
            text.insert(text.begin()+x, Str);
            x++;
        }
    }
    infile.close();
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
