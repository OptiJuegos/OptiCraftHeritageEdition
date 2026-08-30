# PS2 platform layout

The PS2 backend is grouped by responsibility instead of keeping every platform file in one directory.

- `boot/`: pre-game UI and startup sequence helpers.
- `input/`: DualShock initialization, polling and state shared by the LWJGL adapters and game UI.
- `java/`: PS2 implementations of Java-like runtime abstractions.
- `lwjgl/`: PS2 implementations of the LWJGL compatibility surface.
- `network/`: optional PS2 network backend.
- `render/`: gsKit/OpenGL compatibility, native draw paths, clipping, mesh helpers and VU transform integration.
- `storage/`: device-level POSIX helpers. `storage/assets/` owns install/resource discovery and `storage/save/` owns Memory Card/USB save policy and file backends.
- `system/`: IOP/file-service bring-up, low-level EE runtime helpers and tracing.
- `vu1/`: VU1 microprogram sources.
- `main_ps2.cpp`: high-level bring-up order and hand-off to `Minecraft::start()`.

## Filesystems and assets

Cross-console POSIX filesystem code belongs in `src/platform/storage/`. Wii libfat and PS2 filesystem devices exposed through newlib should reuse that layer. Memory Card code stays PS2-specific because libmc does not provide normal POSIX file semantics.

`storage/assets/Ps2Assets` treats the game installation separately from save storage. It first derives `data/` from the directory of `argv[0]`, so an ELF launched from an already exposed filesystem can load assets from the same location without the resource layer knowing whether that transport is USB, PFS/HDD, CD/DVD or `host:`. Compatibility fallback roots are probed only when the launch path cannot resolve the install.

HDD/PFS roots must already be mounted by the loader/bootstrap before `Ps2Assets` can use them. Optical-disc roots are read-only and depend on the active CD/DVD filesystem driver exposing normal file operations. Neither case changes the save target selected by the user.

`Ps2AssetLocator` owns install discovery. Its priority is an explicit `--data-root=<path>`, then `data/` beside the launched ELF/current directory, then mounted compatibility roots. USB fallbacks get a short spin-up retry; HDD, disc and host paths are probed once so an absent optical disc cannot delay every boot retry. Disc lookups also try the ISO-9660 uppercase spelling and `;1` file version suffix when the normal path is not readable.

The locator does not mount arbitrary HDD partitions itself. A raw `hdd0:` device is not a normal asset filesystem: a launcher or bootstrap must expose the desired partition as `pfsN:` first, or pass an already mounted path with `--data-root`. This avoids baking a project-specific APA partition name into the game.
