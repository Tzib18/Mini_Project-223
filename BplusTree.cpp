#include "BplusTree.hpp"

// ============ helper functions ============

// ------------------------------------------------------------
// INSERT INTO LEAF
// ------------------------------------------------------------
// This helper inserts a new key and its matching record into
// a leaf node in sorted order.
//
// Steps:
// 1. Get references to the leaf's keys and records vectors
// 2. Use upper_bound to find the correct sorted position
// 3. Insert the key into the keys vector
// 4. Insert the matching record into the records vector at
//    the same position so they stay aligned
// ------------------------------------------------------------
void BPlusTree::insertIntoLeaf(BPlusTreeNode* leaf, const string& key, const Record& record)
{
    // Get direct access to the leaf's key list and record list
    vector<string>& keys = leaf->getKeys();
    vector<Record>& records = leaf->getRecords();

    // Find the correct position to keep the keys sorted
    auto it = upper_bound(keys.begin(), keys.end(), key);

    // Convert the iterator position into an integer index
    int index = distance(keys.begin(), it);

    // Insert the key into the sorted key list
    keys.insert(it, key);

    // Insert the matching record at the same index
    records.insert(records.begin() + index, record);
}

// ------------------------------------------------------------
// SPLIT LEAF
// ------------------------------------------------------------
// This helper splits a leaf node when it has too many keys.
//
// Steps:
// 1. Make sure the given node is a valid leaf
// 2. Find the middle index
// 3. Create a new leaf node
// 4. Move the second half of keys/records into the new leaf
// 5. Remove those keys/records from the old leaf
// 6. Fix the linked-list pointer between leaves
// 7. Promote the first key of the new leaf into the parent
// ------------------------------------------------------------
void BPlusTree::splitLeaf(BPlusTreeNode* leaf)
{
    // Only leaf nodes should be split by this function
    if (leaf == nullptr || !leaf->getIsLeaf())
    {
        return;
    }

    // Find the midpoint for the split
    int midIndex = leaf->getKeys().size() / 2;

    // Create the new leaf node
    BPlusTreeNode* newLeaf = new BPlusTreeNode(true);

    // Get references to the old leaf's keys and records
    vector<string>& leafKeys = leaf->getKeys();
    vector<Record>& leafRecords = leaf->getRecords();

    // Get references to the new leaf's keys and records
    vector<string>& newLeafKeys = newLeaf->getKeys();
    vector<Record>& newLeafRecords = newLeaf->getRecords();

    // Move the second half of keys into the new leaf
    newLeafKeys.assign(leafKeys.begin() + midIndex, leafKeys.end());

    // Move the second half of records into the new leaf
    newLeafRecords.assign(leafRecords.begin() + midIndex, leafRecords.end());

    // Remove the moved keys from the original leaf
    leafKeys.erase(leafKeys.begin() + midIndex, leafKeys.end());

    // Remove the moved records from the original leaf
    leafRecords.erase(leafRecords.begin() + midIndex, leafRecords.end());

    // Link the new leaf into the leaf chain
    newLeaf->setNext(leaf->getNext());
    leaf->setNext(newLeaf);

    // The first key of the new right leaf gets promoted upward
    string promotedKey = newLeafKeys.front();

    // Insert the promoted key into the parent
    insertIntoParent(leaf, promotedKey, newLeaf);
}

// ------------------------------------------------------------
// INSERT INTO PARENT
// ------------------------------------------------------------
// This helper is used after a split. It inserts a promoted key
// into the parent node and links the new right child.
//
// Cases:
// 1. If the split node had no parent, create a new root
// 2. Otherwise insert the promoted key into the existing parent
// 3. If the parent becomes too large, split it too
// ------------------------------------------------------------
void BPlusTree::insertIntoParent(BPlusTreeNode* leftNode, const string& promotedKey, BPlusTreeNode* rightNode)
{
    // If there is no parent, the old node was the root
    if (leftNode->getParent() == nullptr)
    {
        // Create a new root as an internal node
        BPlusTreeNode* newRoot = new BPlusTreeNode(false);

        // Store the promoted key in the new root
        newRoot->getKeys().push_back(promotedKey);

        // The two split nodes become children of the new root
        newRoot->getChildren().push_back(leftNode);
        newRoot->getChildren().push_back(rightNode);

        // Update parent pointers
        leftNode->setParent(newRoot);
        rightNode->setParent(newRoot);

        // Update the tree's root pointer
        root = newRoot;
        return;
    }

    // Otherwise, use the existing parent
    BPlusTreeNode* parent = leftNode->getParent();
    vector<string>& parentKeys = parent->getKeys();
    vector<BPlusTreeNode*>& parentChildren = parent->getChildren();

    // Find where the left child is stored in the parent's child list
    int childIndex = 0;
    while (childIndex < static_cast<int>(parentChildren.size()) &&
           parentChildren[childIndex] != leftNode)
    {
        childIndex++;
    }

    // Insert the promoted key at the correct key position
    parentKeys.insert(parentKeys.begin() + childIndex, promotedKey);

    // Insert the new right child immediately after the left child
    parentChildren.insert(parentChildren.begin() + childIndex + 1, rightNode);

    // Update the new child's parent pointer
    rightNode->setParent(parent);

    // If the parent overflowed, split it too
    if (static_cast<int>(parentKeys.size()) > maxKeys)
    {
        splitInternal(parent);
    }
}

// ------------------------------------------------------------
// SPLIT INTERNAL
// ------------------------------------------------------------
// This helper splits an internal node when it has too many keys.
//
// Steps:
// 1. Make sure the node is valid and not a leaf
// 2. Find the middle key to promote
// 3. Create a new internal node
// 4. Move keys after the promoted key into the new node
// 5. Move the matching children into the new node
// 6. Update the moved children's parent pointers
// 7. Remove moved keys/children from the old node
// 8. Promote the middle key upward
// ------------------------------------------------------------
void BPlusTree::splitInternal(BPlusTreeNode* node)
{
    // Internal splits only work on internal nodes
    if (node == nullptr || node->getIsLeaf())
    {
        return;
    }

    // Access the original node's keys and children
    vector<string>& keys = node->getKeys();
    vector<BPlusTreeNode*>& children = node->getChildren();

    // Find the middle key that will be promoted upward
    int midIndex = static_cast<int>(keys.size()) / 2;
    string promotedKey = keys[midIndex];

    // Create the new internal node
    BPlusTreeNode* newInternal = new BPlusTreeNode(false);

    // Access the new internal node's storage
    vector<string>& newKeys = newInternal->getKeys();
    vector<BPlusTreeNode*>& newChildren = newInternal->getChildren();

    // Move keys after the promoted key into the new internal node
    newKeys.assign(keys.begin() + midIndex + 1, keys.end());

    // Move matching children into the new internal node
    newChildren.assign(children.begin() + midIndex + 1, children.end());

    // Update parent pointers for all children that were moved
    for (int i = 0; i < static_cast<int>(newChildren.size()); i++)
    {
        if (newChildren[i] != nullptr)
        {
            newChildren[i]->setParent(newInternal);
        }
    }

    // Remove the promoted key and all moved keys from the old node
    keys.erase(keys.begin() + midIndex, keys.end());

    // Remove the moved children from the old node
    children.erase(children.begin() + midIndex + 1, children.end());

    // Set the parent pointer of the new internal node
    newInternal->setParent(node->getParent());

    // Insert the promoted key into the parent
    insertIntoParent(node, promotedKey, newInternal);
}

// ------------------------------------------------------------
// FIND LEAF
// ------------------------------------------------------------
// This helper walks down the tree to find the leaf node where
// a given key belongs.
//
// Steps:
// 1. Start at the root
// 2. While the current node is not a leaf:
//    - compare the key to the node's keys
//    - choose the correct child pointer
// 3. Return the leaf node reached at the bottom
// ------------------------------------------------------------
BPlusTreeNode* BPlusTree::findLeaf(const string& key)
{
    // If the tree is empty, there is no leaf to return
    if (root == nullptr)
    {
        return nullptr;
    }

    // Start traversal at the root
    BPlusTreeNode* current = root;

    // Keep moving downward until we reach a leaf
    while (!current->getIsLeaf())
    {
        vector<string>& keys = current->getKeys();
        vector<BPlusTreeNode*>& children = current->getChildren();

        // Find which child branch the key belongs to
        int i = 0;
        while (i < static_cast<int>(keys.size()) && key >= keys[i])
        {
            i++;
        }

        // Move to the matching child
        current = children[i];
    }

    // Return the leaf node where the key belongs
    return current;
}

// ------------------------------------------------------------
// DESTROY TREE
// ------------------------------------------------------------
// This helper recursively frees all dynamically allocated nodes
// in the B+ tree.
//
// Steps:
// 1. Stop if the node is null
// 2. Recursively destroy all child subtrees
// 3. Delete the current node
// ------------------------------------------------------------
void BPlusTree::destroyTree(BPlusTreeNode* node)
{
    // Base case: nothing to delete
    if (node == nullptr)
    {
        return;
    }

    // Recursively destroy all child nodes first
    vector<BPlusTreeNode*>& children = node->getChildren();
    for (int i = 0; i < static_cast<int>(children.size()); i++)
    {
        destroyTree(children[i]);
    }

    // Delete the current node after its children are gone
    delete node;
}


// ============ main public functions ============

// ------------------------------------------------------------
// INSERT
// ------------------------------------------------------------
// This is the main public insert function for the B+ tree.
//
// Steps:
// 1. If the tree is empty, create the root as a leaf
// 2. Otherwise find the correct leaf for the key
// 3. Insert the key/record into that leaf
// 4. If the node overflows, split it
//
// Note:
// Right now your overflow check is only checking the root.
// Later you may want this logic to check the actual leaf that
// was inserted into instead of always checking root.
// ------------------------------------------------------------
void BPlusTree::insert(const string& key, const Record& record)
{
    // If tree is empty, create a new root leaf
    if (root == nullptr)
    {
        root = new BPlusTreeNode(true);
        insertIntoLeaf(root, key, record);
    }
    else
    {
        // Find the correct leaf for this key
        BPlusTreeNode* leaf = findLeaf(key);

        // Insert the key and record into that leaf
        insertIntoLeaf(leaf, key, record);
    }

    // If the root now has too many keys, split it
    if (static_cast<int>(root->getKeys().size()) > maxKeys)
    {
        splitInternal(root);
    }
}

// ------------------------------------------------------------
// SEARCH
// ------------------------------------------------------------
// This is the main public search function.
//
// Steps:
// 1. Find the leaf node where the key should be
// 2. If no leaf exists, return nullptr
// 3. Search through the leaf's keys
// 4. Return the matching record if found
// 5. Otherwise return nullptr
//
// Note:
// Your uploaded version is still unfinished and always returns
// nullptr. The version below includes the intended logic.
// ------------------------------------------------------------
Record* BPlusTree::search(const string& key)
{
    // Find the leaf where this key should be located
    BPlusTreeNode* leaf = findLeaf(key);

    // If no leaf was found, the tree is empty or key cannot exist
    if (leaf == nullptr)
    {
        return nullptr;
    }

    // Access the leaf's keys and records
    vector<string>& keys = leaf->getKeys();
    vector<Record>& records = leaf->getRecords();

    // Search through the leaf for the matching key
    for (int i = 0; i < static_cast<int>(keys.size()); i++)
    {
        if (keys[i] == key)
        {
            return &records[i];
        }
    }

    // Key was not found in this leaf
    return nullptr;
}

// ------------------------------------------------------------
// SERIALIZE
// ------------------------------------------------------------
// This function writes the contents of the B+ tree to disk.
//
// Current approach:
// 1. Open the output file
// 2. Move to the leftmost leaf
// 3. Walk through all leaves using the next pointer
// 4. Write each key and all record fields separated by '|'
//
// This saves a simple flattened representation of the tree's
// leaf contents rather than the exact pointer structure.
// ------------------------------------------------------------
void BPlusTree::serialize(const string& dbFilename)
{
    // Open the output file
    ofstream outFile(dbFilename);

    // Make sure the file opened correctly
    if (!outFile.is_open())
    {
        cout << "Error: Could not open file " << dbFilename << " for writing." << endl;
        return;
    }

    // If tree is empty, there is nothing to save
    if (root == nullptr)
    {
        outFile.close();
        return;
    }

    // Move to the leftmost leaf in the tree
    BPlusTreeNode* current = root;
    while (!current->getIsLeaf())
    {
        current = current->getChildren()[0];
    }

    // Walk through all leaf nodes using the linked list
    while (current != nullptr)
    {
        vector<string>& keys = current->getKeys();
        vector<Record>& records = current->getRecords();

        // Write every key-record pair in this leaf
        for (int i = 0; i < static_cast<int>(keys.size()); i++)
        {
            // Write the key first
            outFile << keys[i];

            // Then write all fields in the record
            for (int j = 0; j < static_cast<int>(records[i].fields.size()); j++)
            {
                outFile << "|" << records[i].fields[j];
            }

            // End the record on a new line
            outFile << endl;
        }

        // Move to the next leaf node
        current = current->getNext();
    }

    outFile.close();
}

// ------------------------------------------------------------
// DESERIALIZE
// ------------------------------------------------------------
// This function is supposed to load the tree back from disk.
//
// Important:
// Your current version is not yet a real deserialize function.
// Right now it opens an output file and writes data again, which
// makes it behave more like another serialize function.
//
// What it should eventually do instead:
// 1. Open the saved .db file for reading
// 2. Read each line
// 3. Split the line by '|'
// 4. Rebuild the Record
// 5. Insert the key/record back into a new tree
// ------------------------------------------------------------
void BPlusTree::deserialize(const string& dbFilename)
{
    ifstream inFile(dbFilename);

    if (!inFile.is_open())
    {
        cout << "Error: Could not open file " << dbFilename << " for reading." << endl;
        return;
    }

    string line;

    while (getline(inFile, line))
    {
        if (line.empty())
        {
            continue;
        }

        stringstream ss(line);
        string value;
        vector<string> parts;

        while (getline(ss, value, '|'))
        {
            parts.push_back(value);
        }

        if (parts.size() == 0)
        {
            continue;
        }

        string key = parts[0];

        Record record;

        for (int i = 1; i < static_cast<int>(parts.size()); i++)
        {
            record.fields.push_back(parts[i]);
        }

        insert(key, record);
    }

    inFile.close();
}