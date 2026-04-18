#include "tree_node.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define GET_INSERT_POS(node) (node->dir.children[node->dir.num_children])



Tree_Node* tree_node_create(const char* name, NODE_TYPE type)
{
    Tree_Node* tn = malloc(sizeof(Tree_Node));

    tn->name = malloc(MAX_NAME_SIZE);
    strcpy(tn->name, name);

    tn->type = type;

    switch (type) {

    case NODE_BRANCH:               // we store pointers to tree nodes
        tn->dir.children = (Tree_Node**)malloc(sizeof(Tree_Node*) * MAX_CHILDREN);
        tn->dir.num_children = 0;
        break;

    case NODE_LEAF:
        tn->file.data = malloc(MAX_FILE_SIZE);
        tn->file.size = 0;
        tn->file.pos = 0;
        break;
    }

    return tn;
}

void tree_node_destroy(Tree_Node* tn)
{
    if (!tn) { return; }

    free(tn->name);

    if (tn->type == NODE_BRANCH) {
        // recursivly destroy all children first
        for (uint32_t i = 0; i < tn->dir.num_children; i++) {
            tree_node_destroy(tn->dir.children[i]);
        }
        free((void*)tn->dir.children);

    } else if (tn->type == NODE_LEAF) {
        free(tn->file.data);
    }

    free(tn);
}

Tree_Node* tree_node_create_child(Tree_Node* tn, const char* name, NODE_TYPE type)
{
    // current node is file - can't have nested files/dirs
    if (tn->type != NODE_BRANCH) { return NULL; }
    // max number of children reached
    if (tn->dir.num_children == MAX_CHILDREN) { return NULL; }

    Tree_Node* cn = tree_node_create(name, type);
    GET_INSERT_POS(tn) = cn;  // set child
    tn->dir.num_children++; // child counter increment

    printf("Created: %s/%s\n", tn->name, name);
    return cn;
}

Tree_Node* tree_node_append_node(Tree_Node* parent, Tree_Node* child)
{
    if (parent->type != NODE_BRANCH) { return NULL; }
    if (parent->dir.num_children == MAX_CHILDREN) { return NULL; }

    GET_INSERT_POS(parent) = child;
    parent->dir.num_children++;

    return child;
}

void tree_node_read_file(Tree_Node* tn, uint32_t start, uint32_t size)
{
    if (tn->type != NODE_LEAF) { return; }

    // clamp to max file size if size is greater
    uint32_t end = (start + size) >= tn->file.size ?
                    tn->file.size - 1 : (start + size);

    printf("\n%s:\n", tn->name);
    for (uint32_t i = start; i < end; i++) {
        putchar(tn->file.data[i]);
    }
    putchar('\n');
    putchar('\n');
}


char* tree_node_write_file(Tree_Node* tn, const char* data, uint32_t size, bool overwrite)
{
    if (tn->type != NODE_LEAF) { return NULL; }
    // if overwriting, start writing from the start
    if (overwrite) {
        tn->file.pos = 0;
    }
    
    // if there is more to write than we have space, we only write as much as we have space
    if (tn->file.pos + size >= MAX_FILE_SIZE) {
        size = MAX_FILE_SIZE - tn->file.pos - 1;
        // TODO: check
        tn->file.size = size;
    } else {
        tn->file.size = tn->file.pos + size;
    }
    
    memcpy(tn->file.data + tn->file.pos, data, size);

    printf("Written to: %s\n", tn->name);
    return tn->file.data;   // return the file's data
}

bool tree_node_delete_child(Tree_Node* parent, Tree_Node* child)
{
    uint32_t child_idx = tree_node_find_child(parent, child);
    // not found
    if (child_idx == MAX_CHILDREN) { return false; }

    printf("Deleted: %s/%s\n", parent->name, child->name);
    tree_node_destroy(child);

    // swap delete the parent
    parent->dir.num_children--;
    parent->dir.children[child_idx] = GET_INSERT_POS(parent);

    return true;
}

bool tree_node_delete_child_by_name(Tree_Node* tn, const char* name)
{
    uint32_t child_idx = tree_node_find_child_by_name(tn, name);

    if (child_idx == MAX_CHILDREN) { return false; }

    tree_node_destroy(tn->dir.children[child_idx]);

    // swap delete the parent
    tn->dir.num_children--;
    tn->dir.children[child_idx] = GET_INSERT_POS(tn);

    printf("Deleted: %s/%s\n", tn->name, name);
    return true;
}

bool tree_node_delete_child_ref(Tree_Node* parent, Tree_Node* child)
{
    uint32_t child_idx = tree_node_find_child(parent, child);

    if (child_idx == MAX_CHILDREN) { return false; }

    // swap delete the parent
    parent->dir.num_children--;
    parent->dir.children[child_idx] = GET_INSERT_POS(parent);

    return true;
}

uint32_t tree_node_find_child(Tree_Node* parent, Tree_Node* child)
{
    if (parent->type != NODE_BRANCH) { return MAX_CHILDREN; }

    uint32_t child_idx = MAX_CHILDREN;
    for (uint32_t i = 0; i < parent->dir.num_children; i++) {
        if (strcmp(parent->dir.children[i]->name, child->name) == 0) {
            child_idx = i;
            break;
        }
    }

    return child_idx;
}

uint32_t tree_node_find_child_by_name(Tree_Node* parent, const char* name)
{
    if (parent->type != NODE_BRANCH) { return MAX_CHILDREN; }

    uint32_t child_idx = MAX_CHILDREN;
    for (uint32_t i = 0; i < parent->dir.num_children; i++) {
        if (strcmp(parent->dir.children[i]->name, name) == 0) {
            child_idx = i;
            break;
        }
    }

    return child_idx;
}

