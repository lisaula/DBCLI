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


void from_String_to_uint(string s, uint32 *n){
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
        if((*entrance)[1].find("(") != std::string::npos){
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
        }else{
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
        }
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
    return -1;
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
