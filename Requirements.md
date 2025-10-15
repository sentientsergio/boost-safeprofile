# Requirements.md — Boost.SafeProfile (Tool)

## 1. Why (Problem Statement & Rationale)

Modern C++ faces sustained pressure from industry and regulators to reduce memory-safety defects. WG21 Safety Profiles aim to define opt-in, tool-checked subsets of C++ that provide stronger safety guarantees. Organizations want a practical way to **adopt these profiles at scale**, particularly across large, mixed-quality codebases and dependency graphs.

Today’s gaps:
- Significant **annotation debt** (ownership, lifetime, nullability) blocks adoption.
- **Static analyzer findings** are hard to triage and explain; teams drown in false positives.
- There is no standard **evidence pack** that turns “clean runs” into auditable conformance claims.
- Tooling is fragmented across compilers, linters, sanitizers, and build systems.

**Boost.SafeProfile** addresses these gaps with a developer-facing tool that helps projects **conform to Safety Profiles** and **produce auditable evidence**, while remaining vendor-neutral and Boost-licensed.

---

## 2. What (Product Outcomes & Scope)

### 2.1 Primary Outcomes
1. **Profile Conformance Assessment**
   - Parse a C++ project and report conformance against a chosen Safety Profile (draft or custom).
   - Emit actionable findings with precise source locations and rule identifiers.

2. **Annotation Assistance**
   - Suggest missing ownership/lifetime/nullability annotations and safe API substitutions.
   - Provide minimal diffs or guidance that reduce violations without altering semantics.

3. **Human-Readable Explanations**
   - For each violation, generate a concise explanation of **why** it violates the profile (dataflow, escape, lifetime, or bounds rationale).

4. **Evidence Pack Generation**
   - Produce a reproducible bundle (reports, configs, logs) suitable for audits:
     - Machine-readable SARIF/JSON and human-readable summaries.
     - Toolchain fingerprints and rule versions.
     - Optional runtime evidence (sanitizer logs, fuzz campaigns) mapped to rules.

5. **Continuous Integration Readiness**
   - Deterministic CLI that can gate PRs on **profile cleanliness**.
   - Baseline + diff mode to track safety drift over time.

### 2.2 Users & Stakeholders
- **Library authors** (Boost and beyond) seeking to make their code profile-clean.
- **Product teams** needing a certifiable safety story without rewriting in another language.
- **Security/QA** teams performing independent verification.
- **Standards & tooling contributors** needing empirical feedback on rule usability.

### 2.3 Supported Code Shapes (Initial)
- C++17/C++20 projects using major compilers on Linux/macOS/Windows.
- Monorepos and multi-module builds, with or without CMake.
- Third-party headers included in analysis (configurable).

---

## 3. Functional Requirements

### 3.1 Profile Handling
- RQ-P1: Load one or more **profile definitions** (built-in drafts, MISRA-style overlays, or user-defined YAML).
- RQ-P2: Validate profile syntax and resolve rule conflicts (priority order).
- RQ-P3: Map profile rules to concrete diagnostics (rule → detector set).

### 3.2 Project Intake
- RQ-I1: Accept sources via compile_commands.json or explicit include/options sets.
- RQ-I2: Allow target scoping (analyze subset, exclude generated code).
- RQ-I3: Resolve headers and conditional compilation paths deterministically.
- RQ-I4: **Repository ingestion** — accept a repository root (local or remote). Must:
  - Detect and initialize analysis from common build descriptors (CMake, compile_commands.json, Makefile, BUILD, etc.).
  - Support shallow/full clones for remote repositories (Git/HTTPS/SSH).
  - Respect `.gitignore` / `.boostsafeignore` to filter generated/irrelevant paths.
  - Optionally resolve submodules and vendored deps if within scope.
  - Produce a deterministic project manifest (file list, hash, build info).

### 3.3 Analysis & Findings
- RQ-A1: Detect **forbidden constructs** per profile (e.g., owning raw pointers, unchecked casts, UB-prone idioms).
- RQ-A2: Identify **lifetime risks** (dangling references, escaping borrows) and **bounds risks**.
- RQ-A3: Classify finding **category**, **rule id**, **severity**, and **fix potential**.
- RQ-A4: Provide **explanations** referencing code elements and control/dataflow.
- RQ-A5: Support **waivers** with scoped justifications; waivers appear in evidence.

### 3.4 Suggestions & Guidance
- RQ-S1: Offer **non-binding** suggestions: annotations to add, API substitutions (e.g., span for pointer+size).
- RQ-S2: Generate minimal patch hunks (diffs) when safe; otherwise provide precise edits as guidance.
- RQ-S3: Respect user-configured **coding standards** (naming, header style) when formatting suggestions.

### 3.5 Reports & Evidence
- RQ-R1: Emit **SARIF** and **JSONL** for automated consumption; **Markdown/HTML** summaries for humans.
- RQ-R2: Include **toolchain fingerprints** (versions, flags), **profile versioning**, and **rule coverage**.
- RQ-R3: Optional ingestion of **sanitizer/fuzzer** artifacts; correlate to profile rules.
- RQ-R4: Provide a single **evidence pack** artifact suitable for CI storage and audits.

### 3.6 CI & Workflow Integration
- RQ-C1: Exit codes suitable for gating (0 clean, >0 with thresholds).
- RQ-C2: Baseline mode to compare current run against pinned “golden” results.
- RQ-C3: Configurable **allowlists/denylists** for directories, files, and rules.
- RQ-C4: Deterministic output for identical inputs (stable IDs, order).

---

## 4. Non-Functional Requirements

- **Accuracy**: Findings must be precise and reproducible; suggestions must never change program semantics.
- **Performance**: Handle >1M LOC repositories with parallel analysis; provide incremental re-runs.
- **Portability**: Work across major OS/compilers commonly used by Boost and industry.
- **Determinism**: Same inputs → same outputs; required for evidence and audits.
- **Extensibility**: Pluggable rules and emitters; custom profile overlays.
- **Privacy**: No code leaves the machine by default; offline-only mode must be first-class.
- **Transparency**: Each finding cites the exact rule and rationale; evidence includes tool versions and configs.
- **Internal Memory Safety (NF-S1)**: The tool SHALL conform to a documented **Safe C++ profile** for its own codebase (no raw owning pointers; RAII; bounds-checked access; no UB-prone casts; safe concurrency).
- **Static Assurance (NF-S2)**: Every release SHALL ship with clean results from: warnings-as-errors, configured static analysis/lifetime rules, and a Safety-Profile conformance report for the tool itself.
- **Dynamic Assurance (NF-S3)**: CI SHALL run AddressSanitizer + UBsan (+ TSan where applicable) and coverage-guided fuzzing on parsers/importers. Crashes/leaks are release blockers.
- **Secure Extensibility (NF-S4)**: Optional plug-ins MUST execute under strict contracts: narrow, versioned ABI; validated inputs; ability to disable plug-ins; clear sandboxing guidance.
- **Supply Chain Hygiene (NF-S5)**: Third-party components SHALL be SBOM’d and pinned; only memory-safe or vetted dependencies admitted; reproducible builds required for release artifacts.
- **Process Isolation (NF-S6)**: When invoking external tools, the tool SHALL treat them as untrusted subprocesses (time/CPU/memory limits; sanitized env; robust I/O handling).

---

## 5. Out-of-Scope (Initial)

- Replacing compilers or formal verifiers.
- Guaranteeing global program correctness beyond the selected profile’s guarantees.
- Auto-fixing complex architectural changes (e.g., refactoring ownership across subsystem boundaries).
- Enforcing org-specific secure-coding policies unrelated to memory safety (can be future overlays).

---

## 6. Constraints & Assumptions

- Safety Profiles may evolve; the tool must support **versioned profiles** and coexist with drafts.
- Large codebases have mixed quality; the tool must **prioritize signal** over completeness when tradeoffs are required.
- Some users will forbid network access; all functionality must operate fully offline.
- Third-party dependencies may be partially analyzed; the tool should support **adapter boundaries** and **assumed-safe modules**.

---

## 7. Success Metrics

- **Adoption**: Number of Boost libraries and external projects generating clean evidence packs.
- **Signal Quality**: ≥80% developer-rated usefulness for top N rule findings (survey/telemetry opt-in).
- **Time-to-Clean**: Median time to reduce a module from baseline violations to threshold target.
- **Annotation Coverage**: % of suggested annotations accepted without modification.
- **Determinism**: 100% reproducible reports across runs with identical inputs.

---

## 8. Deliverables

- **Command-line tool** producing conformance reports and evidence packs.
- **Profile packs**: built-in drafts (WG21-aligned) and a minimal example profile.
- **Config schema** for rule enablement, waivers, and project mapping.
- **Documentation**: user guide, profile authoring guide, evidence pack format, CI recipes.
- **Sample repos** demonstrating adoption on real-world code (including a Boost library).

---

## 9. Dependencies & Interoperability (Descriptive)

- Consumes standard C++ compilation databases (compile_commands.json).
- Interoperates with mainstream static analyzers/linters/sanitizers where present.
- Emits **SARIF** for broad ecosystem compatibility (code scanning, dashboards).
- Optional integration with editor tooling (surfacing findings and suggestions inline).

---

## 10. Risks & Mitigations

- **Rule churn** (profiles evolve): Versioned profiles and explicit pinning in evidence packs.
- **False positives / developer fatigue**: Tighten defaults; provide baselining and high-signal rule sets.
- **Performance on large repos**: Incremental analysis and parallelism; cache ASTs where possible.
- **Legal/Supply chain concerns**: Default offline mode; clear licensing; SBOM for bundled components.

---

## 11. Acceptance Criteria (Minimum Viable)

- Analyze a non-trivial C++ project and produce:
  - A conformance report with rule IDs, severities, and explanations.
  - At least three classes of suggestions (ownership, bounds, nullability).
  - A deterministic evidence pack including configs and tool fingerprints.
- Integrate in CI with configurable thresholds and stable exit codes.
- Operate fully offline with documented reproducibility steps.

---

## 12. Glossary (Working)

- **Safety Profile**: An opt-in, rule-based subset of C++ with defined diagnostics and guarantees.
- **Evidence Pack**: A reproducible bundle of artifacts proving conformance for a given codebase and profile.
- **Annotation**: Source-level metadata (ownership, lifetime, nullability) aiding static reasoning.
- **Waiver**: A scoped, justified exception to a rule, recorded and auditable.
