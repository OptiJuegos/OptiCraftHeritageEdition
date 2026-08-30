/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ public class jy
/*     */   extends vp
/*     */ {
/*  22 */   private int lastMouseX = 0;
/*  23 */   private int lastMouseY = 0;
/*  24 */   private long mouseStillTime = 0L;
/*     */ 
/*     */   
/*     */   public jy(vp par1GuiScreen, hu par2GameSettings) {
/*  28 */     this.a = "Video Settings";
/*  29 */     this.d = false;
/*  30 */     this.b = par1GuiScreen;
/*  31 */     this.c = par2GameSettings;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void c() {
/*  39 */     adn stringtranslate = adn.a();
/*  40 */     this.a = stringtranslate.b("options.videoTitle");
/*  41 */     int i = 0;
/*  42 */     zq[] aobj = e;
/*  43 */     int j = aobj.length;
/*  44 */     for (int k = 0; k < j; k++) {
/*     */       
/*  46 */       zq enumoptions = aobj[k];
/*     */       
/*  48 */       int m = this.q / 2 - 155 + i % 2 * 160;
/*  49 */       int n = this.r / 6 + 21 * i / 2 - 10;
/*     */ 
/*     */       
/*  52 */       if (!enumoptions.a()) {
/*     */         
/*  54 */         this.s.add(new z(enumoptions.c(), m, n, enumoptions, this.c.c(enumoptions)));
/*     */       } else {
/*     */         
/*  57 */         this.s.add(new ajz(enumoptions.c(), m, n, enumoptions, this.c.c(enumoptions), this.c.a(enumoptions)));
/*     */       } 
/*  59 */       i++;
/*     */     } 
/*     */     
/*  62 */     int y = this.r / 6 + 21 * i / 2 - 10;
/*  63 */     int x = 0;
/*     */     
/*  65 */     x = this.q / 2 - 155 + 0;
/*  66 */     this.s.add(new z(101, x, y, "Details..."));
/*     */     
/*  68 */     x = this.q / 2 - 155 + 160;
/*  69 */     this.s.add(new z(102, x, y, "Quality..."));
/*     */     
/*  71 */     y += 21;
/*     */     
/*  73 */     x = this.q / 2 - 155 + 0;
/*  74 */     this.s.add(new z(111, x, y, "Animations..."));
/*     */     
/*  76 */     x = this.q / 2 - 155 + 160;
/*  77 */     this.s.add(new z(112, x, y, "Performance..."));
/*     */     
/*  79 */     y += 21;
/*     */     
/*  81 */     x = this.q / 2 - 155 + 0;
/*  82 */     this.s.add(new z(121, x, y, "Texture Packs..."));
/*     */     
/*  84 */     x = this.q / 2 - 155 + 160;
/*  85 */     this.s.add(new z(122, x, y, "Other..."));
/*     */     
/*  87 */     this.s.add(new abp(200, this.q / 2 - 100, this.r / 6 + 168 + 11, stringtranslate.b("gui.done")));
/*     */     
/*  89 */     this.d = false;
/*  90 */     String[] aobj2 = { "sun.arch.data.model", "com.ibm.vm.bitmode", "os.arch" };
/*     */ 
/*     */     
/*  93 */     String[] as = aobj2;
/*  94 */     int l = as.length;
/*  95 */     int i1 = 0;
/*     */ 
/*     */     
/*  98 */     while (i1 < l) {
/*     */ 
/*     */ 
/*     */       
/* 102 */       String s = as[i1];
/* 103 */       String s1 = System.getProperty(s);
/* 104 */       if (s1 != null && s1.indexOf("64") >= 0) {
/*     */         
/* 106 */         this.d = true;
/*     */         break;
/*     */       } 
/* 109 */       i1++;
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   protected void a(abp par1GuiButton) {
/* 118 */     if (!par1GuiButton.h) {
/*     */       return;
/*     */     }
/*     */     
/* 122 */     int i = this.c.P;
/* 123 */     if (par1GuiButton.f < 100 && par1GuiButton instanceof z) {
/*     */       
/* 125 */       this.c.a(((z)par1GuiButton).a(), 1);
/* 126 */       par1GuiButton.e = this.c.c(zq.a(par1GuiButton.f));
/*     */     } 
/* 128 */     if (par1GuiButton.f == 200) {
/*     */       
/* 130 */       this.p.A.b();
/* 131 */       this.p.a(this.b);
/*     */     } 
/*     */     
/* 134 */     if (par1GuiButton.f == 101) {
/*     */       
/* 136 */       this.p.A.b();
/* 137 */       GuiDetailSettingsOF scr = new GuiDetailSettingsOF(this, this.c);
/* 138 */       this.p.a(scr);
/*     */     } 
/*     */     
/* 141 */     if (par1GuiButton.f == 102) {
/*     */       
/* 143 */       this.p.A.b();
/* 144 */       GuiQualitySettingsOF scr = new GuiQualitySettingsOF(this, this.c);
/* 145 */       this.p.a(scr);
/*     */     } 
/*     */     
/* 148 */     if (par1GuiButton.f == 111) {
/*     */       
/* 150 */       this.p.A.b();
/* 151 */       GuiAnimationSettingsOF scr = new GuiAnimationSettingsOF(this, this.c);
/* 152 */       this.p.a(scr);
/*     */     } 
/*     */     
/* 155 */     if (par1GuiButton.f == 112) {
/*     */       
/* 157 */       this.p.A.b();
/* 158 */       GuiPerformanceSettingsOF scr = new GuiPerformanceSettingsOF(this, this.c);
/* 159 */       this.p.a(scr);
/*     */     } 
/*     */     
/* 162 */     if (par1GuiButton.f == 121) {
/*     */       
/* 164 */       this.p.A.b();
/* 165 */       em scr = new em(this);
/* 166 */       this.p.a((vp)scr);
/*     */     } 
/*     */     
/* 169 */     if (par1GuiButton.f == 122) {
/*     */       
/* 171 */       this.p.A.b();
/* 172 */       GuiOtherSettingsOF scr = new GuiOtherSettingsOF(this, this.c);
/* 173 */       this.p.a(scr);
/*     */     } 
/*     */     
/* 176 */     if (par1GuiButton.f == zq.AO_LEVEL.ordinal()) {
/*     */       return;
/*     */     }
/* 179 */     if (this.c.P != i) {
/*     */       
/* 181 */       agd scaledresolution = new agd(this.p.A, this.p.d, this.p.e);
/* 182 */       int j = scaledresolution.a();
/* 183 */       int k = scaledresolution.b();
/* 184 */       a(this.p, j, k);
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void a(int x, int y, float f) {
/* 193 */     k();
/* 194 */     a(this.u, this.a, this.q / 2, 20, 16777215);
/* 195 */     if (this.d || this.c.e == 0);
/*     */ 
/*     */ 
/*     */ 
/*     */     
/* 200 */     super.a(x, y, f);
/*     */     
/* 202 */     if (Math.abs(x - this.lastMouseX) > 5 || Math.abs(y - this.lastMouseY) > 5) {
/*     */       
/* 204 */       this.lastMouseX = x;
/* 205 */       this.lastMouseY = y;
/* 206 */       this.mouseStillTime = System.currentTimeMillis();
/*     */       
/*     */       return;
/*     */     } 
/* 210 */     int activateDelay = 700;
/* 211 */     if (System.currentTimeMillis() < this.mouseStillTime + activateDelay) {
/*     */       return;
/*     */     }
/* 214 */     int x1 = this.q / 2 - 150;
/* 215 */     int y1 = this.r / 6 - 5;
/* 216 */     if (y <= y1 + 98)
/* 217 */       y1 += 105; 
/* 218 */     int x2 = x1 + 150 + 150;
/* 219 */     int y2 = y1 + 84 + 10;
/*     */     
/* 221 */     abp btn = getSelectedButton(x, y);
/* 222 */     if (btn != null) {
/*     */       
/* 224 */       String s = getButtonName(btn.e);
/* 225 */       String[] lines = getTooltipLines(s);
/* 226 */       if (lines == null)
/*     */         return; 
/* 228 */       a(x1, y1, x2, y2, -536870912, -536870912);
/* 229 */       for (int i = 0; i < lines.length; i++) {
/*     */         
/* 231 */         String line = lines[i];
/* 232 */         this.u.a(line, x1 + 5, y1 + 5 + i * 11, 14540253);
/*     */       } 
/*     */     } 
/*     */   }
/*     */ 
/*     */   
/*     */   private String[] getTooltipLines(String btnName) {
/* 239 */     if (btnName.equals("Graphics")) {
/* 240 */       return new String[] { "Visual quality", "  Fast  - lower quality, faster", "  Fancy - higher quality, slower", "Changes the appearance of clouds, leaves, water,", "shadows and grass sides." };
/*     */     }
/*     */ 
/*     */ 
/*     */     
/* 245 */     if (btnName.equals("Render Distance")) {
/* 246 */       return new String[] { "Visible distance", "  Tiny - 32m (fastest)", "  Short - 64m (faster)", "  Normal - 128m", "  Far - 256m (slower)", "  Extreme - 512m (slowest!)", "The Extreme view distance is very resource demanding!" };
/*     */     }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */     
/* 254 */     if (btnName.equals("Smooth Lighting")) {
/* 255 */       return new String[] { "Smooth lighting", "  OFF - no smooth lighting (faster)", "  1% - light smooth lighting (slower)", "  100% - dark smooth lighting (slower)" };
/*     */     }
/*     */ 
/*     */     
/* 259 */     if (btnName.equals("Performance")) {
/* 260 */       return new String[] { "FPS Limit", "  Max FPS - no limit (fastest)", "  Balanced - limit 120 FPS (slower)", "  Power saver - limit 40 FPS (slowest)", "  VSync - limit to monitor framerate (60, 30, 20)", "Balanced and Power saver decrease the FPS even if", "the limit value is not reached." };
/*     */     }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */     
/* 267 */     if (btnName.equals("3D Anaglyph"))
/* 268 */       return new String[] { "3D mode used with red-cyan 3D glasses." }; 
/* 269 */     if (btnName.equals("View Bobbing")) {
/* 270 */       return new String[] { "More realistic movement.", "When using mipmaps set it to OFF for best results." };
/*     */     }
/* 272 */     if (btnName.equals("GUI Scale")) {
/* 273 */       return new String[] { "GUI Scale", "Smaller GUI might be faster" };
/*     */     }
/* 275 */     if (btnName.equals("Advanced OpenGL")) {
/* 276 */       return new String[] { "Detect and render only visible geometry", "  OFF - all geometry is rendered (slower)", "  Fast - only visible geometry is rendered (fastest)", "  Fancy - conservative, avoids visual artifacts (faster)", "The option is available only if it is supported by the ", "graphic card." };
/*     */     }
/*     */ 
/*     */ 
/*     */ 
/*     */     
/* 282 */     if (btnName.equals("Fog")) {
/* 283 */       return new String[] { "Fog type", "  Fast - faster fog", "  Fancy - slower fog, looks better", "  OFF - no fog, fastest", "The fancy fog is available only if it is supported by the ", "graphic card." };
/*     */     }
/*     */ 
/*     */ 
/*     */ 
/*     */     
/* 289 */     if (btnName.equals("Fog Start")) {
/* 290 */       return new String[] { "Fog start", "  0.2 - the fog starts near the player", "  0.8 - the fog starts far from the player", "This option usually does not affect the performance." };
/*     */     }
/*     */ 
/*     */     
/* 294 */     if (btnName.equals("Brightness")) {
/* 295 */       return new String[] { "Increases the brightness of darker objects", "  OFF - standard brightness", "  100% - maximum brightness for darker objects", "This options does not change the brightness of ", "fully black objects" };
/*     */     }
/*     */ 
/*     */ 
/*     */     
/* 300 */     if (btnName.equals("Clouds")) {
/* 301 */       return new String[] { "Rendering of clouds", "  ON - standard clouds (slower)", "  OFF - no clouds (faster)" };
/*     */     }
/*     */ 
/*     */ 
/*     */     
/* 306 */     return null;
/*     */   }
/*     */ 
/*     */   
/*     */   private String getButtonName(String displayString) {
/* 311 */     int pos = displayString.indexOf(':');
/* 312 */     if (pos < 0) {
/* 313 */       return displayString;
/*     */     }
/* 315 */     return displayString.substring(0, pos);
/*     */   }
/*     */ 
/*     */   
/*     */   private abp getSelectedButton(int i, int j) {
/* 320 */     for (int k = 0; k < this.s.size(); k++) {
/*     */       
/* 322 */       abp btn = this.s.get(k);
/* 323 */       boolean flag = (i >= btn.c && j >= btn.d && i < btn.c + btn.a && j < btn.d + btn.b);
/* 324 */       if (flag)
/* 325 */         return btn; 
/*     */     } 
/* 327 */     return null;
/*     */   }
/*     */ 
/*     */ 
/*     */   
/* 332 */   private static zq[] e = new zq[] { zq.m, zq.RENDER_DISTANCE_FINE, zq.AO_LEVEL, zq.k, zq.i, zq.h, zq.o, zq.j, zq.f, zq.p, zq.FOG_FANCY, zq.FOG_START };
/*     */   private vp b;
/*     */   protected String a;
/*     */   private hu c;
/*     */   private boolean d;
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\jy.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */