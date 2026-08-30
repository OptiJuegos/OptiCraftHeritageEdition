/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ public class GuiQualitySettingsOF
/*     */   extends vp
/*     */ {
/*     */   private vp prevScreen;
/*     */   protected String title;
/*     */   private hu settings;
/*     */   
/*     */   public GuiQualitySettingsOF(vp guiscreen, hu gamesettings) {
/* 237 */     this.lastMouseX = 0;
/* 238 */     this.lastMouseY = 0;
/* 239 */     this.mouseStillTime = 0L;
/*     */     this.title = "Quality Settings";
/*     */     this.prevScreen = guiscreen;
/*     */     this.settings = gamesettings;
/* 243 */   } private static zq[] enumOptions = new zq[] { zq.MIPMAP_LEVEL, zq.MIPMAP_TYPE, zq.CLEAR_WATER, zq.RANDOM_MOBS, zq.BETTER_GRASS, zq.BETTER_SNOW, zq.CUSTOM_FONTS, zq.CUSTOM_COLORS, zq.SWAMP_COLORS, zq.SMOOTH_BIOMES, zq.CONNECTED_TEXTURES, zq.NATURAL_TEXTURES };
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
/*     */     if (btnName.equals("Mipmap Level"))
/*     */       return new String[] { "Visual effect which makes distant objects look better", "by smoothing the texture details", "  OFF - no smoothing", "  1 - minimum smoothing", "  4 - maximum smoothing", "This option usually does not affect the performance." }; 
/*     */     if (btnName.equals("Mipmap Type"))
/*     */       return new String[] { "Visual effect which makes distant objects look better", "by smoothing the texture details", "  Nearest - rough smoothing", "  Linear - fine smoothing", "This option usually does not affect the performance." }; 
/*     */     if (btnName.equals("Clear Water"))
/*     */       return new String[] { "Clear Water", "  ON - clear, transparent water", "  OFF - default water" }; 
/*     */     if (btnName.equals("Better Grass"))
/*     */       return new String[] { "Better Grass", "  OFF - default side grass texture, fastest", "  Fast - full side grass texture, slower", "  Fancy - dynamic side grass texture, slowest" }; 
/*     */     if (btnName.equals("Better Snow"))
/*     */       return new String[] { "Better Snow", "  OFF - default snow, faster", "  ON - better snow, slower", "Shows snow under transparent blocks (fence, tall grass)", "when bordering with snow blocks" }; 
/*     */     if (btnName.equals("Random Mobs"))
/*     */       return new String[] { "Random Mobs", "  OFF - no random mobs, faster", "  ON - random mobs, slower", "Random mobs uses random textures for the game creatures.", "It needs a texture pack which has multiple mob textures." }; 
/*     */     if (btnName.equals("Swamp Colors"))
/*     */       return new String[] { "Swamp Colors", "  ON - use swamp colors (default), slower", "  OFF - do not use swamp colors, faster", "The swamp colors affect grass, leaves, vines and water." }; 
/*     */     if (btnName.equals("Smooth Biomes"))
/*     */       return new String[] { "Smooth Biomes", "  ON - smoothing of biome borders (default), slower", "  OFF - no smoothing of biome borders, faster", "The smoothing of biome borders is done by sampling and", "averaging the color of all surounding blocks.", "Affected are grass, leaves, vines and water." }; 
/*     */     if (btnName.equals("Custom Fonts"))
/*     */       return new String[] { "Custom Fonts", "  ON - uses custom fonts (default), slower", "  OFF - uses default font, faster", "The custom fonts are supplied by the current", "texture pack" }; 
/*     */     if (btnName.equals("Custom Colors"))
/*     */       return new String[] { "Custom Colors", "  ON - uses custom colors (default), slower", "  OFF - uses default colors, faster", "The custom colors are supplied by the current", "texture pack" }; 
/*     */     if (btnName.equals("Show Capes"))
/*     */       return new String[] { "Show Capes", "  ON - show player capes (default)", "  OFF - do not show player capes" }; 
/*     */     if (btnName.equals("Connected Textures"))
/*     */       return new String[] { "Connected Textures", "  OFF - no connected textures (default)", "  Fast - fast connected textures", "  Fancy - fancy connected textures", "Connected textures joins the textures of glass,", "sandstone and bookshelves when placed next to", "each other. The connected textures are supplied", "by the current texture pack." }; 
/*     */     if (btnName.equals("Far View"))
/*     */       return new String[] { "Far View", " OFF - (default) standard view distance", " ON - 3x view distance", "Far View is very resource demanding!", "3x view distance => 9x chunks to be loaded => FPS / 9", "Standard view distances: 32, 64, 128, 256", "Far view distances: 96, 192, 384, 512" }; 
/*     */     if (btnName.equals("Natural Textures"))
/*     */       return new String[] { "Natural Textures", "  OFF - no natural textures (default)", "  ON - use natural textures", "Natural textures remove the gridlike pattern", "created by repeating blocks of the same type.", "It uses rotated and flipped variants of the base", "block texture. The configuration for the natural", "textures is supplied by the current texture pack" }; 
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


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\GuiQualitySettingsOF.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */