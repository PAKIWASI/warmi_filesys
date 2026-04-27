# WArmi FS: Multi-Threading Refactor & Usage Guide

This document explains the transition of the **WArmi Tree-Based File System** from a single-user shell to a multi-threaded simulation engine.

## 1. The Core Refactoring Logic

The refactor changed how the program interacts with the filesystem. Instead of a single global state, we now use a **Thread-Safe Shared Resource** model.

### Key Changes:
* **Shared Root:** All threads point to the same `filesystem*` object.
* **Concurrency Control:** A `pthread_mutex_t` (lock) was added to the `filesystem` struct. Every time a thread wants to `mkdir`, `touch`, or `write`, it must "grab the key," perform the action, and "release the key."
* **Thread Contexts:** Each thread has its own `thread_context` which tracks its unique "Current Working Directory" (CWD). This allows Thread A to be in `/Folder1` while Thread B is in `/Folder2` simultaneously.
* **Isolated Logging:** Each thread redirects its `stdout` (the output you usually see in the terminal) to a private file named `output_threadX.txt`.



---

## 2. How to Use the System

### Step 1: Preparation
Ensure your files are organized in the `src`, `include`, and `data` directories.

### Step 2: Create Simulation Inputs
Create text files in the root folder for each thread. **Note:** Use `mkdir` and `write` (not `echo` inside the simulation files).

```bash
# Example for Thread 0
cat <<EOF > input_thread0.txt
mkdir Armi_Project
cd Armi_Project
touch status.txt
write status.txt Phase 1 Complete
ls
EOF

# Example for Thread 1
cat <<EOF > input_thread1.txt
mkdir Wasi_Project
cd Wasi_Project
touch log.txt
write log.txt Initializing System...
tree
EOF
