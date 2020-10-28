#include "../hdr/util.h"

extern class filesys* myFilesys;

bool make_vector_of_directories(std::string filepath, std::vector<std::string>* dirs)
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

    return true;
}


int get_last_directory_inode_index(std::vector<std::string>* dirs, FILE* fp, int inode_index)
{
    if (dirs->size() == 1)
        return inode_index;

    uint8_t inode_buffer[sizeof(inode_t)];
    inode_t* inode = (inode_t*) &inode_buffer;

    // Find and read the inode
    fseek(fp, sizeof(superblock_t) + (inode_index * sizeof(inode_t)), SEEK_SET);
    fread(inode_buffer, sizeof(inode_t), 1, fp);

    if (inode->type != 0x3333)
        return inode_index;

    if (inode->size == 0)
        return -1;

    std::string toLookFor = dirs->at(0);
    dirs->erase(dirs->begin());

    char dirToLookFor[16];
    memset(dirToLookFor, 0, 16);
    strcpy(dirToLookFor, toLookFor.c_str());

    int8_t* db = (int8_t *) &inode->datablocks;

    for (unsigned int i = 0; i < 26; i++)
    {
        if (*db != -1)
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

                    if (strcmp(cur_dir_name_ptr, (char*) &dirToLookFor) == 0)
                    {
                        rewind(fp);
                        return get_last_directory_inode_index(dirs, fp, cur_inode);
                    }

                }
                datablock_entry++;
            }
        }
        db++;
    }
    
    return -1;
}

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

void write_new_directory_entry_to_data_block(FILE* fp, std::string dir, int new_inode_index, int data_block_index, int new_entry_index)
{
    // Declare memory for found datablock
    uint8_t datablock[SECTOR_SIZE];
    uint8_t* db_ptr = (uint8_t *) &datablock;

    // Find and read datablock
    fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (data_block_index * sizeof(data_block_t)), SEEK_SET);
    fread(datablock, SECTOR_SIZE, 1, fp);

    // Convert filename to char ptr
    char char_dir[16];
    memset(char_dir, 0, 16);
    strcpy(char_dir, dir.c_str());

    dir_entry_t new_entry;
    memcpy(&new_entry.filename, char_dir, 16);
    new_entry.inode_num = new_inode_index;

    // Write new data block to file
    fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (data_block_index * sizeof(data_block_t) + (new_entry_index * sizeof(dir_entry_t))), SEEK_SET);
    fwrite(&new_entry, sizeof(dir_entry_t), 1, fp);

    rewind(fp);
}

