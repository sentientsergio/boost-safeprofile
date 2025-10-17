# GitHub Repository Discovery

This document explains how boost-safeprofile is made discoverable on GitHub and beyond.

---

## Repository Topics (GitHub's "Hashtags")

Topics are searchable tags that help people discover repositories. They appear at the top of the repo page.

### Our Topics

**Language & Standards:**
- `cpp` - C++ programming language
- `cpp20` - C++20 standard
- `cpp-core-guidelines` - C++ Core Guidelines compliance

**Technologies:**
- `boost` - Boost C++ libraries
- `clang` - Clang compiler toolchain
- `llvm` - LLVM compiler infrastructure
- `ast` - Abstract Syntax Tree analysis

**Purpose & Domain:**
- `static-analysis` - Static code analysis tool
- `safety` - Safety-critical programming
- `memory-safety` - Memory safety verification
- `security` - Security analysis
- `code-quality` - Code quality tooling

**Standards & Formats:**
- `wg21` - WG21 C++ Standards Committee
- `sarif` - SARIF output format

### How Topics Help

1. **Search Discovery:**
   - Users can search: https://github.com/topics/cpp-static-analysis
   - Find: https://github.com/topics/memory-safety
   - Explore: https://github.com/topics/boost

2. **Related Projects:**
   - GitHub shows "Related repositories" sidebar
   - Topics cluster similar projects together

3. **Trending:**
   - Popular topics appear in GitHub Explore
   - Topics can trend within specific communities

### Managing Topics

**Add a topic:**
```bash
gh repo edit sentientsergio/boost-safeprofile --add-topic <topic-name>
```

**Remove a topic:**
```bash
gh repo edit sentientsergio/boost-safeprofile --remove-topic <topic-name>
```

**Best practices:**
- Use lowercase, hyphenated names
- Limit to 20 topics maximum
- Choose specific over generic (`cpp20` better than `programming`)
- Include technology stack (LLVM, Boost, SARIF)

---

## Repository About Section

**Current description:**
> "C++ Safety Profile conformance analysis tool"

**Should include:**
- What the tool does (✓)
- Target audience
- Key features
- Status/maturity

**Suggested update:**
> WG21 Safety Profile analyzer for C++20. AST-based static analysis with LLVM/Clang. Detects ownership, bounds, lifetime, and type safety violations. SARIF output for CI/CD integration.

---

## README.md Discoverability

Our README should have these sections for SEO/discovery:

### Current Structure ✓
- Overview with keywords (WG21, Safety Profiles, C++)
- Architecture diagram (visual appeal)
- Quickstart (easy onboarding)
- Technology stack (searchable terms)

### Could Add:
- **Badges** at top:
  ```markdown
  ![C++20](https://img.shields.io/badge/C++-20-blue)
  ![LLVM](https://img.shields.io/badge/LLVM-21-green)
  ![License](https://img.shields.io/badge/license-BSL--1.0-blue)
  ![Build](https://img.shields.io/badge/build-passing-green)
  ```

- **Use Cases** section
- **Comparison** with similar tools (clang-tidy, cppcheck)
- **Contributing** guide

---

## GitHub Features for Discovery

### 1. GitHub Stars ⭐
- Users star repos they find useful
- High star count = higher visibility
- Shown in search results ranking

### 2. GitHub Sponsors
- Optional: Enable sponsorship
- Appears in Explore tab
- Shows project sustainability

### 3. Social Preview Image
- Custom OG image for social media shares
- Repository Settings → Social preview
- Recommended: 1280×640px PNG

### 4. Repository Website
- Link to documentation site
- Appears at top of repo page
- Current: Not set

**Could set to:**
- https://sentientsergio.github.io/boost-safeprofile (GitHub Pages)
- https://boost-safeprofile.readthedocs.io (Read the Docs)

### 5. License
- ✓ Already set: Boost Software License 1.0
- Shown in repo sidebar
- Searchable by license type

---

## External Discovery

### Package Registries

**vcpkg (C++ package manager):**
- Submit package manifest
- Users: `vcpkg install boost-safeprofile`
- Discovery: vcpkg package search

**Conan:**
- Create Conan recipe
- Users: `conan install boost-safeprofile`
- Discovery: Conan Center

**Homebrew:**
- Create Homebrew formula
- Users: `brew install boost-safeprofile`
- Discovery: Homebrew search

### Community Engagement

**C++ Communities:**
- r/cpp (Reddit)
- cpplang.slack.com
- isocpp.org forums
- C++ conference presentations (CppCon, Meeting C++)

**Blog Posts:**
- "Building a WG21 Safety Profile Analyzer"
- "Zero False Positives with LLVM AST Analysis"
- "Integrating Safety Analysis into CI/CD"

**Academic Papers:**
- Conference submissions (ICSE, FSE, ASE)
- Workshop presentations
- Tool demonstration tracks

---

## SEO Keywords

These terms should appear naturally in documentation:

**Primary:**
- C++ static analysis
- C++ safety profile
- Memory safety checker
- C++ security analysis
- LLVM static analyzer

**Secondary:**
- WG21 safety profiles
- C++ Core Guidelines checker
- SARIF generator
- AST-based analysis
- Bounds checking tool

**Long-tail:**
- "How to check C++ memory safety"
- "WG21 safety profile implementation"
- "LLVM-based C++ analyzer"
- "Detect unsafe C++ patterns"

---

## Tracking Discovery

### GitHub Insights

Repository → Insights → Traffic shows:
- **Views:** How many people visit
- **Clones:** How many clone the repo
- **Referrers:** Where traffic comes from
- **Popular content:** Most viewed pages

### GitHub Pulse

Shows weekly activity:
- Commits, PRs, issues
- Contributors
- Release activity

---

## Action Items for Maximum Discoverability

**High Priority:**
1. ✅ Add repository topics (done)
2. ⬜ Add badges to README
3. ⬜ Create social preview image
4. ⬜ Set repository website URL
5. ⬜ Add "Used by" section (once adopted)

**Medium Priority:**
6. ⬜ Submit to Awesome C++ list
7. ⬜ Create GitHub Pages documentation
8. ⬜ Write introductory blog post
9. ⬜ Post to r/cpp when ready

**Future:**
10. ⬜ Package for vcpkg/Conan/Homebrew
11. ⬜ Present at local C++ meetup
12. ⬜ Submit talk to CppCon/Meeting C++

---

## Current Status

**Discoverable via:**
- ✅ GitHub search: "C++ safety profile"
- ✅ GitHub topics: memory-safety, static-analysis, cpp20
- ✅ Direct link sharing
- ✅ Google indexing (within days)

**Not yet discoverable via:**
- ⬜ Package managers (vcpkg, Conan, Homebrew)
- ⬜ Awesome C++ lists
- ⬜ Conference proceedings
- ⬜ Technical blog posts

**Last updated:** 2025-10-16
