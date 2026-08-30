/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ public class GuiDetailSettingsOF
/*     */   extends vp
/*     */ {
/*     */   private vp prevScreen;
/*     */   protected String title;
/*     */   private hu settings;
/*     */   
/*     */   public GuiDetailSettingsOF(vp guiscreen, hu gamesettings) {
/* 207 */     this.lastMouseX = 0;
/* 208 */     this.lastMouseY = 0;
/* 209 */     this.mouseStillTime = 0L;
/*     */     this.title = "Detail Settings";
/*     */     this.prevScreen = guiscreen;
/*     */     this.settings = gamesettings;
/* 213 */   } private static zq[] enumOptions = new zq[] { zq.CLOUDS, zq.CLOUD_HEIGHT, zq.TREES, zq.GRASS, zq.WATER, zq.RAIN, zq.SKY, zq.STARS, zq.SUN_MOON, zq.SHOW_CAPES, zq.DEPTH_FOG };
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
/*     */     if (btnName.equals("Clouds"))
/*     */       return new String[] { "Clouds", "  Default - as set by setting Graphics", "  Fast - lower quality, faster", "  Fancy - higher quality, slower", "  OFF - no clouds, fastest", "Fast clouds are rendered 2D.", "Fancy clouds are rendered 3D." }; 
/*     */     if (btnName.equals("Cloud Height"))
/*     */       return new String[] { "Cloud Height", "  OFF - default height", "  100% - above world height limit" }; 
/*     */     if (btnName.equals("Trees"))
/*     */       return new String[] { "Trees", "  Default - as set by setting Graphics", "  Fast - lower quality, faster", "  Fancy - higher quality, slower", "Fast trees have opaque leaves.", "Fancy trees have transparent leaves." }; 
/*     */     if (btnName.equals("Grass"))
/*     */       return new String[] { "Grass", "  Default - as set by setting Graphics", "  Fast - lower quality, faster", "  Fancy - higher quality, slower", "Fast grass uses default side texture.", "Fancy grass uses biome side texture." }; 
/*     */     if (btnName.equals("Water"))
/*     */       return new String[] { "Water", "  Default - as set by setting Graphics", "  Fast  - lower quality, faster", "  Fancy - higher quality, slower", "Fast water (1 pass) has some visual artifacts", "Fancy water (2 pass) has no visual artifacts" }; 
/*     */     if (btnName.equals("Rain & Snow"))
/*     */       return new String[] { "Rain & Snow", "  Default - as set by setting Graphics", "  Fast  - light rain/snow, faster", "  Fancy - heavy rain/snow, slower", "  OFF - no rain/snow, fastest", "When rain is OFF the splashes and rain sounds", "are still active." }; 
/*     */     if (btnName.equals("Sky"))
/*     */       return new String[] { "Sky", "  ON - sky is visible, slower", "  OFF  - sky is not visible, faster", "When sky is OFF the moon and sun are still visible." }; 
/*     */     if (btnName.equals("Stars"))
/*     */       return new String[] { "Stars", "  ON - stars are visible, slower", "  OFF  - stars are not visible, faster" }; 
/*     */     if (btnName.equals("Depth Fog"))
/*     */       return new String[] { "Depth Fog", "  ON - fog moves closer at bedrock levels (default)", "  OFF - same fog at all levels" }; 
/*     */     if (btnName.equals("Show Capes"))
/*     */       return new String[] { "Show Capes", "  ON - show player capes (default)", "  OFF - do not show player capes" }; 
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


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\GuiDetailSettingsOF.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */