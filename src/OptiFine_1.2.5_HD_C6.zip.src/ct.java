/*     */ import java.util.ArrayList;
/*     */ import java.util.HashSet;
/*     */ import java.util.List;
/*     */ import org.lwjgl.opengl.GL11;
/*     */ 
/*     */ 
/*     */ public class ct
/*     */ {
/*     */   public xd a;
/*     */   private int y;
/*  11 */   public static int b = 0;
/*     */ 
/*     */   
/*     */   public int c;
/*     */ 
/*     */   
/*     */   public int d;
/*     */ 
/*     */   
/*     */   public int e;
/*     */ 
/*     */   
/*     */   public int f;
/*     */ 
/*     */   
/*     */   public int g;
/*     */ 
/*     */   
/*     */   public int h;
/*     */   
/*     */   public int i;
/*     */   
/*     */   public int j;
/*     */   
/*     */   public int k;
/*     */   
/*     */   public boolean l;
/*     */   
/*     */   public boolean[] m;
/*     */   
/*     */   public int n;
/*     */   
/*     */   public int o;
/*     */   
/*     */   public int p;
/*     */   
/*     */   public boolean q;
/*     */   
/*     */   public wu r;
/*     */   
/*     */   public int s;
/*     */   
/*     */   public boolean t;
/*     */   
/*     */   public boolean u;
/*     */   
/*     */   public int v;
/*     */   
/*     */   public boolean w;
/*     */   
/*     */   private boolean A;
/*     */   
/*     */   public List x;
/*     */   
/*     */   private List B;
/*     */   
/*     */   private int C;
/*     */   
/*     */   public boolean isVisibleFromPosition = false;
/*     */   
/*     */   public double visibleFromX;
/*     */   
/*     */   public double visibleFromY;
/*     */   
/*     */   public double visibleFromZ;
/*     */   
/*     */   private boolean needsBoxUpdate = false;
/*     */   
/*     */   public boolean isInFrustrumFully = false;
/*     */   
/*  81 */   public static int globalChunkOffsetX = 0;
/*  82 */   public static int globalChunkOffsetZ = 0;
/*     */ 
/*     */   
/*     */   public ct(xd par1World, List par2List, int par3, int par4, int par5, int par6) {
/*  86 */     this.y = -1;
/*  87 */     this.l = false;
/*  88 */     this.m = new boolean[2];
/*  89 */     this.t = true;
/*  90 */     this.A = false;
/*  91 */     this.x = new ArrayList();
/*  92 */     this.a = par1World;
/*  93 */     this.B = par2List;
/*  94 */     this.y = par6;
/*  95 */     this.c = -999;
/*  96 */     a(par3, par4, par5);
/*  97 */     this.q = false;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void a(int px, int py, int pz) {
/* 105 */     if (px == this.c && py == this.d && pz == this.e) {
/*     */       return;
/*     */     }
/*     */ 
/*     */     
/* 110 */     b();
/* 111 */     this.c = px;
/* 112 */     this.d = py;
/* 113 */     this.e = pz;
/* 114 */     this.n = px + 8;
/* 115 */     this.o = py + 8;
/* 116 */     this.p = pz + 8;
/* 117 */     this.i = px & 0x3FF;
/* 118 */     this.j = py;
/* 119 */     this.k = pz & 0x3FF;
/* 120 */     this.f = px - this.i;
/* 121 */     this.g = py - this.j;
/* 122 */     this.h = pz - this.k;
/*     */     
/* 124 */     float f = 0.0F;
/*     */     
/* 126 */     this.r = wu.a((px - f), (py - f), (pz - f), ((px + 16) + f), ((py + 16) + f), ((pz + 16) + f));
/*     */     
/* 128 */     this.needsBoxUpdate = true;
/*     */ 
/*     */ 
/*     */ 
/*     */     
/* 133 */     f();
/*     */     
/* 135 */     this.isVisibleFromPosition = false;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private void g() {
/* 143 */     GL11.glTranslatef(this.i, this.j, this.k);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void a() {
/* 152 */     if (this.a == null) {
/*     */       return;
/*     */     }
/* 155 */     if (!this.q) {
/*     */       return;
/*     */     }
/*     */     
/* 159 */     if (this.needsBoxUpdate) {
/*     */ 
/*     */       
/* 162 */       float f = 0.0F;
/*     */       
/* 164 */       GL11.glNewList(this.y + 2, 4864);
/* 165 */       tw.a(wu.b((this.i - f), (this.j - f), (this.k - f), ((this.i + 16) + f), ((this.j + 16) + f), ((this.k + 16) + f)));
/* 166 */       GL11.glEndList();
/*     */       
/* 168 */       this.needsBoxUpdate = false;
/*     */     } 
/*     */     
/* 171 */     this.t = true;
/* 172 */     this.isVisibleFromPosition = false;
/*     */     
/* 174 */     this.q = false;
/* 175 */     int xMin = this.c;
/* 176 */     int yMin = this.d;
/* 177 */     int zMin = this.e;
/* 178 */     int xMax = this.c + 16;
/* 179 */     int yMax = this.d + 16;
/* 180 */     int zMax = this.e + 16;
/* 181 */     for (int k1 = 0; k1 < 2; k1++)
/*     */     {
/* 183 */       this.m[k1] = true;
/*     */     }
/*     */ 
/*     */ 
/*     */     
/* 188 */     if (Reflector.hasClass(3)) {
/*     */       
/* 190 */       Object lightCache = Reflector.getFieldValue(30);
/* 191 */       Reflector.callVoid(lightCache, 30, new Object[0]);
/* 192 */       Reflector.callVoid(40, new Object[0]);
/*     */     } 
/*     */     
/* 195 */     ack.a = false;
/* 196 */     HashSet<?> hashset = new HashSet();
/* 197 */     hashset.addAll(this.x);
/* 198 */     this.x.clear();
/* 199 */     int one = 1;
/* 200 */     dv chunkcache = new dv(this.a, xMin - one, yMin - one, zMin - one, xMax + one, yMax + one, zMax + one);
/* 201 */     if (!chunkcache.a()) {
/*     */       
/* 203 */       b++;
/* 204 */       vl renderblocks = new vl((ali)chunkcache);
/* 205 */       this.C = 0;
/*     */       
/* 207 */       adz tessellator = adz.a;
/* 208 */       boolean hasForge = Reflector.hasClass(1);
/*     */       
/* 210 */       for (int renderPass = 0; renderPass < 2; renderPass++) {
/*     */         
/* 212 */         boolean renderNextPass = false;
/* 213 */         boolean hasRenderedBlocks = false;
/* 214 */         boolean hasGlList = false;
/* 215 */         for (int y = yMin; y < yMax; y++) {
/*     */           
/* 217 */           for (int z = zMin; z < zMax; z++) {
/*     */             
/* 219 */             for (int x = xMin; x < xMax; x++) {
/*     */               
/* 221 */               int i3 = chunkcache.a(x, y, z);
/* 222 */               if (i3 > 0) {
/*     */ 
/*     */ 
/*     */                 
/* 226 */                 if (!hasGlList) {
/*     */                   
/* 228 */                   hasGlList = true;
/* 229 */                   GL11.glNewList(this.y + renderPass, 4864);
/*     */                   
/* 231 */                   tessellator.setRenderingChunk(true);
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */                   
/* 239 */                   if (hasForge) {
/* 240 */                     Reflector.callVoid(13, new Object[] { Integer.valueOf(renderPass) });
/*     */                   }
/* 242 */                   tessellator.b();
/*     */                   
/* 244 */                   tessellator.b(-globalChunkOffsetX, 0.0D, -globalChunkOffsetZ);
/*     */                 } 
/* 246 */                 pb block = pb.m[i3];
/* 247 */                 if (renderPass == 0 && block.p()) {
/*     */                   
/* 249 */                   kw tileentity = chunkcache.b(x, y, z);
/* 250 */                   if (ach.a.a(tileentity))
/*     */                   {
/* 252 */                     this.x.add(tileentity);
/*     */                   }
/*     */                 } 
/* 255 */                 int blockPass = block.c();
/* 256 */                 boolean canRender = true;
/* 257 */                 if (blockPass != renderPass) {
/*     */                   
/* 259 */                   renderNextPass = true;
/* 260 */                   canRender = false;
/*     */                 } 
/*     */                 
/* 263 */                 if (hasForge) {
/* 264 */                   canRender = Reflector.callBoolean(11, new Object[] { block, Integer.valueOf(renderPass) });
/*     */                 }
/* 266 */                 if (canRender) {
/*     */ 
/*     */                   
/* 269 */                   if (hasForge) {
/* 270 */                     Reflector.callVoid(15, new Object[] { block, renderblocks });
/*     */                   }
/* 272 */                   hasRenderedBlocks |= renderblocks.b(block, x, y, z);
/*     */                   
/* 274 */                   if (hasForge)
/* 275 */                     Reflector.callVoid(16, new Object[] { block, renderblocks }); 
/*     */                 } 
/*     */               } 
/*     */             } 
/*     */           } 
/*     */         } 
/* 281 */         if (hasGlList) {
/*     */ 
/*     */           
/* 284 */           if (hasForge) {
/* 285 */             Reflector.callVoid(14, new Object[] { Integer.valueOf(renderPass) });
/*     */           }
/* 287 */           this.C += tessellator.a();
/*     */ 
/*     */           
/* 290 */           GL11.glEndList();
/*     */           
/* 292 */           tessellator.setRenderingChunk(false);
/* 293 */           tessellator.b(0.0D, 0.0D, 0.0D);
/*     */         } else {
/*     */           
/* 296 */           hasRenderedBlocks = false;
/*     */         } 
/* 298 */         if (hasRenderedBlocks)
/*     */         {
/* 300 */           this.m[renderPass] = false;
/*     */         }
/* 302 */         if (!renderNextPass) {
/*     */           break;
/*     */         }
/*     */       } 
/*     */     } 
/*     */     
/* 308 */     HashSet hashset1 = new HashSet();
/* 309 */     hashset1.addAll(this.x);
/* 310 */     hashset1.removeAll(hashset);
/* 311 */     this.B.addAll(hashset1);
/* 312 */     hashset.removeAll(this.x);
/* 313 */     this.B.removeAll(hashset);
/* 314 */     this.w = ack.a;
/* 315 */     this.A = true;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public float a(nn par1Entity) {
/* 324 */     float f = (float)(par1Entity.o - this.n);
/* 325 */     float f1 = (float)(par1Entity.p - this.o);
/* 326 */     float f2 = (float)(par1Entity.q - this.p);
/* 327 */     return f * f + f1 * f1 + f2 * f2;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void b() {
/* 335 */     for (int i = 0; i < 2; i++)
/*     */     {
/* 337 */       this.m[i] = true;
/*     */     }
/*     */     
/* 340 */     this.l = false;
/* 341 */     this.A = false;
/*     */   }
/*     */ 
/*     */   
/*     */   public void c() {
/* 346 */     b();
/* 347 */     this.a = null;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public int a(int par1) {
/* 355 */     if (!this.l)
/*     */     {
/* 357 */       return -1;
/*     */     }
/* 359 */     if (!this.m[par1])
/*     */     {
/* 361 */       return this.y + par1;
/*     */     }
/*     */     
/* 364 */     return -1;
/*     */   }
/*     */ 
/*     */ 
/*     */   
/*     */   public void a(amd par1ICamera) {
/* 370 */     this.l = par1ICamera.a(this.r);
/*     */     
/* 372 */     if (this.l && Config.isOcclusionEnabled() && Config.isOcclusionFancy()) {
/* 373 */       this.isInFrustrumFully = par1ICamera.isBoundingBoxInFrustumFully(this.r);
/*     */     } else {
/* 375 */       this.isInFrustrumFully = false;
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void d() {
/* 383 */     GL11.glCallList(this.y + 2);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public boolean e() {
/* 391 */     if (!this.A)
/*     */     {
/* 393 */       return false;
/*     */     }
/*     */     
/* 396 */     return (this.m[0] && this.m[1]);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void f() {
/* 405 */     this.q = true;
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\ct.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */