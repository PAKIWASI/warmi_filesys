#include "filesystem.h"
#include "tree_node.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>


/* WArmi - Tree Based File System
 * Authors:
 *  Wasi Ullah
 *  Armaghan Mehmood 200KG
 
    The File System is a tree with the root being '/'
    where the user starts off and creates children of
    this root by creating files/directories
*/

// helpers
static void build_prompt(filesystem* fs, char* out, uint32_t out_sz);
static bool read_line(const char* prompt, char* buf, uint32_t buf_sz);
static int tokenise(char* line, char** toks, int max_toks);
static void print_menu(void);

// testing scripts
static int test1(void);
static int test2(void);

#define SAVE_PATH   "./data/warmi.bin"
#define INPUT_BUF   512
#define TOKEN_BUF   128





// ls [dirname]
static void cmd_ls(filesystem* fs, char** toks, int n)
{
    const char* dirname = (n >= 2) ? toks[1] : NULL;
    filesystem_ls(fs, dirname);
}

// cd <dirname|..>
static void cmd_cd(filesystem* fs, char** toks, int n)
{
    if (n < 2) { printf("Usage: cd <dirname|..>\n"); return; }
    filesystem_cd(fs, toks[1]);
}

// touch <name>
static void cmd_touch(filesystem* fs, char** toks, int n)
{
    if (n < 2) { printf("Usage: touch <name>\n"); return; }
    filesystem_touch(fs, toks[1]);
}

// mkdir <name>
static void cmd_mkdir(filesystem* fs, char** toks, int n)
{
    if (n < 2) { printf("Usage: mkdir <name>\n"); return; }
    filesystem_mkdir(fs, toks[1]);
}

// rm <name>
static void cmd_rm(filesystem* fs, char** toks, int n)
{
    if (n < 2) { printf("Usage: rm <name>\n"); return; }
    filesystem_rm(fs, toks[1]);
}

// mv <name> <dest|..>
static void cmd_mv(filesystem* fs, char** toks, int n)
{
    if (n < 3) { printf("Usage: mv <name> <dest|..>\n"); return; }
    filesystem_mv(fs, toks[1], toks[2]);
}

// cat <file> [start size]
static void cmd_cat(filesystem* fs, char** toks, int n)
{
    if (n < 2) { printf("Usage: cat <file> [start size]\n"); return; }

    uint32_t start = 0;
    uint32_t size  = MAX_FILE_SIZE; // default: read everything

    if (n >= 4) {
        start = (uint32_t)atoi(toks[2]);
        size  = (uint32_t)atoi(toks[3]);
    }
    filesystem_cat(fs, toks[1], start, size);
}

// write <file> <text...>  — rejoins tokens after filename with spaces
static void cmd_write(filesystem* fs, char** toks, int n, bool overwrite)
{
    if (n < 3) {
        printf("Usage: %s <file> <text>\n", (int)overwrite ? "write" : "append");
        return;
    }

    // rejoin everything after the filename as one string
    char data[INPUT_BUF] = {0};
    for (int i = 2; i < n; i++) {
        strncat(data, toks[i], INPUT_BUF - strlen(data) - 1);
        if (i + 1 < n) { strncat(data, " ", INPUT_BUF - strlen(data) - 1); }
    }

    uint32_t size = (uint32_t)strlen(data);
    filesystem_write_file(fs, toks[1], data, size, overwrite);
}

// seek <file> <pos>
static void cmd_seek(filesystem* fs, char** toks, int n)
{
    if (n < 3) { printf("Usage: seek <file> <pos>\n"); return; }
    uint32_t pos = (uint32_t)atoi(toks[2]);
    filesystem_move_cursor(fs, toks[1], pos);
}

// save [path]
static void cmd_save(filesystem* fs, char** toks, int n)
{
    const char* path = (n >= 2) ? toks[1] : SAVE_PATH;
    filesystem_save(fs, path);
}

// load [path] — replaces the current fs, returns the new one (or old on fail)
static filesystem* cmd_load(filesystem* fs, char** toks, int n)
{
    const char* path = (n >= 2) ? toks[1] : SAVE_PATH;
    filesystem* loaded = filesystem_load(path);
    if (!loaded) {
        printf("Load failed, keeping current filesystem.\n");
        return fs;
    }
    filesystem_destroy(fs);
    return loaded;
}

int main(void)
{
    // try to load an existing save, otherwise start fresh
    filesystem* fs = filesystem_load(SAVE_PATH);
    if (!fs) {
        printf("No save found, starting fresh.\n");
        fs = filesystem_create();
    }

    print_menu();

    char line[INPUT_BUF];
    char prompt[INPUT_BUF];
    char* toks[16];

    while (true) {
        build_prompt(fs, prompt, sizeof(prompt));

        if (!read_line(prompt, line, sizeof(line))) {
            // EOF (e.g. Ctrl-D)
            break;
        }

        if (line[0] == '\0') { continue; } // blank line

        int n = tokenise(line, toks, 16);
        if (n == 0) { continue; }

        char* cmd = toks[0];

        if (strcmp(cmd, "help") == 0) {
            print_menu();

        } else if (strcmp(cmd, "ls") == 0) {
            cmd_ls(fs, toks, n);

        } else if (strcmp(cmd, "cd") == 0) {
            cmd_cd(fs, toks, n);

        } else if (strcmp(cmd, "tree") == 0) {
            filesystem_print(fs);

        } else if (strcmp(cmd, "touch") == 0) {
            cmd_touch(fs, toks, n);

        } else if (strcmp(cmd, "mkdir") == 0) {
            cmd_mkdir(fs, toks, n);

        } else if (strcmp(cmd, "rm") == 0) {
            cmd_rm(fs, toks, n);

        } else if (strcmp(cmd, "mv") == 0) {
            cmd_mv(fs, toks, n);

        } else if (strcmp(cmd, "cat") == 0) {
            cmd_cat(fs, toks, n);

        } else if (strcmp(cmd, "write") == 0) {
            cmd_write(fs, toks, n, true);      // overwrite mode

        } else if (strcmp(cmd, "append") == 0) {
            cmd_write(fs, toks, n, false);     // append at cursor

        } else if (strcmp(cmd, "seek") == 0) {
            cmd_seek(fs, toks, n);

        } else if (strcmp(cmd, "save") == 0) {
            cmd_save(fs, toks, n);

        } else if (strcmp(cmd, "load") == 0) {
            fs = cmd_load(fs, toks, n);

        } else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
            printf("Saving before exit...\n");
            filesystem_save(fs, SAVE_PATH);
            break;

        } else {
            printf("Unknown command: %s  (type 'help' for commands)\n", cmd);
        }
    }

    filesystem_destroy(fs);
    printf("Goodbye.\n");
    return 0;
}

static void print_menu(void)
{
    printf("\nWarmi - Tree Based Filesystem\n");
    printf("===============================\n");
    printf("  Navigation:\n");
    printf("    ls [dirname]              list current dir (or subdir)\n");
    printf("    cd <dirname|..>           change directory\n");
    printf("    tree                      print full filesystem tree\n");
    printf("\n");
    printf("  Create / Delete:\n");
    printf("    touch <name>              create a file\n");
    printf("    mkdir <name>              create a directory\n");
    printf("    rm    <name>              delete file or directory\n");
    printf("\n");
    printf("  Move:\n");
    printf("    mv <name> <dest|..>       move file/dir to dest or up one level\n");
    printf("\n");
    printf("  File I/O:\n");
    printf("    cat  <file> [start size]  read file (default: read all)\n");
    printf("    write <file> <text>       overwrite file with text\n");
    printf("    append <file> <text>      append text at cursor position\n");
    printf("    seek <file> <pos>         move write cursor to byte pos\n");
    printf("\n");
    printf("  Persistence:\n");
    printf("    save [path]               save filesystem (default: %s)\n", SAVE_PATH);
    printf("    load [path]               load filesystem (default: %s)\n", SAVE_PATH);
    printf("\n");
    printf("  Other:\n");
    printf("    help                      show this menu\n");
    printf("    exit / quit               save and quit\n");
    printf("===============================\n\n");
}

// build a prompt like "warmi:/root/dir1> "
static void build_prompt(filesystem* fs, char* out, uint32_t out_sz)
{
    // collect names from the depth stack
    char tmp[INPUT_BUF] = "warmi:/";
    for (uint32_t i = 1; i < fs->curr_depth; i++) {
        strncat(tmp, fs->curr_dirs[i]->name, INPUT_BUF - strlen(tmp) - 1);
        if (i + 1 < fs->curr_depth) {
            strncat(tmp, "/", INPUT_BUF - strlen(tmp) - 1);
        }
    }
    strncat(tmp, "> ", INPUT_BUF - strlen(tmp) - 1);
    strncpy(out, tmp, out_sz - 1);
    out[out_sz - 1] = '\0';
}

// read one trimmed line into buf, return false on EOF
static bool read_line(const char* prompt, char* buf, uint32_t buf_sz)
{
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, (int)buf_sz, stdin)) { return false; }
    // strip trailing newline
    buf[strcspn(buf, "\n")] = '\0';
    return true;
}

// tokenise in-place, store up to max_toks pointers, return count
static int tokenise(char* line, char** toks, int max_toks)
{
    int n = 0;
    char* p = strtok(line, " \t");
    while (p && n < max_toks) {
        toks[n++] = p;
        p = strtok(NULL, " \t");
    }
    return n;
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

int test2(void)
{
    filesystem* fs = filesystem_load("./data/warmi.bin");

    filesystem_print(fs);

    filesystem_destroy(fs);
    return 0;
}
