#ifndef CLI_H
#define CLI_H
#include <iterator> //for std::istream_iterator
#include "Utils.h"
#include "DatabaseManager.h"
#include "Lexer.h"
class CLI
{
    public:
        Lexer *lexer;
        DatabaseManager * dbm;
        string getString();

        void init();
        CLI();
        virtual ~CLI();

    protected:

    private:
        bool pass(string);
};

#endif // CLI_H
