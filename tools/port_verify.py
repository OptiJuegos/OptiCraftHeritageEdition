#!/usr/bin/env python3
"""Static consistency checks for the Minecraft 1.2.5 port.

This deliberately does not compile the project. It catches integration mistakes that
are cheap to detect from source: target-class coverage, local includes, packet
registrations and explicit incomplete-code markers.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
import zipfile
from dataclasses import dataclass
from pathlib import Path

SOURCE_EXTENSIONS = {".h", ".hpp", ".cpp", ".cc"}
COMMON_PC_ROOTS = (
    "src/java",
    "src/net/minecraft/src",
    "src/client",
    "src/lwjgl",
    "src/platform",
    "src/pc",
)
AUDIT_SOURCE_EXTENSIONS = {".h", ".hpp", ".cpp", ".cc", ".c"}
STUB_RE = re.compile(r"\b(?:TODO|FIXME)\b|not implemented|assert\s*\(\s*false\s*\)", re.I)
PACKET_JAVA_RE = re.compile(
    r"addIdClassMapping\(\s*(\d+)\s*,\s*(true|false)\s*,\s*(true|false)\s*,\s*(Packet\w+)\.class\s*\)"
)
PACKET_CPP_RE = re.compile(
    r"addIdClassMapping\(\s*(\d+)\s*,\s*(true|false)\s*,\s*(true|false)\s*,\s*createPacket<(Packet\w+)>\s*\)"
)


class JavaSource:
    """Read normalized Minecraft Java sources from a directory or ZIP file."""

    _PACKAGE_PREFIX = "net/minecraft/src/"
    _ARCHIVE_PREFIXES = (_PACKAGE_PREFIX, "src/" + _PACKAGE_PREFIX)

    def __init__(self, path: Path):
        self.path = Path(path)
        self._archive: zipfile.ZipFile | None = None
        self._entries: dict[str, str] = {}

        if self.path.is_dir():
            for prefix in (Path(), Path("src")):
                package = self.path / prefix / "net" / "minecraft" / "src"
                if not package.is_dir():
                    continue
                for source_path in package.rglob("*.java"):
                    relative = source_path.relative_to(self.path / prefix).as_posix()
                    self._entries.setdefault(relative, str(source_path))
        elif self.path.is_file() and zipfile.is_zipfile(self.path):
            self._archive = zipfile.ZipFile(self.path)
            for raw_name in self._archive.namelist():
                name = raw_name.replace("\\", "/").lstrip("/")
                for prefix in self._ARCHIVE_PREFIXES:
                    if name.startswith(prefix) and name.endswith(".java"):
                        normalized = name[len("src/"):] if name.startswith("src/") else name
                        self._entries.setdefault(normalized, raw_name)
                        break
        else:
            raise ValueError(f"Java source must be a directory or ZIP file: {self.path}")

    def __enter__(self) -> "JavaSource":
        return self

    def __exit__(self, exc_type, exc_value, traceback) -> None:
        self.close()

    def close(self) -> None:
        if self._archive is not None:
            self._archive.close()
            self._archive = None

    def java_paths(self) -> list[str]:
        return sorted(self._entries)

    def read(self, relative_path: str) -> str:
        normalized = relative_path.replace("\\", "/").lstrip("/")
        entry = self._entries.get(normalized)
        if entry is None:
            raise FileNotFoundError(normalized)
        if self._archive is not None:
            return self._archive.read(entry).decode("utf-8", "replace")
        return Path(entry).read_text(encoding="utf-8", errors="replace")


def common_pc_source_files(project: Path) -> list[Path]:
    project = Path(project).resolve()
    found: dict[Path, Path] = {}
    for relative_root in COMMON_PC_ROOTS:
        root = project / relative_root
        if not root.is_dir():
            continue
        for path in root.rglob("*"):
            if path.is_file() and path.suffix in AUDIT_SOURCE_EXTENSIONS:
                resolved = path.resolve()
                found.setdefault(resolved, path)
    return sorted(found.values(), key=lambda path: path.relative_to(project).as_posix())


@dataclass(frozen=True)
class InitializationCandidate:
    path: str
    line: int
    class_name: str
    member: str
    declared_type: str
    constructors: tuple[str, ...]
    category: str
    reason: str
    status: str = "candidate"


def _mask_comments_and_literals(text: str) -> str:
    result = list(text)
    index = 0
    state = "code"
    while index < len(text):
        char = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and following == "/":
                result[index] = result[index + 1] = " "
                index += 2
                state = "line_comment"
                continue
            if char == "/" and following == "*":
                result[index] = result[index + 1] = " "
                index += 2
                state = "block_comment"
                continue
            if char in {'"', "'"}:
                result[index] = " "
                state = "string" if char == '"' else "character"
        elif state == "line_comment":
            if char == "\n":
                state = "code"
            else:
                result[index] = " "
        elif state == "block_comment":
            if char == "*" and following == "/":
                result[index] = result[index + 1] = " "
                index += 2
                state = "code"
                continue
            if char != "\n":
                result[index] = " "
        else:
            if char == "\\" and following:
                result[index] = " "
                if following != "\n":
                    result[index + 1] = " "
                index += 2
                continue
            quote = '"' if state == "string" else "'"
            if char == quote:
                state = "code"
            if char != "\n":
                result[index] = " "
        index += 1
    return "".join(result)


def _matching_brace(text: str, opening: int) -> int | None:
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return index
    return None


def _top_level_statements(body: str) -> list[tuple[int, str]]:
    statements: list[tuple[int, str]] = []
    depth = 0
    start = 0
    for index, char in enumerate(body):
        if char == "{":
            depth += 1
        elif char == "}":
            depth = max(0, depth - 1)
            if depth == 0:
                start = index + 1
        elif char == ";" and depth == 0:
            statements.append((start, body[start:index + 1]))
            start = index + 1
    return statements


def _split_declarators(text: str) -> list[str]:
    parts: list[str] = []
    start = 0
    depths = {"<": 0, "(": 0, "[": 0, "{": 0}
    closing = {">": "<", ")": "(", "]": "[", "}": "{"}
    for index, char in enumerate(text):
        if char in depths:
            depths[char] += 1
        elif char in closing:
            key = closing[char]
            depths[key] = max(0, depths[key] - 1)
        elif char == "," and not any(depths.values()):
            parts.append(text[start:index].strip())
            start = index + 1
    parts.append(text[start:].strip())
    return [part for part in parts if part]


def _is_audited_member_type(declared_type: str, enum_names: set[str]) -> bool:
    if "*" in declared_type:
        return True
    normalized = re.sub(r"\b(?:const|volatile|signed|unsigned)\b", "", declared_type)
    normalized = " ".join(normalized.split())
    if not normalized and re.search(r"\b(?:signed|unsigned)\b", declared_type):
        normalized = "int"
    scalar_types = {
        "bool", "bool_t", "char", "char_t", "uchar_t", "byte_t", "ubyte_t",
        "short", "short_t", "ushort_t", "int", "int_t", "uint_t", "long",
        "long_t", "ulong_t", "long long", "float", "float_t", "double",
        "double_t", "size_t", "std::size_t", "std::int8_t", "std::uint8_t",
        "std::int16_t", "std::uint16_t", "std::int32_t", "std::uint32_t",
        "std::int64_t", "std::uint64_t",
    }
    return normalized in scalar_types or normalized in enum_names


def _member_declarations(
    statement: str,
    enum_names: set[str],
) -> list[tuple[str, str, bool]]:
    cleaned = re.sub(r"\b(?:public|private|protected)\s*:\s*", "", statement).strip()
    cleaned = cleaned[:-1].strip() if cleaned.endswith(";") else cleaned
    preprocessor = "#" in cleaned
    cleaned = "\n".join(line for line in cleaned.splitlines() if not line.lstrip().startswith("#")).strip()
    if not cleaned or "(" in cleaned or re.match(
        r"^(?:static|using|typedef|friend|class|struct|union|enum|template|return)\b", cleaned
    ):
        return []

    parts = _split_declarators(cleaned)
    first = re.match(
        r"^(?P<type>.+?)(?P<name>[A-Za-z_]\w*)\s*(?P<array>\[[^\]]+\])?\s*"
        r"(?P<bits>:\s*\d+)?\s*(?P<init>=.*|\{.*\})?$",
        parts[0],
        re.S,
    )
    if first is None:
        return []
    declared_type = first.group("type").strip()
    if not declared_type or declared_type.endswith(("=", ".", "->")):
        return []
    if not _is_audited_member_type(declared_type, enum_names):
        return []

    declarations = [
        (first.group("name"), declared_type, first.group("init") is not None)
    ]
    for part in parts[1:]:
        match = re.match(
            r"^(?:[*&]\s*)?(?P<name>[A-Za-z_]\w*)\s*(?:\[[^\]]+\])?\s*"
            r"(?::\s*\d+)?\s*(?P<init>=.*|\{.*\})?$",
            part,
            re.S,
        )
        if match is not None:
            declarations.append((match.group("name"), declared_type, match.group("init") is not None))
    return [(name, declared_type, initialized or False) for name, declared_type, initialized in declarations]


def _constructors(
    class_name: str,
    body: str,
    require_scope: bool = False,
) -> list[tuple[str, str, str, bool]]:
    constructors: list[tuple[str, str, str, bool]] = []
    name_pattern = (
        rf"\b{re.escape(class_name)}\s*::\s*{re.escape(class_name)}"
        if require_scope
        else rf"\b{re.escape(class_name)}"
    )
    pattern = re.compile(
        rf"(?:\bexplicit\s+)?(?<!~){name_pattern}\s*\(([^)]*)\)\s*"
        rf"(?::\s*([^{{]*))?\{{",
        re.S,
    )
    for match in pattern.finditer(body):
        opening = match.end() - 1
        closing = _matching_brace(body, opening)
        if closing is None:
            continue
        parameters = " ".join(match.group(1).split())
        initializer = match.group(2) or ""
        constructor_body = body[opening + 1:closing]
        delegating = re.search(rf"(?:^|,)\s*{re.escape(class_name)}\s*\(", initializer) is not None
        constructors.append((f"{class_name}({parameters})", initializer, constructor_body, delegating))
    return constructors


def _scoped_constructor_index(text: str) -> dict[str, list[tuple[str, str, str, bool]]]:
    constructors: dict[str, list[tuple[str, str, str, bool]]] = {}
    pattern = re.compile(
        r"\b(?P<class>[A-Za-z_]\w*)\s*::\s*(?P=class)\s*\((?P<parameters>[^)]*)\)\s*"
        r"(?::\s*(?P<initializer>[^\{]*))?\{",
        re.S,
    )
    for match in pattern.finditer(text):
        opening = match.end() - 1
        closing = _matching_brace(text, opening)
        if closing is None:
            continue
        class_name = match.group("class")
        parameters = " ".join(match.group("parameters").split())
        initializer = match.group("initializer") or ""
        constructor_body = text[opening + 1:closing]
        delegating = re.search(rf"(?:^|,)\s*{re.escape(class_name)}\s*\(", initializer) is not None
        constructors.setdefault(class_name, []).append((
            f"{class_name}({parameters})",
            initializer,
            constructor_body,
            delegating,
        ))
    return constructors


def _constructor_initializes(member: str, initializer: str, body: str) -> bool:
    if re.search(rf"(?:^|,)\s*{re.escape(member)}\s*[({{]", initializer):
        return True
    assignment = re.search(rf"(?:\bthis\s*->\s*)?\b{re.escape(member)}\s*=", body)
    if assignment is None:
        return False
    return re.search(rf"\b{re.escape(member)}\b", body[:assignment.start()]) is None


def initialization_candidates_from_files(
    project: Path,
    source_files: list[Path],
) -> list[InitializationCandidate]:
    project = Path(project).resolve()
    candidates: list[InitializationCandidate] = []
    unique_paths = sorted(
        {Path(path).resolve() for path in source_files},
        key=lambda path: path.relative_to(project).as_posix(),
    )
    masked_sources = {
        path: _mask_comments_and_literals(path.read_text(encoding="utf-8", errors="replace"))
        for path in unique_paths
    }
    combined_sources = "\n".join(masked_sources.values())
    scoped_constructors = _scoped_constructor_index(combined_sources)
    enum_names = set(re.findall(r"\benum(?:\s+class)?\s+([A-Za-z_]\w*)", combined_sources))
    for path in unique_paths:
        if path.suffix not in {".h", ".hpp"}:
            continue
        original = path.read_text(encoding="utf-8", errors="replace")
        masked = masked_sources[path]
        for class_match in re.finditer(r"\b(?:class|struct)\s+([A-Za-z_]\w*)[^;{]*\{", masked):
            class_name = class_match.group(1)
            opening = class_match.end() - 1
            closing = _matching_brace(masked, opening)
            if closing is None:
                continue
            body = masked[opening + 1:closing]
            constructors = _constructors(class_name, body)
            constructors.extend(scoped_constructors.get(class_name, ()))
            constructors = list(dict.fromkeys(constructors))
            effective = [constructor for constructor in constructors if not constructor[3]]
            constructor_names = tuple(constructor[0] for constructor in constructors) or (f"{class_name}() <implicit>",)
            for offset, statement in _top_level_statements(body):
                ambiguous = "#" in statement
                for member, declared_type, initialized in _member_declarations(statement, enum_names):
                    if initialized:
                        continue
                    omissions = not effective or any(
                        not _constructor_initializes(member, initializer, constructor_body)
                        for _, initializer, constructor_body, _ in effective
                    )
                    if not omissions:
                        continue
                    line = original.count("\n", 0, opening + 1 + offset) + 1
                    category = "manual_review" if ambiguous else "java_default_missing"
                    reason = (
                        "Preprocessor-controlled declaration requires manual constructor analysis"
                        if ambiguous
                        else "At least one non-delegating constructor omits Java-compatible initialization"
                    )
                    candidates.append(InitializationCandidate(
                        path=path.relative_to(project).as_posix(),
                        line=line,
                        class_name=class_name,
                        member=member,
                        declared_type=declared_type,
                        constructors=constructor_names,
                        category=category,
                        reason=reason,
                    ))
    unique = {
        (item.path, item.line, item.class_name, item.member, item.category): item
        for item in candidates
    }
    return sorted(unique.values(), key=lambda item: (item.path, item.line, item.class_name, item.member))


def find_initialization_candidates(project: Path) -> list[InitializationCandidate]:
    project = Path(project).resolve()
    return initialization_candidates_from_files(project, common_pc_source_files(project))


def target_classes(source: JavaSource) -> set[str]:
    return {
        Path(name).stem
        for name in source.java_paths()
        if name.startswith(JavaSource._PACKAGE_PREFIX)
    }


def cpp_classes(source_dir: Path) -> set[str]:
    return {p.stem for p in source_dir.iterdir() if p.is_file() and p.suffix in SOURCE_EXTENSIONS}


def packet_mappings_java(source: JavaSource) -> list[tuple[int, bool, bool, str]]:
    text = source.read("net/minecraft/src/Packet.java")
    return [(int(i), c == "true", s == "true", n) for i, c, s, n in PACKET_JAVA_RE.findall(text)]


def packet_mappings_cpp(packet_cpp: Path) -> list[tuple[int, bool, bool, str]]:
    text = packet_cpp.read_text(encoding="utf-8", errors="replace")
    return [(int(i), c == "true", s == "true", n) for i, c, s, n in PACKET_CPP_RE.findall(text)]


def missing_local_includes(project: Path, source_dir: Path) -> list[tuple[Path, int, str]]:
    missing: list[tuple[Path, int, str]] = []
    roots = [source_dir, project / "src"]
    for path in source_dir.iterdir():
        if not path.is_file() or path.suffix not in SOURCE_EXTENSIONS:
            continue
        for line_no, line in enumerate(path.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
            match = re.match(r'\s*#include\s+"([^"]+)"', line)
            if not match:
                continue
            include = match.group(1)
            candidates = [path.parent / include] + [root / include for root in roots]
            if not any(candidate.exists() for candidate in candidates):
                # External headers are intentionally resolved by the toolchain.
                if "/" not in include and include.startswith(("SDL", "stb_", "glad")):
                    continue
                missing.append((path, line_no, include))
    return missing


def stub_markers(source_dir: Path) -> list[tuple[Path, int, str]]:
    found: list[tuple[Path, int, str]] = []
    for path in source_dir.iterdir():
        if not path.is_file() or path.suffix not in SOURCE_EXTENSIONS:
            continue
        for line_no, line in enumerate(path.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
            if STUB_RE.search(line):
                # Known platform documentation comments are not implementation stubs.
                lowered = line.lower()
                if "intentionally avoided" in lowered or "framebuffer readback is not implemented" in lowered:
                    continue
                found.append((path, line_no, line.strip()))
    return found



def incomplete_block_dereferences(source_dir: Path) -> list[tuple[Path, str, str]]:
    """Find Block::<name>-> dereferences whose static type is only forward-declared."""
    block_header = source_dir / "Block.h"
    if not block_header.exists():
        return []
    text = block_header.read_text(encoding="utf-8", errors="replace")
    block_vars = {
        match.group(2): match.group(1)
        for match in re.finditer(r"\bstatic\s+(Block\w*)\s*\*\s*(\w+)\s*;", text)
    }
    issues: list[tuple[Path, str, str]] = []
    for path in source_dir.iterdir():
        if not path.is_file() or path.suffix not in SOURCE_EXTENSIONS:
            continue
        source = path.read_text(encoding="utf-8", errors="replace")
        includes = set(re.findall(r'#include\s+"([^"]+)"', source))
        for member, concrete_type in block_vars.items():
            if concrete_type == "Block":
                continue
            if not re.search(rf"Block::{re.escape(member)}\s*->", source):
                continue
            required_header = f"{concrete_type}.h"
            if (source_dir / required_header).exists() and required_header not in includes:
                issues.append((path, member, required_header))
    return issues


def invalid_jstring_constexpr(source_dir: Path) -> list[tuple[Path, int, str]]:
    """A jstring is not constexpr-constructible in the Java compatibility layer."""
    issues: list[tuple[Path, int, str]] = []
    for path in source_dir.iterdir():
        if not path.is_file() or path.suffix not in {".h", ".hpp"}:
            continue
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        if not any(re.search(r"\bjstring\b", line) for line in lines):
            continue
        class_names = re.findall(r"\bclass\s+(\w+)", "\n".join(lines))
        for class_name in class_names:
            pattern = re.compile(rf"\bconstexpr\s+{re.escape(class_name)}\s*\(")
            for line_no, line in enumerate(lines, 1):
                if pattern.search(line):
                    issues.append((path, line_no, line.strip()))
    return issues


def known_ambiguous_null_overloads(project: Path) -> list[tuple[Path, int, str]]:
    """Catch integral zero literals at startWorld's integer/pointer overload boundary."""
    path = project / "src/client/Minecraft.cpp"
    if not path.exists():
        return []
    issues: list[tuple[Path, int, str]] = []
    pattern = re.compile(r"\bstartWorld\s*\([^;]*,\s*0L\s*\)")
    for line_no, line in enumerate(path.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
        if pattern.search(line):
            issues.append((path, line_no, line.strip()))
    return issues


def audit_finding_key(candidate: InitializationCandidate) -> str:
    return "\t".join((candidate.path, candidate.class_name, candidate.member, candidate.category))


def load_audit_status(path: Path) -> dict[str, dict[str, str]]:
    path = Path(path)
    if not path.exists():
        return {}
    document = json.loads(path.read_text(encoding="utf-8"))
    entries = document.get("findings", {})
    if not isinstance(entries, dict):
        raise ValueError("Audit status 'findings' must be an object")
    allowed = {"confirmed", "safe", "manual_review"}
    for key, entry in entries.items():
        if not isinstance(entry, dict) or entry.get("status") not in allowed:
            raise ValueError(f"Invalid audit status for {key}")
        if not isinstance(entry.get("evidence"), str) or not entry["evidence"].strip():
            raise ValueError(f"Audit status evidence is required for {key}")
    return entries


def build_common_pc_audit_report(
    candidates: list[InitializationCandidate],
    files_scanned: int,
    status_entries: dict[str, dict[str, str]],
) -> dict:
    findings = []
    known_keys = set()
    for candidate in sorted(candidates, key=lambda item: (item.path, item.line, item.class_name, item.member)):
        key = audit_finding_key(candidate)
        known_keys.add(key)
        overlay = status_entries.get(key)
        status = "candidate"
        evidence = ""
        if overlay is not None:
            status = overlay.get("status", "")
            evidence = overlay.get("evidence", "")
            if status not in {"confirmed", "safe", "manual_review"}:
                raise ValueError(f"Invalid audit status for {key}")
            if not isinstance(evidence, str) or not evidence.strip():
                raise ValueError(f"Audit status evidence is required for {key}")
        findings.append({
            "path": candidate.path,
            "line": candidate.line,
            "class_name": candidate.class_name,
            "member": candidate.member,
            "declared_type": candidate.declared_type,
            "constructors": list(candidate.constructors),
            "category": candidate.category,
            "reason": candidate.reason,
            "status": status,
            "evidence": evidence,
        })
    unknown_keys = sorted(set(status_entries) - known_keys)
    if unknown_keys:
        raise ValueError(f"Audit status does not match a current finding: {unknown_keys[0]}")
    return {
        "schema_version": 1,
        "scope": "common-pc",
        "summary": {
            "files_scanned": files_scanned,
            "initialization_candidates": len(findings),
            "confirmed_defects": sum(finding["status"] == "confirmed" for finding in findings),
        },
        "findings": findings,
    }


def audit_report_json(report: dict) -> str:
    return json.dumps(report, indent=2, sort_keys=True, ensure_ascii=False) + "\n"


def audit_report_markdown(report: dict) -> str:
    summary = report["summary"]
    lines = [
        "# Common and PC Port Audit",
        "",
        "This report contains static initialization candidates. A candidate is not a confirmed defect until its status and evidence say so.",
        "",
        "## Summary",
        "",
        f"- Files scanned: {summary['files_scanned']}",
        f"- Initialization candidates: {summary['initialization_candidates']}",
        f"- Confirmed defects: {summary['confirmed_defects']}",
    ]
    categories = sorted({finding["category"] for finding in report["findings"]})
    for category in categories:
        lines.extend(("", f"## {category}", ""))
        for finding in report["findings"]:
            if finding["category"] != category:
                continue
            lines.extend((
                f"### `{finding['path']}:{finding['line']}` — `{finding['class_name']}::{finding['member']}`",
                "",
                f"- **Status:** {finding['status']}",
                f"- **Type:** `{finding['declared_type']}`",
                f"- **Reason:** {finding['reason']}",
                f"- **Constructors:** {', '.join(finding['constructors'])}",
                f"- **Evidence:** {finding['evidence'] or 'Not classified'}",
                "",
            ))
    return "\n".join(lines).rstrip() + "\n"


def write_audit_json(path: Path, report: dict) -> None:
    Path(path).write_text(audit_report_json(report), encoding="utf-8", newline="\n")


def write_audit_markdown(path: Path, report: dict) -> None:
    Path(path).write_text(audit_report_markdown(report), encoding="utf-8", newline="\n")

def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--project", default=".")
    parser.add_argument("--target-java", required=True)
    parser.add_argument("--audit-common-pc", action="store_true")
    parser.add_argument("--audit-status")
    parser.add_argument("--audit-json")
    parser.add_argument("--audit-markdown")
    args = parser.parse_args(argv)

    audit_paths = (args.audit_status, args.audit_json, args.audit_markdown)
    if args.audit_common_pc and not all(audit_paths):
        parser.error("--audit-common-pc requires --audit-status, --audit-json and --audit-markdown")

    project = Path(args.project).resolve()
    target_zip = Path(args.target_java).resolve()
    source_dir = project / "src/net/minecraft/src"

    failures = 0

    java_source = JavaSource(target_zip)
    target = target_classes(java_source)
    present = cpp_classes(source_dir)
    missing = sorted(target - present)
    print(f"Target classes: {len(target)}")
    print(f"C++ class names present: {len(target & present)}")
    if missing:
        failures += 1
        print(f"ERROR: {len(missing)} target classes have no C++ file")
        for name in missing[:50]:
            print(f"  {name}")
    else:
        print("OK: every target Java class has a matching C++ file name")

    java_packets = packet_mappings_java(java_source)
    cpp_packets = packet_mappings_cpp(source_dir / "Packet.cpp")
    if java_packets != cpp_packets:
        failures += 1
        print("ERROR: Packet registration table differs from Minecraft 1.2.5")
        java_set, cpp_set = set(java_packets), set(cpp_packets)
        for entry in sorted(java_set - cpp_set):
            print("  missing C++ mapping:", entry)
        for entry in sorted(cpp_set - java_set):
            print("  extra C++ mapping:", entry)
    else:
        print(f"OK: packet registration table matches all {len(java_packets)} Java mappings")

    includes = missing_local_includes(project, source_dir)
    if includes:
        failures += 1
        print(f"ERROR: {len(includes)} unresolved local includes in Minecraft sources")
        for path, line_no, include in includes[:50]:
            print(f"  {path.relative_to(project)}:{line_no}: {include}")
    else:
        print("OK: Minecraft-source local includes resolve")

    markers = stub_markers(source_dir)
    if markers:
        failures += 1
        print(f"ERROR: {len(markers)} explicit incomplete-code markers")
        for path, line_no, text in markers[:50]:
            print(f"  {path.relative_to(project)}:{line_no}: {text}")
    else:
        print("OK: no explicit incomplete-code markers in Minecraft sources")

    block_derefs = incomplete_block_dereferences(source_dir)
    if block_derefs:
        failures += 1
        print(f"ERROR: {len(block_derefs)} concrete Block pointers are dereferenced without their defining header")
        for path, member, header in block_derefs[:50]:
            print(f"  {path.relative_to(project)}: Block::{member}-> requires {header}")
    else:
        print("OK: concrete Block pointer dereferences include their defining headers")

    constexpr_issues = invalid_jstring_constexpr(source_dir)
    if constexpr_issues:
        failures += 1
        print(f"ERROR: {len(constexpr_issues)} constexpr constructors contain jstring state")
        for path, line_no, text in constexpr_issues[:50]:
            print(f"  {path.relative_to(project)}:{line_no}: {text}")
    else:
        print("OK: no constexpr constructors conflict with jstring state")

    ambiguous_calls = known_ambiguous_null_overloads(project)
    if ambiguous_calls:
        failures += 1
        print(f"ERROR: {len(ambiguous_calls)} ambiguous startWorld zero-literal call(s)")
        for path, line_no, text in ambiguous_calls:
            print(f"  {path.relative_to(project)}:{line_no}: {text}")
    else:
        print("OK: no known ambiguous startWorld zero-literal calls")

    if args.audit_common_pc:
        source_files = common_pc_source_files(project)
        candidates = initialization_candidates_from_files(project, source_files)
        status_entries = load_audit_status(Path(args.audit_status))
        report = build_common_pc_audit_report(candidates, len(source_files), status_entries)
        write_audit_json(Path(args.audit_json), report)
        write_audit_markdown(Path(args.audit_markdown), report)
        print(f"Common/PC audit candidates: {len(candidates)}")

    java_source.close()
    if failures:
        print(f"Static verification failed in {failures} category/categories")
        return 1
    print("Static verification passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
