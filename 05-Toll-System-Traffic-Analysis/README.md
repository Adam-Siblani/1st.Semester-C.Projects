# Toll System Traffic Analysis & Search Engine

## 📝 Problem Description
This project simulates a high-performance tracking system for secret services using data from toll gate cameras. The program ingests an unordered list of transit reports (Camera ID, License Plate, and Timestamp) and provides a fast search engine. It can identify exact matches or find the chronologically closest "Previous" and "Next" sightings for any specific vehicle.

## 🚀 Key Features
- **$O(\log N)$ Search Performance:** Uses **Binary Search** (`lower_bound` and `upper_bound`) to instantly find vehicle records among thousands of entries.
- **Hierarchical Data Management:** Implements a three-tier data structure: 
    - `Toll`: The global database.
    - `Car`: Unique vehicle records with linked history.
    - `Peep/Tick`: Specific transit events and timestamps.
- **Robust String Parsing:** Handles complex input formatting with vehicle license plates up to 1000 characters and arbitrary whitespace.
- **Advanced Sorting:** Integrates `qsort` to maintain chronological order for each vehicle's transit history.
- **Manual Memory Management:** Efficiently utilizes dynamic memory (`malloc`/`realloc`) to scale with the number of reports.

## 🛠️ Concepts Used
- Binary Search (Time Complexity Optimization)
- Dynamic Memory & Pointers
- Data Structure Nesting (Structs within Structs)
- Custom Date/Time Validation Logic
- Buffer Management and String Tokenization
