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
/*     */ public class GuiOtherSettingsOF
/*     */   extends vp
/*     */ {
/*     */   private vp prevScreen;
/*     */   protected String title;
/*     */   private hu settings;
/*     */   
/*     */   public GuiOtherSettingsOF(vp guiscreen, hu gamesettings) {
/* 200 */     this.lastMouseX = 0;
/* 201 */     this.lastMouseY = 0;
/* 202 */     this.mouseStillTime = 0L;
/*     */     this.title = "Other Settings";
/*     */     this.prevScreen = guiscreen;
/*     */     this.settings = gamesettings;
/* 206 */   } private static zq[] enumOptions = new zq[] { zq.FAST_DEBUG_INFO, zq.PROFILER, zq.WEATHER, zq.TIME, zq.FULLSCREEN_MODE, zq.AUTOSAVE_TICKS };
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
/*     */     this.s.add(new abp(210, this.q / 2 - 100, this.r / 6 + 168 + 11 - 22, "Reset Video Settings..."));
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
/*     */     if (guibutton.f == 210) {
/*     */       this.p.A.b();
/*     */       mi guiyesno = new mi(this, "Reset all video settings to their default values?", "", 9999);
/*     */       this.p.a((vp)guiyesno);
/*     */     } 
/*     */     if (guibutton.f != zq.CLOUD_HEIGHT.ordinal()) {
/*     */       agd scaledresolution = new agd(this.p.A, this.p.d, this.p.e);
/*     */       int i = scaledresolution.a();
/*     */       int j = scaledresolution.b();
/*     */       a(this.p, i, j);
/*     */     } 
/*     */   }
/*     */   
/*     */   public void a(boolean flag, int i) {
/*     */     if (flag)
/*     */       this.p.A.resetSettings(); 
/*     */     this.p.a(this);
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
/*     */     if (btnName.equals("Autosave"))
/*     */       return new String[] { "Autosave interval", "Default autosave interval (2s) is NOT RECOMMENDED.", "Autosave causes the famous Lag Spike of Death." }; 
/*     */     if (btnName.equals("Fast Debug Info"))
/*     */       return new String[] { "Fast Debug Info", " OFF - default debug info screen, slower", " ON - debug info screen without lagometer, faster", "Removes the lagometer from the debug screen (F3)." }; 
/*     */     if (btnName.equals("Debug Profiler"))
/*     */       return new String[] { "Debug Profiler", "  ON - debug profiler is active, slower", "  OFF - debug profiler is not active, faster", "The debug profiler collects and shows debug information", "when the debug screen is open" }; 
/*     */     if (btnName.equals("Time"))
/*     */       return new String[] { "Time", " Default - normal day/night cycles", " Day Only - day only", " Night Only - night only", "The time setting is only effective in CREATIVE mode." }; 
/*     */     if (btnName.equals("Weather"))
/*     */       return new String[] { "Weather", "  ON - weather is active, slower", "  OFF - weather is not active, faster", "The weather controls rain, snow and thunderstorms." }; 
/*     */     if (btnName.equals("Fullscreen"))
/*     */       return new String[] { "Fullscreen resolution", "  Default - use desktop screen resolution, slower", "  WxH - use custom screen resolution, may be faster", "The selected resolution is used in fullscreen mode (F11)." }; 
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


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\GuiOtherSettingsOF.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */