// Test: Real C++ code with standard library includes, NO violations
// Expected WITH compile_commands.json: Analysis succeeds, 0 violations
// Expected WITHOUT compile_commands.json: Compilation failed

#include <memory>
#include <vector>
#include <array>
#include <string>

// All safe RAII code
void safe_memory_management() {
    auto ptr = std::make_unique<int>(42);
    auto shared = std::shared_ptr<int>(new int(99));  // OK - wrapped in smart pointer

    std::vector<int> vec(100);
    std::array<int, 10> arr;
    std::string str = "safe string";
}

// Safe casts
void safe_casts() {
    double d = 3.14;
    int i = static_cast<int>(d);

    const char* cstr = "test";
    char* mutable_str = const_cast<char*>(cstr);  // OK - named cast
}

// No dangling references
int& safe_return(int& param) {
    return param;  // OK - returning parameter
}
