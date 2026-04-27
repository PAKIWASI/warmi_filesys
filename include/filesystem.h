#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "tree_node.h"
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_DEPTH 256

// Shared resource: Mutex added to handle concurrent access
typedef struct {
    Tree_Node* root;         
    pthread_mutex_t lock;    // Added to prevent data corruption by multiple threads
} filesystem;

// Thread-local state: Moved stack here so threads have independent paths
typedef struct {
    int thread_id;                   
    filesystem* fs;                  
    Tree_Node* curr_dirs[MAX_DEPTH]; // Private stack for this thread
    uint32_t curr_depth;             
    FILE* output_file;               // File pointer for output_thread<x>.txt
} thread_context;

/* --- System Lifecycle --- */
filesystem* filesystem_create(void);
void filesystem_destroy(filesystem* fs);

/* --- Thread-Local Navigation --- */
// Now uses ctx to write output to specific thread files
void filesystem_ls(thread_context* ctx, const char* dirname);
bool filesystem_cd(thread_context* ctx, const char* dirname);
bool filesystem_mv(thread_context* ctx, const char* name, const char* move_to);

/* --- Thread-Safe Creation / Deletion --- */
// These modify the tree and must be locked in the thread worker
Tree_Node* filesystem_touch(thread_context* ctx, const char* name);
Tree_Node* filesystem_mkdir(thread_context* ctx, const char* name);
bool       filesystem_rm(thread_context* ctx, const char* name);

/* --- File I/O --- */
bool filesystem_cat(thread_context* ctx, const char* filename, uint32_t start, uint32_t size);
bool filesystem_write_file(thread_context* ctx, const char* filename, const char* data, uint32_t size, bool overwrite);
bool filesystem_move_cursor(thread_context* ctx, const char* filename, uint32_t pos);

/* --- Persistence & Visuals --- */
bool        filesystem_save(filesystem* fs, const char* path);
filesystem* filesystem_load(const char* filepath);
void        filesystem_print(filesystem* fs, FILE* out); // Updated to take thread output file

#endif // FILESYSTEM_H
