#include "main.h"

#ifndef UTIL_H
#define UTIL_H

bool make_vector_of_directories(std::string filepath, std::vector<std::string>* dirs);
int get_last_directory_inode_index(std::vector<std::string>* dirs, FILE* fp, int inode_index);
int find_first_available_in_bitmap(uint8_t* bitmap, int len);
void write_new_directory_entry_to_data_block(FILE* fp, std::string dir, int new_inode_index, int data_block_index, int new_entry_index);
uint32_t traverse_directory_for_filename(FILE* fp, uint8_t* db, char* dirToLookFor);
uint32_t get_all_content_from_directory(FILE* fp, uint8_t* db, std::vector<dir_entry_t*>* dir_entries);
uint32_t get_number_of_items_in_directory(FILE* fp, uint8_t* db);

#endif