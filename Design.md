# Design.md — Boost.SafeProfile (Tool)

## 0) Design stance
This is **static-analysis–led** with **AI assist**, not the other way around. Deterministic facts (AST/CFG, dataflow, rule matches) come from compilers and analyzers; AI layers **explain, suggest, and prioritize**—never silently change semantics. The tool must run fully **offline**; any AI inference is optional and pluggable.

We **eat our own cooking**: the codebase adheres to the same Safety-Profile rules it enforces. Unsafe constructs are banned in CI; waivers require justification and expiry.

---

## 1) Objectives recap (design-facing)
- Prove conformance to a **Safety Profile** via reproducible static checks.
- Lower **annotation debt** (ownership/lifetime/nullability).
- Turn findings into **human-understandable narratives** and **minimal diffs**.
- Emit an **evidence pack** suitable for audits/CI gates.
- Accept **repos** as the top-level input.

---

## 2) System architecture (high level)

```
                  ┌───────────────────────────────────────────────────┐
                  │                 CLI / SDK / API                   │
                  └──────────────┬───────────────────────┬────────────┘
                                 │                       │
                      (A) Project Intake          (B) Profile Manager
                                 │                       │
                                 ▼                       ▼
                       ┌────────────────┐      ┌────────────────────┐
                       │ Repo Resolver  │      │ Profile Loader/DSL │
                       │ + Build Disc.  │      │ (built-in + user)  │
                       └───────┬────────┘      └──────────┬─────────┘
                               │                          │
                               ▼                          ▼
                    ┌───────────────────┐       ┌──────────────────────┐
                    │ Compilation DB    │       │ Rule Graph (facts →  │
                    │ (compile_cmds)    │       │ detectors → actions) │
                    └────────┬──────────┘       └──────────┬───────────┘
                             │                              │
                             ▼                              ▼
                ┌──────────────────────┐          ┌──────────────────────┐
                │ Static Facts Engine  │◄────────►│ Rule Engine           │
                │ (Clang AST/CFG/DF)   │  facts   │ (match/severity/fix) │
                └───────┬──────────────┘          └──────────┬───────────┘
                        │                                     │
                        │ findings                            │ hints
                        ▼                                     ▼
         ┌──────────────────────────┐            ┌──────────────────────────┐
         │ Explainer (AI-optional) │            │ Suggestion Synth (AI-opt)│
         │ NL rationale + context  │            │ patches / annotation diffs│
         └──────────┬──────────────┘            └───────────┬──────────────┘
                    │                                         │
                    ▼                                         ▼
           ┌─────────────────────┐                   ┌──────────────────────┐
           │ Reports/Emitters    │◄── artifacts ────│ Evidence Packer       │
           │ SARIF/JSON/HTML/MD  │                   │ logs, configs, hashes │
           └────────┬────────────┘                   └──────────┬───────────┘
                    │                                           │
                    ▼                                           ▼
           CI Gate / Baselines                         Archive / Auditor
```

---

## 3) Core components

### 3.1 Project Intake (A)
**Responsibilities**
- Accept **repo URL** or local path; support Git/SSH/HTTPS, shallow/full clone.
- Build discovery: prefer `compile_commands.json`; fallback to CMake/Make/Bazel extractors.
- Path filtering via `.gitignore` and `.boostsafeignore`.
- Produce a **deterministic manifest** (file list + hashes + toolchain metadata).

**Notes**
- Determinism is first-class: same commit → same manifest → same outputs.
- All repository inputs are treated as **untrusted data**. Parsers are fuzz targets; limits on size/time.

### 3.2 Profile Manager (B)
**Responsibilities**
- Load **Safety Profile** definitions (built-in WG21-aligned drafts + user YAML overlays).
- Validate and compose rules (priority, enable/disable, severity).
- Expose a typed **Rule Graph**: `Rule → {detectors, rationale, fix-strategy, evidence-tags}`.

**DSL sketch**
- Declarative YAML/TOML describing:
  - *Forbidden constructs*: patterns (e.g., owning raw pointer), contexts, exceptions.
  - *Required idioms*: “if pointer+size, recommend span”; “no naked new/delete”.
  - *Diagnostics schema*: rule id, title, CWE mapping, help URL.
  - *Fix templates*: patch snips with placeholders.

### 3.3 Static Facts Engine
**Responsibilities**
- Use **Clang libtooling/clang-tidy** to compute:
  - AST, **control-flow graph**, **dataflow/lifetime** edges where possible.
  - Symbol table with ownership/borrow heuristics.
  - Source locations normalized to manifest.
- Emit a **Facts IR**: JSON (or protobuf) stream of typed facts (defs/uses, escapes, casts, array accesses, allocator sites).

**Non-goal**
- We don’t invent compilers. We wrap proven toolchains and cache results.

### 3.4 Rule Engine
**Responsibilities**
- Apply Profile rules to Facts IR to produce **Findings**:
  - category (ownership, lifetime, bounds, UB-prone),
  - severity (blocker/major/minor),
  - stability (confidence score),
  - **explain graph** (minimal slice of facts that justify the finding).
- Generate **Hints** for fix opportunities (annotation candidates, API substitutions).

**Determinism**
- Pure function: (Facts IR, Profile) → (Findings, Hints), stable ordering and IDs.

### 3.5 Explainer (AI-optional)
**Purpose**
- Turn explain graphs into **concise narratives**:
  - “`buf` allocated at A, pointer escapes via lambda at B, lifetime ends at C → potential UAF.”

**Constraints**
- Offline-first: rule-based templating must work without AI.
- When enabled, an LLM can refine phrasing and add code excerpts with guardrails:
  - Strict token budget; no hallucinated code; must anchor to existing locations.
  - If AI fails, fall back to templated text.

### 3.6 Suggestion Synthesizer (AI-optional)
**Purpose**
- Produce **minimal diffs** for safe transformations:
  - pointer+size → `std::span`;
  - raw owning pointer → `std::unique_ptr` factory;
  - `not_null`/`[[nodiscard]]` annotations;
  - bounds checks at hot call sites (guarded with profile tags).

**Safety guardrails**
- Never auto-apply. Output unified diffs with **semantic checks**:
  - dry compile;
  - no public ABI change unless user opts in;
  - risk tag (low/med/high).

### 3.7 Reports & Evidence
**Emitters**
- **SARIF** for code-scanning platforms.
- **JSONL** for pipelines.
- **HTML/Markdown** for humans (topline, drill-down).
- **Evidence Pack** (zip/tar): manifests, configs, tool versions, profile version, findings, sanitizer/fuzzer logs if provided, and baseline diffs.

**Reproducibility**
- Hash all inputs; include a `REPRO.md` with exact CLI used.

---

## 4) Data contracts

### 4.1 Facts IR (excerpt)
```json
{
  "unit": "src/foo.cpp",
  "hash": "sha256:…",
  "functions": [
    {
      "name": "read",
      "loc": {"line": 42, "col": 3},
      "edges": [
        {"kind": "alloc", "var": "buf", "site": {"line": 45}},
        {"kind": "borrow", "from": "buf", "to": "p", "site": {"line": 46}},
        {"kind": "escape", "via": "lambda", "site": {"line": 50}}
      ]
    }
  ]
}
```

### 4.2 Finding
```json
{
  "rule_id": "SP-OWN-001",
  "title": "Owning raw pointer",
  "severity": "blocker",
  "unit": "src/foo.cpp",
  "range": {"begin": {"l":46,"c":9}, "end": {"l":46,"c":20}},
  "explain_graph": {...},
  "hint": {"kind":"replace_with_unique_ptr", "confidence":0.88}
}
```

---

## 5) Profiles: levels & overlays
- **Core**: minimal “no footguns” (no naked new/delete, no `reinterpret_cast`, pointer+size discouraged).
- **Memory Safety**: adds lifetime/borrow rules, bounds guarantees, nullability.
- **Security**: adds hardened library overlays (checked arithmetic, safe alloc wrappers).
- **Org overlays**: MISRA/AUTOSAR/CERT toggles layered on top.

All **versioned** (`name@semver`) and pinned in evidence.

---

## 6) Execution modes

1) **Offline deterministic** (default) — all analysis local; AI disabled or using local small model.
2) **Hybrid** (opt-in) — AI explainer/suggester via local runtime or a configured endpoint; strict data minimization.
3) **CI gate** — Threshold-based exit codes; baselining to prevent noise from blocking.

---

## 7) Boost ecosystem usage (internal plumbing)
- `boost::program_options` — CLI.
- `boost::filesystem / std::filesystem` — repo walking & manifests.
- `boost::json` — Facts IR / findings / evidence serialization.
- `boost::asio` — parallel TU analysis, timeouts, cancellation.
- `boost::process` — invoking compilers/sanitizers/fuzzers.
- `boost::graph` — explain-graph construction/export.
- `boost::outcome/leaf` — error/context propagation.
- `boost::log` — structured logs; `boost::nowide` for Windows consoles.

(Clang/LLVM provides AST/CFG/DF; Boost stays out of parsing.)

---

## 8) Security, privacy & self-conformance
- The tool’s **own code** conforms to a Safe C++ subset; unsafe constructs are CI-blocked.
- Repository inputs treated as **untrusted**; parsers are fuzzed; strong time/size limits.
- No network by default; `--online` is explicit. AI layers cannot write to disk or spawn processes.
- External tool invocations are sandboxed as untrusted subprocesses.
- Releases include a **self evidence pack** (`evidence/self/…`): profile lock, analyzer logs, sanitizer summaries, fuzz coverage, SBOM.

---

## 9) Performance & determinism
- Shard by translation unit with `asio` thread pools; cap parallelism by cores/memory.
- Cache Facts IR per file hash; incremental re-analysis for changed files only.
- Stable sort: findings deterministic across runs; IDs derived from `(rule, file hash, line, col)`.

---

## 10) Extensibility
- Optional plugins (`boost::dll`) for detectors/emitters.
- Profiles as packages (versioned); overlays loaded from `profiles.d/`.
- Adapters for sanitizer/fuzzer logs added without touching core.

---

## 11) AI layers (engineering, not hype)
- **Explainer**: Inputs — rule id, explain graph, code spans. Output — ≤8 lines NL rationale referencing exact lines.
- **Suggester**: Inputs — AST context + template. Output — unified diff + justification that passes static checks.
- **Failure strategy**: timeouts/violations → deterministic templates.
- **Evaluation**: track acceptance rate & human-rated usefulness (opt-in telemetry).

---

## 12) Evidence pack layout

```
evidence/
  manifest.json
  profile.lock.json
  toolchain.json
  findings.sarif
  findings.jsonl
  human-summary.html
  sanitizer/
    asan.log
    ubsan.log
  fuzz/
    coverage.json
  configs/
    boostsafe.yaml
    waivers.yaml
  reproducibility/
    repro.md
    cli.txt
    env.txt
  self/
    findings.sarif
    sanitizers/
    fuzz/
    sbom.json
    profile.lock.json
```

---

## 13) Demo scenarios

1) **Point at a repo** — one-command analysis with HTML + evidence pack.
2) **Fix loop** — apply 2–3 minimal diffs; rerun to green.
3) **CI gate** — fail on blocker; upload SARIF.
4) **Air-gapped audit** — offline run; third-party auditor verifies evidence.
