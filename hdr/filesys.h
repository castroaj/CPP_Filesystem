/***********************************************************************
 * CS-450 PA3: Class header file that defines the the filesys object 
 * 
 * @file filesys.h
 * @author Alexander Castro
 * @version 1.0 11/7/20
 ***********************************************************************/
#include "main.h"

#ifndef FILESYS
#define FILESYS

/********************************************************
 * Class that encapsulates the filesystem when it 
 * is loaded into main memory
 ********************************************************/
class filesys {
    private:
        bool m_mounted;                 // Mounted flag
        file_table_t* m_file_table;     // Open File Table
        FILE* m_partition_ptr;          // Pointer to partition
    public:
        filesys(bool mounted);          // Constructor
        ~filesys();                     // Deconstructor

        /*************************************************
         * GETTER AND SETTER FOR MOUNTED FLAG
         ************************************************/
        bool getMounted() { return m_mounted; }
        void setMounted(bool mounted) { m_mounted = mounted; }

        /*************************************************
         * GETTER FOR THE FILE TABLE
         ************************************************/
        file_table_t* getFileTable() { return m_file_table; }

        /*************************************************
         * GETTER AND SETTER FOR THE PARTITION POINTER
         ************************************************/
        FILE* getPartitionPtr() {return m_partition_ptr; }
        void setPartitionPtr(FILE* ptr) { m_partition_ptr = ptr; }
};

#endif