# CLAUDE.md — Development Context for AI Assistants

> This file captures design decisions, development preferences, and context to assist AI coding assistants (Claude, etc.) working on this project. Human contributors may also find this useful.

---

## Project Identity

**Boost.SafeProfile** is a candidate Boost developer tool for analyzing C++ codebases against WG21 Safety Profiles. It produces actionable findings, suggests fixes, and generates auditable evidence packs.

- **License:** Boost Software License 1.0
- **Positioning:** Boost ecosystem tool (not a runtime library)
- **Philosophy:** Static-analysis-led with optional AI assist; offline-first; self-conforming

See [README.md](./README.md), [Requirements.md](./Requirements.md), and [Design.md](./Design.md) for full context.

---

## Technology Stack (Decided 2025-10-15)

### Language & Standards
- **C++20** (not C++17 or C++23)
  - Rationale: Modern productivity features (concepts, ranges, designated initializers) with widespread toolchain support
  - Minimum compilers: GCC 10+, Clang 10+, MSVC 2019+ (16.11+)
  - Target platforms: Linux, macOS, Windows

### Build System
- **CMake 3.20+**
  - Industry standard, excellent Boost integration
  - Generates `compile_commands.json` for tooling

### Core Dependencies
- **Boost 1.82+** (always prefer Boost libraries when options exist)
  - `program_options` (CLI)
  - `filesystem` (repo walking, manifests)
  - `json` (Facts IR, findings, evidence serialization)
  - `asio` (parallel analysis, timeouts)
  - `process` (invoking external tools)
  - `graph` (explain-graph construction)
  - `outcome` or `leaf` (error propagation)
  - `log` (structured logging)
  - `nowide` (Windows console handling)
  - `test` (unit testing framework)

- **LLVM/Clang 15+**
  - libtooling, clang-tidy infrastructure for AST/CFG/dataflow

- **{fmt} 10+**
  - Modern formatting (potential std::format fallback)

### Deferred Dependencies (Later Phases)
- `libgit2` (Git operations, Phase 1+)
- ONNX Runtime or llama.cpp (local AI models)

---

## Repository Structure

```
boost-safeprofile/
├── CMakeLists.txt
├── LICENSE (Boost Software License 1.0)
├── README.md, Requirements.md, Design.md, CLAUDE.md
├── CONTRIBUTING.md (to be added)
│
├── cmake/                    # Build helpers
│   ├── CompilerWarnings.cmake
│   ├── Sanitizers.cmake
│   └── Dependencies.cmake
│
├── include/boost/safeprofile/ # Minimal public API
│   └── version.hpp
│
├── src/                      # Implementation (modular)
│   ├── main.cpp
│   ├── cli/                  # Command-line interface
│   ├── intake/               # Project ingestion
│   ├── profile/              # Profile management
│   ├── analysis/             # Static analysis engine
│   ├── engine/               # Rule engine
│   └── emit/                 # Output generation (SARIF, HTML, evidence)
│
├── profiles/                 # Built-in Safety Profile definitions
│   ├── core-safety.yaml
│   └── memory-safety.yaml
│
├── tests/
│   ├── unit/                 # Boost.Test per-module
│   ├── integration/          # End-to-end CLI tests
│   └── fixtures/             # Sample C++ projects for analysis
│
├── tools/                    # Dev utilities, Docker
├── docs/                     # Extended user/dev docs
└── .github/workflows/        # CI/CD (Linux/macOS/Windows matrix)
```

**Key principles:**
- Modular `src/` layout matches architecture (intake → profile → analysis → engine → emit)
- Built-in profiles version-controlled alongside code
- Test fixtures include sample C++ codebases for integration testing

---

## Development Methodology (Agreed 2025-10-15)

### Incremental Delivery
- **Small, focused increments**: Each phase produces a working executable
- **Commit early and often** with clear, descriptive messages
- **Tag milestones**: e.g., `v0.0.1` when Phase 0 complete

### Testing Strategy
- **Framework:** Boost.Test (keeps dependencies aligned)
- **Unit tests:** Written alongside implementation, per-module coverage
- **Integration tests:** End-to-end CLI invocations on fixture projects
- **Self-conformance:** Run tool on `src/` directory as soon as first rule works
- **CI from Day 1:**
  - Warnings-as-errors (`-Wall -Wextra -Werror`)
  - AddressSanitizer + UBSan in pipeline
  - Multi-platform matrix (Linux/macOS/Windows)

### Code Quality
- **Clang-format:** Consistent formatting (`.clang-format` in root)
- **Inline documentation:** Brief header comments explaining module purpose
- **Design decisions:** Document non-obvious choices in code and/or this file
- **Self-conformance:** Treat violations in our own code as bugs

### Documentation Hygiene
- Update `docs/` as features land (not retroactively)
- Keep README quickstart accurate
- Add inline examples where helpful

---

## Phase 0 Roadmap (MVP Vertical Slice)

**Goal:** End-to-end "hello world" demonstrating the full pipeline with minimal functionality.

### Deliverables (Week 1-2):
1. **Bootstrap:** Basic CMake + empty `main()` compiles
2. **CLI skeleton:** Parses args, prints `--help` and `--version`
3. **Intake stub:** Accepts local path, lists C++ files
4. **Profile stub:** Loads one hardcoded rule (e.g., "naked new")
5. **Detector stub:** Scans for simple violation keyword
6. **SARIF emit:** Outputs valid minimal SARIF file
7. **Self-test:** Run on `src/`, verify no violations
8. **Tag `v0.0.1`**

**Why this order:**
- Establishes end-to-end flow early
- Validates build system and dependencies
- Provides foundation for iterative expansion
- Defers complexity (Git cloning, multiple rules, AI, HTML) until core works

---

## Key Design Constraints

### Self-Conformance (Critical)
- The tool's own codebase **must** conform to a Safe C++ subset
- No owning raw pointers; RAII everywhere; bounds-checked access
- Unsafe constructs are CI-blocked; waivers require justification and expiry
- Release artifacts include tool's own evidence pack (`evidence/self/`)

### Offline-First
- All functionality must work fully offline (default mode)
- AI layers are optional, pluggable, and degrade gracefully
- No network access during analysis unless `--online` explicitly enabled

### Determinism & Reproducibility
- Same inputs → same outputs (stable IDs, ordering)
- All inputs hashed; evidence packs include `REPRO.md` with exact CLI
- Required for audit trails and CI baselines

### Security & Privacy
- Repository inputs treated as **untrusted data**
- Parsers are fuzz targets with strict size/time limits
- External tools invoked as sandboxed subprocesses
- No code leaves the machine by default

### Boost-First Dependency Policy
- **Always prefer Boost libraries when options exist**
- Rationale: Alignment with Boost ecosystem, licensing consistency, quality assurance

---

## Common Pitfalls to Avoid

1. **Over-engineering profiles early:** Start with one hardcoded rule; generalize later
2. **Premature AI integration:** Defer AI layers until deterministic pipeline works
3. **Skipping self-tests:** Run tool on `src/` continuously to catch self-conformance violations
4. **Ignoring Windows:** Test on all platforms early; path handling and console I/O differ
5. **Monolithic commits:** Keep commits focused and logical; easier to review and revert

---

## Interaction Preferences

- **When making changes:** Update this file if design decisions evolve
- **Before large refactors:** Discuss approach and rationale
- **Testing:** Write tests alongside implementation (not after)
- **Documentation:** Update user-facing docs as features land

---

## References

- **WG21 Safety Profiles:** [P2687](http://wg21.link/p2687) and related papers
- **C++ Core Guidelines:** https://isocpp.github.io/CppCoreGuidelines/
- **Boost conventions:** https://www.boost.org/development/requirements.html
- **SARIF spec:** https://docs.oasis-open.org/sarif/sarif/v2.1.0/

---

## Changelog

- **2025-10-15:** Initial creation; captured technology stack, structure, methodology, and Phase 0 roadmap decisions.

---

*This file is a living document. Update as the project evolves.*
