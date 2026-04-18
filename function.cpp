#include "header.hpp"
void parse(const string& filename) {
    ifstream infile(filename);

    if(filename.empty()) {
        cout << "Error: No file provided." << endl;
        return;
    }

}