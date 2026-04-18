#include "filesystem.h"


/* WArmi - Tree Based File System
 * Authors:
 *  Wasi Ullah
 *  Armaghan Mehmood 200KG
 
    The File System is a tree with the root being '/'
    where the user starts off and creates children of
    this root by creating files/directories
*/



int main(void)
{
    filesystem* fs = filesystem_create();

    filesystem_touch(fs, "file1.txt");
    filesystem_touch(fs, "file2.txt");
    filesystem_touch(fs, "file3.txt");
    filesystem_touch(fs, "file4.txt");

    filesystem_ls(fs);

    filesystem_mkdir(fs, "dir1");
    filesystem_ls(fs);

    filesystem_cd(fs, "file");
    filesystem_cd(fs, "file1.txt");
    filesystem_cd(fs, "dir1");

    filesystem_touch(fs, "wasi.txt");

    filesystem_ls(fs);

    filesystem_rm(fs, "wasi.txt");

    filesystem_ls(fs);

    filesystem_destroy(fs);
    return 0;
}
