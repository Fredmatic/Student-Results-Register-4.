#define STUDENT_RESULTS_REGISTER_TEST
#include "StudentResultsRegister.cpp"
#include <cstdio>

int main()
{
    int passed = 0, failed = 0;
    auto check = [&](const char *name, bool ok) {
        std::cout << (ok ? "PASS" : "FAIL") << " | " << name << '\n';
        ok ? ++passed : ++failed;
    };

    DynamicArray<StudentRecord> a;
    check("Empty structure", a.empty() && a.size() == 0);
    a.push_back({1, "Solo Student", "CS100", 50.0});
    check("One record", a.size() == 1 && a.at(0).id == 1);
    check("Capacity allocated for first insertion", a.getCapacity() == 1);
    a.push_back({10, "Growth Two", "CS100", 55.0});
    check("Capacity doubles when full", a.getCapacity() == 2 && a.size() == 2);
    a.removeAt(1);
    a.push_back({2, "First Equal", "CS100", 70.0});
    a.push_back({3, "Second Equal", "CS100", 70.0});
    insertionSort(a.raw(), a.size(), ByScore{});
    check("Insertion sort preserves duplicate-key order", a.at(1).id == 2 && a.at(2).id == 3);
    mergeSort(a.raw(), a.size(), ByScore{});
    check("Merge sort is ordered and stable for equal scores", a.at(1).id == 2 && a.at(2).id == 3);

    int found = binarySearch(a.raw(), a.size(), 70.0,
        [](const StudentRecord &r, double k) { return r.score < k; },
        [](double k, const StudentRecord &r) { return k < r.score; });
    check("Binary search finds existing key", found >= 0 && a.at(found).score == 70.0);
    int absent = binarySearch(a.raw(), a.size(), 999.0,
        [](const StudentRecord &r, double k) { return r.score < k; },
        [](double k, const StudentRecord &r) { return k < r.score; });
    check("Binary search reports absent key", absent == -1);

    a.removeAt(0);
    check("Remove first record", a.size() == 2 && a.at(0).id == 2);
    a.removeAt(a.size() - 1);
    check("Remove last record", a.size() == 1 && a.at(0).id == 2);
    bool caught = false;
    try { (void)a.at(5); } catch (const std::out_of_range &) { caught = true; }
    check("Reject invalid index", caught);
    caught = false;
    try { a.removeAt(8); } catch (const std::out_of_range &) { caught = true; }
    check("Reject invalid removal index", caught);

    DynamicArray<StudentRecord> copy = a;
    copy.at(0).name = "Changed Copy";
    check("Deep copy is independent", a.at(0).name == "First Equal" && copy.at(0).name == "Changed Copy");
    DynamicArray<StudentRecord> assigned;
    assigned = a;
    assigned.at(0).name = "Changed Assignment";
    check("Copy assignment is independent", a.at(0).name == "First Equal" && assigned.at(0).name == "Changed Assignment");

    DynamicArray<StudentRecord> sorted;
    sorted.push_back({1, "A", "C", 10}); sorted.push_back({2, "B", "C", 20});
    sorted.push_back({3, "C", "C", 30});
    mergeSort(sorted.raw(), sorted.size(), ByScore{});
    check("Merge sort handles already-sorted input", sorted.at(0).score == 10 && sorted.at(2).score == 30);

    DynamicArray<StudentRecord> reverse;
    reverse.push_back({4, "D", "C", 4}); reverse.push_back({3, "C", "C", 3});
    reverse.push_back({2, "B", "C", 2}); reverse.push_back({1, "A", "C", 1});
    mergeSort(reverse.raw(), reverse.size(), ByScore{});
    check("Merge sort handles reverse-ordered input", reverse.at(0).score == 1 && reverse.at(3).score == 4);

    a.saveToFile("test_output.csv");
    DynamicArray<StudentRecord> loaded;
    loaded.loadFromFile("test_output.csv");
    check("Save and reload records", loaded.size() == a.size() && loaded.at(0).id == a.at(0).id);
    std::remove("test_output.csv");

    std::cout << "\nTOTAL: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
