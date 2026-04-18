#include "filesystem.h"
#include "tree_node.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GET_CURR_DIR(fs)    (fs->curr_dirs[fs->curr_depth - 1])
#define GET_INSERT_POS(fs)  (fs->curr_dirs[fs->curr_depth])



filesystem* filesystem_create(void)
{
    filesystem* fs = malloc(sizeof(filesystem));

    // create root node (a directory)
    fs->root = tree_node_create("root", NODE_BRANCH);

    // set the curr_dirs stack
    fs->curr_dirs = (Tree_Node**)malloc(sizeof(Tree_Node*) * MAX_DEPTH);
    
    // metadata
    fs->curr_depth = 0;

    GET_INSERT_POS(fs) = fs->root;
    fs->curr_depth++;    // curr dir is root, stack has 1 item

    return fs;
}

void filesystem_destroy(filesystem* fs)
{
    // recursivly destroy whole tree, from leaves to root
    tree_node_destroy(fs->root);

    free((void*)fs->curr_dirs);

    free(fs);
}

void filesystem_ls(filesystem* fs, const char* dirname)
{
    Tree_Node* curr_dir = GET_CURR_DIR(fs);
    uint32_t num_children = curr_dir->dir.num_children;

    if (num_children == 0) {
        printf("Show Dir \"%s\": Empty Directory\n", curr_dir->name);
        return;
    }

    if (dirname) {
        uint32_t child_idx = tree_node_find_child_by_name(curr_dir, dirname);
        if (child_idx == MAX_CHILDREN) {
            printf("%s not found\n", dirname);
            return;
        }
        // now the nested dir will be printed
        curr_dir = curr_dir->dir.children[child_idx];
    }

    printf("\nShow Dir \"%s\":\n", curr_dir->name);
    for (uint32_t i = 0; i < curr_dir->dir.num_children; i++) {
        printf("\t- %s\n", curr_dir->dir.children[i]->name);
    }
    putchar('\n');
}

bool filesystem_cd(filesystem* fs, const char* dirname)
{
    if (fs->curr_depth >= MAX_DEPTH) { 
        printf("Maximum Depth reached\n");
        return false;
    }

    Tree_Node* curr_dir = GET_CURR_DIR(fs);

    // go up 1 level
    if (strcmp(dirname, "..") == 0) {
        fs->curr_depth--;
        printf("Changed Dir to %s\n", dirname);
        return true;
    }

    uint32_t child_idx = tree_node_find_child_by_name(curr_dir, dirname);

    if (child_idx == MAX_CHILDREN) {
        printf("%s not found\n", dirname);
        return false;
    }

    Tree_Node* child = curr_dir->dir.children[child_idx];

    if (child->type != NODE_BRANCH) {
        printf("%s is a file\n", dirname);
        return false;
    }

    GET_INSERT_POS(fs) = child;
    fs->curr_depth++;

    printf("Changed Dir to %s\n", dirname);
    return true;
}

bool filesystem_mv(filesystem* fs, const char* name, const char* move_to)
{
    Tree_Node* curr_dir = GET_CURR_DIR(fs);
    uint32_t child_idx = tree_node_find_child_by_name(curr_dir, name);
    
    if (child_idx == MAX_CHILDREN) {
        printf("%s not found\n", name);
        return false;
    }

    Tree_Node* child = curr_dir->dir.children[child_idx];

    // we are moving one level up
    if (strcmp(move_to, "..") == 0) {
        // we are in root dir
        if (fs->curr_depth == 1) { return false; }

        Tree_Node* parent = fs->curr_dirs[fs->curr_depth - 2];
        // parent is full
        if (parent->dir.num_children == MAX_CHILDREN) {
            printf("%s is full\n", parent->name);
            return false;
        }

        parent->dir.children[parent->dir.num_children] = child;
        parent->dir.num_children++;

        printf("Moved %s to %s\n", name, move_to);
        return tree_node_delete_child_ref(curr_dir, child);
    }

    // move to a directory in curr dir
    uint32_t dest_idx = tree_node_find_child_by_name(curr_dir, move_to);
    
    if (dest_idx == MAX_CHILDREN) {
        printf("%s not found\n", move_to);
        return false;
    }
    
    Tree_Node* dest_dir = curr_dir->dir.children[dest_idx];

    if (dest_dir->type != NODE_BRANCH) {
        printf("%s is a file\n", move_to);
        return false;
    }

    if (dest_dir->dir.num_children == MAX_CHILDREN) {
        printf("%s is full\n", dest_dir->name);
        return false;
    }

    dest_dir->dir.children[dest_dir->dir.num_children] = child;
    dest_dir->dir.num_children++;

    printf("Moved %s to %s", name, move_to);
    // remove ref of the moved file from curr_dir
    return tree_node_delete_child_ref(curr_dir, child);
}

Tree_Node* filesystem_touch(filesystem* fs, const char* name)
{
    Tree_Node* curr_dir = GET_CURR_DIR(fs);

    if (curr_dir->dir.num_children == MAX_CHILDREN) {
        printf("%s is full\n", curr_dir->name);
        return NULL;
    }
    return tree_node_create_child(curr_dir, name, NODE_LEAF);
}

Tree_Node* filesystem_mkdir(filesystem* fs, const char* name)
{
    Tree_Node* curr_dir = GET_CURR_DIR(fs);

    if (curr_dir->dir.num_children == MAX_CHILDREN) {
        printf("%s is full\n", curr_dir->name);
        return NULL;
    }

    return tree_node_create_child(curr_dir, name, NODE_BRANCH);
}

bool filesystem_rm(filesystem* fs, const char* name)
{
    Tree_Node* curr_dir = GET_CURR_DIR(fs);

    if (!tree_node_delete_child_by_name(curr_dir, name)) {
        printf("%s not found\n", name);
        return false;
    }
    return true;
}

bool filesystem_cat(filesystem* fs, const char* filename, uint32_t start, uint32_t size)
{
    Tree_Node* curr_dir = GET_CURR_DIR(fs);
    uint32_t idx = tree_node_find_child_by_name(curr_dir, filename);

    if (idx == MAX_CHILDREN) {
        printf("%s not found\n", filename);
        return false;
    }

    Tree_Node* node = curr_dir->dir.children[idx];

    if (node->type != NODE_LEAF) {
        printf("%s is a directory\n", filename);
        return false;
    }

    tree_node_read_file(node, start, size);
    return true;
}

bool filesystem_write_file(filesystem* fs, const char* filename, const char* data, uint32_t size, bool overwrite)
{
    Tree_Node* curr_dir = GET_CURR_DIR(fs);

    uint32_t idx = tree_node_find_child_by_name(curr_dir, filename);
    if (idx == MAX_CHILDREN) {
        printf("%s not found\n", filename);
        return false;
    }
    
    Tree_Node* child = curr_dir->dir.children[idx];
    if (child->type != NODE_LEAF) {
        printf("%s is not a file\n", filename);
        return false;
    }

    return tree_node_write_file(child, data, size, overwrite) != NULL;
}

bool filesystem_move_cursor(filesystem* fs, const char* filename, uint32_t pos)
{
    Tree_Node* curr_dir = GET_CURR_DIR(fs);

    uint32_t idx = tree_node_find_child_by_name(curr_dir, filename);
    if (idx == MAX_CHILDREN) {
        printf("%s not found\n", filename);
        return false;
    }
    
    Tree_Node* child = curr_dir->dir.children[idx];
    if (child->type != NODE_LEAF) {
        printf("%s is not a file\n", filename);
        return false;
    }
    
    child->file.pos = pos;
    printf("%s cusor moved to %d\n", child->name, pos);
    return true;
}

static void filesystem_print_recursive(Tree_Node* node, uint32_t depth)
{
    // indent by depth
    for (uint32_t i = 0; i < depth; i++) { printf("  "); }

    if (node->type == NODE_BRANCH) {
        printf("[%s]\n", node->name);
        for (uint32_t i = 0; i < node->dir.num_children; i++) {
            filesystem_print_recursive(node->dir.children[i], depth + 1);
        }
    } else {
        printf("%s (%u bytes)\n", node->name, node->file.size);
    }
}

void filesystem_print(filesystem* fs)
{
    printf("\n--- Filesystem Tree ---\n");
    filesystem_print_recursive(fs->root, 0);
    printf("--- ----------------- ---\n\n");
}


// helper: recursively write a node
static void save_node(Tree_Node* node, FILE* f)
{
    // write type and name
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

bool filesystem_save(filesystem* fs, const char* path)
{
    FILE* f = fopen(path, "wb");
    if (!f) {
        printf("Could not open %s for writing\n", path);
        return false;
    }

    save_node(fs->root, f);
    fclose(f);
    printf("Saved filesystem to %s\n", path);
    return true;
}

// helper: recursively read a node
static Tree_Node* load_node(FILE* f)
{
    NODE_TYPE type;
    char name[MAX_NAME_SIZE];

    if (fread(&type, sizeof(NODE_TYPE), 1, f) != 1) { return NULL; }
    if (fread(name,  MAX_NAME_SIZE,    1, f) != 1) { return NULL; }

    Tree_Node* node = tree_node_create(name, type);

    // we are at an edge
    if (type == NODE_LEAF) {
        fread(&node->file.size, sizeof(uint32_t), 1, f);
        fread(&node->file.pos,  sizeof(uint32_t), 1, f);
        fread(node->file.data,  MAX_FILE_SIZE,    1, f);
        // it has nested nodes
    } else {
        uint32_t num_children;
        fread(&num_children, sizeof(uint32_t), 1, f);
        for (uint32_t i = 0; i < num_children; i++) {
            Tree_Node* child = load_node(f);
            if (child) { tree_node_append_node(node, child); }
        }
    }

    return node;
}

filesystem* filesystem_load(const char* filepath)
{
    FILE* f = fopen(filepath, "rb");
    if (!f) {
        printf("Could not open %s\n", filepath);
        return NULL;
    }

    filesystem* fs = malloc(sizeof(filesystem));
    fs->curr_dirs  = (Tree_Node**)malloc(sizeof(Tree_Node*) * MAX_DEPTH);
    fs->curr_depth = 0;

    fs->root = load_node(f);
    fclose(f);

    // start at root
    fs->curr_dirs[0] = fs->root;
    fs->curr_depth   = 1;

    printf("Loaded filesystem from %s\n", filepath);
    return fs;
}
