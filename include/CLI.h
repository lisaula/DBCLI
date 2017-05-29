#ifndef CLI_H
#define CLI_H
#include <iterator> //for std::istream_iterator
#include "Utils.h"
#include "DatabaseManager.h"
class CLI
{
    public:
        DatabaseManager * dbm;
        vector<string> getString();
        void init();
        CLI();
        virtual ~CLI();

    protected:

    private:
        bool pass(string);
};

#endif // CLI_H
