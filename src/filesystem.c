#include "filesystem.h"
#include "tree_node.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- Macros for Thread-Local Navigation --- */
#define GET_CURR_DIR(ctx)    (ctx->curr_dirs[ctx->curr_depth - 1])
#define GET_INSERT_POS(ctx)  (ctx->curr_dirs[ctx->curr_depth])

/* --- System Lifecycle --- */

filesystem* filesystem_create(void) {
    filesystem* fs = malloc(sizeof(filesystem));
    if (!fs) return NULL;

    fs->root = tree_node_create("root", NODE_BRANCH);
    
    // Global lock for the shared tree structure
    if (pthread_mutex_init(&fs->lock, NULL) != 0) {
        free(fs);
        return NULL;
    }
    return fs;
}

void filesystem_destroy(filesystem* fs) {
    if (!fs) return;
    tree_node_destroy(fs->root);
    pthread_mutex_destroy(&fs->lock);
    free(fs);
}

/* --- Navigation (Context-Aware) --- */

void filesystem_ls(thread_context* ctx, const char* dirname) {
    Tree_Node* curr_dir = GET_CURR_DIR(ctx);

    if (dirname) {
        uint32_t idx = tree_node_find_child_by_name(curr_dir, dirname);
        if (idx == MAX_CHILDREN) {
            fprintf(ctx->output_file, "ls: %s not found\n", dirname);
            return;
        }
        curr_dir = curr_dir->dir.children[idx];
    }

    fprintf(ctx->output_file, "\nShow Dir \"%s\":\n", curr_dir->name);
    if (curr_dir->dir.num_children == 0) {
        fprintf(ctx->output_file, "  Empty Directory\n");
    } else {
        for (uint32_t i = 0; i < curr_dir->dir.num_children; i++) {
            fprintf(ctx->output_file, "\t- %s\n", curr_dir->dir.children[i]->name);
        }
    }
}

bool filesystem_cd(thread_context* ctx, const char* dirname) {
    if (strcmp(dirname, "..") == 0) {
        if (ctx->curr_depth > 1) {
            ctx->curr_depth--;
            return true;
        }
        return false;
    }

    if (ctx->curr_depth >= MAX_DEPTH) return false;

    Tree_Node* curr_dir = GET_CURR_DIR(ctx);
    uint32_t idx = tree_node_find_child_by_name(curr_dir, dirname);

    if (idx == MAX_CHILDREN) {
        fprintf(ctx->output_file, "cd: %s not found\n", dirname);
        return false;
    }

    Tree_Node* child = curr_dir->dir.children[idx];
    if (child->type != NODE_BRANCH) return false;

    ctx->curr_dirs[ctx->curr_depth++] = child;
    return true;
}

bool filesystem_mv(thread_context* ctx, const char* name, const char* move_to) {
    Tree_Node* curr_dir = GET_CURR_DIR(ctx);
    uint32_t child_idx = tree_node_find_child_by_name(curr_dir, name);
    
    if (child_idx == MAX_CHILDREN) return false;

    Tree_Node* child = curr_dir->dir.children[child_idx];
    Tree_Node* dest_dir = NULL;

    if (strcmp(move_to, "..") == 0) {
        if (ctx->curr_depth <= 1) return false;
        dest_dir = ctx->curr_dirs[ctx->curr_depth - 2];
    } else {
        uint32_t dest_idx = tree_node_find_child_by_name(curr_dir, move_to);
        if (dest_idx == MAX_CHILDREN) return false;
        dest_dir = curr_dir->dir.children[dest_idx];
    }

    if (dest_dir->type != NODE_BRANCH || dest_dir->dir.num_children >= MAX_CHILDREN) return false;

    dest_dir->dir.children[dest_dir->dir.num_children++] = child;
    return tree_node_delete_child_ref(curr_dir, child);
}

/* --- Creation / Deletion --- */

Tree_Node* filesystem_touch(thread_context* ctx, const char* name) {
    return tree_node_create_child(GET_CURR_DIR(ctx), name, NODE_LEAF);
}

Tree_Node* filesystem_mkdir(thread_context* ctx, const char* name) {
    return tree_node_create_child(GET_CURR_DIR(ctx), name, NODE_BRANCH);
}

bool filesystem_rm(thread_context* ctx, const char* name) {
    return tree_node_delete_child_by_name(GET_CURR_DIR(ctx), name);
}

/* --- File I/O --- */

bool filesystem_cat(thread_context* ctx, const char* filename, uint32_t start, uint32_t size) {
    Tree_Node* curr_dir = GET_CURR_DIR(ctx);
    uint32_t idx = tree_node_find_child_by_name(curr_dir, filename);
    if (idx == MAX_CHILDREN) return false;

    Tree_Node* node = curr_dir->dir.children[idx];
    if (node->type != NODE_LEAF) return false;

    // Note: You may need to update tree_node_read_file to take ctx->output_file
    tree_node_read_file(node, start, size); 
    return true;
}

bool filesystem_write_file(thread_context* ctx, const char* filename, const char* data, uint32_t size, bool overwrite) {
    Tree_Node* curr_dir = GET_CURR_DIR(ctx);
    uint32_t idx = tree_node_find_child_by_name(curr_dir, filename);
    if (idx == MAX_CHILDREN) return false;
    
    Tree_Node* child = curr_dir->dir.children[idx];
    return tree_node_write_file(child, data, size, overwrite) != NULL;
}

bool filesystem_move_cursor(thread_context* ctx, const char* filename, uint32_t pos) {
    Tree_Node* curr_dir = GET_CURR_DIR(ctx);
    uint32_t idx = tree_node_find_child_by_name(curr_dir, filename);
    if (idx == MAX_CHILDREN) return false;
    
    Tree_Node* child = curr_dir->dir.children[idx];
    if (child->type != NODE_LEAF) return false;
    
    child->file.pos = pos;
    fprintf(ctx->output_file, "Cursor for %s moved to %u\n", child->name, pos);
    return true;
}

/* --- Persistence (Shared Logic) --- */

static void save_node(Tree_Node* node, FILE* f) {
    fwrite(&node->type, sizeof(NODE_TYPE), 1, f);
    fwrite(node->name, MAX_NAME_SIZE, 1, f);

    if (node->type == NODE_LEAF) {
        fwrite(&node->file.size, sizeof(uint32_t), 1, f);
        fwrite(&node->file.pos,  sizeof(uint32_t), 1, f);
        fwrite(node->file.data,  MAX_FILE_SIZE,    1, f);
    } else {
        fwrite(&node->dir.num_children, sizeof(uint32_t), 1, f);
        for (uint32_t i = 0; i < node->dir.num_children; i++) {
            save_node(node->dir.children[i], f);
        }
    }
}

bool filesystem_save(filesystem* fs, const char* path) {
    FILE* f = fopen(path, "wb");
    if (!f) return false;

    pthread_mutex_lock(&fs->lock); // Lock while traversing for save
    save_node(fs->root, f);
    pthread_mutex_unlock(&fs->lock);

    fclose(f);
    return true;
}

static Tree_Node* load_node(FILE* f) {
    NODE_TYPE type;
    char name[MAX_NAME_SIZE];

    if (fread(&type, sizeof(NODE_TYPE), 1, f) != 1) return NULL;
    if (fread(name,  MAX_NAME_SIZE,    1, f) != 1) return NULL;

    Tree_Node* node = tree_node_create(name, type);
    if (type == NODE_LEAF) {
        fread(&node->file.size, sizeof(uint32_t), 1, f);
        fread(&node->file.pos,  sizeof(uint32_t), 1, f);
        fread(node->file.data,  MAX_FILE_SIZE,    1, f);
    } else {
        uint32_t num_children;
        fread(&num_children, sizeof(uint32_t), 1, f);
        for (uint32_t i = 0; i < num_children; i++) {
            Tree_Node* child = load_node(f);
            if (child) tree_node_append_node(node, child);
        }
    }
    return node;
}

filesystem* filesystem_load(const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f) return NULL;

    filesystem* fs = malloc(sizeof(filesystem));
    pthread_mutex_init(&fs->lock, NULL);
    fs->root = load_node(f);
    fclose(f);

    return fs;
}

/* --- Visuals --- */

static void filesystem_print_recursive(Tree_Node* node, uint32_t depth, FILE* out) {
    for (uint32_t i = 0; i < depth; i++) fprintf(out, "  ");
    if (node->type == NODE_BRANCH) {
        fprintf(out, "[%s]\n", node->name);
        for (uint32_t i = 0; i < node->dir.num_children; i++) {
            filesystem_print_recursive(node->dir.children[i], depth + 1, out);
        }
    } else {
        fprintf(out, "%s (%u bytes)\n", node->name, node->file.size);
    }
}

void filesystem_print(filesystem* fs, FILE* out) {
    fprintf(out, "\n--- Filesystem Tree ---\n");
    pthread_mutex_lock(&fs->lock);
    filesystem_print_recursive(fs->root, 0, out);
    pthread_mutex_unlock(&fs->lock);
    fprintf(out, "-----------------------\n\n");
}
