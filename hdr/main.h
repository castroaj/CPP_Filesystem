/***********************************************************************
 * CS-450 PA3: Header file import standard libary packages and declare
 *             necessary data structures for the application.
 * 
 * @file main.h
 * @author Alexander Castro
 * @version 1.0 10/29/20
 ***********************************************************************/

#ifndef MAIN_H
#define MAIN_H

    #include <iostream>
    #include <vector>
    #include <fstream>
    #include <cstring>
    #include <climits>
    #include <stdio.h>
    #include <inttypes.h>

    #define SECTOR_SIZE 512
    #define NUM_SECTORS 256
    #define MAGIC_NUM 12345
    #define RESERVED_SECTORS 1
    #define INODE_SECTORS 9
    #define DATA_SECTORS 246

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
            uint8_t datablocks[26];
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
            uint8_t data[SECTOR_SIZE];
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

    ///////////////////////////////////////////////////////////////////

    #ifndef FILE_TABLE_ENTRY
    #define FILE_TABLE_ENTRY

        typedef struct {
            bool isAllocated;
            uint32_t inode_num;
            uint32_t file_offset;
        } file_table_entry_t;

    #endif

    ///////////////////////////////////////////////////////////////////

    #ifndef FILE_TABLE
    #define FILE_TABLE

        typedef struct {
            file_table_entry_t entries[32];
        } file_table_t;


    #endif

    ////////////////////////////////////////////////////////////////////

    #ifndef DIR_ENTRY
    #define DIR_ENTRY

        typedef struct {
            char filename[16];
            uint32_t inode_num;
        } dir_entry_t;

    #endif 


#endif