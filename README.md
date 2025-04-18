# 🗂️ Simple File Explorer (Windows, C)

A command-line file explorer written in C for Windows. Perform basic file operations like listing, creating, deleting, renaming, searching, and copying files or directories — all from the terminal. Includes colored outputs, fun quotes, a basic loading animation, and logs every operation to a log file.

---

## 📋 Features

- List files and directories  
- Change current directory  
- Create, delete, and rename files or folders  
- Search for files by name pattern  
- View file properties (size, last modified)  
- Copy files  
- Console color-coded output and feedback  
- Fun quote on startup  
- Operation logging to `file_explorer_log.txt`

---

## 🛠️ Requirements

- Windows OS  
- C compiler (like MinGW or MSVC)

---

## 🔧 How to Compile

Using GCC (MinGW):

```bash
gcc fileexp.c -o fileexp.exe
