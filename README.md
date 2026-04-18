# WArmi - Tree Based Filesystem

## Authors
- Wasi Ullah
- Armaghan Mehmood Shams (200KG)

**GitHub:** [github.com/PAKIWASI/warmi_filesys](https://github.com/PAKIWASI/warmi_filesys)

---

## Overview

WArmi is a tree-based in-memory filesystem implemented in C, built as Operating Systems Lab 11. It simulates core file management operations — creating, deleting, moving, reading, and writing files and directories — all organized as a tree structure. The filesystem supports persistence by serializing the entire tree to a single binary file.

---

## Directory Structure

```
warmi/
├── include/
│   ├── filesystem.h      # Filesystem API declarations
│   └── tree_node.h       # Tree node struct and API declarations
├── src/
│   ├── filesystem.c      # Filesystem operations implementation
│   ├── tree_node.c       # Tree node operations implementation
│   └── main.c            # CLI entry point
├── data/
│   └── warmi.bin         # Persistent binary save file (generated on run)
├── build/
│   └── main.exe
└── README.md
```

---

## Internal Design

The filesystem is represented as an **n-ary tree**:

- **Branch nodes** (`NODE_BRANCH`) represent directories. Each branch holds an array of up to 256 child pointers.
- **Leaf nodes** (`NODE_LEAF`) represent files. Each leaf holds a 4KB data buffer, a size counter, and a cursor position.
- The `filesystem` struct wraps the tree with a **depth stack** (`curr_dirs`) that acts like a call stack for navigation — `cd` pushes/pops from it, so the current directory is always `curr_dirs[curr_depth - 1]`.

```
root/
├── file1.txt   (NODE_LEAF)
├── file2.txt   (NODE_LEAF)
└── dir1/       (NODE_BRANCH)
    └── wasi.txt (NODE_LEAF)
```

**Persistence** is done by a depth-first recursive binary serialization of the tree into a single `.bin` file. Each node writes its type, name, and either its file data or its child count followed by its children. Loading reconstructs the tree in the same order.

---

## Building

```bash
gcc src/main.c src/filesystem.c src/tree_node.c -Iinclude -o warmi
mkdir -p data
```

---

## Modes of Execution

WArmi has two ways to use it.

### 1. Interactive CLI

Run the executable with no arguments to enter the interactive shell. On startup it automatically loads `./data/warmi.bin` if one exists, or creates a fresh filesystem if not. Your state is saved automatically on `exit`.

```bash
./warmi
```

The prompt shows your current path and updates as you navigate:

```
warmi:/> mkdir projects
warmi:/> cd projects
warmi:/projects> touch notes.txt
warmi:/projects> write notes.txt hello world
warmi:/projects> cat notes.txt
warmi:/projects> cd ..
warmi:/> tree
warmi:/> exit
```

Full list of CLI commands:

| Command | Description |
|---|---|
| `ls [dirname]` | List current directory, or a named subdirectory |
| `cd <dirname\|..>` | Change into a directory, or go up one level |
| `tree` | Print the full filesystem tree from root |
| `touch <n>` | Create a new empty file |
| `mkdir <n>` | Create a new directory |
| `rm <n>` | Delete a file or directory |
| `mv <n> <dest\|..>` | Move a file/dir into a subdirectory, or up one level |
| `cat <file> [start size]` | Read a file. Defaults to reading the whole file |
| `write <file> <text>` | Overwrite a file with text |
| `append <file> <text>` | Append text at the current cursor position |
| `seek <file> <pos>` | Move the write cursor to a byte position |
| `save [path]` | Save the filesystem (default: `./data/warmi.bin`) |
| `load [path]` | Load a filesystem from file |
| `help` | Show the command menu |
| `exit / quit` | Save and quit |

### 2. C API (Programmatic)

You can use the filesystem directly in C code by calling the API functions. This is useful for scripting a sequence of operations or embedding the filesystem in a larger program.

```c
#include "filesystem.h"

int main(void)
{
    filesystem* fs = filesystem_create();

    filesystem_touch(fs, "notes.txt");
    filesystem_mkdir(fs, "projects");
    filesystem_cd(fs, "projects");
    filesystem_touch(fs, "todo.txt");

    filesystem_write_file(fs, "todo.txt", "buy milk\n", 9, true);

    // append more at byte 9
    filesystem_move_cursor(fs, "todo.txt", 9);
    filesystem_write_file(fs, "todo.txt", "write code\n", 11, false);

    filesystem_cat(fs, "todo.txt", 0, 1000);

    filesystem_cd(fs, "..");
    filesystem_print(fs);

    filesystem_save(fs, "./data/warmi.bin");
    filesystem_destroy(fs);

    // reload and verify
    filesystem* fs2 = filesystem_load("./data/warmi.bin");
    filesystem_print(fs2);
    filesystem_destroy(fs2);

    return 0;
}
```

---

## API Reference

### Filesystem Lifecycle

| Function | Description |
|---|---|
| `filesystem_create()` | Creates a new filesystem with an empty root directory |
| `filesystem_destroy(fs)` | Recursively frees the entire tree and the filesystem struct |

### Navigation

| Function | Description |
|---|---|
| `filesystem_ls(fs, dirname)` | Lists contents of current dir, or a named subdirectory. Pass `NULL` for current dir |
| `filesystem_cd(fs, dirname)` | Change into a subdirectory. Pass `".."` to go up one level |
| `filesystem_mv(fs, name, move_to)` | Move a file or directory into a subdirectory, or `".."` to move it up |

### File & Directory Creation/Deletion

| Function | Description |
|---|---|
| `filesystem_touch(fs, name)` | Create a new empty file in the current directory |
| `filesystem_mkdir(fs, name)` | Create a new subdirectory in the current directory |
| `filesystem_rm(fs, name)` | Delete a file or directory (recursive) from the current directory |

### File Reading & Writing

| Function | Description |
|---|---|
| `filesystem_cat(fs, filename, start, size)` | Print `size` bytes of a file starting at byte `start` |
| `filesystem_write_file(fs, filename, data, size, overwrite)` | Write data to a file. `overwrite=true` resets cursor to 0 first; `false` writes at the current cursor position |
| `filesystem_move_cursor(fs, filename, pos)` | Move the write cursor to a specific byte position in a file |

### Persistence

| Function | Description |
|---|---|
| `filesystem_save(fs, path)` | Serialize the entire filesystem tree to a binary file |
| `filesystem_load(path)` | Deserialize and reconstruct a filesystem from a binary file |

### Visualization

| Function | Description |
|---|---|
| `filesystem_print(fs)` | Pretty-print the full filesystem tree from root, showing file sizes |

---

## Limits

| Constant | Value | Meaning |
|---|---|---|
| `MAX_DEPTH` | 256 | Maximum directory nesting depth |
| `MAX_CHILDREN` | 256 | Maximum files/dirs per directory |
| `MAX_FILE_SIZE` | 4096 bytes | Maximum size of a single file |
| `MAX_NAME_SIZE` | 128 bytes | Maximum length of a file/directory name |
