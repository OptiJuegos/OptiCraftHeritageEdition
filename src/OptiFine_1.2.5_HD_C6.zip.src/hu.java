/*      */ import java.io.BufferedReader;
/*      */ import java.io.File;
/*      */ import java.io.FileReader;
/*      */ import java.io.FileWriter;
/*      */ import java.io.PrintWriter;
/*      */ import java.util.Arrays;
/*      */ import java.util.List;
/*      */ import net.minecraft.client.Minecraft;
/*      */ import org.lwjgl.input.Keyboard;
/*      */ import org.lwjgl.opengl.Display;
/*      */ 
/*      */ public class hu {
/*   13 */   private static final String[] S = new String[] { "options.renderDistance.far", "options.renderDistance.normal", "options.renderDistance.short", "options.renderDistance.tiny" };
/*      */ 
/*      */   
/*   16 */   private static final String[] T = new String[] { "options.difficulty.peaceful", "options.difficulty.easy", "options.difficulty.normal", "options.difficulty.hard" };
/*      */ 
/*      */   
/*   19 */   private static final String[] U = new String[] { "options.guiScale.auto", "options.guiScale.small", "options.guiScale.normal", "options.guiScale.large" };
/*      */ 
/*      */   
/*   22 */   private static final String[] V = new String[] { "options.particles.all", "options.particles.decreased", "options.particles.minimal" };
/*      */ 
/*      */   
/*   25 */   private static final String[] W = new String[] { "performance.max", "performance.balanced", "performance.powersaver" };
/*      */   
/*      */   public float a;
/*      */   
/*      */   public float b;
/*      */   
/*      */   public float c;
/*      */   
/*      */   public boolean d;
/*      */   
/*      */   public int e;
/*      */   
/*      */   public boolean f;
/*      */   
/*      */   public boolean g;
/*      */   
/*      */   public boolean h;
/*      */   
/*      */   public int i;
/*      */   
/*      */   public boolean j;
/*      */   public boolean k;
/*      */   public boolean l;
/*   48 */   public int ofRenderDistanceFine = 128;
/*   49 */   public int ofFogType = 1;
/*   50 */   public float ofFogStart = 0.8F;
/*   51 */   public int ofMipmapLevel = 0;
/*      */   public boolean ofMipmapLinear = false;
/*      */   public boolean ofLoadFar = false;
/*   54 */   public int ofPreloadedChunks = 0;
/*      */   public boolean ofOcclusionFancy = false;
/*      */   public boolean ofSmoothFps = false;
/*      */   public boolean ofSmoothInput = true;
/*   58 */   public float ofAoLevel = 1.0F;
/*   59 */   public int ofAaLevel = 0;
/*   60 */   public int ofAfLevel = 1;
/*      */   
/*   62 */   public int ofClouds = 0;
/*   63 */   public float ofCloudsHeight = 0.0F;
/*   64 */   public int ofTrees = 0;
/*   65 */   public int ofGrass = 0;
/*   66 */   public int ofRain = 0;
/*   67 */   public int ofWater = 0;
/*   68 */   public int ofBetterGrass = 3;
/*   69 */   public int ofAutoSaveTicks = 4000;
/*      */   public boolean ofFastDebugInfo = false;
/*      */   public boolean ofWeather = true;
/*      */   public boolean ofSky = true;
/*      */   public boolean ofStars = true;
/*      */   public boolean ofSunMoon = true;
/*   75 */   public int ofChunkUpdates = 1;
/*      */   public boolean ofChunkUpdatesDynamic = false;
/*   77 */   public int ofTime = 0;
/*      */   public boolean ofClearWater = false;
/*      */   public boolean ofDepthFog = true;
/*      */   public boolean ofProfiler = false;
/*      */   public boolean ofBetterSnow = false;
/*   82 */   public String ofFullscreenMode = "Default";
/*      */   public boolean ofSwampColors = true;
/*      */   public boolean ofRandomMobs = true;
/*      */   public boolean ofSmoothBiomes = true;
/*      */   public boolean ofCustomFonts = true;
/*      */   public boolean ofCustomColors = true;
/*      */   public boolean ofShowCapes = true;
/*   89 */   public int ofConnectedTextures = 2;
/*      */   
/*      */   public boolean ofNaturalTextures = false;
/*   92 */   public int ofAnimatedWater = 0;
/*   93 */   public int ofAnimatedLava = 0;
/*      */   
/*      */   public boolean ofAnimatedFire = true;
/*      */   
/*      */   public boolean ofAnimatedPortal = true;
/*      */   
/*      */   public boolean ofAnimatedRedstone = true;
/*      */   
/*      */   public boolean ofAnimatedExplosion = true;
/*      */   
/*      */   public boolean ofAnimatedFlame = true;
/*      */   
/*      */   public boolean ofAnimatedSmoke = true;
/*      */   
/*      */   public boolean ofVoidParticles = true;
/*      */   
/*      */   public boolean ofWaterParticles = true;
/*      */   
/*      */   public boolean ofRainSplash = true;
/*      */   
/*      */   public boolean ofPortalParticles = true;
/*      */   
/*      */   public boolean ofDrippingWaterLava = true;
/*      */   
/*      */   public boolean ofAnimatedTerrain = true;
/*      */   
/*      */   public boolean ofAnimatedItems = true;
/*      */   
/*      */   public boolean ofAnimatedTextures = true;
/*      */   
/*      */   public static final int DEFAULT = 0;
/*      */   
/*      */   public static final int FAST = 1;
/*      */   
/*      */   public static final int FANCY = 2;
/*      */   
/*      */   public static final int OFF = 3;
/*      */   
/*      */   public static final int ANIM_ON = 0;
/*      */   
/*      */   public static final int ANIM_GENERATED = 1;
/*      */   
/*      */   public static final int ANIM_OFF = 2;
/*      */   
/*      */   public static final String DEFAULT_STR = "Default";
/*      */   
/*      */   public afu ofKeyBindZoom;
/*      */   
/*      */   public String m;
/*      */   
/*      */   public afu n;
/*      */   
/*      */   public afu o;
/*      */   
/*      */   public afu p;
/*      */   public afu q;
/*      */   public afu r;
/*      */   public afu s;
/*      */   public afu t;
/*      */   public afu u;
/*      */   public afu v;
/*      */   public afu w;
/*      */   public afu x;
/*      */   public afu y;
/*      */   public afu z;
/*      */   public afu[] A;
/*      */   protected Minecraft B;
/*      */   private File X;
/*      */   public int C;
/*      */   public boolean D;
/*      */   public int E;
/*      */   public boolean F;
/*      */   public boolean G;
/*      */   public String H;
/*      */   public boolean I;
/*      */   public boolean J;
/*      */   public boolean K;
/*      */   public float L;
/*      */   public float M;
/*      */   public float N;
/*      */   public float O;
/*      */   public int P;
/*      */   public int Q;
/*      */   public String R;
/*      */   private File optionsFileOF;
/*      */   
/*      */   public hu(Minecraft par1Minecraft, File par2File) {
/*  180 */     this.a = 1.0F;
/*  181 */     this.b = 1.0F;
/*  182 */     this.c = 0.5F;
/*  183 */     this.d = false;
/*  184 */     this.e = 0;
/*  185 */     this.f = true;
/*  186 */     this.g = false;
/*  187 */     this.h = false;
/*  188 */     this.i = 1;
/*  189 */     this.j = true;
/*  190 */     this.k = true;
/*  191 */     this.l = true;
/*  192 */     this.m = "Default";
/*      */     
/*  194 */     this.e = 1;
/*  195 */     this.i = 0;
/*      */     
/*  197 */     this.n = new afu("key.forward", 17);
/*  198 */     this.o = new afu("key.left", 30);
/*  199 */     this.p = new afu("key.back", 31);
/*  200 */     this.q = new afu("key.right", 32);
/*  201 */     this.r = new afu("key.jump", 57);
/*  202 */     this.s = new afu("key.inventory", 18);
/*  203 */     this.t = new afu("key.drop", 16);
/*  204 */     this.u = new afu("key.chat", 20);
/*  205 */     this.v = new afu("key.sneak", 42);
/*  206 */     this.w = new afu("key.attack", -100);
/*  207 */     this.x = new afu("key.use", -99);
/*  208 */     this.y = new afu("key.playerlist", 15);
/*  209 */     this.z = new afu("key.pickItem", -98);
/*      */     
/*  211 */     this.ofKeyBindZoom = new afu("Zoom", 29);
/*  212 */     this.A = new afu[] { this.w, this.x, this.n, this.o, this.p, this.q, this.r, this.v, this.t, this.s, this.u, this.y, this.z, this.ofKeyBindZoom };
/*      */ 
/*      */ 
/*      */     
/*  216 */     this.C = 2;
/*  217 */     this.D = false;
/*  218 */     this.E = 0;
/*  219 */     this.F = false;
/*  220 */     this.G = false;
/*  221 */     this.H = "";
/*  222 */     this.I = false;
/*  223 */     this.J = false;
/*  224 */     this.K = false;
/*  225 */     this.L = 1.0F;
/*  226 */     this.M = 1.0F;
/*  227 */     this.N = 0.0F;
/*  228 */     this.O = 0.0F;
/*  229 */     this.P = 0;
/*  230 */     this.Q = 0;
/*  231 */     this.R = "en_US";
/*  232 */     this.B = par1Minecraft;
/*  233 */     this.X = new File(par2File, "options.txt");
/*  234 */     this.optionsFileOF = new File(par2File, "optionsof.txt");
/*  235 */     a();
/*      */     
/*  237 */     Config.setGameSettings(this);
/*      */   }
/*      */ 
/*      */   
/*      */   public hu() {
/*  242 */     this.a = 1.0F;
/*  243 */     this.b = 1.0F;
/*  244 */     this.c = 0.5F;
/*  245 */     this.d = false;
/*  246 */     this.e = 0;
/*  247 */     this.f = true;
/*  248 */     this.g = false;
/*  249 */     this.h = false;
/*  250 */     this.i = 1;
/*  251 */     this.j = true;
/*  252 */     this.k = true;
/*  253 */     this.l = true;
/*  254 */     this.m = "Default";
/*      */     
/*  256 */     this.e = 1;
/*  257 */     this.i = 0;
/*      */     
/*  259 */     this.n = new afu("key.forward", 17);
/*  260 */     this.o = new afu("key.left", 30);
/*  261 */     this.p = new afu("key.back", 31);
/*  262 */     this.q = new afu("key.right", 32);
/*  263 */     this.r = new afu("key.jump", 57);
/*  264 */     this.s = new afu("key.inventory", 18);
/*  265 */     this.t = new afu("key.drop", 16);
/*  266 */     this.u = new afu("key.chat", 20);
/*  267 */     this.v = new afu("key.sneak", 42);
/*  268 */     this.w = new afu("key.attack", -100);
/*  269 */     this.x = new afu("key.use", -99);
/*  270 */     this.y = new afu("key.playerlist", 15);
/*  271 */     this.z = new afu("key.pickItem", -98);
/*  272 */     this.A = new afu[] { this.w, this.x, this.n, this.o, this.p, this.q, this.r, this.v, this.t, this.s, this.u, this.y, this.z };
/*      */ 
/*      */ 
/*      */     
/*  276 */     this.C = 2;
/*  277 */     this.D = false;
/*  278 */     this.E = 0;
/*  279 */     this.F = false;
/*  280 */     this.G = false;
/*  281 */     this.H = "";
/*  282 */     this.I = false;
/*  283 */     this.J = false;
/*  284 */     this.K = false;
/*  285 */     this.L = 1.0F;
/*  286 */     this.M = 1.0F;
/*  287 */     this.N = 0.0F;
/*  288 */     this.O = 0.0F;
/*  289 */     this.P = 0;
/*  290 */     this.Q = 0;
/*  291 */     this.R = "en_US";
/*      */     
/*  293 */     Config.setGameSettings(this);
/*      */   }
/*      */ 
/*      */   
/*      */   public String a(int par1) {
/*  298 */     adn stringtranslate = adn.a();
/*  299 */     return stringtranslate.b((this.A[par1]).c);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public String b(int par1) {
/*  307 */     int i = (this.A[par1]).d;
/*  308 */     return c(i);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static String c(int par0) {
/*  316 */     if (par0 < 0)
/*      */     {
/*  318 */       return cy.a("key.mouseButton", new Object[] { Integer.valueOf(par0 + 101) });
/*      */     }
/*      */ 
/*      */ 
/*      */     
/*  323 */     return Keyboard.getKeyName(par0);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(int par1, int par2) {
/*  332 */     (this.A[par1]).d = par2;
/*  333 */     b();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(zq par1EnumOptions, float par2) {
/*  341 */     if (par1EnumOptions == zq.a) {
/*      */       
/*  343 */       this.a = par2;
/*  344 */       this.B.C.a();
/*      */     } 
/*  346 */     if (par1EnumOptions == zq.b) {
/*      */       
/*  348 */       this.b = par2;
/*  349 */       this.B.C.a();
/*      */     } 
/*  351 */     if (par1EnumOptions == zq.d)
/*      */     {
/*  353 */       this.c = par2;
/*      */     }
/*  355 */     if (par1EnumOptions == zq.e)
/*      */     {
/*  357 */       this.N = par2;
/*      */     }
/*  359 */     if (par1EnumOptions == zq.f)
/*      */     {
/*  361 */       this.O = par2;
/*      */     }
/*      */     
/*  364 */     if (par1EnumOptions == zq.CLOUD_HEIGHT)
/*      */     {
/*  366 */       this.ofCloudsHeight = par2;
/*      */     }
/*  368 */     if (par1EnumOptions == zq.AO_LEVEL) {
/*      */       
/*  370 */       this.ofAoLevel = par2;
/*  371 */       this.k = (this.ofAoLevel > 0.0F);
/*  372 */       this.B.g.a();
/*      */     } 
/*  374 */     if (par1EnumOptions == zq.RENDER_DISTANCE_FINE) {
/*      */       
/*  376 */       this.ofRenderDistanceFine = 32 + (int)(par2 * 480.0F);
/*  377 */       this.ofRenderDistanceFine = this.ofRenderDistanceFine >> 4 << 4;
/*  378 */       this.ofRenderDistanceFine = Config.limit(this.ofRenderDistanceFine, 32, 512);
/*  379 */       this.e = 3;
/*  380 */       if (this.ofRenderDistanceFine > 32)
/*  381 */         this.e = 2; 
/*  382 */       if (this.ofRenderDistanceFine > 64)
/*  383 */         this.e = 1; 
/*  384 */       if (this.ofRenderDistanceFine > 128)
/*  385 */         this.e = 0; 
/*  386 */       this.B.g.a();
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void updateWaterOpacity() {
/*  397 */     int opacity = 3;
/*  398 */     if (this.ofClearWater) {
/*  399 */       opacity = 1;
/*      */     }
/*  401 */     pb.B.f(opacity);
/*  402 */     pb.A.f(opacity);
/*      */     
/*  404 */     if (this.B.f == null)
/*      */       return; 
/*  406 */     ca cp = this.B.f.v;
/*  407 */     if (cp == null) {
/*      */       return;
/*      */     }
/*  410 */     for (int x = -512; x < 512; x++) {
/*      */       
/*  412 */       for (int z = -512; z < 512; z++) {
/*      */         
/*  414 */         if (cp.a(x, z)) {
/*      */ 
/*      */           
/*  417 */           ack c = cp.b(x, z);
/*  418 */           if (c != null && !(c instanceof ace)) {
/*      */ 
/*      */ 
/*      */             
/*  422 */             zg[] ebss = c.i();
/*  423 */             for (int i = 0; i < ebss.length; i++) {
/*      */               
/*  425 */               zg ebs = ebss[i];
/*  426 */               if (ebs != null) {
/*      */                 
/*  428 */                 qq na = ebs.l();
/*  429 */                 if (na != null) {
/*      */ 
/*      */                   
/*  432 */                   byte[] data = na.a;
/*  433 */                   for (int d = 0; d < data.length; d++)
/*  434 */                     data[d] = 0; 
/*      */                 } 
/*      */               } 
/*  437 */             }  c.b();
/*      */           } 
/*      */         } 
/*      */       } 
/*  441 */     }  this.B.g.a();
/*      */   }
/*      */ 
/*      */   
/*      */   public void setAllAnimations(boolean flag) {
/*  446 */     int animVal = flag ? 0 : 2;
/*      */     
/*  448 */     this.ofAnimatedWater = animVal;
/*  449 */     this.ofAnimatedLava = animVal;
/*  450 */     this.ofAnimatedFire = flag;
/*  451 */     this.ofAnimatedPortal = flag;
/*  452 */     this.ofAnimatedRedstone = flag;
/*  453 */     this.ofAnimatedExplosion = flag;
/*  454 */     this.ofAnimatedFlame = flag;
/*  455 */     this.ofAnimatedSmoke = flag;
/*  456 */     this.ofVoidParticles = flag;
/*  457 */     this.ofWaterParticles = flag;
/*  458 */     this.ofRainSplash = flag;
/*  459 */     this.ofPortalParticles = flag;
/*  460 */     this.Q = flag ? 0 : 2;
/*  461 */     this.ofDrippingWaterLava = flag;
/*  462 */     this.ofAnimatedTerrain = flag;
/*  463 */     this.ofAnimatedItems = flag;
/*  464 */     this.ofAnimatedTextures = flag;
/*      */     
/*  466 */     this.B.p.b();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(zq par1EnumOptions, int par2) {
/*  474 */     if (par1EnumOptions == zq.c)
/*      */     {
/*  476 */       this.d = !this.d;
/*      */     }
/*  478 */     if (par1EnumOptions == zq.g) {
/*      */       
/*  480 */       this.e = this.e + par2 & 0x3;
/*      */       
/*  482 */       this.ofRenderDistanceFine = 32 << 3 - this.e;
/*      */     } 
/*  484 */     if (par1EnumOptions == zq.o)
/*      */     {
/*  486 */       this.P = this.P + par2 & 0x3;
/*      */     }
/*  488 */     if (par1EnumOptions == zq.q)
/*      */     {
/*  490 */       this.Q = (this.Q + par2) % 3;
/*      */     }
/*  492 */     if (par1EnumOptions == zq.h)
/*      */     {
/*  494 */       this.f = !this.f;
/*      */     }
/*  496 */     if (par1EnumOptions == zq.p)
/*      */     {
/*  498 */       this.l = !this.l;
/*      */     }
/*  500 */     if (par1EnumOptions == zq.j) {
/*      */ 
/*      */       
/*  503 */       if (!Config.isOcclusionAvailable()) {
/*      */         
/*  505 */         this.ofOcclusionFancy = false;
/*  506 */         this.h = false;
/*      */ 
/*      */ 
/*      */       
/*      */       }
/*  511 */       else if (!this.h) {
/*      */         
/*  513 */         this.h = true;
/*  514 */         this.ofOcclusionFancy = false;
/*      */ 
/*      */       
/*      */       }
/*  518 */       else if (!this.ofOcclusionFancy) {
/*  519 */         this.ofOcclusionFancy = true;
/*      */       } else {
/*      */         
/*  522 */         this.ofOcclusionFancy = false;
/*  523 */         this.h = false;
/*      */       } 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */       
/*  530 */       this.B.g.setAllRenderersVisible();
/*      */     } 
/*  532 */     if (par1EnumOptions == zq.i) {
/*      */       
/*  534 */       this.g = !this.g;
/*  535 */       this.B.p.b();
/*      */     } 
/*  537 */     if (par1EnumOptions == zq.k) {
/*      */       
/*  539 */       this.i = (this.i + par2) % 4;
/*      */       
/*  541 */       Display.setVSyncEnabled((this.i == 3));
/*      */     } 
/*  543 */     if (par1EnumOptions == zq.l)
/*      */     {
/*  545 */       this.C = this.C + par2 & 0x3;
/*      */     }
/*  547 */     if (par1EnumOptions == zq.m) {
/*      */       
/*  549 */       this.j = !this.j;
/*  550 */       this.B.g.a();
/*      */     } 
/*  552 */     if (par1EnumOptions == zq.n) {
/*      */       
/*  554 */       this.k = !this.k;
/*  555 */       this.B.g.a();
/*      */     } 
/*      */     
/*  558 */     if (par1EnumOptions == zq.FOG_FANCY)
/*      */     {
/*  560 */       switch (this.ofFogType) {
/*      */         
/*      */         case 1:
/*  563 */           this.ofFogType = 2;
/*      */           
/*  565 */           if (!Config.isFancyFogAvailable())
/*  566 */             this.ofFogType = 3; 
/*      */           break;
/*      */         case 2:
/*  569 */           this.ofFogType = 3;
/*      */           break;
/*      */         case 3:
/*  572 */           this.ofFogType = 1;
/*      */           break;
/*      */         default:
/*  575 */           this.ofFogType = 1;
/*      */           break;
/*      */       }  } 
/*  578 */     if (par1EnumOptions == zq.FOG_START) {
/*      */       
/*  580 */       this.ofFogStart += 0.2F;
/*  581 */       if (this.ofFogStart > 0.81F)
/*  582 */         this.ofFogStart = 0.2F; 
/*      */     } 
/*  584 */     if (par1EnumOptions == zq.MIPMAP_LEVEL) {
/*      */       
/*  586 */       this.ofMipmapLevel++;
/*  587 */       if (this.ofMipmapLevel > 4) {
/*  588 */         this.ofMipmapLevel = 0;
/*      */       }
/*  590 */       this.B.p.b();
/*      */     } 
/*      */ 
/*      */     
/*  594 */     if (par1EnumOptions == zq.MIPMAP_TYPE) {
/*      */       
/*  596 */       this.ofMipmapLinear = !this.ofMipmapLinear;
/*      */       
/*  598 */       this.B.p.b();
/*      */     } 
/*      */ 
/*      */     
/*  602 */     if (par1EnumOptions == zq.LOAD_FAR) {
/*      */       
/*  604 */       this.ofLoadFar = !this.ofLoadFar;
/*      */       
/*  606 */       this.B.g.a();
/*      */     } 
/*  608 */     if (par1EnumOptions == zq.PRELOADED_CHUNKS) {
/*      */       
/*  610 */       this.ofPreloadedChunks += 2;
/*  611 */       if (this.ofPreloadedChunks > 8) {
/*  612 */         this.ofPreloadedChunks = 0;
/*      */       }
/*  614 */       this.B.g.a();
/*      */     } 
/*  616 */     if (par1EnumOptions == zq.SMOOTH_FPS)
/*      */     {
/*  618 */       this.ofSmoothFps = !this.ofSmoothFps;
/*      */     }
/*  620 */     if (par1EnumOptions == zq.SMOOTH_INPUT)
/*      */     {
/*  622 */       this.ofSmoothInput = !this.ofSmoothInput;
/*      */     }
/*  624 */     if (par1EnumOptions == zq.CLOUDS) {
/*      */       
/*  626 */       this.ofClouds++;
/*  627 */       if (this.ofClouds > 3)
/*  628 */         this.ofClouds = 0; 
/*      */     } 
/*  630 */     if (par1EnumOptions == zq.TREES) {
/*      */       
/*  632 */       this.ofTrees++;
/*  633 */       if (this.ofTrees > 2)
/*  634 */         this.ofTrees = 0; 
/*  635 */       this.B.g.a();
/*      */     } 
/*  637 */     if (par1EnumOptions == zq.GRASS) {
/*      */       
/*  639 */       this.ofGrass++;
/*  640 */       if (this.ofGrass > 2)
/*  641 */         this.ofGrass = 0; 
/*  642 */       vl.b = Config.isGrassFancy();
/*  643 */       this.B.g.a();
/*      */     } 
/*  645 */     if (par1EnumOptions == zq.RAIN) {
/*      */       
/*  647 */       this.ofRain++;
/*  648 */       if (this.ofRain > 3)
/*  649 */         this.ofRain = 0; 
/*      */     } 
/*  651 */     if (par1EnumOptions == zq.WATER) {
/*      */       
/*  653 */       this.ofWater++;
/*  654 */       if (this.ofWater > 2)
/*  655 */         this.ofWater = 0; 
/*      */     } 
/*  657 */     if (par1EnumOptions == zq.ANIMATED_WATER) {
/*      */       
/*  659 */       this.ofAnimatedWater++;
/*  660 */       if (this.ofAnimatedWater > 2)
/*  661 */         this.ofAnimatedWater = 0; 
/*  662 */       this.B.p.b();
/*      */     } 
/*  664 */     if (par1EnumOptions == zq.ANIMATED_LAVA) {
/*      */       
/*  666 */       this.ofAnimatedLava++;
/*  667 */       if (this.ofAnimatedLava > 2)
/*  668 */         this.ofAnimatedLava = 0; 
/*  669 */       this.B.p.b();
/*      */     } 
/*  671 */     if (par1EnumOptions == zq.ANIMATED_FIRE) {
/*      */       
/*  673 */       this.ofAnimatedFire = !this.ofAnimatedFire;
/*  674 */       this.B.p.b();
/*      */     } 
/*  676 */     if (par1EnumOptions == zq.ANIMATED_PORTAL) {
/*      */       
/*  678 */       this.ofAnimatedPortal = !this.ofAnimatedPortal;
/*  679 */       this.B.p.b();
/*      */     } 
/*  681 */     if (par1EnumOptions == zq.ANIMATED_REDSTONE)
/*      */     {
/*  683 */       this.ofAnimatedRedstone = !this.ofAnimatedRedstone;
/*      */     }
/*  685 */     if (par1EnumOptions == zq.ANIMATED_EXPLOSION)
/*      */     {
/*  687 */       this.ofAnimatedExplosion = !this.ofAnimatedExplosion;
/*      */     }
/*  689 */     if (par1EnumOptions == zq.ANIMATED_FLAME)
/*      */     {
/*  691 */       this.ofAnimatedFlame = !this.ofAnimatedFlame;
/*      */     }
/*  693 */     if (par1EnumOptions == zq.ANIMATED_SMOKE)
/*      */     {
/*  695 */       this.ofAnimatedSmoke = !this.ofAnimatedSmoke;
/*      */     }
/*  697 */     if (par1EnumOptions == zq.VOID_PARTICLES)
/*      */     {
/*  699 */       this.ofVoidParticles = !this.ofVoidParticles;
/*      */     }
/*  701 */     if (par1EnumOptions == zq.WATER_PARTICLES)
/*      */     {
/*  703 */       this.ofWaterParticles = !this.ofWaterParticles;
/*      */     }
/*  705 */     if (par1EnumOptions == zq.PORTAL_PARTICLES)
/*      */     {
/*  707 */       this.ofPortalParticles = !this.ofPortalParticles;
/*      */     }
/*  709 */     if (par1EnumOptions == zq.DRIPPING_WATER_LAVA)
/*      */     {
/*  711 */       this.ofDrippingWaterLava = !this.ofDrippingWaterLava;
/*      */     }
/*  713 */     if (par1EnumOptions == zq.ANIMATED_TERRAIN) {
/*      */       
/*  715 */       this.ofAnimatedTerrain = !this.ofAnimatedTerrain;
/*  716 */       this.B.p.b();
/*      */     } 
/*  718 */     if (par1EnumOptions == zq.ANIMATED_TEXTURES)
/*      */     {
/*  720 */       this.ofAnimatedTextures = !this.ofAnimatedTextures;
/*      */     }
/*      */     
/*  723 */     if (par1EnumOptions == zq.ANIMATED_ITEMS) {
/*      */       
/*  725 */       this.ofAnimatedItems = !this.ofAnimatedItems;
/*  726 */       this.B.p.b();
/*      */     } 
/*  728 */     if (par1EnumOptions == zq.RAIN_SPLASH)
/*      */     {
/*  730 */       this.ofRainSplash = !this.ofRainSplash;
/*      */     }
/*  732 */     if (par1EnumOptions == zq.FAST_DEBUG_INFO)
/*      */     {
/*  734 */       this.ofFastDebugInfo = !this.ofFastDebugInfo;
/*      */     }
/*  736 */     if (par1EnumOptions == zq.AUTOSAVE_TICKS) {
/*      */       
/*  738 */       this.ofAutoSaveTicks *= 10;
/*  739 */       if (this.ofAutoSaveTicks > 40000)
/*  740 */         this.ofAutoSaveTicks = 40; 
/*      */     } 
/*  742 */     if (par1EnumOptions == zq.BETTER_GRASS) {
/*      */       
/*  744 */       this.ofBetterGrass++;
/*  745 */       if (this.ofBetterGrass > 3)
/*  746 */         this.ofBetterGrass = 1; 
/*  747 */       this.B.g.a();
/*      */     } 
/*  749 */     if (par1EnumOptions == zq.CONNECTED_TEXTURES) {
/*      */       
/*  751 */       this.ofConnectedTextures++;
/*  752 */       if (this.ofConnectedTextures > 3)
/*  753 */         this.ofConnectedTextures = 1; 
/*  754 */       this.B.g.a();
/*      */     } 
/*  756 */     if (par1EnumOptions == zq.WEATHER)
/*      */     {
/*  758 */       this.ofWeather = !this.ofWeather;
/*      */     }
/*  760 */     if (par1EnumOptions == zq.SKY)
/*      */     {
/*  762 */       this.ofSky = !this.ofSky;
/*      */     }
/*  764 */     if (par1EnumOptions == zq.STARS)
/*      */     {
/*  766 */       this.ofStars = !this.ofStars;
/*      */     }
/*  768 */     if (par1EnumOptions == zq.SUN_MOON)
/*      */     {
/*  770 */       this.ofSunMoon = !this.ofSunMoon;
/*      */     }
/*  772 */     if (par1EnumOptions == zq.CHUNK_UPDATES) {
/*      */       
/*  774 */       this.ofChunkUpdates++;
/*  775 */       if (this.ofChunkUpdates > 5)
/*  776 */         this.ofChunkUpdates = 1; 
/*      */     } 
/*  778 */     if (par1EnumOptions == zq.CHUNK_UPDATES_DYNAMIC)
/*      */     {
/*  780 */       this.ofChunkUpdatesDynamic = !this.ofChunkUpdatesDynamic;
/*      */     }
/*  782 */     if (par1EnumOptions == zq.TIME) {
/*      */       
/*  784 */       this.ofTime++;
/*  785 */       if (this.ofTime > 3)
/*  786 */         this.ofTime = 0; 
/*      */     } 
/*  788 */     if (par1EnumOptions == zq.CLEAR_WATER) {
/*      */       
/*  790 */       this.ofClearWater = !this.ofClearWater;
/*  791 */       updateWaterOpacity();
/*      */     } 
/*  793 */     if (par1EnumOptions == zq.DEPTH_FOG)
/*      */     {
/*  795 */       this.ofDepthFog = !this.ofDepthFog;
/*      */     }
/*  797 */     if (par1EnumOptions == zq.PROFILER)
/*      */     {
/*  799 */       this.ofProfiler = !this.ofProfiler;
/*      */     }
/*  801 */     if (par1EnumOptions == zq.BETTER_SNOW) {
/*      */       
/*  803 */       this.ofBetterSnow = !this.ofBetterSnow;
/*  804 */       this.B.g.a();
/*      */     } 
/*  806 */     if (par1EnumOptions == zq.SWAMP_COLORS) {
/*      */       
/*  808 */       this.ofSwampColors = !this.ofSwampColors;
/*  809 */       this.B.g.a();
/*      */     } 
/*  811 */     if (par1EnumOptions == zq.RANDOM_MOBS) {
/*      */       
/*  813 */       this.ofRandomMobs = !this.ofRandomMobs;
/*  814 */       this.B.p.b();
/*      */     } 
/*  816 */     if (par1EnumOptions == zq.SMOOTH_BIOMES) {
/*      */       
/*  818 */       this.ofSmoothBiomes = !this.ofSmoothBiomes;
/*  819 */       this.B.g.a();
/*      */     } 
/*  821 */     if (par1EnumOptions == zq.CUSTOM_FONTS) {
/*      */       
/*  823 */       this.ofCustomFonts = !this.ofCustomFonts;
/*  824 */       this.B.p.b();
/*      */     } 
/*  826 */     if (par1EnumOptions == zq.CUSTOM_COLORS) {
/*      */       
/*  828 */       this.ofCustomColors = !this.ofCustomColors;
/*  829 */       this.B.p.b();
/*  830 */       this.B.g.a();
/*      */     } 
/*  832 */     if (par1EnumOptions == zq.SHOW_CAPES) {
/*      */       
/*  834 */       this.ofShowCapes = !this.ofShowCapes;
/*  835 */       this.B.g.updateCapes();
/*      */     } 
/*  837 */     if (par1EnumOptions == zq.NATURAL_TEXTURES) {
/*      */       
/*  839 */       this.ofNaturalTextures = !this.ofNaturalTextures;
/*  840 */       this.B.p.b();
/*  841 */       this.B.g.a();
/*      */     } 
/*  843 */     if (par1EnumOptions == zq.FULLSCREEN_MODE) {
/*      */       
/*  845 */       List<String> modeList = Arrays.asList(Config.getFullscreenModes());
/*  846 */       if (this.ofFullscreenMode.equals("Default")) {
/*      */         
/*  848 */         this.ofFullscreenMode = modeList.get(0);
/*      */       }
/*      */       else {
/*      */         
/*  852 */         int index = modeList.indexOf(this.ofFullscreenMode);
/*  853 */         if (index < 0) {
/*      */           
/*  855 */           this.ofFullscreenMode = "Default";
/*      */         }
/*      */         else {
/*      */           
/*  859 */           index++;
/*  860 */           if (index >= modeList.size()) {
/*  861 */             this.ofFullscreenMode = "Default";
/*      */           } else {
/*  863 */             this.ofFullscreenMode = modeList.get(index);
/*      */           } 
/*      */         } 
/*      */       } 
/*      */     } 
/*  868 */     b();
/*      */   }
/*      */ 
/*      */   
/*      */   public float a(zq par1EnumOptions) {
/*  873 */     if (par1EnumOptions == zq.e)
/*      */     {
/*  875 */       return this.N;
/*      */     }
/*  877 */     if (par1EnumOptions == zq.f)
/*      */     {
/*  879 */       return this.O;
/*      */     }
/*  881 */     if (par1EnumOptions == zq.a)
/*      */     {
/*  883 */       return this.a;
/*      */     }
/*  885 */     if (par1EnumOptions == zq.b)
/*      */     {
/*  887 */       return this.b;
/*      */     }
/*  889 */     if (par1EnumOptions == zq.d)
/*      */     {
/*  891 */       return this.c;
/*      */     }
/*      */     
/*  894 */     if (par1EnumOptions == zq.CLOUD_HEIGHT)
/*      */     {
/*  896 */       return this.ofCloudsHeight;
/*      */     }
/*  898 */     if (par1EnumOptions == zq.AO_LEVEL)
/*      */     {
/*  900 */       return this.ofAoLevel;
/*      */     }
/*  902 */     if (par1EnumOptions == zq.RENDER_DISTANCE_FINE)
/*      */     {
/*  904 */       return (this.ofRenderDistanceFine - 32) / 480.0F;
/*      */     }
/*      */     
/*  907 */     return 0.0F;
/*      */   }
/*      */ 
/*      */   
/*      */   public boolean b(zq par1EnumOptions) {
/*  912 */     switch (xp.a[par1EnumOptions.ordinal()]) {
/*      */       
/*      */       case 1:
/*  915 */         return this.d;
/*      */       
/*      */       case 2:
/*  918 */         return this.f;
/*      */       
/*      */       case 3:
/*  921 */         return this.g;
/*      */       
/*      */       case 4:
/*  924 */         return this.h;
/*      */       
/*      */       case 5:
/*  927 */         return this.k;
/*      */       
/*      */       case 6:
/*  930 */         return this.l;
/*      */     } 
/*  932 */     return false;
/*      */   }
/*      */ 
/*      */   
/*      */   private static String a(String[] par0ArrayOfStr, int par1) {
/*  937 */     if (par1 < 0 || par1 >= par0ArrayOfStr.length)
/*      */     {
/*  939 */       par1 = 0;
/*      */     }
/*  941 */     adn stringtranslate = adn.a();
/*  942 */     return stringtranslate.b(par0ArrayOfStr[par1]);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public String c(zq par1EnumOptions) {
/*  950 */     adn stringtranslate = adn.a();
/*      */     
/*  952 */     String prefix = stringtranslate.b(par1EnumOptions.d());
/*  953 */     if (prefix == null)
/*  954 */       prefix = par1EnumOptions.d(); 
/*  955 */     String s = prefix + ": ";
/*      */     
/*  957 */     if (par1EnumOptions.a()) {
/*      */       
/*  959 */       float f = a(par1EnumOptions);
/*  960 */       if (par1EnumOptions == zq.d) {
/*      */         
/*  962 */         if (f == 0.0F)
/*      */         {
/*  964 */           return s + stringtranslate.b("options.sensitivity.min");
/*      */         }
/*  966 */         if (f == 1.0F)
/*      */         {
/*  968 */           return s + stringtranslate.b("options.sensitivity.max");
/*      */         }
/*      */         
/*  971 */         return s + (int)(f * 200.0F) + "%";
/*      */       } 
/*      */       
/*  974 */       if (par1EnumOptions == zq.e) {
/*      */         
/*  976 */         if (f == 0.0F)
/*      */         {
/*  978 */           return s + stringtranslate.b("options.fov.min");
/*      */         }
/*  980 */         if (f == 1.0F)
/*      */         {
/*  982 */           return s + stringtranslate.b("options.fov.max");
/*      */         }
/*      */         
/*  985 */         return s + (int)(70.0F + f * 40.0F);
/*      */       } 
/*      */       
/*  988 */       if (par1EnumOptions == zq.f) {
/*      */         
/*  990 */         if (f == 0.0F)
/*      */         {
/*  992 */           return s + stringtranslate.b("options.gamma.min");
/*      */         }
/*  994 */         if (f == 1.0F)
/*      */         {
/*  996 */           return s + stringtranslate.b("options.gamma.max");
/*      */         }
/*      */         
/*  999 */         return s + "+" + (int)(f * 100.0F) + "%";
/*      */       } 
/*      */ 
/*      */       
/* 1003 */       if (par1EnumOptions == zq.RENDER_DISTANCE_FINE) {
/*      */         
/* 1005 */         String str = "Tiny";
/* 1006 */         int baseDist = 32;
/* 1007 */         if (this.ofRenderDistanceFine >= 64) {
/*      */           
/* 1009 */           str = "Short";
/* 1010 */           baseDist = 64;
/*      */         } 
/* 1012 */         if (this.ofRenderDistanceFine >= 128) {
/*      */           
/* 1014 */           str = "Normal";
/* 1015 */           baseDist = 128;
/*      */         } 
/* 1017 */         if (this.ofRenderDistanceFine >= 256) {
/*      */           
/* 1019 */           str = "Far";
/* 1020 */           baseDist = 256;
/*      */         } 
/* 1022 */         if (this.ofRenderDistanceFine >= 512) {
/*      */           
/* 1024 */           str = "Extreme";
/* 1025 */           baseDist = 512;
/*      */         } 
/* 1027 */         int diff = this.ofRenderDistanceFine - baseDist;
/* 1028 */         if (diff == 0) {
/* 1029 */           return s + str;
/*      */         }
/* 1031 */         return s + str + " +" + diff;
/*      */       } 
/*      */       
/* 1034 */       if (f == 0.0F)
/*      */       {
/* 1036 */         return s + stringtranslate.b("options.off");
/*      */       }
/*      */       
/* 1039 */       return s + (int)(f * 100.0F) + "%";
/*      */     } 
/*      */ 
/*      */     
/* 1043 */     if (par1EnumOptions == zq.j) {
/*      */       
/* 1045 */       if (!this.h)
/*      */       {
/* 1047 */         return s + "OFF";
/*      */       }
/* 1049 */       if (this.ofOcclusionFancy) {
/* 1050 */         return s + "Fancy";
/*      */       }
/* 1052 */       return s + "Fast";
/*      */     } 
/*      */     
/* 1055 */     if (par1EnumOptions.b()) {
/*      */       
/* 1057 */       boolean flag = b(par1EnumOptions);
/* 1058 */       if (flag)
/*      */       {
/* 1060 */         return s + stringtranslate.b("options.on");
/*      */       }
/*      */       
/* 1063 */       return s + stringtranslate.b("options.off");
/*      */     } 
/*      */     
/* 1066 */     if (par1EnumOptions == zq.g)
/*      */     {
/* 1068 */       return s + a(S, this.e);
/*      */     }
/* 1070 */     if (par1EnumOptions == zq.l)
/*      */     {
/* 1072 */       return s + a(T, this.C);
/*      */     }
/* 1074 */     if (par1EnumOptions == zq.o)
/*      */     {
/* 1076 */       return s + a(U, this.P);
/*      */     }
/* 1078 */     if (par1EnumOptions == zq.q)
/*      */     {
/* 1080 */       return s + a(V, this.Q);
/*      */     }
/* 1082 */     if (par1EnumOptions == zq.k) {
/*      */ 
/*      */       
/* 1085 */       if (this.i == 3) {
/* 1086 */         return s + "VSync";
/*      */       }
/* 1088 */       return s + a(W, this.i);
/*      */     } 
/*      */     
/* 1091 */     if (par1EnumOptions == zq.FOG_FANCY) {
/*      */       
/* 1093 */       switch (this.ofFogType) {
/*      */         
/*      */         case 1:
/* 1096 */           return s + "Fast";
/*      */         case 2:
/* 1098 */           return s + "Fancy";
/*      */         case 3:
/* 1100 */           return s + "OFF";
/*      */       } 
/* 1102 */       return s + "OFF";
/*      */     } 
/*      */     
/* 1105 */     if (par1EnumOptions == zq.FOG_START)
/*      */     {
/* 1107 */       return s + this.ofFogStart;
/*      */     }
/* 1109 */     if (par1EnumOptions == zq.MIPMAP_LEVEL) {
/*      */       
/* 1111 */       if (this.ofMipmapLevel == 0)
/* 1112 */         return s + "OFF"; 
/* 1113 */       if (this.ofMipmapLevel == 4) {
/* 1114 */         return s + "Max";
/*      */       }
/* 1116 */       return s + this.ofMipmapLevel;
/*      */     } 
/* 1118 */     if (par1EnumOptions == zq.MIPMAP_TYPE) {
/*      */       
/* 1120 */       if (this.ofMipmapLinear) {
/* 1121 */         return s + "Linear";
/*      */       }
/* 1123 */       return s + "Nearest";
/*      */     } 
/* 1125 */     if (par1EnumOptions == zq.LOAD_FAR) {
/*      */       
/* 1127 */       if (this.ofLoadFar) {
/* 1128 */         return s + "ON";
/*      */       }
/* 1130 */       return s + "OFF";
/*      */     } 
/* 1132 */     if (par1EnumOptions == zq.PRELOADED_CHUNKS) {
/*      */       
/* 1134 */       if (this.ofPreloadedChunks == 0) {
/* 1135 */         return s + "OFF";
/*      */       }
/* 1137 */       return s + this.ofPreloadedChunks;
/*      */     } 
/* 1139 */     if (par1EnumOptions == zq.SMOOTH_FPS) {
/*      */       
/* 1141 */       if (this.ofSmoothFps) {
/* 1142 */         return s + "ON";
/*      */       }
/* 1144 */       return s + "OFF";
/*      */     } 
/* 1146 */     if (par1EnumOptions == zq.SMOOTH_INPUT) {
/*      */       
/* 1148 */       if (this.ofSmoothInput) {
/* 1149 */         return s + "ON";
/*      */       }
/* 1151 */       return s + "OFF";
/*      */     } 
/* 1153 */     if (par1EnumOptions == zq.CLOUDS) {
/*      */       
/* 1155 */       switch (this.ofClouds) {
/*      */         case 1:
/* 1157 */           return s + "Fast";
/* 1158 */         case 2: return s + "Fancy";
/* 1159 */         case 3: return s + "OFF";
/* 1160 */       }  return s + "Default";
/*      */     } 
/*      */     
/* 1163 */     if (par1EnumOptions == zq.TREES) {
/*      */       
/* 1165 */       switch (this.ofTrees) {
/*      */         case 1:
/* 1167 */           return s + "Fast";
/* 1168 */         case 2: return s + "Fancy";
/* 1169 */       }  return s + "Default";
/*      */     } 
/*      */     
/* 1172 */     if (par1EnumOptions == zq.GRASS) {
/*      */       
/* 1174 */       switch (this.ofGrass) {
/*      */         case 1:
/* 1176 */           return s + "Fast";
/* 1177 */         case 2: return s + "Fancy";
/* 1178 */       }  return s + "Default";
/*      */     } 
/*      */     
/* 1181 */     if (par1EnumOptions == zq.RAIN) {
/*      */       
/* 1183 */       switch (this.ofRain) {
/*      */         case 1:
/* 1185 */           return s + "Fast";
/* 1186 */         case 2: return s + "Fancy";
/* 1187 */         case 3: return s + "OFF";
/* 1188 */       }  return s + "Default";
/*      */     } 
/*      */     
/* 1191 */     if (par1EnumOptions == zq.WATER) {
/*      */       
/* 1193 */       switch (this.ofWater) {
/*      */         case 1:
/* 1195 */           return s + "Fast";
/* 1196 */         case 2: return s + "Fancy";
/* 1197 */         case 3: return s + "OFF";
/* 1198 */       }  return s + "Default";
/*      */     } 
/*      */     
/* 1201 */     if (par1EnumOptions == zq.ANIMATED_WATER) {
/*      */       
/* 1203 */       switch (this.ofAnimatedWater) {
/*      */         case 1:
/* 1205 */           return s + "Dynamic";
/* 1206 */         case 2: return s + "OFF";
/*      */       } 
/* 1208 */       return s + "ON";
/*      */     } 
/* 1210 */     if (par1EnumOptions == zq.ANIMATED_LAVA) {
/*      */       
/* 1212 */       switch (this.ofAnimatedLava) {
/*      */         case 1:
/* 1214 */           return s + "Dynamic";
/* 1215 */         case 2: return s + "OFF";
/*      */       } 
/* 1217 */       return s + "ON";
/*      */     } 
/* 1219 */     if (par1EnumOptions == zq.ANIMATED_FIRE) {
/*      */       
/* 1221 */       if (this.ofAnimatedFire) {
/* 1222 */         return s + "ON";
/*      */       }
/* 1224 */       return s + "OFF";
/*      */     } 
/* 1226 */     if (par1EnumOptions == zq.ANIMATED_PORTAL) {
/*      */       
/* 1228 */       if (this.ofAnimatedPortal) {
/* 1229 */         return s + "ON";
/*      */       }
/* 1231 */       return s + "OFF";
/*      */     } 
/* 1233 */     if (par1EnumOptions == zq.ANIMATED_REDSTONE) {
/*      */       
/* 1235 */       if (this.ofAnimatedRedstone) {
/* 1236 */         return s + "ON";
/*      */       }
/* 1238 */       return s + "OFF";
/*      */     } 
/* 1240 */     if (par1EnumOptions == zq.ANIMATED_EXPLOSION) {
/*      */       
/* 1242 */       if (this.ofAnimatedExplosion) {
/* 1243 */         return s + "ON";
/*      */       }
/* 1245 */       return s + "OFF";
/*      */     } 
/* 1247 */     if (par1EnumOptions == zq.ANIMATED_FLAME) {
/*      */       
/* 1249 */       if (this.ofAnimatedFlame) {
/* 1250 */         return s + "ON";
/*      */       }
/* 1252 */       return s + "OFF";
/*      */     } 
/* 1254 */     if (par1EnumOptions == zq.ANIMATED_SMOKE) {
/*      */       
/* 1256 */       if (this.ofAnimatedSmoke) {
/* 1257 */         return s + "ON";
/*      */       }
/* 1259 */       return s + "OFF";
/*      */     } 
/* 1261 */     if (par1EnumOptions == zq.VOID_PARTICLES) {
/*      */       
/* 1263 */       if (this.ofVoidParticles) {
/* 1264 */         return s + "ON";
/*      */       }
/* 1266 */       return s + "OFF";
/*      */     } 
/* 1268 */     if (par1EnumOptions == zq.WATER_PARTICLES) {
/*      */       
/* 1270 */       if (this.ofWaterParticles) {
/* 1271 */         return s + "ON";
/*      */       }
/* 1273 */       return s + "OFF";
/*      */     } 
/* 1275 */     if (par1EnumOptions == zq.PORTAL_PARTICLES) {
/*      */       
/* 1277 */       if (this.ofPortalParticles) {
/* 1278 */         return s + "ON";
/*      */       }
/* 1280 */       return s + "OFF";
/*      */     } 
/* 1282 */     if (par1EnumOptions == zq.DRIPPING_WATER_LAVA) {
/*      */       
/* 1284 */       if (this.ofDrippingWaterLava) {
/* 1285 */         return s + "ON";
/*      */       }
/* 1287 */       return s + "OFF";
/*      */     } 
/* 1289 */     if (par1EnumOptions == zq.ANIMATED_TERRAIN) {
/*      */       
/* 1291 */       if (this.ofAnimatedTerrain) {
/* 1292 */         return s + "ON";
/*      */       }
/* 1294 */       return s + "OFF";
/*      */     } 
/* 1296 */     if (par1EnumOptions == zq.ANIMATED_TEXTURES) {
/*      */       
/* 1298 */       if (this.ofAnimatedTextures) {
/* 1299 */         return s + "ON";
/*      */       }
/* 1301 */       return s + "OFF";
/*      */     } 
/* 1303 */     if (par1EnumOptions == zq.ANIMATED_ITEMS) {
/*      */       
/* 1305 */       if (this.ofAnimatedItems) {
/* 1306 */         return s + "ON";
/*      */       }
/* 1308 */       return s + "OFF";
/*      */     } 
/* 1310 */     if (par1EnumOptions == zq.RAIN_SPLASH) {
/*      */       
/* 1312 */       if (this.ofRainSplash) {
/* 1313 */         return s + "ON";
/*      */       }
/* 1315 */       return s + "OFF";
/*      */     } 
/* 1317 */     if (par1EnumOptions == zq.FAST_DEBUG_INFO) {
/*      */       
/* 1319 */       if (this.ofFastDebugInfo) {
/* 1320 */         return s + "ON";
/*      */       }
/* 1322 */       return s + "OFF";
/*      */     } 
/* 1324 */     if (par1EnumOptions == zq.AUTOSAVE_TICKS) {
/*      */       
/* 1326 */       if (this.ofAutoSaveTicks <= 40)
/* 1327 */         return s + "Default (2s)"; 
/* 1328 */       if (this.ofAutoSaveTicks <= 400)
/* 1329 */         return s + "20s"; 
/* 1330 */       if (this.ofAutoSaveTicks <= 4000) {
/* 1331 */         return s + "3min";
/*      */       }
/* 1333 */       return s + "30min";
/*      */     } 
/* 1335 */     if (par1EnumOptions == zq.BETTER_GRASS) {
/*      */       
/* 1337 */       switch (this.ofBetterGrass) {
/*      */         
/*      */         case 1:
/* 1340 */           return s + "Fast";
/*      */         case 2:
/* 1342 */           return s + "Fancy";
/*      */       } 
/* 1344 */       return s + "OFF";
/*      */     } 
/* 1346 */     if (par1EnumOptions == zq.CONNECTED_TEXTURES) {
/*      */       
/* 1348 */       switch (this.ofConnectedTextures) {
/*      */         
/*      */         case 1:
/* 1351 */           return s + "Fast";
/*      */         case 2:
/* 1353 */           return s + "Fancy";
/*      */       } 
/* 1355 */       return s + "OFF";
/*      */     } 
/* 1357 */     if (par1EnumOptions == zq.WEATHER) {
/*      */       
/* 1359 */       if (this.ofWeather) {
/* 1360 */         return s + "ON";
/*      */       }
/* 1362 */       return s + "OFF";
/*      */     } 
/* 1364 */     if (par1EnumOptions == zq.SKY) {
/*      */       
/* 1366 */       if (this.ofSky) {
/* 1367 */         return s + "ON";
/*      */       }
/* 1369 */       return s + "OFF";
/*      */     } 
/* 1371 */     if (par1EnumOptions == zq.STARS) {
/*      */       
/* 1373 */       if (this.ofStars) {
/* 1374 */         return s + "ON";
/*      */       }
/* 1376 */       return s + "OFF";
/*      */     } 
/* 1378 */     if (par1EnumOptions == zq.SUN_MOON) {
/*      */       
/* 1380 */       if (this.ofSunMoon) {
/* 1381 */         return s + "ON";
/*      */       }
/* 1383 */       return s + "OFF";
/*      */     } 
/* 1385 */     if (par1EnumOptions == zq.CHUNK_UPDATES)
/*      */     {
/* 1387 */       return s + this.ofChunkUpdates;
/*      */     }
/* 1389 */     if (par1EnumOptions == zq.CHUNK_UPDATES_DYNAMIC) {
/*      */       
/* 1391 */       if (this.ofChunkUpdatesDynamic) {
/* 1392 */         return s + "ON";
/*      */       }
/* 1394 */       return s + "OFF";
/*      */     } 
/* 1396 */     if (par1EnumOptions == zq.TIME) {
/*      */       
/* 1398 */       if (this.ofTime == 1)
/* 1399 */         return s + "Day Only"; 
/* 1400 */       if (this.ofTime == 3)
/* 1401 */         return s + "Night Only"; 
/* 1402 */       return s + "Default";
/*      */     } 
/* 1404 */     if (par1EnumOptions == zq.CLEAR_WATER) {
/*      */       
/* 1406 */       if (this.ofClearWater) {
/* 1407 */         return s + "ON";
/*      */       }
/* 1409 */       return s + "OFF";
/*      */     } 
/* 1411 */     if (par1EnumOptions == zq.DEPTH_FOG) {
/*      */       
/* 1413 */       if (this.ofDepthFog) {
/* 1414 */         return s + "ON";
/*      */       }
/* 1416 */       return s + "OFF";
/*      */     } 
/* 1418 */     if (par1EnumOptions == zq.PROFILER) {
/*      */       
/* 1420 */       if (this.ofProfiler) {
/* 1421 */         return s + "ON";
/*      */       }
/* 1423 */       return s + "OFF";
/*      */     } 
/* 1425 */     if (par1EnumOptions == zq.BETTER_SNOW) {
/*      */       
/* 1427 */       if (this.ofBetterSnow) {
/* 1428 */         return s + "ON";
/*      */       }
/* 1430 */       return s + "OFF";
/*      */     } 
/* 1432 */     if (par1EnumOptions == zq.SWAMP_COLORS) {
/*      */       
/* 1434 */       if (this.ofSwampColors) {
/* 1435 */         return s + "ON";
/*      */       }
/* 1437 */       return s + "OFF";
/*      */     } 
/* 1439 */     if (par1EnumOptions == zq.RANDOM_MOBS) {
/*      */       
/* 1441 */       if (this.ofRandomMobs) {
/* 1442 */         return s + "ON";
/*      */       }
/* 1444 */       return s + "OFF";
/*      */     } 
/* 1446 */     if (par1EnumOptions == zq.SMOOTH_BIOMES) {
/*      */       
/* 1448 */       if (this.ofSmoothBiomes) {
/* 1449 */         return s + "ON";
/*      */       }
/* 1451 */       return s + "OFF";
/*      */     } 
/* 1453 */     if (par1EnumOptions == zq.CUSTOM_FONTS) {
/*      */       
/* 1455 */       if (this.ofCustomFonts) {
/* 1456 */         return s + "ON";
/*      */       }
/* 1458 */       return s + "OFF";
/*      */     } 
/* 1460 */     if (par1EnumOptions == zq.CUSTOM_COLORS) {
/*      */       
/* 1462 */       if (this.ofCustomColors) {
/* 1463 */         return s + "ON";
/*      */       }
/* 1465 */       return s + "OFF";
/*      */     } 
/* 1467 */     if (par1EnumOptions == zq.SHOW_CAPES) {
/*      */       
/* 1469 */       if (this.ofShowCapes) {
/* 1470 */         return s + "ON";
/*      */       }
/* 1472 */       return s + "OFF";
/*      */     } 
/* 1474 */     if (par1EnumOptions == zq.NATURAL_TEXTURES) {
/*      */       
/* 1476 */       if (this.ofNaturalTextures) {
/* 1477 */         return s + "ON";
/*      */       }
/* 1479 */       return s + "OFF";
/*      */     } 
/* 1481 */     if (par1EnumOptions == zq.FULLSCREEN_MODE)
/*      */     {
/* 1483 */       return s + this.ofFullscreenMode;
/*      */     }
/*      */     
/* 1486 */     if (par1EnumOptions == zq.m) {
/*      */       
/* 1488 */       if (this.j)
/*      */       {
/* 1490 */         return s + stringtranslate.b("options.graphics.fancy");
/*      */       }
/*      */       
/* 1493 */       return s + stringtranslate.b("options.graphics.fast");
/*      */     } 
/*      */ 
/*      */     
/* 1497 */     return s;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a() {
/*      */     try {
/* 1508 */       if (!this.X.exists()) {
/*      */         return;
/*      */       }
/*      */       
/* 1512 */       BufferedReader bufferedreader = new BufferedReader(new FileReader(this.X));
/* 1513 */       for (String s = ""; (s = bufferedreader.readLine()) != null;) {
/*      */ 
/*      */         
/*      */         try {
/* 1517 */           String[] as = s.split(":");
/* 1518 */           if (as[0].equals("music"))
/*      */           {
/* 1520 */             this.a = a(as[1]);
/*      */           }
/* 1522 */           if (as[0].equals("sound"))
/*      */           {
/* 1524 */             this.b = a(as[1]);
/*      */           }
/* 1526 */           if (as[0].equals("mouseSensitivity"))
/*      */           {
/* 1528 */             this.c = a(as[1]);
/*      */           }
/* 1530 */           if (as[0].equals("fov"))
/*      */           {
/* 1532 */             this.N = a(as[1]);
/*      */           }
/* 1534 */           if (as[0].equals("gamma"))
/*      */           {
/* 1536 */             this.O = a(as[1]);
/*      */           }
/* 1538 */           if (as[0].equals("invertYMouse"))
/*      */           {
/* 1540 */             this.d = as[1].equals("true");
/*      */           }
/* 1542 */           if (as[0].equals("viewDistance")) {
/*      */             
/* 1544 */             this.e = Integer.parseInt(as[1]);
/*      */             
/* 1546 */             this.ofRenderDistanceFine = 32 << 3 - this.e;
/*      */           } 
/* 1548 */           if (as[0].equals("guiScale"))
/*      */           {
/* 1550 */             this.P = Integer.parseInt(as[1]);
/*      */           }
/* 1552 */           if (as[0].equals("particles"))
/*      */           {
/* 1554 */             this.Q = Integer.parseInt(as[1]);
/*      */           }
/* 1556 */           if (as[0].equals("bobView"))
/*      */           {
/* 1558 */             this.f = as[1].equals("true");
/*      */           }
/* 1560 */           if (as[0].equals("anaglyph3d"))
/*      */           {
/* 1562 */             this.g = as[1].equals("true");
/*      */           }
/* 1564 */           if (as[0].equals("advancedOpengl"))
/*      */           {
/* 1566 */             this.h = as[1].equals("true");
/*      */           }
/* 1568 */           if (as[0].equals("fpsLimit")) {
/*      */             
/* 1570 */             this.i = Integer.parseInt(as[1]);
/*      */             
/* 1572 */             Display.setVSyncEnabled((this.i == 3));
/*      */           } 
/* 1574 */           if (as[0].equals("difficulty"))
/*      */           {
/* 1576 */             this.C = Integer.parseInt(as[1]);
/*      */           }
/* 1578 */           if (as[0].equals("fancyGraphics"))
/*      */           {
/* 1580 */             this.j = as[1].equals("true");
/*      */           }
/* 1582 */           if (as[0].equals("ao")) {
/*      */             
/* 1584 */             this.k = as[1].equals("true");
/*      */             
/* 1586 */             if (this.k) {
/* 1587 */               this.ofAoLevel = 1.0F;
/*      */             } else {
/* 1589 */               this.ofAoLevel = 0.0F;
/*      */             } 
/* 1591 */           }  if (as[0].equals("clouds"))
/*      */           {
/* 1593 */             this.l = as[1].equals("true");
/*      */           }
/* 1595 */           if (as[0].equals("skin"))
/*      */           {
/* 1597 */             this.m = as[1];
/*      */           }
/* 1599 */           if (as[0].equals("lastServer") && as.length >= 2)
/*      */           {
/* 1601 */             this.H = as[1];
/*      */           }
/* 1603 */           if (as[0].equals("lang") && as.length >= 2)
/*      */           {
/* 1605 */             this.R = as[1];
/*      */           }
/* 1607 */           int i = 0;
/* 1608 */           while (i < this.A.length)
/*      */           {
/* 1610 */             if (as[0].equals("key_" + (this.A[i]).c))
/*      */             {
/* 1612 */               (this.A[i]).d = Integer.parseInt(as[1]);
/*      */             }
/* 1614 */             i++;
/*      */           }
/*      */         
/* 1617 */         } catch (Exception exception1) {
/*      */           
/* 1619 */           System.out.println("Skipping bad option: " + s);
/*      */         } 
/*      */       } 
/*      */       
/* 1623 */       afu.b();
/* 1624 */       bufferedreader.close();
/*      */     }
/* 1626 */     catch (Exception exception) {
/*      */       
/* 1628 */       System.out.println("Failed to load options");
/* 1629 */       exception.printStackTrace();
/*      */     } 
/*      */ 
/*      */     
/*      */     try {
/* 1634 */       File ofReadFile = this.optionsFileOF;
/*      */       
/* 1636 */       if (!ofReadFile.exists()) {
/* 1637 */         ofReadFile = this.X;
/*      */       }
/* 1639 */       if (!ofReadFile.exists()) {
/*      */         return;
/*      */       }
/*      */       
/* 1643 */       BufferedReader bufferedreader = new BufferedReader(new FileReader(ofReadFile));
/* 1644 */       for (String s = ""; (s = bufferedreader.readLine()) != null;) {
/*      */ 
/*      */         
/*      */         try {
/* 1648 */           String[] as = s.split(":");
/*      */           
/* 1650 */           if (as[0].equals("ofRenderDistanceFine") && as.length >= 2) {
/*      */             
/* 1652 */             this.ofRenderDistanceFine = Integer.valueOf(as[1]).intValue();
/* 1653 */             this.ofRenderDistanceFine = Config.limit(this.ofRenderDistanceFine, 32, 512);
/*      */           } 
/* 1655 */           if (as[0].equals("ofFogType") && as.length >= 2) {
/*      */             
/* 1657 */             this.ofFogType = Integer.valueOf(as[1]).intValue();
/* 1658 */             this.ofFogType = Config.limit(this.ofFogType, 1, 3);
/*      */           } 
/* 1660 */           if (as[0].equals("ofFogStart") && as.length >= 2) {
/*      */             
/* 1662 */             this.ofFogStart = Float.valueOf(as[1]).floatValue();
/* 1663 */             if (this.ofFogStart < 0.2F)
/* 1664 */               this.ofFogStart = 0.2F; 
/* 1665 */             if (this.ofFogStart > 0.81F)
/* 1666 */               this.ofFogStart = 0.8F; 
/*      */           } 
/* 1668 */           if (as[0].equals("ofMipmapLevel") && as.length >= 2) {
/*      */             
/* 1670 */             this.ofMipmapLevel = Integer.valueOf(as[1]).intValue();
/* 1671 */             if (this.ofMipmapLevel < 0)
/* 1672 */               this.ofMipmapLevel = 0; 
/* 1673 */             if (this.ofMipmapLevel > 4)
/* 1674 */               this.ofMipmapLevel = 4; 
/*      */           } 
/* 1676 */           if (as[0].equals("ofMipmapLinear") && as.length >= 2)
/*      */           {
/* 1678 */             this.ofMipmapLinear = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1680 */           if (as[0].equals("ofLoadFar") && as.length >= 2)
/*      */           {
/* 1682 */             this.ofLoadFar = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1684 */           if (as[0].equals("ofPreloadedChunks") && as.length >= 2) {
/*      */             
/* 1686 */             this.ofPreloadedChunks = Integer.valueOf(as[1]).intValue();
/* 1687 */             if (this.ofPreloadedChunks < 0)
/* 1688 */               this.ofPreloadedChunks = 0; 
/* 1689 */             if (this.ofPreloadedChunks > 8)
/* 1690 */               this.ofPreloadedChunks = 8; 
/*      */           } 
/* 1692 */           if (as[0].equals("ofOcclusionFancy") && as.length >= 2)
/*      */           {
/* 1694 */             this.ofOcclusionFancy = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1696 */           if (as[0].equals("ofSmoothFps") && as.length >= 2)
/*      */           {
/* 1698 */             this.ofSmoothFps = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1700 */           if (as[0].equals("ofSmoothInput") && as.length >= 2)
/*      */           {
/* 1702 */             this.ofSmoothInput = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1704 */           if (as[0].equals("ofAoLevel") && as.length >= 2) {
/*      */             
/* 1706 */             this.ofAoLevel = Float.valueOf(as[1]).floatValue();
/* 1707 */             this.ofAoLevel = Config.limit(this.ofAoLevel, 0.0F, 1.0F);
/* 1708 */             this.k = (this.ofAoLevel > 0.0F);
/*      */           } 
/* 1710 */           if (as[0].equals("ofClouds") && as.length >= 2) {
/*      */             
/* 1712 */             this.ofClouds = Integer.valueOf(as[1]).intValue();
/* 1713 */             this.ofClouds = Config.limit(this.ofClouds, 0, 3);
/*      */           } 
/* 1715 */           if (as[0].equals("ofCloudsHeight") && as.length >= 2) {
/*      */             
/* 1717 */             this.ofCloudsHeight = Float.valueOf(as[1]).floatValue();
/* 1718 */             this.ofCloudsHeight = Config.limit(this.ofCloudsHeight, 0.0F, 1.0F);
/*      */           } 
/* 1720 */           if (as[0].equals("ofTrees") && as.length >= 2) {
/*      */             
/* 1722 */             this.ofTrees = Integer.valueOf(as[1]).intValue();
/* 1723 */             this.ofTrees = Config.limit(this.ofTrees, 0, 2);
/*      */           } 
/* 1725 */           if (as[0].equals("ofGrass") && as.length >= 2) {
/*      */             
/* 1727 */             this.ofGrass = Integer.valueOf(as[1]).intValue();
/* 1728 */             this.ofGrass = Config.limit(this.ofGrass, 0, 2);
/*      */           } 
/* 1730 */           if (as[0].equals("ofRain") && as.length >= 2) {
/*      */             
/* 1732 */             this.ofRain = Integer.valueOf(as[1]).intValue();
/* 1733 */             this.ofRain = Config.limit(this.ofRain, 0, 3);
/*      */           } 
/* 1735 */           if (as[0].equals("ofWater") && as.length >= 2) {
/*      */             
/* 1737 */             this.ofWater = Integer.valueOf(as[1]).intValue();
/* 1738 */             this.ofWater = Config.limit(this.ofWater, 0, 3);
/*      */           } 
/* 1740 */           if (as[0].equals("ofAnimatedWater") && as.length >= 2) {
/*      */             
/* 1742 */             this.ofAnimatedWater = Integer.valueOf(as[1]).intValue();
/* 1743 */             this.ofAnimatedWater = Config.limit(this.ofAnimatedWater, 0, 2);
/*      */           } 
/* 1745 */           if (as[0].equals("ofAnimatedLava") && as.length >= 2) {
/*      */             
/* 1747 */             this.ofAnimatedLava = Integer.valueOf(as[1]).intValue();
/* 1748 */             this.ofAnimatedLava = Config.limit(this.ofAnimatedLava, 0, 2);
/*      */           } 
/* 1750 */           if (as[0].equals("ofAnimatedFire") && as.length >= 2)
/*      */           {
/* 1752 */             this.ofAnimatedFire = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1754 */           if (as[0].equals("ofAnimatedPortal") && as.length >= 2)
/*      */           {
/* 1756 */             this.ofAnimatedPortal = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1758 */           if (as[0].equals("ofAnimatedRedstone") && as.length >= 2)
/*      */           {
/* 1760 */             this.ofAnimatedRedstone = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1762 */           if (as[0].equals("ofAnimatedExplosion") && as.length >= 2)
/*      */           {
/* 1764 */             this.ofAnimatedExplosion = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1766 */           if (as[0].equals("ofAnimatedFlame") && as.length >= 2)
/*      */           {
/* 1768 */             this.ofAnimatedFlame = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1770 */           if (as[0].equals("ofAnimatedSmoke") && as.length >= 2)
/*      */           {
/* 1772 */             this.ofAnimatedSmoke = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1774 */           if (as[0].equals("ofVoidParticles") && as.length >= 2)
/*      */           {
/* 1776 */             this.ofVoidParticles = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1778 */           if (as[0].equals("ofWaterParticles") && as.length >= 2)
/*      */           {
/* 1780 */             this.ofWaterParticles = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1782 */           if (as[0].equals("ofPortalParticles") && as.length >= 2)
/*      */           {
/* 1784 */             this.ofPortalParticles = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1786 */           if (as[0].equals("ofDrippingWaterLava") && as.length >= 2)
/*      */           {
/* 1788 */             this.ofDrippingWaterLava = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1790 */           if (as[0].equals("ofAnimatedTerrain") && as.length >= 2)
/*      */           {
/* 1792 */             this.ofAnimatedTerrain = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1794 */           if (as[0].equals("ofAnimatedTextures") && as.length >= 2)
/*      */           {
/* 1796 */             this.ofAnimatedTextures = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1798 */           if (as[0].equals("ofAnimatedItems") && as.length >= 2)
/*      */           {
/* 1800 */             this.ofAnimatedItems = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1802 */           if (as[0].equals("ofRainSplash") && as.length >= 2)
/*      */           {
/* 1804 */             this.ofRainSplash = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1806 */           if (as[0].equals("ofFastDebugInfo") && as.length >= 2)
/*      */           {
/* 1808 */             this.ofFastDebugInfo = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1810 */           if (as[0].equals("ofAutoSaveTicks") && as.length >= 2) {
/*      */             
/* 1812 */             this.ofAutoSaveTicks = Integer.valueOf(as[1]).intValue();
/* 1813 */             this.ofAutoSaveTicks = Config.limit(this.ofAutoSaveTicks, 40, 40000);
/*      */           } 
/* 1815 */           if (as[0].equals("ofBetterGrass") && as.length >= 2) {
/*      */             
/* 1817 */             this.ofBetterGrass = Integer.valueOf(as[1]).intValue();
/* 1818 */             this.ofBetterGrass = Config.limit(this.ofBetterGrass, 1, 3);
/*      */           } 
/* 1820 */           if (as[0].equals("ofConnectedTextures") && as.length >= 2) {
/*      */             
/* 1822 */             this.ofConnectedTextures = Integer.valueOf(as[1]).intValue();
/* 1823 */             this.ofConnectedTextures = Config.limit(this.ofConnectedTextures, 1, 3);
/*      */           } 
/* 1825 */           if (as[0].equals("ofWeather") && as.length >= 2)
/*      */           {
/* 1827 */             this.ofWeather = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1829 */           if (as[0].equals("ofSky") && as.length >= 2)
/*      */           {
/* 1831 */             this.ofSky = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1833 */           if (as[0].equals("ofStars") && as.length >= 2)
/*      */           {
/* 1835 */             this.ofStars = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1837 */           if (as[0].equals("ofSunMoon") && as.length >= 2)
/*      */           {
/* 1839 */             this.ofSunMoon = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1841 */           if (as[0].equals("ofChunkUpdates") && as.length >= 2) {
/*      */             
/* 1843 */             this.ofChunkUpdates = Integer.valueOf(as[1]).intValue();
/* 1844 */             this.ofChunkUpdates = Config.limit(this.ofChunkUpdates, 1, 5);
/*      */           } 
/* 1846 */           if (as[0].equals("ofChunkUpdatesDynamic") && as.length >= 2)
/*      */           {
/* 1848 */             this.ofChunkUpdatesDynamic = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1850 */           if (as[0].equals("ofTime") && as.length >= 2) {
/*      */             
/* 1852 */             this.ofTime = Integer.valueOf(as[1]).intValue();
/* 1853 */             this.ofTime = Config.limit(this.ofTime, 0, 3);
/*      */           } 
/* 1855 */           if (as[0].equals("ofClearWater") && as.length >= 2) {
/*      */             
/* 1857 */             this.ofClearWater = Boolean.valueOf(as[1]).booleanValue();
/* 1858 */             updateWaterOpacity();
/*      */           } 
/* 1860 */           if (as[0].equals("ofDepthFog") && as.length >= 2)
/*      */           {
/* 1862 */             this.ofDepthFog = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1864 */           if (as[0].equals("ofAaLevel") && as.length >= 2) {
/*      */             
/* 1866 */             this.ofAaLevel = Integer.valueOf(as[1]).intValue();
/* 1867 */             this.ofAaLevel = Config.limit(this.ofAaLevel, 0, 16);
/*      */           } 
/* 1869 */           if (as[0].equals("ofAfLevel") && as.length >= 2) {
/*      */             
/* 1871 */             this.ofAfLevel = Integer.valueOf(as[1]).intValue();
/* 1872 */             this.ofAfLevel = Config.limit(this.ofAfLevel, 1, 16);
/*      */           } 
/* 1874 */           if (as[0].equals("ofProfiler") && as.length >= 2)
/*      */           {
/* 1876 */             this.ofProfiler = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1878 */           if (as[0].equals("ofBetterSnow") && as.length >= 2)
/*      */           {
/* 1880 */             this.ofBetterSnow = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1882 */           if (as[0].equals("ofSwampColors") && as.length >= 2)
/*      */           {
/* 1884 */             this.ofSwampColors = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1886 */           if (as[0].equals("ofRandomMobs") && as.length >= 2)
/*      */           {
/* 1888 */             this.ofRandomMobs = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1890 */           if (as[0].equals("ofSmoothBiomes") && as.length >= 2)
/*      */           {
/* 1892 */             this.ofSmoothBiomes = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1894 */           if (as[0].equals("ofCustomFonts") && as.length >= 2)
/*      */           {
/* 1896 */             this.ofCustomFonts = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1898 */           if (as[0].equals("ofCustomColors") && as.length >= 2)
/*      */           {
/* 1900 */             this.ofCustomColors = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1902 */           if (as[0].equals("ofShowCapes") && as.length >= 2)
/*      */           {
/* 1904 */             this.ofShowCapes = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1906 */           if (as[0].equals("ofNaturalTextures") && as.length >= 2)
/*      */           {
/* 1908 */             this.ofNaturalTextures = Boolean.valueOf(as[1]).booleanValue();
/*      */           }
/* 1910 */           if (as[0].equals("ofFullscreenMode") && as.length >= 2)
/*      */           {
/* 1912 */             this.ofFullscreenMode = as[1];
/*      */           
/*      */           }
/*      */         }
/* 1916 */         catch (Exception exception1) {
/*      */           
/* 1918 */           System.out.println("Skipping bad option: " + s);
/*      */         } 
/*      */       } 
/*      */       
/* 1922 */       afu.b();
/* 1923 */       bufferedreader.close();
/*      */     }
/* 1925 */     catch (Exception exception) {
/*      */       
/* 1927 */       System.out.println("Failed to load options");
/* 1928 */       exception.printStackTrace();
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private float a(String par1Str) {
/* 1937 */     if (par1Str.equals("true"))
/*      */     {
/* 1939 */       return 1.0F;
/*      */     }
/* 1941 */     if (par1Str.equals("false"))
/*      */     {
/* 1943 */       return 0.0F;
/*      */     }
/*      */     
/* 1946 */     return Float.parseFloat(par1Str);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void b() {
/*      */     try {
/* 1957 */       PrintWriter printwriter = new PrintWriter(new FileWriter(this.X));
/* 1958 */       printwriter.println("music:" + this.a);
/* 1959 */       printwriter.println("sound:" + this.b);
/* 1960 */       printwriter.println("invertYMouse:" + this.d);
/* 1961 */       printwriter.println("mouseSensitivity:" + this.c);
/* 1962 */       printwriter.println("fov:" + this.N);
/* 1963 */       printwriter.println("gamma:" + this.O);
/* 1964 */       printwriter.println("viewDistance:" + this.e);
/* 1965 */       printwriter.println("guiScale:" + this.P);
/* 1966 */       printwriter.println("particles:" + this.Q);
/* 1967 */       printwriter.println("bobView:" + this.f);
/* 1968 */       printwriter.println("anaglyph3d:" + this.g);
/* 1969 */       printwriter.println("advancedOpengl:" + this.h);
/* 1970 */       printwriter.println("fpsLimit:" + this.i);
/* 1971 */       printwriter.println("difficulty:" + this.C);
/* 1972 */       printwriter.println("fancyGraphics:" + this.j);
/* 1973 */       printwriter.println("ao:" + this.k);
/* 1974 */       printwriter.println("clouds:" + this.l);
/* 1975 */       printwriter.println("skin:" + this.m);
/* 1976 */       printwriter.println("lastServer:" + this.H);
/* 1977 */       printwriter.println("lang:" + this.R);
/* 1978 */       for (int i = 0; i < this.A.length; i++)
/*      */       {
/* 1980 */         printwriter.println("key_" + (this.A[i]).c + ":" + (this.A[i]).d);
/*      */       }
/* 1982 */       printwriter.close();
/*      */     }
/* 1984 */     catch (Exception exception) {
/*      */       
/* 1986 */       System.out.println("Failed to save options");
/* 1987 */       exception.printStackTrace();
/*      */     } 
/*      */ 
/*      */     
/*      */     try {
/* 1992 */       PrintWriter printwriter = new PrintWriter(new FileWriter(this.optionsFileOF));
/*      */       
/* 1994 */       printwriter.println("ofRenderDistanceFine:" + this.ofRenderDistanceFine);
/* 1995 */       printwriter.println("ofFogType:" + this.ofFogType);
/* 1996 */       printwriter.println("ofFogStart:" + this.ofFogStart);
/* 1997 */       printwriter.println("ofMipmapLevel:" + this.ofMipmapLevel);
/* 1998 */       printwriter.println("ofMipmapLinear:" + this.ofMipmapLinear);
/* 1999 */       printwriter.println("ofLoadFar:" + this.ofLoadFar);
/* 2000 */       printwriter.println("ofPreloadedChunks:" + this.ofPreloadedChunks);
/* 2001 */       printwriter.println("ofOcclusionFancy:" + this.ofOcclusionFancy);
/* 2002 */       printwriter.println("ofSmoothFps:" + this.ofSmoothFps);
/* 2003 */       printwriter.println("ofSmoothInput:" + this.ofSmoothInput);
/* 2004 */       printwriter.println("ofAoLevel:" + this.ofAoLevel);
/* 2005 */       printwriter.println("ofClouds:" + this.ofClouds);
/* 2006 */       printwriter.println("ofCloudsHeight:" + this.ofCloudsHeight);
/* 2007 */       printwriter.println("ofTrees:" + this.ofTrees);
/* 2008 */       printwriter.println("ofGrass:" + this.ofGrass);
/* 2009 */       printwriter.println("ofRain:" + this.ofRain);
/* 2010 */       printwriter.println("ofWater:" + this.ofWater);
/* 2011 */       printwriter.println("ofAnimatedWater:" + this.ofAnimatedWater);
/* 2012 */       printwriter.println("ofAnimatedLava:" + this.ofAnimatedLava);
/* 2013 */       printwriter.println("ofAnimatedFire:" + this.ofAnimatedFire);
/* 2014 */       printwriter.println("ofAnimatedPortal:" + this.ofAnimatedPortal);
/* 2015 */       printwriter.println("ofAnimatedRedstone:" + this.ofAnimatedRedstone);
/* 2016 */       printwriter.println("ofAnimatedExplosion:" + this.ofAnimatedExplosion);
/* 2017 */       printwriter.println("ofAnimatedFlame:" + this.ofAnimatedFlame);
/* 2018 */       printwriter.println("ofAnimatedSmoke:" + this.ofAnimatedSmoke);
/* 2019 */       printwriter.println("ofVoidParticles:" + this.ofVoidParticles);
/* 2020 */       printwriter.println("ofWaterParticles:" + this.ofWaterParticles);
/* 2021 */       printwriter.println("ofPortalParticles:" + this.ofPortalParticles);
/* 2022 */       printwriter.println("ofDrippingWaterLava:" + this.ofDrippingWaterLava);
/* 2023 */       printwriter.println("ofAnimatedTerrain:" + this.ofAnimatedTerrain);
/* 2024 */       printwriter.println("ofAnimatedTextures:" + this.ofAnimatedTextures);
/* 2025 */       printwriter.println("ofAnimatedItems:" + this.ofAnimatedItems);
/* 2026 */       printwriter.println("ofRainSplash:" + this.ofRainSplash);
/* 2027 */       printwriter.println("ofFastDebugInfo:" + this.ofFastDebugInfo);
/* 2028 */       printwriter.println("ofAutoSaveTicks:" + this.ofAutoSaveTicks);
/* 2029 */       printwriter.println("ofBetterGrass:" + this.ofBetterGrass);
/* 2030 */       printwriter.println("ofConnectedTextures:" + this.ofConnectedTextures);
/* 2031 */       printwriter.println("ofWeather:" + this.ofWeather);
/* 2032 */       printwriter.println("ofSky:" + this.ofSky);
/* 2033 */       printwriter.println("ofStars:" + this.ofStars);
/* 2034 */       printwriter.println("ofSunMoon:" + this.ofSunMoon);
/* 2035 */       printwriter.println("ofChunkUpdates:" + this.ofChunkUpdates);
/* 2036 */       printwriter.println("ofChunkUpdatesDynamic:" + this.ofChunkUpdatesDynamic);
/* 2037 */       printwriter.println("ofTime:" + this.ofTime);
/* 2038 */       printwriter.println("ofClearWater:" + this.ofClearWater);
/* 2039 */       printwriter.println("ofDepthFog:" + this.ofDepthFog);
/* 2040 */       printwriter.println("ofAaLevel:" + this.ofAaLevel);
/* 2041 */       printwriter.println("ofAfLevel:" + this.ofAfLevel);
/* 2042 */       printwriter.println("ofProfiler:" + this.ofProfiler);
/* 2043 */       printwriter.println("ofBetterSnow:" + this.ofBetterSnow);
/* 2044 */       printwriter.println("ofSwampColors:" + this.ofSwampColors);
/* 2045 */       printwriter.println("ofRandomMobs:" + this.ofRandomMobs);
/* 2046 */       printwriter.println("ofSmoothBiomes:" + this.ofSmoothBiomes);
/* 2047 */       printwriter.println("ofCustomFonts:" + this.ofCustomFonts);
/* 2048 */       printwriter.println("ofCustomColors:" + this.ofCustomColors);
/* 2049 */       printwriter.println("ofShowCapes:" + this.ofShowCapes);
/* 2050 */       printwriter.println("ofNaturalTextures:" + this.ofNaturalTextures);
/* 2051 */       printwriter.println("ofFullscreenMode:" + this.ofFullscreenMode);
/*      */       
/* 2053 */       printwriter.close();
/*      */     }
/* 2055 */     catch (Exception exception) {
/*      */       
/* 2057 */       System.out.println("Failed to save options");
/* 2058 */       exception.printStackTrace();
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void resetSettings() {
/* 2068 */     this.e = 1;
/* 2069 */     this.f = true;
/* 2070 */     this.g = false;
/* 2071 */     this.h = false;
/* 2072 */     this.i = 0;
/* 2073 */     this.j = true;
/* 2074 */     this.k = true;
/* 2075 */     this.l = true;
/* 2076 */     this.N = 0.0F;
/* 2077 */     this.O = 0.0F;
/* 2078 */     this.P = 0;
/* 2079 */     this.Q = 0;
/*      */     
/* 2081 */     this.ofRenderDistanceFine = 32 << 3 - this.e;
/* 2082 */     this.ofFogType = 1;
/* 2083 */     this.ofFogStart = 0.8F;
/* 2084 */     this.ofMipmapLevel = 0;
/* 2085 */     this.ofMipmapLinear = false;
/* 2086 */     this.ofLoadFar = false;
/* 2087 */     this.ofPreloadedChunks = 0;
/* 2088 */     this.ofOcclusionFancy = false;
/* 2089 */     this.ofSmoothFps = false;
/* 2090 */     this.ofSmoothInput = true;
/*      */     
/* 2092 */     if (this.k) {
/* 2093 */       this.ofAoLevel = 1.0F;
/*      */     } else {
/* 2095 */       this.ofAaLevel = 0;
/*      */     } 
/* 2097 */     this.ofAaLevel = 0;
/* 2098 */     this.ofAfLevel = 1;
/*      */     
/* 2100 */     this.ofClouds = 0;
/* 2101 */     this.ofCloudsHeight = 0.0F;
/* 2102 */     this.ofTrees = 0;
/* 2103 */     this.ofGrass = 0;
/* 2104 */     this.ofRain = 0;
/* 2105 */     this.ofWater = 0;
/* 2106 */     this.ofBetterGrass = 3;
/* 2107 */     this.ofAutoSaveTicks = 4000;
/* 2108 */     this.ofFastDebugInfo = false;
/* 2109 */     this.ofWeather = true;
/* 2110 */     this.ofSky = true;
/* 2111 */     this.ofStars = true;
/* 2112 */     this.ofSunMoon = true;
/* 2113 */     this.ofChunkUpdates = 1;
/* 2114 */     this.ofChunkUpdatesDynamic = false;
/* 2115 */     this.ofTime = 0;
/* 2116 */     this.ofClearWater = false;
/* 2117 */     this.ofDepthFog = true;
/* 2118 */     this.ofProfiler = false;
/* 2119 */     this.ofBetterSnow = false;
/* 2120 */     this.ofFullscreenMode = "Default";
/* 2121 */     this.ofSwampColors = true;
/* 2122 */     this.ofRandomMobs = true;
/* 2123 */     this.ofSmoothBiomes = true;
/* 2124 */     this.ofCustomFonts = true;
/* 2125 */     this.ofCustomColors = true;
/* 2126 */     this.ofShowCapes = true;
/* 2127 */     this.ofConnectedTextures = 2;
/* 2128 */     this.ofNaturalTextures = false;
/*      */     
/* 2130 */     this.ofAnimatedWater = 0;
/* 2131 */     this.ofAnimatedLava = 0;
/* 2132 */     this.ofAnimatedFire = true;
/* 2133 */     this.ofAnimatedPortal = true;
/* 2134 */     this.ofAnimatedRedstone = true;
/* 2135 */     this.ofAnimatedExplosion = true;
/* 2136 */     this.ofAnimatedFlame = true;
/* 2137 */     this.ofAnimatedSmoke = true;
/* 2138 */     this.ofVoidParticles = true;
/* 2139 */     this.ofWaterParticles = true;
/* 2140 */     this.ofRainSplash = true;
/* 2141 */     this.ofPortalParticles = true;
/* 2142 */     this.ofDrippingWaterLava = true;
/* 2143 */     this.ofAnimatedTerrain = true;
/* 2144 */     this.ofAnimatedItems = true;
/* 2145 */     this.ofAnimatedTextures = true;
/*      */     
/* 2147 */     this.B.g.updateCapes();
/*      */     
/* 2149 */     updateWaterOpacity();
/* 2150 */     this.B.g.setAllRenderersVisible();
/* 2151 */     this.B.p.b();
/* 2152 */     this.B.g.a();
/*      */     
/* 2154 */     b();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean c() {
/* 2163 */     return (this.ofRenderDistanceFine > 64 && this.l);
/*      */   }
/*      */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\hu.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */