#ifndef UTILS_H
#define UTILS_H

#define BLOCK_SIZE 1024
#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <sstream>
#include <math.h>

#define uint32 unsigned int
#define uint64 unsigned long int
#define uint16 unsigned short int
#define MAXIMUN_DATABASE_SIZE_GB 4095
#define TO_BYTE 1
#define SB_SIZE sizeof(struct SB)
#define ITABLE_SIZE sizeof(struct i_table)
#define CHAR_BITS_SIZE (sizeof(char)*8)

#define PERCENTAGE_FOR_ITABLES 0.05
#define MAX_STRING_SIZE 24

using namespace std;

const string PATH = "MyDataBasesFolder\\";
struct SB{
    char name[MAX_STRING_SIZE];
    uint64 database_size;
    uint32 blocks_count;
    uint32 free_blocks_count;
    uint64 ptr_blocks_bitmap;
    uint32 itables_count;
    uint32 free_itables_count;
    uint64 ptr_itable_bipmap;
};

struct Database_Handler{
    struct SB sb;
    uint32 itable_bitmap_size;
    uint32 blocks_bitmap_size;
    char* itable_bitmap;
    char* blocks_bitmap;
    uint32 FD_size;
};

struct i_table{
    char name[MAX_STRING_SIZE];
    uint32 first_block;
    uint32 records_count;
    uint16 fields_count;
    uint64 table_size;
};

uint64 from_GB_bytes_convertion(uint32 n, int type);
uint64 from_MB_bytes_convertion(uint32 n, int type);
void printMsg(string msg);
void from_String_to_uint(string s, uint32 *n);
void setBlock_unuse(char* bitmap, uint32 blocknum);
void setBlock_use(char* bitmap, uint32 blocknum);
bool is_block_in_use(char* bitmap, uint32 blocknum);
uint32 next_available(char* bitmap, uint32 b_size);

#endif // UTILS_H
