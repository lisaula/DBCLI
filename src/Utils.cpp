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
