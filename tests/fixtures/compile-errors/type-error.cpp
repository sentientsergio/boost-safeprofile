// Test: File with type error should report compilation failure
// Expected: Tool reports "Compilation failed" NOT "No violations found!"

void type_mismatch() {
    int x = "this is a string";  // Type error

    int* p = new int(42);  // SP-OWN-001 - but won't be detected
    delete p;  // SP-OWN-002 - but won't be detected
}
