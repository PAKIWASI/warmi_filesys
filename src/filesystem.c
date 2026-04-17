#include "filesystem.h"
#include "tree_node.h"

#include <stdlib.h>


// helpers

static void filesystem_destroy_helper(Tree_Node* tn);



filesystem* filesystem_create(void)
{
    filesystem* fs = malloc(sizeof(filesystem));

    // create root node (a directory)
    fs->root = tree_node_create(NODE_BRANCH);
    
    // metadata
    fs->total_dirs = 1; // the root dir
    fs->total_files = 0;
    // only root so depth of tree is 0
    fs->depth = 0;

    return fs;
}

void filesystem_destroy(filesystem* fs)
{
    // recursivly destroy whole tree, from leaves to root
    filesystem_destroy_helper(fs->root);
    free(fs);
}





// TODO: 
static void filesystem_destroy_helper(Tree_Node* tn)
{

}
