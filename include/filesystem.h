#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "tree_node.h"
#include <stdint.h>

// max recursive depth of filesystem
#define MAX_DEPTH 256


// filesystem representation as tree
typedef struct {
    // the root (starting point) of the filesystem
    Tree_Node* root;
    // stack of current directories indexed by depth
    // top of stack points to current directory we are in
    Tree_Node** curr_dirs;

    // metadata about filesystem
    uint32_t depth;
    uint32_t total_files;
    uint32_t total_dirs;
} filesystem;


// create new file system with only root '/' dir
filesystem* filesystem_create(void);

// recursivly destroy whole filesystem tree
void filesystem_destroy(filesystem* fs);


// filesystem navigation

// the "ls" command equivalent
bool filesystem_show_dir(filesystem* fs);

// the "cd" command equivalent
// supports ".." to go to parent
bool filesystem_change_dir(filesystem* fs, const char* dirname);

// TODO: how to go back to parent dir ? how will we set the filesystem.parent ?


// file/directory creation/deletion

// create file in current directory
Tree_Node* filesystem_create_file(filesystem* fs, const char* name);

// delete file in the current directory
bool filesystem_delete_file(filesystem* fs, const char* name);


// file reading/writing

// view a file in the current directory
bool filesystem_view_file(filesystem* fs, const char* name, uint32_t start, uint32_t size);

bool filesystem_write_file(filesystem* fs, const char* data, bool overwrite);

#endif // FILESYSTEM_H
