#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

struct StudentRecord
{                       // One row of the student results register
    int id;             // Unique student ID (search/sort key candidate)
    std::string name;   // Student full name
    std::string course; // Course code, e.g. "CS101"
    double score;       // Final mark 0..100 (meaningful sort key)
};

// ---------------------------------------------------------------------
// 2. PRIMARY DATA STRUCTURE (A1): DynamicArray<T>
//    - Own class template in a header-style section (single file for submission)
//    - Manual memory management: new[] / delete[]
//    - Capacity doubling on push_back
//    - Deep copy constructor + destructor  (rule of three)
// ---------------------------------------------------------------------
template <typename T>
class DynamicArray
{
private:
    T *data;      // Pointer to heap block holding the records
    int count;    // Number of records currently stored
    int capacity; // Allocated slots in the heap block

    // Grow the buffer to double its size. Called only when full.
    void grow()
    {
        int newCap = (capacity == 0) ? 1 : capacity * 2; // doubling (or start at 1)
        T *newData = new T[newCap];                      // allocate new block
        for (int i = 0; i < count; ++i)                  // copy existing records
            newData[i] = data[i];                        // shallow copy of POD-ish struct
        delete[] data;                                   // free old block (no leak)
        data = newData;                                  // point to new block
        capacity = newCap;                               // update capacity
    }

public:
    // --- Constructor: start empty, no allocation yet ---
    DynamicArray() : data(nullptr), count(0), capacity(0) {}

    // --- Deep copy constructor (A1 requirement) ---
    DynamicArray(const DynamicArray &other)
        : data(nullptr), count(other.count), capacity(other.capacity)
    {
        if (capacity > 0)
        {
            data = new T[capacity]; // allocate same capacity
            for (int i = 0; i < count; ++i)
                data[i] = other.data[i]; // deep copy each element
        }
    }

    // --- Copy assignment (rule of three: avoids double-free) ---
    DynamicArray &operator=(const DynamicArray &other)
    {
        if (this == &other)
            return *this; // self-assignment guard
        T *newData = nullptr;
        if (other.capacity > 0)
        {
            newData = new T[other.capacity]; // allocate first (exception safety)
            for (int i = 0; i < other.count; ++i)
                newData[i] = other.data[i];
        }
        delete[] data;  // free old memory
        data = newData; // adopt new block
        count = other.count;
        capacity = other.capacity;
        return *this;
    }

    // --- Destructor (A1 requirement: correct memory management) ---
    ~DynamicArray()
    {
        delete[] data;  // free heap block once
        data = nullptr; // avoid dangling pointer
    }

    // --- Append with capacity doubling ---
    void push_back(const T &value)
    {
        if (count == capacity)
            grow();            // double when full
        data[count++] = value; // store and advance count
    }

    // --- Remove at index; shift left; reject invalid index safely ---
    void removeAt(int index)
    {
        if (index < 0 || index >= count) // boundary check
            throw std::out_of_range("removeAt: index out of range");
        for (int i = index; i < count - 1; ++i) // shift elements left
            data[i] = data[i + 1];
        --count; // one fewer record
        // Shrink when very sparse (optional but good practice; keeps capacity >= 1)
        if (capacity > 4 && count < capacity / 4)
        {
            int newCap = capacity / 2;
            T *newData = new T[newCap];
            for (int i = 0; i < count; ++i)
                newData[i] = data[i];
            delete[] data;
            data = newData;
            capacity = newCap;
        }
    }

    // --- Element access with bounds checking (safe rejection) ---
    T &at(int index)
    {
        if (index < 0 || index >= count)
            throw std::out_of_range("at: index out of range");
        return data[index];
    }
    const T &at(int index) const
    { // const overload for read-only use
        if (index < 0 || index >= count)
            throw std::out_of_range("at: index out of range");
        return data[index];
    }

    // --- Simple accessors ---
    int size() const { return count; }           // O(1)
    int getCapacity() const { return capacity; } // O(1), used in tests
    bool empty() const { return count == 0; }    // O(1)

    // --- Linear search by predicate (A1 search helper) ---
    template <typename Pred>
    int findIndex(Pred pred) const
    {
        for (int i = 0; i < count; ++i) // scan all records
            if (pred(data[i]))
                return i; // first match
        return -1;        // not found
    }

    // --- Load records from a text file (CSV: id,name,course,score) ---
    void loadFromFile(const std::string &filename)
    {
        std::ifstream in(filename); // open for reading
        if (!in)
            throw std::runtime_error("cannot open input file: " + filename);
        std::string line;
        while (std::getline(in, line))
        { // read one CSV line
            if (line.empty())
                continue;                // skip blank lines
            std::istringstream ss(line); // parse fields
            std::string idStr, name, course, scoreStr;
            if (!std::getline(ss, idStr, ','))
                continue;
            if (!std::getline(ss, name, ','))
                continue;
            if (!std::getline(ss, course, ','))
                continue;
            if (!std::getline(ss, scoreStr, ','))
                continue;
            StudentRecord r;         // build one record
            r.id = std::stoi(idStr); // may throw on bad data (safe)
            r.name = name;
            r.course = course;
            r.score = std::stod(scoreStr);
            push_back(r); // append to dynamic array
        }
    }

    // --- Save records back to the same format ---
    void saveToFile(const std::string &filename) const
    {
        std::ofstream out(filename); // open for writing (truncates)
        if (!out)
            throw std::runtime_error("cannot open output file: " + filename);
        for (int i = 0; i < count; ++i)
        { // write every record
            out << data[i].id << ',' << data[i].name << ','
                << data[i].course << ',' << data[i].score << '\n';
        }
    }

    // --- Raw pointer for sorting/searching helpers (internal use) ---
    T *raw() { return data; }
    const T *raw() const { return data; }
};


#endif // DYNAMIC_ARRAY_H
