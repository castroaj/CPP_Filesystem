#include "../hdr/dir.h"
#include "../hdr/util.h"
#include "../hdr/filesys.h"

extern class filesys* myFilesys;

int dir_create(std::string filepath)
{
    // Copy the filepath to new string
    std::string fpcpy = filepath;

    // Check to see if filepath is valid
    if (filepath.size() == 0 || filepath.at(0) != '/' || filepath.compare("/") == 0)
        return -1;
    
    // Create a vector of all the directories that need to be visited
    std::vector<std::string> dirs;
    make_vector_of_directories(filepath, &dirs);

    FILE* fp = myFilesys->getPartitionPtr();

    // Find the inode of the last directory in the given filepath
    int fin_dir_inode_index = get_last_directory_inode_index(&dirs, fp, 0);

    if (fin_dir_inode_index == -1)
        return -1;

    //////////////////////////////////////////////////////////////////////
    // Declare memory for each section of current node
    //////////////////////////////////////////////////////////////////////

    // Read in the superblock
    uint8_t superblock_buffer[sizeof(superblock_t)];
    superblock_t* sb = (superblock_t*) &superblock_buffer;

    // Declare memory for found inode
    uint8_t inode_buffer[sizeof(inode_t)];
    inode_t* inode = (inode_t*) &inode_buffer;

    // Declare memory for found datablock
    uint8_t datablock[SECTOR_SIZE];
    uint8_t* db_ptr = (uint8_t *) datablock;

    /////////////////////////////////////////////////////////////////////
    // Read data into each of the above buffers
    /////////////////////////////////////////////////////////////////////

    // Read Superblock
    fread(superblock_buffer, sizeof(superblock_t), 1, fp);

    // Find and read the inode
    fseek(fp, sizeof(superblock_t) + (fin_dir_inode_index * sizeof(inode_t)), SEEK_SET);
    fread(inode_buffer, sizeof(inode_t), 1, fp);

    ///////////////////////////////////////////////////////////////////////
    // Do calculations for db indexing
    ///////////////////////////////////////////////////////////////////////

    // Calculate offsets
    int bitmap_index = (inode->size / 20) / 25;
    int new_entry_index = ((inode->size / 20) % 25);

    // Get the datablock bitmap from the inode
    int8_t* db_bm = (int8_t *) &inode->datablocks;

    // Get the data block index for the new entry
    db_bm += bitmap_index;
    int data_block = *db_bm; 

    // Advance pointer to where new entry will go
    db_ptr += (new_entry_index * sizeof(dir_entry_t));

    ///////////////////////////////////////////////////////////////////////
    // Update directory inode
    ///////////////////////////////////////////////////////////////////////

    // Add 20 bytes to totalsize to account for new directory
    inode->size += sizeof(dir_entry_t);

    // Write updated directory inode to file
    fseek(fp, sizeof(superblock_t) + (fin_dir_inode_index * sizeof(inode_t)), SEEK_SET);
    fwrite((void*) inode, sizeof(inode_t), 1, fp);

    ///////////////////////////////////////////////////////////////////////
    // Create new inode for new directory
    ///////////////////////////////////////////////////////////////////////

    // Get inode bmap from superblock
    uint8_t* inode_bm = (uint8_t *) &sb->inode_bmap;
    
    // Get db bmap from superblock
    uint8_t* db_bmap = (uint8_t *) &sb->dblock_bmap;

    // Declare indexes
    int new_inode_index = -1;
    int new_db_index = -1;

    // Find first available inode index
    for (unsigned int i = 0; i < sizeof(sb->inode_bmap); i++)
    {
        if (*inode_bm == 0)
        {
            *inode_bm = 1;
            new_inode_index = i;
            break;
        }
        inode_bm++;
    }

    // Find first available datablock index
    for (unsigned int i = 0; i < sizeof(sb->dblock_bmap); i++)
    {
        if (*db_bmap == 0)
        {
            *db_bmap = 1;
            new_db_index = i;
            break;
        }
        db_bmap++;
    }

    inode_t new_inode;

    new_inode.state = 1111;
    new_inode.type = 3333;
    new_inode.size = 0;
    int8_t* data_block_arr = (int8_t*) &new_inode.datablocks;
    std::memset(data_block_arr, -1, sizeof(new_inode.datablocks));
    *data_block_arr = new_db_index;

    fseek(fp, sizeof(superblock_t) + (new_inode_index * sizeof(inode_t)), SEEK_SET);
    fwrite(&new_inode, sizeof(inode_t), 1, fp);

    ////////////////////////////////////////////////////////////////////////
    // Write updated Superblock to file
    ////////////////////////////////////////////////////////////////////////
    
    fseek(fp, 0, SEEK_SET);
    fwrite((void *)sb, sizeof(superblock_t), 1, fp);

    ////////////////////////////////////////////////////////////////////////
    // Write new entry to Datablock
    ////////////////////////////////////////////////////////////////////////

    // Find and read datablock
    fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (*db_bm * sizeof(data_block_t)), SEEK_SET);
    fread(datablock, SECTOR_SIZE, 1, fp);

    // Convert filename to char ptr
    std::string dir = dirs.at(dirs.size() -1);    
    char char_dir[16];
    memset(char_dir, 0, 16);
    strcpy(char_dir, dir.c_str());

    dir_entry_t new_entry;
    memcpy(&new_entry.filename, char_dir, 16);
    new_entry.inode_num = new_inode_index;

    // Write new data block to file
    fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (*db_bm * sizeof(data_block_t) + ( new_entry_index * sizeof(dir_entry_t))), SEEK_SET);
    fwrite(&new_entry, sizeof(dir_entry_t), 1, fp);

    rewind(fp);

    return 0;
}

int dir_size(std::string filepath)
{
    return 0;
}