#include <stdio.h>
#include <yaml.h>
#include "../config.h"

int getRomIndex(long addr);

int main() {
    // Test case 1: Address within ROM range
    long addr1 = 0x00008000;
    int romIndex1 = getRomIndex(addr1);
    printf("ROM index for address 0x%lx is %d\n", addr1, romIndex1);

    // Test case 2: Address outside ROM range
    long addr2 = 0x0000A000;
    int romIndex2 = getRomIndex(addr2);
    printf("ROM index for address 0x%lx is %d\n", addr2, romIndex2);

    // Test case 3: Address at the start of ROM range
    long addr3 = 0x00000000;
    int romIndex3 = getRomIndex(addr3);
    printf("ROM index for address 0x%lx is %d\n", addr3, romIndex3);

    // Test case 4: Address at the end of ROM range
    long addr4 = 0x0000FFFF;
    int romIndex4 = getRomIndex(addr4);
    printf("ROM index for address 0x%lx is %d\n", addr4, romIndex4);

    return 0;
}