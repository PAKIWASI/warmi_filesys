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
    fs->total_dirs = 1; // the root dir
    fs->total_files = 0;

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

void filesystem_ls(filesystem* fs)
{
    Tree_Node* curr_dir = GET_CURR_DIR(fs);
    uint32_t num_children = curr_dir->dir.num_children;

    if (num_children == 0) {
        printf("Empty Directory\n");
        return;
    }

    printf("%s:\n", curr_dir->name);
    for (uint32_t i = 0; i < num_children; i++) {
        printf("\t- %s\n", curr_dir->dir.children[i]->name);
    }
    putchar('\n');
}

bool filesystem_cd(filesystem* fs, const char* dirname)
{
    Tree_Node* curr_dir = GET_CURR_DIR(fs);
    uint32_t child_idx = tree_node_find_child_by_name(curr_dir, dirname);

    if (child_idx == MAX_CHILDREN) {
        printf("Director %s not found\n", dirname);
        return false;
    }

    Tree_Node* child = curr_dir->dir.children[child_idx];

    if (child->type != NODE_BRANCH) {
        printf("%s is a file\n", dirname);
        return false;
    }

    if (fs->curr_depth == MAX_DEPTH) {
        printf("max depth of filesystem reached\n");
        return false;
    }

    GET_INSERT_POS(fs) = child;
    fs->curr_depth++;

    return true;
}

// TODO: how will we handle num_children decrement?
bool filesyste_mv(filesystem* fs, const char* name, const char* move_to)
{
    Tree_Node* curr_dir = GET_CURR_DIR(fs);
    uint32_t child_idx = tree_node_find_child_by_name(curr_dir, name);

    Tree_Node* child = curr_dir->dir.children[child_idx];

    // we are moving one level up
    if (strcmp(move_to, "..") == 0) {
        // we are in root dir
        if (fs->curr_depth == 1) { return false; }

        Tree_Node* parent = fs->curr_dirs[fs->curr_depth - 2];
        // parent is full
        if (parent->dir.num_children == MAX_CHILDREN) { return false; }


    }

}

