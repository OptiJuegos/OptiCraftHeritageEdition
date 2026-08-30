/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ public class GuiAnimationSettingsOF
/*     */   extends vp
/*     */ {
/*     */   private vp prevScreen;
/*     */   protected String title;
/*     */   private hu settings;
/*     */   
/*     */   public GuiAnimationSettingsOF(vp guiscreen, hu gamesettings) {
/*  19 */     this.title = "Animation Settings";
/*  20 */     this.prevScreen = guiscreen;
/*  21 */     this.settings = gamesettings;
/*     */   }
/*     */ 
/*     */   
/*     */   public void c() {
/*  26 */     adn stringtranslate = adn.a();
/*  27 */     int i = 0;
/*  28 */     zq[] aenumoptions = enumOptions;
/*  29 */     int j = aenumoptions.length;
/*  30 */     for (int k = 0; k < j; k++) {
/*     */       
/*  32 */       zq enumoptions = aenumoptions[k];
/*     */       
/*  34 */       int x = this.q / 2 - 155 + i % 2 * 160;
/*  35 */       int y = this.r / 6 + 21 * i / 2 - 10;
/*     */ 
/*     */       
/*  38 */       if (!enumoptions.a()) {
/*     */         
/*  40 */         this.s.add(new z(enumoptions.c(), x, y, enumoptions, this.settings.c(enumoptions)));
/*     */       } else {
/*     */         
/*  43 */         this.s.add(new ajz(enumoptions.c(), x, y, enumoptions, this.settings.c(enumoptions), this.settings.a(enumoptions)));
/*     */       } 
/*  45 */       i++;
/*     */     } 
/*     */ 
/*     */ 
/*     */     
/*  50 */     this.s.add(new abp(210, this.q / 2 - 155, this.r / 6 + 168 + 11, 70, 20, "All ON"));
/*  51 */     this.s.add(new abp(211, this.q / 2 - 155 + 80, this.r / 6 + 168 + 11, 70, 20, "All OFF"));
/*     */     
/*  53 */     this.s.add(new z(200, this.q / 2 + 5, this.r / 6 + 168 + 11, stringtranslate.b("gui.done")));
/*     */   }
/*     */ 
/*     */ 
/*     */   
/*     */   protected void a(abp guibutton) {
/*  59 */     if (!guibutton.h) {
/*     */       return;
/*     */     }
/*     */     
/*  63 */     if (guibutton.f < 100 && guibutton instanceof z) {
/*     */       
/*  65 */       this.settings.a(((z)guibutton).a(), 1);
/*  66 */       guibutton.e = this.settings.c(zq.a(guibutton.f));
/*     */     } 
/*  68 */     if (guibutton.f == 200) {
/*     */       
/*  70 */       this.p.A.b();
/*  71 */       this.p.a(this.prevScreen);
/*     */     } 
/*     */     
/*  74 */     if (guibutton.f == 210)
/*     */     {
/*  76 */       this.p.A.setAllAnimations(true);
/*     */     }
/*     */     
/*  79 */     if (guibutton.f == 211)
/*     */     {
/*  81 */       this.p.A.setAllAnimations(false);
/*     */     }
/*  83 */     if (guibutton.f != zq.CLOUD_HEIGHT.ordinal()) {
/*     */       
/*  85 */       agd scaledresolution = new agd(this.p.A, this.p.d, this.p.e);
/*  86 */       int i = scaledresolution.a();
/*  87 */       int j = scaledresolution.b();
/*  88 */       a(this.p, i, j);
/*     */     } 
/*     */   }
/*     */ 
/*     */   
/*     */   public void a(int i, int j, float f) {
/*  94 */     k();
/*  95 */     a(this.u, this.title, this.q / 2, 20, 16777215);
/*  96 */     super.a(i, j, f);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/* 106 */   private static zq[] enumOptions = new zq[] { zq.ANIMATED_WATER, zq.ANIMATED_LAVA, zq.ANIMATED_FIRE, zq.ANIMATED_PORTAL, zq.ANIMATED_REDSTONE, zq.ANIMATED_EXPLOSION, zq.ANIMATED_FLAME, zq.ANIMATED_SMOKE, zq.VOID_PARTICLES, zq.WATER_PARTICLES, zq.RAIN_SPLASH, zq.PORTAL_PARTICLES, zq.q, zq.DRIPPING_WATER_LAVA, zq.ANIMATED_TERRAIN, zq.ANIMATED_ITEMS, zq.ANIMATED_TEXTURES };
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\GuiAnimationSettingsOF.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */