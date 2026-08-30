# PS2 ELF Size Reduction Design

## Objective

Reduce the PlayStation 2 release ELF incrementally while preserving the complete game, existing saves, texture packs, audio, and platform behavior. The initial packaged ELF is 5,897,340 bytes. The compact link map indicates that a valid compact build should first reach roughly 4.3-4.5 MB. Further work targets approximately 3.2-3.7 MB without removing gameplay features; 3.0 MB is a stretch goal, not a compatibility requirement.

The work must not hide runtime memory failures behind file compression. Every stage must report both on-disk ELF size and loaded memory size.

## Current Findings

The packaged ELF has one loadable segment with a file size of 5,759,368 bytes and an initial memory size of 6,861,948 bytes. Most of the file is executable code. The principal non-game contributors in the latest map are approximately 823 KiB from `libstdc++` and 568 KiB from `.eh_frame` plus `.gcc_except_table`.

The configured link command currently contains two complete linker scripts: the PS2SDK `startup/linkfile` retained in the CMake cache and the generated `ps2_linkfile_compact.ld`. The map reflects two section layouts and abnormal load addresses. Correcting this is a correctness prerequisite, not merely a size optimization.

Application translation units are not compiled with LTO. The existing `-flto=1` link option primarily accommodates slim LTO objects shipped by PS2SDK and gsKit.

PS2 also retains live references to `std::filesystem` from code that is unnecessary in a local-resource build. Filesystem paths pull additional path conversion, locale, and wide-character runtime code into `libstdc++`.

## Constraints

- Preserve all gameplay features during the initial four stages.
- Preserve existing world, options, statistics, resource, and texture-pack formats.
- Do not use an executable packer as a substitute for reducing loaded code.
- Do not apply `-fno-exceptions` or `-fno-rtti` globally.
- Keep the normal PS2 release optimized for size while retaining the current `-O2` overrides for measured hot rendering and world-generation translation units.
- Each stage must be independently disableable and measurable.
- The automated workflow may validate an ELF after a user-initiated build, but must not start a build itself.

## Architecture

### Stage 1: Deterministic Linker Configuration

The PS2 toolchain wrapper will remove only the stock PS2SDK `-T.../startup/linkfile` argument from both the initialization value and an existing cached `CMAKE_EXE_LINKER_FLAGS` value. `cmake/ps2.cmake` remains the single owner of the active linker script.

Configuration will fail if the stock script remains when the target-specific script is active. This prevents a stale build directory from silently restoring the duplicate script.

A post-link validation script will inspect the generated ELF before packaging. It will require a coherent loadable segment, non-overlapping allocated sections, and expected core sections. Packaging will occur only after validation succeeds, preventing an old USB ELF from masquerading as the latest result.

This stage changes no runtime behavior.

### Stage 2: Full Application LTO

A `PS2_FULL_LTO` option, enabled for compact Release builds, will add matching LTO options to compilation and linking. The existing serial, non-WHOPR settings remain necessary because the Windows toolchain path and slim SDK archives cannot safely use the external parallel LTO makefile path.

The current optimization precedence is preserved: `-Os` is the final general optimization option, and the explicit hot-source list receives a final `-O2`. VU and externally assembled objects are not treated as C/C++ LTO inputs.

LTO can change static initialization order assumptions, expose aliasing violations, or mishandle platform assembly boundaries. It therefore remains independently disableable, and the non-LTO compact build remains supported as a reference.

### Stage 3: PS2 Lightweight I/O

The first I/O pass removes live `std::filesystem` dependencies without changing public serialization contracts.

- Network/download filesystem code will not be emitted in local-resource PS2 builds.
- Runtime path checks, directory enumeration, whole-file reads, and whole-file writes will use the existing `PlatformStorage`, `PosixFileSystem`, `PlatformResources`, and `Ps2Assets` abstractions.
- PS2 code will not introduce a second path or file abstraction.
- Formatting-only streams in PS2-reachable configuration and statistics paths will be replaced with existing string parsing helpers or small locale-independent helpers.
- NBT, region, resource-stream, and texture-pack stream interfaces remain unchanged in the first pass.

Subsequent stream removal is conditional on map evidence. If file streams still dominate after filesystem removal, a later sub-stage may introduce platform-backed byte readers and writers behind the existing high-level formats. That sub-stage must use golden compatibility tests before replacing any NBT or region interface.

### Stage 4: Exception-Free Leaf Islands

Exception removal is source-scoped. CMake will maintain an explicit list of PS2 leaf translation units that:

- report failure using status values, error codes, or null pointers;
- do not contain exception syntax or RTTI operations;
- do not permit exceptions to cross their stack frames;
- do not own cleanup that depends on stack unwinding.

Only those sources receive `-fno-exceptions`, `-fno-unwind-tables`, and `-fno-asynchronous-unwind-tables`. A configuration-time audit rejects a listed source containing `throw`, `try`, `catch`, `dynamic_cast`, or `typeid`.

Token scanning is a guard, not proof of safety. Each candidate must also be reviewed for allocations and calls into code that may throw. General gameplay, containers, NBT, resources, save conversion, and texture packs remain exception-enabled until their error contracts are explicitly redesigned.

The stage begins with a deliberately small list. Expansion happens only when the map shows measurable savings and runtime tests remain stable.

## Data Flow

The build pipeline is:

1. Configure PS2 options.
2. Sanitize cached and initial linker flags.
3. Generate or select exactly one linker script.
4. Compile application sources with the selected LTO and exception policy.
5. Link once and produce the map.
6. Validate ELF headers, segments, allocated-section ranges, and required sections.
7. Strip non-loaded diagnostic sections.
8. Revalidate the stripped ELF.
9. Package that exact ELF into the USB directory.
10. Produce a size report for comparison with the previous stage.

The PS2 resource I/O path remains:

`game consumer -> GameResources/PlatformResources -> Ps2Assets -> PlatformStorage/POSIX filesystem`

No stage changes the install-root or save-storage selection policy.

## Error Handling

Build configuration errors are fatal and descriptive. A duplicate linker script, missing ELF inspection tool, malformed segment layout, overlapping allocated sections, or missing required section stops packaging.

Runtime I/O preserves current failure semantics. Optional resources remain nullable or graceful no-ops. Required resource failures continue through the existing exception-enabled boundary. Save writes must report failure without replacing a previously valid file.

Exception-free leaf code converts platform failures into its existing status-based contract. It must never synthesize a successful result after a failed allocation, device operation, upload, or module call.

## Compatibility Risks

- Linker changes can prevent boot or corrupt load addresses if the active script loses required PS2SDK metadata.
- LTO can expose undefined behavior, initialization-order dependencies, or assembly calling-convention mismatches.
- Filesystem replacement can change device-prefix handling, slash normalization, directory classification, or Memory Card behavior.
- Stream replacement can change text parsing, binary endianness, length fields, or compressed NBT output.
- Removing unwind metadata from a frame traversed by an exception can skip destructors or terminate the process.
- A failed link can leave an older packaged ELF unless packaging is strictly ordered after validation.

## Verification

### Static and Host Tests

- Test linker-flag sanitization with fresh and pre-populated cache values.
- Test the ELF validator with valid, duplicate-layout, overlapping-section, and missing-section fixtures or captured metadata.
- Test PS2 path normalization, device prefixes, directory listing, and file read/write behavior.
- Compare options and statistics text output against golden files.
- Compare representative compressed and uncompressed NBT output byte for byte.
- Round-trip representative region and world metadata through the unchanged public format.
- Verify that every exception-free source passes the syntax audit and manual dependency review.

### User-Initiated Build Measurements

After each stage, record:

- packaged ELF file size;
- loadable segment file and memory sizes;
- `.text`, `.rodata`, `.data`, `.bss`, `.eh_frame`, and `.gcc_except_table` sizes;
- `libstdc++`, application, and platform contributions from the map;
- hash equality between the generated and packaged ELF.

### PS2 Smoke Tests

- Boot to the title screen.
- Create, save, exit, and reload a world.
- Load an existing world created before the change.
- Exercise Memory Card and USB save targets.
- Load default and custom texture resources.
- Play representative sound effects and music.
- Enter the Overworld, Nether, and End.
- Exercise chunk generation, entity spawning, inventory, crafting, and settings persistence.

## Rollout and Stop Conditions

Stages land in order. A stage proceeds only after its size report and compatibility checks pass. If LTO or an exception-free island causes a hardware-only regression, that stage is disabled while earlier reductions remain usable.

The initial success target is a valid, stable ELF below 3.7 MB without feature removal. Reaching exactly 3.0 MB is not allowed to override save compatibility or runtime stability. If the four stages remain above 3.0 MB, feature removal or a complete stream/serialization redesign requires a separate design.
