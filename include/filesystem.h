#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "tree_node.h"
#include <stdint.h>


// filesystem representation as tree
typedef struct {
    Tree_Node* root;
    uint32_t total_files;
    uint32_t total_dirs;
    uint32_t depth;
} filesystem;


// create new file system with only root '/' dir
filesystem* filesystem_create(void);

// recursivly destroy whole filesystem tree
void filesystem_destroy(filesystem* fs);


// file/directory creation/deletion

void filesystem_create_file(filesystem* fs, const char* name);

#endif // FILESYSTEM_H
