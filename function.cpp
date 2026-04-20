#include "BplusTree.hpp"

void parse(const string& filename, const string& primaryCol){
    ifstream infile(filename);

    if (filename.empty())
    {
        cout << "Error: No file provided." << endl;
        return;
    }

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
    for (int i = 0; i < headers.size(); i++)
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

        stringstream ss(line);
        string value;
        Record record;

        while (getline(ss, value, ','))
        {
            record.fields.push_back(value);
        }

        // Make sure the row has enough fields
        if (primaryIndex >= record.fields.size())
        {
            cout << "Skipping bad row " << rowNumber << endl;
            continue;
        }

        string key = record.fields[primaryIndex];

        // Later:
        // check duplicate key
        // insert into B+ tree
        cout << "Key: " << key << endl;
    }

    infile.close();
}

void peekCSV(const string& filename){
    // Open the file
    ifstream infile(filename);

    // Check for empty filename
    if (filename.empty())
    {
        cout << "Error: No file provided." << endl;
        return;
    }

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
    for (int i = 0; i < headers.size(); i++)
    {
        cout << "Col " << i << " " << headers[i] << " ";

        if (i < sampleValues.size())
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


