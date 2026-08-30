/*     */ 
/*     */ 
/*     */ public enum zq
/*     */ {
/*   5 */   a("options.music", true, false),
/*   6 */   b("options.sound", true, false),
/*   7 */   c("options.invertMouse", false, true),
/*   8 */   d("options.sensitivity", true, false),
/*   9 */   e("options.fov", true, false),
/*  10 */   f("options.gamma", true, false),
/*  11 */   g("options.renderDistance", false, false),
/*  12 */   h("options.viewBobbing", false, true),
/*  13 */   i("options.anaglyph", false, true),
/*  14 */   j("options.advancedOpengl", false, true),
/*  15 */   k("options.framerateLimit", false, false),
/*  16 */   l("options.difficulty", false, false),
/*  17 */   m("options.graphics", false, false),
/*  18 */   n("options.ao", false, true),
/*  19 */   o("options.guiScale", false, false),
/*  20 */   p("options.renderClouds", false, true),
/*  21 */   q("options.particles", false, false),
/*     */   
/*  23 */   FOG_FANCY("Fog", false, false),
/*  24 */   FOG_START("Fog Start", false, false),
/*  25 */   MIPMAP_LEVEL("Mipmap Level", false, false),
/*  26 */   MIPMAP_TYPE("Mipmap Type", false, false),
/*  27 */   LOAD_FAR("Load Far", false, false),
/*  28 */   PRELOADED_CHUNKS("Preloaded Chunks", false, false),
/*  29 */   SMOOTH_FPS("Smooth FPS", false, false),
/*     */   
/*  31 */   CLOUDS("Clouds", false, false),
/*  32 */   CLOUD_HEIGHT("Cloud Height", true, false),
/*  33 */   TREES("Trees", false, false),
/*  34 */   GRASS("Grass", false, false),
/*  35 */   RAIN("Rain & Snow", false, false),
/*  36 */   WATER("Water", false, false),
/*  37 */   ANIMATED_WATER("Water Animated", false, false),
/*  38 */   ANIMATED_LAVA("Lava Animated", false, false),
/*  39 */   ANIMATED_FIRE("Fire Animated", false, false),
/*  40 */   ANIMATED_PORTAL("Portal Animated", false, false),
/*  41 */   AO_LEVEL("Smooth Lighting", true, false),
/*  42 */   FAST_DEBUG_INFO("Fast Debug Info", false, false),
/*  43 */   AUTOSAVE_TICKS("Autosave", false, false),
/*  44 */   BETTER_GRASS("Better Grass", false, false),
/*  45 */   ANIMATED_REDSTONE("Redstone Animated", false, false),
/*  46 */   ANIMATED_EXPLOSION("Explosion Animated", false, false),
/*  47 */   ANIMATED_FLAME("Flame Animated", false, false),
/*  48 */   ANIMATED_SMOKE("Smoke Animated", false, false),
/*  49 */   WEATHER("Weather", false, false),
/*  50 */   SKY("Sky", false, false),
/*  51 */   STARS("Stars", false, false),
/*  52 */   SUN_MOON("Sun & Moon", false, false),
/*  53 */   CHUNK_UPDATES("Chunk Updates", false, false),
/*  54 */   CHUNK_UPDATES_DYNAMIC("Dynamic Updates", false, false),
/*  55 */   TIME("Time", false, false),
/*  56 */   CLEAR_WATER("Clear Water", false, false),
/*  57 */   SMOOTH_INPUT("Smooth Input", false, false),
/*  58 */   DEPTH_FOG("Depth Fog", false, false),
/*  59 */   VOID_PARTICLES("Void Particles", false, false),
/*  60 */   WATER_PARTICLES("Water Particles", false, false),
/*  61 */   RAIN_SPLASH("Rain Splash", false, false),
/*  62 */   PORTAL_PARTICLES("Portal Particles", false, false),
/*  63 */   PROFILER("Debug Profiler", false, false),
/*  64 */   DRIPPING_WATER_LAVA("Dripping Water/Lava", false, false),
/*  65 */   BETTER_SNOW("Better Snow", false, false),
/*  66 */   FULLSCREEN_MODE("Fullscreen", false, false),
/*  67 */   ANIMATED_TERRAIN("Terrain Animated", false, false),
/*  68 */   ANIMATED_ITEMS("Items Animated", false, false),
/*  69 */   SWAMP_COLORS("Swamp Colors", false, false),
/*  70 */   RANDOM_MOBS("Random Mobs", false, false),
/*  71 */   SMOOTH_BIOMES("Smooth Biomes", false, false),
/*  72 */   CUSTOM_FONTS("Custom Fonts", false, false),
/*  73 */   CUSTOM_COLORS("Custom Colors", false, false),
/*  74 */   SHOW_CAPES("Show Capes", false, false),
/*  75 */   CONNECTED_TEXTURES("Connected Textures", false, false),
/*  76 */   AA_LEVEL("Antialiasing", false, false),
/*  77 */   AF_LEVEL("Anisotropic Filtering", false, false),
/*  78 */   RENDER_DISTANCE_FINE("Render Distance", true, false),
/*  79 */   ANIMATED_TEXTURES("Textures Animated", false, false),
/*  80 */   NATURAL_TEXTURES("Natural Textures", false, false);
/*     */   
/*     */   private final boolean r;
/*     */   
/*     */   private final boolean s;
/*     */   
/*     */   private final String t;
/*     */   
/*     */   public static zq a(int par0) {
/*  89 */     zq[] aenumoptions = values();
/*  90 */     int i = aenumoptions.length;
/*  91 */     for (int j = 0; j < i; j++) {
/*     */       
/*  93 */       zq enumoptions = aenumoptions[j];
/*  94 */       if (enumoptions.c() == par0)
/*     */       {
/*  96 */         return enumoptions;
/*     */       }
/*     */     } 
/*     */     
/* 100 */     return null;
/*     */   }
/*     */ 
/*     */   
/*     */   zq(String par3Str, boolean par4, boolean par5) {
/* 105 */     this.t = par3Str;
/* 106 */     this.r = par4;
/* 107 */     this.s = par5;
/*     */   }
/*     */ 
/*     */   
/*     */   public boolean a() {
/* 112 */     return this.r;
/*     */   }
/*     */ 
/*     */   
/*     */   public boolean b() {
/* 117 */     return this.s;
/*     */   }
/*     */ 
/*     */   
/*     */   public int c() {
/* 122 */     return ordinal();
/*     */   }
/*     */ 
/*     */   
/*     */   public String d() {
/* 127 */     return this.t;
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\zq.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */