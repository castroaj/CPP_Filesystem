#include "main.h"

#ifndef FILESYS
#define FILESYS

class filesys {
    private:
        bool m_mounted;
        file_table_t* m_file_table;
        FILE* m_partition_ptr;
    public:
        filesys(bool mounted);
        ~filesys();

        bool getMounted() { return m_mounted; }
        void setMounted(bool mounted) { m_mounted = mounted; }

        file_table_t* getFileTable() { return m_file_table; }

        FILE* getPartitionPtr() {return m_partition_ptr; }
        void setPartitionPtr(FILE* ptr) { m_partition_ptr = ptr; }
};

#endif