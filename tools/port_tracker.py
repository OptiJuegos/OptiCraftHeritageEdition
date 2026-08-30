#!/usr/bin/env python3
"""Minecraft Java -> Beta++ C++ port progress tracker.

Generates a deterministic Markdown report describing target Java classes, the
current C++ tree, baseline Java changes, dependency pressure, protocol changes,
and optional manual port status overrides.

The script intentionally uses only the Python standard library so it can run on
Windows without installing packages.
"""

from __future__ import annotations

import argparse
import collections
import dataclasses
import datetime as dt
import hashlib
import json
import os
import re
import sys
import tempfile
import zipfile
from pathlib import Path
from typing import Counter, Dict, Iterable, Iterator, List, Mapping, MutableMapping, Optional, Sequence, Set, Tuple


JAVA_EXT = ".java"
CPP_SOURCE_EXTS = {".c", ".cc", ".cpp", ".cxx"}
CPP_HEADER_EXTS = {".h", ".hh", ".hpp", ".hxx"}
CPP_EXTS = CPP_SOURCE_EXTS | CPP_HEADER_EXTS

JAVA_KEYWORDS = {
    "abstract", "assert", "boolean", "break", "byte", "case", "catch", "char",
    "class", "const", "continue", "default", "do", "double", "else", "enum",
    "extends", "final", "finally", "float", "for", "goto", "if", "implements",
    "import", "instanceof", "int", "interface", "long", "native", "new",
    "package", "private", "protected", "public", "return", "short", "static",
    "strictfp", "super", "switch", "synchronized", "this", "throw", "throws",
    "transient", "try", "void", "volatile", "while", "true", "false", "null",
}

CONTROL_KEYWORDS = {
    "if", "else", "for", "while", "switch", "case", "catch", "try", "throw",
    "return", "new", "instanceof", "synchronized", "break", "continue",
}

STUB_PATTERNS = [
    re.compile(r"\bTODO\b", re.IGNORECASE),
    re.compile(r"\bFIXME\b", re.IGNORECASE),
    re.compile(r"not\s+implemented", re.IGNORECASE),
    re.compile(r"throw\s+std::(?:logic_error|runtime_error)\s*\(\s*[\"'](?:not implemented|todo)", re.IGNORECASE),
    re.compile(r"assert\s*\(\s*false\s*\)", re.IGNORECASE),
]

JAVA_METHOD_RE = re.compile(
    r"(?m)^\s*(?:public|protected|private)?\s*(?:static\s+)?(?:final\s+)?"
    r"(?:synchronized\s+)?(?:native\s+)?(?:abstract\s+)?"
    r"(?:[\w$<>\[\].?,]+\s+)?([A-Za-z_$][\w$]*)\s*\(([^;{}()]*)\)"
    r"\s*(?:throws\s+[^{;]+)?\s*(?:\{|;)",
)

JAVA_DECL_RE = re.compile(r"\b(?:class|interface|enum)\s+([A-Za-z_$][\w$]*)")
CLASS_REF_RE = re.compile(r"\b[A-Z][A-Za-z0-9_$]*\b")
TOKEN_RE = re.compile(
    r"[A-Za-z_$][A-Za-z0-9_$]*|(?:0[xX][0-9A-Fa-f]+|\d+(?:\.\d+)?)|"
    r"==|!=|<=|>=|&&|\|\||<<|>>>|>>|\+\+|--|[{}()\[\];,.?:+\-*/%&|^!~=<>]"
)

PACKET_IO_RE = re.compile(
    r"\b(?:read|write)(?:Byte|UnsignedByte|Short|UnsignedShort|Int|Long|Float|Double|Boolean|Char|UTF|Fully)\b"
    r"|\.read\s*\(|\.write\s*\(",
    re.IGNORECASE,
)

IMPORT_RE = re.compile(r"(?m)^\s*import\s+([A-Za-z0-9_.$*]+)\s*;")

PLATFORM_RISK_RULES: Sequence[Tuple[str, re.Pattern[str]]] = (
    ("graphics-backend", re.compile(r"\b(?:GL11|GL12|GL13|OpenGlHelper|org\.lwjgl\.opengl|Tessellator)\b")),
    ("input-backend", re.compile(r"\b(?:Keyboard|Mouse|Controllers|org\.lwjgl\.input)\b")),
    ("threading", re.compile(r"\b(?:Thread|Runnable|synchronized|wait\s*\(|notify(?:All)?\s*\()\b")),
    ("filesystem", re.compile(r"\b(?:File|RandomAccessFile|FileInputStream|FileOutputStream|java\.io)\b")),
    ("network", re.compile(r"\b(?:Socket|ServerSocket|URL|HttpURLConnection|java\.net)\b")),
    ("desktop-only", re.compile(r"\b(?:java\.awt|javax\.swing|Applet|Canvas|Frame)\b")),
    ("native-buffer", re.compile(r"\b(?:ByteBuffer|IntBuffer|FloatBuffer|ShortBuffer|BufferUtils)\b")),
    ("audio-backend", re.compile(r"\b(?:SoundSystem|paulscode|OpenAL|SoundManager)\b")),
)


@dataclasses.dataclass(frozen=True)
class SourceFile:
    name: str
    relative_path: str
    text: str
    lines: int
    sha256: str


@dataclasses.dataclass
class JavaClassInfo:
    name: str
    file: SourceFile
    category: str
    methods: Set[Tuple[str, int]]
    dependencies: Set[str]
    baseline_state: str = "new"
    baseline_similarity: Optional[float] = None
    added_methods: Set[Tuple[str, int]] = dataclasses.field(default_factory=set)
    removed_methods: Set[Tuple[str, int]] = dataclasses.field(default_factory=set)
    cpp_headers: List[str] = dataclasses.field(default_factory=list)
    cpp_sources: List[str] = dataclasses.field(default_factory=list)
    cpp_lines: int = 0
    stub_markers: int = 0
    manual_status: str = "untracked"
    manual_priority: str = ""
    manual_notes: str = ""
    dependents: Set[str] = dataclasses.field(default_factory=set)
    risk_tags: Set[str] = dataclasses.field(default_factory=set)

    @property
    def cpp_presence(self) -> str:
        if self.cpp_headers and self.cpp_sources:
            return "header+source"
        if self.cpp_headers:
            return "header-only"
        if self.cpp_sources:
            return "source-only"
        return "missing"

    @property
    def is_missing(self) -> bool:
        return not self.cpp_headers and not self.cpp_sources

    @property
    def change_percent(self) -> Optional[int]:
        if self.baseline_similarity is None:
            return None
        return int(round((1.0 - self.baseline_similarity) * 100.0))


@dataclasses.dataclass
class ManualStatus:
    status: str = "untracked"
    priority: str = ""
    notes: str = ""


class SourceTree:
    def __init__(self, source: Path, suffixes: Set[str]) -> None:
        self.source = source
        self.suffixes = {s.lower() for s in suffixes}
        self._tempdir: Optional[tempfile.TemporaryDirectory[str]] = None
        self.root = self._resolve_root(source)

    def _resolve_root(self, source: Path) -> Path:
        if source.is_dir():
            return source.resolve()
        if source.is_file() and zipfile.is_zipfile(source):
            self._tempdir = tempfile.TemporaryDirectory(prefix="port_tracker_")
            with zipfile.ZipFile(source) as archive:
                archive.extractall(self._tempdir.name)
            return Path(self._tempdir.name)
        raise FileNotFoundError(f"Source does not exist or is not a supported ZIP/directory: {source}")

    def files(self) -> Iterator[Path]:
        for path in self.root.rglob("*"):
            if path.is_file() and path.suffix.lower() in self.suffixes:
                yield path

    def close(self) -> None:
        if self._tempdir is not None:
            self._tempdir.cleanup()
            self._tempdir = None

    def __enter__(self) -> "SourceTree":
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def source_file(path: Path, root: Path) -> SourceFile:
    text = read_text(path)
    relative = path.relative_to(root).as_posix()
    return SourceFile(
        name=path.stem,
        relative_path=relative,
        text=text,
        lines=count_code_lines(text),
        sha256=hashlib.sha256(text.encode("utf-8", errors="replace")).hexdigest(),
    )


def count_code_lines(text: str) -> int:
    return sum(1 for line in text.splitlines() if line.strip())


def strip_comments_and_strings(text: str) -> str:
    # This is intentionally lexical rather than a full parser. It is sufficient
    # for stable structural comparisons while avoiding comment/name noise.
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.DOTALL)
    text = re.sub(r"//[^\n]*", " ", text)
    text = re.sub(r'"(?:\\.|[^"\\])*"', ' "STR" ', text)
    text = re.sub(r"'(?:\\.|[^'\\])*'", " 'CHR' ", text)
    return text


def java_methods(text: str, class_name: str) -> Set[Tuple[str, int]]:
    clean = strip_comments_and_strings(text)
    methods: Set[Tuple[str, int]] = set()
    for match in JAVA_METHOD_RE.finditer(clean):
        name = match.group(1)
        if name in CONTROL_KEYWORDS:
            continue
        args = match.group(2).strip()
        arity = 0 if not args else len(split_arguments(args))
        methods.add((name, arity))
    return methods


def split_arguments(args: str) -> List[str]:
    parts: List[str] = []
    current: List[str] = []
    depth = 0
    for char in args:
        if char in "<([":
            depth += 1
        elif char in ">)]" and depth > 0:
            depth -= 1
        if char == "," and depth == 0:
            parts.append("".join(current).strip())
            current = []
        else:
            current.append(char)
    if current:
        parts.append("".join(current).strip())
    return [part for part in parts if part]


def structural_counter(text: str, known_classes: Set[str]) -> Counter[str]:
    clean = strip_comments_and_strings(text)
    tokens = TOKEN_RE.findall(clean)
    normalized: List[str] = []
    for token in tokens:
        if token in JAVA_KEYWORDS:
            normalized.append(token)
        elif token in known_classes:
            normalized.append("CLASS:" + token)
        elif re.match(r"^(?:0[xX][0-9A-Fa-f]+|\d)", token):
            normalized.append("NUM")
        elif re.match(r"^[A-Za-z_$]", token):
            normalized.append("ID")
        else:
            normalized.append(token)

    # Local token trigrams retain ordering information without expensive full
    # sequence alignment on large decompiled classes.
    grams: Counter[str] = collections.Counter()
    for i in range(max(0, len(normalized) - 2)):
        grams[" ".join(normalized[i : i + 3])] += 1
    if not grams and normalized:
        grams.update(normalized)
    return grams


def counter_dice(a: Counter[str], b: Counter[str]) -> float:
    if not a and not b:
        return 1.0
    if not a or not b:
        return 0.0
    intersection = sum((a & b).values())
    return (2.0 * intersection) / (sum(a.values()) + sum(b.values()))


def extract_dependencies(text: str, class_name: str, known_classes: Set[str]) -> Set[str]:
    clean = strip_comments_and_strings(text)
    refs = set(CLASS_REF_RE.findall(clean))
    refs.intersection_update(known_classes)
    refs.discard(class_name)
    return refs


def extract_imports(text: str) -> Set[str]:
    return set(IMPORT_RE.findall(strip_comments_and_strings(text)))


def platform_risks(text: str) -> Set[str]:
    clean = strip_comments_and_strings(text)
    return {name for name, pattern in PLATFORM_RISK_RULES if pattern.search(clean)}


def is_concrete_packet(name: str) -> bool:
    return re.match(r"^Packet\d", name) is not None


def suggested_cpp_paths(info: JavaClassInfo) -> Tuple[str, str]:
    path = info.file.relative_path.replace("\\", "/")
    if "/net/minecraft/client/" in "/" + path or path.startswith("src/net/minecraft/client/"):
        base = "src/client"
    else:
        base = "src/net/minecraft/src"
    return f"{base}/{info.name}.h", f"{base}/{info.name}.cpp"


def categorize(name: str, relative_path: str = "") -> str:
    if name.startswith("Packet") or name in {
        "NetworkManager", "NetHandler", "NetClientHandler", "NetLoginHandler",
        "TcpConnection", "ThreadConnectToServer", "ThreadLoginVerifier",
    }:
        return "Network / Protocol"
    if name.startswith("EntityAI") or name in {
        "PathNavigate", "EntityMoveHelper", "EntityLookHelper", "EntityJumpHelper",
        "EntitySenses", "EntityCreature", "EntityLiving", "EntityMob", "EntityAnimal",
    }:
        return "Entities / AI"
    if name.startswith("Entity") or name.startswith("Model") and "Model" in name:
        return "Entities / Models"
    if name.startswith("Render") or name.startswith("Texture") or name.startswith("Tessellator") or name in {
        "OpenGlHelper", "ActiveRenderInfo", "ClippingHelper", "Frustrum", "GLAllocation",
        "ItemRenderer", "MapItemRenderer", "TileEntityRenderer", "WorldRenderer",
    }:
        return "Rendering"
    if name.startswith("Gui") or name in {"ScaledResolution", "FontRenderer"}:
        return "GUI"
    if name.startswith("Block") or name in {"Material", "MapColor", "StepSound"}:
        return "Blocks"
    if name.startswith("Item") or name.startswith("Container") or name.startswith("Slot") or name.endswith("Recipes") or name in {
        "InventoryPlayer", "IInventory", "CraftingManager", "FurnaceRecipes", "ShapedRecipes", "ShapelessRecipes",
    }:
        return "Items / Inventory / Crafting"
    if name.startswith("NBT") or name.startswith("Save") or name.startswith("Anvil") or name in {
        "RegionFile", "RegionFileCache", "CompressedStreamTools", "ThreadedFileIOBase", "IThreadedFileIO",
    }:
        return "Save / NBT / Anvil"
    if name.startswith("Biome") or name.startswith("GenLayer") or name.startswith("WorldGen") or name.startswith("MapGen") or name.startswith("Structure") or name.startswith("Component") or name in {
        "ChunkProviderGenerate", "ChunkProviderHell", "ChunkProviderEnd", "WorldChunkManager", "NoiseGenerator",
    }:
        return "Worldgen / Biomes / Structures"
    if name.startswith("Chunk") or name.startswith("ExtendedBlockStorage") or name in {
        "World", "WorldClient", "WorldServer", "WorldProvider", "WorldProviderHell", "WorldProviderEnd",
        "WorldInfo", "WorldSettings", "WorldType", "IBlockAccess", "EnumSkyBlock",
    }:
        return "World / Chunks"
    if name.startswith("TileEntity"):
        return "Tile Entities"
    if name.startswith("Potion") or name.startswith("Enchantment") or name in {"FoodStats", "PlayerCapabilities"}:
        return "Gameplay Systems"
    if name.startswith("Achievement") or name.startswith("Stat"):
        return "Stats / Achievements"
    if name.startswith("Sound") or name.startswith("Mus"):
        return "Audio"
    if "minecraft/client" in relative_path.replace("\\", "/") or name in {"Minecraft", "MinecraftApplet"}:
        return "Client Core"
    return "Other"


def load_java_tree(source: Path) -> Dict[str, SourceFile]:
    files: Dict[str, SourceFile] = {}
    with SourceTree(source, {JAVA_EXT}) as tree:
        for path in tree.files():
            item = source_file(path, tree.root)
            # MCP uses one top-level class per source file for these versions.
            # If duplicate stems ever exist, prefer the net/minecraft source.
            old = files.get(item.name)
            if old is None or "net/minecraft" in item.relative_path:
                files[item.name] = item
    return files


def index_cpp_tree(project_root: Path) -> Dict[str, List[SourceFile]]:
    src_root = project_root / "src"
    if not src_root.exists():
        src_root = project_root
    result: Dict[str, List[SourceFile]] = collections.defaultdict(list)
    for path in src_root.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in CPP_EXTS:
            continue
        item = source_file(path, project_root)
        result[path.stem].append(item)
    return dict(result)


def load_manual_status(path: Optional[Path]) -> Dict[str, ManualStatus]:
    if path is None or not path.exists():
        return {}
    raw = json.loads(path.read_text(encoding="utf-8"))
    classes = raw.get("classes", raw) if isinstance(raw, dict) else {}
    result: Dict[str, ManualStatus] = {}
    for name, value in classes.items():
        if isinstance(value, str):
            result[name] = ManualStatus(status=value)
        elif isinstance(value, dict):
            result[name] = ManualStatus(
                status=str(value.get("status", "untracked")),
                priority=str(value.get("priority", "")),
                notes=str(value.get("notes", "")),
            )
    return result


def build_infos(
    target: Mapping[str, SourceFile],
    baseline: Mapping[str, SourceFile],
    cpp: Mapping[str, List[SourceFile]],
    manual: Mapping[str, ManualStatus],
) -> Dict[str, JavaClassInfo]:
    known_classes = set(target)
    baseline_classes = set(baseline)
    all_known_for_similarity = known_classes | baseline_classes
    infos: Dict[str, JavaClassInfo] = {}

    baseline_struct: Dict[str, Counter[str]] = {}
    target_struct: Dict[str, Counter[str]] = {}

    for name, item in target.items():
        methods = java_methods(item.text, name)
        deps = extract_dependencies(item.text, name, known_classes)
        info = JavaClassInfo(
            name=name,
            file=item,
            category=categorize(name, item.relative_path),
            methods=methods,
            dependencies=deps,
        )

        cpp_files = cpp.get(name, [])
        info.cpp_headers = sorted(f.relative_path for f in cpp_files if Path(f.relative_path).suffix.lower() in CPP_HEADER_EXTS)
        info.cpp_sources = sorted(f.relative_path for f in cpp_files if Path(f.relative_path).suffix.lower() in CPP_SOURCE_EXTS)
        info.cpp_lines = sum(f.lines for f in cpp_files)
        info.stub_markers = sum(count_stub_markers(f.text) for f in cpp_files)

        manual_entry = manual.get(name, ManualStatus())
        info.manual_status = manual_entry.status
        info.manual_priority = manual_entry.priority
        info.manual_notes = manual_entry.notes
        info.risk_tags = platform_risks(item.text)

        if name in baseline:
            info.baseline_state = "shared"
            target_struct[name] = structural_counter(item.text, all_known_for_similarity)
            baseline_struct[name] = structural_counter(baseline[name].text, all_known_for_similarity)
            info.baseline_similarity = counter_dice(target_struct[name], baseline_struct[name])
            baseline_methods = java_methods(baseline[name].text, name)
            info.added_methods = methods - baseline_methods
            info.removed_methods = baseline_methods - methods
        else:
            info.baseline_state = "new"
        infos[name] = info

    for name, info in infos.items():
        for dep in info.dependencies:
            if dep in infos:
                infos[dep].dependents.add(name)

    return infos


def count_stub_markers(text: str) -> int:
    return sum(len(pattern.findall(text)) for pattern in STUB_PATTERNS)


def packet_io_sequence(text: str) -> List[str]:
    clean = strip_comments_and_strings(text)
    sequence: List[str] = []
    for match in PACKET_IO_RE.finditer(clean):
        token = match.group(0)
        token = re.sub(r"\s+", "", token)
        sequence.append(token.lower())
    return sequence


def status_icon(status: str) -> str:
    normalized = status.lower().strip()
    return {
        "done": "DONE",
        "complete": "DONE",
        "in_progress": "WIP",
        "in-progress": "WIP",
        "wip": "WIP",
        "verify": "VERIFY",
        "blocked": "BLOCKED",
        "todo": "TODO",
        "untracked": "-",
    }.get(normalized, normalized.upper() if normalized else "-")


def pct(numerator: int, denominator: int) -> float:
    return (100.0 * numerator / denominator) if denominator else 0.0


def md_escape(value: object) -> str:
    text = str(value)
    return text.replace("|", "\\|").replace("\n", " ").replace("\r", " ")


def code_list(values: Iterable[str], limit: int = 6) -> str:
    seq = list(values)
    if not seq:
        return "-"
    shown = seq[:limit]
    text = ", ".join(f"`{md_escape(v)}`" for v in shown)
    if len(seq) > limit:
        text += f" +{len(seq) - limit}"
    return text


def format_methods(methods: Iterable[Tuple[str, int]], limit: int = 8) -> str:
    values = sorted(methods)
    if not values:
        return "-"
    shown = [f"`{name}/{arity}`" for name, arity in values[:limit]]
    if len(values) > limit:
        shown.append(f"+{len(values) - limit}")
    return ", ".join(shown)


def blocking_score(info: JavaClassInfo, infos: Mapping[str, JavaClassInfo]) -> int:
    direct_dependents = len(info.dependents)
    missing_dependents = sum(1 for name in info.dependents if infos[name].is_missing)
    core_bonus = 0
    if info.category in {"World / Chunks", "Rendering", "Network / Protocol", "Worldgen / Biomes / Structures"}:
        core_bonus = 4
    if info.name in {"ExtendedBlockStorage", "Chunk", "World", "WorldChunkManager", "ChunkProviderGenerate", "AnvilChunkLoader", "Tessellator"}:
        core_bonus += 10
    return direct_dependents * 2 + missing_dependents + core_bonus


def build_report(
    infos: Mapping[str, JavaClassInfo],
    target: Mapping[str, SourceFile],
    baseline: Mapping[str, SourceFile],
    cpp: Mapping[str, List[SourceFile]],
    args: argparse.Namespace,
) -> str:
    now = dt.datetime.now().astimezone()
    target_names = set(target)
    baseline_names = set(baseline)
    cpp_names = set(cpp)

    missing = [info for info in infos.values() if info.is_missing]
    present = [info for info in infos.values() if not info.is_missing]
    header_only = [info for info in infos.values() if info.cpp_presence == "header-only"]
    source_only = [info for info in infos.values() if info.cpp_presence == "source-only"]
    new_target = [info for info in infos.values() if info.baseline_state == "new"]
    shared = [info for info in infos.values() if info.baseline_state == "shared"]
    removed = sorted(baseline_names - target_names)
    cpp_extra = sorted(cpp_names - target_names)
    with_stubs = [info for info in infos.values() if info.stub_markers > 0]
    manually_done = [info for info in infos.values() if info.manual_status.lower() in {"done", "complete"}]
    manually_wip = [info for info in infos.values() if info.manual_status.lower() in {"wip", "in_progress", "in-progress"}]
    manually_verify = [info for info in infos.values() if info.manual_status.lower() == "verify"]

    categories: Dict[str, List[JavaClassInfo]] = collections.defaultdict(list)
    for info in infos.values():
        categories[info.category].append(info)

    packet_shared = sorted(name for name in target_names & baseline_names if is_concrete_packet(name))
    packet_changes: List[Tuple[str, bool, int, int]] = []
    for name in packet_shared:
        base_seq = packet_io_sequence(baseline[name].text)
        target_seq = packet_io_sequence(target[name].text)
        packet_changes.append((name, base_seq == target_seq, len(base_seq), len(target_seq)))

    high_changed = sorted(
        (info for info in shared if info.baseline_similarity is not None),
        key=lambda x: (x.baseline_similarity or 0.0, -x.file.lines),
    )

    blockers = sorted(
        missing,
        key=lambda x: (-blocking_score(x, infos), -len(x.dependents), -x.file.lines, x.name),
    )

    target_imports: Set[str] = set()
    baseline_imports: Set[str] = set()
    for item in target.values():
        target_imports.update(extract_imports(item.text))
    for item in baseline.values():
        baseline_imports.update(extract_imports(item.text))
    new_imports = sorted(target_imports - baseline_imports)
    removed_imports = sorted(baseline_imports - target_imports)
    risky = sorted((info for info in infos.values() if info.risk_tags), key=lambda x: (x.category, x.name))

    lines: List[str] = []
    add = lines.append

    add(f"# {args.report_title}")
    add("")
    add(f"> Generated by `tools/port_tracker.py` on {now.isoformat(timespec='seconds')}. Do not hand-edit generated sections; keep manual state in `{args.status_display}`.")
    add("")
    add("## Scope")
    add("")
    add(f"- Target Java: **{md_escape(args.target_version)}** (`{md_escape(str(args.target_java))}`)")
    if baseline:
        add(f"- Baseline Java: **{md_escape(args.baseline_version)}** (`{md_escape(str(args.baseline_java))}`)")
    add(f"- C++ project: `{md_escape(str(args.project))}`")
    add(f"- Report mode: filename/class matching + Java structural delta + dependency graph + manual status overlay")
    add("")

    add("## Executive summary")
    add("")
    add("| Metric | Count | Percent |")
    add("| --- | ---: | ---: |")
    add(f"| Target Java classes | {len(target)} | 100.0% |")
    add(f"| C++ class names present | {len(present)} | {pct(len(present), len(target)):.1f}% |")
    add(f"| Missing C++ class names | {len(missing)} | {pct(len(missing), len(target)):.1f}% |")
    add(f"| Header-only matches | {len(header_only)} | {pct(len(header_only), len(target)):.1f}% |")
    add(f"| Source-only matches | {len(source_only)} | {pct(len(source_only), len(target)):.1f}% |")
    add(f"| New classes vs baseline | {len(new_target)} | {pct(len(new_target), len(target)):.1f}% |")
    add(f"| Shared classes vs baseline | {len(shared)} | {pct(len(shared), len(target)):.1f}% |")
    add(f"| Removed baseline classes | {len(removed)} | - |")
    add(f"| Matching C++ classes with stub markers | {len(with_stubs)} | {pct(len(with_stubs), len(target)):.1f}% |")
    add(f"| Manual DONE | {len(manually_done)} | {pct(len(manually_done), len(target)):.1f}% |")
    add(f"| Manual WIP | {len(manually_wip)} | {pct(len(manually_wip), len(target)):.1f}% |")
    add(f"| Manual VERIFY | {len(manually_verify)} | {pct(len(manually_verify), len(target)):.1f}% |")
    add("")
    add("**Important:** `C++ class names present` means a matching `.h` and/or `.cpp` exists. It does **not** mean the 1.2.5 behavior is already ported. Use the Java delta and manual status columns to decide actual completion.")
    add("")

    add("## Progress by subsystem")
    add("")
    add("| Subsystem | Target | C++ present | Missing | New vs baseline | DONE | WIP | VERIFY |")
    add("| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |")
    for category in sorted(categories):
        group = categories[category]
        group_present = sum(not x.is_missing for x in group)
        group_missing = sum(x.is_missing for x in group)
        group_new = sum(x.baseline_state == "new" for x in group)
        group_done = sum(x.manual_status.lower() in {"done", "complete"} for x in group)
        group_wip = sum(x.manual_status.lower() in {"wip", "in_progress", "in-progress"} for x in group)
        group_verify = sum(x.manual_status.lower() == "verify" for x in group)
        add(f"| {md_escape(category)} | {len(group)} | {group_present} ({pct(group_present, len(group)):.1f}%) | {group_missing} | {group_new} | {group_done} | {group_wip} | {group_verify} |")
    add("")

    add("## Platform-sensitive target classes")
    add("")
    add("Classes whose Java source touches APIs that commonly need explicit PC/Wii/PS2 adaptation. A tag is a review signal, not proof that the class must be rewritten.")
    add("")
    add("| Class | Subsystem | Risk tags | C++ | Java LOC |")
    add("| --- | --- | --- | --- | ---: |")
    for info in risky:
        add(f"| `{info.name}` | {md_escape(info.category)} | {code_list(sorted(info.risk_tags), 10)} | {info.cpp_presence} | {info.file.lines} |")
    if not risky:
        add("| _None_ | - | - | - | - |")
    add("")

    add("## Java API/import delta")
    add("")
    add("New imports can expose compatibility work that a filename comparison cannot see, especially desktop, IO, networking, buffer, and concurrency APIs.")
    add("")
    add(f"- Imports used by target: **{len(target_imports)}**")
    add(f"- Imports used by baseline: **{len(baseline_imports)}**")
    add(f"- New imports vs baseline: {code_list(new_imports, 80)}")
    add(f"- Removed imports vs baseline: {code_list(removed_imports, 80)}")
    add("")

    add("## Recommended blockers to port first")
    add("")
    add("Automatically ranked from dependency pressure plus core-system weighting. This is a planning hint, not a build dependency guarantee.")
    add("")
    add("| Rank | Class | Subsystem | Java LOC | Dependents | Missing dependents | Score |")
    add("| ---: | --- | --- | ---: | ---: | ---: | ---: |")
    for rank, info in enumerate(blockers[: args.top_blockers], start=1):
        missing_dependents = sum(1 for name in info.dependents if infos[name].is_missing)
        add(f"| {rank} | `{info.name}` | {md_escape(info.category)} | {info.file.lines} | {len(info.dependents)} | {missing_dependents} | {blocking_score(info, infos)} |")
    if not blockers:
        add("| - | _No missing target classes_ | - | - | - | - | - |")
    add("")

    add("## Largest Java changes in shared classes")
    add("")
    add("Structural delta is computed from normalized Java token trigrams. It ignores most variable-name/decompiler noise but is still heuristic.")
    add("")
    add("| Class | Subsystem | Structural change | Beta LOC | Target LOC | Methods + | Methods - | C++ |")
    add("| --- | --- | ---: | ---: | ---: | ---: | ---: | --- |")
    for info in high_changed[: args.top_changed]:
        base_lines = baseline[info.name].lines
        add(
            f"| `{info.name}` | {md_escape(info.category)} | {info.change_percent}% | {base_lines} | {info.file.lines} | "
            f"{len(info.added_methods)} | {len(info.removed_methods)} | {info.cpp_presence} |"
        )
    add("")

    add("## Protocol delta")
    add("")
    target_packets = sorted(name for name in target if is_concrete_packet(name))
    baseline_packets = sorted(name for name in baseline if is_concrete_packet(name))
    add(f"- Target packets: **{len(target_packets)}**")
    add(f"- Baseline packets: **{len(baseline_packets)}**")
    add(f"- New packet classes: {code_list(sorted(set(target_packets) - set(baseline_packets)), 30)}")
    add(f"- Removed packet classes: {code_list(sorted(set(baseline_packets) - set(target_packets)), 30)}")
    io_changed = [name for name, same, _, _ in packet_changes if not same]
    add(f"- Shared packet classes whose primitive IO-call sequence changed: **{len(io_changed)}/{len(packet_changes)}**")
    add("")
    add("| Packet | Primitive IO sequence | Baseline ops | Target ops | C++ |")
    add("| --- | --- | ---: | ---: | --- |")
    for name, same, base_count, target_count in packet_changes:
        if same and not args.show_unchanged_packets:
            continue
        presence = infos[name].cpp_presence if name in infos else "missing"
        add(f"| `{name}` | {'same' if same else '**changed**'} | {base_count} | {target_count} | {presence} |")
    add("")

    add("## Missing target classes")
    add("")
    add("Every target Java class with no same-name C/C++ source or header in the current project.")
    add("")
    add("| Class | Subsystem | Java file | LOC | Baseline | Dependents | Dependencies | Priority | Manual status | Notes |")
    add("| --- | --- | --- | ---: | --- | ---: | ---: | --- | --- | --- |")
    for info in sorted(missing, key=lambda x: (x.category, x.name)):
        add(
            f"| `{info.name}` | {md_escape(info.category)} | `{md_escape(info.file.relative_path)}` | {info.file.lines} | "
            f"{info.baseline_state} | {len(info.dependents)} | {len(info.dependencies)} | {md_escape(info.manual_priority or '-')} | "
            f"{status_icon(info.manual_status)} | {md_escape(info.manual_notes or '-')} |"
        )
    if not missing:
        add("| _None_ | - | - | - | - | - | - | - | - | - |")
    add("")

    add("## Missing-class dependency map")
    add("")
    add("Detailed generated map for planning individual ports. `Suggested files` follows the current Beta++ layout and can be overridden manually if a class belongs in a platform-specific module.")
    add("")
    add("| Class | Suggested files | Depends on | Used by | Platform risks |")
    add("| --- | --- | --- | --- | --- |")
    for info in sorted(missing, key=lambda x: (-blocking_score(x, infos), x.name)):
        suggested = suggested_cpp_paths(info)
        add(
            f"| `{info.name}` | {code_list(suggested, 4)} | {code_list(sorted(info.dependencies), 12)} | "
            f"{code_list(sorted(info.dependents), 12)} | {code_list(sorted(info.risk_tags), 8)} |"
        )
    if not missing:
        add("| _None_ | - | - | - | - |")
    add("")

    add("## New classes in target Java")
    add("")
    add("Classes that do not exist in the baseline Java tree. These usually require a real new port rather than adapting an existing Beta++ class.")
    add("")
    add("| Class | Subsystem | Java LOC | C++ presence | Dependents | Direct dependencies |")
    add("| --- | --- | ---: | --- | ---: | ---: |")
    for info in sorted(new_target, key=lambda x: (x.category, x.name)):
        add(f"| `{info.name}` | {md_escape(info.category)} | {info.file.lines} | {info.cpp_presence} | {len(info.dependents)} | {len(info.dependencies)} |")
    add("")

    add("## Shared classes requiring review")
    add("")
    add("A matching C++ file already exists, but the target Java class differs from the baseline. These are the easiest classes to mistakenly mark as complete.")
    add("")
    add("| Class | Subsystem | Java change | Added methods | Removed methods | C++ files | C++ LOC | Stubs | Status |")
    add("| --- | --- | ---: | --- | --- | --- | ---: | ---: | --- |")
    shared_review = sorted(
        (x for x in shared if not x.is_missing),
        key=lambda x: (x.baseline_similarity if x.baseline_similarity is not None else 1.0, x.name),
    )
    for info in shared_review:
        cpp_files = info.cpp_headers + info.cpp_sources
        add(
            f"| `{info.name}` | {md_escape(info.category)} | {info.change_percent}% | {format_methods(info.added_methods)} | "
            f"{format_methods(info.removed_methods)} | {code_list(cpp_files, 4)} | {info.cpp_lines} | {info.stub_markers} | {status_icon(info.manual_status)} |"
        )
    add("")

    add("## Header/source mismatches")
    add("")
    add("These are not necessarily bugs, but they deserve inspection because the class matcher found only one side of the usual C++ pair.")
    add("")
    add("| Class | State | Files |")
    add("| --- | --- | --- |")
    for info in sorted(header_only + source_only, key=lambda x: x.name):
        add(f"| `{info.name}` | {info.cpp_presence} | {code_list(info.cpp_headers + info.cpp_sources, 8)} |")
    if not header_only and not source_only:
        add("| _None_ | - | - |")
    add("")

    add("## C++ files with incomplete-code markers")
    add("")
    add("This only searches explicit markers such as TODO/FIXME/not implemented/assert(false). It intentionally does not guess that every `return nullptr` is a stub.")
    add("")
    add("| Class | Markers | C++ files | Manual status |")
    add("| --- | ---: | --- | --- |")
    for info in sorted(with_stubs, key=lambda x: (-x.stub_markers, x.name)):
        add(f"| `{info.name}` | {info.stub_markers} | {code_list(info.cpp_headers + info.cpp_sources, 6)} | {status_icon(info.manual_status)} |")
    if not with_stubs:
        add("| _None_ | - | - | - |")
    add("")

    add("## Removed baseline Java classes")
    add("")
    add("These existed in the baseline but not in the target. They may become obsolete, be replaced, or remain as platform compatibility code.")
    add("")
    if removed:
        add(code_list(removed, max(1, len(removed))))
    else:
        add("_None._")
    add("")

    add("## C++ class names not present in target Java")
    add("")
    add("This includes platform backends and Beta++-specific infrastructure, so entries here are **not automatically dead code**.")
    add("")
    if cpp_extra:
        # Use a compact table grouped by inferred category to keep this readable.
        extra_by_category: Dict[str, List[str]] = collections.defaultdict(list)
        for name in cpp_extra:
            sample_path = cpp[name][0].relative_path if cpp.get(name) else ""
            extra_by_category[categorize(name, sample_path)].append(name)
        add("| Subsystem | Count | Classes |")
        add("| --- | ---: | --- |")
        for category in sorted(extra_by_category):
            names = sorted(extra_by_category[category])
            add(f"| {md_escape(category)} | {len(names)} | {code_list(names, 25)} |")
    else:
        add("_None._")
    add("")

    add("## Full target class matrix")
    add("")
    add("This is the canonical generated inventory. Sort/filter it in an editor or consume `UPDATE.json` for tooling.")
    add("")
    add("| Class | Subsystem | Baseline | Java change | Java LOC | C++ | C++ LOC | Deps | Used by | Stubs | Priority | Status |")
    add("| --- | --- | --- | ---: | ---: | --- | ---: | ---: | ---: | ---: | --- | --- |")
    for info in sorted(infos.values(), key=lambda x: (x.category, x.name)):
        change = "new" if info.change_percent is None else f"{info.change_percent}%"
        add(
            f"| `{info.name}` | {md_escape(info.category)} | {info.baseline_state} | {change} | {info.file.lines} | "
            f"{info.cpp_presence} | {info.cpp_lines} | {len(info.dependencies)} | {len(info.dependents)} | {info.stub_markers} | "
            f"{md_escape(info.manual_priority or '-')} | {status_icon(info.manual_status)} |"
        )
    add("")

    add("## Manual status file")
    add("")
    add("Keep human decisions outside this generated report. Example:")
    add("")
    add("```json")
    add("{")
    add('  "classes": {')
    add('    "ExtendedBlockStorage": {')
    add('      "status": "in_progress",')
    add('      "priority": "P0",')
    add('      "notes": "Implement storage and NBT before Chunk migration"')
    add("    },")
    add('    "Packet51MapChunk": {')
    add('      "status": "verify",')
    add('      "priority": "P1",')
    add('      "notes": "Wire format differs from Beta 1.7.3"')
    add("    }")
    add("  }")
    add("}")
    add("```")
    add("")
    add("Supported status values are free-form, but `todo`, `in_progress`, `verify`, `blocked`, and `done` receive normalized labels in the report.")
    add("")

    add("## Reproduction command")
    add("")
    command = [
        "python", "tools/port_tracker.py",
        "--target-java", str(args.target_java),
        "--project", str(args.project),
    ]
    if args.baseline_java:
        command += ["--baseline-java", str(args.baseline_java)]
    command += ["--target-version", args.target_version, "--baseline-version", args.baseline_version, "--output", str(args.output)]
    if args.json_output:
        command += ["--json-output", str(args.json_output)]
    if args.status_file:
        command += ["--status-file", str(args.status_file)]
    add("```text")
    add(" ".join(command))
    add("```")
    add("")
    return "\n".join(lines)


def json_report(
    infos: Mapping[str, JavaClassInfo],
    target: Mapping[str, SourceFile],
    baseline: Mapping[str, SourceFile],
    cpp: Mapping[str, List[SourceFile]],
    args: argparse.Namespace,
) -> Dict[str, object]:
    class_rows = []
    for info in sorted(infos.values(), key=lambda x: x.name):
        class_rows.append(
            {
                "name": info.name,
                "category": info.category,
                "java_file": info.file.relative_path,
                "java_lines": info.file.lines,
                "baseline_state": info.baseline_state,
                "baseline_similarity": info.baseline_similarity,
                "java_change_percent": info.change_percent,
                "added_methods": [{"name": n, "arity": a} for n, a in sorted(info.added_methods)],
                "removed_methods": [{"name": n, "arity": a} for n, a in sorted(info.removed_methods)],
                "cpp_presence": info.cpp_presence,
                "cpp_headers": info.cpp_headers,
                "cpp_sources": info.cpp_sources,
                "cpp_lines": info.cpp_lines,
                "stub_markers": info.stub_markers,
                "dependencies": sorted(info.dependencies),
                "dependents": sorted(info.dependents),
                "blocking_score": blocking_score(info, infos),
                "manual_status": info.manual_status,
                "manual_priority": info.manual_priority,
                "manual_notes": info.manual_notes,
                "platform_risks": sorted(info.risk_tags),
                "suggested_cpp_paths": list(suggested_cpp_paths(info)),
            }
        )
    return {
        "generated_at": dt.datetime.now().astimezone().isoformat(timespec="seconds"),
        "target_version": args.target_version,
        "baseline_version": args.baseline_version,
        "summary": {
            "target_classes": len(target),
            "baseline_classes": len(baseline),
            "cpp_stems": len(cpp),
            "cpp_present": sum(not info.is_missing for info in infos.values()),
            "missing": sum(info.is_missing for info in infos.values()),
            "new_vs_baseline": sum(info.baseline_state == "new" for info in infos.values()),
        },
        "classes": class_rows,
    }


def create_default_status(path: Path) -> None:
    if path.exists():
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    content = {
        "classes": {
            "ExtendedBlockStorage": {
                "status": "todo",
                "priority": "P0",
                "notes": "Core 1.2.5 chunk-section storage.",
            },
            "Chunk": {
                "status": "verify",
                "priority": "P0",
                "notes": "Existing Beta++ class must be migrated to 16 section slots and 256 logical height.",
            },
            "AnvilChunkLoader": {
                "status": "todo",
                "priority": "P0",
                "notes": "Required for .mca / section-based saves.",
            },
            "WorldChunkManager": {
                "status": "todo",
                "priority": "P0",
                "notes": "Replace Beta climate lookup with GenLayer biome pipeline.",
            },
            "ChunkProviderGenerate": {
                "status": "verify",
                "priority": "P0",
                "notes": "Port 1.2.5 generation behavior and preserve Java RNG/overflow semantics.",
            },
            "Packet51MapChunk": {
                "status": "verify",
                "priority": "P1",
                "notes": "1.2.5 uses section bitmasks/add bitmasks and a different payload layout.",
            },
            "Tessellator": {
                "status": "verify",
                "priority": "P1",
                "notes": "Add 1.2.5 brightness/lightmap data without regressing Wii/PS2 backends.",
            },
        }
    }
    path.write_text(json.dumps(content, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate a comprehensive Java-to-C++ port progress report.")
    parser.add_argument("--target-java", type=Path, required=True, help="Target Java source directory or ZIP.")
    parser.add_argument("--project", type=Path, default=Path("."), help="Beta++ project root containing src/.")
    parser.add_argument("--baseline-java", type=Path, default=None, help="Baseline Java source directory or ZIP.")
    parser.add_argument("--target-version", default="target", help="Human-readable target version label.")
    parser.add_argument("--baseline-version", default="baseline", help="Human-readable baseline version label.")
    parser.add_argument("--output", type=Path, default=Path("UPDATE.md"), help="Markdown output path.")
    parser.add_argument("--json-output", type=Path, default=None, help="Optional machine-readable JSON output path.")
    parser.add_argument("--status-file", type=Path, default=None, help="Optional manual status JSON file.")
    parser.add_argument("--create-status", action="store_true", help="Create a starter status JSON if it does not exist.")
    parser.add_argument("--top-blockers", type=int, default=40, help="Number of blocker candidates in the summary.")
    parser.add_argument("--top-changed", type=int, default=50, help="Number of largest shared-class changes in the summary.")
    parser.add_argument("--show-unchanged-packets", action="store_true", help="Include unchanged packet rows in protocol table.")
    parser.add_argument("--report-title", default="Minecraft 1.2.5 Port Update", help="Markdown report title.")
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    args.project = args.project.resolve()
    args.target_java = args.target_java.resolve()
    if args.baseline_java is None:
        candidate = args.project / "srcorigjava"
        if candidate.exists():
            args.baseline_java = candidate
    args.baseline_java = args.baseline_java.resolve() if args.baseline_java else None
    if args.status_file is None:
        candidate = args.project / "tools" / "port_tracker_status.json"
        if candidate.exists() or args.create_status:
            args.status_file = candidate
    args.output = args.output.resolve()
    args.json_output = args.json_output.resolve() if args.json_output else None
    args.status_file = args.status_file.resolve() if args.status_file else None
    if args.status_file:
        try:
            args.status_display = args.status_file.relative_to(args.project).as_posix()
        except ValueError:
            args.status_display = str(args.status_file)
    else:
        args.status_display = "tools/port_tracker_status.json"

    if args.create_status and args.status_file:
        create_default_status(args.status_file)

    target = load_java_tree(args.target_java)
    if not target:
        raise RuntimeError("No Java source files were found in --target-java.")

    baseline: Dict[str, SourceFile] = {}
    if args.baseline_java:
        baseline = load_java_tree(args.baseline_java)

    cpp = index_cpp_tree(args.project)
    manual = load_manual_status(args.status_file)
    infos = build_infos(target, baseline, cpp, manual)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    report = build_report(infos, target, baseline, cpp, args)
    args.output.write_text(report + "\n", encoding="utf-8")

    if args.json_output:
        args.json_output.parent.mkdir(parents=True, exist_ok=True)
        args.json_output.write_text(
            json.dumps(json_report(infos, target, baseline, cpp, args), indent=2, ensure_ascii=False) + "\n",
            encoding="utf-8",
        )

    print(f"Target Java classes: {len(target)}")
    print(f"C++ class names present: {sum(not info.is_missing for info in infos.values())}")
    print(f"Missing target classes: {sum(info.is_missing for info in infos.values())}")
    print(f"Wrote: {args.output}")
    if args.json_output:
        print(f"Wrote: {args.json_output}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except KeyboardInterrupt:
        raise SystemExit(130)
    except Exception as exc:
        print(f"port_tracker.py: error: {exc}", file=sys.stderr)
        raise SystemExit(1)
