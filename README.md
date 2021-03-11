# Application Description:

    Author: Alexander Castro
    
    Implementation of a multipurpose filesystem in c++. This application allows
    for the formatting, mounting, and manipulation of a custom multipurpose 
    filesystem. The applications provides the user with the following user 
    interface to acomplish these tasks:
    
      1) Format a file system
      2) Mount a file system
      3) Create a directory
      4) Remove a directory
      5) List the contents of a directory
      6) Create a file
      7) Remove a file
      8) Open a file
      9) Read from a file
      10) Write to a file
      11) Seek to a location in a file
      12) Close a file
      13) Recover a deleted file
      14) Exit the program
    
    Each option provides the user with the labeled functionality.
    
    FORMAT:
      If the user chooses to format a filesystem, the program will prompt the
      user for the name of the file to store the file system in. It will then
      indicate whether or not the operation succeeded or failed.
    
    
    MOUNT:
      If the user chooses to mount a filesystem, the program will prompt the
      user for the name of the file that holds the filesystem. It will then
      indicate whether or not the operation succeeded or failed.
    
    
    CREATE DIRECTORY:
      If the user chooses to create a directory, the program will prompt the
      user for the absolute path to the directory to be created. It will then
      indicate whether or not the operation succeeded or failed.


    REMOVE DIRECTORY:
      If the user chooses to remove a directory, the program will prompt the
      user for the absolute path to the directory to be removed. It will then
      indicate whether or not the operation succeeded or failed.
    
    
    LIST CONTENTS OF DIRECTORY:
      If the user chooses to list the contents of a directory, the program will
      prompt the user for the absolute path to the directory. If the path is valid, 
      the application will output all existing elements within that directory.
    
    
    CREATE FILE:
      If the user chooses to create a file, the program will prompt the user for
      the absolute path to the file to be created. It will then
      indicate whether or not the operation succeeded or failed.
    
    
    REMOVE FILE:
      If the user chooses to remove a file, the program will prompt the user for
      the absolute path to the file to be removed. It will then
      indicate whether or not the operation succeeded or failed.
    
    
    OPEN A FILE:
      If the user chooses to open a file, the program will prompt the user for
      the absolute path to the file to be opened. It will then
      indicate whether or not the operation succeeded or failed.


    READ FROM A FILE:
       If the user chooses to read from a file, the program will prompt the user
       for a file descriptor, the name of a file to write to, and a number of bytes to read.
       The application will then read the content from that file descriptor into the
       the specifed file.
    
    
    WRITE TO A FILE:
       If the user chooses to write to a file, the program will prompt the user
       for a file descriptor, the name of a file to read from, and a number of bytes to read.
       The application will then read the content from that file into the the specifed file
       descriptor within the file system.
    
    
    SEEK TO A LOCATION IN FILE:
      If the user chooses to seek to a location in a file, the program will prompt
      the user for a file descriptor and the location in the file to which to seek.
      It will then indicate whether or not the operation succeeded or failed.
    
    
    CLOSE A FILE:
      If the user chooses to close a file, the program will prompt the user for a
      file descriptor to close. It will then indicate whether or not the operation
      succeeded or failed.
    
    RECOVER A FILE:
      If the user chooses to recover a deleted file, the program will prompt the
      user for the absolute path to the file to be recovered. It will then indicate
      whether or not the operation succeeded or failed.
    
    EXIT THE PROGRAM:
      This will exit the program. If a filesystem was formatted, it should be visible
      within the home directory. Using the linux "hd" command will give you a good 
      indicator of how the filesystem is storing the data.

# Compiling:

	The provided makefile should build the project for you. Run the following 
	command to use the makefile (You will need the g++ compiler for this step):

		make
	
	After you build the project, an executable named pa3 should be placed in the directory.

# Cleaning:

	If you wish to clean the project, then 
	you can run the following command to do so:
		
		make clean

# Running:
    
    In order to run the application, you will need to compile and build the application
    first. (See Compiling)
    
    Once the application is compiled and built, an executatble should be placed in your
    directory. You can now run the application by executing the following command:
    
        ./pa3
    
    This will prompt the user with the application user interface that is detailed above.
    
    The project does not support any command line arguements. All interations and user input
    are done at runtime.

# Known Bugs:

    If a file is deleted from a directory, the directory entry will be marked with a '~' so that it could be
    potentially recovered. However, if you decided to not recoved the file, the entry will remain within the
    parent directory. This will not be noticable from a user's  perspective, however it is visible within the
    hexdump. This bug does not effect datablocks. Any newly allocated files/directories will work correctly.
