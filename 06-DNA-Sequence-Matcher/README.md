# DNA Sequence Matcher (Bioinformatics Tool)

## 📝 Problem Description
This program serves as a DNA database search engine. It maintains a collection of DNA sequences (composed of A, T, C, and G bases) and allows for complex pattern matching. Unlike standard string searching, this tool enforces "Triplet Alignment," meaning a sequence is only considered a match if it starts at a valid codon boundary (positions 0, 3, 6, etc.).

## 🚀 Key Features
- **Triplet-Boundary Alignment:** Implements a specialized search algorithm that validates matches based on genetic triplet positioning (Codon boundaries).
- **Frequency-Based Ranking:** Results are automatically sorted in descending order based on a provided frequency parameter (`frq`).
- **Data Filtering:** Implements a "Top 50" limit for search results to ensure performance and readability when handling large datasets.
- **Strict Validation:** Rigorously checks for DNA sequence integrity (base-3 length requirements) and numerical frequency validity.

## 🛠️ Concepts Used
- Pattern Matching on Codon Boundaries
- Dynamic Array Management for Variable-Length DNA Samples
- Custom Sorting Algorithms (`qsort` with struct-based comparators)
- String Tokenization and Decimal Parsing
- Memory Optimization for Large Databases
