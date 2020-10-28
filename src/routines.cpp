#include "../hdr/routines.h"
#include "../hdr/filesys.h"

extern class filesys* myFilesys;

int format(std::string fileName)
{
    partition_t* partition = (partition_t*) malloc(sizeof(partition_t));

    if (!partition) return -1;

    // Zero out partition
    std::memset(partition, 0, sizeof(partition_t));

    // initalize superblock
    partition->superblock.magic_num = MAGIC_NUM;
    partition->superblock.reserved_sectors = RESERVED_SECTORS;
    partition->superblock.inode_sectors = INODE_SECTORS;
    partition->superblock.data_sectors = DATA_SECTORS;
    
    // Set first inode as allocated for root directory
    partition->superblock.inode_bmap[0] = 0x11;

    // Set first dblock as allocated for root directory
    partition->superblock.dblock_bmap[0] = 0x11;

    // Get first inode block
    inode_block_t* first_inode_block = (inode_block_t*) &partition->inode_blocks;

    // Get first inode in first block for the root directory
    inode_t* root_dir_inode = (inode_t*) &first_inode_block->inodes[0];

    // Setup Root directory inode
    root_dir_inode->state = 0x1111;
    root_dir_inode->type = 0x3333;
    root_dir_inode->size = 0;
    int8_t* data_block_arr = (int8_t*) &root_dir_inode->datablocks;
    std::memset(data_block_arr, -1, sizeof(root_dir_inode->datablocks));
    *data_block_arr = 0;

    if (fileName.length() == 0) return -1;

    // Convert filename to char ptr
    char fn[fileName.length() + 1];
    strcpy(fn, fileName.c_str());

    // Open and write partition to specified file
    FILE* fd = fopen(fn, "w+");

    if (!fd) 
        return -1;

    fwrite((void*) partition, sizeof(partition_t), 1, fd);
    fclose(fd);
    free(partition);

    return 0;
}

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

