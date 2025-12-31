# Circular Road Maintenance Optimization

## 📝 Problem Description
This system manages the maintenance costs of a circular bypass road divided into $N$ sections. Two companies are assigned continuous segments of the road such that the difference in their total maintenance costs is minimized. The problem is complicated by "Dynamic Pricing," where the maintenance cost of any section can change at any point in history.

## 🚀 Key Features
- **Circular Buffer Logic:** Implements algorithms to handle continuous assignments on a circular road (e.g., a segment spanning from the last index back to the first).
- **Time-Series Data Management:** Tracks cost histories for each section using dynamic arrays, allowing for cost aggregation over arbitrary date intervals.
- **Prefix Sum Optimization:** Uses a prefix sum array on a doubled range to efficiently calculate segment costs in $O(1)$ time after initial setup.
- **ISO Date Parsing:** Includes a custom Gregorian date-to-day converter to handle intervals and leap years accurately.
- **Robust Memory Management:** Utilizes dynamic memory allocation (`malloc`/`realloc`) to handle up to 10,000 road sections and 300,000 cost updates.

## 🛠️ Concepts Used
- Circular Array Algorithms
- Time-Interval Aggregation
- Prefix Sums (Dynamic Programming pattern)
- Data Structures (Structs & Dynamic Arrays)
- ISO 8601 Date Validation
