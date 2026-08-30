/*      */ import java.awt.Dimension;
/*      */ import java.awt.image.BufferedImage;
/*      */ import java.lang.reflect.Field;
/*      */ import java.nio.FloatBuffer;
/*      */ import java.util.Calendar;
/*      */ import java.util.Date;
/*      */ import java.util.List;
/*      */ import java.util.Random;
/*      */ import net.minecraft.client.Minecraft;
/*      */ import org.lwjgl.input.Keyboard;
/*      */ import org.lwjgl.input.Mouse;
/*      */ import org.lwjgl.opengl.Display;
/*      */ import org.lwjgl.opengl.DisplayMode;
/*      */ import org.lwjgl.opengl.GL11;
/*      */ import org.lwjgl.opengl.GLContext;
/*      */ import org.lwjgl.util.glu.GLU;
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ public class lr
/*      */ {
/*      */   public static boolean a = false;
/*      */   public static int b;
/*      */   private Minecraft r;
/*      */   private float s;
/*      */   public mn c;
/*      */   private int t;
/*      */   private nn u;
/*      */   private ch v;
/*      */   private ch w;
/*      */   private ch x;
/*      */   private ch y;
/*      */   private ch z;
/*      */   private ch A;
/*      */   private float B;
/*      */   private float C;
/*      */   private float D;
/*      */   private float E;
/*      */   private float F;
/*      */   private float G;
/*      */   private float H;
/*      */   private float I;
/*      */   private float J;
/*      */   private float K;
/*      */   private float L;
/*      */   private float M;
/*      */   private float N;
/*      */   private float O;
/*      */   private float P;
/*      */   public int d;
/*      */   private int[] Q;
/*      */   private float R;
/*      */   private float S;
/*      */   private float T;
/*      */   private boolean U;
/*      */   private double V;
/*      */   private double W;
/*      */   private double X;
/*      */   private long Y;
/*      */   private long Z;
/*      */   private boolean aa;
/*      */   float e;
/*      */   float f;
/*      */   float g;
/*      */   float h;
/*      */   private Random ab;
/*      */   private int ac;
/*      */   float[] i;
/*      */   float[] j;
/*      */   volatile int k;
/*      */   volatile int l;
/*      */   FloatBuffer m;
/*      */   float n;
/*      */   float o;
/*      */   float p;
/*      */   private float ad;
/*      */   private float ae;
/*      */   public int q;
/*  157 */   private xd updatedWorld = null;
/*      */   private boolean showDebugInfo = false;
/*      */   private boolean zoomMode = false;
/*      */   private boolean fullscreenModeChecked = false;
/*      */   private boolean desktopModeChecked = false;
/*  162 */   private String lastTexturePack = null;
/*      */ 
/*      */   
/*      */   public lr(Minecraft par1Minecraft) {
/*  166 */     this.s = 0.0F;
/*  167 */     this.u = null;
/*  168 */     this.v = new ch();
/*  169 */     this.w = new ch();
/*  170 */     this.x = new ch();
/*  171 */     this.y = new ch();
/*  172 */     this.z = new ch();
/*  173 */     this.A = new ch();
/*  174 */     this.B = 4.0F;
/*  175 */     this.C = 4.0F;
/*  176 */     this.D = 0.0F;
/*  177 */     this.E = 0.0F;
/*  178 */     this.F = 0.0F;
/*  179 */     this.G = 0.0F;
/*  180 */     this.M = 0.0F;
/*  181 */     this.N = 0.0F;
/*  182 */     this.O = 0.0F;
/*  183 */     this.P = 0.0F;
/*  184 */     this.U = false;
/*  185 */     this.V = 1.0D;
/*  186 */     this.W = 0.0D;
/*  187 */     this.X = 0.0D;
/*  188 */     this.Y = System.currentTimeMillis();
/*  189 */     this.Z = 0L;
/*  190 */     this.aa = false;
/*  191 */     this.e = 0.0F;
/*  192 */     this.f = 0.0F;
/*  193 */     this.g = 0.0F;
/*  194 */     this.h = 0.0F;
/*  195 */     this.ab = new Random();
/*  196 */     this.ac = 0;
/*  197 */     this.k = 0;
/*  198 */     this.l = 0;
/*  199 */     this.m = ew.e(16);
/*  200 */     this.r = par1Minecraft;
/*  201 */     this.c = new mn(par1Minecraft);
/*  202 */     this.d = par1Minecraft.p.a(new BufferedImage(16, 16, 1));
/*  203 */     this.Q = new int[256];
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a() {
/*  211 */     c();
/*  212 */     d();
/*  213 */     this.ad = this.ae;
/*  214 */     this.C = this.B;
/*  215 */     this.E = this.D;
/*  216 */     this.G = this.F;
/*  217 */     this.N = this.M;
/*  218 */     this.P = this.O;
/*  219 */     if (this.r.A.J) {
/*      */       
/*  221 */       float f = this.r.A.c * 0.6F + 0.2F;
/*  222 */       float f2 = f * f * f * 8.0F;
/*  223 */       this.J = this.v.a(this.H, 0.05F * f2);
/*  224 */       this.K = this.w.a(this.I, 0.05F * f2);
/*  225 */       this.L = 0.0F;
/*  226 */       this.H = 0.0F;
/*  227 */       this.I = 0.0F;
/*      */     } 
/*  229 */     if (this.r.i == null)
/*      */     {
/*  231 */       this.r.i = (acq)this.r.h;
/*      */     }
/*  233 */     float f1 = this.r.f.c(gk.c(this.r.i.o), gk.c(this.r.i.p), gk.c(this.r.i.q));
/*  234 */     float f3 = (3 - this.r.A.e) / 3.0F;
/*  235 */     float f4 = f1 * (1.0F - f3) + f3;
/*  236 */     this.ae += (f4 - this.ae) * 0.1F;
/*  237 */     this.t++;
/*  238 */     this.c.a();
/*  239 */     f();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(float par1) {
/*  247 */     if (this.r.i == null) {
/*      */       return;
/*      */     }
/*      */     
/*  251 */     if (this.r.f == null) {
/*      */       return;
/*      */     }
/*      */     
/*  255 */     double d = this.r.c.b();
/*  256 */     this.r.z = this.r.i.a(d, par1);
/*  257 */     double d1 = d;
/*  258 */     bo vec3d = this.r.i.j(par1);
/*      */ 
/*      */     
/*  261 */     d1 = d = 6.0D;
/*      */ 
/*      */     
/*  264 */     if (d1 > 3.0D)
/*      */     {
/*  266 */       d1 = 3.0D;
/*      */     }
/*  268 */     d = d1;
/*      */     
/*  270 */     if (this.r.z != null)
/*      */     {
/*  272 */       d1 = this.r.z.f.d(vec3d);
/*      */     }
/*  274 */     bo vec3d1 = this.r.i.k(par1);
/*  275 */     bo vec3d2 = vec3d.c(vec3d1.a * d, vec3d1.b * d, vec3d1.c * d);
/*  276 */     this.u = null;
/*  277 */     float f = 1.0F;
/*  278 */     List<nn> list = this.r.f.b(this.r.i, this.r.i.y.a(vec3d1.a * d, vec3d1.b * d, vec3d1.c * d).b(f, f, f));
/*  279 */     double d2 = d1;
/*  280 */     for (int i = 0; i < list.size(); i++) {
/*      */       
/*  282 */       nn entity = list.get(i);
/*  283 */       if (entity.l_()) {
/*      */ 
/*      */ 
/*      */         
/*  287 */         float f1 = entity.j_();
/*  288 */         wu axisalignedbb = entity.y.b(f1, f1, f1);
/*  289 */         pl movingobjectposition = axisalignedbb.a(vec3d, vec3d2);
/*  290 */         if (axisalignedbb.a(vec3d)) {
/*      */           
/*  292 */           if (0.0D < d2 || d2 == 0.0D)
/*      */           {
/*  294 */             this.u = entity;
/*  295 */             d2 = 0.0D;
/*      */           }
/*      */         
/*      */         }
/*  299 */         else if (movingobjectposition != null) {
/*      */ 
/*      */ 
/*      */           
/*  303 */           double d3 = vec3d.d(movingobjectposition.f);
/*  304 */           if (d3 < d2 || d2 == 0.0D) {
/*      */             
/*  306 */             this.u = entity;
/*  307 */             d2 = d3;
/*      */           } 
/*      */         } 
/*      */       } 
/*  311 */     }  if (this.u != null && (d2 < d1 || this.r.z == null))
/*      */     {
/*  313 */       this.r.z = new pl(this.u);
/*      */     }
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void c() {
/*  323 */     if (this.r.i instanceof vq) {
/*      */       
/*  325 */       vq entityplayersp = (vq)this.r.i;
/*  326 */       this.T = entityplayersp.I_();
/*      */     }
/*      */     else {
/*      */       
/*  330 */       this.T = this.r.h.I_();
/*      */     } 
/*      */     
/*  333 */     this.S = this.R;
/*  334 */     this.R += (this.T - this.R) * 0.5F;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private float a(float par1, boolean par2) {
/*  342 */     if (this.q > 0)
/*      */     {
/*  344 */       return 90.0F;
/*      */     }
/*      */     
/*  347 */     acq entityplayer = this.r.i;
/*      */     
/*  349 */     float f = 70.0F;
/*  350 */     if (par2) {
/*      */       
/*  352 */       f += this.r.A.N * 40.0F;
/*  353 */       f *= this.S + (this.R - this.S) * par1;
/*      */     } 
/*      */     
/*  356 */     boolean zoomActive = false;
/*  357 */     if (this.r.A.ofKeyBindZoom.d < 0) {
/*  358 */       zoomActive = Mouse.isButtonDown(this.r.A.ofKeyBindZoom.d + 100);
/*      */     } else {
/*  360 */       zoomActive = Keyboard.isKeyDown(this.r.A.ofKeyBindZoom.d);
/*  361 */     }  if (zoomActive) {
/*      */       
/*  363 */       if (!this.zoomMode) {
/*      */         
/*  365 */         this.zoomMode = true;
/*  366 */         this.r.A.J = true;
/*      */       } 
/*  368 */       if (this.zoomMode) {
/*  369 */         f /= 4.0F;
/*      */       
/*      */       }
/*      */     }
/*  373 */     else if (this.zoomMode) {
/*      */       
/*  375 */       this.zoomMode = false;
/*  376 */       this.r.A.J = false;
/*      */       
/*  378 */       this.v = new ch();
/*  379 */       this.w = new ch();
/*      */     } 
/*      */ 
/*      */     
/*  383 */     if (entityplayer.bb() <= 0) {
/*      */       
/*  385 */       float f1 = entityplayer.bD + par1;
/*  386 */       f /= (1.0F - 500.0F / (f1 + 500.0F)) * 2.0F + 1.0F;
/*      */     } 
/*  388 */     int i = aau.a(this.r.f, entityplayer, par1);
/*  389 */     if (i != 0 && (pb.m[i]).cd == acn.g)
/*      */     {
/*  391 */       f = f * 60.0F / 70.0F;
/*      */     }
/*  393 */     return f + this.N + (this.M - this.N) * par1;
/*      */   }
/*      */ 
/*      */   
/*      */   private void d(float par1) {
/*  398 */     acq entityliving = this.r.i;
/*  399 */     float f = entityliving.bA - par1;
/*  400 */     if (entityliving.bb() <= 0) {
/*      */       
/*  402 */       float f1 = entityliving.bD + par1;
/*  403 */       GL11.glRotatef(40.0F - 8000.0F / (f1 + 200.0F), 0.0F, 0.0F, 1.0F);
/*      */     } 
/*  405 */     if (f < 0.0F) {
/*      */       return;
/*      */     }
/*      */ 
/*      */     
/*  410 */     f /= entityliving.bB;
/*  411 */     f = gk.a(f * f * f * f * 3.1415927F);
/*  412 */     float f2 = entityliving.bC;
/*  413 */     GL11.glRotatef(-f2, 0.0F, 1.0F, 0.0F);
/*  414 */     GL11.glRotatef(-f * 14.0F, 0.0F, 0.0F, 1.0F);
/*  415 */     GL11.glRotatef(f2, 0.0F, 1.0F, 0.0F);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void e(float par1) {
/*  425 */     if (!(this.r.i instanceof yw)) {
/*      */       return;
/*      */     }
/*      */ 
/*      */     
/*  430 */     yw entityplayer = (yw)this.r.i;
/*  431 */     float f = entityplayer.L - entityplayer.K;
/*  432 */     float f1 = -(entityplayer.L + f * par1);
/*  433 */     float f2 = entityplayer.aw + (entityplayer.ax - entityplayer.aw) * par1;
/*  434 */     float f3 = entityplayer.bF + (entityplayer.bG - entityplayer.bF) * par1;
/*  435 */     GL11.glTranslatef(gk.a(f1 * 3.1415927F) * f2 * 0.5F, -Math.abs(gk.b(f1 * 3.1415927F) * f2), 0.0F);
/*  436 */     GL11.glRotatef(gk.a(f1 * 3.1415927F) * f2 * 3.0F, 0.0F, 0.0F, 1.0F);
/*  437 */     GL11.glRotatef(Math.abs(gk.b(f1 * 3.1415927F - 0.2F) * f2) * 5.0F, 1.0F, 0.0F, 0.0F);
/*  438 */     GL11.glRotatef(f3, 1.0F, 0.0F, 0.0F);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void f(float par1) {
/*  448 */     acq entityliving = this.r.i;
/*  449 */     float f = entityliving.H - 1.62F;
/*  450 */     double d = entityliving.l + (entityliving.o - entityliving.l) * par1;
/*  451 */     double d1 = entityliving.m + (entityliving.p - entityliving.m) * par1 - f;
/*  452 */     double d2 = entityliving.n + (entityliving.q - entityliving.n) * par1;
/*  453 */     GL11.glRotatef(this.P + (this.O - this.P) * par1, 0.0F, 0.0F, 1.0F);
/*  454 */     if (entityliving.az()) {
/*      */       
/*  456 */       f = (float)(f + 1.0D);
/*  457 */       GL11.glTranslatef(0.0F, 0.3F, 0.0F);
/*  458 */       if (!this.r.A.K)
/*      */       {
/*  460 */         int i = this.r.f.a(gk.c(entityliving.o), gk.c(entityliving.p), gk.c(entityliving.q));
/*      */         
/*  462 */         boolean isForgeBed = false;
/*  463 */         if (Reflector.hasMethod(51)) {
/*      */           
/*  465 */           int x = gk.c(entityliving.o);
/*  466 */           int y = gk.c(entityliving.p);
/*  467 */           int z = gk.c(entityliving.q);
/*  468 */           pb block = pb.m[this.r.f.a(x, y, z)];
/*  469 */           isForgeBed = Reflector.callBoolean(block, 51, new Object[] { this.r.f, Integer.valueOf(x), Integer.valueOf(y), Integer.valueOf(z), entityliving });
/*      */         } 
/*      */         
/*  472 */         if (i == pb.S.bO || isForgeBed) {
/*      */           
/*  474 */           int j = this.r.f.e(gk.c(entityliving.o), gk.c(entityliving.p), gk.c(entityliving.q));
/*  475 */           int k = j & 0x3;
/*  476 */           GL11.glRotatef((k * 90), 0.0F, 1.0F, 0.0F);
/*      */         } 
/*  478 */         GL11.glRotatef(entityliving.w + (entityliving.u - entityliving.w) * par1 + 180.0F, 0.0F, -1.0F, 0.0F);
/*  479 */         GL11.glRotatef(entityliving.x + (entityliving.v - entityliving.x) * par1, -1.0F, 0.0F, 0.0F);
/*      */       }
/*      */     
/*  482 */     } else if (this.r.A.E > 0) {
/*      */       
/*  484 */       double d3 = (this.C + (this.B - this.C) * par1);
/*  485 */       if (this.r.A.K) {
/*      */         
/*  487 */         float f1 = this.E + (this.D - this.E) * par1;
/*  488 */         float f3 = this.G + (this.F - this.G) * par1;
/*  489 */         GL11.glTranslatef(0.0F, 0.0F, (float)-d3);
/*  490 */         GL11.glRotatef(f3, 1.0F, 0.0F, 0.0F);
/*  491 */         GL11.glRotatef(f1, 0.0F, 1.0F, 0.0F);
/*      */       } else {
/*      */         
/*  494 */         float f2 = entityliving.u;
/*  495 */         float f4 = entityliving.v;
/*  496 */         if (this.r.A.E == 2)
/*      */         {
/*  498 */           f4 += 180.0F;
/*      */         }
/*  500 */         double d4 = (-gk.a(f2 / 180.0F * 3.1415927F) * gk.b(f4 / 180.0F * 3.1415927F)) * d3;
/*  501 */         double d5 = (gk.b(f2 / 180.0F * 3.1415927F) * gk.b(f4 / 180.0F * 3.1415927F)) * d3;
/*  502 */         double d6 = -gk.a(f4 / 180.0F * 3.1415927F) * d3;
/*  503 */         for (int l = 0; l < 8; l++) {
/*      */           
/*  505 */           float f5 = ((l & 0x1) * 2 - 1);
/*  506 */           float f6 = ((l >> 1 & 0x1) * 2 - 1);
/*  507 */           float f7 = ((l >> 2 & 0x1) * 2 - 1);
/*  508 */           f5 *= 0.1F;
/*  509 */           f6 *= 0.1F;
/*  510 */           f7 *= 0.1F;
/*  511 */           pl movingobjectposition = this.r.f.a(bo.b(d + f5, d1 + f6, d2 + f7), bo.b(d - d4 + f5 + f7, d1 - d6 + f6, d2 - d5 + f7));
/*  512 */           if (movingobjectposition != null) {
/*      */ 
/*      */ 
/*      */             
/*  516 */             double d7 = movingobjectposition.f.d(bo.b(d, d1, d2));
/*  517 */             if (d7 < d3)
/*      */             {
/*  519 */               d3 = d7;
/*      */             }
/*      */           } 
/*      */         } 
/*  523 */         if (this.r.A.E == 2)
/*      */         {
/*  525 */           GL11.glRotatef(180.0F, 0.0F, 1.0F, 0.0F);
/*      */         }
/*  527 */         GL11.glRotatef(entityliving.v - f4, 1.0F, 0.0F, 0.0F);
/*  528 */         GL11.glRotatef(entityliving.u - f2, 0.0F, 1.0F, 0.0F);
/*  529 */         GL11.glTranslatef(0.0F, 0.0F, (float)-d3);
/*  530 */         GL11.glRotatef(f2 - entityliving.u, 0.0F, 1.0F, 0.0F);
/*  531 */         GL11.glRotatef(f4 - entityliving.v, 1.0F, 0.0F, 0.0F);
/*      */       } 
/*      */     } else {
/*      */       
/*  535 */       GL11.glTranslatef(0.0F, 0.0F, -0.1F);
/*      */     } 
/*  537 */     if (!this.r.A.K) {
/*      */       
/*  539 */       GL11.glRotatef(entityliving.x + (entityliving.v - entityliving.x) * par1, 1.0F, 0.0F, 0.0F);
/*  540 */       GL11.glRotatef(entityliving.w + (entityliving.u - entityliving.w) * par1 + 180.0F, 0.0F, 1.0F, 0.0F);
/*      */     } 
/*  542 */     GL11.glTranslatef(0.0F, f, 0.0F);
/*  543 */     d = entityliving.l + (entityliving.o - entityliving.l) * par1;
/*  544 */     d1 = entityliving.m + (entityliving.p - entityliving.m) * par1 - f;
/*  545 */     d2 = entityliving.n + (entityliving.q - entityliving.n) * par1;
/*  546 */     this.U = this.r.g.a(d, d1, d2, par1);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void a(float par1, int par2) {
/*  556 */     this.s = (32 << 3 - this.r.A.e);
/*      */     
/*  558 */     this.s = this.r.A.ofRenderDistanceFine;
/*      */     
/*  560 */     if (Config.isFogFancy())
/*      */     {
/*      */       
/*  563 */       this.s *= 0.95F;
/*      */     }
/*  565 */     if (Config.isFogFast())
/*      */     {
/*      */       
/*  568 */       this.s *= 0.83F;
/*      */     }
/*      */ 
/*      */     
/*  572 */     GL11.glMatrixMode(5889);
/*  573 */     GL11.glLoadIdentity();
/*  574 */     float f = 0.07F;
/*  575 */     if (this.r.A.g)
/*      */     {
/*  577 */       GL11.glTranslatef(-(par2 * 2 - 1) * f, 0.0F, 0.0F);
/*      */     }
/*      */     
/*  580 */     float clipDistance = this.s * 2.0F;
/*  581 */     if (clipDistance < 128.0F) {
/*  582 */       clipDistance = 128.0F;
/*      */     }
/*  584 */     if (this.V != 1.0D) {
/*      */       
/*  586 */       GL11.glTranslatef((float)this.W, (float)-this.X, 0.0F);
/*  587 */       GL11.glScaled(this.V, this.V, 1.0D);
/*      */     } 
/*  589 */     GLU.gluPerspective(a(par1, true), this.r.d / this.r.e, 0.05F, clipDistance);
/*  590 */     if (this.r.c.e()) {
/*      */       
/*  592 */       float f1 = 0.6666667F;
/*  593 */       GL11.glScalef(1.0F, f1, 1.0F);
/*      */     } 
/*  595 */     GL11.glMatrixMode(5888);
/*  596 */     GL11.glLoadIdentity();
/*  597 */     if (this.r.A.g)
/*      */     {
/*  599 */       GL11.glTranslatef((par2 * 2 - 1) * 0.1F, 0.0F, 0.0F);
/*      */     }
/*  601 */     d(par1);
/*  602 */     if (this.r.A.f)
/*      */     {
/*  604 */       e(par1);
/*      */     }
/*  606 */     float f2 = this.r.h.aS + (this.r.h.aR - this.r.h.aS) * par1;
/*  607 */     if (f2 > 0.0F) {
/*      */       
/*  609 */       int i = 20;
/*  610 */       if (this.r.h.a(aad.k))
/*      */       {
/*  612 */         i = 7;
/*      */       }
/*  614 */       float f3 = 5.0F / (f2 * f2 + 5.0F) - f2 * 0.04F;
/*  615 */       f3 *= f3;
/*  616 */       GL11.glRotatef((this.t + par1) * i, 0.0F, 1.0F, 1.0F);
/*  617 */       GL11.glScalef(1.0F / f3, 1.0F, 1.0F);
/*  618 */       GL11.glRotatef(-(this.t + par1) * i, 0.0F, 1.0F, 1.0F);
/*      */     } 
/*  620 */     f(par1);
/*  621 */     if (this.q > 0) {
/*      */       
/*  623 */       int j = this.q - 1;
/*  624 */       if (j == 1)
/*      */       {
/*  626 */         GL11.glRotatef(90.0F, 0.0F, 1.0F, 0.0F);
/*      */       }
/*  628 */       if (j == 2)
/*      */       {
/*  630 */         GL11.glRotatef(180.0F, 0.0F, 1.0F, 0.0F);
/*      */       }
/*  632 */       if (j == 3)
/*      */       {
/*  634 */         GL11.glRotatef(-90.0F, 0.0F, 1.0F, 0.0F);
/*      */       }
/*  636 */       if (j == 4)
/*      */       {
/*  638 */         GL11.glRotatef(90.0F, 1.0F, 0.0F, 0.0F);
/*      */       }
/*  640 */       if (j == 5)
/*      */       {
/*  642 */         GL11.glRotatef(-90.0F, 1.0F, 0.0F, 0.0F);
/*      */       }
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void b(float par1, int par2) {
/*  652 */     if (this.q > 0) {
/*      */       return;
/*      */     }
/*      */     
/*  656 */     GL11.glMatrixMode(5889);
/*  657 */     GL11.glLoadIdentity();
/*  658 */     float f = 0.07F;
/*  659 */     if (this.r.A.g)
/*      */     {
/*  661 */       GL11.glTranslatef(-(par2 * 2 - 1) * f, 0.0F, 0.0F);
/*      */     }
/*  663 */     if (this.V != 1.0D) {
/*      */       
/*  665 */       GL11.glTranslatef((float)this.W, (float)-this.X, 0.0F);
/*  666 */       GL11.glScaled(this.V, this.V, 1.0D);
/*      */     } 
/*  668 */     GLU.gluPerspective(a(par1, false), this.r.d / this.r.e, 0.05F, this.s * 2.0F);
/*  669 */     if (this.r.c.e()) {
/*      */       
/*  671 */       float f1 = 0.6666667F;
/*  672 */       GL11.glScalef(1.0F, f1, 1.0F);
/*      */     } 
/*  674 */     GL11.glMatrixMode(5888);
/*  675 */     GL11.glLoadIdentity();
/*  676 */     if (this.r.A.g)
/*      */     {
/*  678 */       GL11.glTranslatef((par2 * 2 - 1) * 0.1F, 0.0F, 0.0F);
/*      */     }
/*  680 */     GL11.glPushMatrix();
/*  681 */     d(par1);
/*  682 */     if (this.r.A.f)
/*      */     {
/*  684 */       e(par1);
/*      */     }
/*  686 */     if (this.r.A.E == 0 && !this.r.i.az() && !this.r.A.D && !this.r.c.e()) {
/*      */       
/*  688 */       b(par1);
/*  689 */       this.c.a(par1);
/*  690 */       a(par1);
/*      */     } 
/*  692 */     GL11.glPopMatrix();
/*  693 */     if (this.r.A.E == 0 && !this.r.i.az()) {
/*      */       
/*  695 */       this.c.b(par1);
/*  696 */       d(par1);
/*      */     } 
/*  698 */     if (this.r.A.f)
/*      */     {
/*  700 */       e(par1);
/*      */     }
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(double par1) {
/*  709 */     es.a(es.b);
/*  710 */     GL11.glDisable(3553);
/*  711 */     es.a(es.a);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void b(double par1) {
/*  719 */     es.a(es.b);
/*  720 */     GL11.glMatrixMode(5890);
/*  721 */     GL11.glLoadIdentity();
/*  722 */     float f = 0.00390625F;
/*  723 */     GL11.glScalef(f, f, f);
/*  724 */     GL11.glTranslatef(8.0F, 8.0F, 8.0F);
/*  725 */     GL11.glMatrixMode(5888);
/*  726 */     this.r.p.b(this.d);
/*      */ 
/*      */ 
/*      */     
/*  730 */     GL11.glTexParameteri(3553, 10241, 9729);
/*  731 */     GL11.glTexParameteri(3553, 10240, 9729);
/*  732 */     GL11.glTexParameteri(3553, 10242, 10496);
/*  733 */     GL11.glTexParameteri(3553, 10243, 10496);
/*  734 */     GL11.glColor4f(1.0F, 1.0F, 1.0F, 1.0F);
/*  735 */     GL11.glEnable(3553);
/*  736 */     es.a(es.a);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void d() {
/*  744 */     this.f = (float)(this.f + (Math.random() - Math.random()) * Math.random() * Math.random());
/*  745 */     this.h = (float)(this.h + (Math.random() - Math.random()) * Math.random() * Math.random());
/*  746 */     this.f = (float)(this.f * 0.9D);
/*  747 */     this.h = (float)(this.h * 0.9D);
/*  748 */     this.e += (this.f - this.e) * 1.0F;
/*  749 */     this.g += (this.h - this.g) * 1.0F;
/*  750 */     this.aa = true;
/*      */   }
/*      */ 
/*      */   
/*      */   private void e() {
/*  755 */     xd world = this.r.f;
/*  756 */     if (world == null) {
/*      */       return;
/*      */     }
/*      */ 
/*      */     
/*  761 */     if (CustomColorizer.updateLightmap(world, this, this.Q)) {
/*      */ 
/*      */       
/*  764 */       this.r.p.a(this.Q, 16, 16, this.d);
/*      */       
/*      */       return;
/*      */     } 
/*      */     
/*  769 */     for (int i = 0; i < 256; i++) {
/*      */       
/*  771 */       float f = world.b(1.0F) * 0.95F + 0.05F;
/*  772 */       float f1 = world.t.f[i / 16] * f;
/*  773 */       float f2 = world.t.f[i % 16] * (this.e * 0.1F + 1.5F);
/*  774 */       if (world.n > 0)
/*      */       {
/*  776 */         f1 = world.t.f[i / 16];
/*      */       }
/*  778 */       float f3 = f1 * (world.b(1.0F) * 0.65F + 0.35F);
/*  779 */       float f4 = f1 * (world.b(1.0F) * 0.65F + 0.35F);
/*  780 */       float f5 = f1;
/*  781 */       float f6 = f2;
/*  782 */       float f7 = f2 * ((f2 * 0.6F + 0.4F) * 0.6F + 0.4F);
/*  783 */       float f8 = f2 * (f2 * f2 * 0.6F + 0.4F);
/*  784 */       float f9 = f3 + f6;
/*  785 */       float f10 = f4 + f7;
/*  786 */       float f11 = f5 + f8;
/*  787 */       f9 = f9 * 0.96F + 0.03F;
/*  788 */       f10 = f10 * 0.96F + 0.03F;
/*  789 */       f11 = f11 * 0.96F + 0.03F;
/*  790 */       if (world.t.g == 1) {
/*      */         
/*  792 */         f9 = 0.22F + f6 * 0.75F;
/*  793 */         f10 = 0.28F + f7 * 0.75F;
/*  794 */         f11 = 0.25F + f8 * 0.75F;
/*      */       } 
/*  796 */       float f12 = this.r.A.O;
/*  797 */       if (f9 > 1.0F)
/*      */       {
/*  799 */         f9 = 1.0F;
/*      */       }
/*  801 */       if (f10 > 1.0F)
/*      */       {
/*  803 */         f10 = 1.0F;
/*      */       }
/*  805 */       if (f11 > 1.0F)
/*      */       {
/*  807 */         f11 = 1.0F;
/*      */       }
/*  809 */       float f13 = 1.0F - f9;
/*  810 */       float f14 = 1.0F - f10;
/*  811 */       float f15 = 1.0F - f11;
/*  812 */       f13 = 1.0F - f13 * f13 * f13 * f13;
/*  813 */       f14 = 1.0F - f14 * f14 * f14 * f14;
/*  814 */       f15 = 1.0F - f15 * f15 * f15 * f15;
/*  815 */       f9 = f9 * (1.0F - f12) + f13 * f12;
/*  816 */       f10 = f10 * (1.0F - f12) + f14 * f12;
/*  817 */       f11 = f11 * (1.0F - f12) + f15 * f12;
/*  818 */       f9 = f9 * 0.96F + 0.03F;
/*  819 */       f10 = f10 * 0.96F + 0.03F;
/*  820 */       f11 = f11 * 0.96F + 0.03F;
/*  821 */       if (f9 > 1.0F)
/*      */       {
/*  823 */         f9 = 1.0F;
/*      */       }
/*  825 */       if (f10 > 1.0F)
/*      */       {
/*  827 */         f10 = 1.0F;
/*      */       }
/*  829 */       if (f11 > 1.0F)
/*      */       {
/*  831 */         f11 = 1.0F;
/*      */       }
/*  833 */       if (f9 < 0.0F)
/*      */       {
/*  835 */         f9 = 0.0F;
/*      */       }
/*  837 */       if (f10 < 0.0F)
/*      */       {
/*  839 */         f10 = 0.0F;
/*      */       }
/*  841 */       if (f11 < 0.0F)
/*      */       {
/*  843 */         f11 = 0.0F;
/*      */       }
/*  845 */       char c = 'ÿ';
/*  846 */       int j = (int)(f9 * 255.0F);
/*  847 */       int k = (int)(f10 * 255.0F);
/*  848 */       int l = (int)(f11 * 255.0F);
/*  849 */       this.Q[i] = c << 24 | j << 16 | k << 8 | l;
/*      */     } 
/*      */     
/*  852 */     this.r.p.a(this.Q, 16, 16, this.d);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void b(float par1) {
/*  860 */     lv.a("lightTex");
/*      */     
/*  862 */     xd world = this.r.f;
/*      */     
/*  864 */     checkDisplayMode();
/*      */     
/*  866 */     if (world != null && Config.getNewRelease() != null) {
/*      */       
/*  868 */       String fullNewVer = "HD " + Config.getNewRelease();
/*  869 */       this.r.w.a("A new §eOptiFine§f version is available: §e" + fullNewVer + "§f");
/*  870 */       Config.setNewRelease(null);
/*      */     } 
/*      */     
/*  873 */     if (this.r.s instanceof xt) {
/*  874 */       updateMainMenu((xt)this.r.s);
/*      */     }
/*  876 */     if (this.updatedWorld != world) {
/*      */       
/*  878 */       RandomMobs.worldChanged(this.updatedWorld, world);
/*      */       
/*  880 */       this.updatedWorld = world;
/*      */     } 
/*      */     
/*  883 */     lv.profilerGlobalEnabled = this.r.A.ofProfiler;
/*      */     
/*  885 */     int targetPrio = 10;
/*  886 */     if (this.r.A.ofSmoothInput)
/*  887 */       targetPrio = 5; 
/*  888 */     if (Thread.currentThread().getPriority() != targetPrio) {
/*  889 */       Thread.currentThread().setPriority(targetPrio);
/*      */     }
/*  891 */     Minecraft.J = 0L;
/*      */     
/*  893 */     if (this.lastTexturePack == null)
/*  894 */       this.lastTexturePack = this.r.E.a.a; 
/*  895 */     if (!this.lastTexturePack.equals(this.r.E.a.a)) {
/*      */       
/*  897 */       this.r.g.a();
/*  898 */       this.lastTexturePack = this.r.E.a.a;
/*      */     } 
/*      */     
/*  901 */     vl.b = (Config.isGrassFancy() || Config.isBetterGrassFancy());
/*  902 */     pb.K.b(Config.isTreesFancy());
/*      */     
/*  904 */     if (Config.getIconWidthTerrain() > 16)
/*      */     {
/*  906 */       if (!(this.c instanceof ItemRendererHD)) {
/*      */         
/*  908 */         this.c = new ItemRendererHD(this.r);
/*  909 */         ahu.a.f = this.c;
/*      */       } 
/*      */     }
/*      */     
/*  913 */     if (world != null) {
/*  914 */       world.p = this.r.A.ofAutoSaveTicks;
/*      */     }
/*  916 */     if (!Config.isWeatherEnabled())
/*      */     {
/*  918 */       if (world != null && world.x != null)
/*      */       {
/*  920 */         world.x.b(false);
/*      */       }
/*      */     }
/*      */     
/*  924 */     if (world != null && !world.F && world.x != null && world.x.q() == 1) {
/*      */       
/*  926 */       long time = world.w();
/*  927 */       long timeOfDay = time % 24000L;
/*  928 */       if (Config.isTimeDayOnly()) {
/*      */         
/*  930 */         if (timeOfDay <= 1000L)
/*  931 */           world.a(time - timeOfDay + 1001L); 
/*  932 */         if (timeOfDay >= 11000L)
/*  933 */           world.a(time - timeOfDay + 24001L); 
/*      */       } 
/*  935 */       if (Config.isTimeNightOnly()) {
/*      */         
/*  937 */         if (timeOfDay <= 14000L)
/*  938 */           world.a(time - timeOfDay + 14001L); 
/*  939 */         if (timeOfDay >= 22000L) {
/*  940 */           world.a(time - timeOfDay + 24000L + 14001L);
/*      */         }
/*      */       } 
/*      */     } 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */     
/*  988 */     if (this.aa)
/*      */     {
/*  990 */       e();
/*      */     }
/*  992 */     lv.b();
/*  993 */     if (!Display.isActive()) {
/*      */       
/*  995 */       if (System.currentTimeMillis() - this.Y > 500L)
/*      */       {
/*  997 */         this.r.i();
/*      */       }
/*      */     } else {
/*      */       
/* 1001 */       this.Y = System.currentTimeMillis();
/*      */     } 
/* 1003 */     lv.a("mouse");
/* 1004 */     if (this.r.R) {
/*      */       
/* 1006 */       this.r.D.c();
/* 1007 */       float f = this.r.A.c * 0.6F + 0.2F;
/* 1008 */       float f1 = f * f * f * 8.0F;
/* 1009 */       float f2 = this.r.D.a * f1;
/* 1010 */       float f3 = this.r.D.b * f1;
/* 1011 */       int l = 1;
/* 1012 */       if (this.r.A.d)
/*      */       {
/* 1014 */         l = -1;
/*      */       }
/* 1016 */       if (this.r.A.J) {
/*      */         
/* 1018 */         this.H += f2;
/* 1019 */         this.I += f3;
/* 1020 */         float f4 = par1 - this.L;
/* 1021 */         this.L = par1;
/* 1022 */         f2 = this.J * f4;
/* 1023 */         f3 = this.K * f4;
/* 1024 */         this.r.h.c(f2, f3 * l);
/*      */       } else {
/*      */         
/* 1027 */         this.r.h.c(f2, f3 * l);
/*      */       } 
/*      */     } 
/* 1030 */     lv.b();
/* 1031 */     if (this.r.x) {
/*      */       return;
/*      */     }
/*      */     
/* 1035 */     a = this.r.A.g;
/* 1036 */     agd scaledresolution = new agd(this.r.A, this.r.d, this.r.e);
/* 1037 */     int i = scaledresolution.a();
/* 1038 */     int j = scaledresolution.b();
/* 1039 */     int k = Mouse.getX() * i / this.r.d;
/* 1040 */     int i1 = j - Mouse.getY() * j / this.r.e - 1;
/*      */     
/* 1042 */     char fpsLimit = 'È';
/* 1043 */     if (this.r.A.i == 1)
/* 1044 */       fpsLimit = 'x'; 
/* 1045 */     if (this.r.A.i == 2)
/* 1046 */       fpsLimit = '('; 
/* 1047 */     if (this.r.f != null) {
/*      */       
/* 1049 */       lv.a("level");
/* 1050 */       if (this.r.A.i == 0) {
/*      */         
/* 1052 */         a(par1, 0L);
/*      */       } else {
/*      */         
/* 1055 */         a(par1, this.Z + (long)(1.0E9D / fpsLimit));
/*      */       } 
/* 1057 */       lv.c("sleep");
/* 1058 */       if (this.r.A.i == 2) {
/*      */         
/* 1060 */         long l1 = (this.Z + (long)(1.0E9D / fpsLimit) - System.nanoTime()) / 1000000L;
/* 1061 */         if (l1 > 0L && l1 < 500L) {
/*      */           
/*      */           try {
/*      */             
/* 1065 */             Thread.sleep(l1);
/*      */           }
/* 1067 */           catch (InterruptedException interruptedexception) {
/*      */             
/* 1069 */             interruptedexception.printStackTrace();
/*      */           } 
/*      */         }
/*      */       } 
/* 1073 */       this.Z = System.nanoTime();
/* 1074 */       lv.c("gui");
/* 1075 */       if (!this.r.A.D || this.r.s != null) {
/*      */ 
/*      */         
/* 1078 */         if (this.r.A.ofFastDebugInfo) {
/*      */           
/* 1080 */           if (Minecraft.u()) {
/* 1081 */             this.showDebugInfo = !this.showDebugInfo;
/*      */           }
/* 1083 */           if (this.showDebugInfo) {
/* 1084 */             this.r.A.F = true;
/*      */           }
/*      */         } 
/* 1087 */         this.r.w.a(par1, (this.r.s != null), k, i1);
/*      */         
/* 1089 */         if (this.r.A.ofFastDebugInfo)
/*      */         {
/*      */           
/* 1092 */           this.r.A.F = false;
/*      */         }
/*      */       } 
/* 1095 */       lv.b();
/*      */     } else {
/*      */       
/* 1098 */       GL11.glViewport(0, 0, this.r.d, this.r.e);
/* 1099 */       GL11.glMatrixMode(5889);
/* 1100 */       GL11.glLoadIdentity();
/* 1101 */       GL11.glMatrixMode(5888);
/* 1102 */       GL11.glLoadIdentity();
/* 1103 */       b();
/* 1104 */       long l2 = (this.Z + (1000000000 / fpsLimit) - System.nanoTime()) / 1000000L;
/* 1105 */       if (l2 < 0L)
/*      */       {
/* 1107 */         l2 += 10L;
/*      */       }
/* 1109 */       if (l2 > 0L && l2 < 500L) {
/*      */         
/*      */         try {
/*      */           
/* 1113 */           Thread.sleep(l2);
/*      */         }
/* 1115 */         catch (InterruptedException interruptedexception1) {
/*      */           
/* 1117 */           interruptedexception1.printStackTrace();
/*      */         } 
/*      */       }
/* 1120 */       this.Z = System.nanoTime();
/*      */     } 
/* 1122 */     if (this.r.s != null) {
/*      */       
/* 1124 */       GL11.glClear(256);
/* 1125 */       this.r.s.a(k, i1, par1);
/* 1126 */       if (this.r.s != null && this.r.s.v != null)
/*      */       {
/* 1128 */         this.r.s.v.a(par1);
/*      */       }
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void updateMainMenu(xt mainGui) {
/*      */     try {
/* 1138 */       String str = null;
/* 1139 */       Calendar calendar = Calendar.getInstance();
/* 1140 */       calendar.setTime(new Date());
/* 1141 */       int day = calendar.get(5);
/* 1142 */       int month = calendar.get(2) + 1;
/*      */       
/* 1144 */       if (day == 8 && month == 4) {
/* 1145 */         str = "Happy birthday, OptiFine!";
/*      */       }
/* 1147 */       if (day == 14 && month == 8) {
/* 1148 */         str = "Happy birthday, sp614x!";
/*      */       }
/* 1150 */       if (str == null) {
/*      */         return;
/*      */       }
/* 1153 */       Field[] fs = xt.class.getDeclaredFields();
/* 1154 */       for (int i = 0; i < fs.length; i++) {
/*      */         
/* 1156 */         if (fs[i].getType() == String.class) {
/*      */           
/* 1158 */           fs[i].setAccessible(true);
/* 1159 */           fs[i].set(mainGui, str);
/*      */           
/*      */           break;
/*      */         } 
/*      */       } 
/* 1164 */     } catch (Throwable e) {}
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void checkDisplayMode() {
/*      */     try {
/* 1177 */       if (Display.isFullscreen())
/*      */       {
/* 1179 */         if (this.fullscreenModeChecked) {
/*      */           return;
/*      */         }
/* 1182 */         this.fullscreenModeChecked = true;
/* 1183 */         this.desktopModeChecked = false;
/*      */         
/* 1185 */         DisplayMode mode = Display.getDisplayMode();
/*      */         
/* 1187 */         Dimension dim = Config.getFullscreenDimension();
/*      */         
/* 1189 */         if (mode.getWidth() == dim.width && mode.getHeight() == dim.height) {
/*      */           return;
/*      */         }
/* 1192 */         DisplayMode newMode = Config.getDisplayMode(dim);
/* 1193 */         Display.setDisplayMode(newMode);
/*      */         
/* 1195 */         this.r.d = Display.getDisplayMode().getWidth();
/* 1196 */         this.r.e = Display.getDisplayMode().getHeight();
/* 1197 */         if (this.r.d <= 0)
/* 1198 */           this.r.d = 1; 
/* 1199 */         if (this.r.e <= 0) {
/* 1200 */           this.r.e = 1;
/*      */         }
/* 1202 */         Display.setFullscreen(true);
/* 1203 */         Display.update();
/* 1204 */         GL11.glEnable(3553);
/*      */       }
/*      */       else
/*      */       {
/* 1208 */         if (this.desktopModeChecked) {
/*      */           return;
/*      */         }
/* 1211 */         this.desktopModeChecked = true;
/* 1212 */         this.fullscreenModeChecked = false;
/*      */         
/* 1214 */         if (Config.getDesktopDisplayMode() == null) {
/* 1215 */           Config.setDesktopDisplayMode(Display.getDesktopDisplayMode());
/*      */         }
/* 1217 */         DisplayMode mode = Display.getDisplayMode();
/*      */         
/* 1219 */         if (mode.equals(Config.getDesktopDisplayMode())) {
/*      */           return;
/*      */         }
/* 1222 */         Display.setDisplayMode(Config.getDesktopDisplayMode());
/*      */         
/* 1224 */         if (this.r.m != null) {
/*      */           
/* 1226 */           this.r.d = this.r.m.getWidth();
/* 1227 */           this.r.e = this.r.m.getHeight();
/*      */         } 
/* 1229 */         if (this.r.d <= 0)
/* 1230 */           this.r.d = 1; 
/* 1231 */         if (this.r.e <= 0) {
/* 1232 */           this.r.e = 1;
/*      */         }
/* 1234 */         Display.setFullscreen(false);
/* 1235 */         Display.update();
/* 1236 */         GL11.glEnable(3553);
/*      */       }
/*      */     
/* 1239 */     } catch (Exception e) {
/*      */       
/* 1241 */       e.printStackTrace();
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(float par1, long par2) {
/*      */     // Byte code:
/*      */     //   0: ldc_w 'lightTex'
/*      */     //   3: invokestatic a : (Ljava/lang/String;)V
/*      */     //   6: aload_0
/*      */     //   7: getfield aa : Z
/*      */     //   10: ifeq -> 17
/*      */     //   13: aload_0
/*      */     //   14: invokespecial e : ()V
/*      */     //   17: sipush #2884
/*      */     //   20: invokestatic glEnable : (I)V
/*      */     //   23: sipush #2929
/*      */     //   26: invokestatic glEnable : (I)V
/*      */     //   29: aload_0
/*      */     //   30: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   33: getfield i : Lacq;
/*      */     //   36: ifnonnull -> 53
/*      */     //   39: aload_0
/*      */     //   40: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   43: aload_0
/*      */     //   44: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   47: getfield h : Lvq;
/*      */     //   50: putfield i : Lacq;
/*      */     //   53: ldc_w 'pick'
/*      */     //   56: invokestatic c : (Ljava/lang/String;)V
/*      */     //   59: aload_0
/*      */     //   60: fload_1
/*      */     //   61: invokevirtual a : (F)V
/*      */     //   64: aload_0
/*      */     //   65: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   68: getfield i : Lacq;
/*      */     //   71: astore #4
/*      */     //   73: aload_0
/*      */     //   74: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   77: getfield g : Ll;
/*      */     //   80: astore #5
/*      */     //   82: aload_0
/*      */     //   83: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   86: getfield j : Lcw;
/*      */     //   89: astore #6
/*      */     //   91: aload #4
/*      */     //   93: getfield N : D
/*      */     //   96: aload #4
/*      */     //   98: getfield o : D
/*      */     //   101: aload #4
/*      */     //   103: getfield N : D
/*      */     //   106: dsub
/*      */     //   107: fload_1
/*      */     //   108: f2d
/*      */     //   109: dmul
/*      */     //   110: dadd
/*      */     //   111: dstore #7
/*      */     //   113: aload #4
/*      */     //   115: getfield O : D
/*      */     //   118: aload #4
/*      */     //   120: getfield p : D
/*      */     //   123: aload #4
/*      */     //   125: getfield O : D
/*      */     //   128: dsub
/*      */     //   129: fload_1
/*      */     //   130: f2d
/*      */     //   131: dmul
/*      */     //   132: dadd
/*      */     //   133: dstore #9
/*      */     //   135: aload #4
/*      */     //   137: getfield P : D
/*      */     //   140: aload #4
/*      */     //   142: getfield q : D
/*      */     //   145: aload #4
/*      */     //   147: getfield P : D
/*      */     //   150: dsub
/*      */     //   151: fload_1
/*      */     //   152: f2d
/*      */     //   153: dmul
/*      */     //   154: dadd
/*      */     //   155: dstore #11
/*      */     //   157: ldc_w 'center'
/*      */     //   160: invokestatic c : (Ljava/lang/String;)V
/*      */     //   163: aload_0
/*      */     //   164: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   167: getfield f : Lxd;
/*      */     //   170: invokevirtual z : ()Lca;
/*      */     //   173: astore #13
/*      */     //   175: aload #13
/*      */     //   177: instanceof hy
/*      */     //   180: ifeq -> 221
/*      */     //   183: aload #13
/*      */     //   185: checkcast hy
/*      */     //   188: astore #14
/*      */     //   190: dload #7
/*      */     //   192: d2i
/*      */     //   193: i2f
/*      */     //   194: invokestatic d : (F)I
/*      */     //   197: iconst_4
/*      */     //   198: ishr
/*      */     //   199: istore #15
/*      */     //   201: dload #11
/*      */     //   203: d2i
/*      */     //   204: i2f
/*      */     //   205: invokestatic d : (F)I
/*      */     //   208: iconst_4
/*      */     //   209: ishr
/*      */     //   210: istore #16
/*      */     //   212: aload #14
/*      */     //   214: iload #15
/*      */     //   216: iload #16
/*      */     //   218: invokevirtual d : (II)V
/*      */     //   221: iconst_0
/*      */     //   222: istore #14
/*      */     //   224: iload #14
/*      */     //   226: iconst_2
/*      */     //   227: if_icmpge -> 1460
/*      */     //   230: aload_0
/*      */     //   231: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   234: getfield A : Lhu;
/*      */     //   237: getfield g : Z
/*      */     //   240: ifeq -> 271
/*      */     //   243: iload #14
/*      */     //   245: putstatic lr.b : I
/*      */     //   248: getstatic lr.b : I
/*      */     //   251: ifne -> 264
/*      */     //   254: iconst_0
/*      */     //   255: iconst_1
/*      */     //   256: iconst_1
/*      */     //   257: iconst_0
/*      */     //   258: invokestatic glColorMask : (ZZZZ)V
/*      */     //   261: goto -> 271
/*      */     //   264: iconst_1
/*      */     //   265: iconst_0
/*      */     //   266: iconst_0
/*      */     //   267: iconst_0
/*      */     //   268: invokestatic glColorMask : (ZZZZ)V
/*      */     //   271: ldc_w 'clear'
/*      */     //   274: invokestatic c : (Ljava/lang/String;)V
/*      */     //   277: iconst_0
/*      */     //   278: iconst_0
/*      */     //   279: aload_0
/*      */     //   280: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   283: getfield d : I
/*      */     //   286: aload_0
/*      */     //   287: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   290: getfield e : I
/*      */     //   293: invokestatic glViewport : (IIII)V
/*      */     //   296: aload_0
/*      */     //   297: fload_1
/*      */     //   298: invokespecial g : (F)V
/*      */     //   301: sipush #16640
/*      */     //   304: invokestatic glClear : (I)V
/*      */     //   307: sipush #2884
/*      */     //   310: invokestatic glEnable : (I)V
/*      */     //   313: ldc_w 'camera'
/*      */     //   316: invokestatic c : (Ljava/lang/String;)V
/*      */     //   319: aload_0
/*      */     //   320: fload_1
/*      */     //   321: iload #14
/*      */     //   323: invokespecial a : (FI)V
/*      */     //   326: aload_0
/*      */     //   327: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   330: getfield h : Lvq;
/*      */     //   333: aload_0
/*      */     //   334: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   337: getfield A : Lhu;
/*      */     //   340: getfield E : I
/*      */     //   343: iconst_2
/*      */     //   344: if_icmpne -> 351
/*      */     //   347: iconst_1
/*      */     //   348: goto -> 352
/*      */     //   351: iconst_0
/*      */     //   352: invokestatic a : (Lyw;Z)V
/*      */     //   355: ldc_w 'frustrum'
/*      */     //   358: invokestatic c : (Ljava/lang/String;)V
/*      */     //   361: invokestatic a : ()Lwc;
/*      */     //   364: pop
/*      */     //   365: invokestatic isSkyEnabled : ()Z
/*      */     //   368: ifne -> 383
/*      */     //   371: invokestatic isSunMoonEnabled : ()Z
/*      */     //   374: ifne -> 383
/*      */     //   377: invokestatic isStarsEnabled : ()Z
/*      */     //   380: ifeq -> 404
/*      */     //   383: aload_0
/*      */     //   384: iconst_m1
/*      */     //   385: fload_1
/*      */     //   386: invokespecial a : (IF)V
/*      */     //   389: ldc_w 'sky'
/*      */     //   392: invokestatic c : (Ljava/lang/String;)V
/*      */     //   395: aload #5
/*      */     //   397: fload_1
/*      */     //   398: invokevirtual a : (F)V
/*      */     //   401: goto -> 410
/*      */     //   404: sipush #3042
/*      */     //   407: invokestatic glDisable : (I)V
/*      */     //   410: sipush #2912
/*      */     //   413: invokestatic glEnable : (I)V
/*      */     //   416: aload_0
/*      */     //   417: iconst_1
/*      */     //   418: fload_1
/*      */     //   419: invokespecial a : (IF)V
/*      */     //   422: aload_0
/*      */     //   423: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   426: getfield A : Lhu;
/*      */     //   429: getfield k : Z
/*      */     //   432: ifeq -> 441
/*      */     //   435: sipush #7425
/*      */     //   438: invokestatic glShadeModel : (I)V
/*      */     //   441: ldc_w 'culling'
/*      */     //   444: invokestatic c : (Ljava/lang/String;)V
/*      */     //   447: new nq
/*      */     //   450: dup
/*      */     //   451: invokespecial <init> : ()V
/*      */     //   454: astore #15
/*      */     //   456: aload #15
/*      */     //   458: dload #7
/*      */     //   460: dload #9
/*      */     //   462: dload #11
/*      */     //   464: invokevirtual a : (DDD)V
/*      */     //   467: aload_0
/*      */     //   468: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   471: getfield g : Ll;
/*      */     //   474: aload #15
/*      */     //   476: fload_1
/*      */     //   477: invokevirtual a : (Lamd;F)V
/*      */     //   480: iload #14
/*      */     //   482: ifne -> 540
/*      */     //   485: ldc_w 'updatechunks'
/*      */     //   488: invokestatic c : (Ljava/lang/String;)V
/*      */     //   491: aload_0
/*      */     //   492: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   495: getfield g : Ll;
/*      */     //   498: aload #4
/*      */     //   500: iconst_0
/*      */     //   501: invokevirtual a : (Lacq;Z)Z
/*      */     //   504: ifne -> 540
/*      */     //   507: lload_2
/*      */     //   508: lconst_0
/*      */     //   509: lcmp
/*      */     //   510: ifne -> 516
/*      */     //   513: goto -> 540
/*      */     //   516: lload_2
/*      */     //   517: invokestatic nanoTime : ()J
/*      */     //   520: lsub
/*      */     //   521: lstore #16
/*      */     //   523: lload #16
/*      */     //   525: lconst_0
/*      */     //   526: lcmp
/*      */     //   527: iflt -> 540
/*      */     //   530: lload #16
/*      */     //   532: l2d
/*      */     //   533: ldc2_w 1.0E9
/*      */     //   536: dcmpg
/*      */     //   537: ifle -> 491
/*      */     //   540: aload_0
/*      */     //   541: iconst_0
/*      */     //   542: fload_1
/*      */     //   543: invokespecial a : (IF)V
/*      */     //   546: sipush #2912
/*      */     //   549: invokestatic glEnable : (I)V
/*      */     //   552: sipush #3553
/*      */     //   555: aload_0
/*      */     //   556: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   559: getfield p : Laaw;
/*      */     //   562: ldc_w '/terrain.png'
/*      */     //   565: invokevirtual b : (Ljava/lang/String;)I
/*      */     //   568: invokestatic glBindTexture : (II)V
/*      */     //   571: invokestatic a : ()V
/*      */     //   574: invokestatic isUseAlphaFunc : ()Z
/*      */     //   577: ifeq -> 589
/*      */     //   580: sipush #516
/*      */     //   583: invokestatic getAlphaFuncLevel : ()F
/*      */     //   586: invokestatic glAlphaFunc : (IF)V
/*      */     //   589: ldc_w 'terrain'
/*      */     //   592: invokestatic c : (Ljava/lang/String;)V
/*      */     //   595: aload #5
/*      */     //   597: aload #4
/*      */     //   599: iconst_0
/*      */     //   600: fload_1
/*      */     //   601: f2d
/*      */     //   602: invokevirtual a : (Lacq;ID)I
/*      */     //   605: pop
/*      */     //   606: sipush #7424
/*      */     //   609: invokestatic glShadeModel : (I)V
/*      */     //   612: iconst_1
/*      */     //   613: invokestatic hasClass : (I)Z
/*      */     //   616: istore #16
/*      */     //   618: aload_0
/*      */     //   619: getfield q : I
/*      */     //   622: ifne -> 888
/*      */     //   625: invokestatic b : ()V
/*      */     //   628: ldc_w 'entities'
/*      */     //   631: invokestatic c : (Ljava/lang/String;)V
/*      */     //   634: aload #5
/*      */     //   636: aload #4
/*      */     //   638: fload_1
/*      */     //   639: invokevirtual j : (F)Lbo;
/*      */     //   642: aload #15
/*      */     //   644: fload_1
/*      */     //   645: invokevirtual a : (Lbo;Lamd;F)V
/*      */     //   648: aload_0
/*      */     //   649: fload_1
/*      */     //   650: f2d
/*      */     //   651: invokevirtual b : (D)V
/*      */     //   654: ldc_w 'litParticles'
/*      */     //   657: invokestatic c : (Ljava/lang/String;)V
/*      */     //   660: aload #6
/*      */     //   662: aload #4
/*      */     //   664: fload_1
/*      */     //   665: invokevirtual b : (Lnn;F)V
/*      */     //   668: invokestatic a : ()V
/*      */     //   671: aload_0
/*      */     //   672: iconst_0
/*      */     //   673: fload_1
/*      */     //   674: invokespecial a : (IF)V
/*      */     //   677: ldc_w 'particles'
/*      */     //   680: invokestatic c : (Ljava/lang/String;)V
/*      */     //   683: aload #6
/*      */     //   685: aload #4
/*      */     //   687: fload_1
/*      */     //   688: invokevirtual a : (Lnn;F)V
/*      */     //   691: aload_0
/*      */     //   692: fload_1
/*      */     //   693: f2d
/*      */     //   694: invokevirtual a : (D)V
/*      */     //   697: aload_0
/*      */     //   698: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   701: getfield z : Lpl;
/*      */     //   704: ifnull -> 888
/*      */     //   707: aload #4
/*      */     //   709: getstatic acn.g : Lacn;
/*      */     //   712: invokevirtual a : (Lacn;)Z
/*      */     //   715: ifeq -> 888
/*      */     //   718: aload #4
/*      */     //   720: instanceof yw
/*      */     //   723: ifeq -> 888
/*      */     //   726: aload_0
/*      */     //   727: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   730: getfield A : Lhu;
/*      */     //   733: getfield D : Z
/*      */     //   736: ifne -> 888
/*      */     //   739: aload #4
/*      */     //   741: checkcast yw
/*      */     //   744: astore #17
/*      */     //   746: sipush #3008
/*      */     //   749: invokestatic glDisable : (I)V
/*      */     //   752: ldc_w 'outline'
/*      */     //   755: invokestatic c : (Ljava/lang/String;)V
/*      */     //   758: iload #16
/*      */     //   760: ifeq -> 821
/*      */     //   763: bipush #10
/*      */     //   765: bipush #6
/*      */     //   767: anewarray java/lang/Object
/*      */     //   770: dup
/*      */     //   771: iconst_0
/*      */     //   772: aload #5
/*      */     //   774: aastore
/*      */     //   775: dup
/*      */     //   776: iconst_1
/*      */     //   777: aload #17
/*      */     //   779: aastore
/*      */     //   780: dup
/*      */     //   781: iconst_2
/*      */     //   782: aload_0
/*      */     //   783: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   786: getfield z : Lpl;
/*      */     //   789: aastore
/*      */     //   790: dup
/*      */     //   791: iconst_3
/*      */     //   792: iconst_0
/*      */     //   793: invokestatic valueOf : (I)Ljava/lang/Integer;
/*      */     //   796: aastore
/*      */     //   797: dup
/*      */     //   798: iconst_4
/*      */     //   799: aload #17
/*      */     //   801: getfield ap : Laak;
/*      */     //   804: invokevirtual b : ()Laan;
/*      */     //   807: aastore
/*      */     //   808: dup
/*      */     //   809: iconst_5
/*      */     //   810: fload_1
/*      */     //   811: invokestatic valueOf : (F)Ljava/lang/Float;
/*      */     //   814: aastore
/*      */     //   815: invokestatic callBoolean : (I[Ljava/lang/Object;)Z
/*      */     //   818: ifne -> 882
/*      */     //   821: aload #5
/*      */     //   823: aload #17
/*      */     //   825: aload_0
/*      */     //   826: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   829: getfield z : Lpl;
/*      */     //   832: iconst_0
/*      */     //   833: aload #17
/*      */     //   835: getfield ap : Laak;
/*      */     //   838: invokevirtual b : ()Laan;
/*      */     //   841: fload_1
/*      */     //   842: invokevirtual a : (Lyw;Lpl;ILaan;F)V
/*      */     //   845: aload_0
/*      */     //   846: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   849: getfield A : Lhu;
/*      */     //   852: getfield D : Z
/*      */     //   855: ifne -> 882
/*      */     //   858: aload #5
/*      */     //   860: aload #17
/*      */     //   862: aload_0
/*      */     //   863: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   866: getfield z : Lpl;
/*      */     //   869: iconst_0
/*      */     //   870: aload #17
/*      */     //   872: getfield ap : Laak;
/*      */     //   875: invokevirtual b : ()Laan;
/*      */     //   878: fload_1
/*      */     //   879: invokevirtual b : (Lyw;Lpl;ILaan;F)V
/*      */     //   882: sipush #3008
/*      */     //   885: invokestatic glEnable : (I)V
/*      */     //   888: sipush #3042
/*      */     //   891: invokestatic glDisable : (I)V
/*      */     //   894: sipush #2884
/*      */     //   897: invokestatic glEnable : (I)V
/*      */     //   900: sipush #770
/*      */     //   903: sipush #771
/*      */     //   906: invokestatic glBlendFunc : (II)V
/*      */     //   909: iconst_1
/*      */     //   910: invokestatic glDepthMask : (Z)V
/*      */     //   913: aload_0
/*      */     //   914: iconst_0
/*      */     //   915: fload_1
/*      */     //   916: invokespecial a : (IF)V
/*      */     //   919: sipush #3042
/*      */     //   922: invokestatic glEnable : (I)V
/*      */     //   925: sipush #2884
/*      */     //   928: invokestatic glDisable : (I)V
/*      */     //   931: sipush #3553
/*      */     //   934: aload_0
/*      */     //   935: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   938: getfield p : Laaw;
/*      */     //   941: ldc_w '/terrain.png'
/*      */     //   944: invokevirtual b : (Ljava/lang/String;)I
/*      */     //   947: invokestatic glBindTexture : (II)V
/*      */     //   950: invokestatic isWaterFancy : ()Z
/*      */     //   953: ifeq -> 1067
/*      */     //   956: ldc_w 'water'
/*      */     //   959: invokestatic c : (Ljava/lang/String;)V
/*      */     //   962: aload_0
/*      */     //   963: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   966: getfield A : Lhu;
/*      */     //   969: getfield k : Z
/*      */     //   972: ifeq -> 981
/*      */     //   975: sipush #7425
/*      */     //   978: invokestatic glShadeModel : (I)V
/*      */     //   981: iconst_0
/*      */     //   982: iconst_0
/*      */     //   983: iconst_0
/*      */     //   984: iconst_0
/*      */     //   985: invokestatic glColorMask : (ZZZZ)V
/*      */     //   988: aload #5
/*      */     //   990: iconst_1
/*      */     //   991: fload_1
/*      */     //   992: f2d
/*      */     //   993: invokevirtual renderAllSortedRenderers : (ID)I
/*      */     //   996: istore #17
/*      */     //   998: aload_0
/*      */     //   999: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   1002: getfield A : Lhu;
/*      */     //   1005: getfield g : Z
/*      */     //   1008: ifeq -> 1037
/*      */     //   1011: getstatic lr.b : I
/*      */     //   1014: ifne -> 1027
/*      */     //   1017: iconst_0
/*      */     //   1018: iconst_1
/*      */     //   1019: iconst_1
/*      */     //   1020: iconst_1
/*      */     //   1021: invokestatic glColorMask : (ZZZZ)V
/*      */     //   1024: goto -> 1044
/*      */     //   1027: iconst_1
/*      */     //   1028: iconst_0
/*      */     //   1029: iconst_0
/*      */     //   1030: iconst_1
/*      */     //   1031: invokestatic glColorMask : (ZZZZ)V
/*      */     //   1034: goto -> 1044
/*      */     //   1037: iconst_1
/*      */     //   1038: iconst_1
/*      */     //   1039: iconst_1
/*      */     //   1040: iconst_1
/*      */     //   1041: invokestatic glColorMask : (ZZZZ)V
/*      */     //   1044: iload #17
/*      */     //   1046: ifle -> 1058
/*      */     //   1049: aload #5
/*      */     //   1051: iconst_1
/*      */     //   1052: fload_1
/*      */     //   1053: f2d
/*      */     //   1054: invokevirtual renderAllSortedRenderers : (ID)I
/*      */     //   1057: pop
/*      */     //   1058: sipush #7424
/*      */     //   1061: invokestatic glShadeModel : (I)V
/*      */     //   1064: goto -> 1082
/*      */     //   1067: ldc_w 'water'
/*      */     //   1070: invokestatic c : (Ljava/lang/String;)V
/*      */     //   1073: aload #5
/*      */     //   1075: iconst_1
/*      */     //   1076: fload_1
/*      */     //   1077: f2d
/*      */     //   1078: invokevirtual renderAllSortedRenderers : (ID)I
/*      */     //   1081: pop
/*      */     //   1082: iconst_1
/*      */     //   1083: invokestatic glDepthMask : (Z)V
/*      */     //   1086: sipush #2884
/*      */     //   1089: invokestatic glEnable : (I)V
/*      */     //   1092: sipush #3042
/*      */     //   1095: invokestatic glDisable : (I)V
/*      */     //   1098: aload_0
/*      */     //   1099: getfield V : D
/*      */     //   1102: dconst_1
/*      */     //   1103: dcmpl
/*      */     //   1104: ifne -> 1298
/*      */     //   1107: aload #4
/*      */     //   1109: instanceof yw
/*      */     //   1112: ifeq -> 1298
/*      */     //   1115: aload_0
/*      */     //   1116: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   1119: getfield A : Lhu;
/*      */     //   1122: getfield D : Z
/*      */     //   1125: ifne -> 1298
/*      */     //   1128: aload_0
/*      */     //   1129: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   1132: getfield z : Lpl;
/*      */     //   1135: ifnull -> 1298
/*      */     //   1138: aload #4
/*      */     //   1140: getstatic acn.g : Lacn;
/*      */     //   1143: invokevirtual a : (Lacn;)Z
/*      */     //   1146: ifne -> 1298
/*      */     //   1149: aload #4
/*      */     //   1151: checkcast yw
/*      */     //   1154: astore #17
/*      */     //   1156: sipush #3008
/*      */     //   1159: invokestatic glDisable : (I)V
/*      */     //   1162: ldc_w 'outline'
/*      */     //   1165: invokestatic c : (Ljava/lang/String;)V
/*      */     //   1168: iload #16
/*      */     //   1170: ifeq -> 1231
/*      */     //   1173: bipush #10
/*      */     //   1175: bipush #6
/*      */     //   1177: anewarray java/lang/Object
/*      */     //   1180: dup
/*      */     //   1181: iconst_0
/*      */     //   1182: aload #5
/*      */     //   1184: aastore
/*      */     //   1185: dup
/*      */     //   1186: iconst_1
/*      */     //   1187: aload #17
/*      */     //   1189: aastore
/*      */     //   1190: dup
/*      */     //   1191: iconst_2
/*      */     //   1192: aload_0
/*      */     //   1193: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   1196: getfield z : Lpl;
/*      */     //   1199: aastore
/*      */     //   1200: dup
/*      */     //   1201: iconst_3
/*      */     //   1202: iconst_0
/*      */     //   1203: invokestatic valueOf : (I)Ljava/lang/Integer;
/*      */     //   1206: aastore
/*      */     //   1207: dup
/*      */     //   1208: iconst_4
/*      */     //   1209: aload #17
/*      */     //   1211: getfield ap : Laak;
/*      */     //   1214: invokevirtual b : ()Laan;
/*      */     //   1217: aastore
/*      */     //   1218: dup
/*      */     //   1219: iconst_5
/*      */     //   1220: fload_1
/*      */     //   1221: invokestatic valueOf : (F)Ljava/lang/Float;
/*      */     //   1224: aastore
/*      */     //   1225: invokestatic callBoolean : (I[Ljava/lang/Object;)Z
/*      */     //   1228: ifne -> 1292
/*      */     //   1231: aload #5
/*      */     //   1233: aload #17
/*      */     //   1235: aload_0
/*      */     //   1236: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   1239: getfield z : Lpl;
/*      */     //   1242: iconst_0
/*      */     //   1243: aload #17
/*      */     //   1245: getfield ap : Laak;
/*      */     //   1248: invokevirtual b : ()Laan;
/*      */     //   1251: fload_1
/*      */     //   1252: invokevirtual a : (Lyw;Lpl;ILaan;F)V
/*      */     //   1255: aload_0
/*      */     //   1256: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   1259: getfield A : Lhu;
/*      */     //   1262: getfield D : Z
/*      */     //   1265: ifne -> 1292
/*      */     //   1268: aload #5
/*      */     //   1270: aload #17
/*      */     //   1272: aload_0
/*      */     //   1273: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   1276: getfield z : Lpl;
/*      */     //   1279: iconst_0
/*      */     //   1280: aload #17
/*      */     //   1282: getfield ap : Laak;
/*      */     //   1285: invokevirtual b : ()Laan;
/*      */     //   1288: fload_1
/*      */     //   1289: invokevirtual b : (Lyw;Lpl;ILaan;F)V
/*      */     //   1292: sipush #3008
/*      */     //   1295: invokestatic glEnable : (I)V
/*      */     //   1298: ldc_w 'weather'
/*      */     //   1301: invokestatic c : (Ljava/lang/String;)V
/*      */     //   1304: aload_0
/*      */     //   1305: fload_1
/*      */     //   1306: invokevirtual c : (F)V
/*      */     //   1309: sipush #2912
/*      */     //   1312: invokestatic glDisable : (I)V
/*      */     //   1315: aload_0
/*      */     //   1316: getfield u : Lnn;
/*      */     //   1319: ifnonnull -> 1322
/*      */     //   1322: aload_0
/*      */     //   1323: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   1326: getfield A : Lhu;
/*      */     //   1329: invokevirtual c : ()Z
/*      */     //   1332: ifeq -> 1377
/*      */     //   1335: ldc_w 'clouds'
/*      */     //   1338: invokestatic c : (Ljava/lang/String;)V
/*      */     //   1341: invokestatic glPushMatrix : ()V
/*      */     //   1344: aload_0
/*      */     //   1345: iconst_0
/*      */     //   1346: fload_1
/*      */     //   1347: invokespecial a : (IF)V
/*      */     //   1350: sipush #2912
/*      */     //   1353: invokestatic glEnable : (I)V
/*      */     //   1356: aload #5
/*      */     //   1358: fload_1
/*      */     //   1359: invokevirtual b : (F)V
/*      */     //   1362: sipush #2912
/*      */     //   1365: invokestatic glDisable : (I)V
/*      */     //   1368: aload_0
/*      */     //   1369: iconst_1
/*      */     //   1370: fload_1
/*      */     //   1371: invokespecial a : (IF)V
/*      */     //   1374: invokestatic glPopMatrix : ()V
/*      */     //   1377: iload #16
/*      */     //   1379: ifeq -> 1409
/*      */     //   1382: ldc_w 'fhooks'
/*      */     //   1385: invokestatic c : (Ljava/lang/String;)V
/*      */     //   1388: bipush #17
/*      */     //   1390: iconst_2
/*      */     //   1391: anewarray java/lang/Object
/*      */     //   1394: dup
/*      */     //   1395: iconst_0
/*      */     //   1396: aload #5
/*      */     //   1398: aastore
/*      */     //   1399: dup
/*      */     //   1400: iconst_1
/*      */     //   1401: fload_1
/*      */     //   1402: invokestatic valueOf : (F)Ljava/lang/Float;
/*      */     //   1405: aastore
/*      */     //   1406: invokestatic callVoid : (I[Ljava/lang/Object;)V
/*      */     //   1409: ldc_w 'hand'
/*      */     //   1412: invokestatic c : (Ljava/lang/String;)V
/*      */     //   1415: aload_0
/*      */     //   1416: getfield V : D
/*      */     //   1419: dconst_1
/*      */     //   1420: dcmpl
/*      */     //   1421: ifne -> 1437
/*      */     //   1424: sipush #256
/*      */     //   1427: invokestatic glClear : (I)V
/*      */     //   1430: aload_0
/*      */     //   1431: fload_1
/*      */     //   1432: iload #14
/*      */     //   1434: invokespecial b : (FI)V
/*      */     //   1437: aload_0
/*      */     //   1438: getfield r : Lnet/minecraft/client/Minecraft;
/*      */     //   1441: getfield A : Lhu;
/*      */     //   1444: getfield g : Z
/*      */     //   1447: ifne -> 1454
/*      */     //   1450: invokestatic b : ()V
/*      */     //   1453: return
/*      */     //   1454: iinc #14, 1
/*      */     //   1457: goto -> 224
/*      */     //   1460: iconst_1
/*      */     //   1461: iconst_1
/*      */     //   1462: iconst_1
/*      */     //   1463: iconst_0
/*      */     //   1464: invokestatic glColorMask : (ZZZZ)V
/*      */     //   1467: invokestatic b : ()V
/*      */     //   1470: return
/*      */     // Line number table:
/*      */     //   Java source line number -> byte code offset
/*      */     //   #1247	-> 0
/*      */     //   #1248	-> 6
/*      */     //   #1250	-> 13
/*      */     //   #1252	-> 17
/*      */     //   #1253	-> 23
/*      */     //   #1254	-> 29
/*      */     //   #1256	-> 39
/*      */     //   #1258	-> 53
/*      */     //   #1259	-> 59
/*      */     //   #1260	-> 64
/*      */     //   #1261	-> 73
/*      */     //   #1262	-> 82
/*      */     //   #1263	-> 91
/*      */     //   #1264	-> 113
/*      */     //   #1265	-> 135
/*      */     //   #1266	-> 157
/*      */     //   #1267	-> 163
/*      */     //   #1268	-> 175
/*      */     //   #1270	-> 183
/*      */     //   #1271	-> 190
/*      */     //   #1272	-> 201
/*      */     //   #1273	-> 212
/*      */     //   #1275	-> 221
/*      */     //   #1277	-> 230
/*      */     //   #1279	-> 243
/*      */     //   #1280	-> 248
/*      */     //   #1282	-> 254
/*      */     //   #1285	-> 264
/*      */     //   #1288	-> 271
/*      */     //   #1289	-> 277
/*      */     //   #1290	-> 296
/*      */     //   #1291	-> 301
/*      */     //   #1292	-> 307
/*      */     //   #1293	-> 313
/*      */     //   #1294	-> 319
/*      */     //   #1295	-> 326
/*      */     //   #1296	-> 355
/*      */     //   #1297	-> 361
/*      */     //   #1300	-> 365
/*      */     //   #1302	-> 383
/*      */     //   #1303	-> 389
/*      */     //   #1304	-> 395
/*      */     //   #1310	-> 404
/*      */     //   #1312	-> 410
/*      */     //   #1313	-> 416
/*      */     //   #1314	-> 422
/*      */     //   #1316	-> 435
/*      */     //   #1318	-> 441
/*      */     //   #1319	-> 447
/*      */     //   #1320	-> 456
/*      */     //   #1321	-> 467
/*      */     //   #1322	-> 480
/*      */     //   #1324	-> 485
/*      */     //   #1328	-> 491
/*      */     //   #1330	-> 513
/*      */     //   #1332	-> 516
/*      */     //   #1333	-> 523
/*      */     //   #1335	-> 540
/*      */     //   #1336	-> 546
/*      */     //   #1337	-> 552
/*      */     //   #1338	-> 571
/*      */     //   #1340	-> 574
/*      */     //   #1341	-> 580
/*      */     //   #1342	-> 589
/*      */     //   #1343	-> 595
/*      */     //   #1344	-> 606
/*      */     //   #1346	-> 612
/*      */     //   #1347	-> 618
/*      */     //   #1349	-> 625
/*      */     //   #1350	-> 628
/*      */     //   #1351	-> 634
/*      */     //   #1352	-> 648
/*      */     //   #1353	-> 654
/*      */     //   #1354	-> 660
/*      */     //   #1355	-> 668
/*      */     //   #1356	-> 671
/*      */     //   #1357	-> 677
/*      */     //   #1358	-> 683
/*      */     //   #1359	-> 691
/*      */     //   #1360	-> 697
/*      */     //   #1362	-> 739
/*      */     //   #1363	-> 746
/*      */     //   #1364	-> 752
/*      */     //   #1366	-> 758
/*      */     //   #1368	-> 821
/*      */     //   #1370	-> 845
/*      */     //   #1371	-> 858
/*      */     //   #1373	-> 882
/*      */     //   #1376	-> 888
/*      */     //   #1377	-> 894
/*      */     //   #1378	-> 900
/*      */     //   #1379	-> 909
/*      */     //   #1380	-> 913
/*      */     //   #1381	-> 919
/*      */     //   #1382	-> 925
/*      */     //   #1383	-> 931
/*      */     //   #1385	-> 950
/*      */     //   #1388	-> 956
/*      */     //   #1389	-> 962
/*      */     //   #1391	-> 975
/*      */     //   #1394	-> 981
/*      */     //   #1395	-> 988
/*      */     //   #1397	-> 998
/*      */     //   #1399	-> 1011
/*      */     //   #1401	-> 1017
/*      */     //   #1404	-> 1027
/*      */     //   #1409	-> 1037
/*      */     //   #1411	-> 1044
/*      */     //   #1412	-> 1049
/*      */     //   #1414	-> 1058
/*      */     //   #1415	-> 1064
/*      */     //   #1417	-> 1067
/*      */     //   #1419	-> 1073
/*      */     //   #1421	-> 1082
/*      */     //   #1422	-> 1086
/*      */     //   #1423	-> 1092
/*      */     //   #1424	-> 1098
/*      */     //   #1426	-> 1149
/*      */     //   #1427	-> 1156
/*      */     //   #1428	-> 1162
/*      */     //   #1430	-> 1168
/*      */     //   #1432	-> 1231
/*      */     //   #1434	-> 1255
/*      */     //   #1435	-> 1268
/*      */     //   #1437	-> 1292
/*      */     //   #1439	-> 1298
/*      */     //   #1440	-> 1304
/*      */     //   #1441	-> 1309
/*      */     //   #1442	-> 1315
/*      */     //   #1443	-> 1322
/*      */     //   #1445	-> 1335
/*      */     //   #1446	-> 1341
/*      */     //   #1447	-> 1344
/*      */     //   #1448	-> 1350
/*      */     //   #1449	-> 1356
/*      */     //   #1450	-> 1362
/*      */     //   #1451	-> 1368
/*      */     //   #1452	-> 1374
/*      */     //   #1455	-> 1377
/*      */     //   #1457	-> 1382
/*      */     //   #1458	-> 1388
/*      */     //   #1461	-> 1409
/*      */     //   #1462	-> 1415
/*      */     //   #1464	-> 1424
/*      */     //   #1465	-> 1430
/*      */     //   #1467	-> 1437
/*      */     //   #1469	-> 1450
/*      */     //   #1470	-> 1453
/*      */     //   #1275	-> 1454
/*      */     //   #1474	-> 1460
/*      */     //   #1475	-> 1467
/*      */     //   #1476	-> 1470
/*      */     // Local variable table:
/*      */     //   start	length	slot	name	descriptor
/*      */     //   190	31	14	chunkproviderloadorgenerate	Lhy;
/*      */     //   201	20	15	j	I
/*      */     //   212	9	16	k	I
/*      */     //   523	17	16	l	J
/*      */     //   746	142	17	entityplayer	Lyw;
/*      */     //   998	66	17	num	I
/*      */     //   1156	142	17	entityplayer1	Lyw;
/*      */     //   456	998	15	frustrum	Lnq;
/*      */     //   618	836	16	hasForge	Z
/*      */     //   224	1236	14	i	I
/*      */     //   0	1471	0	this	Llr;
/*      */     //   0	1471	1	par1	F
/*      */     //   0	1471	2	par2	J
/*      */     //   73	1398	4	entityliving	Lacq;
/*      */     //   82	1389	5	renderglobal	Ll;
/*      */     //   91	1380	6	effectrenderer	Lcw;
/*      */     //   113	1358	7	d	D
/*      */     //   135	1336	9	d1	D
/*      */     //   157	1314	11	d2	D
/*      */     //   175	1296	13	ichunkprovider	Lca;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void f() {
/* 1480 */     float f = this.r.f.j(1.0F);
/*      */     
/* 1482 */     if (!Config.isRainFancy())
/*      */     {
/*      */       
/* 1485 */       f /= 2.0F;
/*      */     }
/* 1487 */     if (f == 0.0F) {
/*      */       return;
/*      */     }
/*      */     
/* 1491 */     if (!Config.isRainSplash())
/*      */       return; 
/* 1493 */     this.ab.setSeed(this.t * 312987231L);
/* 1494 */     acq entityliving = this.r.i;
/* 1495 */     xd world = this.r.f;
/* 1496 */     int i = gk.c(entityliving.o);
/* 1497 */     int j = gk.c(entityliving.p);
/* 1498 */     int k = gk.c(entityliving.q);
/* 1499 */     byte byte0 = 10;
/* 1500 */     double d = 0.0D;
/* 1501 */     double d1 = 0.0D;
/* 1502 */     double d2 = 0.0D;
/* 1503 */     int l = 0;
/* 1504 */     int i1 = (int)(100.0F * f * f);
/* 1505 */     if (this.r.A.Q == 1) {
/*      */       
/* 1507 */       i1 >>= 1;
/*      */     }
/* 1509 */     else if (this.r.A.Q == 2) {
/*      */       
/* 1511 */       i1 = 0;
/*      */     } 
/* 1513 */     for (int j1 = 0; j1 < i1; j1++) {
/*      */       
/* 1515 */       int k1 = i + this.ab.nextInt(byte0) - this.ab.nextInt(byte0);
/* 1516 */       int l1 = k + this.ab.nextInt(byte0) - this.ab.nextInt(byte0);
/* 1517 */       int i2 = world.f(k1, l1);
/* 1518 */       int j2 = world.a(k1, i2 - 1, l1);
/* 1519 */       abn biomegenbase = world.a(k1, l1);
/* 1520 */       if (i2 <= j + byte0 && i2 >= j - byte0 && biomegenbase.d() && biomegenbase.j() > 0.2F) {
/*      */ 
/*      */ 
/*      */         
/* 1524 */         float f1 = this.ab.nextFloat();
/* 1525 */         float f2 = this.ab.nextFloat();
/* 1526 */         if (j2 > 0)
/*      */         {
/*      */ 
/*      */           
/* 1530 */           if ((pb.m[j2]).cd == acn.h) {
/*      */             
/* 1532 */             this.r.j.a((rt)new alh(world, (k1 + f1), (i2 + 0.1F) - (pb.m[j2]).bW, (l1 + f2), 0.0D, 0.0D, 0.0D));
/*      */           } else {
/*      */             
/* 1535 */             if (this.ab.nextInt(++l) == 0) {
/*      */               
/* 1537 */               d = (k1 + f1);
/* 1538 */               d1 = (i2 + 0.1F) - (pb.m[j2]).bW;
/* 1539 */               d2 = (l1 + f2);
/*      */             } 
/*      */             
/* 1542 */             rg fx = new rg(world, (k1 + f1), (i2 + 0.1F) - (pb.m[j2]).bW, (l1 + f2));
/* 1543 */             CustomColorizer.updateWaterFX((rt)fx, (ali)world);
/* 1544 */             this.r.j.a((rt)fx);
/*      */           }  } 
/*      */       } 
/* 1547 */     }  if (l > 0 && this.ab.nextInt(3) < this.ac++) {
/*      */       
/* 1549 */       this.ac = 0;
/* 1550 */       if (d1 > entityliving.p + 1.0D && world.f(gk.c(entityliving.o), gk.c(entityliving.q)) > gk.c(entityliving.p)) {
/*      */         
/* 1552 */         this.r.f.a(d, d1, d2, "ambient.weather.rain", 0.1F, 0.5F);
/*      */       } else {
/*      */         
/* 1555 */         this.r.f.a(d, d1, d2, "ambient.weather.rain", 0.2F, 1.0F);
/*      */       } 
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected void c(float par1) {
/* 1565 */     float f = this.r.f.j(par1);
/* 1566 */     if (f <= 0.0F) {
/*      */       return;
/*      */     }
/*      */     
/* 1570 */     b(par1);
/* 1571 */     if (this.i == null) {
/*      */       
/* 1573 */       this.i = new float[1024];
/* 1574 */       this.j = new float[1024];
/* 1575 */       for (int i = 0; i < 32; i++) {
/*      */         
/* 1577 */         for (int j = 0; j < 32; j++) {
/*      */           
/* 1579 */           float f1 = (j - 16);
/* 1580 */           float f2 = (i - 16);
/* 1581 */           float f3 = gk.c(f1 * f1 + f2 * f2);
/* 1582 */           this.i[i << 5 | j] = -f2 / f3;
/* 1583 */           this.j[i << 5 | j] = f1 / f3;
/*      */         } 
/*      */       } 
/*      */     } 
/*      */     
/* 1588 */     if (Config.isRainOff()) {
/*      */       return;
/*      */     }
/* 1591 */     acq entityliving = this.r.i;
/* 1592 */     xd world = this.r.f;
/* 1593 */     int k = gk.c(entityliving.o);
/* 1594 */     int l = gk.c(entityliving.p);
/* 1595 */     int i1 = gk.c(entityliving.q);
/* 1596 */     adz tessellator = adz.a;
/* 1597 */     GL11.glDisable(2884);
/* 1598 */     GL11.glNormal3f(0.0F, 1.0F, 0.0F);
/* 1599 */     GL11.glEnable(3042);
/* 1600 */     GL11.glBlendFunc(770, 771);
/* 1601 */     GL11.glAlphaFunc(516, 0.01F);
/* 1602 */     GL11.glBindTexture(3553, this.r.p.b("/environment/snow.png"));
/* 1603 */     double d = entityliving.N + (entityliving.o - entityliving.N) * par1;
/* 1604 */     double d1 = entityliving.O + (entityliving.p - entityliving.O) * par1;
/* 1605 */     double d2 = entityliving.P + (entityliving.q - entityliving.P) * par1;
/* 1606 */     int j1 = gk.c(d1);
/* 1607 */     int k1 = 5;
/*      */     
/* 1609 */     if (Config.isRainFancy())
/*      */     {
/*      */       
/* 1612 */       k1 = 10;
/*      */     }
/* 1614 */     boolean flag = false;
/* 1615 */     byte byte0 = -1;
/* 1616 */     float f4 = this.t + par1;
/*      */     
/* 1618 */     if (Config.isRainFancy())
/*      */     {
/* 1620 */       k1 = 10;
/*      */     }
/* 1622 */     GL11.glColor4f(1.0F, 1.0F, 1.0F, 1.0F);
/* 1623 */     flag = false;
/* 1624 */     for (int l1 = i1 - k1; l1 <= i1 + k1; l1++) {
/*      */       
/* 1626 */       for (int i2 = k - k1; i2 <= k + k1; i2++) {
/*      */         
/* 1628 */         int j2 = (l1 - i1 + 16) * 32 + i2 - k + 16;
/* 1629 */         float f5 = this.i[j2] * 0.5F;
/* 1630 */         float f6 = this.j[j2] * 0.5F;
/* 1631 */         abn biomegenbase = world.a(i2, l1);
/* 1632 */         if (biomegenbase.d() || biomegenbase.c()) {
/*      */ 
/*      */ 
/*      */           
/* 1636 */           int k2 = world.f(i2, l1);
/* 1637 */           int l2 = l - k1;
/* 1638 */           int i3 = l + k1;
/* 1639 */           if (l2 < k2)
/*      */           {
/* 1641 */             l2 = k2;
/*      */           }
/* 1643 */           if (i3 < k2)
/*      */           {
/* 1645 */             i3 = k2;
/*      */           }
/* 1647 */           float f7 = 1.0F;
/* 1648 */           int j3 = k2;
/* 1649 */           if (j3 < j1)
/*      */           {
/* 1651 */             j3 = j1;
/*      */           }
/* 1653 */           if (l2 != i3) {
/*      */ 
/*      */ 
/*      */             
/* 1657 */             this.ab.setSeed((i2 * i2 * 3121 + i2 * 45238971 ^ l1 * l1 * 418711 + l1 * 13761));
/* 1658 */             float f8 = biomegenbase.j();
/* 1659 */             if (world.i().a(f8, k2) >= 0.15F)
/*      */             
/* 1661 */             { if (byte0 != 0) {
/*      */                 
/* 1663 */                 if (byte0 >= 0)
/*      */                 {
/* 1665 */                   tessellator.a();
/*      */                 }
/* 1667 */                 byte0 = 0;
/* 1668 */                 GL11.glBindTexture(3553, this.r.p.b("/environment/rain.png"));
/* 1669 */                 tessellator.b();
/*      */               } 
/* 1671 */               float f9 = ((this.t + i2 * i2 * 3121 + i2 * 45238971 + l1 * l1 * 418711 + l1 * 13761 & 0x1F) + par1) / 32.0F * (3.0F + this.ab.nextFloat());
/* 1672 */               double d3 = (i2 + 0.5F) - entityliving.o;
/* 1673 */               double d4 = (l1 + 0.5F) - entityliving.q;
/* 1674 */               float f13 = gk.a(d3 * d3 + d4 * d4) / k1;
/* 1675 */               float f14 = 1.0F;
/* 1676 */               tessellator.b(world.b(i2, j3, l1, 0));
/* 1677 */               tessellator.a(f14, f14, f14, ((1.0F - f13 * f13) * 0.5F + 0.5F) * f);
/* 1678 */               tessellator.b(-d * 1.0D, -d1 * 1.0D, -d2 * 1.0D);
/* 1679 */               tessellator.a((i2 - f5) + 0.5D, l2, (l1 - f6) + 0.5D, (0.0F * f7), (l2 * f7 / 4.0F + f9 * f7));
/* 1680 */               tessellator.a((i2 + f5) + 0.5D, l2, (l1 + f6) + 0.5D, (1.0F * f7), (l2 * f7 / 4.0F + f9 * f7));
/* 1681 */               tessellator.a((i2 + f5) + 0.5D, i3, (l1 + f6) + 0.5D, (1.0F * f7), (i3 * f7 / 4.0F + f9 * f7));
/* 1682 */               tessellator.a((i2 - f5) + 0.5D, i3, (l1 - f6) + 0.5D, (0.0F * f7), (i3 * f7 / 4.0F + f9 * f7));
/* 1683 */               tessellator.b(0.0D, 0.0D, 0.0D); }
/*      */             else
/*      */             
/* 1686 */             { if (byte0 != 1) {
/*      */                 
/* 1688 */                 if (byte0 >= 0)
/*      */                 {
/* 1690 */                   tessellator.a();
/*      */                 }
/* 1692 */                 byte0 = 1;
/* 1693 */                 GL11.glBindTexture(3553, this.r.p.b("/environment/snow.png"));
/* 1694 */                 tessellator.b();
/*      */               } 
/* 1696 */               float f10 = ((this.t & 0x1FF) + par1) / 512.0F;
/* 1697 */               float f11 = this.ab.nextFloat() + f4 * 0.01F * (float)this.ab.nextGaussian();
/* 1698 */               float f12 = this.ab.nextFloat() + f4 * (float)this.ab.nextGaussian() * 0.001F;
/* 1699 */               double d5 = (i2 + 0.5F) - entityliving.o;
/* 1700 */               double d6 = (l1 + 0.5F) - entityliving.q;
/* 1701 */               float f15 = gk.a(d5 * d5 + d6 * d6) / k1;
/* 1702 */               float f16 = 1.0F;
/* 1703 */               tessellator.b((world.b(i2, j3, l1, 0) * 3 + 15728880) / 4);
/* 1704 */               tessellator.a(f16, f16, f16, ((1.0F - f15 * f15) * 0.3F + 0.5F) * f);
/* 1705 */               tessellator.b(-d * 1.0D, -d1 * 1.0D, -d2 * 1.0D);
/* 1706 */               tessellator.a((i2 - f5) + 0.5D, l2, (l1 - f6) + 0.5D, (0.0F * f7 + f11), (l2 * f7 / 4.0F + f10 * f7 + f12));
/* 1707 */               tessellator.a((i2 + f5) + 0.5D, l2, (l1 + f6) + 0.5D, (1.0F * f7 + f11), (l2 * f7 / 4.0F + f10 * f7 + f12));
/* 1708 */               tessellator.a((i2 + f5) + 0.5D, i3, (l1 + f6) + 0.5D, (1.0F * f7 + f11), (i3 * f7 / 4.0F + f10 * f7 + f12));
/* 1709 */               tessellator.a((i2 - f5) + 0.5D, i3, (l1 - f6) + 0.5D, (0.0F * f7 + f11), (i3 * f7 / 4.0F + f10 * f7 + f12));
/* 1710 */               tessellator.b(0.0D, 0.0D, 0.0D); } 
/*      */           } 
/*      */         } 
/*      */       } 
/* 1714 */     }  if (byte0 >= 0)
/*      */     {
/* 1716 */       tessellator.a();
/*      */     }
/* 1718 */     GL11.glEnable(2884);
/* 1719 */     GL11.glDisable(3042);
/* 1720 */     GL11.glAlphaFunc(516, 0.1F);
/* 1721 */     a(par1);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void b() {
/* 1729 */     agd scaledresolution = new agd(this.r.A, this.r.d, this.r.e);
/* 1730 */     GL11.glClear(256);
/* 1731 */     GL11.glMatrixMode(5889);
/* 1732 */     GL11.glLoadIdentity();
/* 1733 */     GL11.glOrtho(0.0D, scaledresolution.a, scaledresolution.b, 0.0D, 1000.0D, 3000.0D);
/* 1734 */     GL11.glMatrixMode(5888);
/* 1735 */     GL11.glLoadIdentity();
/* 1736 */     GL11.glTranslatef(0.0F, 0.0F, -2000.0F);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void g(float par1) {
/* 1744 */     xd world = this.r.f;
/* 1745 */     acq entityliving = this.r.i;
/* 1746 */     float f = 1.0F / (4 - this.r.A.e);
/* 1747 */     f = 1.0F - (float)Math.pow(f, 0.25D);
/* 1748 */     bo vec3d = world.a(this.r.i, par1);
/*      */     
/* 1750 */     int worldType = world.t.g;
/* 1751 */     switch (worldType) {
/*      */ 
/*      */       
/*      */       case 0:
/* 1755 */         vec3d = CustomColorizer.getSkyColor(vec3d, (ali)this.r.f, this.r.i.o, this.r.i.p + 1.0D, this.r.i.q);
/*      */         break;
/*      */       
/*      */       case 1:
/* 1759 */         vec3d = CustomColorizer.getSkyColorEnd(vec3d);
/*      */         break;
/*      */     } 
/*      */     
/* 1763 */     float f1 = (float)vec3d.a;
/* 1764 */     float f2 = (float)vec3d.b;
/* 1765 */     float f3 = (float)vec3d.c;
/* 1766 */     bo vec3d1 = world.g(par1);
/*      */     
/* 1768 */     switch (worldType) {
/*      */ 
/*      */       
/*      */       case -1:
/* 1772 */         vec3d1 = CustomColorizer.getFogColorNether(vec3d1);
/*      */         break;
/*      */       
/*      */       case 0:
/* 1776 */         vec3d1 = CustomColorizer.getFogColor(vec3d1, (ali)this.r.f, this.r.i.o, this.r.i.p + 1.0D, this.r.i.q);
/*      */         break;
/*      */       
/*      */       case 1:
/* 1780 */         vec3d1 = CustomColorizer.getFogColorEnd(vec3d1);
/*      */         break;
/*      */     } 
/*      */     
/* 1784 */     this.n = (float)vec3d1.a;
/* 1785 */     this.o = (float)vec3d1.b;
/* 1786 */     this.p = (float)vec3d1.c;
/* 1787 */     if (this.r.A.e < 2) {
/*      */       
/* 1789 */       bo vec3d2 = (gk.a(world.e(par1)) <= 0.0F) ? bo.b(1.0D, 0.0D, 0.0D) : bo.b(-1.0D, 0.0D, 0.0D);
/* 1790 */       float f5 = (float)entityliving.k(par1).b(vec3d2);
/* 1791 */       if (f5 < 0.0F)
/*      */       {
/* 1793 */         f5 = 0.0F;
/*      */       }
/* 1795 */       if (f5 > 0.0F) {
/*      */         
/* 1797 */         float[] af = world.t.a(world.c(par1), par1);
/* 1798 */         if (af != null) {
/*      */           
/* 1800 */           f5 *= af[3];
/* 1801 */           this.n = this.n * (1.0F - f5) + af[0] * f5;
/* 1802 */           this.o = this.o * (1.0F - f5) + af[1] * f5;
/* 1803 */           this.p = this.p * (1.0F - f5) + af[2] * f5;
/*      */         } 
/*      */       } 
/*      */     } 
/* 1807 */     this.n += (f1 - this.n) * f;
/* 1808 */     this.o += (f2 - this.o) * f;
/* 1809 */     this.p += (f3 - this.p) * f;
/* 1810 */     float f4 = world.j(par1);
/* 1811 */     if (f4 > 0.0F) {
/*      */       
/* 1813 */       float f6 = 1.0F - f4 * 0.5F;
/* 1814 */       float f8 = 1.0F - f4 * 0.4F;
/* 1815 */       this.n *= f6;
/* 1816 */       this.o *= f6;
/* 1817 */       this.p *= f8;
/*      */     } 
/* 1819 */     float f7 = world.i(par1);
/* 1820 */     if (f7 > 0.0F) {
/*      */       
/* 1822 */       float f9 = 1.0F - f7 * 0.5F;
/* 1823 */       this.n *= f9;
/* 1824 */       this.o *= f9;
/* 1825 */       this.p *= f9;
/*      */     } 
/* 1827 */     int i = aau.a(this.r.f, entityliving, par1);
/* 1828 */     if (this.U) {
/*      */       
/* 1830 */       bo vec3d3 = world.f(par1);
/* 1831 */       this.n = (float)vec3d3.a;
/* 1832 */       this.o = (float)vec3d3.b;
/* 1833 */       this.p = (float)vec3d3.c;
/*      */     }
/* 1835 */     else if (i != 0 && (pb.m[i]).cd == acn.g) {
/*      */       
/* 1837 */       this.n = 0.02F;
/* 1838 */       this.o = 0.02F;
/* 1839 */       this.p = 0.2F;
/*      */       
/* 1841 */       bo colUnderwater = CustomColorizer.getUnderwaterColor((ali)this.r.f, this.r.i.o, this.r.i.p + 1.0D, this.r.i.q);
/* 1842 */       if (colUnderwater != null)
/*      */       {
/* 1844 */         this.n = (float)colUnderwater.a;
/* 1845 */         this.o = (float)colUnderwater.b;
/* 1846 */         this.p = (float)colUnderwater.c;
/*      */       }
/*      */     
/*      */     }
/* 1850 */     else if (i != 0 && (pb.m[i]).cd == acn.h) {
/*      */       
/* 1852 */       this.n = 0.6F;
/* 1853 */       this.o = 0.1F;
/* 1854 */       this.p = 0.0F;
/*      */     } 
/* 1856 */     float f10 = this.ad + (this.ae - this.ad) * par1;
/* 1857 */     this.n *= f10;
/* 1858 */     this.o *= f10;
/* 1859 */     this.p *= f10;
/* 1860 */     double d = (entityliving.O + (entityliving.p - entityliving.O) * par1) * world.t.k();
/* 1861 */     if (entityliving.a(aad.q)) {
/*      */       
/* 1863 */       int j = entityliving.b(aad.q).b();
/* 1864 */       if (j < 20) {
/*      */         
/* 1866 */         d *= (1.0F - j / 20.0F);
/*      */       } else {
/*      */         
/* 1869 */         d = 0.0D;
/*      */       } 
/*      */     } 
/* 1872 */     if (d < 1.0D) {
/*      */       
/* 1874 */       if (d < 0.0D)
/*      */       {
/* 1876 */         d = 0.0D;
/*      */       }
/* 1878 */       d *= d;
/* 1879 */       this.n = (float)(this.n * d);
/* 1880 */       this.o = (float)(this.o * d);
/* 1881 */       this.p = (float)(this.p * d);
/*      */     } 
/* 1883 */     if (this.r.A.g) {
/*      */       
/* 1885 */       float f11 = (this.n * 30.0F + this.o * 59.0F + this.p * 11.0F) / 100.0F;
/* 1886 */       float f12 = (this.n * 30.0F + this.o * 70.0F) / 100.0F;
/* 1887 */       float f13 = (this.n * 30.0F + this.p * 70.0F) / 100.0F;
/* 1888 */       this.n = f11;
/* 1889 */       this.o = f12;
/* 1890 */       this.p = f13;
/*      */     } 
/* 1892 */     GL11.glClearColor(this.n, this.o, this.p, 0.0F);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void a(int par1, float par2) {
/* 1901 */     acq entityliving = this.r.i;
/* 1902 */     boolean flag = false;
/* 1903 */     if (entityliving instanceof yw)
/*      */     {
/* 1905 */       flag = ((yw)entityliving).aT.d;
/*      */     }
/* 1907 */     if (par1 == 999) {
/*      */       
/* 1909 */       GL11.glFog(2918, a(0.0F, 0.0F, 0.0F, 1.0F));
/* 1910 */       GL11.glFogi(2917, 9729);
/* 1911 */       GL11.glFogf(2915, 0.0F);
/* 1912 */       GL11.glFogf(2916, 8.0F);
/* 1913 */       if ((GLContext.getCapabilities()).GL_NV_fog_distance)
/*      */       {
/* 1915 */         GL11.glFogi(34138, 34139);
/*      */       }
/* 1917 */       GL11.glFogf(2915, 0.0F);
/*      */       return;
/*      */     } 
/* 1920 */     GL11.glFog(2918, a(this.n, this.o, this.p, 1.0F));
/* 1921 */     GL11.glNormal3f(0.0F, -1.0F, 0.0F);
/* 1922 */     GL11.glColor4f(1.0F, 1.0F, 1.0F, 1.0F);
/* 1923 */     int i = aau.a(this.r.f, entityliving, par2);
/* 1924 */     if (entityliving.a(aad.q)) {
/*      */       
/* 1926 */       float f = 5.0F;
/* 1927 */       int j = entityliving.b(aad.q).b();
/* 1928 */       if (j < 20)
/*      */       {
/* 1930 */         f = 5.0F + (this.s - 5.0F) * (1.0F - j / 20.0F);
/*      */       }
/* 1932 */       GL11.glFogi(2917, 9729);
/* 1933 */       if (par1 < 0) {
/*      */         
/* 1935 */         GL11.glFogf(2915, 0.0F);
/* 1936 */         GL11.glFogf(2916, f * 0.8F);
/*      */       } else {
/*      */         
/* 1939 */         GL11.glFogf(2915, f * 0.25F);
/* 1940 */         GL11.glFogf(2916, f);
/*      */       } 
/*      */       
/* 1943 */       if (Config.isFogFancy())
/*      */       {
/* 1945 */         GL11.glFogi(34138, 34139);
/*      */       }
/*      */     }
/* 1948 */     else if (this.U) {
/*      */       
/* 1950 */       GL11.glFogi(2917, 2048);
/* 1951 */       GL11.glFogf(2914, 0.1F);
/* 1952 */       float f1 = 1.0F;
/* 1953 */       float f5 = 1.0F;
/* 1954 */       float f8 = 1.0F;
/* 1955 */       if (this.r.A.g)
/*      */       {
/* 1957 */         float f11 = (f1 * 30.0F + f5 * 59.0F + f8 * 11.0F) / 100.0F;
/* 1958 */         float f15 = (f1 * 30.0F + f5 * 70.0F) / 100.0F;
/* 1959 */         float f18 = (f1 * 30.0F + f8 * 70.0F) / 100.0F;
/* 1960 */         f1 = f11;
/* 1961 */         f5 = f15;
/* 1962 */         f8 = f18;
/*      */       }
/*      */     
/* 1965 */     } else if (i > 0 && (pb.m[i]).cd == acn.g) {
/*      */       
/* 1967 */       GL11.glFogi(2917, 2048);
/*      */       
/* 1969 */       float waterFogDensity = 0.1F;
/* 1970 */       if (!entityliving.a(aad.o)) {
/*      */         
/* 1972 */         waterFogDensity = 0.1F;
/*      */       } else {
/*      */         
/* 1975 */         waterFogDensity = 0.05F;
/*      */       } 
/*      */       
/* 1978 */       if (Config.isClearWater()) {
/* 1979 */         waterFogDensity /= 5.0F;
/*      */       }
/* 1981 */       GL11.glFogf(2914, waterFogDensity);
/*      */       
/* 1983 */       float f2 = 0.4F;
/* 1984 */       float f6 = 0.4F;
/* 1985 */       float f9 = 0.9F;
/* 1986 */       if (this.r.A.g)
/*      */       {
/* 1988 */         float f12 = (f2 * 30.0F + f6 * 59.0F + f9 * 11.0F) / 100.0F;
/* 1989 */         float f16 = (f2 * 30.0F + f6 * 70.0F) / 100.0F;
/* 1990 */         float f19 = (f2 * 30.0F + f9 * 70.0F) / 100.0F;
/* 1991 */         f2 = f12;
/* 1992 */         f6 = f16;
/* 1993 */         f9 = f19;
/*      */       }
/*      */     
/* 1996 */     } else if (i > 0 && (pb.m[i]).cd == acn.h) {
/*      */       
/* 1998 */       GL11.glFogi(2917, 2048);
/* 1999 */       GL11.glFogf(2914, 2.0F);
/* 2000 */       float f3 = 0.4F;
/* 2001 */       float f7 = 0.3F;
/* 2002 */       float f10 = 0.3F;
/* 2003 */       if (this.r.A.g) {
/*      */         
/* 2005 */         float f13 = (f3 * 30.0F + f7 * 59.0F + f10 * 11.0F) / 100.0F;
/* 2006 */         float f17 = (f3 * 30.0F + f7 * 70.0F) / 100.0F;
/* 2007 */         float f20 = (f3 * 30.0F + f10 * 70.0F) / 100.0F;
/* 2008 */         f3 = f13;
/* 2009 */         f7 = f17;
/* 2010 */         f10 = f20;
/*      */       } 
/*      */     } else {
/*      */       
/* 2014 */       float f4 = this.s;
/*      */       
/* 2016 */       if (Config.isDepthFog() && this.r.f.t.j() && !flag) {
/*      */         
/* 2018 */         double d = ((entityliving.b(par2) & 0xF00000) >> 20) / 16.0D + (entityliving.O + (entityliving.p - entityliving.O) * par2 + 4.0D) / 32.0D;
/* 2019 */         if (d < 1.0D) {
/*      */           
/* 2021 */           if (d < 0.0D)
/*      */           {
/* 2023 */             d = 0.0D;
/*      */           }
/* 2025 */           d *= d;
/* 2026 */           float f14 = 100.0F * (float)d;
/* 2027 */           if (f14 < 5.0F)
/*      */           {
/* 2029 */             f14 = 5.0F;
/*      */           }
/* 2031 */           if (f4 > f14)
/*      */           {
/* 2033 */             f4 = f14;
/*      */           }
/*      */         } 
/*      */       } 
/*      */       
/* 2038 */       GL11.glFogi(2917, 9729);
/*      */ 
/*      */       
/* 2041 */       if ((GLContext.getCapabilities()).GL_NV_fog_distance) {
/*      */ 
/*      */         
/* 2044 */         if (Config.isFogFancy())
/*      */         {
/*      */           
/* 2047 */           GL11.glFogi(34138, 34139);
/*      */         }
/* 2049 */         if (Config.isFogFast())
/*      */         {
/*      */           
/* 2052 */           GL11.glFogi(34138, 34140);
/*      */         }
/*      */       } 
/*      */       
/* 2056 */       float fogStart = Config.getFogStart();
/* 2057 */       float fogEnd = 1.0F;
/* 2058 */       if (par1 < 0) {
/*      */ 
/*      */         
/* 2061 */         fogStart = 0.0F;
/* 2062 */         fogEnd = 0.8F;
/*      */       } 
/*      */       
/* 2065 */       if (this.r.f.t.b((int)entityliving.o, (int)entityliving.q)) {
/*      */         
/* 2067 */         fogStart = 0.05F;
/* 2068 */         fogEnd = 1.0F;
/*      */         
/* 2070 */         f4 = this.s;
/*      */       } 
/*      */ 
/*      */       
/* 2074 */       GL11.glFogf(2915, f4 * fogStart);
/* 2075 */       GL11.glFogf(2916, f4 * fogEnd);
/*      */     } 
/*      */     
/* 2078 */     GL11.glEnable(2903);
/* 2079 */     GL11.glColorMaterial(1028, 4608);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private FloatBuffer a(float par1, float par2, float par3, float par4) {
/* 2087 */     this.m.clear();
/* 2088 */     this.m.put(par1).put(par2).put(par3).put(par4);
/* 2089 */     this.m.flip();
/* 2090 */     return this.m;
/*      */   }
/*      */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\lr.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */