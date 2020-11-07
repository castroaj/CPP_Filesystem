/***********************************************************************
 * CS-450 PA3: Implementation of the filesys object
 * 
 * @file filesys.cpp
 * @author Alexander Castro
 * @version 1.0 11/7/20
 ***********************************************************************/
#include "../hdr/filesys.h"

/***********************************************************************
 * Constuctor
 ***********************************************************************/
filesys::filesys(bool mounted)
    :m_mounted(mounted)
{
    // Init file table
    m_file_table = (file_table_t *) malloc(sizeof(file_table_t));

    // Set all the of the memory in the file table to be 0xff
    memset(m_file_table, 0xff, sizeof(file_table_t));
}

/***********************************************************************
 * Destructor
 ***********************************************************************/
filesys::~filesys()
{
    // Free File Table
    free(m_file_table);

    // Close mounted Filesystem
    fclose(m_partition_ptr);
}