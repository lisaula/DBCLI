#include "CLI.h"

const string commands = "1. CREATE \n\tDATABASE <NAME> <SIZE_IN_GB>[;]\n\tTABLE <NAME> \n\t\t <FIELD_NAME> <TYPE(SIZE)>[;]\n>"
"2. DROP \n\tDATABASE <NAME>[;]\n\tTABLE <NAME>[;]\n"
"3. INSERT INTO <TABLE>\n\t<COLUMN1> = <VALUE>\n\t<COLUMN2> = <VALUE>\n\t<COLUMN...> = <VALUE> [;]\n"
"4. UPDATE <TABLE>\n\t<COLUMN1> = <VALUE>\n\t<COLUMN2> = <VALUE>\n\t<COLUMN...> = <VALUE>\n   WHERE [CONDITION] [;]\n"
"5. DELETE <TABLE>\n   WHERE [CONDITION] [;]\n"
"6. SELECT [*]\n\t<FIELD1>\n\t<FIELD2>\n\t<FIELD3>\n   FROM <TABLE>\n   WHERE [CONDITION] [;]\n"
"7. HELP[;]\n"
"8. EXIT [;]";
CLI::CLI()
{
    //ctor
    init();
}

void printMsg(string msg){
    cout<<msg<<endl;
}

bool validateEntranceLen(vector<string> entrance, int expected){
    if(entrance.size() < expected){
        printMsg("Not enough entrance detected");
        return false;
    }
    return true;
}

void CLI::init(){
    bool exit = false;
    while(!exit){
        vector<string> entrance = getString();
        if(entrance.size() ==0)
            continue;
        if(entrance[0] == "HELP"){
            cout<<commands<<endl;
        }else if(entrance[0] == "CREATE"){

            if(validateEntranceLen(entrance, 2) && entrance[1] == "TABLE"){
                printMsg("Create table successfully");
            }else if(validateEntranceLen(entrance, 2) && entrance[1] == "DATABASE"){
                printMsg("Create database successfully");
            }else{
                printMsg("Command not supported.");
            }
        }
        else if(entrance[0] == "DROP"){
            if(validateEntranceLen(entrance, 2) && entrance[1] == "TABLE"){
                printMsg("Drop table successfully");
            }else if(validateEntranceLen(entrance, 2) && entrance[1] == "DATABASE"){
                printMsg("Drop database successfully");
            }else{
                printMsg("Command not supported.");
            }
        }
        else if(entrance[0] == "INSERT"){
            printMsg("Insert command.");
        }
        else if(entrance[0] == "UPDATE"){
            printMsg("Update command.");
        }
        else if(entrance[0] == "DELETE"){
            printMsg("Delete command.");
        }
        else if(entrance[0] == "SELECT"){
            printMsg("Select command.");
        }
        else if(entrance[0] == "EXIT"){
            exit = true;
        }else{
            cout<<"Command not supported."<<endl;
        }
    }
}

vector<string> CLI::getString(){

    cout<<"DBCLI >";
    string txt;
    vector<string> entrance;
    bool exit = false;
    while(!exit){
        getline(cin, txt);
        if (txt.find(";") != std::string::npos){
            int len = txt.size();
            txt[len-1]=' ';
            exit = true;
        }

        std::istringstream ss(txt);
        std::istream_iterator<std::string> begin(ss), end;
        std::vector<std::string> arrayTokens(begin, end);
        while(arrayTokens.size()>0){
            entrance.push_back(arrayTokens[0]);
            arrayTokens.erase(arrayTokens.begin());
        }
    }
    return entrance;
}

CLI::~CLI()
{
    //dtor
}