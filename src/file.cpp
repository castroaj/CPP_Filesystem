#include "../hdr/file.h"
#include "../hdr/helper.h"
#include "../hdr/filesys.h"

extern class filesys* myFilesys;

int file_create(std::string filepath)
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

    FILE* fp = myFilesys->getPartitionPtr();

    // Find the inode of the last directory in the given filepath
    int fin_dir_inode_index = get_last_directory_inode_index(&dirs, fp, 0, go_to_len_zero);

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
    // Check to see if directory already exists
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
    // Create new inode for new file
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
    new_inode.type = 0x2222;
    new_inode.size = 0;
    uint8_t* data_block_arr = (uint8_t*) &new_inode.datablocks;
    std::memset(data_block_arr, 255, sizeof(new_inode.datablocks));
    *data_block_arr = new_db_index;

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

int file_open(std::string filepath)
{
    // Copy the filepath to new string
    std::string fpcpy = filepath;
    bool go_to_len_zero = true;

    // Check to see if filepath is valid
    if (filepath.size() == 0 || filepath.at(0) != '/' || filepath.compare("/") == 0)
        return -1;
    
    // Create a vector of all the directories that need to be visited
    std::vector<std::string> dirs;
    make_vector_of_directories(filepath, &dirs);

    FILE* fp = myFilesys->getPartitionPtr();

    // Find the inode of the last directory in the given filepath
    int fin_dir_inode_index = get_last_directory_inode_index(&dirs, fp, 0, go_to_len_zero);

    if (fin_dir_inode_index == -1)
        return -1;

    // Declare memory for found inode
    uint8_t inode_buffer[sizeof(inode_t)];
    inode_t* inode = (inode_t*) &inode_buffer;

    // Find and read the inode
    fseek(fp, sizeof(superblock_t) + (fin_dir_inode_index * sizeof(inode_t)), SEEK_SET);
    fread(inode_buffer, sizeof(inode_t), 1, fp);

    if (inode->type != 0x2222)
        return -1;

    file_table_t* ft = myFilesys->getFileTable();
    file_table_entry_t* ft_entry = (file_table_entry_t *) ft;

    // Check to see if file is already opened
    for (int i = 0; i < 32; i++)
    {
        if (fin_dir_inode_index == ft_entry->inode_num)
        {
            std::cout << "File is already open" << std::endl;
            return -1;
        }
         ft_entry++;
    }

    int next_fd = find_next_available_file_descriptor(ft);

    if (next_fd == -1)
        return -1;

    uint8_t* db_index_ptr = (uint8_t *)inode->datablocks;

    int db_index = (int) *db_index_ptr;

    // Used for indexing
    file_table_entry_t* cur_entry = (file_table_entry_t *) ft;
    cur_entry += next_fd;

    file_table_entry_t new_entry;

    new_entry.isAllocated = true;
    new_entry.inode_num = fin_dir_inode_index;
    new_entry.file_offset = 0;

    // Copy new entry into file table
    memcpy(cur_entry, &new_entry, sizeof(file_table_entry_t));

    return next_fd;
}

int file_read(int file_descriptor, uint8_t* buffer, int bytes_to_read)
{
    file_table_t* ft = myFilesys->getFileTable();
    file_table_entry_t* ft_entry = (file_table_entry_t *) ft;

    ft_entry += file_descriptor;

    if (!ft_entry->isAllocated)
    {
        std::cout << "File is not open" << std::endl;
        return -1;
    }

    FILE* fp = myFilesys->getPartitionPtr();
    uint32_t inode_num = ft_entry->inode_num;

    // Declare memory for found inode
    uint8_t inode_buffer[sizeof(inode_t)];
    inode_t* inode = (inode_t*) &inode_buffer;

    // Find and read the inode
    fseek(fp, sizeof(superblock_t) + (inode_num * sizeof(inode_t)), SEEK_SET);
    fread(inode_buffer, sizeof(inode_t), 1, fp);

    uint8_t* db = (uint8_t *) inode->datablocks;

    int bytes_read = traverse_to_fill_buffer_with_file_content(db, fp, buffer, bytes_to_read, inode->size, ft_entry->file_offset);

    ft_entry->file_offset = ft_entry->file_offset + bytes_read;

    return bytes_read;
}

int file_write(int file_descriptor, uint8_t* buffer, int buffer_len)
{
    file_table_t* ft = myFilesys->getFileTable();
    file_table_entry_t* ft_entry = (file_table_entry_t *) ft;

    ft_entry += file_descriptor;

    if (!ft_entry->isAllocated)
    {
        std::cout << "File is not open" << std::endl;
        return -1;
    }

    FILE* fp = myFilesys->getPartitionPtr();
    uint32_t inode_num = ft_entry->inode_num;

    // Read in the superblock
    uint8_t superblock_buffer[sizeof(superblock_t)];
    superblock_t* sb = (superblock_t*) &superblock_buffer;

    // Read Superblock
    fseek(fp, 0, SEEK_SET);
    fread(superblock_buffer, sizeof(superblock_t), 1, fp);


    // Declare memory for found inode
    uint8_t inode_buffer[sizeof(inode_t)];
    inode_t* inode = (inode_t*) &inode_buffer;

    // Find and read the inode
    fseek(fp, sizeof(superblock_t) + (inode_num * sizeof(inode_t)), SEEK_SET);
    fread(inode_buffer, sizeof(inode_t), 1, fp);

    inode->size += buffer_len;

    uint8_t* db = (uint8_t *) inode->datablocks;

    int number_of_blocks_needed = ((buffer_len + ft_entry->file_offset) / 512) + 1;
    int num_of_allocated_blocks = find_number_of_allocated_db(db, sizeof(inode->datablocks));

    if (num_of_allocated_blocks < number_of_blocks_needed)
    {
        int num_to_allocate = number_of_blocks_needed - num_of_allocated_blocks;

        uint8_t* db_ptr_cpy = db;

        for (int i = 0; i < num_to_allocate; i++)
        {
            int new_data_block_index = find_first_available_in_bitmap((uint8_t *) &sb->dblock_bmap, sizeof(sb->dblock_bmap));
            sb->dblock_bmap[new_data_block_index] = 0x11;

            if (new_data_block_index == -1)
                break;

            int first_unallocated_index = find_first_unallocated_db_index(db, sizeof(inode->datablocks));
            db_ptr_cpy += first_unallocated_index;
            *db_ptr_cpy = new_data_block_index;
            db_ptr_cpy = db;
        }
    }

    uint8_t* buffer_ptr_cpy = buffer;
    uint8_t* db_ptr_cpy = db;

    for (int i = 0; i < number_of_blocks_needed; i++)
    {
        int bytes_to_write = SECTOR_SIZE;

        if (i == number_of_blocks_needed - 1)
            bytes_to_write = buffer_len % SECTOR_SIZE;

        fseek(fp, sizeof(superblock_t) + (9 * sizeof(inode_block_t)) + (*db_ptr_cpy * sizeof(data_block_t)) + ft_entry->file_offset, SEEK_SET);
        fwrite(buffer_ptr_cpy, bytes_to_write, 1, fp);

        buffer_ptr_cpy += SECTOR_SIZE;
        db_ptr_cpy++;
    }

    fseek(fp, sizeof(superblock_t) + (inode_num * sizeof(inode_t)), SEEK_SET);
    fwrite(inode_buffer, sizeof(inode_t), 1, fp);

    fseek(fp, 0, SEEK_SET);
    fwrite(sb, sizeof(superblock_t), 1, fp);

    ft_entry->file_offset = ft_entry->file_offset + buffer_len;

    return buffer_len;
}

int file_seek(int file_descriptor, int offset)
{
    file_table_t* ft = myFilesys->getFileTable();
    file_table_entry_t* ft_entry = (file_table_entry_t *) ft;

    ft_entry += file_descriptor;

    FILE* fp = myFilesys->getPartitionPtr();

    if (!ft_entry->isAllocated)
    {
        std::cout << "File is not open" << std::endl;
        return -1;
    }

    // Declare memory for found inode
    uint8_t inode_buffer[sizeof(inode_t)];
    inode_t* inode = (inode_t*) &inode_buffer;

    // Find and read the inode
    fseek(fp, sizeof(superblock_t) + (ft_entry->inode_num * sizeof(inode_t)), SEEK_SET);
    fread(inode_buffer, sizeof(inode_t), 1, fp);


    if (inode->size < offset)
    {
        std::cout << "Offset is larger then the size of the file" << std::endl;
        return -1;
    }

    ft_entry->file_offset = offset;

    return offset;
}


int file_close(int file_descriptor)
{
    file_table_t* ft = myFilesys->getFileTable();
    file_table_entry_t* ft_entry = (file_table_entry_t *) ft;

    ft_entry += file_descriptor;

    if (!ft_entry->isAllocated)
    {
        std::cout << "File is not open" << std::endl;
        return -1;
    }

    memset(ft_entry, 255, sizeof(file_table_entry_t));

    return 0;
}
