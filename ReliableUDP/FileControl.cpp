#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// LoadFile function: Reads a file with the specified filename 
// and returns it as a string.
void LoadFile(const string& filename, string& data)
{
    ifstream file(filename);
    if (!file)
    {
        cout << "Unable to open file: " << filename << endl;
        // Set data to an empty string when the file cannot be opened
        data = "";
        return;
    }

    string line;
    data = "";
    while (getline(file, line))
    {
        data += line + "\n";
    }

    file.close();
}