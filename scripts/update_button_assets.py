#!/usr/bin/env python3
"""
Generates the PS2 controller button texture atlas (buttons_ps2.png) from source icons,
updates assets.pak by injecting assets/gui/buttons_ps2.png, and generates Ps2ButtonAtlasData.h.
"""

import os
import sys
import struct
import shutil
from PIL import Image

def unpack_pak(pak, out_dir):
    with open(pak, 'rb') as f:
        magic, ver, count, tbl_off, names_off, names_len, align, _ = struct.unpack('>4sIIIIIII', f.read(32))
        assert magic == b'MCPK'
        f.seek(tbl_off)
        entries = [struct.unpack('>IIII', f.read(16)) for _ in range(count)]
        f.seek(names_off)
        names_block = f.read(names_len)
        for h, n_off, d_off, sz in entries:
            name = names_block[n_off:].split(b'\0')[0].decode('utf-8')
            dest = os.path.join(out_dir, name.replace('/', os.sep))
            os.makedirs(os.path.dirname(dest), exist_ok=True)
            f.seek(d_off)
            with open(dest, 'wb') as out_f:
                out_f.write(f.read(sz))

def generate_atlas(src_dir, out_png):
    button_map = [
        ('cross', 'Button - PS Cross 2.png'),
        ('circle', 'Button - PS Circle 2.png'),
        ('square', 'Button - PS Square 2.png'),
        ('triangle', 'Button - PS Triangle 2.png'),
        ('dpad', 'Button - PS Directional Arrows.png'),
        ('l1', [f for f in os.listdir(src_dir) if 'L1' in f][0]),
        ('r1', [f for f in os.listdir(src_dir) if 'R1' in f][0]),
        ('l2', [f for f in os.listdir(src_dir) if 'L2' in f][0]),
        ('r2', [f for f in os.listdir(src_dir) if 'R2' in f][0]),
        ('l3', 'Button - PS Analogue L.png'),
        ('r3', 'Button - PS Analogue R.png'),
        ('select', 'ButtonIcon-PS3-Select.png'),
        ('start', 'ButtonIcon-PS3-Start.png'),
    ]

    atlas_size = 256
    cell_size = 64
    grid_cols = 4

    atlas = Image.new('RGBA', (atlas_size, atlas_size), (0, 0, 0, 0))

    for idx, (bname, fname) in enumerate(button_map):
        im = Image.open(os.path.join(src_dir, fname))
        bbox = im.getbbox()
        cropped = im.crop(bbox)
        
        col = idx % grid_cols
        row = idx // grid_cols
        
        cell_x = col * cell_size
        cell_y = row * cell_size
        
        max_dim = cell_size - 4
        scale = min(max_dim / cropped.width, max_dim / cropped.height)
        new_w = int(round(cropped.width * scale))
        new_h = int(round(cropped.height * scale))
        
        resized = cropped.resize((new_w, new_h), Image.Resampling.LANCZOS)
        
        offset_x = cell_x + (cell_size - new_w) // 2
        offset_y = cell_y + (cell_size - new_h) // 2
        
        atlas.paste(resized, (offset_x, offset_y))

    atlas.save(out_png, optimize=True)
    print(f'[OK] Generated button atlas: {out_png} ({os.path.getsize(out_png)} bytes)')

def main():
    repo_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    src_dir = r'C:\Users\user\Downloads\botones'
    pak_path = os.path.join(repo_dir, 'assets.pak')
    btn_png = os.path.join(repo_dir, 'buttons_ps2.png')
    
    if not os.path.isdir(src_dir):
        print(f'[!] Source button directory not found: {src_dir}')
        return 1

    generate_atlas(src_dir, btn_png)
    
    # Also update Ps2ButtonAtlasData.h
    out_header = os.path.join(repo_dir, 'src', 'net', 'minecraft', 'src', 'legacy', 'Ps2ButtonAtlasData.h')
    with open(btn_png, 'rb') as f:
        data = f.read()

    lines = [
        '// Auto-generated fallback data for PS2 button atlas',
        '#pragma once',
        '#include <cstddef>',
        '',
        f'constexpr size_t PS2_BUTTON_ATLAS_PNG_SIZE = {len(data)};',
        'inline const unsigned char s_ps2ButtonAtlasPngData[] = {'
    ]
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        hex_str = ', '.join(f'0x{b:02x}' for b in chunk)
        lines.append(f'    {hex_str},')
    lines.append('};')
    lines.append('')
    with open(out_header, 'w') as f:
        f.write('\n'.join(lines))
    print(f'[OK] Generated {out_header}')

    # Inject into assets.pak
    if os.path.isfile(pak_path):
        staged_dir = os.path.join(repo_dir, 'staged_data_btn')
        if os.path.exists(staged_dir):
            shutil.rmtree(staged_dir)
        os.makedirs(staged_dir, exist_ok=True)

        unpack_pak(pak_path, staged_dir)
        gui_dir = os.path.join(staged_dir, 'assets', 'gui')
        os.makedirs(gui_dir, exist_ok=True)
        shutil.copy2(btn_png, os.path.join(gui_dir, 'buttons_ps2.png'))

        sys.path.insert(0, os.path.join(repo_dir, 'scripts'))
        import make_pak
        count, total = make_pak.build(staged_dir, pak_path)
        shutil.rmtree(staged_dir)
        print(f'[OK] Injected into assets.pak: {count} entries, {total / (1024*1024):.2f} MB')

if __name__ == '__main__':
    main()
