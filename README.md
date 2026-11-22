# Lab 3: Investigating Process Lifecycles and OS Interaction

This lab demonstrates how C programs interact with the Linux kernel as
**processes** — querying their own process identity, controlling their
execution time, and reporting success or failure back to the OS shell
via **exit codes**.

## Lab Contents

| Task | File | Concept Demonstrated |
|------|------|---------------------|
| 1 | `task1_alive.c` | Long-running process monitored with `ps` |
| 2 | `task2_identity.c` | Process identity via `getpid()` / `getppid()` |
| 3 | `task3_exit.c` | Exit codes and shell feedback (`$?`) |
| 4 | `task4_input.c` | Standard I/O streams (stdin/stdout) |
| 5 | `task5_control.c` | Conditional execution and termination |

## How to Build and Run

Requirements: Linux, `gcc`.

```bash
# Compile all tasks
gcc task1_alive.c -o task1
gcc task2_identity.c -o task2
gcc task3_exit.c -o task3
gcc task4_input.c -o task4
gcc task5_control.c -o task5

# Task 1: run in background and monitor from a second terminal
./task1 &
ps aux | grep task1

# Task 2: verify PID/PPID against the OS
./task2 &
ps -p <PID> -o pid,ppid,cmd

# Tasks 3-5: interactive; check the exit code after each run
./task3 ; echo $?
./task4
./task5 ; echo $?
```

## Key System Calls Used

| System Call / Mechanism | Purpose |
|---|---|
| `getpid()` | Returns the calling process's unique Process ID (PID) |
| `getppid()` | Returns the Parent Process ID (PPID) |
| `sleep(seconds)` | Suspends the process for a given time (state `S`, sleeping) |
| `return N` from `main()` | Hands an exit code to the kernel; `0` = success, non-zero = error |
| `$?` | Shell variable holding the last command's exit code |
| `ps` | Userspace tool that reads process info from the `/proc` filesystem |

## Documentation

See [`LAB_REPORT.md`](./LAB_REPORT.md) for the full write-up: theoretical
concepts, flow diagrams, and terminal-output screenshots for every
verification step.
