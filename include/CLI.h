#ifndef CLI_H
#define CLI_H
#include <iostream>
#include <string>
#include <sstream>  //for std::istringstream
#include <iterator> //for std::istream_iterator
#include <vector>   //for std::vector
using namespace std;
class CLI
{
    public:
        vector<string> getString();
        void init();
        CLI();
        virtual ~CLI();

    protected:

    private:
        bool pass(string);
};

#endif // CLI_H
