#include "header.hpp"
// ------------------------------------------------------------
// PEEK CSV
// ------------------------------------------------------------
// This function opens a CSV file and only reads the first two rows:
//   1. the header row
//   2. the first data row
//
// It then prints each column index, the column name, and a sample
// value from the second row so the user can preview the file layout.
// ------------------------------------------------------------
void peekCSV(const string& filename)
{
    // Check if the user gave an empty file name
    if (filename.empty())
    {
        cout << "Error: No file provided." << endl;
        return;
    }

    // Open the file for reading
    ifstream infile(filename);

    // Make sure the file opened correctly
    if (!infile.is_open())
    {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }

    string headerLine;   // holds the first row (column names)
    string sampleLine;   // holds the second row (sample data)

    // Read the first row of the file
    if (!getline(infile, headerLine))
    {
        cout << "Error: File is empty." << endl;
        return;
    }

    // Read the second row of the file
    if (!getline(infile, sampleLine))
    {
        cout << "Error: File does not contain a sample data row." << endl;
        return;
    }

    vector<string> headers;       // stores column names
    vector<string> sampleValues;  // stores the sample row values
    string field;                 // temporary string for splitting

    // Split the header row by commas
    stringstream headerStream(headerLine);
    while (getline(headerStream, field, ','))
    {
        headers.push_back(field);
    }

    // Split the sample row by commas
    stringstream sampleStream(sampleLine);
    while (getline(sampleStream, field, ','))
    {
        sampleValues.push_back(field);
    }

    // Print each column number, header name, and sample value
    for (int i = 0; i < static_cast<int>(headers.size()); i++)
    {
        cout << "Col " << i << " " << headers[i] << " ";

        // If the sample row has a value for this column, print it
        if (i < static_cast<int>(sampleValues.size()))
        {
            cout << sampleValues[i];
        }
        else
        {
            // If the row is missing a value, show that clearly
            cout << "(missing)";
        }

        cout << endl;
    }

    infile.close();
}

// ------------------------------------------------------------
// PARSE ONE ROW
// ------------------------------------------------------------
// This helper function takes one CSV line and splits it by commas.
// It returns a Record struct whose fields vector holds all values
// from that row.
// ------------------------------------------------------------
Record parseRow(const string &line)
{
    Record record;       // this will store one full CSV row
    stringstream ss(line);
    string value;        // one field at a time

    // Split the line by commas and store each field
    while (getline(ss, value, ','))
    {
        record.fields.push_back(value);
    }

    return record;
}

// ------------------------------------------------------------
// UPLOAD CSV
// ------------------------------------------------------------
// This function reads the whole CSV file, finds the chosen primary
// key column, parses every row, checks for duplicate keys, and
// inserts each record into the B+ tree.
//
// Parameters:
//   filename   = CSV file to load
//   primaryCol = name of the primary key column
//   tree       = B+ tree to insert records into
// ------------------------------------------------------------
void uploadCSV(const string& filename, const string& primaryCol, BPlusTree& tree)
{
    // Make sure the user gave a file name
    if (filename.empty())
    {
        cout << "Error: No file provided." << endl;
        return;
    }

    // Open the CSV file
    ifstream infile(filename);

    // Check that the file opened successfully
    if (!infile.is_open())
    {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }

    string line;

    // Read the header row first
    if (!getline(infile, line))
    {
        cout << "Error: File is empty." << endl;
        return;
    }

    vector<string> headers;      // stores the header names
    stringstream headerStream(line);
    string field;

    // Split the header row into separate column names
    while (getline(headerStream, field, ','))
    {
        headers.push_back(field);
    }

    // Find which column index matches the chosen primary key name
    int primaryIndex = -1;
    for (int i = 0; i < static_cast<int>(headers.size()); i++)
    {
        if (headers[i] == primaryCol)
        {
            primaryIndex = i;
            break;
        }
    }

    // If we never found the primary key column, stop
    if (primaryIndex == -1)
    {
        cout << "Error: Primary column not found." << endl;
        return;
    }

    int rowNumber = 1; // start counting rows; header is row 1

    // Read the rest of the file one row at a time
    while (getline(infile, line))
    {
        rowNumber++;

        // Convert the raw line into a Record object
        Record record = parseRow(line);

        // Make sure this row actually has the primary key field
        if (primaryIndex >= static_cast<int>(record.fields.size()))
        {
            cout << "Skipping bad row " << rowNumber << endl;
            continue;
        }

        // Extract the primary key value from the row
        string key = record.fields[primaryIndex];

        // Check if this key already exists in the tree
        // Since primary keys must be unique, stop if a duplicate is found
        if (tree.search(key) != nullptr)
        {
            cout << "Error: Duplicate key '" << key
                 << "' found at row " << rowNumber << endl;
            return;
        }

        // Insert the row into the B+ tree using the primary key
        tree.insert(key, record);
    }

    infile.close();
    cout << "Upload complete for file: " << filename << endl;
}

// ------------------------------------------------------------
// FIND COLUMN INDEX
// ------------------------------------------------------------
// This helper searches through the header names and returns the
// index of the matching column name.
//
// Returns:
//   index if found
//   -1 if not found
// ------------------------------------------------------------
int findColumnIndex(const vector<string>& headers, const string& columnName)
{
    for (int i = 0; i < static_cast<int>(headers.size()); i++)
    {
        if (headers[i] == columnName)
        {
            return i;
        }
    }

    return -1;
}

// ------------------------------------------------------------
// PARSE HEADER
// ------------------------------------------------------------
// This helper splits a CSV header line into a vector of column names.
// ------------------------------------------------------------
vector<string> parseHeader(const string &line)
{
    vector<string> headers;
    stringstream headerStream(line);
    string field;

    while (getline(headerStream, field, ','))
    {
        headers.push_back(field);
    }

    return headers;
}

// ------------------------------------------------------------
// MAKE INDEX FILE NAME
// ------------------------------------------------------------
// This function converts a CSV filename into the required primary
// index filename format.
//
// Example:
//   "uszips.csv" -> "uszips_pi.db"
//   "amazon.csv" -> "amazon_pi.db"
// ------------------------------------------------------------
string makeIndexFilename(const string &csvFilename)
{
    size_t dotPos = csvFilename.find_last_of('.');
    string baseName;

    // If there is no file extension, use the whole name
    if (dotPos == string::npos)
    {
        baseName = csvFilename;
    }
    else
    {
        // Otherwise remove the extension
        baseName = csvFilename.substr(0, dotPos);
    }

    return baseName + "_pi.db";
}

// ------------------------------------------------------------
// PRINT RECORD
// ------------------------------------------------------------
// This function prints out all fields in a record on one line.
// It separates the values with commas for readability.
// ------------------------------------------------------------
void printRecord(const Record &record)
{
    for (int i = 0; i < static_cast<int>(record.fields.size()); i++)
    {
        cout << record.fields[i];

        // Print comma only between fields, not after the last one
        if (i < static_cast<int>(record.fields.size()) - 1)
        {
            cout << ", ";
        }
    }
    cout << endl;
}

// ------------------------------------------------------------
// PRINT HELP
// ------------------------------------------------------------
// Displays all available commands for the user.
// ------------------------------------------------------------
void printHelp()
{
    cout << "Available commands:" << endl;
    cout << "  peek <filename.csv>                - Preview the structure of a CSV file" << endl;
    cout << "  upload <filename.csv> <primaryCol> - Upload a CSV file into the B+ tree index" << endl;
    cout << "  find <filename.csv> <keyValue>     - Find and display a record by primary key" << endl;
    cout << "  help                               - Show this help message" << endl;
    cout << "  exit                               - Exit the application" << endl;
}

// ------------------------------------------------------------
// HANDLE PEEK
// ------------------------------------------------------------
// Wrapper function for the PEEK command.
// Calls peekCSV using the provided file name.
// ------------------------------------------------------------
void handlePeek(const string &filename)
{
    peekCSV(filename);
}

// ------------------------------------------------------------
// HANDLE UPLOAD
// ------------------------------------------------------------
// Wrapper function for the UPLOAD command.
// Creates a B+ tree, uploads the CSV into it, and then serializes
// the finished index to disk.
// ------------------------------------------------------------
void handleUpload(const string &filename, const string &primaryCol)
{
    // Create a B+ tree of order 3
    BPlusTree tree(3);

    // Read the file and insert all records into the tree
    uploadCSV(filename, primaryCol, tree);

    // Build the output index file name
    string indexFilename = makeIndexFilename(filename);

    // Save the tree to disk
    tree.serialize(indexFilename);

    cout << "Index saved to " << indexFilename << endl;
}

// ------------------------------------------------------------
// HANDLE FIND
// ------------------------------------------------------------
// Wrapper function for the FIND command.
// Loads an index file from disk, searches for the given key, and
// prints the matching record if found.
// ------------------------------------------------------------
void handleFind(const string &csvName, const string &keyValue)
{
    // Convert CSV name into the saved index filename
    string indexFilename = makeIndexFilename(csvName);

    // Create a B+ tree and load the saved index from disk
    BPlusTree tree(3);
    tree.deserialize(indexFilename);

    // Search for the requested key
    Record* record = tree.search(keyValue);

    // Print the record if found
    if (record != nullptr)
    {
        printRecord(*record);
    }
    else
    {
        cout << "Record not found for key: " << keyValue << endl;
    }
}

// ------------------------------------------------------------
// RUN APP
// ------------------------------------------------------------
// This function runs the main menu loop for the whole program.
// It repeatedly shows the menu, gets the user's choice, and calls
// the correct handler function.
//
// Menu choices:
//   1 = Peek CSV
//   2 = Upload CSV
//   3 = Find record
//   4 = Help
//   5 = Exit
// ------------------------------------------------------------
void run_app()
{
    int choice = 0;

    // Keep looping until the user chooses to exit
    do
    {
        cout << "\n========== MENU ==========" << endl;
        cout << "1. Peek CSV file" << endl;
        cout << "2. Upload CSV file" << endl;
        cout << "3. Find record by key" << endl;
        cout << "4. Help" << endl;
        cout << "5. Exit" << endl;
        cout << "Enter your choice: ";

        cin >> choice;

        // If the user entered invalid input such as text instead of a number
        if (cin.fail())
        {
            cout << "Invalid input. Please enter a number from 1 to 5." << endl;

            cin.clear(); // reset the fail state
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input
            continue;
        }

        // Clear the leftover newline before using getline later
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        // Use switch to handle each menu choice
        switch (choice)
        {
            case 1:
            {
                string filename;
                cout << "Enter CSV filename: ";
                getline(cin, filename);
                handlePeek(filename);
                break;
            }

            case 2:
            {
                string filename, primaryCol;
                cout << "Enter CSV filename: ";
                getline(cin, filename);

                cout << "Enter primary key column name: ";
                getline(cin, primaryCol);

                handleUpload(filename, primaryCol);
                break;
            }

            case 3:
            {
                string csvName, keyValue;
                cout << "Enter CSV base name (without extension): ";
                getline(cin, csvName);

                cout << "Enter key value: ";
                getline(cin, keyValue);

                handleFind(csvName, keyValue);
                break;
            }

            case 4:
            {
                printHelp();
                break;
            }

            case 5:
            {
                cout << "Goodbye!" << endl;
                break;
            }

            default:
            {
                cout << "Invalid choice. Please enter a number from 1 to 5." << endl;
                break;
            }
        }

    } while (choice != 5);
}