---
title: Memory Forensics and Analysis of RAM
date: 2026-03-07
language: 'en'
tags: ['Blog', 'Forensics', 'DFIR']
draft: false
description: "Let's contine being a Forensic Investigator"
---

This is **Part 2** of our Digital Forensics series. If you haven't read the previous part, go ahead and read it right now. That part talks about what **Digital Forensics is**, why we use it and how we start off with an investigation.

In the previous part we only learned how we can **clone an entire disk or a partition** of the disk. But another important part of a machine is the Live Memory, or RAM.

If a system is On, we need to capture the RAM because it can contain information about what processes were running and possibly also encryption keys and other important evidence.

# What is Volatile Memory?

Unlike a Hard Disk or a Storage Device, the RAM is Volatile Memory. This means that when the machine is switched off, all the data is erased from existence.

When the system shuts down:
- All contents in the RAM are **immediately erased**
- Running Processes disappear
- Temporary Artifacts are lost forever

Additionally, launching a new application will change the state of the RAM, creating new processes and erasing existing processes.

Due to the Volatile nature of RAM, investigators mus capture the memory as quickly and securely as possible without altering the RAM state by a lot.

This means that we cannot use tools like `dd` or `Guymager` as these tools are only for disk drivers and not for memory acquisition.

# Memory Acquisition Tools

Due to the difference in the Operating System structure, each OS uses different tools to capture the Live Memory.

On Windows systems, we can use tools like `FTK Imager` and `DumpIt` in order to capture the **full memory dump**.

**Keep in mind that the memory dump size is going to be as big as your RAM size.**
So if you have a machine with 32GB of RAM, the memory dump that is created will also be 32GB.

On Linux it's a little trickier than installing an application and clicking "Capture". Linux typically requires a few **kernel level tools** in order to extract the memory safely.

# Memory Capture in Linux

On Linux systems, the commonly used tools for capturing RAM are:
- **LiME (Linux Memory Extractor) [Link to GitHub](https://github.com/504ensicsLabs/LiME)**
- **AVML (Acquire Volatile Memory for Linux) [Link to GitHub](https://github.com/microsoft/avml)**

Both these tools allow investigators to create a memory dump (snapshot) of the machine's RAM at the given state of time

Now in order to analyze what the RAM Dump says, we use another tool called **Volatility** which we will also be going over here.

## LiME

LiME is a Loadable Kernel Module, meaning that it's a piece of software that can be added to the OS in order to extend the capabilities of the OS.
LiME can also be added and removed dynamically, meaning that we don't have to reboot the system in order for it to work.

Using LiME is very easy and straightforward. Steps included in this are:

1. Clone the Repository:
```shell
git clone https://github.com/504ensicsLabs/LiME.git
```

2. Compile the Module:
```shell
cd LiME/
make
```

The `make` command compiles the source code and generates a `.ko` file, which is a Linux Kernel Module.

3. Load into the Kernel
```shell
sudo insmod lime.ko "path=~/Evidence/dump.mem format=raw"
```

Using the `insmod` command, we load the Linux Kernel Module and we can extract the RAM dump which is stored at the location that you chose while running the command.

## AVML

AVML is another tool that has been developed by Microsoft (ironic) and the advantage it has over LiME is that there is no on machine compilation required.

It's a static binary that has been deployed and can be used on x86_64 machines without any need of compilation or knowing the kernel at all.

The image will also be compressed by default, or uses the LiME output format when uncompressed, so all the output files have the extension `.lime`

1. Installation
Depending on your distro, it's as easy as

```shell
yay -S avml # Arch

sudo apt install avml # Debian
```

2. Usage
```shell
sudo avml output.lime # Uncompressed
sudo avml --compress output.lime.compressed # Compressed
```

Although it's not as easy as clicking a button that says "Acquire", it's still relatively easy to get the memory dump of Linux machines that can be used for analysis

# Analysis Through Volatility

Capturing the RAM Dump is only the first step of Memory Forensics. The real fun comes in when you have to take this gibberish and make sense out of it.

The output that we see is in a very weird file format which cannot be read by any of our standard tools. So for this we have to introduce another tool called as **Volatility**

This is an open source memory forensic framework that allows us to analyze RAM dumps and get artifacts like:
- Running Processes
- Network Connections
- Loaded Kernel Modules

It works on a **plugin-based system**, where each plugin extends the capabilities of the tool further and further, thus making it easier and convenient to extract specific information from the memory image.

## Installing Volatility

The newer version of Volatility, **Volatility 3** is what we'll be using because it is actively maintained by the community

The recommended way to install Volatility is through Python's Package manager, pip

```shell
# Create a Virtual Environment
python3 -m venv .venv
source .venv/bin/activate
pip install volatility3
```

Another way to install is by cloning the source repository and building it from source, which is more reliable in my opinion

```shell
git clone https://github.com/volatilityfoundation/volatility3.git
cd volatility3
pip install -r requirements.txt
```

Now you can run Volatility by running the command
```shell
python3 vol.py -h
```

This command displays all the available plugins and how to use Volatility

### Viewing System information

Before we analyze any of the processes or files, we first have to learn about what the system is, or gather the **basic system information**

Volatility provides the **banners** plugin which helps identify the kernel and system information

```shell
python3 vol.py -f <path_to_memory_dump> banners
```

The banners plugin gives us information on
- Kernel Version
- Operating System Details
- Build Information

Knowing this, we can understand what kind of machine the memory dump came from

### Listing Running Processes

The main reason we take a memory dump is because we want to make a list of all the running processes at that specific moment in time

The **pslist** plugin allows us to list out all the processes that were running

```shell
# Linux RAM Dump
python3 vol.py -f <path_to_memory_dump> linux.pslist

# Windows RAM Dump
python3 vol.py -f <path_to_memory_dump> windows.pslist
```

This command will display the list of processes running along with:
- Process ID (PID)
- Parent Process ID (PPID)
- Process Name
- Process Start Time
- Offset
- Number of Threads
And many other details

We can use this information in order to generate a tree of which process was spawned by which process and we can also identify certain suspicious or unknown processes.

### Process Tree

Like we mentioned earlier, we can make a tree of the processes based on who spawned what, by using the **pstree** banner

```shell
# Linux RAM Dump
python3 vol.py -f <path_to_memory_dump> linux.pstree

# Windows RAM Dump
python3 vol.py -f <path_to_memory_dump> windows.pstree
```

This tree of processes allows us to understand
- Which process has been spawned by which parent process
- Whether a suspicious program was hiding behind a legitimate program

## Why Memory Analysis Matters

Memory Analysis allows us to read the machine as though it was right there and running, that may not be visible on the disk at all.

Artifacts that can be discovered in the RAM can include:
- Malware that never touched the disk (Fileless Malware)
- Decrypted Files or Encryption Keys
- Active Network Sessions
- Scripts that are running in the background

# Wrapping it Up

In this part of our series in becoming a Digital Forensic Investigator, we learnt what a Memory Dump is, how to capture the Live Memory and how we can analyze the memory using tools like Volatility
