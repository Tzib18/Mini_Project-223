#include "BplusTree.hpp"

// ============ helper functions ============
void BPlusTree::insertIntoLeaf(BPlusTreeNode* leaf, const string& key, const Record& record){
    vector<string>& keys = leaf->getKeys();
    vector<Record>& records = leaf->getRecords();

    // Find the correct position to insert the new key
    auto it = upper_bound(keys.begin(), keys.end(), key);
    int index = distance(keys.begin(), it);

    // Insert the key and record at the found position
    keys.insert(it, key);
    records.insert(records.begin() + index, record);
}

void BPlusTree::splitLeaf(BPlusTreeNode* leaf){

    if(leaf == nullptr || !leaf->getIsLeaf())
    {
        return; // only split leaf nodes
    }
    int midIndex = leaf->getKeys().size() / 2; // find the middle index for splitting
    BPlusTreeNode* newLeaf = new BPlusTreeNode(true); // create a new leaf node
    vector<string>& leafKeys = leaf->getKeys();
    vector<Record>& leafRecords = leaf->getRecords();
    vector<string>& newLeafKeys = newLeaf->getKeys();
    vector<Record>& newLeafRecords = newLeaf->getRecords();

    // Move the second half of the keys and records to the new leaf
    newLeafKeys.assign(leafKeys.begin() + midIndex, leafKeys.end());
    newLeafRecords.assign(leafRecords.begin() + midIndex, leafRecords.end());

    // Erase the moved keys and records from the original leaf
    leafKeys.erase(leafKeys.begin() + midIndex, leafKeys.end());
    leafRecords.erase(leafRecords.begin() + midIndex, leafRecords.end());

    // Update the next pointers for the leaf nodes
    newLeaf->setNext(leaf->getNext());
    leaf->setNext(newLeaf);

    // Insert the new leaf into the parent node
    string promotedKey = newLeafKeys.front(); // the first key of the new leaf will be promoted to the parent
    insertIntoParent(leaf, promotedKey, newLeaf);
}

void BPlusTree::insertIntoParent(BPlusTreeNode *leftNode, const string &promotedKey, BPlusTreeNode *rightNode)
{
    if (leftNode->getParent() == nullptr)
    {
        BPlusTreeNode* newRoot = new BPlusTreeNode(false);

        newRoot->getKeys().push_back(promotedKey);
        newRoot->getChildren().push_back(leftNode);
        newRoot->getChildren().push_back(rightNode);

        leftNode->setParent(newRoot);
        rightNode->setParent(newRoot);

        root = newRoot;
        return;
    }

    BPlusTreeNode* parent = leftNode->getParent();
    vector<string>& parentKeys = parent->getKeys();
    vector<BPlusTreeNode*>& parentChildren = parent->getChildren();

    // Find where leftNode is in the parent's children
    int childIndex = 0;
    while (childIndex < static_cast<int>(parentChildren.size()) &&
           parentChildren[childIndex] != leftNode)
    {
        childIndex++;
    }

    // Insert promoted key at the matching key position
    parentKeys.insert(parentKeys.begin() + childIndex, promotedKey);

    // Insert rightNode immediately after leftNode
    parentChildren.insert(parentChildren.begin() + childIndex + 1, rightNode);

    rightNode->setParent(parent);

    if (static_cast<int>(parentKeys.size()) > maxKeys)
    {
        splitInternal(parent);
    }
}

void BPlusTree::splitInternal(BPlusTreeNode *node)
{
    // Only split valid internal nodes
    if (node == nullptr || node->getIsLeaf())
    {
        return;
    }

    vector<string>& keys = node->getKeys();
    vector<BPlusTreeNode*>& children = node->getChildren();

    // Find the middle key to promote
    int midIndex = static_cast<int>(keys.size()) / 2;
    string promotedKey = keys[midIndex];

    // Create the new internal node
    BPlusTreeNode* newInternal = new BPlusTreeNode(false);

    vector<string>& newKeys = newInternal->getKeys();
    vector<BPlusTreeNode*>& newChildren = newInternal->getChildren();

    // Move keys AFTER the promoted key into the new internal node
    newKeys.assign(keys.begin() + midIndex + 1, keys.end());

    // Move the matching children into the new internal node
    newChildren.assign(children.begin() + midIndex + 1, children.end());

    // Update parent pointers of moved children
    for (int i = 0; i < static_cast<int>(newChildren.size()); i++)
    {
        if (newChildren[i] != nullptr)
        {
            newChildren[i]->setParent(newInternal);
        }
    }

    // Remove promoted key and moved keys from original node
    keys.erase(keys.begin() + midIndex, keys.end());

    // Remove moved children from original node
    children.erase(children.begin() + midIndex + 1, children.end());

    // Set new internal node's parent
    newInternal->setParent(node->getParent());

    // Insert promoted key into the parent
    insertIntoParent(node, promotedKey, newInternal);
}

BPlusTreeNode* BPlusTree::findLeaf(const string& key){
    if(root == nullptr)
    {
        return nullptr; // tree is empty
    }
    BPlusTreeNode* current = root;// this will be used to traverse the tree, starting at the root
    while(!current->getIsLeaf()) // while we are not at a leaf node
    {
        vector<string>& keys = current->getKeys();
        vector<BPlusTreeNode*>& children = current->getChildren();

        // Find the correct child to follow
        int i = 0;
        while(i < static_cast<int>(keys.size()) && key >= keys[i])
        {
            i++;
        }
        current = children[i]; // move down to the child node
    }
    return current; // return the leaf node where the key should be
}

void BPlusTree::destroyTree(BPlusTreeNode *node)
{
    // empty check
    if(node == nullptr)
    {
        return;
    }

    // Recursively destroy all child subtrees
    vector<BPlusTreeNode*>& children = node->getChildren();
    for (int i = 0; i < static_cast<int>(children.size()); i++)
    {
        destroyTree(children[i]);
    }

    // Delete the current node after its children are gone
    delete node;
}


// ============ main public functions ============
void BPlusTree::insert(const string &key, const Record &record)
{

    if(root == nullptr)
    {
        root = new BPlusTreeNode(true); // create a new leaf node
        insertIntoLeaf(root, key, record); // stores key and record in the leaf
    }
    else
    {
        BPlusTreeNode* leaf = findLeaf(key); // this fimds the correct leaf 
        insertIntoLeaf(leaf, key, record); // this puts the key and record into the leaf 
    }

    if(root->getKeys().size() > maxKeys) // if the root node has too many keys, we need to split it
    {
        splitInternal(root); // this will split the root node and create a new root if necessary
    }
}

Record *BPlusTree::search(const string &key)
{
        BPlusTreeNode* leaf = findLeaf(key); // find the correct leaf node for the key
        if(leaf == nullptr)
        {
            return nullptr; // key not found
        }
        vector<string>& keys = leaf->getKeys();
        vector<Record>& records = leaf->getRecords();
    return nullptr;
}

void BPlusTree::serialize(const string &dbFilename)
{
    ofstream outFile(dbFilename);

    if (!outFile.is_open())
    {
        cout << "Error: Could not open file " << dbFilename << " for writing." << endl;
        return;
    }

    // If tree is empty, nothing to save
    if (root == nullptr)
    {
        outFile.close();
        return;
    }

    // Start at the leftmost leaf
    BPlusTreeNode* current = root;
    while (!current->getIsLeaf())
    {
        current = current->getChildren()[0];
    }

    // Walk through all leaf nodes using next pointers
    while (current != nullptr)
    {
        vector<string>& keys = current->getKeys();
        vector<Record>& records = current->getRecords();

        for (int i = 0; i < static_cast<int>(keys.size()); i++)
        {
            // Write key first
            outFile << keys[i];

            // Then write all fields in the record
            for (int j = 0; j < static_cast<int>(records[i].fields.size()); j++)
            {
                outFile << "|" << records[i].fields[j];
            }

            outFile << endl;
        }

        current = current->getNext();
    }
    outFile.close();
}

void BPlusTree::deserialize(const string &dbFilename)
{
     // Open the file where the index will be saved
    ofstream outFile(dbFilename);

    // Make sure the file opened successfully
    if (!outFile.is_open())
    {
        cout << "Error: Could not open file " << dbFilename << " for writing." << endl;
        return;
    }

    // If the tree is empty, there is nothing to save
    if (root == nullptr)
    {
        outFile.close();
        return;
    }

    // Move down to the leftmost leaf node
    BPlusTreeNode* current = root;
    while (!current->getIsLeaf())
    {
        current = current->getChildren()[0];
    }

    // Traverse all leaf nodes using the next pointer
    while (current != nullptr)
    {
        vector<string>& keys = current->getKeys();
        vector<Record>& records = current->getRecords();

        // Write each key and its matching record to the file
        for (int i = 0; i < static_cast<int>(keys.size()); i++)
        {
            // Write the key first
            outFile << keys[i];

            // Then write all fields from the record separated by '|'
            for (int j = 0; j < static_cast<int>(records[i].fields.size()); j++)
            {
                outFile << "|" << records[i].fields[j];
            }

            // End this record on a new line
            outFile << endl;
        }

        // Move to the next leaf node
        current = current->getNext();
    }

    // Close the file when finished
    outFile.close();
}

