# Railroad Track Combinatorics

## 📝 Problem Description
This program solves a variation of the "Change-making problem" or "Linear Diophantine Equation" problem. Given two railroad tracks of different fixed lengths, the program calculates how many ways they can be combined to reach an exact target distance without cutting any tracks.

## 🚀 Key Features
- **Dual Operation Modes:** 
    - `+` Mode: Lists every specific combination of tracks.
    - `-` Mode: Simply counts the total number of valid combinations (optimized for large datasets).
- **Mathematical Logic:** Uses a search algorithm to find non-negative integer solutions to the equation: `(a * x) + (b * y) = distance`.
- **Large Number Support:** Uses `long long` data types to handle very large distances and track lengths.
- **Strict Input Validation:** Ensures track lengths are positive, distinct, and that the distance is non-negative.

## 🛠️ Concepts Used
- Brute-force Search Algorithms
- Linear Diophantine Equations
- Input Parsing (handling character + integer pairs)
- Error Handling & Program Termination
