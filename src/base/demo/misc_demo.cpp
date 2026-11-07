#include "stdio.h"
#include <stdint.h>


union UnionUint16{
    uint16_t val;
    struct{
        uint8_t low;
        uint8_t high;
    };
};

void misc_demo(){
    uint8_t val = 5;
    val = 261; // overflow or max limit?
    printf("%u\n", val);
    UnionUint16 un = {0x1386};
    printf("%x,%x,%x\n", un.val, un.low,un.high);
}