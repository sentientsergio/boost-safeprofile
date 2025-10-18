// Demo: Intentional Safety Violations (no includes needed)

// SP-OWN-001: Naked new expression
void demo_naked_new() {
    int* ptr = new int(42);  // VIOLATION: Should use std::make_unique
    delete ptr;  // SP-OWN-002: Naked delete (another violation)
}

// SP-OWN-002: Naked delete expression (array form)
void demo_naked_delete_array() {
    int* arr = new int[10];  // Also SP-OWN-001
    arr[0] = 100;
    delete[] arr;  // VIOLATION: Manual memory management
}

// SP-BOUNDS-001: C-style arrays
void demo_c_array() {
    int numbers[5];  // VIOLATION: Should use std::array<int, 5>
    numbers[0] = 1;

    char buffer[256];  // VIOLATION: Should use std::array or std::string
    buffer[0] = 'H';
}

// SP-TYPE-001: C-style casts
void demo_c_casts() {
    double pi = 3.14159;
    int truncated = (int)pi;  // VIOLATION: Should use static_cast<int>

    const char* str = "Hello";
    char* mutable_str = (char*)str;  // VIOLATION: Should use const_cast

    void* void_ptr = &truncated;
    int* int_ptr = (int*)void_ptr;  // VIOLATION: Should use static_cast
}

// Multiple violations in one function
void demo_multiple_violations() {
    int buffer[100];           // SP-BOUNDS-001
    int* heap = new int[50];   // SP-OWN-001

    float f = 3.14f;
    int i = (int)f;            // SP-TYPE-001

    delete[] heap;             // SP-OWN-002
}

// SP-LIFE-003: Return reference to local
int* demo_return_local_pointer() {
    int local = 99;
    return &local;  // VIOLATION: Dangling pointer
}

int& demo_return_local_reference() {
    int temp = 77;
    return temp;  // VIOLATION: Dangling reference
}
