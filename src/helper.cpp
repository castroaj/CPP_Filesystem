#include "../hdr/helper.h"

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int get_last_directory_inode_index(std::vector<std::string>* dirs, FILE* fp, int inode_index, bool go_to_len_zero)
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

    if (inode->size == 0)
        return -1;

    std::string toLookFor = dirs->at(0);
    dirs->erase(dirs->begin());

    char dirToLookFor[16];
    memset(dirToLookFor, 0, 16);
    strcpy(dirToLookFor, toLookFor.c_str());

    uint8_t* db = (uint8_t *) &inode->datablocks;

    uint32_t inode_num = traverse_directory_for_filename(fp, db, (char *) &dirToLookFor);

    if (inode_num == 255) 
        return -1;
    else
        return get_last_directory_inode_index(dirs, fp, inode_num, go_to_len_zero);
}

uint32_t traverse_directory_for_filename(FILE* fp, uint8_t* db, char* dirToLookFor)
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

                    if (strcmp(cur_dir_name_ptr, (char*) dirToLookFor) == 0)
                    {
                        return cur_inode;
                    }
                }
                datablock_entry++;
            }
        }
        db++;
    }
    return 255;
}

int traverse_to_remove_dir_entry_with_inode(FILE* fp, uint8_t* db, int inode_to_remove)
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
                    memset(datablock_entry, 0x00, sizeof(dir_entry_t));

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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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

int find_first_unallocated_db_index(uint8_t* bitmap, int len)
{
    uint8_t* cpy_ptr = bitmap;

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
                    dir_entry_t* new_dir_entry = (dir_entry_t *) malloc(sizeof(dir_entry_t));
                    memcpy(new_dir_entry, datablock_entry, sizeof(dir_entry_t));

                    dir_entries->push_back(new_dir_entry);

                    totalCount++;
                }
                datablock_entry++;
            }
        }
        db++;
    }
    return totalCount;
}

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
                    totalCount++;
                }
                datablock_entry++;
            }
        }
        db++;
    }
    return totalCount;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int read_file_from_stu(std::string filename, uint8_t* buffer, int num_of_bytes)
{
    std::ifstream infile(filename, std::ios::in | std::ios::binary);
    infile.read((char *)buffer, num_of_bytes);
    return infile.gcount();
}

