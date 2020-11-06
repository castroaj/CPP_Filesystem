/***********************************************************************
 * CS-450 PA3: Set of helper functions that abstract away some of the 
 *             reusable functionality within the application.
 * 
 * @file util.h
 * @author Alexander Castro
 * @version 1.0 10/29/20
 ***********************************************************************/

#include "main.h"

#ifndef HELPER_H
#define HELPER_H

// Vector creators
bool make_vector_of_directories(std::string filepath, std::vector<std::string>* dirs);
void make_vector_of_allocated_data_blocks(uint8_t* db_bm, int db_len, std::vector<uint8_t>* dbs);

// Directory Traversal
int get_last_directory_inode_index(std::vector<std::string>* dirs, FILE* fp, int inode_index, bool go_to_len_zero, bool isDeleted);
uint32_t traverse_directory_for_filename(FILE* fp, uint8_t* db, char* dirToLookFor, bool isDeleted);
int traverse_to_remove_dir_entry_with_inode(FILE* fp, uint8_t* db, int inode_to_remove, bool isDir);
int traverse_to_find_first_open_entry(FILE* fp, uint8_t* db);
int traverse_to_fill_buffer_with_file_content(uint8_t* db, FILE* fp, uint8_t* buffer, int bytes_to_read, int file_size, int file_offset);
void traverse_directory_to_recover_file(FILE* fp, uint8_t* db, const char* dirToLookFor);

// Bitmap parsers
int find_first_available_in_bitmap(uint8_t* bitmap, int len);
int find_number_of_allocated_db(uint8_t* db, int len);
int find_first_unallocated_db_index(uint8_t* bitmap, int len);

// File/Dir Creation helper
void write_new_entry_to_data_block(FILE* fp, std::string filename, int new_inode_index, int data_block_index, int new_entry_index);

// Specified Directory helpers
uint32_t get_all_content_from_directory(FILE* fp, uint8_t* db, std::vector<dir_entry_t*>* dir_entries);
uint32_t get_number_of_items_in_directory(FILE* fp, uint8_t* db);

// File table
int find_next_available_file_descriptor(file_table_t* ft);

// Read/Write Files
int read_file_from_stu(std::string filename, uint8_t* buffer, int num_of_bytes);
int write_file_to_stu(std::string filename, uint8_t* buffer, int num_of_bytes);

#endif