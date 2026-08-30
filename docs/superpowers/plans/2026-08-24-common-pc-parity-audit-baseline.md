# Common and PC Parity Audit Baseline Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Produce a tested, deterministic common/PC audit inventory that accepts the checked-in Java 1.2.5 source tree and classifies possible C++ initialization defects without reporting heuristics as confirmed failures.

**Architecture:** Extend `tools/port_verify.py`, the existing parity verifier, with a Java-source abstraction and a conservative initialization-candidate reporter. Keep proven invariants as failing verification checks and emit uncertain findings into stable JSON and Markdown reports for source-by-source confirmation in the next implementation plan.

**Tech Stack:** Python 3 standard library, `unittest`, C++17 source inspection, Java 1.2.5 reference sources.

**Spec:** `docs/superpowers/specs/2026-08-24-common-pc-java-parity-audit-design.md`

## Global Constraints

- Audit only shared and PC code; exclude `src/wii`, `src/ps2`, and third-party source.
- Treat Minecraft Java 1.2.5 as the gameplay and implicit-initialization baseline.
- Preserve deliberate Beta++ and OptiFine behavior.
- Candidate heuristics generate reports and do not fail verification.
- Proven structural mismatches retain nonzero exit behavior.
- Do not configure or compile the C++ project automatically.
- Do not inspect Git state or create commits.
- Keep production identifiers and comments in English and preserve local formatting.

## File Structure

- Modify `tools/port_verify.py`: own Java source access, common/PC source selection, candidate extraction, stable report serialization, and the command-line interface.
- Create `tests/test_port_verify.py`: exercise the real verifier functions using temporary Java/C++ fixture trees and ZIP archives.
- Create `tools/port_audit_status.json`: persist manual classifications separately from generated reports.
- Create `PORT_COMMON_PC_AUDIT.json`: generated machine-readable candidate inventory.
- Create `PORT_COMMON_PC_AUDIT.md`: generated human-readable summary of the same inventory.
- Modify `PORT_STATIC_AUDIT.md`: link to the new candidate report and state its evidentiary limits.

---

### Task 1: Java source directory and ZIP abstraction

**Files:**

- Create: `tests/test_port_verify.py`
- Modify: `tools/port_verify.py`

**Interfaces:**

- Consumes: a directory containing `net/minecraft/src/*.java` or a ZIP containing the same relative tree.
- Produces: `JavaSource.read(relative_path) -> str` and `JavaSource.java_paths() -> list[str]` with identical normalized results for both storage forms.

- [ ] **Step 1: Write the failing storage-equivalence tests**

Create `tests/test_port_verify.py` with a temporary fixture containing `Packet.java` and `Example.java`. Package the same files into a ZIP and assert that both `JavaSource` instances return the same sorted paths, source text, target class set, and packet mappings. The packet fixture must contain this literal mapping so expected output is independent of the parser:

```python
PACKET_SOURCE = """package net.minecraft.src;
class Packet {
    void register() {
        addIdClassMapping(7, true, false, PacketExample.class);
    }
}
"""

self.assertEqual(
    [(7, True, False, "PacketExample")],
    port_verify.packet_mappings_java(directory_source),
)
```

The production regression named by this test is a verifier that accepts a ZIP but rejects or reads different content from the checked-in `srcorigjava` directory.

- [ ] **Step 2: Run the focused test and verify RED**

Run:

```powershell
python -m unittest tests.test_port_verify.JavaSourceTests -v
```

Expected: `ERROR` because `tools.port_verify.JavaSource` does not exist.

- [ ] **Step 3: Implement the source abstraction**

Add a small context-managed class to `tools/port_verify.py`. Normalize archive names to `/`, reject a path that is neither directory nor ZIP, and keep decoding behavior at UTF-8 with replacement. Directory lookup must accept both `net/minecraft/src/...` and a leading `src/net/minecraft/src/...`; ZIP lookup must resolve either archive layout without matching unrelated suffixes.

Use the abstraction in `target_classes` and `packet_mappings_java`. Replace the `Path` parameter with `JavaSource`, and preserve the existing tuple format and sorting behavior.

- [ ] **Step 4: Run the focused test and verify GREEN**

Run the Step 2 command.

Expected: both directory and ZIP subtests pass.

- [ ] **Step 5: Add CLI coverage for the checked-in source layout**

Add a test that calls `port_verify.main([...])` with a minimal temporary project and `--target-java <fixture-directory>`. Refactor `main` to accept `argv: list[str] | None = None`, pass `argv` to `parse_args`, and return the exit code. Assert the command reaches normal verification rather than raising `BadZipFile`.

- [ ] **Step 6: Run all verifier unit tests**

Run:

```powershell
python -m unittest tests.test_port_verify -v
```

Expected: all tests pass without configuring or compiling C++.

---

### Task 2: Explicit common/PC audit scope

**Files:**

- Modify: `tests/test_port_verify.py`
- Modify: `tools/port_verify.py`

**Interfaces:**

- Consumes: project root and source files below `src`.
- Produces: `common_pc_source_files(project: Path) -> list[Path]`, sorted by project-relative POSIX path.

- [ ] **Step 1: Write the failing scope test**

Build a fixture with one `.h` and one `.cpp` in every scoped root (`java`, `net/minecraft/src`, `client`, `lwjgl`, `platform`, `pc`) plus files in `wii`, `ps2`, `external`, `build`, and an unrelated root. Assert the exact result contains only the twelve scoped files in stable path order.

The production regression named by this test is a report whose result changes by platform checkout contents or accidentally includes console/third-party candidates.

- [ ] **Step 2: Verify RED**

Run:

```powershell
python -m unittest tests.test_port_verify.CommonPcScopeTests -v
```

Expected: `ERROR` because `common_pc_source_files` does not exist.

- [ ] **Step 3: Implement deterministic source selection**

Define the exact allowed roots as constants:

```python
COMMON_PC_ROOTS = (
    "src/java",
    "src/net/minecraft/src",
    "src/client",
    "src/lwjgl",
    "src/platform",
    "src/pc",
)
AUDIT_SOURCE_EXTENSIONS = {".h", ".hpp", ".cpp", ".cc", ".c"}
```

Resolve each allowed root beneath the project, collect only files with an allowed suffix, de-duplicate resolved paths, and sort on `relative_to(project).as_posix()`.

- [ ] **Step 4: Verify GREEN and regression coverage**

Run:

```powershell
python -m unittest tests.test_port_verify -v
```

Expected: the scope tests and Task 1 tests pass.

---

### Task 3: Conservative initialization candidate extraction

**Files:**

- Modify: `tests/test_port_verify.py`
- Modify: `tools/port_verify.py`

**Interfaces:**

- Consumes: scoped header/source text.
- Produces: `InitializationCandidate` records containing `path`, `line`, `class_name`, `member`, `declared_type`, `constructors`, `category`, `reason`, and `status="candidate"`.

- [ ] **Step 1: Write failing tests for candidate and exclusion behavior**

Use a fixture class with these cases:

```cpp
class Example
{
public:
    Example() : initializedByCtor(1) {}
    explicit Example(int value) : initializedByCtor(value), initializedEverywhere(false) {}

private:
    int missingJavaDefault;
    int initializedByCtor;
    bool initializedEverywhere = false;
    static int staticStorage;
    int function(int argument);
    int arrayWithInitializer[2] = {};
};
```

Assert that `missingJavaDefault` is emitted once with both constructors named, `initializedByCtor` is emitted because it is not initialized by every constructor only if the fixture includes a constructor that omits it, and the in-class, static, function, parameter, and initialized-array declarations are excluded. Add separate fixtures for pointer, enum, scalar array, bit-field, multiple declarators, delegating constructor, constructor assignment in the body, preprocessor branches, and a nested class.

The production regression named by these tests is either missing a plausible Java-default mismatch or reporting declarations that cannot be instance data requiring Java-style initialization.

- [ ] **Step 2: Verify RED**

Run:

```powershell
python -m unittest tests.test_port_verify.InitializationCandidateTests -v
```

Expected: `ERROR` because the candidate API does not exist.

- [ ] **Step 3: Implement lexical preprocessing without pretending to parse C++**

Add helpers that strip comments and string/character literal contents while preserving line breaks, track balanced braces, identify class/struct bodies, and collect top-level member declarations and constructor definitions. Do not expand macros or infer template semantics.

Only emit a candidate when all of these are true:

- The declaration is a non-static instance data member.
- Its type is scalar, pointer, enum-like, or a fixed-size array of those forms.
- It has no in-class initializer.
- At least one discovered non-delegating constructor lacks both a member initializer and an unambiguous top-level body assignment before any other statement that may read the member.

When constructor discovery, macro structure, or declaration parsing is ambiguous, emit category `manual_review` with the ambiguity in `reason`; never silently mark it safe.

- [ ] **Step 4: Verify GREEN**

Run the Step 2 command.

Expected: every candidate/exclusion fixture passes.

- [ ] **Step 5: Add stable ordering and duplicate suppression tests**

Feed the extractor files in reverse order and repeated paths. Assert byte-for-byte identical serialized candidate lists ordered by path, line, class, and member with no duplicate record.

- [ ] **Step 6: Run the complete Python suite**

Run:

```powershell
python -m unittest discover -s tests -p "test_*.py" -v
```

Expected: all Python tests pass.

---

### Task 4: Machine-readable and human-readable reports

**Files:**

- Modify: `tests/test_port_verify.py`
- Modify: `tools/port_verify.py`

**Interfaces:**

- Consumes: sorted `InitializationCandidate` records and scan metadata.
- Produces: schema version 1 JSON and a Markdown summary derived from the same in-memory report.

- [ ] **Step 1: Write failing serialization tests**

Construct two literal candidates in reverse order. Assert the JSON summary contains:

```json
{
  "schema_version": 1,
  "scope": "common-pc",
  "summary": {
    "files_scanned": 2,
    "initialization_candidates": 2,
    "confirmed_defects": 0
  }
}
```

The test must then assert `findings` contains the two full finding objects in stable path/line order and that Markdown groups counts by category while labeling every unclassified entry `candidate`, never `confirmed`. Normalize newlines to `\n` and terminate both files with one newline.

The production regression named by this test is a non-reproducible report or one that overstates heuristic evidence.

- [ ] **Step 2: Verify RED**

Run:

```powershell
python -m unittest tests.test_port_verify.AuditReportTests -v
```

Expected: `ERROR` because report serializers do not exist.

- [ ] **Step 3: Implement one report model and two serializers**

Add `load_audit_status`, `build_common_pc_audit_report`, `write_audit_json`, and `write_audit_markdown`. `load_audit_status` accepts a missing file as an empty classification map and keys records by the stable tuple `(path, class_name, member, category)`. JSON uses `indent=2`, `sort_keys=True`, UTF-8, and no timestamps. Markdown is derived from the same report object and contains scope, limitations, summary, grouped findings, and the exact source location/reason for each candidate.

- [ ] **Step 4: Add CLI options**

Add:

```text
--audit-common-pc
--audit-status PATH
--audit-json PATH
--audit-markdown PATH
```

Require all three paths when `--audit-common-pc` is used. Generating candidate reports must not alter the verifier exit code; only existing proven-invariant failures determine success/failure.

- [ ] **Step 5: Verify GREEN and CLI behavior**

Run all tests from Task 3 Step 6.

Expected: all pass, including a CLI fixture proving candidates are written while `main` returns zero when all proven checks pass.

---

### Task 5: Generate and review the real common/PC baseline

**Files:**

- Create: `PORT_COMMON_PC_AUDIT.json`
- Create: `PORT_COMMON_PC_AUDIT.md`
- Create: `tools/port_audit_status.json`
- Modify: `PORT_STATIC_AUDIT.md`

**Interfaces:**

- Consumes: the checked-in `srcorigjava` and current common/PC sources.
- Produces: reproducible audit artifacts that drive confirmed-defect investigation.

- [ ] **Step 1: Run the verifier against the real tree**

Run:

```powershell
python tools/port_verify.py --project . --target-java srcorigjava --audit-common-pc --audit-status tools/port_audit_status.json --audit-json PORT_COMMON_PC_AUDIT.json --audit-markdown PORT_COMMON_PC_AUDIT.md
```

Expected: the command completes without ZIP-only errors, writes both reports, and prints existing proven verification results. If an existing proven invariant fails, record it separately; do not weaken the check.

- [ ] **Step 2: Check deterministic regeneration**

Record SHA-256 hashes, run the same command again, and compare hashes:

```powershell
Get-FileHash PORT_COMMON_PC_AUDIT.json, PORT_COMMON_PC_AUDIT.md -Algorithm SHA256
```

Expected: each file has the same hash before and after the second generation.

- [ ] **Step 3: Manually classify the first risk slice**

Review every candidate in `src/java` first, followed by registry/process-wide candidates in `src/net/minecraft/src`. For each finding, compare all constructors, first reads, and the Java counterpart. Store its stable key in `tools/port_audit_status.json` with status `confirmed`, `safe`, or `manual_review`, plus a concise evidence field naming the relevant C++ and Java symbols. Do not edit generated reports or production code in this task.

- [ ] **Step 4: Validate report status vocabulary**

Add a unit test that loads a status overlay and rejects any status outside `confirmed`, `safe`, or `manual_review`, any record without non-empty evidence, and any overlay key that does not match a current finding. Run the full Python suite and regenerate once more, confirming classifications survive regeneration.

- [ ] **Step 5: Update the static-audit index**

Add a short section to `PORT_STATIC_AUDIT.md` linking `PORT_COMMON_PC_AUDIT.md`, listing candidate and confirmed counts, and explicitly stating that unclassified candidates are not proven defects. Do not copy the full finding list into the index.

- [ ] **Step 6: Final non-build verification**

Run:

```powershell
python -m unittest discover -s tests -p "test_*.py" -v
python tools/port_verify.py --project . --target-java srcorigjava --audit-common-pc --audit-status tools/port_audit_status.json --audit-json PORT_COMMON_PC_AUDIT.json --audit-markdown PORT_COMMON_PC_AUDIT.md
```

Expected: Python tests pass; the verifier reads the directory reference, emits deterministic reports, and returns according to proven checks only. No C++ configure or build command is run.

## Follow-up Boundary

This plan ends with evidence, not speculative production edits. The next plan consumes only `confirmed` findings from the first risk slice and gives each defect family its own failing regression test, minimal correction, and user-run C++ build/test checkpoint. After primitive and initialization defects are closed, later plans proceed to persistence/networking, gameplay, and fixed-seed terrain parity in the order defined by the approved specification.
