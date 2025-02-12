/*******************************************************************************************************************
 * CS-450 PA3: Implementation of functions that deal with the creation, reading, and deletion of directories.
 *             
 * @file dir.cpp
 * @author Alexander Castro
 * @version 1.0 11/6/20
 ********************************************************************************************************************/

#include "../hdr/dir.h"
#include "../hdr/helper.h"
#include "../hdr/filesys.h"

extern class filesys* myFilesys;

/********************************************************************************************************************
 * Function that takes a given filepath and creates a new directory in that location.
 * 
 * @param filepath is the filepath given by the user, that will be used
 *                 for the new directory.
 * 
 * @return an integer that indicates whether or not the function
 *         succeeded or not
  *******************************************************************************************************************/
int dir_create(std::string filepath)
{
    // Copy the filepath to new string
    std::string fpcpy = filepath;
    bool go_to_len_zero = false;

    // Check to see if filepath is valid
    if (filepath.size() == 0 || filepath.at(0) != '/' || filepath.compare("/") == 0)
        return -1;
    
    // Create a vector of all the directories that need to be visited
    std::vector<std::string> dirs;
    make_vector_of_directories(filepath, &dirs);

    // Gets the file pointer from the mounted filesystem
    FILE* fp = myFilesys->getPartitionPtr();

    // Find the inode of the last directory in the given filepath
    int fin_dir_inode_index = get_last_directory_inode_index(&dirs, fp, 0, go_to_len_zero, false);

    // Check to see if inode index is valid
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

    /////////////////////////////////////////////////////////////////////
    // Read data into each of the above buffers
    /////////////////////////////////////////////////////////////////////

    // Read Superblock
    fseek(fp, 0, SEEK_SET);
    fread(superblock_buffer, sizeof(superblock_t), 1, fp);

    // Find and read the inode
    fseek(fp, sizeof(superblock_t) + (fin_dir_inode_index * sizeof(inode_t)), SEEK_SET);
    fread(inode_buffer, sizeof(inode_t), 1, fp);

    ///////////////////////////////////////////////////////////////////////
    // Check to see if inode is directory and that is does not already exist
    ///////////////////////////////////////////////////////////////////////

    if (inode->type != 0x3333)
        return -1;

    // Get the datablock bitmap from the inode
    uint8_t* db_bm = (uint8_t *) &inode->datablocks;

    std::vector<dir_entry_t*> dir_entries;

    int num_of_items = get_all_content_from_directory(fp, db_bm, &dir_entries);
    bool dir_already_exists = false;

    for (uint8_t i = 0; i < dir_entries.size(); i++)
    {
        dir_entry_t* cur_dir = dir_entries.at(i);

        char dirToLookFor[16];
        memset(dirToLookFor, 0, 16);
        strcpy(dirToLookFor, dirs.at(dirs.size() - 1 ).c_str());

        if (strcmp((char *) &cur_dir->filename, (char *) &dirToLookFor) == 0)
        {
            std::cout << "Filenames are the same" << std::endl;
            dir_already_exists = true;
            free(cur_dir);
        }
    }

    if (dir_already_exists)
        return -1;


    ///////////////////////////////////////////////////////////////////////
    // Update parent directory inode
    ///////////////////////////////////////////////////////////////////////

    // Add 20 bytes to totalsize to account for new directory
    inode->size += sizeof(dir_entry_t);

    ///////////////////////////////////////////////////////////////////////
    // Do calculations for db indexing
    ///////////////////////////////////////////////////////////////////////

    int num_of_dir = get_number_of_items_in_directory(fp, (uint8_t *) inode->datablocks);

    // Calculate offsets
    int bitmap_index = (num_of_dir + 1) / 25;
    int new_entry_index = traverse_to_find_first_open_entry(fp, (uint8_t *) inode->datablocks);

    // Get the data block index for the new entry
    db_bm += bitmap_index;

    int number_of_allocated_db = find_number_of_allocated_db(db_bm, sizeof(inode->datablocks));

    if ((bitmap_index + 1) > number_of_allocated_db)
    {
        int new_data_block_index = find_first_available_in_bitmap((uint8_t *) &sb->dblock_bmap, sizeof(sb->dblock_bmap));
        sb->dblock_bmap[new_data_block_index] = 0x11;

        int first_unallocated_index = find_first_unallocated_db_index(db_bm, sizeof(inode->datablocks));
        *db_bm = new_data_block_index;
    }

    ///////////////////////////////////////////////////////////////////////
    // Create new inode for new directory
    ///////////////////////////////////////////////////////////////////////

    // Declare indexes
    int new_inode_index = find_first_available_in_bitmap((uint8_t *) &sb->inode_bmap, sizeof(sb->inode_bmap));
    int new_db_index = find_first_available_in_bitmap((uint8_t *) &sb->dblock_bmap, sizeof(sb->dblock_bmap));

    if (new_inode_index == -1) return -1;
    if (new_db_index == -1) return -1;

    // Update superblock bitmap
    sb->inode_bmap[new_inode_index] = 0x11;
    sb->dblock_bmap[new_db_index] = 0x11;

    inode_t new_inode;

    new_inode.state = 0x1111;
    new_inode.type = 0x3333;
    new_inode.size = 0;
    uint8_t* data_block_arr = (uint8_t*) &new_inode.datablocks;
    std::memset(data_block_arr, 255, sizeof(new_inode.datablocks));
    *data_block_arr = new_db_index;

    // Zero out newly allocated data block
    uint8_t zeros[SECTOR_SIZE];
    memset(zeros, 0, SECTOR_SIZE);
    fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (new_db_index * sizeof(data_block_t)), SEEK_SET);
    fwrite(zeros, SECTOR_SIZE, 1, fp);

    ////////////////////////////////////////////////////////////////////////
    // Write Changes to file
    ////////////////////////////////////////////////////////////////////////

    write_new_entry_to_data_block(fp, dirs.at(dirs.size() - 1), new_inode_index, *db_bm, new_entry_index);

    // Write superblock to file
    fseek(fp, 0, SEEK_SET);
    fwrite((void *)sb, sizeof(superblock_t), 1, fp);

    // Write updated directory inode to file
    fseek(fp, sizeof(superblock_t) + (fin_dir_inode_index * sizeof(inode_t)), SEEK_SET);
    fwrite((void*) inode, sizeof(inode_t), 1, fp);

    // Write new inode to file
    fseek(fp, sizeof(superblock_t) + (new_inode_index * sizeof(inode_t)), SEEK_SET);
    fwrite(&new_inode, sizeof(inode_t), 1, fp);

    return 0;
}

/******************************************************************************************************************
 * Function that takes a given filepath and returns the number of bytes contained within a directory.
 * 
 * @param filepath is the filepath given by the user, that will be
 *                 searched for.
 * 
 * @return an integer that indicates whether or not the funtion 
 *         succeeded or not
*******************************************************************************************************************/
int dir_size(std::string filepath)
{
    // Copy the filepath to new string
    std::string fpcpy = filepath;
    bool go_to_len_zero = true;

    // Check to see if filepath is valid
    if (filepath.size() == 0 || filepath.at(0) != '/')
        return -1;
    
    // Create a vector of all the directories that need to be visited
    std::vector<std::string> dirs;

    if (filepath.compare("/") != 0)
        make_vector_of_directories(filepath, &dirs);
    else
    {
        dirs.push_back(filepath);
        go_to_len_zero = false;
    }   

    FILE* fp = myFilesys->getPartitionPtr();

    // Find the inode of the last directory in the given filepath
    int fin_dir_inode_index = get_last_directory_inode_index(&dirs, fp, 0, go_to_len_zero, false);

    if (fin_dir_inode_index == -1 || (fin_dir_inode_index == 0 && filepath.compare("/") != 0))
        return -1;

    // Declare memory for found inode
    uint8_t inode_buffer[sizeof(inode_t)];
    inode_t* inode = (inode_t*) &inode_buffer;

    // Find and read the inode
    fseek(fp, sizeof(superblock_t) + (fin_dir_inode_index * sizeof(inode_t)), SEEK_SET);
    fread(inode_buffer, sizeof(inode_t), 1, fp);

    return inode->size;
}

/******************************************************************************************************************** 
 * Function that takes a given filepath and prints out the data within that directory if it exists
 * 
 * @param filepath is the filepath given by the user, that will be
 *                 searched for.
 * 
 * @return an integer that indicates whether or not the funtion 
 *         succeeded or not
 *******************************************************************************************************************/
int dir_read(std::string filepath)
{
    // Copy the filepath to new string
    std::string fpcpy = filepath;
    bool go_to_len_zero = true;

    // Check to see if filepath is valid
    if (filepath.size() == 0 || filepath.at(0) != '/')
        return -1;
    
    // Create a vector of all the directories that need to be visited
    std::vector<std::string> dirs;
    if (filepath.compare("/") != 0)
        make_vector_of_directories(filepath, &dirs);
    else
    {
        dirs.push_back(filepath);
        go_to_len_zero = false;
    }

    FILE* fp = myFilesys->getPartitionPtr();

    // Find the inode of the last directory in the given filepath
    int fin_dir_inode_index = get_last_directory_inode_index(&dirs, fp, 0, go_to_len_zero, false);

    if (fin_dir_inode_index == -1 || (fin_dir_inode_index == 0 && filepath.compare("/") != 0))
        return -1;

    // Declare memory for found inode
    uint8_t inode_buffer[sizeof(inode_t)];
    inode_t* inode = (inode_t*) &inode_buffer;

    // Find and read the inode
    fseek(fp, sizeof(superblock_t) + (fin_dir_inode_index * sizeof(inode_t)), SEEK_SET);
    fread(inode_buffer, sizeof(inode_t), 1, fp);

    uint8_t* db = (uint8_t *) &inode->datablocks;

    // Get all of the content from the current directory's datablock
    std::vector<dir_entry_t *> dir_entries;
    uint32_t num_of_items = get_all_content_from_directory(fp, db, &dir_entries);

    if (num_of_items != dir_entries.size())
        return -1;

    if (num_of_items > 0)
    {
        std::cout << "\nDirectory Contents:\n";
        for (unsigned int i = 0; i < num_of_items; i++)
        {
            dir_entry_t* cur_entry = dir_entries.at(i);
            std::cout << cur_entry->filename << std::endl;
            free(cur_entry);
        }
        std::cout << "\n";
    }

    return num_of_items;
}

/******************************************************************************************************************
 * Function that takes a given filepath and deletes the directory if it exists
 * 
 * @param filepath is the filepath given by the user, that will be used
 *                 for the new directory.
 * 
 * @return an integer that indicates whether or not the function
 *         succeeded or not
 *******************************************************************************************************************/
int dir_unlink(std::string filepath)
{
    // Copy the filepath to new string
    std::string fpcpy = filepath;

    // Check to see if filepath is valid
    if (filepath.size() == 0 || filepath.at(0) != '/' || filepath.compare("/") == 0)
        return -1;
    
    // Create a vector of all the directories that need to be visited
    std::vector<std::string> dirs;
    make_vector_of_directories(filepath, &dirs);

    // Create a second vector in order to get the parent inode
    std::vector<std::string> dirs2;
    make_vector_of_directories(filepath, &dirs2);

    FILE* fp = myFilesys->getPartitionPtr();

    // Find the inode of the last directory in the given filepath
    int fin_dir_inode_index = get_last_directory_inode_index(&dirs, fp, 0, true, false);
    int parent_inode_index = get_last_directory_inode_index(&dirs2, fp, 0, false, false);

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

    // Declare memory for found inode
    uint8_t parent_inode_buffer[sizeof(inode_t)];
    inode_t* parent_inode = (inode_t*) &parent_inode_buffer;

    /////////////////////////////////////////////////////////////////////
    // Read data into each of the above buffers
    /////////////////////////////////////////////////////////////////////

    // Read Superblock
    fseek(fp, 0, SEEK_SET);
    fread(superblock_buffer, sizeof(superblock_t), 1, fp);

    // Find and read the inode
    fseek(fp, sizeof(superblock_t) + (fin_dir_inode_index * sizeof(inode_t)), SEEK_SET);
    fread(inode_buffer, sizeof(inode_t), 1, fp);

    // Find and read the parent inode
    fseek(fp, sizeof(superblock_t) + (parent_inode_index * sizeof(inode_t)), SEEK_SET);
    fread(parent_inode_buffer, sizeof(inode_t), 1, fp);

    // Check to see if inode is a directory
    if (inode->type != 0x3333)
        return -1;

    // Validate that the directory is empty
    if (inode->size != 0)
        return -1;

    ///////////////////////////////////////////////////////////////////////
    // Make changes to superblock
    ///////////////////////////////////////////////////////////////////////

    uint8_t* inode_bmap = (uint8_t *) &sb->inode_bmap;
    
    inode_bmap += fin_dir_inode_index;
    *inode_bmap = 0x00;

    // Get the datablock bitmap from the inode
    uint8_t* db_bm = (uint8_t *) &inode->datablocks;

    // Get a vector of the datablock numbers
    std::vector<uint8_t> cur_data_blocks;
    make_vector_of_allocated_data_blocks(db_bm, sizeof(inode->datablocks), &cur_data_blocks);

    uint8_t* sb_db_bmap = (uint8_t *) &sb->dblock_bmap; 
    uint8_t* sb_db_bmap_cpy = sb_db_bmap;

    // Remove the previously allocated data blocks
    for (int i = 0; i < cur_data_blocks.size(); i++)
    {
        sb_db_bmap_cpy += cur_data_blocks.at(i);
        *sb_db_bmap_cpy = 0x00;
        sb_db_bmap_cpy = sb_db_bmap;
    }

    // Write superblock to file
    fseek(fp, 0, SEEK_SET);
    fwrite((void *)sb, sizeof(superblock_t), 1, fp);

    ///////////////////////////////////////////////////////////////////////
    // Make changes to inode
    ///////////////////////////////////////////////////////////////////////
    inode->state = 0x0000;

    // Write updated directory inode to file
    fseek(fp, sizeof(superblock_t) + (fin_dir_inode_index * sizeof(inode_t)), SEEK_SET);
    fwrite((void*) inode, sizeof(inode_t), 1, fp);

    ///////////////////////////////////////////////////////////////////////
    // Make changes to parent inode
    ///////////////////////////////////////////////////////////////////////

    parent_inode->size = parent_inode->size - sizeof(dir_entry_t);

    uint8_t* parent_db_bm = (uint8_t *) &parent_inode->datablocks;

    // Write updated directory inode to file
    fseek(fp, sizeof(superblock_t) + (parent_inode_index * sizeof(inode_t)), SEEK_SET);
    fwrite((void*) parent_inode, sizeof(inode_t), 1, fp);

    return traverse_to_remove_dir_entry_with_inode(fp, parent_db_bm, fin_dir_inode_index, true);
}
