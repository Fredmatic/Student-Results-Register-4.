// =====================================================================
//  FILE: StudentResultsRegister.cpp
//  SCENARIO 6: Student Results Register
//  PRIMARY DATA STRUCTURE (Task A1): Dynamic array with capacity doubling
//  COURSE: C++17, compiles with g++ / clang++ (no STL containers used in A1)
//
//  Build:  g++ -std=c++17 -Wall -Wextra -fsanitize=address StudentResultsRegister.cpp -o register
//  Run:    ./register
// =====================================================================

#include <iostream>  // std::cout, std::cerr, std::cin — console I/O
#include <fstream>   // std::ifstream, std::ofstream — file load/save (A1 requirement)
#include <string>    // std::string, std::getline — student name field
#include <sstream>   // std::istringstream — parse one CSV line
#include <stdexcept> // std::out_of_range, std::invalid_argument — safe rejection (A1)
#include <chrono>    // std::chrono::high_resolution_clock — timing experiment (A4)
#include <random>    // std::mt19937 — random pivot for quicksort (A2, odd reg. no.)
#include <algorithm> // std::swap, std::stable_sort — swap helper + comparison test only
#include <cstdlib>   // std::atoi — not strictly needed; kept for portability
#include <iomanip>   // std::setw, std::fixed — formatted output tables

// ---------------------------------------------------------------------
// 1. RECORD TYPE (A1: "store records of a struct type that you define")
// ---------------------------------------------------------------------
#include "DynamicArray.h"

// ---------------------------------------------------------------------
// 3. SORTING (A2): templated sorts taking a comparator
//    - insertionSort  : required (stable, O(n^2) worst)
//    - mergeSort      : required if last reg. digit is EVEN (stable, O(n log n))
//    - quickSortRand  : required if last reg. digit is ODD  (random pivot, O(n log n) avg)
// ---------------------------------------------------------------------

// Insertion sort — stable because equal keys are never swapped past each other.
template <typename T, typename Compare>
void insertionSort(T *a, int n, Compare less)
{
    for (int i = 1; i < n; ++i)
    {                 // start from second element
        T key = a[i]; // element to insert
        int j = i - 1;
        while (j >= 0 && less(key, a[j]))
        { // shift larger elements right
            a[j + 1] = a[j];
            --j;
        }
        a[j + 1] = key; // place key in correct slot
    }
}

// Merge sort helpers — stable because left run wins ties.
template <typename T, typename Compare>
void mergeRuns(T *a, T *tmp, int lo, int mid, int hi, Compare less)
{
    int i = lo, j = mid + 1, k = lo; // i: left, j: right, k: output
    while (i <= mid && j <= hi)
    { // merge two sorted halves
        if (less(a[j], a[i]))
            tmp[k++] = a[j++]; // right strictly smaller
        else
            tmp[k++] = a[i++]; // left wins ties -> stable
    }
    while (i <= mid)
        tmp[k++] = a[i++]; // drain left
    while (j <= hi)
        tmp[k++] = a[j++]; // drain right
    for (int t = lo; t <= hi; ++t)
        a[t] = tmp[t]; // copy back
}

template <typename T, typename Compare>
void mergeSortRec(T *a, T *tmp, int lo, int hi, Compare less)
{
    if (lo >= hi)
        return;                              // 0/1 element: sorted
    int mid = lo + (hi - lo) / 2;            // avoid overflow
    mergeSortRec(a, tmp, lo, mid, less);     // sort left half
    mergeSortRec(a, tmp, mid + 1, hi, less); // sort right half
    mergeRuns(a, tmp, lo, mid, hi, less);    // merge
}

template <typename T, typename Compare>
void mergeSort(T *a, int n, Compare less)
{
    if (n < 2)
        return;                           // nothing to do
    T *tmp = new T[n];                    // scratch buffer (O(n) space)
    mergeSortRec(a, tmp, 0, n - 1, less); // recursive sort
    delete[] tmp;                         // free scratch (no leak)
}

// Quicksort with random pivot — expected O(n log n), NOT stable.
template <typename T, typename Compare>
int partitionRand(T *a, int lo, int hi, Compare less, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> dist(lo, hi); // inclusive range
    int p = dist(rng);                               // random pivot index
    std::swap(a[p], a[hi]);                          // move pivot to end
    T pivot = a[hi];
    int i = lo - 1; // boundary of < pivot region
    for (int j = lo; j < hi; ++j)
    {
        if (less(a[j], pivot))
        { // strictly less than pivot
            ++i;
            std::swap(a[i], a[j]);
        }
    }
    std::swap(a[i + 1], a[hi]); // place pivot in final slot
    return i + 1;               // return pivot position
}

template <typename T, typename Compare>
void quickSortRec(T *a, int lo, int hi, Compare less, std::mt19937 &rng)
{
    if (lo >= hi)
        return;                                  // base case
    int p = partitionRand(a, lo, hi, less, rng); // split around random pivot
    quickSortRec(a, lo, p - 1, less, rng);       // sort left partition
    quickSortRec(a, p + 1, hi, less, rng);       // sort right partition
}

template <typename T, typename Compare>
void quickSortRand(T *a, int n, Compare less)
{
    if (n < 2)
        return;                           // nothing to do
    std::mt19937 rng(12345);              // fixed seed -> reproducible timings
    quickSortRec(a, 0, n - 1, less, rng); // start recursion
}

// ---------------------------------------------------------------------
// 4. SEARCHING (A2): iterative binary search on a sorted array
//    Returns an index containing the matching key, or -1.
//    Precondition: array is sorted by the same field/key ordering used by
//    elementLessKey and keyLessElement.
// ---------------------------------------------------------------------
template <typename T, typename Key, typename ElementKeyCompare, typename KeyElementCompare>
int binarySearch(const T *a, int n, const Key &key,
                 ElementKeyCompare elementLessKey, KeyElementCompare keyLessElement)
{
    int lo = 0, hi = n - 1; // inclusive search window
    while (lo <= hi)
    {                                 // iterative (no recursion)
        int mid = lo + (hi - lo) / 2; // overflow-safe midpoint
        if (elementLessKey(a[mid], key))
            lo = mid + 1; // key is to the right
        else if (keyLessElement(key, a[mid]))
            hi = mid - 1; // key is to the left
        else
            return mid; // match found
    }
    return -1; // absent key
}

// ---------------------------------------------------------------------
// 5. COMPARATORS (A2: "so that records can be sorted by any field")
// ---------------------------------------------------------------------
struct ById
{
    bool operator()(const StudentRecord &x, const StudentRecord &y) const { return x.id < y.id; }
};
struct ByScore
{
    bool operator()(const StudentRecord &x, const StudentRecord &y) const { return x.score < y.score; }
};
struct ByName
{
    bool operator()(const StudentRecord &x, const StudentRecord &y) const { return x.name < y.name; }
};

// Lightweight benchmark record: isolates sorting overhead from string-copy cost.
struct BenchmarkRecord { double score; };
struct ByBenchmarkScore {
    bool operator()(const BenchmarkRecord &x, const BenchmarkRecord &y) const { return x.score < y.score; }
};

// ---------------------------------------------------------------------
// 6. MAIN — A3 test evidence + A4 timing experiment
// ---------------------------------------------------------------------
#ifndef STUDENT_RESULTS_REGISTER_TEST
int main(int argc, char *argv[])
{
    // === Pick your O(n log n) sort by the LAST DIGIT of your reg. number ===
    const int REG_LAST_DIGIT = 8;                     // Registration number ends in 8
    const bool USE_MERGE = (REG_LAST_DIGIT % 2 == 0); // even -> merge, odd -> quick

    try
    {
        // ---------- A1: load at least 10 sample records ----------
        DynamicArray<StudentRecord> reg;  // empty structure (boundary case)
        reg.loadFromFile("students.txt"); // must contain >= 10 rows
        std::cout << "Loaded " << reg.size()
                  << " records (capacity " << reg.getCapacity() << ")\n";

        // ---------- A2: sort by meaningful key (score) ----------
        insertionSort(reg.raw(), reg.size(), ByScore{}); // stable sort by score
        std::cout << "After insertionSort (by score):\n";
        for (int i = 0; i < reg.size(); ++i)
            std::cout << "  " << reg.at(i).id << "  " << reg.at(i).name
                      << "  " << reg.at(i).score << "\n";

        // Binary search on sorted array (by score key)
        double probe = 75.0; // key to find
        int idx = binarySearch(reg.raw(), reg.size(), probe,
                               [](const StudentRecord &a, double k)
                               { return a.score < k; },
                               [](double k, const StudentRecord &a)
                               { return k < a.score; });
        std::cout << "binarySearch(score=" << probe << ") -> "
                  << (idx >= 0 ? "found at index " + std::to_string(idx) : "absent") << "\n";

        // Absent-key boundary case
        probe = 999.0;
        idx = binarySearch(reg.raw(), reg.size(), probe,
                           [](const StudentRecord &a, double k)
                           { return a.score < k; },
                           [](double k, const StudentRecord &a)
                           { return k < a.score; });
        std::cout << "binarySearch(score=" << probe << ") -> "
                  << (idx >= 0 ? "found" : "absent (expected)") << "\n";

        // ---------- A2: O(n log n) sort chosen by reg. digit ----------
        if (USE_MERGE)
        {
            std::cout << "Chosen O(n log n) sort: mergeSort\n";
            mergeSort(reg.raw(), reg.size(), ByScore{});
        }
        else
        {
            std::cout << "Chosen O(n log n) sort: quickSortRand (random pivot)\n";
            quickSortRand(reg.raw(), reg.size(), ByScore{});
        }

        // ---------- A1: save updated records back ----------
        reg.saveToFile("students_out.txt");
        std::cout << "Saved updated records to students_out.txt\n";

        // A4 benchmark is run separately so ordinary execution and tests finish quickly.
        if (argc > 1 && std::string(argv[1]) == "--benchmark")
        {
        // ---------- A4: timing experiment n = 1000..64000, 3 runs each ----------
        std::cout << "\n=== A4 TIMING: insertionSort vs "
                  << (USE_MERGE ? "mergeSort" : "quickSortRand") << " ===\n";
        std::mt19937 rng(42); // reproducible random data
        std::uniform_real_distribution<double> dist(0.0, 100.0);
        std::cout << std::setw(8) << "n"
                  << std::setw(14) << "insertion(ms)"
                  << std::setw(14) << "nlogn(ms)"
                  << std::setw(12) << "ins x"
                  << std::setw(12) << "nlogn x\n";
        double prevIns = 0, prevFast = 0; // for doubling-ratio column
        for (int n = 1000; n <= 64000; n *= 2)
        {
            double insTotal = 0, fastTotal = 0; // accumulate 3 runs
            for (int run = 0; run < 3; ++run)
            {
                DynamicArray<BenchmarkRecord> a, b; // independent arrays of lightweight sortable records
                for (int i = 0; i < n; ++i)
                {
                    BenchmarkRecord r{dist(rng)}; // same generated key in both arrays
                    a.push_back(r);
                    b.push_back(r);
                }
                auto t0 = std::chrono::high_resolution_clock::now();
                insertionSort(a.raw(), a.size(), ByBenchmarkScore{}); // time insertion sort
                auto t1 = std::chrono::high_resolution_clock::now();
                if (USE_MERGE)
                    mergeSort(b.raw(), b.size(), ByBenchmarkScore{});
                else
                    quickSortRand(b.raw(), b.size(), ByBenchmarkScore{});
                auto t2 = std::chrono::high_resolution_clock::now();
                insTotal += std::chrono::duration<double, std::milli>(t1 - t0).count();
                fastTotal += std::chrono::duration<double, std::milli>(t2 - t1).count();
            }
            double insAvg = insTotal / 3.0; // average of 3 runs
            double fastAvg = fastTotal / 3.0;
            double insRatio = prevIns > 0 ? insAvg / prevIns : 0;     // ~4 expected
            double fastRatio = prevFast > 0 ? fastAvg / prevFast : 0; // ~2 expected
            std::cout << std::setw(8) << n
                      << std::setw(14) << std::fixed << std::setprecision(3) << insAvg
                      << std::setw(14) << fastAvg
                      << std::setw(12) << insRatio
                      << std::setw(12) << fastRatio << "\n";
            prevIns = insAvg;
            prevFast = fastAvg;
            std::cout << std::flush;
        }
        } // end optional benchmark
        else
        {
            std::cout << "\nA4 benchmark not run. Use ./register --benchmark to run it.\n";
        }
    }
    catch (const std::exception &ex)
    { // A1: reject invalid ops safely
        std::cerr << "FATAL: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
#endif // STUDENT_RESULTS_REGISTER_TEST
