# Lab 3 Report: Investigating Process Lifecycles and OS Interaction

**Course:** Operating Systems — Lecture 3 Lab
**Objective:** Bridge high-level C code and low-level OS process management by
writing programs that query their own process identity, control execution
time, and report success/failure back to the shell.

---

## 1. Theoretical Concepts

### 1.1 What Is a Process?
A **process** is a program in execution. When you type `./task1`, the shell
(a process itself) asks the Linux kernel (via the `fork()`/`execve()` system
calls) to create a new process: the kernel allocates a PID, a Process
Control Block (PCB), an address space, and file descriptors. The program's
C code then runs *inside* this process and can query the kernel about itself.

### 1.2 Process State
While running, a process moves between states in the kernel's scheduler:

```
            +----------------+
            |     NEW        |
            +-------+--------+
                    |
                    v
            +----------------+      sleep()/I/O wait
   +------->|     RUNNING    |------------------+
   |        +-------+--------+                  |
   |                |                            v
   |          timer interrupt             +----------------+
   |                |                     |     WAITING    |
   |                v                     |  (interruptible|
   |        +----------------+            |   sleep, S)    |
   +--------|     READY      |<-----------+-------+--------+
            +----------------+                    | sleep() returns
                                                    |
            +----------------+                      |
            |  TERMINATED    |<---------------------+ return N from main()
            +----------------+
```
We observe the **WAITING (S)** state directly in Task 1 when `ps` shows the
process as `S` while it sits inside `sleep(1)`.

### 1.3 PID and PPID
- **PID (Process ID):** a unique integer the kernel assigns to every process.
- **PPID (Parent Process ID):** the PID of the process that created it — for
  programs launched from a terminal, this is the shell (`bash`).

### 1.4 Exit Codes
When `main()` returns, the value becomes the process's **exit status**, which
the kernel stores and the parent (shell) reads. By convention:
- `0` → success
- non-zero (e.g., `1`) → failure

The shell exposes the last exit code in the special variable `$?`, enabling
**conditional execution** in scripts (`cmd && next`, `cmd || fix`).

### 1.5 Standard I/O Streams
Every process is born with three open file descriptors:

| fd | Stream | Default target | C interface |
|----|--------|----------------|-------------|
| 0 | stdin | keyboard | `scanf()`, `getchar()` |
| 1 | stdout | terminal | `printf()` |
| 2 | stderr | terminal | `perror()`, `fprintf(stderr, ...)` |

`scanf` blocks reading fd 0; `printf` writes fd 1 — the OS delivers the data.

---

## 2. Task 1 — The Long-Running Process

**File:** `task1_alive.c`

### Code
```c
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("I am starting...\n");
    for(int i = 1; i <= 30; i++) {
        sleep(1);          // Pauses execution for 1 second
    }
    printf("I am finished.\n");
    return 0;
}
```

### Flow Diagram
```
  +-------+   fork/exec by shell    +-------------+
  | shell | ----------------------> | process     |
  +-------+                         | ./task1     |
                                    +------+------+
                                           |
                                     printf("I am starting...")
                                           |
                                           v
                              +------------------------+
                              | loop i = 1..30         |
                              |   sleep(1)  --> state S|
                              +------------------------+
                                           |
                                     printf("I am finished.")
                                           |
                                           v
                                    return 0 (exit status 0)
```

### Execution & Verification
```bash
$ gcc task1_alive.c -o task1
$ ./task1 &
[1] 1602
$ ps aux | grep task1
kimi  1602  0.0  0.0  2476 1328 pts/0 S 11:38 0:00 ./task1
```
The `S` in the `ps` output is the **interruptible sleep** state — the process
exists and is alive but is not consuming CPU; it is waiting for the
`sleep(1)` timer to expire. After 30 seconds the program prints
`I am finished.` and the job control reports `[1]+ Done ./task1`.

![Task 1 terminal output](screenshots/task1.png)

### Observation
In enterprise servers, daemons run for days or months in exactly this state.
`ps`, `top`, and `/proc/<pid>/` are the standard tools used to monitor a live
process without disturbing it.

---

## 3. Task 2 — Process Identity (PID and PPID)

**File:** `task2_identity.c`

### Code
```c
#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t my_pid  = getpid();    // current Process ID
    pid_t my_ppid = getppid();   // Parent Process ID
    printf("My PID is: %d\n", my_pid);
    printf("My Parent PID is: %d\n", my_ppid);
    printf("Sleeping for 20 seconds...\n");
    sleep(20);
    return 0;
}
```

### Flow Diagram
```
 process ./task2                 Linux kernel
 +--------------+              +--------------------+
 | getpid()  ---+------------> | looks up current   |
 |              |              | task_struct->pid   |
 | <--+ pid=1697| <----------- +--------------------+
 |              |
 | getppid() ---+------------> | task_struct->parent |
 | <--+ ppid=181| <----------- +--------------------+
 +--------------+
```

### Execution & Verification
```bash
$ gcc task2_identity.c -o task2
$ ./task2 &
[1] 1697
My PID is: 1697
My Parent PID is: 181
Sleeping for 20 seconds...
$ ps -p 1697 -o pid,ppid,cmd
    PID    PPID CMD
   1697     181 ./task2
```

![Task 2 terminal output](screenshots/task2.png)

### Observation
The PID/PPID printed by the program **match exactly** what the kernel reports
through `ps`. This proves `getpid()`/`getppid()` are thin wrappers over real
kernel state. The PPID (181 here) is the shell that launched the program —
when the shell exits first, the child is reparented to PID 1 (`init`).

---

## 4. Task 3 — Exit Codes and OS Feedback

**File:** `task3_exit.c`

### Code
```c
#include <stdio.h>

int main() {
    int num;
    printf("Enter a number (positive for success, negative for fail): ");
    scanf("%d", &num);

    if (num > 0) {
        printf("Success\n");
        return 0;   // Tell OS: Success
    } else {
        printf("Failure\n");
        return 1;   // Tell OS: Error
    }
}
```

### Flow Diagram
```
 +------------------+
 | read num (scanf) |
 +--------+---------+
          |
    +-----v-----+        no
    | num > 0 ? +----------------> printf("Failure")
    +-----+-----+                  return 1
          | yes                         |
          v                             v
   printf("Success")            exit status 1
   return 0                      --> $? == 1
        |
        v
 exit status 0
 --> $? == 0
```

### Execution & Verification — Scenario A (positive input)
```bash
$ ./task3
Enter a number (positive for success, negative for fail): 5
Success
$ echo $?
0
```

### Scenario B (negative input)
```bash
$ ./task3
Enter a number (positive for success, negative for fail): -5
Failure
$ echo $?
1
```

![Task 3 terminal output](screenshots/task3.png)

### Observation
The shell variable `$?` mirrors the value returned from `main()` — this is
the exact mechanism scripts use to branch: `./task3 && echo ok` only prints
`ok` when the exit code is `0`.

---

## 5. Task 4 — Standard I/O Streams

**File:** `task4_input.c`

### Code
```c
#include <stdio.h>

int main() {
    char name[50];
    printf("Enter your name: ");      // writes to stdout (fd 1)
    scanf("%s", name);                // reads from stdin (fd 0)
    printf("Hello, %s! Welcome to OS Class.\n", name);
    return 0;
}
```

### Flow Diagram
```
  keyboard                OS                process ./task4
     |                    |                       |
     |  "John\n"         |   read(fd=0)          v
     +------------------->+--------------->  scanf fills name[]
                                                  |
                                                  v
                                        printf("Hello, John!...")
                                                  |
                                                  v
                            write(fd=1) <---------+
                                 |
                                 v
                            terminal screen
```

### Execution
```bash
$ gcc task4_input.c -o task4
$ ./task4
Enter your name: John
Hello, John! Welcome to OS Class.
```

![Task 4 terminal output](screenshots/task4.png)

### Observation
Because stdin/stdout are just file descriptors, the same program works
unchanged with redirection: `./task4 < names.txt > greeting.txt` reads the
name from a file and writes the greeting to another file — the C code never
changes, only the OS-level plumbing does.

---

## 6. Task 5 — Conditional Execution and Termination

**File:** `task5_control.c`

### Code
```c
#include <stdio.h>
#include <unistd.h>

int main() {
    int choice;
    printf("Current PID: %d\n", getpid());
    printf("Do you want to continue? (1 for Yes, 0 for No): ");
    scanf("%d", &choice);

    if (choice == 1) {
        printf("Continuing...\n");
        sleep(5);
        return 0;   // Success
    } else {
        printf("Exiting...\n");
        return 1;   // Failure/Abort
    }
}
```

### Flow Diagram
```
 +------------------+
 | print PID        |
 | scanf choice     |
 +--------+---------+
          |
    +-----v-------+        != 1
    | choice == 1 +------------------> printf("Exiting...")
    +-----+-------+                    return 1  (abort)
          | yes                              |
          v                                  v
   printf("Continuing...")          exit code 1
   sleep(5)                              |
   return 0                        shell: $? = 1
        |
        v
 exit code 0
 shell: $? = 0
```

### Execution & Verification
```bash
# Run 1: choose Yes
$ ./task5
Current PID: 1764
Do you want to continue? (1 for Yes, 0 for No): 1
Continuing...
$ echo $?
0

# Run 2: choose No
$ ./task5
Current PID: 1781
Do you want to continue? (1 for Yes, 0 for No): 0
Exiting...
$ echo $?
1
```

![Task 5 terminal output](screenshots/task5.png)

### Observation
The user's decision is encoded as a **binary contract with the OS**: continue
→ exit code 0 (pipeline proceeds); abort → exit code 1 (pipeline halts).
Automation tools (`make`, CI systems, shell `&&` chains) all rely on this
convention.

---

## 7. Conclusion

This lab demonstrated that C programs do not exist in a vacuum; they are
active processes managed by the Linux kernel. Across the five tasks we:

1. Kept a process alive for 30 s and observed its **sleeping state** with `ps`.
2. Queried and **cross-verified** PID/PPID via `getpid()`/`getppid()` against `ps`.
3. Returned **exit codes** and read them back through the shell's `$?`.
4. Used **stdin/stdout streams** for user interaction.
5. Combined identity, branching, and exit codes into a **controlled
   termination** contract with the OS.

By utilizing system calls like `getpid()`, `sleep()`, and return codes, we
successfully bridged the gap between high-level C code and low-level OS
process management.
