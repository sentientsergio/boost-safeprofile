// Test: File with standard library includes (no compile_commands.json)
// Expected: Tool reports "Compilation failed" due to missing system includes
// Note: This SHOULD work with compile_commands.json

#include <memory>
#include <vector>

void has_violation() {
    int* p = new int(42);  // SP-OWN-001
    delete p;  // SP-OWN-002

    int arr[10];  // SP-BOUNDS-001
}

void safe_code() {
    auto ptr = std::make_unique<int>(42);
    std::vector<int> vec(10);
}
