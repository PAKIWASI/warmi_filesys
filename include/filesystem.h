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
    uint32_t curr_depth;

} filesystem;


// create new file system with only root '/' dir
filesystem* filesystem_create(void);

// recursivly destroy whole filesystem tree
void filesystem_destroy(filesystem* fs);


// filesystem navigation

// the "ls" command equivalent
// option dirname for viewing contents of a nested dir
void filesystem_ls(filesystem* fs, const char* dirname);

// the "cd" command equivalent
// supports ".." to go to parent
bool filesystem_cd(filesystem* fs, const char* dirname);

// supports any dir name in curr_dir or ".." to move one layer up
bool filesystem_mv(filesystem* fs, const char* name, const char* move_to);


// file/directory creation/deletion

// create file in current directory
Tree_Node* filesystem_touch(filesystem* fs, const char* name);

// create a directory
Tree_Node* filesystem_mkdir(filesystem* fs, const char* name);

// delete file/dir in the current directory
bool filesystem_rm(filesystem* fs, const char* name);


// file reading/writing

// view a file in the current directory
bool filesystem_cat(filesystem* fs, const char* filename, uint32_t start, uint32_t size);

bool filesystem_write_file(filesystem* fs, const char* filename, const char* data, uint32_t size, bool overwrite);

// move cursor to a specific position in the file to write
bool filesystem_move_cursor(filesystem* fs, const char* filename, uint32_t pos);


// persistance

bool filesystem_save(filesystem* fs, const char* path);

filesystem* filesystem_load(const char* filepath);


// visual

void filesystem_print(filesystem* fs);
// TODO: 11. Show memory map → should show the distribution of files in the memory.

#endif // FILESYSTEM_H
