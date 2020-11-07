/***********************************************************************
 * CS-450 PA3: Set of functions that deal with the creation, reading,
 *             and deletion of files.
 * 
 * @file file.h
 * @author Alexander Castro
 * @version 1.0 10/29/20
 ***********************************************************************/
#include "main.h"

#ifndef FILE_H
#define FILE_H

/***********************************************************************
 * Function that takes a given filepath and creates a new file in
 * that location. 
 * 
 * @param filepath is the filepath given by the user, that will be used
 *                 for the new file.
 * 
 * @return an integer that indicates whether or not the function
 *         succeeded or not
 ***********************************************************************/
int file_create(std::string filepath);


/***********************************************************************
 * Function that takes a given filepath and opens the file if it exists
 * and is not already opened. 
 * 
 * @param filepath is the filepath given by the user, that will be used
 *                 for the new file.
 * 
 * @return an integer that indicates whether or not the function
 *         succeeded or not
 ***********************************************************************/
int file_open(std::string filepath);


/***********************************************************************
 * Function that will read a specified number of bytes into a buffer
 * from the given file descriptor.
 * 
 * @param file_descriptor is the file descriptor given by the user that 
 *                        will be read from
 * 
 * @param buffer is the memory that will be filled with the content from
 *               the file
 * 
 * @param bytes_to_read is the number of bytes specified by the user that
 *                      will be read into the buffer
 * 
 * @return the number of bytes read from the file
 ***********************************************************************/
int file_read(int file_descriptor, uint8_t* buffer, int bytes_to_read);


/***********************************************************************
 * Function that will write a specified number of bytes to a given file
 * descriptor from the given buffer.
 * 
 * @param file_descriptor is the file descriptor given by the user that 
 *                        will be written to
 * 
 * @param buffer is the memory that contains the data that will be written
 *               to the file descriptor
 * 
 * @param bytes_to_read is the number of bytes specified by the user that
 *                      will be written to the file
 * 
 * @return the number of bytes written to the file
 ***********************************************************************/
int file_write(int file_descriptor, uint8_t* buffer, int buffer_len);


/***********************************************************************
 * Function that takes a file descriptor and offset and sets the file
 * pointer in the open file table.
 * 
 * @param file_descriptor is the file descriptor of the open file
 * 
 * @param offset is the number of bytes into the file that the file 
 *               pointer will be set to.
 * 
 * @return an integer that indicates whether or not the function
 *         succeeded or not
 ***********************************************************************/
int file_seek(int file_descriptor, int offset);


/***********************************************************************
 * Function that takes a file descriptor and closes it if it is open
 * 
 * @param file_descriptor is the file descriptor of the file that will be 
 *                        closed
 * 
 * @return an integer that indicates whether or not the function
 *         succeeded or not
 ***********************************************************************/
int file_close(int file_descriptor);

/***********************************************************************
 * Function that takes a given filepath and removes the file if it exists
 * 
 * @param filepath is the filepath given by the user, that will be used
 *                 to remove the file.
 * 
 * @return an integer that indicates whether or not the function
 *         succeeded or not
 ***********************************************************************/
int file_unlink(std::string filepath);


/***********************************************************************
 * Function that recovers a deleted file if it exisits and has not been 
 * corrupted
 * 
 * @param filepath is the filepath given by the user, that will be used
 *                 to recover a deleted file
 * 
 * @return an integer that indicates whether or not the function
 *         succeeded or not
 ***********************************************************************/
int file_ununlink(std::string filepath);

#endif