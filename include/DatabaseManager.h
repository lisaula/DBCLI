#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#include<windows.h>
#include "Utils.h"
class DatabaseManager
{
    public:
        string use;
        struct Database_Handler dbh;
        void create_database(string name, UINT32 database_size);
        void use_database(string name);
        void drop_database(string name);
        void create_table(vector<string>);
        DatabaseManager();
        virtual ~DatabaseManager();

    protected:

    private:
};

#endif // DATABASEMANAGER_H
