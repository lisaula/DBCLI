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
    memset(&it.name,0,MAX_STRING_SIZE);
    it.first_block = -1;
    it.records_count = -1;
    it.fields_count = -1;
    it.table_size = -1;
    it.record_size = -1;
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
    output_file.seekp(0, ios::beg);
    output_file.write(((char*)&dbh.sb), SB_SIZE);

    //WRITING END OF FILE
    char block[BLOCK_SIZE];
    memset(block,0,BLOCK_SIZE);
    output_file.seekp(dbh.sb.database_size-BLOCK_SIZE, ios::beg);
    output_file.write(block, BLOCK_SIZE);
    output_file.close();
    printMsg("Create database successfully");
    cout<<"database size: "<<dbh.sb.database_size<<endl;
    cout<<"blocks count: "<<dbh.sb.blocks_count<<endl;
    cout<<"free blocks count: "<<dbh.sb.free_blocks_count<<endl;
    cout<<"itables count: "<<dbh.sb.itables_count<<endl;
    cout<<"DataBase metadata size: "<<dbh.sb.FD_size<<endl;
    cout<<"itable bitmap pointer: "<<dbh.sb.ptr_itable_bipmap<<endl;
    cout<<"blocks bitmap pointer: "<<dbh.sb.ptr_blocks_bitmap<<endl;
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
    cout<<"database size: "<<dbh.sb.database_size<<endl;
    cout<<"blocks count: "<<dbh.sb.blocks_count<<endl;
    cout<<"free blocks count: "<<dbh.sb.free_blocks_count<<endl;
    cout<<"itables count: "<<dbh.sb.itables_count<<endl;
    cout<<"DataBase metadata size: "<<dbh.sb.FD_size<<endl;
    cout<<"itable bitmap pointer: "<<dbh.sb.ptr_itable_bipmap<<endl;
    cout<<"blocks bitmap pointer: "<<dbh.sb.ptr_blocks_bitmap<<endl;
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

void DatabaseManager::create_table(vector<string>entrance){
    if(use == ""){
        printMsg("No database has been specified to use.");
        return;
    }
    if(entrance.size()< 3 || strlen(entrance[2].c_str())>MAX_STRING_SIZE){
        printMsg("Not enough parameters, or table name is too long.");
        return;
    }

    for(uint32 i =0; i<entrance.size();i++){
        cout<<entrance[i]<<endl;
    }

    if(dbh.sb.free_itables_count ==0 ){
        printMsg("Error: no free itable.");
        return;
    }

    uint32 n_itable = next_available(dbh.itable_bitmap, dbh.itable_bitmap_size);
    if(n_itable == (unsigned int)-1){
        printMsg("Not enough i_tables available");
        return;
    }
    setBlock_use(dbh.itable_bitmap,n_itable);
    struct i_table it;
    memset(it.name,0,MAX_STRING_SIZE);
    it.record_size =0;

    strcpy(it.name,entrance[2].c_str());
    erase_from_vector(&entrance, 3);

    vector<struct field> fields;
    while(entrance.size()>0){
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
        fields.push_back(f);
        it.record_size =f.size;
    }
    it.fields_count = fields.size();
    if(dbh.sb.free_blocks_count ==0 ){
        printMsg("Error: no free itable.");
        return;
    }
    uint32 n_block = next_available(dbh.blocks_bitmap,dbh.sb.blocks_count);
    if(n_block == (unsigned int)-1){
        printMsg("Not enough blocks available");
        return;
    }
    setBlock_use(dbh.blocks_bitmap,n_block);

    it.first_block = n_block;
    it.records_count =0;
    it.table_size =0;
    char block[BLOCK_SIZE];
    memset(block,0, BLOCK_SIZE);
    uint32 ptr_next_block = -1;
    memcpy(block,(void*)&ptr_next_block,BLOCK_PTR_SIZE);

    struct field *fields_array = &fields[0];
    memcpy((void*)&block[BLOCK_PTR_SIZE],(void*)fields_array,FIELD_SIZE*it.fields_count);

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


DatabaseManager::~DatabaseManager(){
}
