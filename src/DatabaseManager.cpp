#include "DatabaseManager.h"
DatabaseManager::DatabaseManager()
{
    //ctor
    CreateDirectory ("MyDataBasesFolder", NULL);
    cout<<"char size "<<CHAR_BITS_SIZE<<endl;
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

    //UPDATING DATABASE HANDLER
    dbh.FD_size = output_file.tellp();

    //UPDATING AND WRITING BLOCKS BITMAP
    uint32 fs_blocks_used = ceil(((double)dbh.FD_size / (double)BLOCK_SIZE));
    dbh.sb.free_blocks_count = dbh.sb.blocks_count-fs_blocks_used;
    for(uint32 i=0; i< fs_blocks_used; i++)
        setBlock_use(dbh.blocks_bitmap,i);
    output_file.seekp(dbh.sb.ptr_blocks_bitmap,ios::beg);
    output_file.write(dbh.blocks_bitmap, dbh.blocks_bitmap_size);

    //WRITING SUPERBLOCK
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
    cout<<"DataBase metadata size: "<<dbh.FD_size<<endl;
    cout<<"itable bitmap pointer: "<<dbh.sb.ptr_itable_bipmap<<endl;
    cout<<"blocks bitmap pointer: "<<dbh.sb.ptr_blocks_bitmap<<endl;
}

void DatabaseManager::use_database(string name){
    name += ".dat";
    string database_path =  PATH + name;
    //output_file.open(path.c_str(), ios::in | ios::out | ios::binary);
    ifstream in(database_path.c_str(),ios::in | ios::out | ios::binary);
    if(!in){
        cout<<"Error while trying to open: "<<name<<endl;
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

    dbh.FD_size = in.tellg();
    in.close();
    cout<<next_available(dbh.blocks_bitmap,dbh.sb.blocks_count)<<endl;
    printMsg("Database loaded successfully");
    cout<<"database size: "<<dbh.sb.database_size<<endl;
    cout<<"blocks count: "<<dbh.sb.blocks_count<<endl;
    cout<<"free blocks count: "<<dbh.sb.free_blocks_count<<endl;
    cout<<"itables count: "<<dbh.sb.itables_count<<endl;
    cout<<"DataBase metadata size: "<<dbh.FD_size<<endl;
    cout<<"itable bitmap pointer: "<<dbh.sb.ptr_itable_bipmap<<endl;
    cout<<"blocks bitmap pointer: "<<dbh.sb.ptr_blocks_bitmap<<endl;
}
void DatabaseManager::drop_database(string name){
    name += ".dat";
    string database_path =  PATH + name;

    if(remove(database_path.c_str()) !=0){
        printMsg("Error: while trying to delete: "+name);
    }else
        printMsg("Drop database successfully");
}


DatabaseManager::~DatabaseManager(){
}
