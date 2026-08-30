# Minecraft Port Tracker

`port_tracker.py` generates a deterministic `UPDATE.md` report for tracking a Java Minecraft version port into the current Beta++ C++ tree.

## What it tracks

- Target Java class inventory.
- Matching C++ headers and source files.
- Missing target classes.
- Header-only/source-only matches.
- New and removed classes compared with a Java baseline.
- Structural Java changes for shared classes.
- Added and removed Java methods by name/arity.
- Java class dependencies and dependents.
- Automatically ranked missing-class blockers.
- Protocol packet additions/removals and primitive IO-sequence changes.
- Explicit TODO/FIXME/not-implemented markers in matching C++ files.
- Platform-sensitive Java code that may require PC/Wii/PS2 backend work.
- Java import/API changes compared with the baseline.
- Manual status, priority, and notes stored outside the generated Markdown.
- Optional machine-readable JSON output.

The tracker does not claim that a class is complete only because a same-name C++ file exists. Existing classes must still be reviewed against the target Java version.

## Recommended layout

Keep the script and manual status file in:

```text
tools/
    port_tracker.py
    port_tracker_status.json
```

Keep the generated files at the project root:

```text
UPDATE.md
UPDATE.json
```

## Run against Minecraft 1.2.5

From the Beta++ project root:

```text
python tools/port_tracker.py --target-java "D:/path/to/minecraft-1.2.5-src.zip" --project . --target-version "Minecraft 1.2.5" --baseline-version "Minecraft Beta 1.7.3" --output UPDATE.md --json-output UPDATE.json
```

When `srcorigjava/` exists, it is automatically used as the baseline. When `tools/port_tracker_status.json` exists, it is automatically loaded.

The target source can be either a directory or a ZIP file.

## Manual status

Do not edit generated status information in `UPDATE.md`. Store human decisions in `tools/port_tracker_status.json`:

```json
{
  "classes": {
    "ExtendedBlockStorage": {
      "status": "in_progress",
      "priority": "P0",
      "notes": "Implement section storage before migrating Chunk."
    }
  }
}
```

Recommended statuses:

- `todo`
- `in_progress`
- `verify`
- `blocked`
- `done`

The status names are not hard-coded, so project-specific values can also be used.

## Machine-readable output

Pass `--json-output UPDATE.json` to generate the same inventory as JSON. This is intended for future CI checks, dashboards, or scripts without parsing Markdown.

## Useful options

```text
--top-blockers N
--top-changed N
--show-unchanged-packets
--create-status
--report-title "Minecraft 1.2.5 Port Update"
```

`--create-status` creates a starter status file when one is not already present.

## Limitations

The tool is intentionally parser-light and dependency-free. Java structural similarity, method extraction, dependencies, and platform-risk tags are heuristics. The generated report is meant to guide a manual 1:1 port audit, not replace source comparison or runtime validation.
