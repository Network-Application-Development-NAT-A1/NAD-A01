#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// LoadFile function: Reads a file with the specified filename and returns it as a string.
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

// SaveFile function: Saves the string data to a file with the specified filename.
void SaveFile(const string& filename, string& data)
{
    ofstream file(filename);
    if (!file)
    {
        cout << "Unable to open file for writing: " << filename << endl;
        return;
    }

    file << data; // Write string data to a file.
    file.close();
}