#include "uscity.hpp"
struct Record
{
    vector<string> fields;
};

class BPlusTreeNode
{
private:
    bool isLeaf;                          // true if this node is a leaf
    vector<string> keys;                  // keys stored in the node

    // For internal nodes
    vector<BPlusTreeNode*> children;      // child pointers

    // For leaf nodes
    vector<Record> records;               // full records associated with keys

    BPlusTreeNode* parent;                // pointer to parent node
    BPlusTreeNode* next;                  // next leaf node

public:
    BPlusTreeNode(bool leaf){
        isLeaf = leaf;
        parent = nullptr;
        next = nullptr;
    }

    // Getters
    bool getIsLeaf() const{
        return isLeaf;
    }
    vector<string>& getKeys(){
        return keys;
    }
    vector<BPlusTreeNode*>& getChildren(){
        return children;
    }
    vector<Record>& getRecords(){
        return records;
    }
    BPlusTreeNode* getParent() const{
        return parent;
    }
    BPlusTreeNode* getNext() const{
        return next;
    }

    // Setters
    void setParent(BPlusTreeNode* p){
        parent = p;
    }

    void setNext(BPlusTreeNode* n){
        next = n;
    }
};

class BPlusTree
{
private:
    BPlusTreeNode* root;     // pointer to the root of the whole tree
    int maxKeys;             // maximum number of keys a node can hold before splitting

    // Helper functions used internally by the tree
    BPlusTreeNode* findLeaf(const string& key);
    void insertIntoLeaf(BPlusTreeNode* leaf, const string& key, const Record& record);
    void splitLeaf(BPlusTreeNode* leaf);
    void insertIntoParent(BPlusTreeNode* leftNode, const string& promotedKey, BPlusTreeNode* rightNode);
    void splitInternal(BPlusTreeNode* node);

public:
    // Constructor
    BPlusTree(int order);

    // Main public operations
    void insert(const string& key, const Record& record);
    Record* search(const string& key);

    // Save/load index file later
    void serialize(const string& dbFilename);
    void deserialize(const string& dbFilename);

    // Getter for root if needed for debugging
    BPlusTreeNode* getRoot() const;
};