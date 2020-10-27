#include "main.h"

#ifndef UTIL_H
#define UTIL_H

bool make_vector_of_directories(std::string filepath, std::vector<std::string>* dirs);
int get_last_directory_inode_index(std::vector<std::string>* dirs, FILE* fp, int inode_index);

#endif