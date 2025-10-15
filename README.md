# README.md — Boost.SafeProfile (Candidate Tool)

> **Status:** *Candidate Boost Tool* (proposed). This repository contains a developer-facing utility intended to support **WG21 Safety Profiles** adoption in C++ projects. See `Requirements.md` for problem framing, audience, and scope.

---

## Overview

**Why**  
Modern C++ teams face sustained pressure to reduce memory-safety defects. WG21 is standardizing **Safety Profiles** (opt-in, tool-checked subsets of C++) so projects can claim stronger guarantees with evidence. Teams need practical help to (1) assess conformance, (2) reduce annotation debt, and (3) produce **auditable evidence packs** for CI and reviews.  
→ Full context in **[`Requirements.md`](./Requirements.md)**.

**Who**  
- Library authors (Boost and beyond) aiming for profile-clean libraries  
- Product teams needing a certifiable safety story without rewrites  
- Security/QA engineers verifying conformance  
- Tooling & standards contributors gathering empirical feedback

---

## What this tool does (short)

- **Analyze** a C++ codebase against a selected **Safety Profile** (WG21-aligned drafts or custom overlays)
- **Report** precise findings (rule id, severity, locations) + **human-readable explanations**
- **Suggest** minimal, non-destructive fixes (annotations, safer APIs)
- **Emit** a reproducible **Evidence Pack** (machine + human reports, tool fingerprints, optional sanitizer/fuzzer artifacts)
- **Gate** CI merges with deterministic exit codes and baselines
- **Ingest** repositories directly (local or remote) for a one-command start

> The tool itself adheres to a documented Safe C++ subset and ships its **own** conformance evidence (see “Self-Conformance”).

---

## Quickstart

Analyze a local project:

```bash
boost-safeprofile analyze --profile core-safety ./path/to/project
```

Point at a remote repo (shallow clone):

```bash
boost-safeprofile analyze --profile core-safety https://github.com/cppalliance/json
```

Generate an **evidence pack** (zip) and a concise HTML summary:

```bash
boost-safeprofile analyze \
  --profile memory-safety \
  --evidence ./out/evidence \
  --report ./out/report.html \
  ./path/to/project
```

Use **offline** mode (default) to ensure no network access during analysis:

```bash
boost-safeprofile analyze --offline ./path
```

Optionally enable AI-assisted explanations/suggestions (still deterministic fallbacks if unavailable):

```bash
boost-safeprofile analyze --online --ai-explain --ai-suggest ./path
```

> **Tip:** For large codebases, provide or generate a `compile_commands.json` for best accuracy.

---

## Typical Workflow

1. **Baseline:** run on main branch, archive evidence pack  
2. **Harden:** apply low-risk suggestions (e.g., `std::span`, `not_null`, RAII)  
3. **Gate:** add CI step with thresholds (e.g., “no blocker findings”)  
4. **Prove:** attach evidence packs to releases or audits

---

## CLI (high level)

```
boost-safeprofile analyze [REPO|PATH]
  --profile <name[@version]>     # select a Safety Profile (built-in or custom)
  --config  boostsafe.yaml       # project config (excludes, waivers, profile overlay)
  --offline | --online           # network policy (offline is default)
  --ai-explain --ai-suggest      # optional AI assistance layers
  --report  out/report.{html,md} # human-readable summary
  --sarif   out/findings.sarif   # machine-readable diagnostics
  --jsonl   out/findings.jsonl   # stream for pipelines
  --evidence out/evidence        # bundle for audits (zip directory)
  --baseline out/baseline.json   # establish/compare a baseline
  --fail-on blocker|major|any    # CI exit threshold
  --jobs    N                    # parallel analysis
```

See **`Requirements.md`** for intake, reporting, and evidence details.

---

## Profiles

- **Built-in drafts:** e.g., `core-safety`, `memory-safety` (WG21-aligned)  
- **Overlays:** add MISRA/AUTOSAR/CERT or org policies via YAML  
- **Pinned:** evidence packs include a `profile.lock.json` so audits are reproducible

Example overlay snippet (YAML):

```yaml
profiles:
  use: memory-safety@1.0.0
  disable:
    - SP-CAST-REINTERPRET
  enable:
    - SP-BOUNDS-CHECK-RECOMMENDED
```

---

## Outputs

- **SARIF / JSONL:** for code scanning and pipelines  
- **HTML / Markdown:** for human review (with code excerpts & explanations)  
- **Evidence Pack:** manifests, profile lock, tool fingerprints, findings, optional sanitizer/fuzzer logs, and reproducibility notes

Directory sketch:

```
out/
  findings.sarif
  findings.jsonl
  report.html
  evidence/
    manifest.json
    profile.lock.json
    toolchain.json
    findings.{sarif,jsonl}
    sanitizer/...
    fuzz/...
    configs/...
    reproducibility/{repro.md, cli.txt, env.txt}
```

---

## CI Integration

Minimal GitHub Actions step:

```yaml
- name: Safety Profile analysis
  run: |
    boost-safeprofile analyze --profile memory-safety \
      --sarif findings.sarif --evidence evidence \
      --fail-on blocker .
- name: Upload SARIF
  uses: github/codeql-action/upload-sarif@v3
  with:
    sarif_file: findings.sarif
```

**Exit codes:** `0` (clean or below threshold), non-zero otherwise.  
**Baselines:** pin a baseline file and compare deltas per PR.

---

## Configuration

Create `boostsafe.yaml` in repo root to keep commands short:

```yaml
project:
  root: .
  excludes: [ "build/**", "third_party/**" ]
intake:
  respect_gitignore: true
profiles:
  use: core-safety@1.0.0
ci:
  fail_on: blocker
  offline: true
reports:
  sarif: out/findings.sarif
  html:  out/report.html
  evidence_dir: out/evidence
```

Run:

```bash
boost-safeprofile analyze --config boostsafe.yaml
```

---

## Self-Conformance

This tool “eats its own cooking”:

- Built under a documented **Safe C++** subset (no owning raw pointers, RAII, bounds-checked access, safe concurrency)
- Release artifacts include the tool’s **own evidence pack** (`evidence/self/…`)
- CI runs **ASan/UBSan/TSan** and **coverage-guided fuzzing** on input handlers
- Static analysis and warnings-as-errors are release blockers

---

## Project Scope & Positioning

- Distributed under the **Boost Software License**  
- Lives alongside other **Boost developer tools** (not a runtime library you link)  
- Aims to **support** WG21 Safety Profiles adoption with analysis, guidance, and auditable evidence  
- **AI-assisted**, not AI-dependent: runs fully offline with deterministic fallbacks

---

## Roadmap (preview)

- v0: Repo intake → core detectors → SARIF/HTML → evidence pack
- v1: Memory/lifetime rules, suggestions (minimal diffs), CI baselines
- v1.x: Optional AI layers (NL explanations, annotation inference), editor integration
- v2: Expanded profiles, sanitizer/fuzzer correlation, performance & scalability passes

(Exact milestones will be tracked in the issue tracker.)

---

## Contributing

Contributions welcome—especially on:
- Rule detectors & profile packs  
- Emitters (reports, dashboards)  
- Large-repo performance & incremental analysis  
- Documentation and examples

Please see `CONTRIBUTING.md` (to be added) and discuss proposals via issues before large PRs.

---

## License

Boost Software License 1.0. See `LICENSE` for details.

---

## Acknowledgments

This candidate tool builds on community work around **WG21 Safety Profiles**, the **C++ Core Guidelines**, and long-standing Boost practices that encourage safer C++.
