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

    if (inode->type != 3333)
        return inode_index;

    if (inode->size == 0)
        return -1;
    

    int8_t* db = (int8_t *) &inode->datablocks;

    for (unsigned int j = 0; j < 26; j++)
    {
        if (*db != -1)
        {
            fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (*db * sizeof(data_block_t)), SEEK_SET);
            uint8_t datablock[SECTOR_SIZE];
            uint8_t* db = (uint8_t*) &datablock;

            fread(datablock, SECTOR_SIZE, 1, fp);

        }
        db++;
    }

    
    return 1;
    

}