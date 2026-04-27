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
#define INPUT_BUF   1024
#define MAX_TOKS    16

/* --- Helper Function: Tokenizer --- */
// Exactly as you had it, used to parse lines from input files.
static int tokenise(char* line, char** toks, int max_toks) {
    int n = 0;
    char* p = strtok(line, " \t\r\n");
    while (p && n < max_toks) {
        toks[n++] = p;
        p = strtok(NULL, " \t\r\n");
    }
    return n;
}

/* --- Core Command Functions --- 
 * Ported from your original code, redirected to thread_context 
 * and using fprintf to log to the thread's unique output file.
 */

static void cmd_ls(thread_context* ctx, char** toks, int n) {
    const char* dirname = (n >= 2) ? toks[1] : NULL;
    filesystem_ls(ctx, dirname);
}

static void cmd_cd(thread_context* ctx, char** toks, int n) {
    if (n < 2) { 
        fprintf(ctx->output_file, "Usage: cd <dirname|..>\n"); 
        return; 
    }
    filesystem_cd(ctx, toks[1]);
}

static void cmd_cat(thread_context* ctx, char** toks, int n) {
    if (n < 2) { 
        fprintf(ctx->output_file, "Usage: cat <file> [start size]\n"); 
        return; 
    }
    uint32_t start = (n >= 3) ? (uint32_t)atoi(toks[2]) : 0;
    uint32_t size  = (n >= 4) ? (uint32_t)atoi(toks[3]) : MAX_FILE_SIZE;
    filesystem_cat(ctx, toks[1], start, size);
}

static void cmd_write(thread_context* ctx, char** toks, int n, bool overwrite) {
    if (n < 3) {
        fprintf(ctx->output_file, "Usage: %s <file> <text>\n", overwrite ? "write" : "append");
        return;
    }
    // Rejoin tokens logic: ensures "write file.txt hello world" works
    char data[INPUT_BUF] = {0};
    for (int i = 2; i < n; i++) {
        strncat(data, toks[i], INPUT_BUF - strlen(data) - 1);
        if (i + 1 < n) { 
            strncat(data, " ", INPUT_BUF - strlen(data) - 1); 
        }
    }
    uint32_t size = (uint32_t)strlen(data);
    filesystem_write_file(ctx, toks[1], data, size, overwrite);
}

static void cmd_seek(thread_context* ctx, char** toks, int n) {
    if (n < 3) { 
        fprintf(ctx->output_file, "Usage: seek <file> <pos>\n"); 
        return; 
    }
    uint32_t pos = (uint32_t)atoi(toks[2]);
    filesystem_move_cursor(ctx, toks[1], pos);
}

/* --- The Thread Worker --- 
 * This is the heart of the refactor. Each thread opens its own files
 * and processes commands while locking the shared tree.
 */
void* session_worker(void* arg) {
    thread_context* ctx = (thread_context*)arg;
    char line[INPUT_BUF];
    char* toks[MAX_TOKS];
    char in_path[64], out_path[64];

    // Build paths for input_threadX.txt and output_threadX.txt
    sprintf(in_path, "input_thread%d.txt", ctx->thread_id);
    sprintf(out_path, "output_thread%d.txt", ctx->thread_id);

    FILE* fin = fopen(in_path, "r");
    ctx->output_file = fopen(out_path, "w");

    if (!fin || !ctx->output_file) {
        fprintf(stderr, "Thread %d: Error opening session files.\n", ctx->thread_id);
        return NULL;
    }

    while (fgets(line, sizeof(line), fin)) {
        int n = tokenise(line, toks, MAX_TOKS);
        if (n == 0) continue;

        char* cmd = toks[0];

        // CRITICAL SECTION: Lock the mutex before touching the shared tree
        pthread_mutex_lock(&ctx->fs->lock);

        if (strcmp(cmd, "ls") == 0)           cmd_ls(ctx, toks, n);
        else if (strcmp(cmd, "cd") == 0)      cmd_cd(ctx, toks, n);
        else if (strcmp(cmd, "touch") == 0)   filesystem_touch(ctx, toks[1]);
        else if (strcmp(cmd, "mkdir") == 0)   filesystem_mkdir(ctx, toks[1]);
        else if (strcmp(cmd, "rm") == 0)      filesystem_rm(ctx, toks[1]);
        else if (strcmp(cmd, "mv") == 0)      filesystem_mv(ctx, toks[1], (n >= 3 ? toks[2] : ".."));
        else if (strcmp(cmd, "cat") == 0)     cmd_cat(ctx, toks, n);
        else if (strcmp(cmd, "write") == 0)   cmd_write(ctx, toks, n, true);
        else if (strcmp(cmd, "append") == 0)  cmd_write(ctx, toks, n, false);
        else if (strcmp(cmd, "seek") == 0)    cmd_seek(ctx, toks, n);
        else if (strcmp(cmd, "tree") == 0)    filesystem_print(ctx->fs, ctx->output_file);

        pthread_mutex_unlock(&ctx->fs->lock);
    }

    fclose(fin);
    fclose(ctx->output_file);
    return NULL;
}

/* --- Main Orchestrator --- */
int main(int argc, char* argv[]) {
    // If a thread count is provided, run simulation (old test1 logic)
    if (argc == 2) {
        int k = atoi(argv[1]);
        if (k <= 0) {
            printf("Usage: %s <num_threads>\n", argv[0]);
            return 1;
        }

        filesystem* fs = filesystem_create();
        pthread_t* threads = malloc(sizeof(pthread_t) * k);
        thread_context* contexts = malloc(sizeof(thread_context) * k);

        printf("Starting Warmi FS Simulation with %d threads...\n", k);

        for (int i = 0; i < k; i++) {
            contexts[i].thread_id = i;
            contexts[i].fs = fs;
            contexts[i].curr_depth = 1;
            contexts[i].curr_dirs[0] = fs->root;
            pthread_create(&threads[i], NULL, session_worker, &contexts[i]);
        }

        for (int i = 0; i < k; i++) {
            pthread_join(threads[i], NULL);
        }

        // Final persistence
        filesystem_save(fs, SAVE_PATH);
        printf("Simulation finished. Final tree saved and printed below:\n");
        filesystem_print(fs, stdout);

        free(threads);
        free(contexts);
        filesystem_destroy(fs);
    } 
    // If no argument, just load and print (old test2 logic)
    else {
        filesystem* fs = filesystem_load(SAVE_PATH);
        if (fs) {
            printf("Loading existing filesystem from %s:\n", SAVE_PATH);
            filesystem_print(fs, stdout);
            filesystem_destroy(fs);
        } else {
            printf("Usage: %s <num_threads>\n", argv[0]);
        }
    }

    return 0;
}
