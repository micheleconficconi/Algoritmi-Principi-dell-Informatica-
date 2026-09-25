# Pastry Shop Management System (C11)

## Overview
This repository contains a highly optimized pastry shop management system developed as the final project for the "Algorithms and Data Structures" course at Politecnico di Milano. The system processes discrete-time events (restocks, orders, recipe management, and courier dispatches) via standard input and output, strictly utilizing standard C11 without any external libraries or multithreading.

## Core Features & Data Structures
To meet rigorous execution time and memory constraints, the system implements several data structures and algorithms from scratch:
* **Custom Hash Tables (with Chaining):** Employed for the `Warehouse` and `Recipe Book` to guarantee \(\mathcal{O}(1)\) average time complexity for lookups and insertions.
* **Min-Heaps:** Utilized to efficiently track and extract the earliest expiring ingredient batches (\(\mathcal{O}(\log N)\)), as well as to manage the queue of ready orders prioritized by weight.
* **MergeSort Algorithm:** Implemented to sort dispatchable orders based on courier weight limits and chronological arrival in \(\mathcal{O}(N \log N)\) time.
* **FIFO Queues (Linked Lists):** Managed pending orders awaiting ingredient restocks.

## Tech Stack & Profiling
* **Language:** Pure C11 (`gcc -Wall -Werror -std=gnu11 -O2`)
* **Memory & Performance Profiling:** Valgrind (Memcheck, Massif) and Callgrind were used to track memory leaks, optimize dynamic allocations, and eliminate execution bottlenecks.

## Results
The implementation was evaluated through automated test batteries measuring both machine time and memory peak footprint, successfully processing millions of discrete events and passing all constraints with maximum grading.
