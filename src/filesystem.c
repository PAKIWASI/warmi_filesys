#include "filesystem.h"
#include "tree_node.h"

#include <stdlib.h>

#define GET_CURR_DIR(fs) (fs->curr_dirs[fs->depth])



filesystem* filesystem_create(void)
{
    filesystem* fs = malloc(sizeof(filesystem));

    // create root node (a directory)
    fs->root = tree_node_create("root", NODE_BRANCH);

    // set the curr_dirs stack
    fs->curr_dirs = (Tree_Node**)malloc(sizeof(Tree_Node*) * MAX_DEPTH);
    
    // metadata
    fs->depth = 0;
    fs->total_dirs = 1; // the root dir
    fs->total_files = 0;

    GET_CURR_DIR(fs) = fs->root;
    fs->depth++;    // curr dir is root, stack has 1 item

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
