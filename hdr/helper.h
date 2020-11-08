/***********************************************************************
 * CS-450 PA3: Set of helper functions that abstract away some of the 
 *             reusable functionality within the application.
 * 
 * @file helper.h
 * @author Alexander Castro
 * @version 1.0 10/29/20
 ***********************************************************************/

#include "main.h"

#ifndef HELPER_H
#define HELPER_H

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
void make_vector_of_directories(std::string filepath, std::vector<std::string>* dirs);


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
void make_vector_of_allocated_data_blocks(uint8_t* db_bm, int db_len, std::vector<uint8_t>* dbs);


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
int get_last_directory_inode_index(std::vector<std::string>* dirs, FILE* fp, int inode_index, bool go_to_len_zero, bool is_deleted);


/*****************************************************************************
 * Traverses an inode's data block bitmap and will return the inode index that 
 * matches the provided filename. Provides support for deleted files.
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
uint32_t traverse_directory_for_filename(FILE* fp, uint8_t* db, char* dir_to_look_for, bool is_deleted);


/******************************************************************************
 * Traverses an inode's data block bitmap and removes any directory entry that 
 * has a matching inode index. Directory inodes are deleted completely whereas 
 * file inode's are modified so that they can be potentially recovered.
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
int traverse_to_remove_dir_entry_with_inode(FILE* fp, uint8_t* db, int inode_to_remove, bool is_dir);


/******************************************************************************
 * Traverses an inode's data block bitmap to find the first open datablock 
 * entry.
 * 
 * @param db is the db bitmap that will be traversed
 * 
 * @return the datablock entry index if found. -1 when not found
 ******************************************************************************/
int traverse_to_find_first_open_entry(FILE* fp, uint8_t* db);


/******************************************************************************
 * Traverses an inode's data block bitmap to fill the provided buffer with the 
 * file's content. Will read up to the number of bytes of provided or until
 * it reaches the end of the file. Will start from the provided file offset.
 * 
 * @param db is the db bitmap that will be traversed
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
int traverse_to_fill_buffer_with_file_content(uint8_t* db, FILE* fp, uint8_t* buffer, int bytes_to_read, int file_size, int file_offset);


/*****************************************************************************
 * Traverses an inode's data block bitmap and will return the inode index that 
 * matches a portion provided filename. It will then recover that file.
 * 
 * @param db is the db bitmap that will be traversed
 * 
 * @param dir_to_look_for is the filename that you are searching for
 *****************************************************************************/
void traverse_directory_to_recover_file(FILE* fp, uint8_t* db, const char* dirToLookFor);


////////////////////////////////////////////////////////////////////////////////////////
// Bitmap parsers
////////////////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 * Finds the first 0x00 in a bitmap. Useful for parsing the superblock bitmaps.
 * 
 * @param bitmap is bmap that will be parsed
 * 
 * @param len is length of the bitmap
 *****************************************************************************/
int find_first_available_in_bitmap(uint8_t* bitmap, int len);


/*****************************************************************************
 * Finds the number of allocated datablocks in a db bitmap.
 * 
 * @param db is the datablock bitmap
 * 
 * @param len is length of the bitmap
 *****************************************************************************/
int find_number_of_allocated_db(uint8_t* db, int len);


/*****************************************************************************
 * Finds the first unallocated datablock in a db bitmap.
 * 
 * @param db is the datablock bitmap
 * 
 * @param len is length of the bitmap
 *****************************************************************************/
int find_first_unallocated_db_index(uint8_t* db, int len);


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
void write_new_entry_to_data_block(FILE* fp, std::string filename, int new_inode_index, int data_block_index, int new_entry_index);


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
uint32_t get_all_content_from_directory(FILE* fp, uint8_t* db, std::vector<dir_entry_t*>* dir_entries);


/*****************************************************************************
 * Gets the number of items within an inode's data blocks.
 * 
 * @param fp is the filepointer to the partition
 * 
 * @param db is the inodes data block bitmap
 * 
 * @return number of entries found
 *****************************************************************************/
uint32_t get_number_of_items_in_directory(FILE* fp, uint8_t* db);


///////////////////////////////////////////////////////////////////////////////////////////
// File table
//////////////////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 * Finds the next available file descriptor within the provided file table.
 * 
 * @param ft is the file table that will be searched through
 * 
 * @return next available file descriptor
 *****************************************************************************/
int find_next_available_file_descriptor(file_table_t* ft);


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
int read_file_from_stu(std::string filename, uint8_t* buffer, int num_of_bytes);

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
int write_file_to_stu(std::string filename, uint8_t* buffer, int num_of_bytes);

#endif