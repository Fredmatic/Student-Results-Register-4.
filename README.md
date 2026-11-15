# Student Results Register — Scenario 6 (C++17)

## Registration-number rule
The submitted registration number ends in **8** (even), therefore the selected O(n log n) algorithm is **Merge Sort**. Random-pivot Quick Sort is retained only as an additional reference implementation; it is not the selected coursework algorithm.

## Files
- `DynamicArray.h` — `StudentRecord` and the `DynamicArray<T>` template, including capacity doubling, deep-copy support, bounds checks, and CSV load/save.
- `StudentResultsRegister.cpp` — generic insertion sort, merge sort, reference randomized quicksort, iterative binary search, demonstration program, and optional timing experiment.
- `tests.cpp` — focused checks for edge cases and correctness.
- `students.txt` — ten sample records in `id,name,course,score` format.

## Build and run (Linux / WSL / MSYS2 UCRT64)
```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic StudentResultsRegister.cpp -o register
./register
```
The normal run loads `students.txt`, demonstrates sorting and searching, and writes `students_out.txt`.

## Run tests
```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -fsanitize=address -fno-omit-frame-pointer tests.cpp -o tests
./tests
```

## Run the A4 timing experiment
```bash
g++ -std=c++17 -O2 -DNDEBUG -Wall -Wextra StudentResultsRegister.cpp -o register
./register --benchmark
```
The benchmark performs three runs at each input size from 1,000 through 64,000 using a lightweight record containing the sortable score. This isolates the sorting algorithms from the extra cost of copying student-name and course strings. Insertion Sort is quadratic and can still take a long time at the largest sizes. Record the actual output from your own computer; timing values differ by hardware and compiler settings.

## Data format
Each non-empty line in `students.txt` contains four comma-separated fields: student ID, name, course code, and score. This simple parser assumes names/course fields do not contain commas.

## Important evidence note
The test program is intended to generate actual PASS/FAIL output on the machine used for submission. The report should include screenshots from the student's own terminal and actual benchmark results. Do not claim that a screenshot, sanitizer result, or timing measurement exists unless you have personally run and captured it.
