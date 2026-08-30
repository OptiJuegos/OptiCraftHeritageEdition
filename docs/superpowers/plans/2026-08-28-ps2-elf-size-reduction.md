# PS2 ELF Size Reduction Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan.

**Goal:** Reduce the packaged PS2 ELF deterministically, targeting a stable first milestone below 3.7 MB and treating 3.0 MB as a stretch goal, without changing gameplay or asset formats.

**Architecture:** Fix the link pipeline before optimizing code: guarantee one linker script, validate both linked and stripped ELFs, then enable application LTO. Remove console-unreachable heavyweight I/O code while preserving the existing storage abstractions, and finally introduce a very small audited set of exception-free PS2 leaf translation units. Each stage remains independently switchable and produces a measurable ELF report.

**Tech Stack:** CMake 3.21+, Ninja, GCC/ELF PS2 toolchain, PowerShell source checks, existing C++ test executables.

**Spec:** `docs/superpowers/specs/2026-08-28-ps2-elf-size-reduction-design.md`

## Global Constraints

- Do not compile automatically. Run source-level checks only; the user performs PS2 configure/build checkpoints.
- Do not inspect or modify Git state and do not create commits.
- Preserve the current hot-source `-O2` overrides and use `-Os` for the remaining compact Release sources.
- Do not globally disable RTTI or exceptions and do not add an executable packer.
- Do not overwrite the staged USB ELF unless link, validation, strip, and post-strip validation all succeed.
- Record total file size, loadable bytes, `.text`, `.rodata`, `.data`, `.eh_frame`, and `.gcc_except_table` after every user build.

---

### Task 1: Make the PS2 link and package pipeline deterministic

**Files:**

- Create: `tests/Ps2ElfBuildChecks.ps1`
- Create: `cmake/ps2_linker_flags.cmake`
- Create: `cmake/ps2_validate_elf.cmake`
- Modify: `cmake/ps2_toolchain.cmake`
- Modify: `cmake/ps2.cmake`

**Step 1: Write the failing source-level checks**

Create `tests/Ps2ElfBuildChecks.ps1`. It must execute the real CMake helper scripts against temporary controlled inputs and demonstrate that:

- the stock `startup/linkfile` is removed while unrelated linker scripts and flags survive unchanged;
- a remaining stock script is rejected when a target linker script is active;
- the ELF validator accepts one coherent LOAD layout and writes the expected literal measurements;
- the ELF validator rejects two LOAD segments, missing required sections, and overlapping allocated sections.

Use a temporary fake `readelf` command that emits hand-written `-lW` and `-SW` fixtures. Assert process exit codes and report contents, not implementation text.

Run only:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/Ps2ElfBuildChecks.ps1
```

Expected: failure because the cached flag cleanup and validator do not exist yet.

**Step 2: Clean both initial and cached global linker flags**

Create `cmake/ps2_linker_flags.cmake` with small functions that remove only the exact `-T.../ps2sdk/ee/startup/linkfile` argument and reject its presence after target linker selection. In `cmake/ps2_toolchain.cmake`, apply the cleanup to:

- `CMAKE_EXE_LINKER_FLAGS_INIT`;
- `CMAKE_EXE_LINKER_FLAGS`, writing the cleaned value back to the cache with `FORCE` only when the value changed.

Do not remove arbitrary user linker scripts. Normalize whitespace after removing the stock script.

**Step 3: Add an ELF layout validator and report generator**

Create `cmake/ps2_validate_elf.cmake`, accepting:

- `PS2_READELF`
- `ELF`
- `REPORT_FILE`
- `LABEL`

The script must:

1. fail when inputs or the ELF are missing;
2. run `readelf -lW` and require exactly one `LOAD` segment;
3. run `readelf -SW` and require `.text`, `.rodata`, `.data`, and `.bss`;
4. parse allocated-section address and size fields, sort by address, and fail on non-zero overlaps;
5. extract file size, LOAD file/memory sizes, `.text`, `.rodata`, `.data`, `.bss`, `.eh_frame`, and `.gcc_except_table`;
6. print the measurements and write them to `REPORT_FILE`.

Treat missing exception-metadata sections as size zero, since later stages may legitimately eliminate them.

**Step 4: Enforce one active linker script and order packaging safely**

In `cmake/ps2.cmake`:

- locate `mips64r5900el-ps2-elf-readelf`/`ee-readelf` beside the toolchain;
- inspect the effective global executable linker flags at configure time and issue `FATAL_ERROR` if the stock startup linkfile remains;
- keep `_PS2_ACTIVE_LINKFILE` as the sole target `-T` option;
- require both `PS2_READELF` and `PS2_OBJCOPY` for the Release packaging path;
- replace the separate strip/copy commands with one ordered POST_BUILD command sequence:
  1. validate linked ELF and write `BetaPlusPlus.linked-size.txt`;
  2. strip the target ELF;
  3. validate stripped ELF and write `BetaPlusPlus.size.txt`;
  4. copy the validated target ELF to the USB staging directory.

Leave the existing IRX packaging after this sequence.

**Step 5: Run source checks**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/Ps2ElfBuildChecks.ps1
```

Expected: PASS.

**Step 6: User build checkpoint and measurement**

Ask the user to run:

```powershell
cmake --preset ps2-release
cmake --build --preset ps2-release --parallel 4
```

Then inspect `build/ps2-release/BetaPlusPlus.size.txt`, the packaged ELF, and `BetaPlusPlus.map`. Confirm there is one LOAD segment and one linker script in the generated link command before continuing.

---

### Task 2: Enable full application LTO for compact Release builds

**Files:**

- Modify: `tests/Ps2ElfBuildChecks.ps1`
- Modify: `cmake/ps2.cmake`
- Modify: `CMakePresets.json`

**Step 1: Extend the failing checks**

Require:

- a `PS2_FULL_LTO` option;
- the PS2 Release preset explicitly enables it;
- application C and C++ compilation receive `-flto=1` and `-fno-fat-lto-objects` only when Release and `PS2_FULL_LTO` are enabled;
- the link retains `-flto=1 -flto-partition=none` for compatibility with the SDK's slim LTO archives;
- disabling `PS2_FULL_LTO` removes application compile-time LTO without removing the SDK-required link-time flags.

Run the check and expect failure.

**Step 2: Add the independently switchable LTO option**

In `cmake/ps2.cmake`, define `PS2_FULL_LTO`, defaulting to the compact Release policy. Add target compile options guarded by Release plus `PS2_FULL_LTO`:

```cmake
-flto=1
-fuse-linker-plugin
-fno-fat-lto-objects
```

Keep linker `-flto=1 -flto-partition=none` unconditional because bundled SDK archives require the linker plugin. Do not change the hot-source optimization overrides.

Set `PS2_FULL_LTO` to `ON` in the `ps2-release` preset and `OFF` in `ps2-debug`.

**Step 3: Run source checks**

Run `tests/Ps2ElfBuildChecks.ps1`; expect PASS.

**Step 4: User build checkpoint and comparison**

Ask the user to configure/build PS2 Release. Verify in `build.ninja` that application compile commands carry LTO and the final link uses serial non-WHOPR LTO. Compare the new size report with Task 1, and retain the stage only if the ELF boots and reaches a world on real hardware or the user's normal PS2 test environment.

---

### Task 3: Remove PS2-unreachable filesystem and stream-heavy paths

**Files:**

- Create: `src/net/minecraft/src/LocalStatsFormat.h`
- Create: `src/net/minecraft/src/LocalStatsFormat.cpp`
- Create: `tests/LocalStatsFormatTests.cpp`
- Modify: `CMakeLists.txt`
- Modify: `src/net/minecraft/src/ThreadDownloadResources.h`
- Modify: `src/net/minecraft/src/ThreadDownloadResources.cpp`
- Modify: `src/net/minecraft/src/StatFileWriter.cpp`
- Modify: `cmake/ps2.cmake`
- Modify: `tests/Ps2ElfBuildChecks.ps1`

**Step 1: Write golden tests for the local stats text format**

Introduce tests covering:

- empty input;
- one and multiple `id value` records;
- negative values;
- arbitrary whitespace accepted by the existing stream parser;
- parsing stops at the first malformed pair, matching `operator>>` behavior;
- serialization produces the exact existing `"id value\n"` representation and preserves caller order.

Add `LocalStatsFormatTests` to the desktop test section in `CMakeLists.txt`. Do not build it automatically in this task.

**Step 2: Implement the stream-free format helper**

Implement a focused helper that parses signed decimal integers with checked overflow and serializes using integer-to-string conversion plus direct `std::string` appends. It must not include `<sstream>`, `<fstream>`, `<filesystem>`, or locale APIs.

Update `StatFileWriter.cpp` to:

- read/write bytes through the existing `PlatformStorage` API;
- use `LocalStatsFormat` instead of `istringstream`/`ostringstream` in local-stat code;
- preserve iteration order and stat lookup behavior;
- guard `StatsSyncher` destruction and all uses behind `!PLATFORM_LOCAL_STATS`.

**Step 3: Compile out network-only resource download code on PS2**

In `ThreadDownloadResources.h/.cpp`, put download/XML/network declarations, definitions, and their heavyweight includes behind `#if !PLATFORM_LOCAL_RESOURCES_ONLY`. The local path must retain only:

- recursive traversal through `PlatformStorage`;
- `Minecraft::installResource`;
- existing resource-scan logging and close handling.

Preserve desktop/network behavior byte-for-byte outside the conditional. Do not alter texture-pack, NBT, region, or asset-loading stream interfaces in this pass.

**Step 4: Exclude the unused asynchronous stats synchronizer from PS2**

Filter `StatsSyncher.cpp` from the PS2 source list only when `PLATFORM_LOCAL_STATS` is the selected PS2 policy. The guarded `StatFileWriter` destructor ensures no missing symbol remains.

Extend `Ps2ElfBuildChecks.ps1` with a configure-only fixture target that uses the PS2 source-selection helper and inspect its resolved source list. Compile a small preprocessing-only fixture for the local-resource branch during the user's build checkpoint; do not add source-text assertions.

**Step 5: Run non-build checks**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/Ps2ElfBuildChecks.ps1
```

Expected: PASS. Report that `LocalStatsFormatTests` were added but not executed because project rules reserve compilation for the user.

**Step 6: User build checkpoint and map audit**

After the user builds, confirm the link map no longer contains `fs_ops.o`, `fs_path.o`, `fs_dir.o`, or `StatsSyncher.cpp.o`. Compare `.text`, `.rodata`, `.eh_frame`, `.gcc_except_table`, LOAD bytes, and total ELF size. Test resource audio discovery plus local-stat load/save before continuing.

---

### Task 4: Add audited exception-free PS2 leaf islands

**Files:**

- Create: `cmake/ps2_no_exceptions.cmake`
- Create: `tests/Ps2ExceptionLeafChecks.ps1`
- Modify: `cmake/ps2.cmake`
- Modify: `CMakePresets.json`

**Step 1: Write failing policy checks**

Create `tests/Ps2ExceptionLeafChecks.ps1`. It must require:

- option `PS2_EXCEPTION_FREE_LEAVES`;
- an exact initial allowlist containing only:
  - `src/ps2/input/Ps2AnalogFilter.cpp`
  - `src/ps2/input/Ps2PadKeyCodes.cpp`
  - `src/ps2/system/Ps2Clock.cpp`
- flags `-fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables` applied per source, never target-wide;
- configure-time rejection if an allowlisted source contains the tokens `throw`, `try`, `catch`, `dynamic_cast`, or `typeid` as C++ tokens;
- the Release preset enables the option and Debug disables it.

Run the check and expect failure.

**Step 2: Implement the audited CMake helper**

Create `cmake/ps2_no_exceptions.cmake` with a function that:

1. receives a target and an explicit source list;
2. resolves every source and fails if it is absent;
3. reads each source and rejects forbidden C++ exception/RTTI tokens using identifier-boundary matching;
4. appends the three compile flags to those source files only.

Document that the caller must manually ensure no exception can cross into the leaf and no required cleanup depends on unwinding. Keep `Ps2MatrixStack.cpp` out because its logging call chain is not yet proven exception-free.

**Step 3: Wire the conservative initial allowlist**

Include the helper from `cmake/ps2.cmake`, add `PS2_EXCEPTION_FREE_LEAVES`, and call it only for the three audited files in compact Release. Set the preset values explicitly.

**Step 4: Run policy checks**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/Ps2ExceptionLeafChecks.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tests/Ps2ElfBuildChecks.ps1
```

Expected: PASS.

**Step 5: User build checkpoint and runtime gate**

After the user builds, compare exception metadata and total size with Task 3. Inspect the map to ensure the allowlisted objects no longer contribute unwind metadata. Test controller deadzone behavior and monotonic PS2 timing. If savings are negligible, keep the mechanism switchable but do not broaden the allowlist without a new call-chain audit.

---

### Task 5: Final verification and size decision

**Files:**

- Modify only if a verification defect is found in files already listed above.

**Step 1: Run all non-compiling checks**

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/Ps2ElfBuildChecks.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tests/Ps2ExceptionLeafChecks.ps1
```

Expected: PASS.

Also run `cmake -P cmake/ps2_validate_elf.cmake` against the latest user-built unstripped/stripped artifacts with explicit arguments where available; expect one LOAD segment, required sections, and no overlaps.

**Step 2: User performs the final Release build and runtime smoke test**

Required runtime coverage:

- boot from the staged USB layout;
- reach the title screen and load/create a world;
- render terrain and entities;
- scan and play local audio resources;
- load and save local stats;
- verify controller analog filtering;
- run long enough to exercise the extended EE clock.

**Step 3: Produce the stage table**

Report Task 1 through Task 4 measurements in a table with:

- packaged ELF bytes;
- LOAD file bytes;
- LOAD memory bytes;
- `.text`;
- `.rodata`;
- `.data`;
- `.eh_frame`;
- `.gcc_except_table`;
- largest remaining map contributors;
- runtime result.

Declare the initial milestone complete only if the packaged ELF is stable below 3.7 MB and the smoke test passes. Treat approximately 3.0 MB as a stretch result; if it is not reached, use the final map report to propose a separately approved second optimization pass instead of weakening correctness safeguards.
