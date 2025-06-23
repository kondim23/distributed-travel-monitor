# Travels Virus Management System

## Overview
The Travels Virus Management System is a robust, distributed C application for managing and monitoring travel requests and vaccination records across multiple countries. It leverages advanced low-level system programming to ensure secure, efficient, and scalable inter-process communication and data management. The architecture consists of a parent process (`travelMonitor`) and multiple child processes (`Monitor`), each responsible for a subset of countries, enabling parallel processing and high availability.

## Key Technologies & Architecture
- **Process Management:** The parent process dynamically spawns multiple Monitors using `fork()` and `exec()`, distributing workload and improving fault tolerance.
- **Inter-Process Communication:** Communication between the parent and each Monitor is achieved via named pipes (FIFOs), with dedicated read/write channels for each process pair. Pipes are created using `mkfifo()` and accessed with `open()`, supporting both blocking and non-blocking I/O via the `O_NONBLOCK` flag.
- **Low-Level I/O:** All data transfer between processes utilizes the `read()` and `write()` system calls, with careful handling of partial reads/writes and custom buffering to ensure data integrity and performance.
- **Signal Handling:** The system responds to signals such as `SIGINT`, `SIGQUIT`, `SIGCHLD`, and `SIGUSR1` for process lifecycle management, dynamic updates, and graceful shutdown. Signal handlers are implemented using `sigaction()` for reliability.
- **Multiplexed I/O:** The parent process employs `poll()` or `select()` to monitor multiple pipes concurrently, enabling responsive, non-blocking communication with all Monitors.
- **Filesystem Traversal:** Input data is organized in a hierarchical directory structure. The system uses `opendir()`, `readdir()`, and `fopen()` to efficiently traverse directories and process input files.
- **Custom Data Structures:** High-performance hash tables, skip lists, and bloom filters are implemented from scratch for fast lookups, efficient updates, and memory efficiency.
- **Custom Communication Protocol:** All inter-process messages are prefixed with their size, allowing for robust message parsing and reliable handling of variable-length data.

## System Logic & Data Flow
1. **Initialization:**
   - The parent parses command-line arguments and scans the input directory for country subdirectories.
   - Named pipes are created, and the parent forks the specified number of Monitors, assigning countries in a round-robin, alphabetical manner.
   - Each Monitor processes its assigned countries' input files, building in-memory data structures (hash tables, skip lists, bloom filters), and sends bloom filters to the parent for centralized queries.
2. **Command Handling:**
   - The parent enters a command loop, accepting user commands from standard input.
   - For travel requests, the parent checks the relevant bloom filter. If a possible vaccination is indicated, it queries the responsible Monitor for confirmation and vaccination date.
   - For statistics and status queries, the parent aggregates data from all Monitors or directs the request to the appropriate Monitor(s).
   - For new vaccination records, the parent signals the relevant Monitor, which scans for new files, updates its data, and sends updated bloom filters.
3. **Inter-Process Communication:**
   - All communication is performed via named pipes, using a custom protocol with message size headers. The parent uses polling to handle multiple Monitors concurrently, avoiding blocking on slow or busy processes.
4. **Signal and Error Handling:**
   - The system responds to signals for graceful shutdown, log file creation, and dynamic updates. If a Monitor terminates unexpectedly, the parent detects this and spawns a replacement, reassigning the relevant countries and restoring system state.
5. **Shutdown:**
   - On exit, the parent sends termination signals to all Monitors, waits for their completion, writes a comprehensive log file, and releases all resources.

## Custom Data Structures in Practice
- **Hash Tables:** Used by both parent and Monitors to store and retrieve citizen records, vaccination information, and virus data. Each citizen or virus is mapped to its corresponding data entry, enabling O(1) average-case access for updates and queries.
- **Skip Lists:** Employed within each Monitor to maintain ordered lists of vaccination records and support efficient range queries, such as retrieving all citizens vaccinated within a specific date range. Skip lists allow for fast insertion, deletion, and search operations, making them ideal for dynamic datasets.
- **Bloom Filters:** Each Monitor maintains a bloom filter for every virus it manages. These probabilistic structures allow the parent to quickly check if a citizen might be vaccinated for a specific virus, reducing unnecessary inter-process queries. Bloom filters are sent from Monitors to the parent during initialization and after updates, enabling fast, approximate membership tests with minimal memory usage.

These structures are optimized for low memory overhead and high concurrency, supporting the system's scalability and responsiveness.

## Low-Level C Engineering Highlights
This project is a showcase of advanced, low-level C programming and systems engineering, including:
- **Direct System Call Usage:** All process management, I/O, and inter-process communication are implemented using direct system calls, providing full control over resource management and performance.
- **Manual Buffering and Message Framing:** Data sent over pipes is manually buffered and framed, with explicit message size headers, ensuring robust and efficient communication even with partial reads/writes.
- **Custom Memory Management:** Dynamic memory allocation and deallocation are handled explicitly, with careful attention to avoiding leaks and fragmentation.
- **Signal-Safe Programming:** Signal handlers are written to be safe and reliable, using only async-signal-safe functions and ensuring consistent state transitions.
- **POSIX Compliance:** The code adheres to POSIX standards, ensuring portability across UNIX-like systems.
- **No External Libraries:** All data structures and algorithms are implemented from scratch, without relying on external or STL libraries, demonstrating deep understanding of algorithmic and systems concepts.
- **Error Handling and Robustness:** All system calls are checked for errors, and the system is designed to recover gracefully from failures, including process crashes and I/O interruptions.

## Build & Usage Instructions
1. Ensure a POSIX-compliant environment with GCC and `make` installed.
2. Compile the project by running:
   ```sh
   make
   ```
   This will generate the `travelMonitor` and `Monitor` executables.
3. Launch the system with:
   ```sh
   ./travelMonitor -m <numMonitors> -b <bufferSize> -s <sizeOfBloom> -i <input_dir>
   ```
   - `<numMonitors>`: Number of Monitor processes to spawn.
   - `<bufferSize>`: Buffer size (in bytes) for pipe communication.
   - `<sizeOfBloom>`: Bloom filter size (in bytes) for vaccination status checks.
   - `<input_dir>`: Directory containing country subdirectories and input files.

## Input Data Format
- The `<input_dir>` should contain one subdirectory per country (e.g., `Greece/`, `Italy/`).
- Each country subdirectory contains one or more text files (e.g., `Greece-1.txt`, `Italy-2.txt`).
- Each file contains lines with the following format:
  ```
  <citizenID> <firstName> <lastName> <country> <age> <virusName> <YES/NO> [<dateVaccinated>]
  ```
  - `citizenID`: Unique identifier for the citizen (string or number)
  - `firstName`, `lastName`: Names without spaces
  - `country`: Country name (matches subdirectory)
  - `age`: Integer (1-120)
  - `virusName`: Name of the virus (alphanumeric, may include '-')
  - `YES/NO`: Vaccination status
  - `dateVaccinated`: Date in `DD-MM-YYYY` format (present only if `YES`)

## Expanded Usage Instructions
4. Once running, the system accepts the following commands via standard input:
   - `/travelRequest <citizenID> <date> <countryFrom> <countryTo> <virusName>`
     - Checks if a citizen is vaccinated for a specific virus before travel. If not vaccinated, prints a rejection message. If possibly vaccinated (per bloom filter), queries the responsible Monitor for confirmation and prints acceptance or rejection with reason.
   - `/travelStats <virusName> <date1> <date2> [country]`
     - Displays the number of travel requests, accepted, and rejected for a virus in a date range, optionally filtered by country.
   - `/addVaccinationRecords <country>`
     - Notifies the Monitor to process new files in the specified country's directory, updating all relevant data structures and bloom filters.
   - `/searchVaccinationStatus <citizenID>`
     - Queries all Monitors for a citizen's vaccination status and prints all available information (including per-virus status and vaccination dates).
   - `/exit`
     - Gracefully terminates the system, kills all Monitors, writes a log file, and releases all resources.

   Each command provides clear feedback and output as specified in the assignment. For detailed command formats and expected outputs, refer to the assignment or in-source documentation.

5. On exit or signal, log files are created in the `logs/` directory, named `log_file.<pid>`, listing all countries, total requests, accepted, and rejected counts.

## Project Structure
```
Travels_Virus_Management_SysPro2_2020-2021/
├── src/                  # Source code for travelMonitor and Monitor
│   ├── travelMonitor.c
│   ├── monitor.c
│   ├── ... (other .c/.h files)
├── include/              # Header files
│   ├── ...
├── data/            # Example input data (country subdirectories and files)
│   ├── Greece/
│   │   ├── Greece-1.txt
│   │   └── ...
│   ├── Italy/
│   │   └── ...
│   └── ...
├── runtime
│   ├── pipes/              # pipe communication
│   ├── logs/               # logging
├── Makefile                # Build instructions
├── README.md               # Project documentation
├── scripts/                # create test data
├── test/                   # testing commands 
```
- All source code is organized under `src/` and `include/` for clarity and modularity.
- Input data for testing is placed in `data/` as described above.
- Log files are generated in the `logs/` directory.
- The `Makefile` automates compilation of all components.

## Design Highlights
- **Fault Tolerance:** The parent process automatically detects and replaces failed Monitors, ensuring continuous operation.
- **Scalability:** Workload is distributed across multiple Monitors, supporting large datasets and high request volumes.
- **Resource Management:** All dynamically allocated memory and system resources are properly released on shutdown.
- **Extensibility:** The modular design and custom data structures facilitate future enhancements and feature additions.

For further technical details, please refer to the in-source documentation and comments.
