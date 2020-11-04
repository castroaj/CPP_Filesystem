#include "main.h"

#ifndef FILE_H
#define FILE_H

int file_create(std::string filepath);

int file_open(std::string filepath);

int file_read(int file_descriptor, uint8_t* buffer, int bytes_to_read);

int file_write(int file_descriptor, uint8_t* buffer, int buffer_len);

int file_seek(int file_descriptor, int offset);

#endif