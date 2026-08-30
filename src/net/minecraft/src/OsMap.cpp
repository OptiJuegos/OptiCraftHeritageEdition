#include "OsMap.h"

// switchMap indexed by EnumOS1 ordinal: linux=0, solaris=1, windows=2, macos=3, unknown=4
// Values match the Java static initializer: linux->1, solaris->2, windows->3, macos->4
const int_t OsMap::switchMap[5] = { 1, 2, 3, 4, 0 };
