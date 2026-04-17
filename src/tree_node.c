#include "tree_node.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define GET_SLOT(node) (node->dir.children[node->dir.num_children])



Tree_Node* tree_node_create(const char* name, NODE_TYPE type)
{
    Tree_Node* tn = malloc(sizeof(Tree_Node));

    tn->name = malloc(MAX_NAME_SIZE);
    strncpy(tn->name, name, MAX_NAME_SIZE - 1);
    tn->name[MAX_NAME_SIZE - 1] = '\0';

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
    GET_SLOT(tn) = cn;  // set child
    tn->dir.num_children++; // child counter increment

    return cn;
}

Tree_Node* tree_node_append_node(Tree_Node* parent, Tree_Node* child)
{
    if (parent->type != NODE_BRANCH) { return NULL; }
    if (parent->dir.num_children == MAX_CHILDREN) { return NULL; }

    GET_SLOT(parent) = child;
    parent->dir.num_children++;

    return child;
}

char* tree_node_read_file(Tree_Node* tn, uint32_t start, uint32_t size)
{
    if (tn->type != NODE_LEAF) { return NULL; }

    // clamp to max file size if size is greater
    size = (start + size) >= MAX_FILE_SIZE ? MAX_FILE_SIZE - start - 1 : size;

    char* read = malloc(size + 1);  // +1 for null terminator
    strncpy(read, tn->file.data + start, size);

    return read;
}

char* tree_node_write_file(Tree_Node* tn, const char* data, uint32_t size, bool overwrite)
{
    if (tn->type != NODE_LEAF) { return NULL; }
    // if overwriting, start writing from the start
    // TODO: also set final size at end
    if (overwrite) {
        tn->file.pos = 0;
    }

    if (tn->file.pos + size >= MAX_FILE_SIZE) { return NULL; }


}

