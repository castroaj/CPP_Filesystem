#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>
#include <climits>
#include <stdio.h>
#include <inttypes.h>

#pragma pack(1)

///////////////////////////////////////////////////////////////////

#ifndef SUPER_BLOCK
#define SUPER_BLOCK

typedef struct {
    uint32_t magic_num;
    uint32_t reserved_sectors;
    uint32_t inode_sectors;
    uint32_t data_sectors;
    uint8_t inode_bmap[144];
    uint8_t dblock_bmap[246];
    uint8_t unused[106];
} superblock_t;

#endif

///////////////////////////////////////////////////////////////////

#ifndef INODE
#define INODE

typedef struct {
    uint16_t state;
    uint16_t type;
    uint16_t size; 
    int8_t datablocks[26];
} inode_t;

#endif

///////////////////////////////////////////////////////////////////

#ifndef INODE_BLOCK
#define INODE_BLOCK

typedef struct {
    inode_t inodes[16];
} inode_block_t;

#endif

///////////////////////////////////////////////////////////////////

#ifndef DATA_BLOCK
#define DATA_BLOCK

typedef struct {
    uint8_t data[512];
} data_block_t;

#endif 

///////////////////////////////////////////////////////////////////

#ifndef PARTITION
#define PARTITION

typedef struct {
    superblock_t superblock;
    inode_block_t inode_blocks[9];
    data_block_t datablocks[246];
} partition_t;

#endif 