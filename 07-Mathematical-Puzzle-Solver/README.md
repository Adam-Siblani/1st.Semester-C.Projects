# Mathematical Expression Puzzle Solver

## 📝 Problem Description
This program is an automated solver for mathematical puzzles involving digit sequences. Given a string of up to 10 digits, the program explores every possible combination of arithmetic operators (`+`, `-`, `*`, `/`) and parentheses to reach a target integer value. The challenge requires maintaining digit order while allowing for number concatenation (e.g., treating "1" and "2" as "12").

## 🚀 Key Features
- **Exhaustive Recursive Search:** Uses a backtracking algorithm to generate all valid mathematical expressions from a fixed digit sequence.
- **Complex Expression Evaluation:** Implements a parser that respects standard mathematical operator precedence and handles nested parentheses.
- **Dual Query Modes:**
    - `?` Mode: Performs a full search and prints every valid mathematical string.
    - `#` Mode: Optimized counting mode that calculates the total number of solutions without string formatting overhead.
- **Number Concatenation:** Logically handles the merging of adjacent digits into multi-digit integers.
- **Robust Error Handling:** Validates digit sequence length, character integrity, and command syntax.

## 🛠️ Concepts Used
- **Advanced Recursion & Backtracking**
- **Operator Precedence & Associativity**
- **Expression Tree Traversal**
- **Memoization / Dynamic Programming** (for optimized `#` queries)
- **String Parsing and Buffer Management**
