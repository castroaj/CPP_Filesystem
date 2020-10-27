#include "../hdr/filesys.h"

filesys::filesys(bool mounted)
    :m_mounted(mounted)
{
    // Init file table
    m_file_table = (file_table_t *) malloc(sizeof(file_table_t));
}


filesys::~filesys()
{
    // Free File Table
    free(m_file_table);

    // Close mounted Filesystem
    rewind(m_partition_ptr);
    fclose(m_partition_ptr);
}