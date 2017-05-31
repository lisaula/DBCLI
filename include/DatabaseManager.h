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
        void print_database_info();
        void print_table_info(string name);
        void use_database(string name);
        void drop_database(string name);
        void create_table(vector<string>);
        void insert_command(vector<string> entrance);
        void select_command(vector<string> entrance);
        DatabaseManager();
        virtual ~DatabaseManager();

    protected:

    private:
        string print_on_column(string s,void* value,uint32 size,int pad, Type t, bool end_line);
        void validate_fields(vector<pair<string,string> > fields_value,vector<struct field> fields, char * block);
        string validate_char_value(string value);
        int validate_int_value(string value);
        double validate_double_value(string value);
};

#endif // DATABASEMANAGER_H
