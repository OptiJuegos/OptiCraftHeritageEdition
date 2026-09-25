#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
OptiCraft Heritage Edition - Herramienta Grafica de Assets.pak
Permite empaquetar una carpeta de recursos a formato assets.pak (MCPK)
y desempaquetar archivos .pak a una carpeta destino.
"""

import os
import sys
import struct
import threading
import tkinter as tk
from tkinter import ttk, filedialog, messagebox

MAGIC = b"MCPK"
VERSION = 1
HEADER_BYTES = 32
ENTRY_BYTES = 16
DATA_ALIGN = 64

EXCLUDED_PREFIXES = (
    "assets/legacy/tutorial/session.lock",
    "assets/legacy/tutorial/level.dat_old",
    "assets/legacy/tutorial/level.dat_mcr",
)

def fnv1a32(text):
    value = 0x811C9DC5
    for byte in text.encode("utf-8"):
        value ^= byte
        value = (value * 0x01000193) & 0xFFFFFFFF
    return value

def align(value, alignment):
    return (value + alignment - 1) // alignment * alignment

def collect_files(root):
    entries = []
    seen = {}
    for directory, _, names in os.walk(root):
        for name in names:
            full = os.path.join(directory, name)
            key = os.path.relpath(full, root).replace(os.sep, "/")
            if key.lower().startswith(EXCLUDED_PREFIXES):
                continue
            lowered = key.lower()
            if lowered in seen:
                raise ValueError(f"Colision de nombres: '{seen[lowered]}' y '{key}' tendrian la misma clave.")
            seen[lowered] = key
            entries.append((key, full, os.path.getsize(full)))
    return entries

def pack_folder(root_dir, output_file, progress_cb=None, log_cb=None):
    if log_cb:
        log_cb(f"Analizando carpeta: {root_dir} ...")
    files = collect_files(root_dir)
    if not files:
        raise ValueError(f"No se encontraron archivos en la carpeta: {root_dir}")

    total_files = len(files)
    if log_cb:
        log_cb(f"Archivos encontrados: {total_files}. Preparando tabla de indices...")

    names = bytearray()
    records = []
    for key, full, size in files:
        name_offset = len(names)
        names += key.encode("utf-8") + b"\0"
        records.append([fnv1a32(key.lower()), name_offset, 0, size, key, full])
    records.sort(key=lambda r: (r[0], r[4]))

    table_offset = HEADER_BYTES
    names_offset = table_offset + len(records) * ENTRY_BYTES
    data_offset = align(names_offset + len(names), DATA_ALIGN)

    cursor = data_offset
    for record in records:
        record[2] = cursor
        cursor = align(cursor + record[3], DATA_ALIGN)

    out_dir = os.path.dirname(os.path.abspath(output_file))
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)

    if log_cb:
        log_cb(f"Escribiendo {output_file}...")

    with open(output_file, "wb") as out:
        out.write(struct.pack(">4sIIIIIII", MAGIC, VERSION, len(records), table_offset,
                              names_offset, len(names), DATA_ALIGN, 0))
        for hash_value, name_offset, offset, size, _, _ in records:
            out.write(struct.pack(">IIII", hash_value, name_offset, offset, size))
        out.write(bytes(names))
        out.write(b"\0" * (data_offset - out.tell()))

        for i, (_, _, offset, size, key, full) in enumerate(records, 1):
            out.write(b"\0" * (offset - out.tell()))
            with open(full, "rb") as src:
                while True:
                    chunk = src.read(1 << 20)
                    if not chunk:
                        break
                    out.write(chunk)
            if progress_cb:
                progress_cb(i, total_files, key)
            if log_cb and (i % 50 == 0 or i == total_files):
                log_cb(f"[{i}/{total_files}] Empaquetando: {key}")

        out.write(b"\0" * (align(out.tell(), DATA_ALIGN) - out.tell()))
        total_size = out.tell()

    if log_cb:
        log_cb(f"\n[OK] Empaquetado exitoso: {total_files} archivos ({total_size / (1024*1024):.2f} MB)")
    return total_files, total_size

def unpack_pak(pak_file, out_dir, progress_cb=None, log_cb=None):
    if not os.path.isfile(pak_file):
        raise FileNotFoundError(f"No se encontro el archivo: {pak_file}")

    with open(pak_file, "rb") as f:
        header = f.read(32)
        if len(header) < 32:
            raise ValueError("El archivo es demasiado corto o no es un paquete valido.")

        magic, version, count, table_off, names_off, names_bytes, data_align, _ = struct.unpack(">4sIIIIIII", header)
        if magic != MAGIC:
            raise ValueError(f"Firma magica no valida: {magic} (se esperaba b'MCPK')")

        if log_cb:
            log_cb(f"Leyendo paquete: {pak_file}")
            log_cb(f"Formato MCPK v{version} - {count} archivos contenidos.")
            log_cb(f"Extrayendo hacia: {out_dir} ...\n")

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

            if progress_cb:
                progress_cb(i, count, rel_path)
            if log_cb and (i % 50 == 0 or i == count):
                log_cb(f"[{i}/{count}] Extrayendo: {rel_path} ({size} bytes)")

    if log_cb:
        log_cb(f"\n[OK] Se extrajeron exitosamente {count} archivos en '{out_dir}'.")
    return count


class PakToolGUI(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("OptiCraft Heritage - Herramienta de Assets.pak")
        self.geometry("680x560")
        self.minsize(580, 480)

        # Centrar ventana
        self.update_idletasks()
        w = self.winfo_width()
        h = self.winfo_height()
        ws = self.winfo_screenwidth()
        hs = self.winfo_screenheight()
        x = (ws // 2) - (w // 2)
        y = (hs // 2) - (h // 2)
        self.geometry(f"+{x}+{y}")

        self._init_styles()
        self._build_ui()

    def _init_styles(self):
        style = ttk.Style(self)
        style.theme_use("clam")

    def _build_ui(self):
        # Header banner
        header_frame = tk.Frame(self, bg="#1E1E1E", height=60)
        header_frame.pack(fill=tk.X, side=tk.TOP)

        title_lbl = tk.Label(header_frame, text="OPTICRAFT HERITAGE - GESTOR DE ASSETS.PAK",
                             font=("Segoe UI", 12, "bold"), fg="#FFFFFF", bg="#1E1E1E")
        title_lbl.pack(pady=(12, 2))

        sub_lbl = tk.Label(header_frame, text="Empaquetador y Desempaquetador de recursos para PS2, Wii y PC",
                           font=("Segoe UI", 9), fg="#AAAAAA", bg="#1E1E1E")
        sub_lbl.pack(pady=(0, 10))

        # Notebook tabs
        self.notebook = ttk.Notebook(self)
        self.notebook.pack(fill=tk.BOTH, expand=True, padx=12, pady=10)

        self.pack_tab = ttk.Frame(self.notebook, padding=12)
        self.unpack_tab = ttk.Frame(self.notebook, padding=12)

        self.notebook.add(self.pack_tab, text="  📦 Empaquetar a .pak  ")
        self.notebook.add(self.unpack_tab, text="  📂 Desempaquetar .pak  ")

        self._build_pack_tab()
        self._build_unpack_tab()

        # Barra de progreso y consola inferior
        bottom_frame = ttk.Frame(self, padding=(12, 0, 12, 10))
        bottom_frame.pack(fill=tk.BOTH, expand=True)

        self.progress_bar = ttk.Progressbar(bottom_frame, orient=tk.HORIZONTAL, mode='determinate')
        self.progress_bar.pack(fill=tk.X, pady=(0, 6))

        self.status_lbl = ttk.Label(bottom_frame, text="Listo.", font=("Segoe UI", 9))
        self.status_lbl.pack(anchor=tk.W, pady=(0, 4))

        # Log box
        log_frame = ttk.Frame(bottom_frame)
        log_frame.pack(fill=tk.BOTH, expand=True)

        self.log_text = tk.Text(log_frame, height=9, font=("Consolas", 8), bg="#F8F8F8", fg="#222222",
                                wrap=tk.WORD, relief=tk.SOLID, bd=1)
        scrollbar = ttk.Scrollbar(log_frame, orient=tk.VERTICAL, command=self.log_text.yview)
        self.log_text.configure(yscrollcommand=scrollbar.set)

        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.log_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

    def _build_pack_tab(self):
        lbl_info = ttk.Label(self.pack_tab,
                             text="Selecciona la carpeta que contiene los recursos (ej. 'data' o 'extracted_assets')\n"
                                  "para generar el archivo 'assets.pak' correspondiente.",
                             font=("Segoe UI", 9))
        lbl_info.pack(anchor=tk.W, pady=(0, 12))

        # Carpeta origen
        f1 = ttk.LabelFrame(self.pack_tab, text=" Carpeta Origen de Recursos ", padding=8)
        f1.pack(fill=tk.X, pady=(0, 10))

        self.pack_src_var = tk.StringVar()
        pack_src_entry = ttk.Entry(f1, textvariable=self.pack_src_var, font=("Segoe UI", 9))
        pack_src_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 8))

        btn_browse_src = ttk.Button(f1, text="Buscar Carpeta...", command=self._browse_pack_src)
        btn_browse_src.pack(side=tk.RIGHT)

        # Archivo destino
        f2 = ttk.LabelFrame(self.pack_tab, text=" Archivo .pak de Destino ", padding=8)
        f2.pack(fill=tk.X, pady=(0, 12))

        self.pack_dest_var = tk.StringVar(value="assets.pak")
        pack_dest_entry = ttk.Entry(f2, textvariable=self.pack_dest_var, font=("Segoe UI", 9))
        pack_dest_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 8))

        btn_browse_dest = ttk.Button(f2, text="Guardar Como...", command=self._browse_pack_dest)
        btn_browse_dest.pack(side=tk.RIGHT)

        # Boton ejecutar empaquetado
        self.btn_pack = tk.Button(self.pack_tab, text="⚡ Iniciar Empaquetado", font=("Segoe UI", 10, "bold"),
                                  bg="#2B5B84", fg="#FFFFFF", activebackground="#1E3E5A", activeforeground="#FFFFFF",
                                  relief=tk.FLAT, padx=16, pady=8, cursor="hand2", command=self._start_pack)
        self.btn_pack.pack(pady=4)

    def _build_unpack_tab(self):
        lbl_info = ttk.Label(self.unpack_tab,
                             text="Selecciona un archivo .pak (ej. 'assets.pak') para extraer todas las texturas,\n"
                                  "sonidos, modelos y configuraciones en una carpeta seleccionada.",
                             font=("Segoe UI", 9))
        lbl_info.pack(anchor=tk.W, pady=(0, 12))

        # Archivo pak origen
        f1 = ttk.LabelFrame(self.unpack_tab, text=" Archivo .pak a Descomprimir ", padding=8)
        f1.pack(fill=tk.X, pady=(0, 10))

        self.unpack_src_var = tk.StringVar()
        unpack_src_entry = ttk.Entry(f1, textvariable=self.unpack_src_var, font=("Segoe UI", 9))
        unpack_src_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 8))

        btn_browse_src = ttk.Button(f1, text="Buscar Archivo...", command=self._browse_unpack_src)
        btn_browse_src.pack(side=tk.RIGHT)

        # Carpeta destino
        f2 = ttk.LabelFrame(self.unpack_tab, text=" Carpeta de Destino ", padding=8)
        f2.pack(fill=tk.X, pady=(0, 12))

        self.unpack_dest_var = tk.StringVar(value="extracted_assets")
        unpack_dest_entry = ttk.Entry(f2, textvariable=self.unpack_dest_var, font=("Segoe UI", 9))
        unpack_dest_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 8))

        btn_browse_dest = ttk.Button(f2, text="Buscar Carpeta...", command=self._browse_unpack_dest)
        btn_browse_dest.pack(side=tk.RIGHT)

        # Boton ejecutar desempaquetado
        self.btn_unpack = tk.Button(self.unpack_tab, text="📂 Iniciar Extracción", font=("Segoe UI", 10, "bold"),
                                    bg="#2B8444", fg="#FFFFFF", activebackground="#1E5A2E", activeforeground="#FFFFFF",
                                    relief=tk.FLAT, padx=16, pady=8, cursor="hand2", command=self._start_unpack)
        self.btn_unpack.pack(pady=4)

    def _browse_pack_src(self):
        d = filedialog.askdirectory(title="Seleccionar carpeta origen a empaquetar")
        if d:
            self.pack_src_var.set(os.path.normpath(d))
            if not self.pack_dest_var.get() or self.pack_dest_var.get() == "assets.pak":
                default_out = os.path.join(os.path.dirname(d), "assets.pak")
                self.pack_dest_var.set(os.path.normpath(default_out))

    def _browse_pack_dest(self):
        f = filedialog.asksaveasfilename(title="Guardar archivo .pak como",
                                         defaultextension=".pak",
                                         filetypes=[("Archivos PAK", "*.pak"), ("Todos los archivos", "*.*")])
        if f:
            self.pack_dest_var.set(os.path.normpath(f))

    def _browse_unpack_src(self):
        f = filedialog.askopenfilename(title="Seleccionar archivo .pak a descomprimir",
                                       filetypes=[("Archivos PAK", "*.pak"), ("Todos los archivos", "*.*")])
        if f:
            self.unpack_src_var.set(os.path.normpath(f))
            if not self.unpack_dest_var.get() or self.unpack_dest_var.get() == "extracted_assets":
                default_dest = os.path.join(os.path.dirname(f), "extracted_assets")
                self.unpack_dest_var.set(os.path.normpath(default_dest))

    def _browse_unpack_dest(self):
        d = filedialog.askdirectory(title="Seleccionar carpeta de destino para la extracción")
        if d:
            self.unpack_dest_var.set(os.path.normpath(d))

    def log(self, text):
        self.log_text.insert(tk.END, text + "\n")
        self.log_text.see(tk.END)

    def set_status(self, text):
        self.status_lbl.config(text=text)

    def set_progress(self, current, total, name=""):
        pct = (current / total) * 100 if total > 0 else 0
        self.progress_bar['value'] = pct
        self.status_lbl.config(text=f"[{current}/{total}] {name}")

    def _start_pack(self):
        src = self.pack_src_var.get().strip()
        dest = self.pack_dest_var.get().strip()
        if not src or not os.path.isdir(src):
            messagebox.showerror("Error", "Por favor selecciona una carpeta origen valida.")
            return
        if not dest:
            messagebox.showerror("Error", "Por favor especifica la ruta de destino para el archivo .pak.")
            return

        self.btn_pack.config(state=tk.DISABLED)
        self.btn_unpack.config(state=tk.DISABLED)
        self.log_text.delete(1.0, tk.END)
        self.progress_bar['value'] = 0

        def run():
            try:
                count, size = pack_folder(src, dest,
                    progress_cb=lambda c, t, n: self.after(0, self.set_progress, c, t, n),
                    log_cb=lambda msg: self.after(0, self.log, msg))
                self.after(0, lambda: messagebox.showinfo(
                    "Empaquetado Completado",
                    f"¡Archivo generado con exito!\n\n"
                    f"Archivos contenidos: {count}\n"
                    f"Tamano final: {size / (1024*1024):.2f} MB\n"
                    f"Destino: {dest}"))
                self.after(0, lambda: self.set_status(f"Completado: {count} archivos empaquetados."))
            except Exception as e:
                self.after(0, lambda: messagebox.showerror("Error", f"Ocurrio un error al empaquetar:\n{e}"))
                self.after(0, lambda: self.log(f"[ERROR] {e}"))
                self.after(0, lambda: self.set_status("Error al empaquetar."))
            finally:
                self.after(0, lambda: self.btn_pack.config(state=tk.NORMAL))
                self.after(0, lambda: self.btn_unpack.config(state=tk.NORMAL))

        threading.Thread(target=run, daemon=True).start()

    def _start_unpack(self):
        src = self.unpack_src_var.get().strip()
        dest = self.unpack_dest_var.get().strip()
        if not src or not os.path.isfile(src):
            messagebox.showerror("Error", "Por favor selecciona un archivo .pak valido.")
            return
        if not dest:
            messagebox.showerror("Error", "Por favor especifica la carpeta de destino.")
            return

        self.btn_pack.config(state=tk.DISABLED)
        self.btn_unpack.config(state=tk.DISABLED)
        self.log_text.delete(1.0, tk.END)
        self.progress_bar['value'] = 0

        def run():
            try:
                count = unpack_pak(src, dest,
                    progress_cb=lambda c, t, n: self.after(0, self.set_progress, c, t, n),
                    log_cb=lambda msg: self.after(0, self.log, msg))
                self.after(0, lambda: messagebox.showinfo(
                    "Extracción Completada",
                    f"¡Extraccion finalizada con exito!\n\n"
                    f"Archivos extraidos: {count}\n"
                    f"Carpeta: {dest}"))
                self.after(0, lambda: self.set_status(f"Completado: {count} archivos extraidos."))
            except Exception as e:
                self.after(0, lambda: messagebox.showerror("Error", f"Ocurrio un error al desempaquetar:\n{e}"))
                self.after(0, lambda: self.log(f"[ERROR] {e}"))
                self.after(0, lambda: self.set_status("Error al desempaquetar."))
            finally:
                self.after(0, lambda: self.btn_pack.config(state=tk.NORMAL))
                self.after(0, lambda: self.btn_unpack.config(state=tk.NORMAL))

        threading.Thread(target=run, daemon=True).start()


def main():
    app = PakToolGUI()
    app.mainloop()

if __name__ == "__main__":
    main()
