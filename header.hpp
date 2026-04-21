#include "BplusTree.hpp"

void peekCSV(const string& filename);
void uploadCSV(const string& filename, const string& primaryCol, BPlusTree& tree);
Record parseRow(const string& line);
int findColumnIndex(const vector<string>& headers, const string& columnName);

vector<string> parseHeader(const string& line);
string makeIndexFilename(const string& csvFilename);
void printRecord(const Record& record);
void printHelp();


void handlePeek(const string& filename);
void handleUpload(const string& filename, const string& primaryCol);
void handleFind(const string& csvName, const string& keyValue);
void run_app();