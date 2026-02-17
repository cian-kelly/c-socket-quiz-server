# TCP Socket Quiz Server

A client-server quiz application written in C, demonstrating low-level TCP socket programming using POSIX APIs on Unix/Linux systems. The server hosts a randomised quiz on Unix programming concepts, communicating with connected clients over a reliable TCP byte stream.

---

## Overview

This project implements a classic client-server architecture using POSIX TCP stream sockets. The server listens for incoming client connections, delivers a five-question quiz drawn randomly from a bank of 43 questions, evaluates answers in real time, and returns a final score. The client handles user interaction and communicates with the server over a persistent TCP connection.

Key concepts covered:

- POSIX socket API (`socket`, `bind`, `listen`, `accept`, `connect`)
- Low-level stream I/O using `read()` / `write()` system calls
- Network byte order conversion (`htons`, `inet_addr`)
- TCP connection lifecycle management
- Defensive stream I/O handling (partial reads/writes, `EINTR` recovery)

---

## Project Structure

```
.
├── client.c        # Client application - user interaction and socket I/O
├── server.c        # Server application - connection handling, quiz logic
├── socket_io.h     # Shared socket read/write utility functions
├── QUIZDB_H.h      # Quiz question and answer database (43 questions)
├── Makefile        # Build system with dependency tracking and debug support
└── .gitignore      # C-specific gitignore
```

---

## Architecture

### Server (`server.c`)
The server runs as a persistent loop, accepting one client connection at a time:

1. Creates a TCP stream socket and binds to a specified IPv4 address and port
2. Sets `SO_REUSEADDR` to allow rapid restarts without `Address already in use` errors
3. Calls `accept()` in an infinite loop, blocking until a client connects
4. Uses `getnameinfo()` to resolve and log the client's hostname and service
5. Sends a welcome message and waits for the client to start or quit
6. If the client starts the quiz, selects 5 unique random questions using `rand()` seeded with `time(0)`
7. Sends each question, reads the client's answer, evaluates it against the answer key, and sends feedback
8. Sends a final score message and closes the connection

### Client (`client.c`)
The client provides a simple terminal interface:

1. Connects to the server at a specified IP address and port
2. Displays the welcome message received from the server
3. Validates user input - only accepts `Y` (start) or `q` (quit)
4. If the quiz starts, loops through 5 questions: displaying each, reading user input, sending the answer, and printing server feedback
5. Displays the final score and closes the connection cleanly

### Shared I/O (`socket_io.h`)
Both programs share `readFromSocket()` and `writeToSocket()` utility functions that:
- Always transmit exactly `BUFSIZE` (500) bytes per message, ensuring fixed-length framing
- Handle partial reads and writes in a loop, as required for reliable TCP I/O
- Correctly handle `EINTR` (signal interruption) by retrying the system call

---

## Build

### Prerequisites
- GCC
- GNU Make
- Linux / Unix system

### Targets

```bash
make              # Build both client and server (default)
make debug        # Build with debug symbols (-g3, -O0) for use with gdb
make clean        # Remove object files and dependency files
make cleanall     # Remove object files, dependency files, and executables
make rebuild      # Full clean and rebuild from scratch
```

### Run

```bash
# Terminal 1 - start the server
make run-server

# Terminal 2 - connect a client
make run-client
```

The default IP and port are `127.0.0.1:8080`. These can be overridden at the command line:

```bash
make run-server IP=0.0.0.0 PORT=9000
make run-client IP=192.168.1.10 PORT=9000
```

Or run the binaries directly:

```bash
./server <IPv4 address> <port>
./client <IPv4 address> <port>
```

---

## Example Session

```
$ ./client 127.0.0.1 8080

Welcome to Unix Programming Quiz!
The quiz comprises of five questions posed to you one after the other.
You have only one attempt to answer a question.
Your final score will be sent to you after conclusion of the quiz.
To start the quiz, press Y and <enter>.
To quit quiz, press q and <enter>.

Y

Question 1:
Is fork() a system call? (Y or N)
Y

Correct Answer

Question 2:
What does the malloc() library function return on failure? (Hint: A four-letter word)
NULL

Correct Answer

...

Your quiz score is 4/5. Goodbye!
```

---

## Technical Notes

- **Fixed-length framing:** All messages are padded/read to exactly `BUFSIZE` (500) bytes. This is a deliberate design choice to simplify the protocol and avoid the need for length-prefix or delimiter-based framing.
- **Single-client model:** The server handles one client at a time. Concurrency (e.g. via `fork()` or `pthread`) is a natural extension.
- **Random question selection:** The server uses a deduplication array to guarantee no question is repeated within a session.
- **Compiler flags:** `-Wall -Wextra -std=c11 -D_GNU_SOURCE` are enforced at build time for strict standards compliance and maximum warning coverage.
