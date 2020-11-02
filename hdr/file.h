#include "main.h"

#ifndef FILE_H
#define FILE_H

int file_create(std::string filepath);

int file_open(std::string filepath);

int file_write(int file_descriptor, uint8_t* buffer, int buffer_len);

#endif