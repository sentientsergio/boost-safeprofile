// Test: File with syntax error should report compilation failure
// Expected: Tool reports "Compilation failed" NOT "No violations found!"

void incomplete_function() {
    int x = 5
    // Missing semicolon - syntax error

    int* p = new int(42);  // SP-OWN-001 - but won't be detected
}
