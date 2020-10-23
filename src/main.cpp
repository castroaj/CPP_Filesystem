#include "../hdr/main.h"
#include "../hdr/routines.h"
#include "../hdr/file.h"
#include "../hdr/dir.h"

void promptUserWithMenu()
{
    using namespace std;
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


int main(int argc, char* argv[])
{
    using namespace std;

    bool promptUser = true;
    std::string input;
    std::string fileName;\

    cout << "Filesystem Menu" << endl;

    while (promptUser)
    {
        promptUserWithMenu();

        cout << "Please Select a option: ";
        std::getline(std::cin, input);

        int inputInt = -1;

        if (is_number(input))
            inputInt = std::stoi( input );

        switch (inputInt)
        {
            case 1:
                cout << "Please enter a filename for the new filesystem: ";
                std::getline(std::cin, fileName);
                format(fileName);
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
            case 5:
                break;
            case 6:
                break;
            case 7:
                break;
            case 8:
                break;
            case 9:
                break;
            case 10:
                break;
            case 11:
                break;
            case 12:
                break;
            case 13:
                break;
            case 14:
                cout << "\nExiting the program" << endl;
                promptUser = false;
                break;
            default:
                cout << "\nInvalid Input" << endl;
                promptUser = true;
                break;
        }

    }
}