#include "filesystem.h"
#include <string.h>


/* WArmi - Tree Based File System
 * Authors:
 *  Wasi Ullah
 *  Armaghan Mehmood 200KG
 
    The File System is a tree with the root being '/'
    where the user starts off and creates children of
    this root by creating files/directories
*/

int text1(void);


int main(void)
{

}


int test1(void)
{
    filesystem* fs = filesystem_create();

    filesystem_touch(fs, "file1.txt");
    filesystem_touch(fs, "file2.txt");
    filesystem_touch(fs, "file3.txt");
    filesystem_touch(fs, "file4.txt");

    filesystem_ls(fs, NULL);

    filesystem_mkdir(fs, "dir1");
    filesystem_ls(fs, NULL);

    filesystem_cd(fs, "file");
    filesystem_cd(fs, "file1.txt");
    filesystem_cd(fs, "dir1");

    filesystem_touch(fs, "wasi.txt");

    filesystem_ls(fs, NULL);

    filesystem_rm(fs, "wasi.txt");
    filesystem_ls(fs, NULL);

    filesystem_touch(fs, "wasi.txt");

    filesystem_mv(fs, "wasi.txt", "..");

    filesystem_ls(fs, NULL);

    filesystem_cd(fs, "..");

    filesystem_ls(fs, NULL);

    filesystem_mv(fs, "wasi.txt", "dir1");

    filesystem_ls(fs, "dir1");

    filesystem_cd(fs, "dir1");

    const char* data = "hello my name is wasi ullah\n idk what to say lol";
    filesystem_write_file(fs, "wasi.txt", data, 49, false);

    filesystem_cat(fs, "wasi.txt", 0, 1000);

    filesystem_move_cursor(fs, "wasi.txt", 10);

    filesystem_write_file(fs, "wasi.txt", data, 49, false);

    filesystem_cat(fs, "wasi.txt", 0, 1000);

    filesystem_write_file(fs, "wasi.txt", data, 49, true);

    filesystem_cat(fs, "wasi.txt", 0, 1000);

    filesystem_mkdir(fs, "dir2");

    filesystem_cd(fs, "dir2");

    filesystem_touch(fs, "file0.txt");

    filesystem_mkdir(fs, "dir3");

    filesystem_cd(fs, "dir3");

    filesystem_touch(fs, "file1.txt");
    filesystem_touch(fs, "file2.txt");

    filesystem_print(fs);

    filesystem_save(fs, "./data/warmi.bin");

    filesystem_destroy(fs);
    return 0;
}
