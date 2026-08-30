# Common and PC Java Parity Audit Design

## Goal

Audit and correct the Minecraft 1.2.5 Java-to-C++ translation in shared and PC code, beginning with initialization and Java semantic compatibility and then verifying terrain generation and other high-risk gameplay systems. Preserve deliberate Beta++ and OptiFine extensions while removing accidental Beta 1.7.3 behavior from the Minecraft 1.2.5 path.

## Scope

The first audit covers:

- `src/java`, `src/net/minecraft/src`, `src/client`, `src/lwjgl`, `src/platform`, and `src/pc`.
- Shared configuration and build declarations that control those directories.
- Java 1.2.5 reference code under `srcorigjava/net/minecraft/src`.
- Static verification tools and focused regression tests under `tools` and `tests`.

Wii- and PS2-specific implementations are excluded. Shared code remains in scope even when consoles also consume it. A platform-specific difference is recorded, but not modified, unless it proves a defect in the shared contract.

The audit does not promise proof of every runtime behavior through static analysis. Compiler diagnostics and gameplay comparisons that require a build remain user-run because repository rules prohibit automatic compilation.

## Behavioral Baseline

Minecraft Java 1.2.5 is the authoritative baseline for gameplay rules, serialized data, protocol behavior, random-number consumption, world generation, and implicit field defaults.

Beta++ and OptiFine behavior is retained when at least one of these conditions holds:

- It is an intentional user-facing extension or optimization.
- It is guarded by a clearly named option or platform capability.
- It preserves externally observable Java 1.2.5 behavior.
- Existing project documentation identifies it as deliberate and current code supports that description.

A difference is treated as a translation defect when it is unexplained, inherits Beta 1.7.3 rules in the active 1.2.5 path, changes Java 1.2.5 results unintentionally, relies on undefined C++ behavior, or exists only because Java and C++ initialization, numeric, lifetime, or container semantics differ.

Ambiguous differences are documented before modification. The audit does not silently remove an optimization merely because its implementation differs structurally from Java.

## Audit Architecture

### 1. Deterministic inventory and evidence

Extend the existing verification tooling instead of creating overlapping scanners. The inventory records findings by category, file, symbol, Java counterpart, evidence, severity, status, and verification method. Generated reports must distinguish a heuristic candidate from a confirmed defect.

The initial scan covers:

- Scalar, pointer, enum, aggregate, and array members without explicit initialization.
- Constructors that omit members which Java initializes implicitly.
- Static initialization order and initialization/cleanup symmetry.
- Suspicious placeholder implementations and incomplete methods.
- Narrowing conversions, signed overflow assumptions, shifts, division, modulo, casts, and integer literal width.
- Java reference/GC assumptions translated into raw pointer ownership or borrowed lifetime.
- Array bounds, negative indices, container invalidation, and null handling.
- Java collection behavior that differs from the selected C++ container.
- String, Unicode, equality, hashing, floating-point, and random-number compatibility.

Static findings do not trigger bulk edits. Each confirmed family receives a focused regression contract and small production changes.

### 2. Java semantic foundation

Audit `src/java` and the primitive compatibility contracts before gameplay classes. This includes Java integer arithmetic, floating-point edge cases, `Random`, strings, hashing, arrays, streams, and type aliases. A defect here can affect hundreds of translated classes and invalidate higher-level comparisons.

For each semantic contract, compare the implementation against Java behavior, add a focused test or static assertion, observe the test fail for the identified defect, and make the smallest correction that passes it.

### 3. Initialization and lifetime audit

Review shared and PC classes in dependency order. Start with registries and process-wide state, then data/value objects, world/chunk storage, entities, networking, client state, rendering orchestration, audio/input, and PC adapters.

Java initializes instance fields to zero, false, or null before constructor execution. C++ does not do this for ordinary scalar members. Every omitted C++ member must therefore be classified as:

- Initialized through an in-class initializer.
- Initialized by every constructor.
- Intentionally indeterminate and never read before assignment, supported by control-flow evidence.
- A confirmed defect requiring explicit Java-compatible initialization.

Pointer changes must document ownership. Raw pointers are not mechanically replaced with smart pointers: engine registries, borrowed world references, packet ownership, and OpenGL/SDL handles have different lifetime contracts. Corrections occur at the ownership source rather than by masking a dangling read.

### 4. Structural and behavioral parity

After the foundation is stable, compare Java and C++ class behavior in risk-ranked groups rather than alphabetical order:

1. Persistence, NBT, Anvil, region files, and chunk data.
2. Networking packet payloads and connection state.
3. World, chunks, blocks, items, registries, and scheduled updates.
4. Entities, AI, pathfinding, combat, and inventories.
5. Client state, GUI, input, audio, and PC integration.
6. Rendering-side gameplay calculations where output affects behavior.

Class-name and method-name presence are inventory evidence only. A class is verified only after its defaults, branches, side effects, ownership, exceptions, numeric behavior, and relevant call sites have been compared.

### 5. Terrain and world-generation parity

World generation begins only after `Random`, integer arithmetic, arrays, and initialization contracts are verified. The comparison follows the data flow:

1. `WorldType`, `WorldInfo`, seed propagation, and provider selection.
2. Java-compatible random seeding and call order.
3. Noise generators and octave composition.
4. `GenLayer`, biome cache, and `WorldChunkManager`.
5. Base terrain and block replacement in `ChunkProviderGenerate`.
6. Caves, ravines, mineshafts, villages, and strongholds.
7. Biome decoration, ores, vegetation, liquids, spawning, snow, and ice.
8. Chunk population flags, negative coordinates, save/load, and regeneration.
9. Nether, End, and superflat behavior after Overworld parity is established.

The recent Beta-worldgen cleanup is treated as an input to this audit, not as proof of parity. Remaining Beta-named code is classified as serialization/protocol compatibility, a deliberate optional feature, a harmless historical name, or an active behavioral residue. Only the last category is corrected.

Seed parity requires comparing compact deterministic outputs rather than screenshots. The planned test surface uses biome grids, chunk block/metadata hashes, height maps, structure decisions, and decoration results for fixed seeds and coordinates. Java fixtures may be generated externally and checked into tests; the C++ side then consumes the same expected values.

## Testing Strategy

Every production correction follows a red-green cycle:

1. Capture the smallest failing behavior using an existing test target, a focused new test target, or a static verification rule when runtime isolation is impractical.
2. Run non-compiling static tests automatically when permitted.
3. For C++ tests, provide the exact configure/build/test command to the user and use the returned output as the required failing evidence.
4. Apply one root-cause correction.
5. Re-run the same check, then the wider relevant suite.

Static checks must fail only on actionable conditions and include file, line, symbol, and reason. Heuristics that can produce false positives generate an audit report rather than failing verification.

The project currently has very limited runtime test coverage. Test infrastructure additions remain focused: compatibility primitives, constructors/value objects, serialization, packet payloads, and deterministic world-generation units. The audit does not introduce a new general-purpose test framework unless the existing executable-style tests prove inadequate for a concrete case.

## User-Run Compiler and Runtime Verification

The assistant does not configure or compile the project automatically. At defined checkpoints, the user receives exact commands for:

- PC Debug compilation with warnings that expose uninitialized reads and conversions.
- Focused CTest targets.
- Optional compiler sanitizers or static analyzers when supported by the selected PC toolchain.
- Fixed-seed world generation and save/load smoke tests.

Build logs are treated as evidence and audited completely. A warning is not suppressed unless the code is proven correct and suppression is narrower and clearer than restructuring it.

## Change Boundaries

- Production identifiers and comments remain in English.
- Existing formatting and local patterns are preserved.
- No adjacent cleanup, dead-code deletion, or unrelated refactoring is included.
- No Git inspection or commits are performed.
- No automatic project compilation is performed.
- Generated reports are regenerated only when their source data or verifier changes.
- Wii and PS2 code is not changed in this stage.
- A shared fix that changes a documented console assumption is stopped and presented for approval before implementation.

## Risk Management

The highest-risk areas and their protections are:

- **Save compatibility:** use byte-/tag-level fixtures and preserve existing worlds before manual tests.
- **Seed determinism:** lock Java-compatible arithmetic and RNG behavior before comparing chunks.
- **Global initialization:** verify initialization order, repeatable initialization, and cleanup symmetry.
- **Ownership:** trace allocation, transfer, borrowing, and destruction before changing pointer types or deletion paths.
- **Protocol compatibility:** compare mapping and payload order; do not infer payload parity from class presence.
- **Debug/Release divergence:** treat indeterminate values, signed overflow, aliasing, and iterator invalidation as priority defects.
- **Optimization regressions:** compare observable outputs and keep deliberate Beta++/OptiFine fast paths when equivalent.

If three attempted fixes in one area expose different shared-state failures, work stops for an architectural review instead of layering another patch.

## Deliverables and Checkpoints

The work is split into independently reviewable checkpoints:

1. Audit tooling and categorized baseline report.
2. Java primitive and compatibility-layer corrections.
3. Shared initialization and lifetime corrections.
4. PC-specific initialization and adapter corrections.
5. Persistence and networking parity.
6. Core gameplay parity.
7. Terrain and world-generation parity.
8. Client/PC behavior and final static re-audit.

Each checkpoint reports confirmed defects, corrected files, tests added, static evidence, commands awaiting user execution, and unresolved candidates. Completion means no confirmed issue remains in that checkpoint and all permitted verification is green; it does not claim exhaustive runtime certification beyond the evidence collected.

## Success Criteria

- All common and PC member initialization candidates are classified, and every confirmed Java-default mismatch is corrected.
- Confirmed undefined behavior and Java/C++ primitive semantic mismatches have regression coverage.
- Ownership and borrowed-lifetime corrections are evidence-based and documented by tests or focused invariants.
- No active Beta 1.7.3 world-generation rule remains in the Minecraft 1.2.5 path unless explicitly retained as an option.
- Fixed-seed terrain fixtures match Java 1.2.5 for the selected seed/coordinate matrix after user-run tests.
- Persistence and representative packet fixtures match the Java 1.2.5 layout.
- Existing deliberate Beta++/OptiFine behavior remains available and documented.
- All automatic static checks pass, and all user-run build/test checkpoints supplied during the audit pass.

## Non-goals

- Wii- or PS2-specific implementation fixes.
- Replacing deliberate Beta++/OptiFine features with vanilla code when behavior is compatible.
- Reintroducing Beta 1.7.3 seed parity into the default Minecraft 1.2.5 generator.
- Broad style rewrites, dead-code removal, or modernization unrelated to a confirmed defect.
- Claiming that static analysis alone proves complete gameplay parity.
