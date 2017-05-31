#ifndef UTILS_H
#define UTILS_H

#define BLOCK_SIZE 1024
#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <sstream>
#include <math.h>
#include <vector>
#include "TYPE.h"
#include <stdexcept>
#include <cstddef>

#define uint32 unsigned int
#define uint64 unsigned long int
#define uint16 unsigned short int
#define MAXIMUN_DATABASE_SIZE_GB 4095
#define TO_BYTE 1
#define SB_SIZE sizeof(struct SB)
#define ITABLE_SIZE sizeof(struct i_table)
#define CHAR_BITS_SIZE (sizeof(char)*8)
#define FIELD_SIZE sizeof(struct field)
#define BLOCK_PTR_SIZE sizeof(uint32)

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
    uint64 FD_size;
    uint64 free_database_space;
};

struct Database_Handler{
    struct SB sb;
    uint32 itable_bitmap_size;
    uint32 blocks_bitmap_size;
    char* itable_bitmap;
    char* blocks_bitmap;
};

struct i_table{
    uint32 index;
    char name[MAX_STRING_SIZE];
    uint32 first_block;
    uint32 last_block;
    uint32 records_count;
    uint32 record_size;
    uint16 fields_count;
    uint64 table_size;
};
struct field{
    uint32 index;
    char name[MAX_STRING_SIZE];
    int type;
    int size;
};

uint32 get_field_padding(struct field f, vector<struct field> fields);
bool is_int_or_double(char currentSymbol);
string trim(const string& str, char trim_value);
vector<struct field>* get_fields(char *block, struct i_table it);
void from_String_to_int(string s, int *n);
void from_String_to_double(string s, double *n);
bool find_i_table(struct Database_Handler dbh,string name, struct i_table *it);
bool white_spaces(vector<string> *entrance);
bool validateEntranceLen(vector<string> entrance, unsigned int expected);
void write_SB(struct Database_Handler dbh);
void write_bitmap(string database_name,char * bitmap,uint32 size, uint64 ptr_bitmap);
int get_type(string s);
int get_type_len(vector<string> *entrance);
void erase_from_vector(vector<string>* vector_, int count);
void read_itable(struct Database_Handler dbh, struct i_table *it, uint32 n_itable);
void write_itable(struct Database_Handler dbh, struct i_table it, uint32 n_itable);
void write_block(struct Database_Handler dbh, char *block, uint32 n_block);
void read_block(struct Database_Handler dbh, char *block, uint32 n_block);
uint64 from_GB_bytes_convertion(uint32 n, int type);
uint64 from_MB_bytes_convertion(uint32 n, int type);
void printMsg(string msg);
void from_String_to_uint(string s, uint32 *n);
void setBlock_unuse(char* bitmap, uint32 blocknum);
void setBlock_use(char* bitmap, uint32 blocknum);
bool is_block_in_use(char* bitmap, uint32 blocknum);
uint32 next_available(char* bitmap, uint32 b_size);

#endif // UTILS_H
