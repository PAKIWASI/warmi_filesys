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
bool filesystem_ls(filesystem* fs);

// the "cd" command equivalent
// supports ".." to go to parent
bool filesystem_cd(filesystem* fs, const char* dirname);

// move a file/dir "src" to "dest"
bool filesystem_mv(filesystem* fs, Tree_Node* dest, Tree_Node* src);

// supports any dir name in curr_dir or ".." to move one layer up
bool filesyste_mv_by_name(filesystem* fs, Tree_Node* tn, const char* dir);


// file/directory creation/deletion

// create file in current directory
Tree_Node* filesystem_touch(filesystem* fs, const char* name);

// create a directory
Tree_Node* filesystem_mkdir(filesystem* fs, const char* name);

// delete file/dir in the current directory
bool filesystem_rm(filesystem* fs, const char* name);


// file reading/writing

// view a file in the current directory
bool filesystem_cat(filesystem* fs, const char* name, uint32_t start, uint32_t size);

bool filesystem_write_file(filesystem* fs, const char* data, bool overwrite);


// persistance

bool filesystem_save(filesystem* fs, const char* path);

bool filesystem_load(const char* filepath);


// TODO:  11.	Show memory map → should show the distribution of files in the memory.

#endif // FILESYSTEM_H
