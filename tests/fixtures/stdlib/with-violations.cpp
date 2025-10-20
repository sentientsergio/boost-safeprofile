// Test: Real C++ code with standard library includes AND violations
// Expected WITH compile_commands.json: Analysis succeeds, finds violations
// Expected WITHOUT compile_commands.json: Compilation failed

#include <memory>
#include <vector>
#include <string>

// SP-OWN-001: Naked new
void naked_new_violation() {
    int* p = new int(42);
    // ... use p ...
    delete p;  // SP-OWN-002
}

// SP-BOUNDS-001: C-style array
void c_array_violation() {
    int buffer[256];
    buffer[0] = 1;
}

// SP-TYPE-001: C-style cast
void c_cast_violation() {
    double d = 3.14;
    int i = (int)d;  // Should use static_cast
}

// Safe code mixed in
void safe_code() {
    auto ptr = std::make_unique<int>(42);
    std::vector<int> vec(10);
    std::string str = "safe";

    int result = static_cast<int>(3.14);  // OK
}
