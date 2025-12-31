# Graph-Based Round Trip Pathfinder

## 📝 Problem Description
This project implements a pathfinding engine for a travel agency to identify all possible "Round Trips" within a specified budget. Given a directed graph of city connections with varying costs, the program finds every unique cycle that starts and ends at a specific city without revisiting any intermediate locations.

## 🚀 Key Features
- **Graph Traversal:** Implements a **Depth-First Search (DFS)** with backtracking to explore all valid paths in a directed graph.
- **Dynamic Cost Constraint:** Efficiently prunes search branches where the cumulative cost exceeds the user-defined `costMax`.
- **Linked List Management:** Results are returned as a custom single-linked list, requiring careful manual memory management and pointer manipulation.
- **Data Serialization:** Parses raw string-based connection data (format: `cost: cityA -> cityB`) into an internal adjacency representation.
- **Custom Sorting:** The resulting paths are automatically sorted in ascending order of total travel cost.

## 🛠️ Concepts Used
- **Directed Graphs & Cycles**
- **Backtracking Algorithms**
- **Manual Memory Management** (Linked Lists of dynamic structs)
- **String Parsing & Buffer Validation**
- **Time Complexity Optimization** (Pruning search paths)
