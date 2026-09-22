## Building for PlayStation 2

![MC:PS2](https://drive.google.com/uc?export=view&id=1U29xSOC7pMjn4WNkd6KTkb78eKljMzT_)

MC:PS2 includes automatic build scripts for Linux and Windows.

The scripts check whether the required build tools and PS2 development environment are already installed. If something is missing, they will ask whether you want to install it automatically.

The build process generates:

```text
bin/ps2/usb/MCBETA/OptiCraft.elf
bin/ps2/usb/MCBETA/assets.pak
```

The default PS2 build includes networking, multiplayer, sound, perspective-correct textures, and VU1 terrain rendering.

### Linux

From the project root, make the build script executable:

```bash
chmod +x build_ps2_linux.sh
```

Then run:

```bash
./build_ps2_linux.sh
```

For a completely clean build:

```bash
./build_ps2_linux.sh --clean
```

To automatically accept installation prompts:

```bash
./build_ps2_linux.sh --clean --yes
```

The script checks for the required build tools and PS2 development environment and offers to install anything that is missing.

The default Linux PS2DEV location is:

```text
/usr/local/ps2dev
```

### Windows

The Windows build script uses PowerShell.

From the project root, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\build_ps2_windows.ps1
```

For a completely clean build:

```powershell
powershell -ExecutionPolicy Bypass -File .\build_ps2_windows.ps1 -Clean
```

To automatically accept installation prompts:

```powershell
powershell -ExecutionPolicy Bypass -File .\build_ps2_windows.ps1 -Clean -Yes
```

The script checks for the required Windows build tools and PS2 development environment and offers to install anything that is missing.

### Build Output

After a successful build, the generated files can be found at:

```text
bin/ps2/usb/MCBETA/
```

The main build files are:

```text
OptiCraft.elf
assets.pak
```

---

## Disclaimer!!!

MC:PS2 is a fork of the original [OptiCraft Heritage](https://github.com/OptiJuegos/OptiCraftHeritageEdition) Edition project.

This fork is not intended to replace the original project. Its primary focus is to improve the PlayStation 2 version while preserving the original projects overall direction and architecture whenever practical.

The project focuses on PS2-specific improvements such as performance and memory optimizations, multiplayer support, bug fixes, controller and interface improvements, storage handling, localization, compatibility with real hardware, and other quality-of-life features.

Whenever possible, changes are kept isolated and maintainable so they can coexist with the broader OptiCraft codebase without unnecessarily diverging from the upstream project.

The original [OptiCraft Heritage Edition](https://github.com/OptiJuegos/OptiCraftHeritageEdition) project remains the upstream project, while MC:PS2 serves as a PS2-focused extension and improvement effort.

This project is not affiliated with, endorsed by, or sponsored by Mojang Studios or Microsoft. Minecraft and its associated trademarks and intellectual property belong to their respective owners.

[![Star History Chart](https://api.star-history.com/svg?repos=thebrokenwall/minecraft-playstation-2-edition&type=Date)](https://www.star-history.com/?repos=thebrokenwall%2Fminecraft-playstation-2-edition&type=date&legend=top-left)
