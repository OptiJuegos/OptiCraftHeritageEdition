#!/usr/bin/env python3
"""
Unpacks assets.pak, adds the legacy skins and previews from Desktop/skins,
generates 64x32 versions for ModelBiped compatibility, extracts pack cover art,
and repacks assets.pak using scripts/make_pak.py.
"""

import os
import sys
import struct
import shutil
from PIL import Image

def unpack_pak(pak_path, output_dir):
    print(f"[*] Unpacking {pak_path} -> {output_dir}...")
    with open(pak_path, "rb") as f:
        magic, ver, count, tbl_off, names_off, names_len, align, _ = struct.unpack(">4sIIIIIII", f.read(32))
        assert magic == b"MCPK", f"Invalid magic: {magic}"
        f.seek(tbl_off)
        entries = [struct.unpack(">IIII", f.read(16)) for _ in range(count)]
        f.seek(names_off)
        names_block = f.read(names_len)
        
        for h, n_off, d_off, sz in entries:
            name_bytes = names_block[n_off:].split(b"\0")[0]
            name = name_bytes.decode("utf-8")
            dest = os.path.join(output_dir, name.replace("/", os.sep))
            os.makedirs(os.path.dirname(dest), exist_ok=True)
            f.seek(d_off)
            data = f.read(sz)
            with open(dest, "wb") as out_file:
                out_file.write(data)
    print(f"[OK] Unpacked {count} files.")

def process_skins(src_dir, image_ref_path, staged_assets_dir):
    skins_out = os.path.join(staged_assets_dir, "assets", "skins")
    os.makedirs(skins_out, exist_ok=True)
    
    # 1. Extract pack cover art from image.png (crop 273, 93 to 600, 420)
    print("[*] Extracting pack cover from reference image...")
    ref_im = Image.open(image_ref_path)
    cover_crop = ref_im.crop((273, 93, 600, 420))
    cover_256 = cover_crop.resize((256, 256), Image.Resampling.LANCZOS)
    cover_path = os.path.join(skins_out, "default_pack.png")
    cover_256.save(cover_path, "PNG")
    print(f"[OK] Saved {cover_path} (256x256)")

    # 2. Process all skins and front previews
    files = os.listdir(src_dir)
    skin_files = [f for f in files if f.endswith(".png") and not f.endswith("_Front.png")]
    print(f"[*] Processing {len(skin_files)} skins from {src_dir}...")
    
    count = 0
    for sf in skin_files:
        base_name = sf[:-4]
        # Normalize filename (e.g. Scottish Steve -> Scottish_Steve)
        safe_name = base_name.replace(" ", "_")
        
        src_skin_path = os.path.join(src_dir, sf)
        src_front_path = os.path.join(src_dir, f"{base_name}_Front.png")
        
        if not os.path.isfile(src_front_path):
            print(f"[!] Warning: Front preview missing for {sf}")
            continue
            
        skin_img = Image.open(src_skin_path).convert("RGBA")
        front_img = Image.open(src_front_path).convert("RGBA")
        
        # Save 64x64 skin
        skin_out_path = os.path.join(skins_out, f"{safe_name}.png")
        skin_img.save(skin_out_path, "PNG")
        
        # Also generate 64x32 skin for ModelBiped (top 32 rows)
        skin_32 = skin_img.crop((0, 0, 64, 32))
        skin_32_out_path = os.path.join(skins_out, f"{safe_name}_32.png")
        skin_32.save(skin_32_out_path, "PNG")
        
        # Save 16x32 Front preview
        front_out_path = os.path.join(skins_out, f"{safe_name}_Front.png")
        front_img.save(front_out_path, "PNG")
        
        # If safe_name differs from base_name, also keep original filename variant
        if safe_name != base_name:
            orig_skin = os.path.join(skins_out, f"{base_name}.png")
            orig_skin_32 = os.path.join(skins_out, f"{base_name}_32.png")
            orig_front = os.path.join(skins_out, f"{base_name}_Front.png")
            skin_img.save(orig_skin, "PNG")
            skin_32.save(orig_skin_32, "PNG")
            front_img.save(orig_front, "PNG")
            
        count += 1
        print(f"  + Added skin: {safe_name} (64x64, 64x32, 16x32 front)")
        
    print(f"[OK] Processed {count} skins.")

def main():
    repo_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    pak_path = os.path.join(repo_dir, "assets.pak")
    staged_dir = os.path.join(repo_dir, "staged_data")
    desktop_skins_dir = r"C:\Users\user\Desktop\skins\Legacy_Skins_Revived\Skin Packs"
    desktop_image_ref = r"C:\Users\user\Desktop\skins\image.png"
    
    if os.path.exists(staged_dir):
        shutil.rmtree(staged_dir)
    os.makedirs(staged_dir, exist_ok=True)
    
    unpack_pak(pak_path, staged_dir)
    process_skins(desktop_skins_dir, desktop_image_ref, staged_dir)
    
    # Also copy the skins folder to repo's assets/skins for loose asset loading
    repo_skins = os.path.join(repo_dir, "assets", "skins")
    if os.path.exists(repo_skins):
        shutil.rmtree(repo_skins)
    shutil.copytree(os.path.join(staged_dir, "assets", "skins"), repo_skins)
    print(f"[OK] Copied skins to workspace: {repo_skins}")
    
    # Repack assets.pak
    make_pak_script = os.path.join(repo_dir, "scripts", "make_pak.py")
    import importlib.util
    spec = importlib.util.spec_from_file_location("make_pak", make_pak_script)
    make_pak = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(make_pak)
    
    print(f"[*] Repacking {pak_path} from {staged_dir}...")
    # Backup original pak if not already backed up
    pak_backup = os.path.join(repo_dir, "assets.pak.orig")
    if not os.path.isfile(pak_backup):
        shutil.copy2(pak_path, pak_backup)
        print(f"[OK] Backed up original assets.pak -> {pak_backup}")
        
    num_entries, total_size = make_pak.build(staged_dir, pak_path)
    print(f"[OK] Repacked assets.pak successfully: {num_entries} entries, {total_size / (1024*1024):.2f} MB")

if __name__ == "__main__":
    main()
