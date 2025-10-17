# Safety Profile Rules Roadmap

**Status:** Phase 1 Planning Document
**Current:** SP-OWN-001 implemented with AST-based detection
**Goal:** Expand to comprehensive WG21 Safety Profile coverage

---

## Implemented Rules ✅

### SP-OWN-001: Naked new expression
- **Status:** ✅ Complete (Phase 1)
- **Severity:** Blocker
- **Detection:** AST matcher `cxxNewExpr()`
- **Filters:** Placement new excluded
- **Coverage:** Scalar and array new

---

## Phase 1.1: Ownership & RAII (High Priority)

### SP-OWN-002: Naked delete expression
- **Severity:** Blocker
- **Description:** Direct use of `delete` or `delete[]` without RAII
- **AST Matcher:** `cxxDeleteExpr()`
- **Rationale:** Matches SP-OWN-001; manual delete is error-prone
- **Examples:**
  ```cpp
  delete ptr;        // Violation
  delete[] arr;      // Violation

  // OK - RAII handles deletion
  std::unique_ptr<T> p = std::make_unique<T>();
  ```
- **Effort:** Small (1-2 hours)
- **Dependencies:** None

### SP-OWN-003: Owning raw pointer in member variable
- **Severity:** Major
- **Description:** Class member that is an owning raw pointer
- **AST Matcher:** `fieldDecl(hasType(pointerType()))` + ownership heuristics
- **Heuristics:**
  - Check if class has destructor with `delete`
  - Check for copy constructor/assignment
  - No `std::unique_ptr`/`std::shared_ptr` wrapper
- **Examples:**
  ```cpp
  class Bad {
      int* data_;  // Violation if owned
      ~Bad() { delete data_; }
  };

  class Good {
      std::unique_ptr<int> data_;  // OK
  };
  ```
- **Effort:** Medium (4-6 hours)
- **Dependencies:** Need ownership analysis

### SP-OWN-004: Raw pointer returned from function
- **Severity:** Major
- **Description:** Function returns owning raw pointer
- **AST Matcher:** `functionDecl(returns(pointerType()))`
- **Heuristics:**
  - Function name contains "create", "make", "alloc"
  - Or function body contains `new` expression
  - Not returning pointer to static/global
- **Examples:**
  ```cpp
  int* create() {      // Violation
      return new int;
  }

  std::unique_ptr<int> create() {  // OK
      return std::make_unique<int>();
  }
  ```
- **Effort:** Medium (4-6 hours)
- **Dependencies:** Control flow analysis

---

## Phase 1.2: Bounds Safety (High Priority)

### SP-BOUNDS-001: C-style array declaration
- **Severity:** Major
- **Description:** Use of C-style arrays instead of `std::array` or `std::vector`
- **AST Matcher:** `varDecl(hasType(arrayType()))`
- **Exceptions:** String literals, fixed-size compile-time arrays
- **Examples:**
  ```cpp
  int arr[10];                    // Violation
  std::array<int, 10> arr;        // OK
  std::vector<int> arr(10);       // OK
  ```
- **Effort:** Small (2-3 hours)
- **Dependencies:** None

### SP-BOUNDS-002: Array subscript without bounds check
- **Severity:** Major
- **Description:** Use of `operator[]` on containers without prior bounds check
- **AST Matcher:** `arraySubscriptExpr()` + control flow analysis
- **Requires:** Flow-sensitive analysis to detect preceding checks
- **Examples:**
  ```cpp
  int val = vec[i];              // Violation if no bounds check

  if (i < vec.size()) {
      int val = vec[i];          // OK - bounds checked
  }

  int val = vec.at(i);           // OK - throws on out-of-bounds
  ```
- **Effort:** Large (8-10 hours)
- **Dependencies:** Control flow graph (CFG) analysis

### SP-BOUNDS-003: Pointer arithmetic
- **Severity:** Major
- **Description:** Pointer arithmetic outside of standard algorithms
- **AST Matcher:** `binaryOperator(hasEitherOperand(hasType(pointerType())))`
- **Exceptions:** Iterator arithmetic in standard algorithms
- **Examples:**
  ```cpp
  int* p = arr + 5;              // Violation

  auto it = vec.begin() + 5;     // OK - iterator, not raw pointer
  ```
- **Effort:** Medium (4-6 hours)
- **Dependencies:** None

---

## Phase 1.3: Lifetime Safety (Medium Priority)

### SP-LIFE-001: Use after free
- **Severity:** Blocker
- **Description:** Use of pointer after explicit delete
- **AST Matcher:** Requires def-use chain analysis
- **Analysis:**
  - Track pointer definitions
  - Detect `delete` operations
  - Flag subsequent uses
- **Examples:**
  ```cpp
  int* p = new int(42);
  delete p;
  *p = 10;                       // Violation - use after free
  ```
- **Effort:** Large (12-16 hours)
- **Dependencies:** Dataflow analysis, reaching definitions

### SP-LIFE-002: Dangling reference
- **Severity:** Blocker
- **Description:** Reference to temporary or out-of-scope object
- **AST Matcher:** `varDecl(hasType(referenceType()))` + lifetime analysis
- **Examples:**
  ```cpp
  const std::string& s = "temp"; // Violation - binds to temporary

  std::string getString();
  const std::string& s = getString(); // Violation - temporary dies
  ```
- **Effort:** Large (10-12 hours)
- **Dependencies:** Lifetime analysis

### SP-LIFE-003: Return reference to local
- **Severity:** Blocker
- **Description:** Function returns reference to local variable
- **AST Matcher:** `returnStmt(hasReturnValue(declRefExpr(to(varDecl(hasLocalStorage())))))`
- **Examples:**
  ```cpp
  int& getLocal() {
      int x = 42;
      return x;                  // Violation
  }
  ```
- **Effort:** Small (2-3 hours)
- **Dependencies:** None

---

## Phase 1.4: Type Safety (Medium Priority)

### SP-TYPE-001: C-style cast
- **Severity:** Major
- **Description:** Use of C-style casts instead of C++ named casts
- **AST Matcher:** `cStyleCastExpr()`
- **Examples:**
  ```cpp
  void* p = (void*)ptr;          // Violation
  void* p = static_cast<void*>(ptr); // OK
  ```
- **Effort:** Small (1-2 hours)
- **Dependencies:** None

### SP-TYPE-002: reinterpret_cast usage
- **Severity:** Major
- **Description:** Use of `reinterpret_cast` (type punning risk)
- **AST Matcher:** `cxxReinterpretCastExpr()`
- **Exceptions:** Low-level system code (with waivers)
- **Examples:**
  ```cpp
  int* p = reinterpret_cast<int*>(ptr); // Violation
  ```
- **Effort:** Small (1 hour)
- **Dependencies:** None

### SP-TYPE-003: const_cast removal
- **Severity:** Major
- **Description:** Use of `const_cast` to remove const
- **AST Matcher:** `cxxConstCastExpr()`
- **Examples:**
  ```cpp
  const int* cp = ...;
  int* p = const_cast<int*>(cp); // Violation
  ```
- **Effort:** Small (1 hour)
- **Dependencies:** None

---

## Phase 1.5: Initialization Safety (Low Priority)

### SP-INIT-001: Uninitialized variable
- **Severity:** Major
- **Description:** Local variable used before initialization
- **AST Matcher:** Requires def-use analysis
- **Note:** Clang has built-in `-Wuninitialized` but we want consistency
- **Examples:**
  ```cpp
  int x;
  int y = x + 1;                 // Violation - x uninitialized
  ```
- **Effort:** Large (8-10 hours)
- **Dependencies:** Dataflow analysis

### SP-INIT-002: Narrow conversion
- **Severity:** Minor
- **Description:** Implicit narrowing conversion (data loss risk)
- **AST Matcher:** `implicitCastExpr()` + type width comparison
- **Examples:**
  ```cpp
  int64_t big = 1000000;
  int32_t small = big;           // Violation - may lose data
  ```
- **Effort:** Medium (4-5 hours)
- **Dependencies:** None

---

## Phase 2+: Advanced Rules (Future)

### SP-CONCUR-001: Data race detection
- **Severity:** Blocker
- **Description:** Shared mutable state without synchronization
- **Requires:** Thread-sensitive analysis, escape analysis
- **Effort:** Very Large (20+ hours)

### SP-CONCUR-002: Double-checked locking
- **Severity:** Major
- **Description:** Incorrect double-checked locking pattern
- **Requires:** Pattern recognition
- **Effort:** Medium (6-8 hours)

### SP-RESOURCE-001: RAII violation
- **Severity:** Major
- **Description:** Resource acquired but not released on all paths
- **Requires:** Path-sensitive analysis
- **Effort:** Large (12-15 hours)

---

## Implementation Strategy

### Priority Tiers

**Tier 1 (Next Sprint):**
1. SP-OWN-002: Naked delete (quick win, pairs with SP-OWN-001)
2. SP-BOUNDS-001: C-style arrays (high impact, easy)
3. SP-TYPE-001: C-style casts (high impact, easy)
4. SP-LIFE-003: Return reference to local (blocker severity, easy)

**Tier 2 (Future Sprints):**
- SP-OWN-003: Owning raw pointer members
- SP-BOUNDS-003: Pointer arithmetic
- SP-TYPE-002/003: reinterpret_cast, const_cast

**Tier 3 (Advanced Analysis):**
- SP-BOUNDS-002: Subscript bounds checking (needs CFG)
- SP-LIFE-001/002: Use-after-free, dangling references (needs dataflow)
- SP-INIT-001: Uninitialized variables (needs dataflow)

### Engineering Approach

Each rule follows this pattern:

1. **Research Phase** (30 min)
   - Use `clang-query` to explore AST patterns
   - Document matcher expressions
   - Identify edge cases

2. **Implementation** (1-8 hours depending on rule)
   - Create matcher in `ast_detector.cpp`
   - Add rule to `profile/loader.cpp`
   - Handle exceptions/waivers

3. **Testing** (1-2 hours)
   - Unit tests with positive/negative cases
   - Integration test on fixture projects
   - Self-test on `src/`

4. **Documentation** (30 min)
   - Update rule documentation
   - Add examples to user guide

### Estimated Total Effort

- **Tier 1 (4 rules):** ~10-12 hours
- **Tier 2 (5 rules):** ~25-30 hours
- **Tier 3 (4 rules):** ~40-50 hours
- **Phase 2+ rules:** TBD (research required)

---

## Notes

- Each rule is an independent engineering slice
- Can be parallelized across contributors
- AST matcher approach proven with SP-OWN-001
- Self-conformance requirement applies to all new code
- Rules align with WG21 P2687 Safety Profiles proposal

**Last Updated:** 2025-10-16
**Contributors:** Claude (planning), Sergio (review)
