/***********************************************************************
 * CS-450 PA3: Implementation of helper functions that abstract away 
 *             some of the reusable functionality within the application.
 * 
 * @file helper.cpp
 * @author Alexander Castro
 * @version 1.0 11/7/20
 ***********************************************************************/
#include "../hdr/helper.h"

// GLOBAL FILESYSTEM
extern class filesys* myFilesys;

///////////////////////////////////////////////////////////////////////////////////////////////
// Vector creators
///////////////////////////////////////////////////////////////////////////////////////////////

/************************************************************************
 * Takes a given filepath string and creates a vector of each directory
 * within the string. 
 * 
 * @param filepath is the filepath provided by the user
 * 
 * @param dirs is the provided vector that is filled with directories to visit
 ************************************************************************/
void make_vector_of_directories(std::string filepath, std::vector<std::string>* dirs)
{
    std::string fpcpy = filepath;

    fpcpy = fpcpy.substr(1, fpcpy.length());

    std::string delim = "/";
    size_t pos = 0;
    std::string token;

    while((pos = fpcpy.find(delim)) != std::string::npos)
    {
        token = fpcpy.substr(0, pos);
        dirs->push_back(token);
        fpcpy.erase(0, pos + delim.length());
    }

    if (fpcpy != "")
        dirs->push_back(fpcpy);
}

/************************************************************************
 * Takes a given datablock bitmap and builds a vector of all of the 
 * allocated indexes.
 * 
 * @param db_bm is the bitmap to traverse
 * 
 * @param db_len is the length of the bitmap
 * 
 * @param dbs is the provided vector that is filled with the allocated
 *            indexes
 ************************************************************************/
void make_vector_of_allocated_data_blocks(uint8_t* db_bm, int db_len, std::vector<uint8_t>* dbs)
{
    uint8_t* db_bm_cpy = db_bm;

    for (int i = 0; i < db_len; i++)
    {
        if (*db_bm_cpy != 0xff)
        {
            dbs->push_back(*db_bm_cpy);
        }
        db_bm_cpy++;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Directory Traversal
/////////////////////////////////////////////////////////////////////////////////////////////////

/**************************************************************************
 * Takes a list of directories and will traverse down through the
 * filesystem and to validate the path while also finding the inode of 
 * the either the parent of or the last element of the dirs vector. This
 * is a recursive function that uses the traverse_directory_for_filename
 * helper function to acomplish this task. This function also also for 
 * the search of deleted files.
 * 
 * @param dirs is the vector or directories that is used to traverse the 
 *             filesystem
 * 
 * @param fp is the file pointer to the partition
 * 
 * @param inode_index is the current inode that will be processed. Initially
 *                    zero.
 * 
 * @param go_to_len_zero is a flag that indicates whether the function will
 *                       return the inode of the parent of or the last element
 * 
 * @param is_deleted is a flag that indicates whether the function is looking for
 *                   a deleted file
 * 
 * @return the inode index of the last thing in the dirs vector
 *************************************************************************/ 
int get_last_directory_inode_index(std::vector<std::string>* dirs, FILE* fp, int inode_index, bool go_to_len_zero, bool isDeleted)
{
    if (dirs->size() == 1 && !go_to_len_zero)
        return inode_index;

    if (dirs->size() == 0 && go_to_len_zero)
        return inode_index;


    uint8_t inode_buffer[sizeof(inode_t)];
    inode_t* inode = (inode_t*) &inode_buffer;

    // Find and read the inode
    fseek(fp, sizeof(superblock_t) + (inode_index * sizeof(inode_t)), SEEK_SET);
    fread(inode_buffer, sizeof(inode_t), 1, fp);

    if (inode->type != 0x3333)
        return -1;

    std::string toLookFor = dirs->at(0);
    dirs->erase(dirs->begin());

    char dirToLookFor[16];
    memset(dirToLookFor, 0, 16);
    strcpy(dirToLookFor, toLookFor.c_str());

    uint8_t* db = (uint8_t *) &inode->datablocks;

    uint32_t inode_num = -1;

    if (go_to_len_zero && isDeleted && dirs->size() == 0)
        inode_num = traverse_directory_for_filename(fp, db, (char *) &dirToLookFor, isDeleted);
    else
        inode_num = traverse_directory_for_filename(fp, db, (char *) &dirToLookFor, false);

    if (inode_num == 255) 
        return -1;
    else
        return get_last_directory_inode_index(dirs, fp, inode_num, go_to_len_zero, isDeleted);
}

/*****************************************************************************
 * Traverses an inodes data block bitmap and will return the inode index that 
 * matches the provided filename. Provides support for deleted files.
 * 
 * @param fp is the file pointer to the partition
 * 
 * @param db is the db bitmap that will be traversed
 * 
 * @param dir_to_look_for is the filename that you are searching for
 * 
 * @param is_deleted is a flag that indicates whether the function is looking for
 *                   a deleted file
 * 
 * @return the inode index of the matching file
 *****************************************************************************/
uint32_t traverse_directory_for_filename(FILE* fp, uint8_t* db, char* dirToLookFor, bool isDeleted)
{
    for (unsigned int i = 0; i < 26; i++)
    {
        if (*db != 0xff)
        {
            uint8_t datablock[SECTOR_SIZE];
            dir_entry_t* datablock_entry = (dir_entry_t*) &datablock;

            fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (*db * sizeof(data_block_t)), SEEK_SET);
            fread(datablock, SECTOR_SIZE, 1, fp);

            for (int j = 0; j < SECTOR_SIZE / sizeof(dir_entry_t); j++)
            {   
                uint32_t cur_inode = datablock_entry->inode_num;

                if (cur_inode > 0)
                {
                    char* cur_dir_name_ptr = (char *) &datablock_entry->filename;

                    if (isDeleted)
                    {
                        if (*cur_dir_name_ptr == '~')
                        {
                            cur_dir_name_ptr++;
                            dirToLookFor++;
                            if (strcmp(cur_dir_name_ptr, dirToLookFor) == 0)
                            {
                                return cur_inode;
                            }
                        }
                    }
                    else
                    {
                        if (strcmp(cur_dir_name_ptr, (char*) dirToLookFor) == 0)
                        {
                            return cur_inode;
                        }
                    }
                }
                datablock_entry++;
            }
        }
        db++;
    }
    return 255;
}

/******************************************************************************
 * Traverses an inode's data block bitmap and removes any directory entry that 
 * has a matching inode index. Directory inodes are deleted completely whereas 
 * file inode's are modified so that they can be potentially recovered.
 * 
 * @param fp is the file pointer to the partition
 * 
 * @param db is the db bitmap that will be traversed
 * 
 * @param inode_to_remove is the inode index that searched for
 * 
 * @param is_dir is a flag that indicates whether or not the inode is a file or
 *               directory
 * 
 * @return 0 on success, -1 on failure
 ******************************************************************************/
int traverse_to_remove_dir_entry_with_inode(FILE* fp, uint8_t* db, int inode_to_remove, bool isDir)
{
    for (unsigned int i = 0; i < 26; i++)
    {
        if (*db != 0xff)
        {
            uint8_t datablock[SECTOR_SIZE];
            dir_entry_t* datablock_entry = (dir_entry_t*) &datablock;

            fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (*db * sizeof(data_block_t)), SEEK_SET);
            fread(datablock, SECTOR_SIZE, 1, fp);

            for (int j = 0; j < SECTOR_SIZE / sizeof(dir_entry_t); j++)
            {   
                uint32_t cur_inode = datablock_entry->inode_num;

                if (cur_inode == inode_to_remove)
                {
                    if (isDir)
                    {
                        memset(datablock_entry, 0x00, sizeof(dir_entry_t));
                    }
                    else
                    {
                        char * filename = (char *) &datablock_entry->filename;
                        *filename = '~';
                    }
                    
                    fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (*db * sizeof(data_block_t) + (j * sizeof(dir_entry_t))), SEEK_SET);
                    fwrite(datablock_entry, sizeof(dir_entry_t), 1, fp);

                    return 0;
                }
                datablock_entry++;
            }
        }
        db++;
    }
    return -1;
}

/******************************************************************************
 * Traverses an inode's data block bitmap to find the first open datablock 
 * entry.
 * 
 * @param fp is the file pointer to the partition
 * 
 * @param db is the db bitmap that will be traversed
 * 
 * @return the datablock entry index if found. -1 when not found
 ******************************************************************************/
int traverse_to_find_first_open_entry(FILE* fp, uint8_t* db)
{
    for (unsigned int i = 0; i < 26; i++)
    {
        if (*db != 0xff)
        {
            uint8_t datablock[SECTOR_SIZE];
            dir_entry_t* datablock_entry = (dir_entry_t*) &datablock;

            fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (*db * sizeof(data_block_t)), SEEK_SET);
            fread(datablock, SECTOR_SIZE, 1, fp);

            for (int j = 0; j < SECTOR_SIZE / sizeof(dir_entry_t); j++)
            {   
                uint32_t cur_inode = datablock_entry->inode_num;

                if (cur_inode == 0x00)
                {
                    return j;
                }
                datablock_entry++;
            }
        }
        db++;
    }
    return -1;
}

/******************************************************************************
 * Traverses an inode's data block bitmap to fill the provided buffer with the 
 * file's content. Will read up to the number of bytes of provided or until
 * it reaches the end of the file. Will start from the provided file offset.
 * 
 * @param db is the db bitmap that will be traversed
 * 
 * @param fp is the file pointer to the partition
 * 
 * @param buffer is the memory that will be filled with the file content
 * 
 * @param bytes_to_read is the number of bytes specifed to be read by the user
 * 
 * @param file_size is number of bytes contained in the file
 * 
 * @param file_offest is the current offset of the file pointer within the file
 * 
 * @return the datablock entry index if found. -1 when not found
 ******************************************************************************/
int traverse_to_fill_buffer_with_file_content(uint8_t* db, FILE* fp, uint8_t* buffer, int bytes_to_read, int file_size, int file_offset)
{
    int file_size_cpy = file_size - file_offset;
    int bytes_to_read_cpy = bytes_to_read;

    file_size = file_size - file_offset;

    int starting_block_index = file_offset / SECTOR_SIZE;
    int starting_byte_index = file_offset % SECTOR_SIZE;

    db = db + starting_block_index;

    for (unsigned int i = starting_block_index; i < 26; i++)
    {
        if (*db != 0xff)
        {
            uint8_t datablock[SECTOR_SIZE - starting_byte_index];

            fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (*db * sizeof(data_block_t)) + starting_byte_index, SEEK_SET);
            fread(datablock, SECTOR_SIZE - starting_byte_index, 1, fp);

            if (bytes_to_read > SECTOR_SIZE - starting_byte_index && file_size > SECTOR_SIZE - starting_byte_index)
            {
                memcpy(buffer, datablock, SECTOR_SIZE - starting_byte_index);
                buffer = buffer + (SECTOR_SIZE - starting_byte_index);
                starting_byte_index = 0;
            }
            else
            {
                if (bytes_to_read > file_size)
                {
                    memcpy(buffer, datablock, file_size);
                    return file_size_cpy;
                }
                else
                {
                    memcpy(buffer, datablock, bytes_to_read);
                    return bytes_to_read_cpy;
                }

            }
            bytes_to_read = bytes_to_read - SECTOR_SIZE;
            file_size = file_size - SECTOR_SIZE;
        }
        db++;
    }
    return -1;
}

/*****************************************************************************
 * Traverses an inode's data block bitmap and will return the inode index that 
 * matches a portion provided filename. It will then recover that file.
 * 
 * @param db is the db bitmap that will be traversed
 * 
 * @param dir_to_look_for is the filename that you are searching for
 *****************************************************************************/
void traverse_directory_to_recover_file(FILE* fp, uint8_t* db, const char* dirToLookFor)
{
    for (unsigned int i = 0; i < 26; i++)
    {
        if (*db != 0xff)
        {
            uint8_t datablock[SECTOR_SIZE];
            dir_entry_t* datablock_entry = (dir_entry_t*) &datablock;

            fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (*db * sizeof(data_block_t)), SEEK_SET);
            fread(datablock, SECTOR_SIZE, 1, fp);

            for (int j = 0; j < SECTOR_SIZE / sizeof(dir_entry_t); j++)
            {   
                uint32_t cur_inode = datablock_entry->inode_num;

                if (cur_inode > 0)
                {
                        char* cur_dir_name_ptr = (char *) &datablock_entry->filename;

                        if (*cur_dir_name_ptr == '~')
                        {
                            cur_dir_name_ptr++;

                            char* dir_to_look_for_cpy = (char *) dirToLookFor;
                            dir_to_look_for_cpy++;

                            if (strcmp(cur_dir_name_ptr, dir_to_look_for_cpy) == 0)
                            {
                                memcpy(&datablock_entry->filename, dirToLookFor, 16);

                                fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (*db * sizeof(data_block_t)) + (j * sizeof(dir_entry_t)), SEEK_SET);
                                fwrite(datablock_entry, sizeof(SECTOR_SIZE), 1, fp);
                                break;
                            }
                        }
                }
                datablock_entry++;
            }
        }
        db++;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bitmap parsers
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 * Finds the first 0x00 in a bitmap. Useful for parsing the superblock bitmaps.
 * 
 * @param bitmap is bmap that will be parsed
 * 
 * @param len is length of the bitmap
 *****************************************************************************/
int find_first_available_in_bitmap(uint8_t* bitmap, int len)
{
    uint8_t* cpy_ptr = bitmap;

    // Find first available inode index
    for (unsigned int i = 0; i < len; i++)
    {
        if (*cpy_ptr == 0x00)
        {
            return i;
        }
        cpy_ptr++;
    }

    return -1;
}

/*****************************************************************************
 * Finds the number of allocated datablocks in a db bitmap.
 * 
 * @param db is the datablock bitmap
 * 
 * @param len is length of the bitmap
 *****************************************************************************/
int find_number_of_allocated_db(uint8_t* db, int len)
{
    uint8_t* cpy_ptr = db;

    int count = 0;

    // Find first available inode index
    for (unsigned int i = 0; i < len; i++)
    {
        if (*cpy_ptr != 0xff)
        {
            count++;
        }
        cpy_ptr++;
    }
    return count;
}

/*****************************************************************************
 * Finds the first unallocated datablock in a db bitmap.
 * 
 * @param db is the datablock bitmap
 * 
 * @param len is length of the bitmap
 *****************************************************************************/
int find_first_unallocated_db_index(uint8_t* db, int len)
{
    uint8_t* cpy_ptr = db;

    // Find first available db index
    for (unsigned int i = 0; i < len; i++)
    {
        if (*cpy_ptr == 0xff)
        {
            return i;
        }
        cpy_ptr++;
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////
// File/Dir Creation helper
///////////////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 * Writes a new entry to a directory data block with the specified content.
 * 
 * @param fp is the filepointer to the partition
 * 
 * @param filename is the filename that will be written to the new entry
 * 
 * @param new_inode_index is the inode index that will be written to the 
 *                        new entry
 * 
 * @param data_block_index is which datablock the entry will be written to
 * 
 * @param new_entry_index is the index offset of where the new entry will be 
 *                        written to
 *****************************************************************************/
void write_new_entry_to_data_block(FILE* fp, std::string filename, int new_inode_index, int data_block_index, int new_entry_index)
{
    // Convert filename to char ptr
    char char_fn[16];
    memset(char_fn, 0, 16);
    strcpy(char_fn, filename.c_str());

    dir_entry_t new_entry;
    memcpy(&new_entry.filename, char_fn, 16);
    new_entry.inode_num = new_inode_index;

    // Write new data block to file
    fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (data_block_index * sizeof(data_block_t) + (new_entry_index * sizeof(dir_entry_t))), SEEK_SET);
    fwrite(&new_entry, sizeof(dir_entry_t), 1, fp);

    rewind(fp);
}


//////////////////////////////////////////////////////////////////////////////////////
// Specified Directory helpers
//////////////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 * Gets all of the directory entries from a inode's datablocks and fills the 
 * provided vector.
 * 
 * @param fp is the filepointer to the partition
 * 
 * @param db is the inodes data block bitmap
 * 
 * @param dir_entries is the vector that will be filled with the directory 
 *                    entries in the inode's data blocks
 * 
 * @return number of entries found
 *****************************************************************************/
uint32_t get_all_content_from_directory(FILE* fp, uint8_t* db, std::vector<dir_entry_t *>* dir_entries)
{
    uint32_t totalCount = 0;

    for (unsigned int i = 0; i < 26; i++)
    {
        if (*db != 0xff)
        {
            uint8_t datablock[SECTOR_SIZE];
            dir_entry_t* datablock_entry = (dir_entry_t*) &datablock;

            fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (*db * sizeof(data_block_t)), SEEK_SET);
            fread(datablock, SECTOR_SIZE, 1, fp);

            for (int j = 0; j < SECTOR_SIZE / sizeof(dir_entry_t); j++)
            {   
                uint32_t cur_inode = datablock_entry->inode_num;

                if (cur_inode > 0)
                {
                    char* fn = (char *) &datablock_entry->filename;

                    if (*fn != '~')
                    {
                        dir_entry_t* new_dir_entry = (dir_entry_t *) malloc(sizeof(dir_entry_t));
                        memcpy(new_dir_entry, datablock_entry, sizeof(dir_entry_t));

                        dir_entries->push_back(new_dir_entry);
                        totalCount++;
                    }
                }
                datablock_entry++;
            }
        }
        db++;
    }
    return totalCount;
}


/*****************************************************************************
 * Gets the number of items within an inode's data blocks.
 * 
 * @param fp is the filepointer to the partition
 * 
 * @param db is the inodes data block bitmap
 * 
 * @return number of entries found
 *****************************************************************************/
uint32_t get_number_of_items_in_directory(FILE* fp, uint8_t* db)
{
    uint32_t totalCount = 0;

    for (unsigned int i = 0; i < 26; i++)
    {
        if (*db != 0xff)
        {
            uint8_t datablock[SECTOR_SIZE];
            dir_entry_t* datablock_entry = (dir_entry_t*) &datablock;

            fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (*db * sizeof(data_block_t)), SEEK_SET);
            fread(datablock, SECTOR_SIZE, 1, fp);

            for (int j = 0; j < SECTOR_SIZE / sizeof(dir_entry_t); j++)
            {   
                uint32_t cur_inode = datablock_entry->inode_num;

                if (cur_inode > 0)
                {
                    char* fn = (char *) &datablock_entry->filename;

                    if (*fn != '~')
                    {
                        totalCount++;
                    }
                }
                datablock_entry++;
            }
        }
        db++;
    }
    return totalCount;
}


////////////////////////////////////////////////////////////////////////////////////
// File table
////////////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 * Finds the next available file descriptor within the provided file table.
 * 
 * @param ft is the file table that will be searched through
 * 
 * @return next available file descriptor
 *****************************************************************************/
int find_next_available_file_descriptor(file_table_t* ft)
{
    file_table_entry_t* cur_entry = (file_table_entry_t *) ft;

    for (int i = 0; i < sizeof(ft->entries); i++)
    {
        if (!cur_entry->isAllocated)
        {
            return i;
        }
        cur_entry++;
    }
    return -1;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Read/Write Files
//////////////////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 * Reads the specified number of bytes into the buffer from the file with the
 * provided filename.
 * 
 * @param filename is the file that will be read from
 * 
 * @param buffer is the memory that will be written into
 * 
 * @param num_of_bytes is the number of bytes that will be read
 * 
 * @return how many bytes were read
 *****************************************************************************/
int read_file_from_stu(std::string filename, uint8_t* buffer, int num_of_bytes)
{
    std::ifstream infile(filename, std::ios::in | std::ios::binary);
    infile.read((char *)buffer, num_of_bytes);
    return infile.gcount();
}


/*****************************************************************************
 * Writes the specified number of bytes from the buffer into the file with the
 * provided filename.
 * 
 * @param filename is the file that will be written to
 * 
 * @param buffer is the memory that will be read from
 * 
 * @param num_of_bytes is the number of bytes that will be written
 * 
 * @return how many bytes were written
 *****************************************************************************/
int write_file_to_stu(std::string filename, uint8_t* buffer, int num_of_bytes)
{
    std::ofstream outfile(filename, std::ios::out | std::ios::binary);
    outfile.write((char *) buffer, num_of_bytes);
    return num_of_bytes;
}