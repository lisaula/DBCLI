#include <iostream>
#include <string.h>
#include "CLI.h"
#include "DatabaseManager.h"
using namespace std;

string getNext(){
    string hola = "hola";
    return hola;
}
int main()
{
    cout << "..............Welcome..............\n" << endl;
    CLI * cli = new CLI();
    /*char n[10] = "mama";
    char block[20];
    memset(block,0,20);
    block[0] = 'a';
    memcpy(&block[1],n,4);
    char n2[10];
    memcpy(n2,&block[1],4);
    n2[4] = (char)NULL;
    cout<<n2<<endl;
    cout<<block<<endl;*/
    /*vector<struct field> fields;

    struct field f1,f2,f3;
    strcpy(f1.name,"hola");
    f1.type = 1;
    f1.size = 11;
    fields.push_back(f1);

    strcpy(f2.name,"mama");
    f2.type = 2;
    f2.size = 12;
    fields.push_back(f2);

    strcpy(f3.name,"como");
    f3.type = 3;
    f3.size = 13;
    fields.push_back(f3);

    uint32 ptr_next_block = -1;
    char block[1024];
    memcpy(block,(void*)&ptr_next_block,BLOCK_PTR_SIZE);
    cout<<BLOCK_PTR_SIZE<<" PTR"<<endl;
    struct field *fields_array = &fields[0];
    memcpy(&block[BLOCK_PTR_SIZE],(void*)fields_array,FIELD_SIZE*3);

    struct field n;
    memcpy((void*)&n,&block[BLOCK_PTR_SIZE+(0*FIELD_SIZE)],FIELD_SIZE);
    cout<<n.name<<endl;
    cout<<n.type<<endl;
    cout<<n.size<<endl;

    memcpy((void*)&n,&block[BLOCK_PTR_SIZE+(1*FIELD_SIZE)],FIELD_SIZE);
    cout<<n.name<<endl;
    cout<<n.type<<endl;
    cout<<n.size<<endl;

    memcpy((void*)&n,&block[BLOCK_PTR_SIZE+(2*FIELD_SIZE)],FIELD_SIZE);
    cout<<n.name<<endl;
    cout<<n.type<<endl;
    cout<<n.size<<endl;*/


    return 0;
}
