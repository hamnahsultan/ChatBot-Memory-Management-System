# 🤖 ChatBot Memory Management System

A C-based chatbot memory management system developed as part of the **CS200 – Introduction to Programming** course. This project demonstrates dynamic memory management, modular software design, and the implementation of core data structures while maintaining memory safety and data consistency.

## 📖 Overview

The ChatBot Memory Management System simulates the memory layer of a chatbot by storing, managing, validating, and compacting user messages. The project emphasizes proper ownership of dynamically allocated memory and synchronization between multiple data structures.

The system supports:

- 💬 Message creation and storage
- 📝 Message parsing and content filtering
- 📚 History management
- 🗑️ Message deletion and history reset
- 🔍 History validation
- 🧹 Memory compaction
- 🧪 Interactive debugging through a developer console

---

## ✨ Features

- Dynamic memory allocation using `malloc`, `realloc`, and `free`
- Dynamic array implementation
- Singly linked list implementation
- Message lifecycle management
- Parser with censorship and policy enforcement
- Memory compaction subsystem
- History validation and consistency checking
- Interactive command-line debug console
- Modular project organization

---

## 🏗️ Project Architecture

```
                User Input
                     │
                     ▼
               Parser Module
                     │
                     ▼
             Message Creation
                     │
                     ▼
             History Management
          ┌──────────────────────┐
          │ Dynamic Array View   │
          │ Linked List View     │
          └──────────────────────┘
                     │
                     ▼
        Validation / Compaction / Reset
```

---

## 📂 Project Structure

```
ChatBot-Memory-Management-System/
│
├── include/
│   ├── compaction.h
│   ├── history.h
│   ├── message.h
│   └── parser.h
│
├── src/
│   ├── compaction.c
│   ├── history.c
│   ├── message.c
│   └── parser.c
│
├── tests/
│
├── main.c
├── Makefile
├── grade.sh
```

---

## ⚙️ Building the Project

This project was developed and tested inside a standardized Linux development environment using **Docker**.

From PowerShell:

```powershell
# Navigate to the project directory
cd "path\to\ChatBot-Memory-Management-System"

# Launch the provided Docker environment
docker run -it --rm -v "${PWD}:/home/...." <university-provided-container>
```

Inside the container:

```bash
cd /home/....
```

Useful Makefile commands:

```bash
make help          # Display available commands
make parser        # Run parser tests
make message       # Run message tests
make history       # Run history tests
make compaction    # Run compaction tests
make full          # Run the complete system demo

make v-history     # Run history tests with Valgrind
make v-full        # Run the full system with Valgrind

make clean         # Remove generated binaries
```

---

## 🧪 Testing

The project includes automated test harnesses for each subsystem.

Testing covers:

- Parser correctness
- Message creation
- History operations
- Memory compaction
- Full system integration
- Memory leak detection using Valgrind

---

## 🧠 Concepts Demonstrated

- Dynamic Memory Allocation
- Pointers and Pointer Ownership
- Structures
- Linked Lists
- Dynamic Arrays
- Modular Programming
- Defensive Programming
- Memory Leak Prevention
- Data Structure Synchronization
- Separation of Concerns

---

## 📚 Academic Note

This project was completed as a **semester programming assignment** for **CS200 – Introduction to Programming**.

To ensure a consistent development and grading environment across all students, the project was developed and tested inside a **Docker container provided by the university**. The container standardized the compiler, libraries, build tools, and testing environment used throughout the assignment.

The source code in this repository represents my implementation of the project requirements.

---

## 👩‍💻 Author

**Hamnah Sultan**

---

## 📄 License

This repository is shared for educational and portfolio purposes.

Please do not copy or submit this work as your own for academic credit.
