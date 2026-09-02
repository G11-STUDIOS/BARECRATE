# RescueBox

**RescueBox** is a small, lightweight Linux rescue environment written in C.

It provides a minimal command interface and executes programs from its own `bin/` directory. RescueBox is designed to combine **BusyBox applets**, standalone Linux utilities, and user-provided programs into one customizable rescue toolbox.

The launcher itself stays small while the tools in `bin/` provide most of the functionality.

---

# Features

## Minimal Command Interface

RescueBox provides a simple interactive prompt:

```text
> 
```

Enter a command, RescueBox runs it, waits for it to finish, and returns to the prompt.

Example:

```text
> echo hello
hello
> ls
BOX  Makefile  bin  box  rescuebox.c
>
```

This keeps the launcher simple while still allowing it to operate as an interactive rescue environment.

---

## Execute Programs From `bin/`

RescueBox searches its own `bin/` directory for commands.

```text
RescueBox
    |
    v
  bin/
    |
    +-- ls
    +-- cat
    +-- nano
    +-- git
    +-- curl
    +-- iw
    +-- your-program
```

If a program exists in `bin/` and is executable, RescueBox can launch it.

This means the launcher does not need to know what the program is.

It can be:

* A BusyBox applet
* A standalone Linux executable
* A custom program
* Another utility you add yourself

---

# Add Programs Without Changing the Source

One of the main design goals of RescueBox is **extensibility**.

To add another program, simply put it in `bin/`.

For example:

```bash
cp /path/to/myprogram bin/
chmod +x bin/myprogram
```

Then:

```text
> myprogram
```

No changes to `rescuebox.c` are required.

You can therefore customize RescueBox for different purposes simply by changing the contents of `bin/`.

---

# BusyBox Integration

RescueBox works with BusyBox to provide a large collection of Linux utilities from a single binary.

BusyBox applets can be exposed through symlinks:

```text
bin/
├── ls -> ../box/busybox
├── cat -> ../box/busybox
├── cp -> ../box/busybox
├── mv -> ../box/busybox
├── rm -> ../box/busybox
└── grep -> ../box/busybox
```

Running:

```text
> ls
```

therefore launches the BusyBox implementation of `ls`.

This allows RescueBox to provide many utilities without having to bundle a separate executable for every command.

---

# Standalone Programs

BusyBox does not provide every program.

RescueBox can therefore mix BusyBox with standalone tools.

For example:

```text
bin/
├── ls -> ../box/busybox
├── grep -> ../box/busybox
├── nano
├── git
├── curl
├── gdb
├── iw
└── tcc
```

RescueBox treats them all as executable commands.

There is no special code required for each individual program.

---

# Command Arguments

Arguments are passed directly to the program.

For example:

```text
> echo one two three
one two three
```

You can therefore use normal command-line options:

```text
> ls -la
> git --version
> curl --version
> iw dev
```

The arguments are passed to the executed program through `execv()`.

---

# Built-in `cd`

`cd` is implemented directly inside RescueBox.

This is necessary because a separate child process cannot change the working directory of its parent process.

Example:

```text
> cd /tmp
> pwd
/tmp
```

The directory remains changed for subsequent commands.

---

# Home Directory Expansion

RescueBox supports:

```text
> cd ~
```

which changes to the user's home directory.

It also supports paths such as:

```text
> cd ~/box
```

The home directory is obtained from the `HOME` environment variable.

---

# Parent Directory Navigation

Normal paths such as `..` work through the underlying Linux filesystem:

```text
> cd /home/vladi
> cd ..
> pwd
/home
```

---

# Error Handling

If a command does not exist in `bin/`, RescueBox reports the error instead of crashing:

```text
> asdfghjkl
asdfghjkl: command not found
>
```

Failed `cd` operations are also reported:

```text
> cd does-not-exist
cd: does-not-exist: No such file or directory
```

The RescueBox prompt remains available after errors.

---

# Process Isolation

External programs are launched in a child process using `fork()` and `execv()`.

Conceptually:

```text
RescueBox
    |
    +-- fork()
          |
          +-- child -> execv(program)
          |
          +-- parent -> wait
```

This allows RescueBox to wait for a utility to finish before returning to the prompt.

---

# Waits For Commands

RescueBox waits for the launched program to finish before displaying the next prompt.

For example:

```text
> sleep 5
```

RescueBox waits for `sleep` to finish and then returns:

```text
>
```

This makes command execution predictable and prevents the prompt from immediately appearing while a foreground utility is still running.

---

# Root Operation

RescueBox can be run as root:

```bash
sudo ./BOX
```

Programs launched by RescueBox inherit the privileges of the RescueBox process.

For example:

```text
> whoami
root
```

This is useful for rescue operations involving:

* Filesystems
* Storage devices
* Networking
* System configuration
* Hardware diagnostics
* Recovery operations

RescueBox itself does **not** need to be SUID. It can simply be started with the required privileges.

---

# Networking Support

RescueBox can run networking utilities placed in `bin/`.

For example:

```text
> iw dev
```

can display wireless interfaces and their state.

Other networking tools can be bundled as needed:

```text
ip
iw
ping
curl
wget
tcpdump
```

This makes RescueBox useful for diagnosing network connectivity from a recovery environment.

---

# Filesystem Tools

Because RescueBox can run BusyBox and standalone utilities, it can perform filesystem operations such as:

```text
> mkdir test
> cd test
> touch hello
> ls
> cd ..
> rm -rf test
```

Additional filesystem and disk utilities can be added to `bin/`, such as:

```text
mount
umount
lsblk
fdisk
cfdisk
parted
wipefs
e2fsck
```

This allows the toolbox to be expanded for serious system recovery tasks.

---

# Text Editors and Development Tools

Standalone applications can be bundled alongside BusyBox.

For example:

```text
nano
git
gdb
tcc
```

This means RescueBox can be used not only for recovery, but also for debugging and development when those tools are included.

---

# Self-Locating `bin/` Directory

RescueBox determines the location of its own executable using:

```text
/proc/self/exe
```

It then finds the `bin/` directory relative to itself.

This means you do not have to start RescueBox from its installation directory.

For example, if RescueBox is installed at:

```text
/home/vladi/box/BOX
```

you can run:

```bash
cd /tmp
/home/vladi/box/BOX
```

and RescueBox can still find:

```text
/home/vladi/box/bin/
```

This is important for making the toolbox portable.

---

# No Hardcoded Current Working Directory

RescueBox does not depend on the directory you happen to be standing in when launching it.

The location of the executable is used to find the toolbox.

This allows the user to freely navigate the filesystem without breaking command lookup.

---

# EOF Exit

Pressing `Ctrl+D` sends EOF to the command input.

RescueBox detects this and exits cleanly.

---

# `exit` Command

RescueBox also provides an explicit exit command:

```text
> exit
```

This terminates the RescueBox session.

---

# `help` Command

The built-in help command provides basic information about RescueBox:

```text
> help
```

It displays the available built-in commands and explains where programs are loaded from.

---

# Makefile Build System

RescueBox includes a Makefile.

Build it with:

```bash
make
```

The compiler command is currently equivalent to:

```bash
gcc -O2 -Wall -Wextra rescuebox.c -o BOX
```

If the binary is already up to date, Make avoids unnecessary recompilation.

Clean the build with:

```bash
make clean
```

Rebuild from scratch:

```bash
make clean
make
```

---

# Simple Architecture

RescueBox intentionally keeps its launcher small.

The architecture is essentially:

```text
                RescueBox
                    |
                    v
              Read command
                    |
                    v
              Parse arguments
                    |
          +---------+---------+
          |                   |
          v                   v
       Built-in             bin/<command>
          |                   |
     +----+----+        +-----+------+
     |    |    |        |            |
     cd  help exit    BusyBox     standalone
                         |            |
                         +-----+------+
                               |
                               v
                         Execute program
                               |
                               v
                              wait
                               |
                               v
                              > 
```

The important idea is that **RescueBox does not need to implement every utility itself**.

The launcher provides the environment.

The tools provide the functionality.

---

# Directory Structure

A typical RescueBox installation looks like:

```text
RescueBox/
├── rescuebox.c
├── Makefile
├── BOX
├── bin/
│   ├── ls -> ../box/busybox
│   ├── cat -> ../box/busybox
│   ├── grep -> ../box/busybox
│   ├── nano
│   ├── git
│   ├── curl
│   ├── iw
│   └── ...
└── box/
    └── busybox
```

---

# Custom Toolboxes

Because programs are loaded from `bin/`, different RescueBox installations can contain different tools.

For example, a networking-focused toolbox might contain:

```text
bin/
├── ip
├── iw
├── ping
├── curl
├── wget
└── tcpdump
```

A disk-recovery toolbox could contain:

```text
bin/
├── lsblk
├── fdisk
├── cfdisk
├── parted
├── wipefs
├── mount
├── umount
└── e2fsck
```

A development toolbox could contain:

```text
bin/
├── nano
├── git
├── gdb
└── tcc
```

The same RescueBox launcher can be used for all of them.

---

# Current Limitations

RescueBox is currently an **early-stage project** and is intentionally much simpler than Bash.

The following shell features are not currently implemented:

* Pipes (`|`)
* Input/output redirection (`>`, `<`)
* Shell quoting
* Environment-variable expansion such as `$HOME`
* Command history
* Tab completion
* Job control
* Advanced shell scripting
* Background processes

For example, this currently does **not** behave like a normal shell:

```text
> echo hello > file.txt
```

because output redirection has not yet been implemented.

Similarly:

```text
> ls | grep box
```

does not currently create a pipe.

These features may be added in future versions.

---

# Design Philosophy

RescueBox is designed around a simple idea:

> **Keep the launcher small. Put the functionality in the toolbox.**

Instead of writing a custom implementation of every Linux command, RescueBox delegates those tasks to programs in `bin/`.

This makes the system:

* Small
* Modular
* Extensible
* Easy to customize
* Easy to build
* Useful for recovery and troubleshooting

Adding functionality should generally mean **adding a program to ****`bin/`**** rather than rewriting RescueBox itself.**

---

# Requirements

Currently designed for Linux.

Build requirements:

* GCC
* GNU Make
* Linux
* `/proc` for executable self-location

The bundled programs may have additional dependencies depending on how they were built.

---

# Project Status

**Experimental / Early Development**

The core RescueBox launcher is functional.

Currently working features include:

* Interactive command prompt
* Program execution
* Command arguments
* BusyBox integration
* Standalone executable support
* Extensible `bin/` directory
* Built-in `cd`
* `~` expansion
* Filesystem operations through bundled tools
* Networking tools through bundled tools
* Root operation
* Error handling
* Working-directory persistence
* Self-location through `/proc/self/exe`
* `help`
* `exit`
* Makefile-based builds

The project is still under active development.

---

# Future Goals

Possible future features include:

* Pipes
* Input/output redirection
* Environment-variable expansion
* Command history
* Tab completion
* Better signal handling
* Background processes
* Improved argument parsing
* More rescue utilities
* Better portability
* Automated toolbox setup

---

# License

License information will be added as the project develops.
