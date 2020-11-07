/*******************************************************************************************************************
 * CS-450 PA3: Driver of the application. Contains the user interface that will allows the user to interact
 * with the filesystem.
 *             
 * @file main.cpp
 * @author Alexander Castro
 * @version 1.0 10/29/20
 ********************************************************************************************************************/

#include "../hdr/util.h"
#include "../hdr/file.h"
#include "../hdr/dir.h"
#include "../hdr/filesys.h"
#include "../hdr/helper.h"

// GLOBAL FILESYSTEM 
filesys* myFilesys;


void promptUserWithMenu()
{
    using namespace std;
    cout << "Filesystem Menu" << endl;
    cout << endl;
    cout << "1) Format a file system" << endl;
    cout << "2) Mount a file system" << endl;
    cout << "3) Create a directory" << endl;
    cout << "4) Remove a directory" << endl;
    cout << "5) List the contents of a directory" << endl;
    cout << "6) Create a file" << endl;
    cout << "7) Remove a file" << endl;
    cout << "8) Open a file" << endl;
    cout << "9) Read from a file" << endl;
    cout << "10) Write to a file" << endl;
    cout << "11) Seek to a location in a file" << endl;
    cout << "12) Close a file" << endl;
    cout << "13) Recover a deleted file (extra credit)" << endl;
    cout << "14) Exit the program" << endl;
    cout << endl;
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool check_if_mounted()
{
    if (!myFilesys) 
    {
        std::cout << "\nFilesystem needs to be mounted" << std::endl;
        return false;
    }
    return true;
}

int write_helper(std::string input3, int fd, int num_of_bytes)
{
    uint8_t buffer[num_of_bytes];
    int bytes_read = read_file_from_stu(input3, buffer, num_of_bytes);

    if (num_of_bytes != 0 && bytes_read != 0)
        return file_write(fd, buffer, bytes_read);
    else
        return -1;
}


int read_helper(std::string input3, int fd, int num_of_bytes)
{
    uint8_t buffer[num_of_bytes];
    int ret = file_read(fd, buffer, num_of_bytes);

    if (ret != -1)
        return write_file_to_stu(input3, buffer, ret);
    else
        return -1;    
}


void print_open_file_table()
{
	file_table_t* ft = (file_table_t *) myFilesys->getFileTable();

    file_table_entry_t* ft_entry = (file_table_entry_t *) ft;

    for (int i = 0; i < 32; i++)
    {
        if (ft_entry->isAllocated != 255)
        {
            std::cout << "Descriptor: " << i << " | IsAllocated: " << ft_entry->isAllocated << " | Inode Number: " << ft_entry->inode_num  << " | File Offset: " <<  ft_entry->file_offset << std::endl;
        }
        ft_entry++;
    }
    std::cout << "\n" << std::endl;
}


int main(int argc, char* argv[])
{
    using namespace std;

    bool promptUser = true;
    std::string input;
    std::string input2;
    std::string input3;
    std::string input4;

    while (promptUser)
    {
        promptUserWithMenu();

        cout << "Please Select a option: ";
        std::getline(std::cin, input);

        int inputInt = -1;
        int fd = -1;
        int num_of_bytes = -1;

        if (is_number(input))
            inputInt = std::stoi( input );

        int ret;

        switch (inputInt)
        {
            /**
             * FORMAT
             */ 
            case 1:
                cout << "Enter a filename for the new filesystem: ";
                std::getline(std::cin, input2);
                ret = format(input2);

                if (ret == -1)
                    cout << "\nFilesystem failed to format\n" << endl;
                else
                    cout << "\n" << input2 << " has been successfully formatted\n" << endl;
                break;

            /**
             * MOUNT
             */ 
            case 2:
                cout << "Enter a filename to mount: ";
                std::getline(std::cin, input2);
                ret = mount(input2);

                if (ret == -1)
                    cout << "\nFilesystem failed to mount\n" << endl;
                else
                    cout << "\n" << input2 << " has been successfully mounted\n" << endl;
                break;

            /**
             * CREATE DIRECTORY
             */ 
            case 3:
                if (!check_if_mounted())
                    break;

                cout << "Enter path to a new directory: ";
                std::getline(std::cin, input2);
                ret = dir_create(input2);

                if (ret == -1)
                    cout << "\nCreation of " << input2 << " failed\n" << endl;
                else
                    cout << "\n" << input2 << " was sucessfully created\n" << endl;
                break;

            /**
             * REMOVE DIRECTORY
             */ 
            case 4:
                if (!check_if_mounted())
                    break;

                cout << "Enter path to a directory to remove: ";
                std::getline(std::cin, input2);
                ret = dir_unlink(input2);
                
                if (ret == -1)
                    cout << "\nDeletion of " << input2 << " failed\n" << endl;
                else
                    cout << "\n" << input2 << " was sucessfully deleted\n" << endl;

                break;

            /**
             * LIST CONTENTS OF DIRECTORY
             */ 
            case 5:
                if (!check_if_mounted())
                    break;

                cout << "Enter a path to the directory: ";
                std::getline(std::cin, input2);
                ret = dir_read(input2);

                if (ret <= 0)
                    cout << "\n" << input2 << " was non-existant or empty\n" << endl;

                break;

            /**
             * CREATE A FILE
             */ 
            case 6:
                if (!check_if_mounted())
                    break;

                cout << "Enter a path to a new file: ";
                std::getline(std::cin, input2);
                ret = file_create(input2);

                if (ret == -1)
                    cout << "\nCreation of " << input2 << " failed\n" << endl;
                else
                    cout << "\n" << input2 << " was sucessfully created\n" << endl;

                break;

            /**
             * REMOVE FILE
             */ 
            case 7:
                if (!check_if_mounted())
                    break;

                cout << "Enter path to a file to remove: ";
                std::getline(std::cin, input2);
                ret = file_unlink(input2);
                
                if (ret == -1)
                    cout << "\nDeletion of " << input2 << " failed\n" << endl;
                else
                    cout << "\n" << input2 << " was sucessfully deleted\n" << endl;

                break;

            /**
             * OPEN A FILE
             */ 
            case 8:
                if (!check_if_mounted())
                    break;

                cout << "Enter a path to the file you wish to open: ";
                std::getline(std::cin, input2);
                ret = file_open(input2);

                if (ret != -1)
                    cout << "\nThe file has been opened.\nThe file descriptor is " << ret << "\n" << endl;
                else
                    cout << "\nThe file failed to open\n" << endl;

                break;

            /**
             * READ FROM FILE
             */ 
            case 9:
                if (!check_if_mounted())
                    break;

                cout << "Enter a file descriptor to read from: ";
                std::getline(std::cin, input2);

                // Validate that the input is a number
                if (is_number(input2))
                {
                    try 
                    {
                        fd = std::stoi( input2 );
                    }
                    catch (std::out_of_range &e)
                    {
                        break;
                    }
                }

                cout << "Enter the name of the new file on stu: ";
                std::getline(std::cin, input3);

                cout << "Enter the number of bytes to read: ";
                std::getline(std::cin, input4);

                // Validate that the input is a number
                if (is_number(input4))
                {
                    try 
                    {
                        num_of_bytes = std::stoi( input4 );
                    }
                    catch (std::out_of_range &e)
                    {
                        cout << "Number of bytes is too large" << endl;
                        break;
                    }
                }

                ret = read_helper(input3, fd, num_of_bytes);
                
                if (ret > 0)
                {
                    cout << "\n" << ret << " bytes have been sucessfully read into " << input3 << "\n" << endl;
                }
                else
                {
                    cout << "\nThe read operation has failed\n" << endl;
                }
            
                break;

            /**
             * WRITE TO A FILE
             */ 
            case 10:
                if (!check_if_mounted())
                    break;

                cout << "Enter a file descriptor to write to: ";
                std::getline(std::cin, input2);

                // Validate that the input is a number
                if (is_number(input2))
                {
                    try 
                    {
                        fd = std::stoi( input2 );
                    }
                    catch (std::out_of_range &e)
                    {
                        cout << "Fd is too large" << endl;
                        break;
                    }
                }

                cout << "Enter the name of a file on stu: ";
                std::getline(std::cin, input3);

                cout << "Enter the number of bytes to write: ";
                std:: getline(std::cin, input4);

                // Validate that the input is a number
                if (is_number(input4))
                {
                    try 
                    {
                        num_of_bytes = std::stoi( input4 );
                    }
                    catch (std::out_of_range &e)
                    {
                        cout << "Number of bytes is too large" << endl;
                        break;
                    }
                }

                ret = write_helper(input3, fd, num_of_bytes);
                
                if (ret > 0)
                {
                    cout << "\n" << ret << " bytes have been sucessfully written from " << input3 << "\n" << endl;
                }
                else
                {
                    cout << "\nThe write operation has failed\n" << endl;
                }

                break;

            /**
             * SEEK TO A LOCATION IN FILE
             */ 
            case 11:
                if (!check_if_mounted())
                    break;

                cout << "Enter a file descriptor to seek within: ";
                std::getline(std::cin, input2);

                if (is_number( input2 ))
                    fd = std::stoi( input2 );
                else
                    break;
                
                cout << "Enter an offset to set the file pointer to: ";
                std::getline(std::cin, input3);

                if (is_number( input3 ))
                    num_of_bytes = std::stoi( input3 );
                else
                    break;
                
                ret = file_seek(fd, num_of_bytes);

                if (ret >= 0)
                {
                    cout << "\nThe new location of the file pointer in descriptor " << fd << " is " << ret << "\n" << endl;
                }
                else
                {
                    cout << "\nThe seek operation has failed\n" << endl;
                }

                break;

            /**
             * CLOSE FILE
             */ 
            case 12:
                if (!check_if_mounted())
                    break;

                cout << "Enter a file descriptor to close: ";
                std::getline(std::cin, input2);

                if (is_number( input2 ))
                    fd = std::stoi( input2 );
                else
                    break;

                ret = file_close(fd);
                
                if (ret != -1)
                    cout << "\nThe file descriptor " << ret << " has been closed." << "\n" << endl;
                else
                    cout << "\nThe file failed to close\n" << endl;

                break;

            /**
             * RECOVER A DELETED FILE
             */ 
            case 13:
                if (!check_if_mounted())
                    break;

                cout << "Enter a path to a deleted file: ";
                std::getline(std::cin, input2);
                ret = file_ununlink(input2);

                if (ret != -1)
                    cout << "\n" << input2 << " was successfully recovered\n" << endl;
                else
                    cout << "\n" << input2 << " could not be recovered\n" << endl;

                break;

            /**
             * EXIT PROGRAM
             */ 
            case 14:
                cout << "\nExiting the program" << endl;
                promptUser = false;
                break;

            /**
             * PRINT FILE TABLE
             */ 
            case 15:
                if (!check_if_mounted())
                    break;

		        print_open_file_table();
		        break;

            default:
                cout << "\nInvalid Input\n" << endl;
                promptUser = true;
                break;
        }

    }

    delete myFilesys;

}
