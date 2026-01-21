# Distributed Vaccine Monitor (IPC & Process Management)

This project extends the Vaccine Monitor(https://github.com/nikolasil/vaccineMonitor_1) into a **Multi-Process Distributed System**. It utilizes a Master-Worker architecture to parallelize data processing and uses Named Pipes for robust Inter-Process Communication (IPC).



---

## ## System Architecture

The system is designed to scale across multiple CPU cores by distributing data processing tasks:

### ### 1. Master Process (travelMonitor)
* **Orchestration:** Spawns multiple Worker processes and assigns specific country directories to each.
* **Communication:** Manages a pool of **Named Pipes (FIFOs)**. It implements a custom communication protocol that handles message chunking based on a configurable `bufferSize`.
* **Signal Management:** Uses `sigaction` to handle `SIGCHLD` (for worker recovery), `SIGINT`, and `SIGQUIT` for graceful system shutdown and log generation.

### ### 2. Worker Processes (Monitor)
* **Data Isolation:** Each worker is responsible for a subset of the database, maintaining its own **AVL Trees**, **Skip Lists**, and **Bloom Filters**.
* **Dynamic Updates:** Handles `SIGUSR1` signals to trigger a re-scan of the input directory, allowing for real-time data updates without restarting the system.



---

## ## Communication Protocol

Since Named Pipes are byte-streams, I implemented a robust messaging layer:
* **Send/Receive Logic:** Every message is preceded by its length.
* **Chunking:** Messages larger than the `bufferSize` are fragmented and reassembled by the receiver to ensure data integrity.
* **Termination:** A specific protocol header (length = -1) signals the end of a transmission stream.

---

## ## Commands & Functionality

* **`/travelRequest`:** Master queries the Bloom Filter locally. If a "Maybe" is returned, it performs an IPC request to the specific Worker holding that citizen's record for a definitive Skip List check.
* **`/addVaccinationRecords`:** Signals workers to ingest new files added to the subdirectories dynamically.
* **`/searchVaccinationStatus`:** A distributed search where the Master broadcasts a query and aggregates results from the relevant Worker.
* **`/travelStats`:** Aggregates and filters historical request data stored in the Master's `statsList`.

---

## ## Getting Started

### ### Compilation
```bash
make
```

### 🎓 Academic Context
Developed as part of the Systems Programming course at the National and Kapodistrian University of Athens (UoA). Based on the UC Berkeley CS188 framework.

### Execution
```bash
./travelMonitor -m inputFile.txt -b bufferSize -s sizeOfBloom -i input_dir
```

### Directory Generator
The system includes a Shell script (create_infiles.sh) to generate a distributed directory structure from a flat input file, simulating a real-world file-system database.

## Technical Keypoints
- Synchronization: Non-blocking signal handling and process wait-states.
- Scalability: The number of Worker processes is configurable at runtime.
- Memory Management: Zero-leak policy with complex pointers across multiple child processes.
