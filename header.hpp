#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>


using namespace std;

void parse(const string& filename, const string& primaryCol);

void peek(const string& filename);

void search(const string& filename, const string& primaryCol, const string& key);

void findRow(const string& filename, const string& primaryCol, const string& key);

void findColumn(const string& filename, const string& columnName);

void uploadCSV(const string& filename, const string& primaryCol);

BPlusTreeNode* findLeaf(const string& key);

void insert(const string& filename, const string& primaryCol, const vector<string>& newRecord);


