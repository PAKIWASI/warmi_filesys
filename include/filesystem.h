#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "tree_node.h"
#include <stdint.h>


// filesystem representation as tree
typedef struct {
    Tree_Node* root;        // the root of the filesystem
    Tree_Node* curr_dir;    // the current directory we are in
    // metadata about filesystem
    uint32_t total_files;
    uint32_t total_dirs;
    uint32_t depth;
} filesystem;


// create new file system with only root '/' dir
filesystem* filesystem_create(void);

// recursivly destroy whole filesystem tree
void filesystem_destroy(filesystem* fs);


// file/directory creation/deletion

Tree_Node* filesystem_create_file(filesystem* fs, const char* name);

void


#endif // FILESYSTEM_H
