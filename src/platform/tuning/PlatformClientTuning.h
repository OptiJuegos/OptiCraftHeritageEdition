#pragma once

// -----------------------------------------------------------------------------
// Client lifecycle policies
// -----------------------------------------------------------------------------
// These describe behavior, not hardware APIs. Minecraft.cpp should consume these
// neutral names rather than branching on Wii/PS2 directly.
#if PLATFORM_PS2
#  define PLATFORM_LOAD_TERRAIN_WARMUP_MS          0
#  define PLATFORM_PRELOAD_LIGHTING_STEPS          4
#  define PLATFORM_UNLOAD_AFTER_PRELOAD             0
#  define PLATFORM_DEFER_PORTAL_TRANSITION          0
#elif PLATFORM_WII
#  define PLATFORM_LOAD_TERRAIN_WARMUP_MS          PLATFORM_WII_LOAD_WARMUP_MS
#  define PLATFORM_PRELOAD_LIGHTING_STEPS          0
#  define PLATFORM_UNLOAD_AFTER_PRELOAD             0
#  define PLATFORM_DEFER_PORTAL_TRANSITION          1
#else
#  define PLATFORM_LOAD_TERRAIN_WARMUP_MS          0
#  define PLATFORM_PRELOAD_LIGHTING_STEPS          0
#  define PLATFORM_UNLOAD_AFTER_PRELOAD             1
#  define PLATFORM_DEFER_PORTAL_TRANSITION          0
#endif

#if PLATFORM_PS2
#  define PLATFORM_CLIENT_TIMER_HACK_THREAD          0
#  define PLATFORM_CLIENT_PAID_CHECK                 0
#  define PLATFORM_SYNC_STATS_ON_GUI_CHANGE          0
#  define PLATFORM_EXIT_PROCESS_ON_SHUTDOWN          0
#  define PLATFORM_RETURN_TO_MENU_ON_OOM             0
#  define PLATFORM_RELEASE_OLD_WORLD_BEFORE_PORTAL   0
#elif PLATFORM_WII
#  define PLATFORM_CLIENT_TIMER_HACK_THREAD          0
#  define PLATFORM_CLIENT_PAID_CHECK                 0
#  define PLATFORM_SYNC_STATS_ON_GUI_CHANGE          0
#  define PLATFORM_EXIT_PROCESS_ON_SHUTDOWN          1
#  if WII_OOM_ERROR_SCREEN
#    define PLATFORM_RETURN_TO_MENU_ON_OOM           0
#  else
#    define PLATFORM_RETURN_TO_MENU_ON_OOM           1
#  endif
#  define PLATFORM_RELEASE_OLD_WORLD_BEFORE_PORTAL   1
#else
#  define PLATFORM_CLIENT_TIMER_HACK_THREAD          1
#  ifdef NO_NETWORK
#    define PLATFORM_CLIENT_PAID_CHECK               0
#  else
#    define PLATFORM_CLIENT_PAID_CHECK               1
#  endif
#  define PLATFORM_SYNC_STATS_ON_GUI_CHANGE          1
#  define PLATFORM_EXIT_PROCESS_ON_SHUTDOWN          1
#  define PLATFORM_RETURN_TO_MENU_ON_OOM             0
#  define PLATFORM_RELEASE_OLD_WORLD_BEFORE_PORTAL   0
#endif
