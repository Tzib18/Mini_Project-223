#include "header.hpp"

void peekCSV(const string& filename)
{
    // Check for empty filename first
    if (filename.empty())
    {
        cout << "Error: No file provided." << endl;
        return;
    }

    // Open the file
    ifstream infile(filename);

    // Check if file opened correctly
    if (!infile.is_open())
    {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }

    string headerLine;
    string sampleLine;

    // Read the first row (headers)
    if (!getline(infile, headerLine))
    {
        cout << "Error: File is empty." << endl;
        return;
    }

    // Read the second row (sample data)
    if (!getline(infile, sampleLine))
    {
        cout << "Error: File does not contain a sample data row." << endl;
        return;
    }

    vector<string> headers;
    vector<string> sampleValues;
    string field;

    // Split header row by commas
    stringstream headerStream(headerLine);
    while (getline(headerStream, field, ','))
    {
        headers.push_back(field);
    }

    // Split sample row by commas
    stringstream sampleStream(sampleLine);
    while (getline(sampleStream, field, ','))
    {
        sampleValues.push_back(field);
    }

    // Print each column index, header, and sample value
    for (int i = 0; i < static_cast<int>(headers.size()); i++)
    {
        cout << "Col " << i << " " << headers[i] << " ";

        if (i < static_cast<int>(sampleValues.size()))
        {
            cout << sampleValues[i];
        }
        else
        {
            cout << "(missing)";
        }

        cout << endl;
    }

    infile.close();
}

Record parseRow(const string &line)
{
        Record record;
        stringstream ss(line);
        string value;
    
        while (getline(ss, value, ','))
        {
            record.fields.push_back(value);
        }
    return record;
}

void uploadCSV(const string& filename, const string& primaryCol, BPlusTree& tree)
{
    if (filename.empty())
    {
        cout << "Error: No file provided." << endl;
        return;
    }

    ifstream infile(filename);

    if (!infile.is_open())
    {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }

    string line;

    // Read header row
    if (!getline(infile, line))
    {
        cout << "Error: File is empty." << endl;
        return;
    }

    vector<string> headers;
    stringstream headerStream(line);
    string field;

    while (getline(headerStream, field, ','))
    {
        headers.push_back(field);
    }

    // Find the primary key column index
    int primaryIndex = -1;
    for (int i = 0; i < static_cast<int>(headers.size()); i++)
    {
        if (headers[i] == primaryCol)
        {
            primaryIndex = i;
            break;
        }
    }

    if (primaryIndex == -1)
    {
        cout << "Error: Primary column not found." << endl;
        return;
    }

    int rowNumber = 1; // header is row 1

    while (getline(infile, line))
    {
        rowNumber++;

        Record record = parseRow(line);

        if (primaryIndex >= static_cast<int>(record.fields.size()))
        {
            cout << "Skipping bad row " << rowNumber << endl;
            continue;
        }

        string key = record.fields[primaryIndex];

        // Check for duplicate primary key
        if (tree.search(key) != nullptr)
        {
            cout << "Error: Duplicate key '" << key
                 << "' found at row " << rowNumber << endl;
            return;
        }

        // Insert into the B+ tree
        tree.insert(key, record);
    }

    infile.close();
    cout << "Upload complete for file: " << filename << endl;
}


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

// ==========new helper functions to be made this is a break for me============
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

string makeIndexFilename(const string &csvFilename)
{
    size_t dotPos = csvFilename.find_last_of('.');
    string baseName;

    if (dotPos == string::npos)
    {
        baseName = csvFilename;
    }
    else
    {
        baseName = csvFilename.substr(0, dotPos);
    }

    return baseName + "_pi.db";
}

void printRecord(const Record &record)
{
    for (size_t i = 0; i < record.fields.size(); i++)
    {
        cout << record.fields[i];
        if (i < record.fields.size() - 1)
        {
            cout << ", ";
        }
    }
    cout << endl;
}

void printHelp()
{
    cout << "Available commands:" << endl;
    cout << "  peek <filename.csv>                - Preview the structure of a CSV file" << endl;
    cout << "  upload <filename.csv> <primaryCol> - Upload a CSV file into the B+ tree index" << endl;
    cout << "  find <filename.csv> <keyValue>     - Find and display a record by primary key" << endl;
    cout << "  help                               - Show this help message" << endl;
    cout << "  exit                               - Exit the application" << endl;
}

void handlePeek(const string &filename)
{
    peekCSV(filename);
}

void handleUpload(const string &filename, const string &primaryCol)
{
    BPlusTree tree(3); // create a B+ tree with order 3 (max 2 keys per node)
    uploadCSV(filename, primaryCol, tree);
    string indexFilename = makeIndexFilename(filename);
    tree.serialize(indexFilename);
    cout << "Index saved to " << indexFilename << endl;
}

void handleFind(const string &csvName, const string &keyValue)
{
    string indexFilename = makeIndexFilename(csvName);
    BPlusTree tree(3); // create a B+ tree with order 3 (max 2 keys per node)
    tree.deserialize(indexFilename);
    Record* record = tree.search(keyValue);

    if (record != nullptr)
    {
        printRecord(*record);
    }
    else
    {
        cout << "Record not found for key: " << keyValue << endl;
    }
}


void run_app()
{
    int choice = 0;

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

        // If user typed something that is not a number
        if (cin.fail())
        {
            cout << "Invalid input. Please enter a number from 1 to 5." << endl;

            cin.clear(); // remove fail state
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input
            continue;
        }

        // Remove leftover newline before using getline later
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

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