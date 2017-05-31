#include "Utils.h"
void printMsg(string msg){
    cout<<msg<<endl;
}

uint64 from_GB_bytes_convertion(uint32 n, int type){
    if(type == TO_BYTE){
        return ((n*1024) * 1024) * 1024;
    }
    return 0;
}

uint64 from_MB_bytes_convertion(uint32 n, int type){
    if(TO_BYTE){
        return ((n*1024) * 1024);
    }
    return 0;
}

uint32 get_field_padding(struct field f, vector<struct field> fields){
    uint32 padding = 1;
    for(uint32 i = 0; i< fields.size();i++){
        if(f.index == fields[i].index)
            return padding;
        padding += fields[i].size;
    }
    return padding;
}

vector<struct field>* get_fields(char *block, struct i_table it){
    vector<struct field> *fields = new vector<struct field>();
    int fields_count =0;
    while (fields_count < it.fields_count){
        struct field f;
        uint32 pos = BLOCK_PTR_SIZE+(fields_count*FIELD_SIZE);
        if(pos < BLOCK_SIZE){
            memcpy((char*)&f,&block[pos],FIELD_SIZE);
            fields->push_back(f);
            fields_count++;
        }else{
            printMsg("block finished");
            return NULL;
        }
    }
    return fields;
}

string trim(const string& str, char trim_value)
{
    size_t first = str.find_first_not_of(trim_value);
    if (string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(trim_value);
    return str.substr(first, (last - first + 1));
}

void from_String_to_uint(string s, uint32 *n){
    stringstream(s)>>(*n);
}

void from_String_to_double(string s, double *n){
    stringstream(s)>>(*n);
}
void from_String_to_int(string s, int *n){
    stringstream(s)>>(*n);
}
void read_itable(struct Database_Handler dbh, struct i_table *it, uint32 n_itable){
    string database_path = PATH+dbh.sb.name;
    database_path += ".dat";
    ifstream in(database_path.c_str(),ios::in | ios::out | ios::binary);
    if(!in){
        cout<<"Error while trying to read: "<<dbh.sb.name<<endl;
        return;
    }
    uint64 pos = SB_SIZE + (n_itable * ITABLE_SIZE);
    in.seekg(pos,ios::beg);
    in.read((char*)it,ITABLE_SIZE);
    in.close();
}

void write_itable(struct Database_Handler dbh, struct i_table it, uint32 n_itable){
    string database_path = PATH+dbh.sb.name;
    database_path += ".dat";
    ofstream output_file(database_path.c_str(), ios::in | ios::out | ios::binary);
    if(!output_file){
        cout<<"Error while trying to write: "<<dbh.sb.name<<endl;
        return;
    }
    uint64 pos = SB_SIZE + (n_itable * ITABLE_SIZE);
    output_file.seekp(pos,ios::beg);
    output_file.write(((char*)&it),ITABLE_SIZE);
    output_file.close();
}

void read_block(struct Database_Handler dbh, char *block, uint32 n_block){
    string database_path = PATH+dbh.sb.name;
    database_path += ".dat";
    ifstream in(database_path.c_str(),ios::in | ios::out | ios::binary);
    if(!in){
        cout<<"Error while trying to read: "<<dbh.sb.name<<endl;
        return;
    }
    uint64 pos = dbh.sb.FD_size + (n_block * BLOCK_SIZE);
    in.seekg(pos,ios::beg);
    in.read(block,BLOCK_SIZE);
    in.close();
}

bool white_spaces(vector<string> *entrance){
    if((*entrance)[0] == "\r" || (*entrance)[0] == "\n" || (*entrance)[0] == "\t"
    || (*entrance)[0] == " " || (*entrance)[0] == ""){
        erase_from_vector(entrance,1);
        return true;
    }
    return false;
}

bool find_i_table(struct Database_Handler dbh,string name, struct i_table *it){
    uint32 i_tables_actives = dbh.sb.itables_count - dbh.sb.free_itables_count;
    uint32 checked = 0;
    //while(checked < i_tables_actives){
        for(uint32 i =0; checked < i_tables_actives && i < dbh.sb.itables_count; i++){
            if(!is_block_in_use(dbh.itable_bitmap,i))
                continue;

            read_itable(dbh,it,i);
            if(strcmp(it->name,name.c_str())==0){
                //cout<<"index itable f: "<<i<<endl;
                return true;
            }
            if(it->first_block != (uint32)-1)
                checked++;
        }
    //}
    return false;
}
vector<char*> *read_all_table(struct Database_Handler dbh,struct i_table it){
    vector<char*> * blocks = new vector<char*>();
    uint32 ptr  = it.first_block;
    do{
        char *block = new char[BLOCK_SIZE];
        read_block(dbh,block,ptr);
        blocks->push_back(block);
        memcpy((void*)&ptr,block,BLOCK_PTR_SIZE);
        cout<<"leyo"<<endl;
    }while(ptr != (uint32)-1 );
    return blocks;
}

vector<uint32> * get_all_tables_used_blocks(struct Database_Handler dbh,struct i_table it){
    vector<uint32> * blocks = new vector<uint32>();
    uint32 ptr  = it.first_block;
    do{
        blocks->push_back(ptr);
        char block[BLOCK_SIZE];
        read_block(dbh,block,ptr);
        memcpy((void*)&ptr,block,BLOCK_PTR_SIZE);
    }while(ptr != (uint32)-1 );
    return blocks;
}

void write_block(struct Database_Handler dbh, char *block, uint32 n_block){
    string database_path = PATH+dbh.sb.name;
    database_path += ".dat";
    ofstream output_file(database_path.c_str(), ios::in | ios::out | ios::binary);
    if(!output_file){
        cout<<"Error while trying to write: "<<dbh.sb.name<<endl;
        return;
    }
    uint64 pos =  dbh.sb.FD_size + (n_block * BLOCK_SIZE);
    output_file.seekp(pos,ios::beg);
    output_file.write(block,BLOCK_SIZE);
    output_file.close();
}

int p_field_is_contained(string p_field, vector<struct field> fields){
    for(uint32 i =0; i < fields.size(); i++){
        if(strcmp(p_field.c_str(),fields[i].name) == 0)
            return i;
    }
    return -1;
}

void write_record(struct Database_Handler dbh, struct i_table it, char * block_field, uint32 index){
    uint32 first_pos = (it.table_size - (it.records_count* it.record_size ));
    uint32 pos = first_pos + (index*it.record_size);
    uint32 block_index = ceil((double)pos / (double)BLOCK_SIZE);
    char block[BLOCK_SIZE];
    uint32 blocks_read = 0;
    uint32 ptr = it.first_block;
    uint32 current_block;
    do{
        current_block = ptr;
        read_block(dbh,block,ptr);
        memcpy(&ptr,block,BLOCK_PTR_SIZE);
        blocks_read++;

    }while( block_index != blocks_read );

    uint32 used_space = (pos % BLOCK_SIZE);
    uint32 free_space = BLOCK_SIZE - used_space;
    if(it.record_size <= free_space){
        memcpy(&block[used_space],block_field,it.record_size);
        write_block(dbh,block,current_block);
    }else{
        memcpy(&block[used_space],block_field,free_space);
        memcpy(&ptr,block,BLOCK_PTR_SIZE);
        write_block(dbh,block,current_block);
        read_block(dbh,block,ptr);
        uint32 difference = it.record_size - free_space;
        memcpy(&block[BLOCK_PTR_SIZE],&block_field[free_space],difference);
        write_block(dbh,block,ptr);
    }
}

string from_int_to_string(int number){
    std::stringstream ss;
    ss << number;
    std::string numberAsString(ss.str());
    return numberAsString;
}

string from_double_to_string(double number){
    std::stringstream ss;
    ss << number;
    std::string numberAsString(ss.str());
    return numberAsString;
}
void write_SB(struct Database_Handler dbh){
    string database_path = PATH+dbh.sb.name;
    database_path += ".dat";
    ofstream output_file(database_path.c_str(), ios::in | ios::out | ios::binary);
    if(!output_file){
        cout<<"Error while trying to write: "<<dbh.sb.name<<endl;
        return;
    }
    output_file.write(((char*)&dbh.sb),SB_SIZE);
    output_file.close();
}

void write_bitmap(string database_name,char * bitmap,uint32 size, uint64 ptr_bitmap){
    string database_path = PATH+database_name;
    database_path += ".dat";
    ofstream output_file(database_path.c_str(), ios::in | ios::out | ios::binary);
    if(!output_file){
        cout<<"Error while trying to write: "<<database_name<<endl;
        return;
    }
    output_file.seekp(ptr_bitmap,ios::beg);
    output_file.write(bitmap,size);
    output_file.close();
}

void erase_from_vector(vector<string>* vector_, int count){
    for(int i =0; i< count; i++){
        vector_->erase(vector_->begin());
    }
}

bool is_int_or_double(char currentSymbol){
    if( ((int)currentSymbol >= 48 && (int)currentSymbol <=57) || currentSymbol == '.' ){
        return true;
    }
    return false;
}

int get_type(string s){
    switch(s[0]){
        case 'I':
            return INT_T;
            break;
        case 'D':
            return DOUBLE_T;
            break;
        case 'C':
            return CHAR_T;
            break;
        default:
            printMsg("TYPE NOT FOUND");
            return -1;
    }
}

int get_type_len(vector<string> *entrance){
    if(!validateEntranceLen((*entrance),2)){
        return -1;
    }
    if ((*entrance)[1].find("INT") != std::string::npos){
        erase_from_vector(entrance,2);
        return 4;
    }else if((*entrance)[1].find("DOUBLE") != std::string::npos){
        erase_from_vector(entrance,2);
        return 8;
    }else if((*entrance)[1].find("CHAR") != std::string::npos){
        /*if((*entrance)[1].find("(") != std::string::npos){
            string s = "";
            uint32 count = 5;
            if((*entrance)[1].size() < count){
                printMsg("Error1: not size found in column "+(*entrance)[0]+" type char.");
                return -1;
            }
            string placeHolder = (*entrance)[1];
            while(true){
                if(placeHolder.size() < count || placeHolder[count] == ')')
                    break;
                s += placeHolder[count++];
            }
            if(s == ""){
                printMsg("Error2: NOT size found in column "+(*entrance)[0]+" type char.");
                return -1;
            }
            uint32 n = 0;
            from_String_to_uint(s,&n);
            if(n>4000){
                printMsg("Error: char size exceed 4000");
                return -1;
            }
            erase_from_vector(entrance,2);
            return n;
        }else{*/
            if(!validateEntranceLen((*entrance),3)){
                return -1;
            }
            if((*entrance)[2].find("(") != std::string::npos){
                string s = "";
                uint32 count = 1;
                if((*entrance)[2].size() < count){
                printMsg("Error3: not size found in column "+(*entrance)[0]+" type char.");
                return -1;
                }
                string placeHolder = (*entrance)[2];
                while(true){
                    if(placeHolder.size() < count || placeHolder[count] == ')')
                        break;
                    s += placeHolder[count++];
                }
                if(s == ""){
                    printMsg("Error4: not size found in column "+(*entrance)[0]+" type char.");
                    return -1;
                }
                uint32 n = 0;
                from_String_to_uint(s,&n);
                if(n>4000){
                    printMsg("Error: char size exceed 4000");
                    return -1;
                }
                erase_from_vector(entrance,3);
                return n;
            }else{
                printMsg("Error: Invalid format.");
                return -1;
            }
        //}
    }else{
        printMsg("Error: not a supported type.");
        return -1;
    }
}

bool validateEntranceLen(vector<string> entrance, unsigned int expected){
    if(entrance.size() < expected){
        printMsg("Not enough entrance detected");
        return false;
    }
    return true;
}

uint32 next_available(char* bitmap, uint32 b_size){
    for(uint32 i =0; i<b_size;i++){
        if(is_block_in_use(bitmap,i)==false){
            return i;
        }
    }
    throw std::invalid_argument("Error200: Not enough blocks available");
}

bool is_block_in_use(char* bitmap, uint32 blocknum)
{
    uint32 indexonbitmap = blocknum / 8;
    uint32 posinchar = blocknum%8;

     if(bitmap[indexonbitmap] & 1<<posinchar){
         return true;
     }else{
         return false;
     }
}

void setBlock_use(char* bitmap, uint32 blocknum){
    uint32 indexonbitmap = blocknum / 8;
    uint32 posinchar = blocknum%8;
    bitmap[indexonbitmap] = bitmap[indexonbitmap] | 1 << posinchar;
}

void setBlock_unuse(char* bitmap, uint32 blocknum){
    uint32 indexonbitmap = blocknum / 8;
    uint32 posinchar = blocknum%8;
    bitmap[indexonbitmap]= bitmap[indexonbitmap] & ~(1<<posinchar);
}
