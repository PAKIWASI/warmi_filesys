#include "filesystem.h"
#include "tree_node.h"

#include <stdlib.h>




filesystem* filesystem_create(void)
{
    filesystem* fs = malloc(sizeof(filesystem));

    // create root node (a directory)
    fs->root = tree_node_create("root", NODE_BRANCH);
    fs->curr_dir = fs->root;    // curr dir set to root
    
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
    tree_node_destroy(fs->root);

    free(fs);
}





// TODO: 
static void filesystem_destroy_helper(Tree_Node* tn)
{

}
