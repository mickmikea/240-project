#include <iostream>
#include "read_write_file.h"
#include <vector>

int main()
{
    read_write_file rw;
    std::vector<std::string> text;
    text = rw.read_file("test.txt");
    rw.write_file(text, "out.txt");

   return 0;
}
