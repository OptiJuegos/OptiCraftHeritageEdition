#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
OptiCraft Heritage Edition - Ensamblador de Release para PlayStation 2
Genera el paquete completo de distribucion con fecha/hora HHMM identico a la version oficial:
- !!INFO!!.txt
- Cover/ (Cover.png, !!CREDITOS!!.txt)
- Version ELF/ (OptiCraftHeritage con OptiCraft.elf y assets.pak, WLE-R3Z.ELF, !!COPY!!.txt)
- Version ISO/ (OPL VERSION con OPL 0.9.2.ELF y !DATA!!.txt, y la ISO SLUS_420.69...iso)
"""

import sys
import os
import io
import shutil
import subprocess
from datetime import datetime

def banner(title):
    print("=" * 79)
    print(f" {title}")
    print("=" * 79, flush=True)

def step(num, total, text):
    print("\n" + "-" * 79)
    print(f"[{num}/{total}] {text}")
    print("-" * 79, flush=True)

def log(text):
    print(f" [*] {text}", flush=True)

def ok(text):
    print(f" [OK] {text}", flush=True)

def warn(text):
    print(f" [!] {text}", flush=True)

def error(text):
    print(f"[ERROR] {text}", file=sys.stderr, flush=True)

def ensure_pycdlib():
    try:
        import pycdlib
        return pycdlib
    except ImportError:
        log("Instalando dependencia 'pycdlib' via pip...")
        res = subprocess.run([sys.executable, "-m", "pip", "install", "pycdlib"], capture_output=True, text=True)
        if res.returncode != 0:
            error("No se pudo instalar pycdlib automaticamente.")
            error(res.stderr)
            sys.exit(1)
        import pycdlib
        return pycdlib

def main():
    repo_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    downloads_dir = r"C:\Users\user\Downloads"
    ref_release = os.path.join(downloads_dir, "OptiCraft Heritage Edition PS2 V1.1")
    
    banner("OPTICRAFT HERITAGE - ENSAMBLADOR DE DISTRIBUCION PARA PS2")
    
    # -------------------------------------------------------------------------
    # PASO 1: Compilar o localizar OptiCraft.elf
    # -------------------------------------------------------------------------
    step(1, 5, "Compilando o localizando el ejecutable OptiCraft.elf...")
    elf_path = None
    
    # 1. Si se especificó por argumento de línea de comandos
    if len(sys.argv) > 1 and os.path.isfile(sys.argv[1]):
        elf_path = os.path.abspath(sys.argv[1])
        ok(f"Usando ejecutable especificado por argumento: {elf_path}")

    # 2. Si ya existe un ejecutable recientemente compilado en el repositorio
    if not elf_path:
        for candidate in [
            os.path.join(repo_dir, "bin", "ps2", "usb", "MCBETA", "OptiCraft.elf"),
            os.path.join(repo_dir, "build", "ps2-release", "OptiCraft.elf"),
            os.path.join(repo_dir, "OptiCraft.elf")
        ]:
            if os.path.isfile(candidate):
                elf_path = candidate
                ok(f"Ejecutable compilado detectado: {elf_path}")
                break

    # 3. Si no existe ejecutable, intentar compilar directamente con CMake
    if not elf_path:
        cmake_exe = r"C:\Program Files\CMake\bin\cmake.exe"
        if os.path.isfile(cmake_exe):
            log("Compilando OptiCraft.elf con CMake preset ps2-release...")
            try:
                p_cfg = subprocess.run([
                    cmake_exe, "--preset", "ps2-release",
                    "-DPS2_ENABLE_SOUND=OFF", "-DPS2_ENABLE_PERSPECTIVE_TEXTURES=ON",
                    "-DPS2_RENDER_STATS=OFF", "-DPS2_ENABLE_VU1_TERRAIN=ON", "-DMC_LOG_LEVEL=0"
                ], cwd=repo_dir)
                if p_cfg.returncode == 0:
                    subprocess.run([cmake_exe, "--build", "--preset", "ps2-release", "--parallel"], cwd=repo_dir)
            except Exception as e:
                warn(f"Fallo durante la invocacion de CMake: {e}")

            for candidate in [
                os.path.join(repo_dir, "bin", "ps2", "usb", "MCBETA", "OptiCraft.elf"),
                os.path.join(repo_dir, "build", "ps2-release", "OptiCraft.elf")
            ]:
                if os.path.isfile(candidate):
                    elf_path = candidate
                    ok(f"Compilacion finalizada exitosamente: {elf_path}")
                    break
                
    # Si no se pudo compilar, buscar en la versión de referencia
    if not elf_path:
        candidate_ref = os.path.join(ref_release, "Version ELF", "OptiCraftHeritage", "OptiCraft.elf")
        if os.path.isfile(candidate_ref):
            elf_path = candidate_ref
            ok(f"Utilizando ejecutable de referencia existente: {elf_path}")
            
    if not elf_path or not os.path.isfile(elf_path):
        error("No se encontro 'OptiCraft.elf'.")
        sys.exit(1)
        
    size_mb = os.path.getsize(elf_path) / (1024 * 1024)
    log(f"Archivo ELF validado: {os.path.basename(elf_path)} ({size_mb:.2f} MB)")

    # -------------------------------------------------------------------------
    # PASO 2: Localizar assets.pak
    # -------------------------------------------------------------------------
    step(2, 5, "Localizando el contenedor de recursos (assets.pak)...")
    pak_path = None
    if len(sys.argv) > 2 and os.path.isfile(sys.argv[2]):
        pak_path = os.path.abspath(sys.argv[2])
        ok(f"Usando contenedor especificado por argumento: {pak_path}")

    if not pak_path:
        for cand in [
            os.path.join(repo_dir, "assets.pak"),
            os.path.join(repo_dir, "bin", "ps2", "usb", "OptiCraftHeritage", "assets.pak"),
            os.path.join(repo_dir, "data", "assets.pak"),
            os.path.join(ref_release, "Version ELF", "OptiCraftHeritage", "assets.pak"),
            os.path.join(ref_release, "Version ISO", "SLUS_420.69.OptiCraft_Heritage_Edition_OPTIJUEGOS", "ASSETS.PAK")
        ]:
            if os.path.isfile(cand):
                pak_path = cand
                break
            
    if not pak_path:
        error("No se encontro 'assets.pak'.")
        sys.exit(1)
        
    pak_size_mb = os.path.getsize(pak_path) / (1024 * 1024)
    ok(f"Contenedor localizado: {pak_path} ({pak_size_mb:.2f} MB)")

    # -------------------------------------------------------------------------
    # PASO 3: Crear carpeta de distribución con hora HHMM
    # -------------------------------------------------------------------------
    step(3, 5, "Creando carpeta de distribucion con indicador horario (HHMM)...")
    now = datetime.now()
    hhmm = now.strftime("%H%M")
    dist_name = f"OptiCraft Heritage Edition PS2 {hhmm}"
    dist_dir = os.path.join(downloads_dir, dist_name)
    
    log(f"Hora actual: {now.strftime('%H:%M:%S')} -> Etiqueta: {hhmm}")
    log(f"Carpeta de destino: {dist_dir}")
    
    cover_dir = os.path.join(dist_dir, "Cover")
    version_elf_dir = os.path.join(dist_dir, "Version ELF")
    opti_heritage_dir = os.path.join(version_elf_dir, "OptiCraftHeritage")
    version_iso_dir = os.path.join(dist_dir, "Version ISO")
    opl_dir = os.path.join(version_iso_dir, "OPL VERSION")
    
    for d in [dist_dir, cover_dir, version_elf_dir, opti_heritage_dir, version_iso_dir, opl_dir]:
        os.makedirs(d, exist_ok=True)
    ok("Arbol de carpetas creado correctamente.")

    # -------------------------------------------------------------------------
    # PASO 4: Ensamblar Version ELF y extras de distribución
    # -------------------------------------------------------------------------
    step(4, 5, "Ensamblando Version ELF y componentes adicionales...")
    
    # 4.1 OptiCraft.elf y assets.pak en OptiCraftHeritage
    shutil.copy2(elf_path, os.path.join(opti_heritage_dir, "OptiCraft.elf"))
    log("Copiado OptiCraft.elf -> Version ELF/OptiCraftHeritage/OptiCraft.elf")
    
    shutil.copy2(pak_path, os.path.join(opti_heritage_dir, "assets.pak"))
    log("Copiado assets.pak -> Version ELF/OptiCraftHeritage/assets.pak")

    # 4.1.1 Copiar modulos IRX (audio, red, etc.) a Version ELF
    irx_src_dir = None
    for cand_irx in [
        os.path.join(repo_dir, "bin", "ps2", "usb", "MCBETA", "data", "irx"),
        os.path.join(r"C:\Users\user\OptiCraftHeritageEdition-PR", "bin", "ps2", "usb", "MCBETA", "data", "irx")
    ]:
        if os.path.isdir(cand_irx):
            irx_src_dir = cand_irx
            break

    if irx_src_dir:
        dest_irx_dir = os.path.join(opti_heritage_dir, "data", "irx")
        os.makedirs(dest_irx_dir, exist_ok=True)
        for irx_file in os.listdir(irx_src_dir):
            if irx_file.endswith(".irx"):
                shutil.copy2(os.path.join(irx_src_dir, irx_file), os.path.join(dest_irx_dir, irx_file))
        log(f"Copiados modulos IRX de {irx_src_dir} -> Version ELF/OptiCraftHeritage/data/irx/")
    
    # 4.1.2 Copiar mods (.ochpack) a Version ELF/OptiCraftHeritage/mods/
    mods_src_dir = None
    for cand_mods in [
        r"C:\Users\user\Downloads\mods para base ochpack",
        os.path.join(repo_dir, "bin", "ps2", "usb", "MCBETA", "mods"),
        os.path.join(r"C:\Users\user\OptiCraftHeritageEdition-PR", "bin", "ps2", "usb", "MCBETA", "mods")
    ]:
        if os.path.isdir(cand_mods):
            mods_src_dir = cand_mods
            break

    if mods_src_dir:
        dest_mods_dir = os.path.join(opti_heritage_dir, "mods")
        os.makedirs(dest_mods_dir, exist_ok=True)
        pack_list = []
        for mod_file in os.listdir(mods_src_dir):
            if mod_file.lower().endswith(".ochpack"):
                # Solamente incluir mod de prueba (sampletest) ya que los demas estan en el codigo fuente
                if "sample" not in mod_file.lower() and "test" not in mod_file.lower():
                    continue
                shutil.copy2(os.path.join(mods_src_dir, mod_file), os.path.join(dest_mods_dir, mod_file))
                pack_list.append(mod_file)
        with open(os.path.join(dest_mods_dir, "packlist.txt"), "w", encoding="utf-8") as f:
            for p in pack_list:
                f.write(p + "\n")
        log(f"Copiados {len(pack_list)} paquetes .ochpack y generado packlist.txt en -> Version ELF/OptiCraftHeritage/mods/")
    
    # 4.2 Archivos auxiliares de Version ELF
    copy_txt = os.path.join(ref_release, "Version ELF", "!!COPY!!.txt")
    if os.path.isfile(copy_txt):
        shutil.copy2(copy_txt, os.path.join(version_elf_dir, "!!COPY!!.txt"))
        
    wle_elf = os.path.join(ref_release, "Version ELF", "WLE-R3Z.ELF")
    if os.path.isfile(wle_elf):
        shutil.copy2(wle_elf, os.path.join(version_elf_dir, "WLE-R3Z.ELF"))
        log("Copiado uLaunchELF (WLE-R3Z.ELF) -> Version ELF/")
        
    # 4.3 !!INFO!!.txt en raíz
    info_txt = os.path.join(ref_release, "!!INFO!!.txt")
    if os.path.isfile(info_txt):
        shutil.copy2(info_txt, os.path.join(dist_dir, "!!INFO!!.txt"))
        log("Copiado !!INFO!!.txt -> raiz/")
        
    # 4.4 Cover
    ref_cover = os.path.join(ref_release, "Cover")
    if os.path.isdir(ref_cover):
        for f in os.listdir(ref_cover):
            src = os.path.join(ref_cover, f)
            if os.path.isfile(src):
                shutil.copy2(src, os.path.join(cover_dir, f))
        log("Copiado Cover/ (Cover.png y creditos)")
        
    # 4.5 OPL VERSION
    ref_opl = os.path.join(ref_release, "Version ISO", "OPL VERSION")
    if os.path.isdir(ref_opl):
        for f in os.listdir(ref_opl):
            src = os.path.join(ref_opl, f)
            if os.path.isfile(src):
                shutil.copy2(src, os.path.join(opl_dir, f))
        log("Copiado OPL VERSION/ (OPL 0.9.2.ELF y datos)")
        
    ok("Archivos de Version ELF y extras copiados con exito.")

    # -------------------------------------------------------------------------
    # PASO 5: Ensamblar dependencias y generar imagen ISO
    # -------------------------------------------------------------------------
    step(5, 5, "Construyendo imagen ISO bootable para PlayStation 2...")
    pycdlib = ensure_pycdlib()
    
    stage_dir = os.path.join(dist_dir, "_temp_iso_stage")
    os.makedirs(stage_dir, exist_ok=True)
    
    # Copiar archivos para la ISO:
    # 1. OptiCraft.elf renombrado a SLUS_420.69
    shutil.copy2(elf_path, os.path.join(stage_dir, "SLUS_420.69"))
    # 2. assets.pak como ASSETS.PAK
    shutil.copy2(pak_path, os.path.join(stage_dir, "ASSETS.PAK"))
    # 3. SYSTEM.CNF
    with open(os.path.join(stage_dir, "SYSTEM.CNF"), "w", encoding="ascii") as f:
        f.write("BOOT2 = cdrom0:\\SLUS_420.69;1\r\nVER = 1.00\r\nVMODE = NTSC\r\n")
        
    iso_name = "SLUS_420.69.OptiCraft_Heritage_Edition_OPTIJUEGOS.iso"
    final_iso = os.path.join(version_iso_dir, iso_name)
    
    log(f"Empaquetando ISO en formato ISO-9660 PS2...")
    iso = pycdlib.PyCdlib()
    iso.new(
        interchange_level=2,
        sys_ident="Win32",
        vol_ident="OPTICRAFT PS2",
        pub_ident_str="OPTIJUEGOS",
        app_ident_str="MKISOFS ISO 9660/HFS FILESYSTEM BUILDER"
    )
    
    for f in sorted(os.listdir(stage_dir)):
        fpath = os.path.join(stage_dir, f)
        iso_item = f.upper()
        if ";" not in iso_item:
            iso_item += ";1"
        size = os.path.getsize(fpath) / (1024 * 1024)
        log(f"  + Sectorizado: {f} -> /{iso_item} ({size:.2f} MB)")
        iso.add_file(fpath, iso_path="/" + iso_item)

    # Agregar modulos IRX al disco ISO si existen
    if irx_src_dir and os.path.isdir(irx_src_dir):
        try:
            iso.add_directory("/DATA")
            iso.add_directory("/DATA/IRX")
            for irx_file in sorted(os.listdir(irx_src_dir)):
                if irx_file.endswith(".irx"):
                    src_f = os.path.join(irx_src_dir, irx_file)
                    item_name = irx_file.upper()
                    if ";" not in item_name:
                        item_name += ";1"
                    log(f"  + Sectorizado IRX: {irx_file} -> /DATA/IRX/{item_name}")
                    iso.add_file(src_f, iso_path="/DATA/IRX/" + item_name)
        except Exception as e:
            warn(f"No se pudieron sectorizar los IRX en la ISO: {e}")

    # Agregar mods (.ochpack) y PACKLIST.TXT al disco ISO si existen
    if mods_src_dir and os.path.isdir(mods_src_dir):
        try:
            iso.add_directory("/MODS")
            pack_list = []
            for mod_file in sorted(os.listdir(mods_src_dir)):
                if mod_file.lower().endswith(".ochpack"):
                    # Solamente incluir mod de prueba (sampletest) ya que los demas estan en el codigo fuente
                    if "sample" not in mod_file.lower() and "test" not in mod_file.lower():
                        continue
                    src_f = os.path.join(mods_src_dir, mod_file)
                    item_name = mod_file.upper()
                    if ";" not in item_name:
                        item_name += ";1"
                    size_kb = os.path.getsize(src_f) / 1024
                    log(f"  + Sectorizado MOD: {mod_file} -> /MODS/{item_name} ({size_kb:.1f} KB)")
                    iso.add_file(src_f, iso_path="/MODS/" + item_name)
                    pack_list.append(mod_file)
                    if mod_file.upper() not in pack_list:
                        pack_list.append(mod_file.upper())

            # PACKLIST.TXT en /MODS
            packlist_bytes = "\r\n".join(pack_list).encode("ascii")
            iso.add_fp(io.BytesIO(packlist_bytes), len(packlist_bytes), iso_path="/MODS/PACKLIST.TXT;1")
            log(f"  + Sectorizado PACKLIST.TXT -> /MODS/PACKLIST.TXT;1 ({len(pack_list)} entradas)")
        except Exception as e:
            warn(f"No se pudieron sectorizar los MODS en la ISO: {e}")
        
    log(f"Escribiendo imagen ISO final en: {final_iso} ...")
    iso.write(final_iso)
    iso.close()
    
    # Limpiar stage
    shutil.rmtree(stage_dir, ignore_errors=True)
    
    iso_size = os.path.getsize(final_iso) / (1024 * 1024)
    ok(f"ISO generada correctamente ({iso_size:.2f} MB)")

    # -------------------------------------------------------------------------
    # RESUMEN FINAL
    # -------------------------------------------------------------------------
    print("\n" + "=" * 79)
    print("        [EXITO] PAQUETE PS2 ENSAMBLADO COMPLETAMENTE")
    print("=" * 79)
    print(f" Ubicacion: {dist_dir}\n")
    print(" Estructura final del paquete:")
    print(f" {dist_name}/")
    print("  |-- !!INFO!!.txt")
    print("  |-- Cover/")
    print("  |   |-- !!CREDITOS!!.txt")
    print("  |   +-- Cover.png")
    print("  |-- Version ELF/")
    print("  |   |-- !!COPY!!.txt")
    print("  |   |-- WLE-R3Z.ELF")
    print("  |   +-- OptiCraftHeritage/")
    print(f"  |       |-- OptiCraft.elf  ({size_mb:.2f} MB)")
    print(f"  |       +-- assets.pak     ({pak_size_mb:.2f} MB)")
    print("  +-- Version ISO/")
    print("      |-- OPL VERSION/")
    print("      |   |-- !DATA!!.txt")
    print("      |   +-- OPL 0.9.2.ELF")
    print(f"      +-- {iso_name} ({iso_size:.2f} MB)")
    print("=" * 79 + "\n")

if __name__ == "__main__":
    main()
