# Distributed File Backup Manager

A concurrent, high-performance C++ backup simulation system designed to demonstrate core Operating Systems concepts including multithreading, synchronization primitives, and resource management.

## 🚀 Overview
The **Distributed File Backup Manager** simulates a real-world environment where multiple clients simultaneously back up files to a central server. The system handles concurrent requests using a worker pool architecture and ensures data integrity through advanced synchronization mechanisms.

## 🛠️ Key Features
- **Multithreaded Architecture:** Utilizes a producer-consumer model with multiple client and worker threads.
- **Thread-Safe Queue:** Implements a bounded buffer for backup requests using mutexes and condition variables.
- **Metadata Management:** Protects file indexes and logs using dedicated mutex locks to prevent race conditions.
- **Storage Throttling:** Uses semaphores to limit concurrent disk write operations, preventing I/O degradation.
- **Fault Tolerance:** Includes a recovery module that tracks and retries failed or interrupted backup operations.
- **Real-time Dashboard:** Features an embedded HTTP server providing a web-based GUI to monitor backup progress, worker status, and queue depth.

## 📂 Project Structure
```text
BACKUP_MANAGER
├── backup_manager (binary)
├── client.cpp / client.h
├── dashboard.html
├── gui_dashboard.cpp / gui_dashboard.h
├── main.cpp
├── metadata_manager.cpp / metadata_manager.h
├── recovery.cpp / recovery.h
├── request_queue.cpp / request_queue.h
├── run.txt
├── storage_controller.cpp / storage_controller.h
├── utils.h
├── web_server.h
└── worker.cpp / worker.h
```

## 🏗️ System Architecture
The project is built with a modular structure to enforce separation of concerns:
- **main.cpp**: Entry point and system initialization.
- **request_queue**: Handles thread-safe communication between clients and workers.
- **metadata_manager**: Manages backup logs and record consistency.
- **storage_controller**: Regulates physical I/O using semaphores.
- **gui_dashboard**: Serves the real-time monitoring interface. 

## 💻 Tech Stack
- **Language:** C++17
- **Libraries:** Standard Template Library (Threads, Mutex, Condition Variables, Semaphores)
- **Frontend:** HTML5 & JavaScript (for the dashboard)
- **Build System:** g++ / Makefile

## 📝 Operating Systems Concepts Applied
- Multithreading & Process Coordination
- Mutual Exclusion (Mutexes)
- Semaphores for Resource Allocation
- Condition Variables for Thread Signaling 
- Producer-Consumer Design Pattern [cite: 86]
- Critical Section Management [cite: 87]
