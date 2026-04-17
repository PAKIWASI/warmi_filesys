#ifndef TREE_NODE_H
#define TREE_NODE_H

#include <stdint.h>
#include <stdbool.h>


#define MAX_FILE_SIZE 4096  // 4KB
#define MAX_NAME_SIZE 128   // bytes
#define MAX_CHILDREN  256   // 256 subdirectories/files possible for each node


// tree node types
typedef enum {
    // a directory/folder
    // contains other branches, leafs
    NODE_BRANCH,
    // a file
    // represents the edges (leaves) of the tree
    NODE_LEAF
} NODE_TYPE;


// Defining a Node in the Tree
typedef struct Tree_Node Tree_Node;

struct Tree_Node {
    // the type of this node
    NODE_TYPE type;
    // name of file/dir
    char* name;
    // depending on type, one of these two values
    // will be active - the file or the dir
    union {
        // when type file
        struct {
            char* data;     // file data of size MAX_FILE_SIZE
            uint32_t size;  // current size of file
            uint32_t pos;   // pos in file (like a cursor)
        } file;
        // when type directory
        struct {
            Tree_Node** children;   // pointer to tree nodes array
            uint32_t num_children;  // no of children
        } dir;
    };
};


// create a new node
Tree_Node* tree_node_create(const char* name, NODE_TYPE type);

// destroy a node. if it's a dir, recursivly delete subtree
void tree_node_destroy(Tree_Node* tn);


// file/ dir creation/deletion

// create new file/directory as a child of this node
Tree_Node* tree_node_create_child(Tree_Node* tn, const char* name, NODE_TYPE type);

// make node 'child' a child of node 'parent'
Tree_Node* tree_node_append_node(Tree_Node* parent, Tree_Node* child);

// delete the child node and set it's parent's slot to NULL
bool tree_node_delete_child(Tree_Node* parent, Tree_Node* child);

bool tree_node_delete_child_by_name(Tree_Node* tn, const char* name);


// file read/write

// print the file content to console
void tree_node_read_file(Tree_Node* tn, uint32_t start, uint32_t size);

// write at the current cursor postition of overwrite
char* tree_node_write_file(Tree_Node* tn, const char* data, uint32_t size, bool overwrite);


// utilites

uint32_t tree_node_find_child(Tree_Node* parent, Tree_Node* child);

uint32_t tree_node_find_child_by_name(Tree_Node* parent, const char* name);


#endif // TREE_NODE_H
