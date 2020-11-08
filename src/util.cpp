/*********************************************************************************************************
 * CS-450 PA3: Implementation of utility functions for the filesystem 
 * 
 * @file util.cpp
 * @author Alexander Castro
 * @version 1.0 11/7/20
 *********************************************************************************************************/
#include "../hdr/util.h"
#include "../hdr/filesys.h"

//GLOBAL FILESYSTEM
extern class filesys* myFilesys;

/**********************************************************************************************************
 * Function that takes a given filename and creates a formatted filesystem file in the current directory
 * 
 * @param filename is the filename for the new partition file
 * 
 * @return an integer that indicates whether or not the function succeeded or not
 **********************************************************************************************************/
int format(std::string fileName)
{
    if (fileName.length() == 0) return -1;

    // Convert filename to char ptr
    char fn[fileName.length() + 1];
    strcpy(fn, fileName.c_str());

    // Open and write partition to specified file
    FILE* fp = fopen(fn, "w+");

    if (!fp) 
        return -1;

    superblock_t sb;

    std::memset(&sb, 0, sizeof(superblock_t));

    sb.magic_num = MAGIC_NUM;
    sb.reserved_sectors = RESERVED_SECTORS;
    sb.inode_sectors = INODE_SECTORS;
    sb.data_sectors = DATA_SECTORS;

    // Set first inode as allocated for root directory
    sb.inode_bmap[0] = 0x11;

    // Set first dblock as allocated for root directory
    sb.dblock_bmap[0] = 0x11;

    // Write superblock to file
    fwrite(&sb, sizeof(superblock_t), 1, fp);

    // Create the root inode
    inode_t root_dir_inode;
    root_dir_inode.state = 0x1111;
    root_dir_inode.type = 0x3333;
    root_dir_inode.size = 0;
    uint8_t* data_block_arr = (uint8_t*) &root_dir_inode.datablocks;
    std::memset(data_block_arr, 0xff, sizeof(root_dir_inode.datablocks));
    *data_block_arr = 0x00;

    // Write root inode to file
    fwrite(&root_dir_inode, sizeof(inode_t), 1, fp);

    // Write zeroed out memory for the rest of the inode sectors
    uint8_t empty_inode_sectors[sizeof(inode_t) * 143];
    memset(empty_inode_sectors, 0x00, sizeof(inode_t) * 143);
    fwrite(empty_inode_sectors, sizeof(inode_t) * 143, 1, fp);

    // Write zeroed out memory for the data sectors
    uint8_t empty_data_block[sizeof(data_block_t)];
    memset(empty_data_block, 0x00, sizeof(data_block_t));
    
    for (int i = 0; i < DATA_SECTORS; i++)
    {
        fwrite(empty_data_block, sizeof(data_block_t), 1, fp);
    }

    fclose(fp);

    return 0;
}

/****************************************************************************************************************
 * Function that mounts a filesytem with the given filename.
 * 
 * @param filename is name of the filesystem that will be mounted
 * 
 * @return an integer that indicates whether or not the function succeeded or not
 ****************************************************************************************************************/
int mount(std::string fileName)
{
    if (myFilesys)
        return -1;

    // Convert filename to char ptr
    char fn[fileName.length() + 1];
    strcpy(fn, fileName.c_str());

    // Open file and check to see if it exists
    FILE* fd = fopen(fn, "r+");
    if (!fd) 
        return -1;

    uint8_t buffer[sizeof(superblock_t)];

    if (fread(buffer, sizeof(superblock_t), 1, fd) == 0) 
    {
        return -1;
    }

    superblock_t* superblock = (superblock_t *) buffer;

    if (superblock->magic_num != MAGIC_NUM)
        return -1;

    rewind(fd);

    myFilesys = new filesys(true);
    myFilesys->setPartitionPtr(fd);

    return 0;
}

