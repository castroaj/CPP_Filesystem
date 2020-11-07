/***********************************************************************
 * CS-450 PA3: Header file for utility functions for the filesystem 
 * 
 * @file util.h
 * @author Alexander Castro
 * @version 1.0 11/7/20
 ***********************************************************************/
#include "main.h"

#ifndef UTIL_H
#define UTIL_H

/***********************************************************************
 * Function that takes a given filename and creates a formatted filesystem
 * file in the current directory
 * 
 * @param filename is the filename for the new partition file
 * 
 * @return an integer that indicates whether or not the function
 *         succeeded or not
 ***********************************************************************/
int format(std::string filename);

/***********************************************************************
 * Function that mounts a filesytem with the given filename.
 * 
 * @param filename is name of the filesystem that will be mounted
 * 
 * @return an integer that indicates whether or not the function
 *         succeeded or not
 ***********************************************************************/
int mount(std::string filename);

#endif
