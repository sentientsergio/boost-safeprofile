# Red Team Testing Plan

**Status:** Mandatory for all milestone releases
**Created:** 2025-10-20
**Purpose:** Adversarial validation before claiming features are "complete"

---

## Philosophy

> **"Does it actually work?" is not answered by demos passing.**

This document defines the adversarial testing protocol that **must** be completed before tagging any milestone release or claiming a feature tier is "complete."

### Core Principle

**Testing is not about validation. Testing is about falsification.**

- Unit tests answer: "Does the code do what I think it does?"
- **Red team tests answer: "What did I forget? What breaks my assumptions?"**

---

## Red Team Sprint Structure

### When to Run Red Team Sprints

**MANDATORY before:**
- Any version tag (v0.x.0)
- Any "Tier X Complete" claim
- Any public release or announcement
- Any "production ready" statement

**RECOMMENDED after:**
- Implementing a new major feature
- Refactoring core analysis logic
- Adding new rule categories

### Sprint Duration

**Minimum:** 4-8 hours of focused adversarial testing
**Typical:** 1-2 days for a milestone release

---

## Test Fixture Categories

### 1. Minimal Fixtures (`tests/fixtures/minimal/`)

**Purpose:** Baseline validation with zero dependencies

**Characteristics:**
- No `#include` directives
- Self-contained functions
- Explicit violations of each rule
- **Expected behavior:** 100% detection rate

**Example Structure:**
```
tests/fixtures/minimal/
├── sp-own-001-violations.cpp  (naked new)
├── sp-own-002-violations.cpp  (naked delete)
├── sp-bounds-001-violations.cpp (C-arrays)
├── sp-type-001-violations.cpp (C-casts)
├── sp-life-003-violations.cpp (dangling refs)
├── clean-code.cpp (no violations)
└── expected-results.json (SARIF baseline)
```

**Status:** ✅ Partially exists (`demo/violations.cpp`)
**Action Needed:** Formalize as test suite with assertions

---

### 2. Standard Library Fixtures (`tests/fixtures/stdlib/`)

**Purpose:** Verify analysis works with real C++ headers

**Characteristics:**
- Uses `<memory>`, `<vector>`, `<string>`, etc.
- Mix of violations and safe RAII code
- Tests compile_commands.json handling
- **Expected behavior:** Analysis succeeds, violations detected

**Example Structure:**
```
tests/fixtures/stdlib/
├── with-includes.cpp (uses std headers)
├── compile_commands.json (minimal compilation database)
├── expected-violations.txt
└── README.md (explains compilation requirements)
```

**Status:** ❌ **MISSING** - Critical gap discovered 2025-10-20
**Action Needed:** Create this fixture category

---

### 3. Real-World Fixtures (`tests/fixtures/real-world/`)

**Purpose:** Prove the tool works on actual production code

**Characteristics:**
- Snippets from Boost libraries or other open-source C++
- Complex includes, templates, modern C++ features
- May require actual compile_commands.json
- **Expected behavior:** Analysis completes, results are meaningful

**Example Targets:**
- `boost-json/` - Small, modern Boost library
- `abseil-subset/` - Google's C++ library subset
- `nlohmann-json/` - Popular header-only JSON library

**Status:** ❌ **MISSING**
**Action Needed:** Select 2-3 small real projects, add as submodules or snapshots

---

### 4. Edge Cases (`tests/fixtures/edge-cases/`)

**Purpose:** Test boundary conditions and unusual C++ constructs

**Test Cases:**
- Placement new (should NOT flag SP-OWN-001)
- Template instantiations
- Macro expansions
- Typedef'd arrays
- Multidimensional arrays
- Function pointer casts
- Nested scopes
- Comments with violation-like syntax
- String literals containing patterns

**Status:** ⚠️ Partial (created during investigation)
**Action Needed:** Formalize and add to test suite

---

### 5. Error Handling (`tests/fixtures/compile-errors/`)

**Purpose:** Verify graceful failure on invalid input

**Characteristics:**
- Syntax errors
- Missing headers
- Type errors
- Incomplete code
- **Expected behavior:** Tool reports error, exits non-zero, does NOT claim "0 violations"

**Critical Test Cases:**
```cpp
// missing-header.cpp
#include <nonexistent.hpp>
int* violation = new int;  // Should report "analysis failed", not "0 violations"

// syntax-error.cpp
int broken { syntax  // Incomplete code
int* violation = new int;

// type-error.cpp
void foo() {
    int x = "string";  // Type error
    int* violation = new int;
}
```

**Status:** ❌ **MISSING** - Silent failure discovered 2025-10-20
**Action Needed:** HIGH PRIORITY - Create this category

---

## Red Team Test Checklist

Before claiming a milestone is complete, verify:

### ✅ Detection Accuracy
- [ ] All minimal fixtures analyzed correctly
- [ ] 100% violation detection on synthetic tests
- [ ] Zero false positives in clean code
- [ ] Zero false negatives in violation code

### ✅ Real-World Viability
- [ ] Can analyze code with standard library includes
- [ ] Can analyze at least one real-world project end-to-end
- [ ] compile_commands.json support working (if applicable)
- [ ] Tool completes on 10k+ LOC codebase in reasonable time

### ✅ Error Handling
- [ ] Compilation errors reported to user (not silent)
- [ ] Exit codes distinguish "clean" vs "violations" vs "error"
- [ ] Missing files handled gracefully
- [ ] Malformed input doesn't crash tool

### ✅ Edge Cases
- [ ] Placement new correctly excluded
- [ ] Template code analyzed correctly
- [ ] Macro expansions handled
- [ ] No false positives in comments/strings

### ✅ Documentation
- [ ] Known limitations documented
- [ ] False positive patterns documented
- [ ] Supported C++ standards specified
- [ ] Compilation requirements stated

---

## Current Status (2025-10-20)

### Tier 1 Rules - Red Team Assessment

**Rules Under Test:**
- SP-OWN-001: Naked new expression
- SP-OWN-002: Naked delete expression
- SP-BOUNDS-001: C-style array declaration
- SP-TYPE-001: C-style cast
- SP-LIFE-003: Return reference to local variable

**Red Team Findings:**

✅ **PASS: Detection Logic**
- Demo file: 15/15 violations detected correctly
- Self-test: 0 violations (correct)
- AST-based analysis is accurate when AST builds

❌ **FAIL: Real-World Viability**
- Cannot analyze files with `#include <memory>`
- Silent failure on compilation errors
- Reports "No violations found! ✓" when analysis didn't run
- No compile_commands.json support

❌ **FAIL: Error Handling**
- Compilation errors return empty findings
- User cannot distinguish "clean" from "failed to analyze"
- Exit code does not reflect analysis failure

⚠️ **PARTIAL: Edge Cases**
- Placement new correctly excluded ✓
- Comments/strings correctly ignored ✓
- Template code: UNTESTED
- Macro expansions: UNTESTED

❌ **FAIL: Documentation**
- Limitations not prominently documented
- README claims broader capability than reality
- No guidance on compilation requirements

**Verdict:** **Tier 1 is NOT production ready**

---

## Action Items for Tier 1 Validation

### Priority 1: Blockers (MUST FIX)

1. **Fix silent failure on compilation errors** (2-3 hours)
   - Distinguish "analysis failed" from "no violations"
   - Report compilation errors to user
   - Exit with error code on analysis failure
   - Update tests to verify error handling

2. **Add compile_commands.json support** (4-6 hours)
   - Read compilation flags from database
   - Fall back to sensible defaults if not present
   - Test with stdlib includes

3. **Create error handling fixtures** (2 hours)
   - `tests/fixtures/compile-errors/`
   - Verify tool reports errors, not false "clean" results

### Priority 2: Real-World Validation (SHOULD FIX)

4. **Create stdlib fixtures** (2-3 hours)
   - `tests/fixtures/stdlib/`
   - Test with real C++ headers

5. **Test on real project** (3-4 hours)
   - Pick small Boost library (e.g., Boost.JSON)
   - Document any failures honestly
   - Update limitations in README

### Priority 3: Edge Case Coverage (NICE TO HAVE)

6. **Formalize edge case tests** (2-3 hours)
7. **Template code testing** (2-3 hours)
8. **Macro expansion testing** (2-3 hours)

**Total Estimated Effort:** 15-25 hours

---

## Red Team Success Criteria

Tier 1 can be claimed "production ready" when:

1. ✅ All 34 unit tests pass
2. ✅ All minimal fixtures pass (100% detection)
3. ✅ All stdlib fixtures pass (analysis succeeds)
4. ✅ At least one real-world project analyzed successfully
5. ✅ Error handling fixtures demonstrate graceful failure
6. ✅ Known limitations documented prominently
7. ✅ README accurately reflects current capabilities

**Until then:** Tier 1 is "implemented, testing in progress"

---

## Future Red Team Considerations

### Phase 2+ Testing

- **Performance testing:** 100k+ LOC codebases
- **Concurrency testing:** Multi-threaded analysis
- **Platform testing:** Linux, macOS, Windows
- **Sanitizer testing:** ASan, UBSan, TSan clean
- **Fuzzing:** AFL/libFuzzer on SARIF parsing and input handling

### Continuous Red Teaming

- Run red team suite in CI on every PR
- Quarterly "break it" sessions with fresh eyes
- Community bug bounty for finding false negatives

---

## Lessons Learned (2025-10-20)

1. **Synthetic demos are necessary but not sufficient**
   - They validate happy path logic
   - They don't reveal real-world gaps

2. **"It works on my code" ≠ "It works"**
   - Self-conformance is valuable but narrow
   - Real projects have includes, dependencies, complexity

3. **Silent failures are worse than loud failures**
   - Reporting "clean" when analysis didn't run is dangerous
   - Users trust the tool - betraying that trust is unacceptable

4. **Testing must be adversarial**
   - Ask "what breaks this?" not "does this work?"
   - Assume every feature is broken until proven otherwise

---

## Conclusion

Red team testing is **not optional**. It is the difference between:

- "I wrote code" and "I shipped a feature"
- "The demo passes" and "This works in production"
- "It compiled" and "It's correct"

Every milestone must pass red team validation before being tagged or announced.

**Next Step:** Fix the blockers, then re-run red team testing on Tier 1.
