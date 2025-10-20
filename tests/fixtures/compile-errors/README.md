# Compilation Error Test Fixtures

**Purpose:** Verify that the tool correctly reports compilation failures instead of silently returning "No violations found!"

## Test Files

### 1. `missing-header.cpp`
- **Issue:** `#include <nonexistent_header.hpp>`
- **Expected:** Tool reports "Compilation failed" with clear error message
- **Has violations:** Yes (if it compiled)
- **Exit code:** 2 (partial failure)

### 2. `syntax-error.cpp`
- **Issue:** Missing semicolon
- **Expected:** Tool reports "Compilation failed"
- **Has violations:** Yes (if it compiled)
- **Exit code:** 2 (partial failure)

### 3. `type-error.cpp`
- **Issue:** `int x = "string";` type mismatch
- **Expected:** Tool reports "Compilation failed"
- **Has violations:** Yes (if it compiled)
- **Exit code:** 2 (partial failure)

### 4. `stdlib-includes.cpp`
- **Issue:** Includes `<memory>` and `<vector>` without compile_commands.json
- **Expected WITHOUT compile_commands.json:** Tool reports "Compilation failed"
- **Expected WITH compile_commands.json:** Analysis succeeds, finds 3 violations
- **Exit code without DB:** 2 (partial failure)
- **Exit code with DB:** 1 (violations found)

## How to Test

### Manual Testing

```bash
# Test that compilation failures are reported
./build/boost-safeprofile tests/fixtures/compile-errors/

# Expected output:
# ⚠️  WARNING: X file(s) failed to compile and were not analyzed:
#   tests/fixtures/compile-errors/missing-header.cpp: Compilation failed (...)
#   tests/fixtures/compile-errors/syntax-error.cpp: Compilation failed (...)
#   ...
#
# Exit code: 2
```

### Automated Testing

Run the integration test script:

```bash
./tests/integration/test_error_handling.sh
```

## Success Criteria

✅ **Tool reports compilation failures explicitly**
✅ **Exit code is 2 (not 0) when files fail to compile**
✅ **Output distinguishes "no violations" from "couldn't analyze"**
✅ **Each failed file is listed with error message**
✅ **No false "No violations found! ✓" when analysis didn't run**

## Red Team Validation

This fixture category addresses the critical bug discovered during red team testing:

**Before fix:**
- Files with includes returned "No violations found! ✓"
- Exit code 0 (success)
- Silent failure - no indication of problem

**After fix:**
- Files with includes report "Compilation failed"
- Exit code 2 (partial failure)
- Explicit warning with actionable guidance

## Related

- See [RED-TEAM-TESTING.md](../../../RED-TEAM-TESTING.md) for full red team protocol
- See [CLAUDE.md](../../../CLAUDE.md) changelog entry for 2025-10-20
