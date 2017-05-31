#include "DatabaseManager.h"
DatabaseManager::DatabaseManager()
{
    //ctor
    CreateDirectory ("MyDataBasesFolder", NULL);
    use="";
}
/*
    char name[24];
    uint64_t database_size;
    uint32_t blocks_count;
    uint32_t free_blocks_count;
    uint64_t ptr_blocks_bitmap;
    uint32_t itables_count;
    uint32_t free_itables_count;
    uint64_t ptr_itable_bipmap;
*/
void DatabaseManager::create_database(string name, UINT32 database_size){
    if(name.size() >MAX_STRING_SIZE){
        printMsg("Error: database name should not be larger than 20 characters");
        return;
    }
    if(database_size > MAXIMUN_DATABASE_SIZE_GB){
        cout<<"Database cannot exceed "<<MAXIMUN_DATABASE_SIZE_GB<<" GB"<<endl;
        return;
    }

    //MAKING SUPERBLOCK
    strcpy(dbh.sb.name,name.c_str());
    dbh.sb.database_size = from_MB_bytes_convertion(database_size, TO_BYTE);
    dbh.sb.blocks_count = dbh.sb.database_size / BLOCK_SIZE;
    dbh.sb.itables_count = (dbh.sb.database_size*PERCENTAGE_FOR_ITABLES) /ITABLE_SIZE;
    dbh.sb.free_itables_count = dbh.sb.itables_count;

    use = name;
    name += ".dat";
    string database_path =  PATH + name;
    ofstream output_file(database_path.c_str(),ios::binary);
    output_file.seekp(SB_SIZE, ios::beg);

    //WRITTING ITABLES
    /*
    char name[MAX_STRING_SIZE];
    uint32 first_block;
    uint32 records_count;
    uint16 fields_count;
    uint64 table_size;
    */

    struct i_table it;
    memset(it.name,0,MAX_STRING_SIZE);
    it.first_block = -1;
    it.records_count = -1;
    it.fields_count = -1;
    it.table_size = -1;
    it.record_size = -1;
    it.index = -1;
    for(uint32 i=0; i < dbh.sb.itables_count;i++){
        output_file.write(((char*)&it), ITABLE_SIZE);
    }
    //WRITTING ITABLE BITMAP
    dbh.sb.ptr_itable_bipmap = output_file.tellp();
    dbh.itable_bitmap_size = dbh.sb.itables_count/CHAR_BITS_SIZE;
    dbh.itable_bitmap = new char[dbh.itable_bitmap_size];
    memset((void*)dbh.itable_bitmap,0, dbh.itable_bitmap_size );
    output_file.write(dbh.itable_bitmap, dbh.itable_bitmap_size);

    //MAKING BLOCKS BITMAP
    dbh.sb.ptr_blocks_bitmap = output_file.tellp();
    dbh.blocks_bitmap_size = dbh.sb.blocks_count / CHAR_BITS_SIZE;
    dbh.blocks_bitmap = new char[dbh.blocks_bitmap_size];
    memset((void*)dbh.blocks_bitmap,0, dbh.blocks_bitmap_size);
    uint32 actual_pos = output_file.tellp();
    output_file.seekp(actual_pos+dbh.blocks_bitmap_size, ios::beg);

    uint32 FD_size = output_file.tellp();

    //UPDATING AND WRITING BLOCKS BITMAP
    cout<<((double)FD_size / (double)BLOCK_SIZE)<<endl;
    uint32 fs_blocks_used = ceil(((double)FD_size / (double)BLOCK_SIZE));
    cout<<ceil(((double)FD_size / (double)BLOCK_SIZE))<<endl;
    dbh.sb.free_blocks_count = dbh.sb.blocks_count-fs_blocks_used;
    for(uint32 i=0; i< fs_blocks_used; i++)
        setBlock_use(dbh.blocks_bitmap,i);
    output_file.seekp(dbh.sb.ptr_blocks_bitmap,ios::beg);
    output_file.write(dbh.blocks_bitmap, dbh.blocks_bitmap_size);

    //WRITING SUPERBLOCK
    dbh.sb.FD_size = fs_blocks_used * BLOCK_SIZE;
    dbh.sb.free_database_space = dbh.sb.database_size - dbh.sb.FD_size;
    output_file.seekp(0, ios::beg);
    output_file.write(((char*)&dbh.sb), SB_SIZE);

    //WRITING END OF FILE
    char block[BLOCK_SIZE];
    memset(block,0,BLOCK_SIZE);
    output_file.seekp(dbh.sb.database_size-BLOCK_SIZE, ios::beg);
    output_file.write(block, BLOCK_SIZE);
    output_file.close();
    printMsg("Create database successfully");
    //cout<<next_available(dbh.itable_bitmap,dbh.sb.itables_count)<<endl;
    //cout<<next_available(dbh.blocks_bitmap,dbh.sb.blocks_count)<<endl;
    print_database_info();
}

void DatabaseManager::print_database_info(){
    if(use == ""){
        printMsg("No database has been specified to use.");
        return;
    }
    cout<<"database size: "<<dbh.sb.database_size<<endl;
    cout<<"free database space: "<<dbh.sb.free_database_space<<endl;
    cout<<"database metadata size: "<<dbh.sb.FD_size<<endl;
    cout<<"blocks count: "<<dbh.sb.blocks_count<<endl;
    cout<<"free blocks count: "<<dbh.sb.free_blocks_count<<endl;
    cout<<"itables count: "<<dbh.sb.itables_count<<endl;
    cout<<"free itables count: "<<dbh.sb.free_itables_count<<endl;
    cout<<"itable bitmap pointer: "<<dbh.sb.ptr_itable_bipmap<<endl;
    cout<<"blocks bitmap pointer: "<<dbh.sb.ptr_blocks_bitmap<<endl;
    cout<<"next free block: "<<next_available(dbh.blocks_bitmap,dbh.sb.blocks_count)<<endl;
    cout<<"next free itable: "<<next_available(dbh.itable_bitmap,dbh.sb.itables_count)<<endl;
}

void DatabaseManager::use_database(string name){
    use = name;
    name += ".dat";
    string database_path =  PATH + name;
    //output_file.open(path.c_str(), ios::in | ios::out | ios::binary);
    ifstream in(database_path.c_str(),ios::in | ios::out | ios::binary);
    if(!in){
        cout<<"Error while trying to open: "<<name<<endl;
        use="";
        return;
    }

    in.read(((char*)&dbh.sb),SB_SIZE);
    /*
    struct SB sb;
    uint32 itable_bitmap_size;
    uint32 blocks_bitmap_size;
    char* itable_bitmap;
    char* blocks_bitmap;
    uint32 FD_size;
    */

    //MAKING DATABASE HANDLER
    dbh.itable_bitmap_size = dbh.sb.itables_count/CHAR_BITS_SIZE;
    dbh.blocks_bitmap_size = dbh.sb.blocks_count/CHAR_BITS_SIZE;

    delete(dbh.itable_bitmap);
    dbh.itable_bitmap = new char[dbh.itable_bitmap_size];
    in.seekg(dbh.sb.ptr_itable_bipmap, ios::beg);
    in.read(dbh.itable_bitmap,dbh.itable_bitmap_size);

    delete(dbh.blocks_bitmap);
    dbh.blocks_bitmap = new char[dbh.blocks_bitmap_size];
    in.seekg(dbh.sb.ptr_blocks_bitmap, ios::beg);
    in.read(dbh.blocks_bitmap,dbh.blocks_bitmap_size);

    in.close();
    printMsg("Database loaded successfully");
    //cout<<next_available(dbh.itable_bitmap,dbh.sb.itables_count)<<endl;
    //cout<<next_available(dbh.blocks_bitmap,dbh.sb.blocks_count)<<endl;
    print_database_info();
}
void DatabaseManager::drop_database(string name){
    name += ".dat";
    string database_path =  PATH + name;

    if(remove(database_path.c_str()) !=0){
        printMsg("Error: while trying to delete: "+name);
    }else{
        use ="";
        printMsg("Drop database successfully");
    }
}
void DatabaseManager::insert_command(vector<string> entrance){
    if(use == ""){
        printMsg("No database has been specified to use.");
        return;
    }
    string table_name = entrance[2];
    erase_from_vector(&entrance,3);
    vector<pair<string,string> > fields_value;
    while(entrance.size()>0){
        if(!validateEntranceLen(entrance,3)){
            printMsg("Expected: <COLUMN_NAME> = <VALUE>");
            return;
        }
        pair<string,string> p(entrance[0], entrance[2]);
        fields_value.push_back(p);
        erase_from_vector(&entrance,3);
    }
    struct i_table it;
    if(!find_i_table(dbh,table_name, &it)){
        printMsg("Couldn't find table "+table_name);
        return;
    }

    char block[BLOCK_SIZE];
    read_block(dbh,block,it.first_block);
    vector<struct field> *fields = get_fields(block,it);
    if(fields == NULL)
        throw std::invalid_argument("Error while reading fields in table "+table_name);

    char block_field[it.record_size];
    memset(block_field,0,it.record_size);
    validate_fields(fields_value,(*fields),block_field);

    uint32 used_space = (it.table_size % BLOCK_SIZE);
    uint32 free_space = BLOCK_SIZE - used_space;
    if(it.record_size <= free_space){
        char new_block[BLOCK_SIZE];
        //memset(new_block,0,BLOCK_SIZE);
        read_block(dbh,new_block,it.last_block);
        //cout<<"guardo pos: "<<used_space<<" en: "<<it.last_block<<" fs: "<<free_space<<endl;
        memcpy(&new_block[used_space],block_field,it.record_size);
        write_block(dbh,new_block,it.last_block);
    }else{
        char new_block[BLOCK_SIZE];
        //memset(new_block,0,BLOCK_SIZE);
        read_block(dbh,new_block,it.last_block);
        memcpy(&new_block[used_space],block_field,free_space);
        uint32 n_block = next_available(dbh.blocks_bitmap,dbh.sb.blocks_count);
        memcpy(&new_block[0],(void*)&n_block,BLOCK_PTR_SIZE);
        write_block(dbh,new_block,it.last_block);
        it.last_block = n_block;
        memset(new_block,0,BLOCK_SIZE);
        n_block = -1;
        memcpy(&new_block[0],(void*)&n_block,BLOCK_PTR_SIZE);
        uint32 difference = it.record_size - free_space;
        memcpy(&new_block[BLOCK_PTR_SIZE],&block_field[free_space],difference);
        write_block(dbh,new_block,it.last_block);
        setBlock_use(dbh.blocks_bitmap,it.last_block);
        write_bitmap(use,dbh.blocks_bitmap,dbh.blocks_bitmap_size,dbh.sb.ptr_blocks_bitmap);
        if(dbh.sb.free_blocks_count>0)
            dbh.sb.free_blocks_count -=1;
    }
    it.table_size += it.record_size;
    it.records_count++;
    dbh.sb.free_database_space -= it.record_size;
    write_SB(dbh);
    //cout<<"index itable: "<<it.index<<endl;
    //cout<<"firsts itable: "<<it.first_block<<endl;
    //cout<<"last itable: "<<it.last_block<<endl;
    write_itable(dbh,it,it.index);
    delete(fields);
    printMsg("Insert command executed successfully");
}

void DatabaseManager::validate_fields(vector<pair<string,string> > fields_value,vector<struct field> fields, char* block){
    vector<struct field> found ;
    vector<pair<string,string> > not_founds;
    while(fields_value.size()> 0){
        uint32 i=0;bool f1 = false;
        for(uint32 x=0; x<fields.size();x++){
            //cout<<"comparando |"<<fields_value[i].first<<"|"<<fields[x].name<<"|"<<endl;
            if(strcmp(fields_value[i].first.c_str(), fields[x].name)==0){
                if(fields[x].type == CHAR_T){
                    string char_value =validate_char_value(fields_value[i].second);
                    if(char_value.size() > fields[x].size){
                        throw std::invalid_argument("Error char value \""+char_value+"\" exceed its size");
                    }
                    char new_char[fields[x].size];
                    memset(new_char,0,fields[x].size);
                    memcpy(new_char,char_value.c_str(), char_value.size());
                    //cout<<"pading "<<get_field_padding(fields[x], fields)<<" val: "<<char_value.c_str()<<endl;
                    memcpy(&block[get_field_padding(fields[x], fields)],new_char,fields[x].size);
                }else if(fields[x].type == INT_T){
                    int n = validate_int_value(fields_value[i].second);
                    memcpy(&block[get_field_padding(fields[x], fields)],(void*)&n,fields[x].size);
                }else if(fields[x].type == DOUBLE_T){
                    double d = validate_double_value(fields_value[i].second);
                    memcpy(&block[get_field_padding(fields[x],fields)],(void*)&d,fields[x].size);
                }
                /*cout<<"init"<<endl;
                for(uint32 i =0; i<fields.size();i++){
                    cout<<fields[i].name<<endl;
                }
                fields.erase(fields.begin()+x);
                cout<<"after"<<endl;
                for(uint32 i =0; i<fields.size();i++){
                    cout<<fields[i].name<<endl;
                }
                cout<<"middle"<<endl;
                for(uint32 i =0; i<fields_value.size();i++){
                    cout<<fields_value[i].first<<endl;
                }*/
                /*cout<<"after2"<<endl;
                for(uint32 i =0; i<fields_value.size();i++){
                    cout<<fields_value[i].first<<endl;
                }*/
                found.push_back(fields[x]);
                f1 = true;
                break;
                //x=0;i=0;
            }
            //cout<<"itero "<<x<<" "<<fields.size()<<endl;
        }
        if(!f1)
            not_founds.push_back(fields_value[i]);
        fields_value.erase(fields_value.begin()+i);
        //cout<<"itero out "<<i<<" "<<fields_value.size()<<endl;
    }
    if(not_founds.size()>0){
        for(uint32 i =0; i<not_founds.size();i++){
            //cout<<not_founds.size()<<endl;
            throw std::invalid_argument("Error100: could not find column "+not_founds[i].first);
        }
    }
    bool f = false;
    if(found.size() < fields.size()){
        for(uint32 i =0; i<fields.size();i++){
             f=false;
            for(uint32 x = 0; x < found.size(); x++){
                if(fields[i].index == found[x].index){
                    f = true;
                }
            }
            if(!f){
                string s = "";
                s += fields[i].name;
                throw std::invalid_argument("Error1001: column "+s+" should not be null");
            }
        }
    }
}
int DatabaseManager::validate_int_value(string value){
    for(uint32 i = 0; i< value.size();i++){
        if(!is_int_or_double(value[i]))
            throw std::invalid_argument("Error: not a int value: "+value);
    }
    int n = 0;
    from_String_to_int(value,&n);
    return n;
}

double DatabaseManager::validate_double_value(string value){
    for(uint32 i = 0; i< value.size();i++){
        if(!is_int_or_double(value[i]))
            throw std::invalid_argument("Error: not a double value: "+value);
    }
    double d=0;
    from_String_to_double(value,&d);
    return d;
}
string DatabaseManager::validate_char_value(string value){
    int count_double_quotes=0;
    for(uint32 i =0; i< value.size();i++){
        if(value[i] == '"')
            count_double_quotes++;
    }
    if(count_double_quotes<2)
        throw std::invalid_argument("Error: not a char value: "+value);

    string char_value = trim(value,'"');
    return char_value;
}
void DatabaseManager::create_table(vector<string>entrance){
    if(use == ""){
        printMsg("No database has been specified to use.");
        return;
    }
    if(entrance.size()< 3 || strlen(entrance[2].c_str())>MAX_STRING_SIZE){
        printMsg("Not enough parameters, or table name is too long.");
        return;
    }

    if(dbh.sb.free_itables_count ==0 ){
        printMsg("Error: no free itable.");
        return;
    }

    uint32 n_itable = next_available(dbh.itable_bitmap, dbh.sb.itables_count);
    //cout<<"next itable available :"<<n_itable<<endl;
    if(n_itable == (unsigned int)-1){
        printMsg("Not enough i_tables available");
        return;
    }
    setBlock_use(dbh.itable_bitmap,n_itable);
    struct i_table it;
    memset(it.name,0,MAX_STRING_SIZE);
    it.index = n_itable;
    it.records_count =0;
    it.record_size =0;
    strcpy(it.name,entrance[2].c_str());
    erase_from_vector(&entrance, 3);

    vector<struct field> fields;
    uint32 index = 0;
    while(entrance.size()>0){
        if(white_spaces(&entrance))
            continue;

        struct field f;
        if(strlen(entrance[0].c_str())>MAX_STRING_SIZE){
            printMsg("Name of column "+entrance[0]+" is too long");
            return;
        }
        memset(f.name,0,MAX_STRING_SIZE);
        strcpy(f.name,entrance[0].c_str());
        if(!validateEntranceLen(entrance,2)){
            return;
        }
        int type = get_type(entrance[1]);
        if(type <0)
            return;
        f.type = type;
        f.size = get_type_len(&entrance);
        if(f.size<0)
            return;
        f.index = index;
        fields.push_back(f);
        index++;
        //cout<<"size "<<f.size<<endl;
        it.record_size +=f.size;
    }
    it.record_size+=1;//byte de borrado;
    it.fields_count = fields.size();
    if(dbh.sb.free_blocks_count ==0 ){
        printMsg("Error: no free itable.");
        return;
    }
    uint32 n_block = next_available(dbh.blocks_bitmap,dbh.sb.blocks_count);
    //cout<<"next block available :"<<n_block<<endl;
    if(n_block == (unsigned int)-1){
        printMsg("Not enough blocks available");
        return;
    }
    setBlock_use(dbh.blocks_bitmap,n_block);

    it.first_block = n_block;
    it.last_block = n_block;
    it.records_count =0;
    it.table_size =(it.fields_count*FIELD_SIZE) +BLOCK_PTR_SIZE;

    //updating database free space
    dbh.sb.free_database_space -= it.table_size;

    char block[BLOCK_SIZE];
    memset(block,0, BLOCK_SIZE);
    uint32 ptr_next_block = -1;
    memcpy(block,(void*)&ptr_next_block,BLOCK_PTR_SIZE);

    struct field *fields_array = &fields[0];
    memcpy(&block[BLOCK_PTR_SIZE],(void*)fields_array,FIELD_SIZE*it.fields_count);

    /*for(uint32 i = 0; i< fields.size(); i++){
        cout<<fields_array[i].name<<endl;
        cout<<fields_array[i].type<<endl;
        cout<<fields_array[i].size<<endl;
    }*/
    //WRITING PROCESS

    write_block(dbh,block,n_block);
    write_itable(dbh,it,n_itable);
    if(dbh.sb.free_blocks_count>0)
        dbh.sb.free_blocks_count--;
    if(dbh.sb.free_itables_count>0)
        dbh.sb.free_itables_count--;

    write_SB(dbh);
    write_bitmap(use,dbh.blocks_bitmap,dbh.blocks_bitmap_size,dbh.sb.ptr_blocks_bitmap);
    write_bitmap(use,dbh.itable_bitmap,dbh.itable_bitmap_size,dbh.sb.ptr_itable_bipmap);
    printMsg("Create table successfully");
}

void DatabaseManager::print_table_info(string table_name){
    if(use == ""){
        printMsg("No database has been specified to use.");
        return;
    }
    struct i_table it;
    if(!find_i_table(dbh,table_name, &it)){
        printMsg("Couldn't find table "+table_name);
        return;
    }

    char block[BLOCK_SIZE];
    read_block(dbh,block,it.first_block);
    vector<struct field> fields;
    int fields_count =0;
    while (fields_count < it.fields_count){
        struct field f;
        uint32 pos = BLOCK_PTR_SIZE+(fields_count*FIELD_SIZE);
        if(pos < BLOCK_SIZE){
            memcpy((char*)&f,&block[pos],FIELD_SIZE);
            fields.push_back(f);
            fields_count++;
        }else{
            printMsg("block finished");
            break;
        }
    }
    cout<<"Name: "<<it.name<<endl;
    cout<<"Index: "<<it.index<<endl;
    cout<<"Records storaged: "<<it.records_count<<endl;
    cout<<"Records size: "<<it.record_size<<endl;
    cout<<"Table bytes storaged: "<<it.table_size<<endl;
    cout<<"Fields: "<<endl;
    cout<<"\tcount: "<<it.fields_count<<endl;

    for(uint32 i = 0; i< fields.size(); i++){
        cout<<"\tName: "<<fields[i].name<<endl;
        cout<<"\tType: "<<fields[i].type<<endl;
        cout<<"\tSize: "<<fields[i].size<<endl;
    }
}

bool DatabaseManager::get_where_statement(vector<string> entrance, pair<int, string>* p, vector<struct field> fields){
    if(entrance.size() == 0)
        return false;
    erase_from_vector(&entrance,1);
    if(!validateEntranceLen(entrance,3)){
        throw std::invalid_argument("Error300: In where statement. Expected <COLUMN_NAME> = <VALUE>");
    }
    int n = p_field_is_contained(entrance[0],fields);
    if(n<0){
        throw std::invalid_argument("Error300: In where statement. Column "+entrance[0]+" not found.");
    }
    p->first = n;
    p->second = entrance[2];
    return true;
}
void DatabaseManager::select_command(vector<string> entrance){
    if(use == ""){
        printMsg("No database has been specified to use.");
        return;
    }
    uint32 count = 1;
    vector<string> p_fields;
    while(entrance.size() > count && entrance[count] != "FROM"){
        if(white_spaces(&entrance))
            continue;
        p_fields.push_back(entrance[count++]);
    }
    erase_from_vector(&entrance,count);
    if(!validateEntranceLen(entrance,2))
        return;
    string table_name = entrance[1];
    erase_from_vector(&entrance,2);

    struct i_table it;
    if(!find_i_table(dbh,table_name, &it)){
        printMsg("Couldn't find table "+table_name);
        return;
    }

    /*vector<char*> *blocks = read_all_table(dbh,it);
    char ** ptr_blocks = &(*blocks)[0];
    char *block = ptr_blocks[0];*/
    char block[BLOCK_SIZE];
    read_block(dbh,block,it.first_block);
    vector<struct field> fields;
    int fields_count =0;
    while (fields_count < it.fields_count){
        struct field f;
        uint32 pos = BLOCK_PTR_SIZE+(fields_count*FIELD_SIZE);
        if(pos < BLOCK_SIZE){
            memcpy((char*)&f,&block[pos],FIELD_SIZE);
            fields.push_back(f);
            fields_count++;
        }else{
            printMsg("block finished");
            break;
        }
    }

    //GETTING WHERE
    pair<int,string> p;
    bool is_where = get_where_statement(entrance, &p, fields);

    vector<int> pad;
    print_table_header(&p_fields,fields,&pad);
    uint32 first_pos = (it.table_size - (it.records_count* it.record_size ));
    uint32 records_count =0;
    while(records_count < it.records_count){
        char block_field[it.record_size];
        uint32 space_available = BLOCK_SIZE-first_pos;
        if(it.record_size <= space_available){
            memcpy(block_field,&block[first_pos],it.record_size);
            records_count++;
            first_pos += it.record_size;
        }else{
            memcpy(block_field,&block[first_pos],space_available);
            uint32 ptr_next_block=0;
            memcpy(&ptr_next_block,block,BLOCK_PTR_SIZE);
            read_block(dbh,block,ptr_next_block);
            first_pos = BLOCK_PTR_SIZE;
            uint32 difference = it.record_size - space_available;
            memcpy(&block_field[space_available],&block[first_pos],difference);
            records_count++;
            first_pos += difference;
        }
        byte b = block_field[0];
        if(b ==1 )
            continue;
        if(is_where){
            bool pass = pass_where(block_field,p,fields);
            if(!pass)
                continue;
        }

        string s = "   ";
        for(int i = 0; i< p_fields.size(); i++){
            int pos = p_field_is_contained(p_fields[i],fields);
            struct field f = fields[pos];
            //cout<<"pading "<<get_field_padding(f,fields)<<" of "<<p_fields[i]<<endl;
            char *value = &block_field[get_field_padding(f,fields)];
            s = print_on_column(s,value,fields[pos].size,pad[i],(Type)fields[pos].type,(i == p_fields.size()-1));
            //(void*)value,fields[i].size,pad[i],fields[i].type,(i==fields.size-1
        }
        cout<<s;
    }
}

void DatabaseManager::delete_command(vector<string> entrance){
    if(use == ""){
        printMsg("No database has been specified to use.");
        return;
    }
    if(!validateEntranceLen(entrance,2))
        return;
    string table_name = entrance[1];
    erase_from_vector(&entrance,2);

    struct i_table it;
    if(!find_i_table(dbh,table_name, &it)){
        printMsg("Couldn't find table "+table_name);
        return;
    }

    char block[BLOCK_SIZE];
    read_block(dbh,block,it.first_block);
    vector<struct field> fields;
    int fields_count =0;
    while (fields_count < it.fields_count){
        struct field f;
        uint32 pos = BLOCK_PTR_SIZE+(fields_count*FIELD_SIZE);
        if(pos < BLOCK_SIZE){
            memcpy((char*)&f,&block[pos],FIELD_SIZE);
            fields.push_back(f);
            fields_count++;
        }else{
            printMsg("block finished");
            break;
        }
    }

    //GETTING WHERE
    pair<int,string> p;
    bool is_where = get_where_statement(entrance, &p, fields);

    uint32 first_pos = (it.table_size - (it.records_count* it.record_size ));
    uint32 records_count =0;
    while(records_count < it.records_count){
        char block_field[it.record_size];
        uint32 space_available = BLOCK_SIZE-first_pos;
        if(it.record_size <= space_available){
            memcpy(block_field,&block[first_pos],it.record_size);
            records_count++;
            first_pos += it.record_size;
        }else{
            memcpy(block_field,&block[first_pos],space_available);
            uint32 ptr_next_block=0;
            memcpy(&ptr_next_block,block,BLOCK_PTR_SIZE);
            read_block(dbh,block,ptr_next_block);
            first_pos = BLOCK_PTR_SIZE;
            uint32 difference = it.record_size - space_available;
            memcpy(&block_field[space_available],&block[first_pos],difference);
            records_count++;
            first_pos += difference;
        }
        byte b = block_field[0];
        if(b ==1 )
            continue;
        if(is_where){
            bool pass = pass_where(block_field,p,fields);
            if(!pass)
                continue;
            memset(block_field,1,1);
        }else{
            memset(block_field,1,1);
        }

        write_record(dbh,it, block_field,records_count-1);
    }
    printMsg("Deleted successfully");
}

void DatabaseManager::drop_table(string table_name){
    if(use == ""){
        printMsg("No database has been specified to use.");
        return;
    }
    struct i_table it;
    if(!find_i_table(dbh,table_name, &it)){
        printMsg("Couldn't find table "+table_name);
        return;
    }

    vector<uint32> * blocks = get_all_tables_used_blocks(dbh,it);
    for(uint32 i=0; i<blocks->size(); i++){
        setBlock_unuse(dbh.blocks_bitmap,(*blocks)[i]);
    }

    setBlock_unuse(dbh.itable_bitmap,it.index);
    dbh.sb.free_blocks_count+= blocks->size();
    dbh.sb.free_database_space += it.table_size;
    dbh.sb.free_itables_count+=1;

    //writing process
    write_bitmap(use,dbh.blocks_bitmap,dbh.blocks_bitmap_size,dbh.sb.ptr_blocks_bitmap);
    write_bitmap(use,dbh.itable_bitmap,dbh.itable_bitmap_size,dbh.sb.ptr_itable_bipmap);
    write_SB(dbh);
}

void DatabaseManager::update_command(vector<string> entrance){
    if(use == ""){
        printMsg("No database has been specified to use.");
        return;
    }
    if(!validateEntranceLen(entrance,2))
        return;
    string table_name = entrance[1];
    erase_from_vector(&entrance,2);

    /*--------------------*/
    vector<pair<string,string> > fields_value;
    while(entrance.size()>0 && entrance[0]!= "WHERE"){
        if(!validateEntranceLen(entrance,3)){
            printMsg("Expected: <COLUMN_NAME> = <VALUE>");
            return;
        }
        pair<string,string> p(entrance[0], entrance[2]);
        fields_value.push_back(p);
        erase_from_vector(&entrance,3);
    }

    struct i_table it;
    if(!find_i_table(dbh,table_name, &it)){
        printMsg("Couldn't find table "+table_name);
        return;
    }

    char block[BLOCK_SIZE];
    read_block(dbh,block,it.first_block);
    vector<struct field> fields;
    int fields_count =0;
    while (fields_count < it.fields_count){
        struct field f;
        uint32 pos = BLOCK_PTR_SIZE+(fields_count*FIELD_SIZE);
        if(pos < BLOCK_SIZE){
            memcpy((char*)&f,&block[pos],FIELD_SIZE);
            fields.push_back(f);
            fields_count++;
        }else{
            printMsg("block finished");
            break;
        }
    }

    //GETTING WHERE
    pair<int,string> p;
    bool is_where = get_where_statement(entrance, &p, fields);
    if(!is_where)
        throw std::invalid_argument("Error: update without where.");


    vector<int> pad;
    vector<string> p_fields;
    for(int i =0; i < fields_value.size();i++){
        p_fields.push_back(fields_value[i].first);
    }
    //print_table_header(&p_fields,fields,&pad);

    uint32 first_pos = (it.table_size - (it.records_count* it.record_size ));
    uint32 records_count =0;
    while(records_count < it.records_count){
        char block_field[it.record_size];
        uint32 space_available = BLOCK_SIZE-first_pos;
        if(it.record_size <= space_available){
            memcpy(block_field,&block[first_pos],it.record_size);
            records_count++;
            first_pos += it.record_size;
        }else{
            memcpy(block_field,&block[first_pos],space_available);
            uint32 ptr_next_block=0;
            memcpy(&ptr_next_block,block,BLOCK_PTR_SIZE);
            read_block(dbh,block,ptr_next_block);
            first_pos = BLOCK_PTR_SIZE;
            uint32 difference = it.record_size - space_available;
            memcpy(&block_field[space_available],&block[first_pos],difference);
            records_count++;
            first_pos += difference;
        }
        byte b = block_field[0];
        if(b ==1 )
            continue;
        if(is_where){
            bool pass = pass_where(block_field,p,fields);
            if(!pass)
                continue;
        }
        struct field f = fields[p.first];
        alter_block(fields_value,block_field,f,fields);

        write_record(dbh,it, block_field,records_count-1);


        /*string s = "   ";
        for(int i = 0; i< p_fields.size(); i++){
            int pos = p_field_is_contained(p_fields[i],fields);
            struct field f = fields[pos];
            //cout<<"pading "<<get_field_padding(f,fields)<<" of "<<p_fields[i]<<endl;
            char *value = &block_field[get_field_padding(f,fields)];
            s = print_on_column(s,value,fields[pos].size,pad[i],(Type)fields[pos].type,(i == p_fields.size()-1));
            //(void*)value,fields[i].size,pad[i],fields[i].type,(i==fields.size-1
        }
        cout<<s;*/
    }
    printMsg("Updated successfully");
}

void DatabaseManager::alter_block(vector<pair<string,string> > fields_value,char *block_field,struct field f,vector<struct field> fields){
    vector<struct field> new_fields;
    for(uint32 i = 0; i < fields.size(); i++){
        for(uint32 x=0; x< fields_value.size(); x++){
            if(strcmp( fields_value[x].first.c_str(),fields[i].name ) ==0){
                new_fields.push_back(fields[i]);
            }
        }
    }

    validate_fields(fields_value,new_fields,block_field);
}

bool DatabaseManager::pass_where(char* block_field, pair<int, string> p, vector<struct field> fields){
    struct field f = fields[p.first];

    bool return_value =false;
    switch (f.type){
        case INT_T:{
            int n = validate_int_value(p.second);
            int n_v =0;
            memcpy(&n_v,&block_field[get_field_padding(f,fields)],f.size);
            return_value =  n_v == n;
            break;
        }
        case CHAR_T:{
            string s = validate_char_value(p.second);
            char value[f.size+1];
            memcpy(value,&block_field[get_field_padding(f,fields)],f.size);
            value[f.size]= (char)NULL;
            return_value = (strcmp( value,s.c_str() ) == 0);
            break;
        }
        case DOUBLE_T:{
            double d = validate_double_value(p.second);
            double d_v;
            memcpy(&d_v,&block_field[get_field_padding(f,fields)],f.size);
            return_value = d_v == d;
            break;
        }
        }
    return return_value;
}

void DatabaseManager::print_table_header(vector<string> *p_fields, vector<struct field> fields, vector<int>* pad){
    bool all = false;
    for(uint32 i = 0; i < p_fields->size() ; i++){
        if((*p_fields)[i].find("*") != std::string::npos){
            all = true;
            break;
        }
    }
    if(all){
        erase_from_vector(p_fields,p_fields->size());
        cout<<"   ";
        int count_pad = 3, l_p_c =0;
        for(uint32 i =0; i< fields.size();i++){
            string place_holder = "";
            place_holder += fields[i].name;
            p_fields->push_back(place_holder);
            cout<<fields[i].name<<"          ";
            int p_c = 0;
            for(uint32 p = strlen(fields[i].name)+10; p<fields[i].size;p++ ){
                cout<<" ";
                p_c++;
            }
            if(i!=0){
                //cout<<"lpc "<<l_p_c<<endl;
                count_pad += strlen(fields[i-1].name)+(10+l_p_c);
            }
            l_p_c = p_c;
            pad->push_back(count_pad);
        }
        cout<<endl;
    }else{
        cout<<"   ";
        int count_pad = 3, l_p_c =0;
        for(uint32 i =0; i< p_fields->size();i++){
            int p_i = p_field_is_contained((*p_fields)[i], fields);
            if( p_i < 0)
                continue;
            cout<<fields[p_i].name<<"          ";
            int p_c = 0;
            for(uint32 p = strlen(fields[p_i].name)+10; p<fields[p_i].size;p++ ){
                cout<<" ";
                p_c++;
            }
            if(i!=0){
                //cout<<"lpc "<<l_p_c<<endl;
                count_pad += strlen((*p_fields)[i-1].c_str())+(10+l_p_c);
            }
            l_p_c = p_c;
            pad->push_back(count_pad);
        }
        cout<<endl;
    }
}

string DatabaseManager::print_on_column(string s,void* value,uint32 size, int pad, Type t, bool end_line){
    switch(t){
        case INT_T:
                {
                int *n = (int*)value;
                while(s.size() < pad ){
                    s+=" ";
                }
                s += from_int_to_string(*n);
                break;
            }
        case CHAR_T:{
            char v [size+1];
            memcpy(v,value,size);
            v[size] = (char)NULL;
            while(s.size() < pad ){
                    s+=" ";
            }
            s += v;
            break;
        }
        case DOUBLE_T:{
            double *n = (double*)value;
            while(s.size() < pad ){
                s+=" ";
            }
            s += from_double_to_string(*n);;
            break;
        }
    }
    if(end_line)
        s+="\n";
    return s;
}


DatabaseManager::~DatabaseManager(){
}
