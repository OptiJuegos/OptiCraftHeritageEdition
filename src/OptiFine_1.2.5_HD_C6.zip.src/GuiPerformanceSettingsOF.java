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
/*     */ public class GuiPerformanceSettingsOF
/*     */   extends vp
/*     */ {
/*     */   private vp prevScreen;
/*     */   protected String title;
/*     */   private hu settings;
/*     */   
/*     */   public GuiPerformanceSettingsOF(vp guiscreen, hu gamesettings) {
/* 186 */     this.lastMouseX = 0;
/* 187 */     this.lastMouseY = 0;
/* 188 */     this.mouseStillTime = 0L;
/*     */     this.title = "Performance Settings";
/*     */     this.prevScreen = guiscreen;
/*     */     this.settings = gamesettings;
/* 192 */   } private static zq[] enumOptions = new zq[] { zq.SMOOTH_FPS, zq.SMOOTH_INPUT, zq.LOAD_FAR, zq.PRELOADED_CHUNKS, zq.CHUNK_UPDATES, zq.CHUNK_UPDATES_DYNAMIC };
/*     */   
/*     */   public void c() {
/*     */     adn stringtranslate = adn.a();
/*     */     int i = 0;
/*     */     zq[] aenumoptions = enumOptions;
/*     */     int j = aenumoptions.length;
/*     */     for (int k = 0; k < j; k++) {
/*     */       zq enumoptions = aenumoptions[k];
/*     */       int x = this.q / 2 - 155 + i % 2 * 160;
/*     */       int y = this.r / 6 + 21 * i / 2 - 10;
/*     */       if (!enumoptions.a()) {
/*     */         this.s.add(new z(enumoptions.c(), x, y, enumoptions, this.settings.c(enumoptions)));
/*     */       } else {
/*     */         this.s.add(new ajz(enumoptions.c(), x, y, enumoptions, this.settings.c(enumoptions), this.settings.a(enumoptions)));
/*     */       } 
/*     */       i++;
/*     */     } 
/*     */     this.s.add(new abp(200, this.q / 2 - 100, this.r / 6 + 168 + 11, stringtranslate.b("gui.done")));
/*     */   }
/*     */   
/*     */   protected void a(abp guibutton) {
/*     */     if (!guibutton.h)
/*     */       return; 
/*     */     if (guibutton.f < 100 && guibutton instanceof z) {
/*     */       this.settings.a(((z)guibutton).a(), 1);
/*     */       guibutton.e = this.settings.c(zq.a(guibutton.f));
/*     */     } 
/*     */     if (guibutton.f == 200) {
/*     */       this.p.A.b();
/*     */       this.p.a(this.prevScreen);
/*     */     } 
/*     */     if (guibutton.f != zq.CLOUD_HEIGHT.ordinal()) {
/*     */       agd scaledresolution = new agd(this.p.A, this.p.d, this.p.e);
/*     */       int i = scaledresolution.a();
/*     */       int j = scaledresolution.b();
/*     */       a(this.p, i, j);
/*     */     } 
/*     */   }
/*     */   
/*     */   public void a(int x, int y, float f) {
/*     */     k();
/*     */     a(this.u, this.title, this.q / 2, 20, 16777215);
/*     */     super.a(x, y, f);
/*     */     if (Math.abs(x - this.lastMouseX) > 5 || Math.abs(y - this.lastMouseY) > 5) {
/*     */       this.lastMouseX = x;
/*     */       this.lastMouseY = y;
/*     */       this.mouseStillTime = System.currentTimeMillis();
/*     */       return;
/*     */     } 
/*     */     int activateDelay = 700;
/*     */     if (System.currentTimeMillis() < this.mouseStillTime + activateDelay)
/*     */       return; 
/*     */     int x1 = this.q / 2 - 150;
/*     */     int y1 = this.r / 6 - 5;
/*     */     if (y <= y1 + 98)
/*     */       y1 += 105; 
/*     */     int x2 = x1 + 150 + 150;
/*     */     int y2 = y1 + 84 + 10;
/*     */     abp btn = getSelectedButton(x, y);
/*     */     if (btn != null) {
/*     */       String s = getButtonName(btn.e);
/*     */       String[] lines = getTooltipLines(s);
/*     */       if (lines == null)
/*     */         return; 
/*     */       a(x1, y1, x2, y2, -536870912, -536870912);
/*     */       for (int i = 0; i < lines.length; i++) {
/*     */         String line = lines[i];
/*     */         this.u.a(line, x1 + 5, y1 + 5 + i * 11, 14540253);
/*     */       } 
/*     */     } 
/*     */   }
/*     */   
/*     */   private String[] getTooltipLines(String btnName) {
/*     */     if (btnName.equals("Smooth FPS"))
/*     */       return new String[] { "Stabilizes FPS by flushing the graphic driver buffers", "  OFF - no stabilization, FPS may fluctuate", "  ON - FPS stabilization", "This option is graphic driver dependant and its effect", "is not always visible" }; 
/*     */     if (btnName.equals("Smooth Input"))
/*     */       return new String[] { "Fixes stuck keys, slow input response and sound lag", "  OFF - no fix for stuck keys", "  ON - fixes stuck keys", "This option sets correct thread priorities", "which fixes the stuck keys, slow input and sound lag." }; 
/*     */     if (btnName.equals("Load Far"))
/*     */       return new String[] { "Loads the world chunks at distance Far.", "Switching the render distance does not cause all chunks ", "to be loaded again.", "  OFF - world chunks loaded up to render distance", "  ON - world chunks loaded at distance Far, allows", "       fast render distance switching" }; 
/*     */     if (btnName.equals("Preloaded Chunks"))
/*     */       return new String[] { "Defines an area in which no chunks will be loaded", "  OFF - after 5m new chunks will be loaded", "  2 - after 32m  new chunks will be loaded", "  8 - after 128m new chunks will be loaded", "Higher values need more time to load all the chunks" }; 
/*     */     if (btnName.equals("Chunk Updates"))
/*     */       return new String[] { "Chunk updates per frame", " 1 - (default) slower world loading, higher FPS", " 3 - faster world loading, lower FPS", " 5 - fastest world loading, lowest FPS" }; 
/*     */     if (btnName.equals("Dynamic Updates"))
/*     */       return new String[] { "Dynamic chunk updates", " OFF - (default) standard chunk updates per frame", " ON - more updates while the player is standing still", "Dynamic updates force more chunk updates while", "the player is standing still to load the world faster." }; 
/*     */     return null;
/*     */   }
/*     */   
/*     */   private String getButtonName(String displayString) {
/*     */     int pos = displayString.indexOf(':');
/*     */     if (pos < 0)
/*     */       return displayString; 
/*     */     return displayString.substring(0, pos);
/*     */   }
/*     */   
/*     */   private abp getSelectedButton(int i, int j) {
/*     */     for (int k = 0; k < this.s.size(); k++) {
/*     */       abp btn = this.s.get(k);
/*     */       boolean flag = (i >= btn.c && j >= btn.d && i < btn.c + btn.a && j < btn.d + btn.b);
/*     */       if (flag)
/*     */         return btn; 
/*     */     } 
/*     */     return null;
/*     */   }
/*     */   
/*     */   private int lastMouseX;
/*     */   private int lastMouseY;
/*     */   private long mouseStillTime;
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\GuiPerformanceSettingsOF.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */