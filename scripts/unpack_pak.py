#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
OptiCraft Heritage Edition - Extractor de assets.pak
Extrae todos los recursos (texturas, sonidos, GUI, modelos, etc.) contenidos en assets.pak.
"""

import os
import sys
import struct

def unpack(pak_file, out_dir):
    if not os.path.isfile(pak_file):
        print(f"[ERROR] No se encontro el archivo: {pak_file}")
        return False

    with open(pak_file, "rb") as f:
        header = f.read(32)
        if len(header) < 32:
            print("[ERROR] Archivo demasiado corto.")
            return False

        magic, version, count, table_off, names_off, names_bytes, data_align, _ = struct.unpack(">4sIIIIIII", header)
        if magic != b"MCPK":
            print(f"[ERROR] Formato no reconocido (Magic: {magic})")
            return False

        print(f"[*] Abriendo {pak_file}")
        print(f"[*] Formato MCPK v{version} - {count} archivos contenidos.")
        print(f"[*] Extrayendo hacia: {out_dir} ...\n")

        f.seek(table_off)
        entries = [struct.unpack(">IIII", f.read(16)) for _ in range(count)]

        f.seek(names_off)
        names_blob = f.read(names_bytes)

        for i, (hash_val, name_offset, data_offset, size) in enumerate(entries, 1):
            null_pos = names_blob.find(b"\x00", name_offset)
            rel_path = names_blob[name_offset:null_pos].decode("utf-8")
            dest = os.path.join(out_dir, rel_path.replace("/", os.sep))
            os.makedirs(os.path.dirname(dest), exist_ok=True)

            f.seek(data_offset)
            content = f.read(size)
            with open(dest, "wb") as out:
                out.write(content)

            if i % 50 == 0 or i == count:
                print(f"  [{i}/{count}] Extrayendo: {rel_path} ({size} bytes)")

    print(f"\n[OK] Se extrajeron exitosamente {count} archivos en '{out_dir}'.")
    return True

def main():
    repo_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    default_pak = os.path.join(r"C:\Users\user\Downloads\OptiCraft Heritage Edition PS2 V1.1\Version ELF\OptiCraftHeritage", "assets.pak")
    
    if len(sys.argv) > 1:
        pak_path = sys.argv[1]
    elif os.path.isfile(os.path.join(repo_dir, "assets.pak")):
        pak_path = os.path.join(repo_dir, "assets.pak")
    else:
        pak_path = default_pak

    if len(sys.argv) > 2:
        dest_dir = sys.argv[2]
    else:
        dest_dir = os.path.join(repo_dir, "extracted_assets")

    unpack(pak_path, dest_dir)

if __name__ == "__main__":
    main()
