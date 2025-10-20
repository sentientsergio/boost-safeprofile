// Test: File with missing header should report compilation failure
// Expected: Tool reports "Compilation failed" NOT "No violations found!"

#include <nonexistent_header.hpp>

void has_violation() {
    int* p = new int(42);  // SP-OWN-001 - but won't be detected due to compilation failure
    delete p;  // SP-OWN-002 - but won't be detected due to compilation failure
}
