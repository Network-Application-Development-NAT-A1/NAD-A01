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

// VerifyFileContent function: 
// Verifies that the contents of the specified file match the given string.
bool VerifyFileContent(string filename, string expectedContent)
{
    string fileContent;
    LoadFile(filename, fileContent);
    // Compare what it read with what it expect
    return fileContent == expectedContent;
}

// TestFileValidation function: 
// Tests the ability to save and load files.
bool TestFileValidation(string filename, string testData)
{
    SaveFile(filename, testData); // Saving test data to a file
    string loadedData;
    LoadFile(filename, loadedData);
    return loadedData == testData; // Verify that the loaded data matches the source
}