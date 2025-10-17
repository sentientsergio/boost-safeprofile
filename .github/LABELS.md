# GitHub Labels Strategy

## Label Categories

### Priority Labels (P0-P3)

- **`priority: P0 - critical`** `#d73a4a` - Blocker issues, security vulnerabilities
- **`priority: P1 - high`** `#e99695` - High priority, needed for next release
- **`priority: P2 - medium`** `#fbca04` - Medium priority, planned work
- **`priority: P3 - low`** `#c2e0c6` - Low priority, nice to have

### Rule Categories (aligned with RULES-ROADMAP.md)

- **`rule: ownership`** `#0e8a16` - Ownership/RAII rules (SP-OWN-xxx)
- **`rule: bounds`** `#1d76db` - Bounds safety rules (SP-BOUNDS-xxx)
- **`rule: lifetime`** `#5319e7` - Lifetime safety rules (SP-LIFE-xxx)
- **`rule: type`** `#f9d0c4` - Type safety rules (SP-TYPE-xxx)
- **`rule: initialization`** `#c5def5` - Initialization rules (SP-INIT-xxx)
- **`rule: concurrency`** `#d4c5f9` - Concurrency rules (SP-CONCUR-xxx)

### Work Type

- **`type: enhancement`** `#a2eeef` - New feature or request
- **`type: bug`** `#d73a4a` - Something isn't working
- **`type: documentation`** `#0075ca` - Improvements or additions to documentation
- **`type: test`** `#d4c5f9` - Testing infrastructure or test cases
- **`type: performance`** `#fbca04` - Performance optimization
- **`type: refactor`** `#c5def5` - Code refactoring

### Component/Module

- **`component: cli`** `#bfd4f2` - CLI argument parsing
- **`component: intake`** `#bfd4f2` - Repository ingestion
- **`component: analysis`** `#bfd4f2` - Analysis engine (AST detector)
- **`component: profile`** `#bfd4f2` - Profile/rule management
- **`component: emit`** `#bfd4f2` - Output generation (SARIF, HTML)
- **`component: build`** `#ededed` - CMake, dependencies, tooling

### Effort Estimates

- **`effort: small`** `#c2e0c6` - 1-3 hours
- **`effort: medium`** `#fbca04` - 4-8 hours
- **`effort: large`** `#e99695` - 8-16 hours
- **`effort: x-large`** `#d73a4a` - 16+ hours

### Status/Process

- **`status: needs-investigation`** `#ededed` - Needs research/exploration
- **`status: ready`** `#0e8a16` - Ready for implementation
- **`status: in-progress`** `#fbca04` - Currently being worked on
- **`status: blocked`** `#d73a4a` - Blocked by dependencies
- **`status: needs-review`** `#c5def5` - PR ready for review

### Special Labels

- **`good first issue`** `#7057ff` - Good for newcomers
- **`help wanted`** `#008672` - Extra attention is needed
- **`wontfix`** `#ffffff` - This will not be worked on
- **`duplicate`** `#cfd3d7` - This issue or PR already exists
- **`phase: 0`** `#ededed` - Related to Phase 0 (MVP)
- **`phase: 1`** `#0e8a16` - Related to Phase 1 (AST detection)
- **`phase: 2`** `#1d76db` - Related to Phase 2+ (future)

## Usage Examples

### New Rule Implementation

```
labels:
  - rule: ownership
  - type: enhancement
  - effort: small
  - priority: P1 - high
  - status: ready
```

Example: Implementing SP-OWN-002 (naked delete)

### Bug Report

```
labels:
  - type: bug
  - component: analysis
  - priority: P0 - critical
  - status: needs-investigation
```

### Test Addition

```
labels:
  - type: test
  - component: analysis
  - effort: small
  - good first issue
```

### Documentation Update

```
labels:
  - type: documentation
  - priority: P2 - medium
  - effort: small
```

## Label Application Guide

### For Rule Implementation Issues

1. Add appropriate `rule:` category
2. Set `type: enhancement`
3. Estimate effort based on RULES-ROADMAP.md
4. Set priority based on tier (Tier 1 = P1, Tier 2 = P2, etc.)
5. Add `phase: 1` or later as appropriate

### For Bugs

1. Always include `type: bug`
2. Add affected `component:`
3. Set priority based on severity:
   - Crashes, security: P0
   - Incorrect results: P1
   - Minor issues: P2-P3
4. Add `status: needs-investigation` initially

### For Good First Issues

1. Mark with `good first issue`
2. Should be `effort: small`
3. Well-defined scope
4. Preferably has example code or tests

## Label Maintenance

- Review labels quarterly
- Remove unused labels
- Merge similar labels if created accidentally
- Update this document when adding new categories
