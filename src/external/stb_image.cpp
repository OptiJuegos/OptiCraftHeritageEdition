// stb_image, built once for the whole project.
//
// STBI_NO_THREAD_LOCALS is not a preference -- it is required for correctness on
// devkitPPC (Wii), where `thread_local` is silently miscompiled.
//
// stb_image declares its "flip vertically on load", "unpremultiply on load",
// "de-iPhone" and failure-reason flags as STBI_THREAD_LOCAL, and reads each one
// as `<flag>_set ? <flag>_local : <flag>_global`. On this toolchain:
//
//   * libogc's linker script (rvl.ld -> libogc_common.ld) declares no .tdata or
//     .tbss output section, so the link emits no PT_TLS segment and every .tbss
//     input section is orphan-placed at TLS offset 0 -- all seven thread_locals
//     end up aliasing the *same* word;
//   * gcc still emits the local-exec model and addresses that word as r2+TPREL,
//     but crt0 loads r2 with _SDA2_BASE_ (the EABI small-data pointer, not a
//     thread pointer). The accesses resolve to r2-0x7000, which in the linked
//     ELF lands inside .eh_frame_hdr.
//
// So `stbi__vertically_flip_on_load_set` reads live DWARF data, which is
// non-zero, the flip flag evaluates to true, and *every* PNG the game decodes
// comes out upside down. That is what turned the Wii main menu into white
// rectangles: the buttons, the logo and the font all sample the top rows of
// their sheets, and a flipped sheet answers with the blank bottom rows. The dirt
// background survived only because tiled noise looks the same either way.
//
// The same aliasing makes stbi_failure_reason() hand back a wild pointer (which
// BufferedImage::ImageIO_read would then construct a std::string from), and
// stbi__err() writing to it corrupts .eh_frame_hdr under a live exception path.
//
// STBI_NO_THREAD_LOCALS drops the qualifier and uses the plain file-scope
// statics. Nothing in this port calls the *_thread setters and the flags are
// only ever read at their zero-initialised defaults, so the change is
// behaviour-identical on desktop and PS2 too -- hence unconditional, rather than
// a WII_PLATFORM branch that would leave two code paths to reason about.
//
// cmake/wii_check_no_tls.cmake asserts after every Wii link that no .tbss
// survived, so a future thread_local cannot reintroduce this quietly.
#define STBI_NO_THREAD_LOCALS

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
