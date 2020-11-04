/***********************************************************************
 * CS-450 PA3: Set of functions that deal with the creation, reading,
 *             and deletion of directories.
 * 
 * @file dir.h
 * @author Alexander Castro
 * @version 1.0 10/29/20
 ***********************************************************************/

#include "main.h"

#ifndef DIR_H
#define DIR_H

/***********************************************************************
 * Function that takes a given filepath and creates a new directory in
 * that location. 
 * 
 * @param filepath is the filepath given by the user, that will be used
 *                 for the new directory.
 * 
 * @return an integer that indicates whether or not the function
 *         succeeded or not
 ***********************************************************************/
int dir_create(std::string filepath);

/***********************************************************************
 * Function that takes a given filepath and returns the number of bytes
 * contained within a directory.
 * 
 * @param filepath is the filepath given by the user, that will be
 *                 searched for.
 * 
 * @return an integer that indicates whether or not the funtion 
 *         succeeded or not
 ***********************************************************************/
int dir_size(std::string filepath);

/***********************************************************************
 * Function that takes a given filepath and prints out the data within
 * that directory if it exists
 * 
 * @param filepath is the filepath given by the user, that will be
 *                 searched for.
 * 
 * @return an integer that indicates whether or not the funtion 
 *         succeeded or not
 ***********************************************************************/
int dir_read(std::string filepath);

#endif
