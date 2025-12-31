# Gregorian Calendar & Date Manipulation

## 📝 Problem Description
This project involves implementing a robust calendar utility to identify and count "Friday the 13th" occurrences within the Gregorian calendar system. The challenge requires handling complex date validation, leap year rules (including the 4000-year exception), and determining weekdays for any given date since 1900.

## 🚀 Key Features
- **Date Validation:** Implements comprehensive checks for valid years, months, and days, including varying month lengths and leap years.
- **Weekday Calculation:** Uses **Zeller’s Congruence algorithm** to mathematically determine the day of the week for any date.
- **Interval Processing:** Efficiently counts specific occurrences between two dates across potentially massive time intervals.
- **Chronological Navigation:** Includes functions to find the nearest preceding or succeeding Friday the 13th from a reference point.

## 🛠️ Concepts Used
- **Zeller's Congruence** (Algorithmic Weekday Determination)
- **Custom Gregorian Rules** (Special leap year logic for years divisible by 4000)
- **C Structs:** Using custom types (`TDATE`) to pass structured data.
- **Defensive Programming:** Handling null pointers and invalid state transitions.
- **Conditional Compilation:** Uses `#ifndef __PROGTEST__` blocks to separate development code from testing environments.
