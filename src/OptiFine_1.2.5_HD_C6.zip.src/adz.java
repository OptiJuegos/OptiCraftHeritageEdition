/*     */ import java.nio.ByteBuffer;
/*     */ import java.nio.ByteOrder;
/*     */ import java.nio.FloatBuffer;
/*     */ import java.nio.IntBuffer;
/*     */ import java.nio.ShortBuffer;
/*     */ import org.lwjgl.opengl.ARBVertexBufferObject;
/*     */ import org.lwjgl.opengl.GL11;
/*     */ import org.lwjgl.opengl.GLContext;
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ public class adz
/*     */ {
/*     */   private static boolean b = false;
/*     */   private static boolean c = false;
/*     */   private ByteBuffer d;
/*     */   private IntBuffer e;
/*     */   private FloatBuffer f;
/*     */   private ShortBuffer g;
/*     */   private int[] h;
/*     */   private int i;
/*     */   private double j;
/*     */   private double k;
/*     */   private int l;
/*     */   private int m;
/*     */   private boolean n;
/*     */   private boolean o;
/*     */   private boolean p;
/*     */   private boolean q;
/*     */   private int r;
/*     */   private int s;
/*     */   private boolean t;
/*     */   public int u;
/*     */   public double v;
/*     */   public double w;
/*     */   public double x;
/*     */   private int y;
/*  98 */   public static adz a = new adz(524288);
/*     */ 
/*     */ 
/*     */   
/*     */   public boolean z;
/*     */ 
/*     */   
/*     */   private boolean A;
/*     */ 
/*     */   
/*     */   private IntBuffer B;
/*     */ 
/*     */   
/*     */   private int C;
/*     */ 
/*     */   
/*     */   private int D;
/*     */ 
/*     */   
/*     */   private int E;
/*     */ 
/*     */   
/*     */   private boolean renderingChunk = false;
/*     */ 
/*     */   
/* 123 */   private static boolean littleEndianByteOrder = (ByteOrder.nativeOrder() == ByteOrder.LITTLE_ENDIAN);
/*     */   
/*     */   public static boolean renderingWorldRenderer = false;
/*     */   public boolean defaultTexture = true;
/* 127 */   public int textureID = 0;
/*     */   
/*     */   public boolean autoGrow = true;
/* 130 */   private adz[] subTessellators = new adz[0];
/* 131 */   private int[] subTextures = new int[0];
/* 132 */   private int terrainTexture = 0;
/* 133 */   private long textureUpdateTime = 0L;
/*     */ 
/*     */ 
/*     */   
/*     */   public adz() {
/* 138 */     this(65536);
/* 139 */     this.defaultTexture = false;
/*     */   }
/*     */ 
/*     */   
/*     */   public adz(int par1) {
/* 144 */     this.i = 0;
/* 145 */     this.n = false;
/* 146 */     this.o = false;
/* 147 */     this.p = false;
/* 148 */     this.q = false;
/* 149 */     this.r = 0;
/* 150 */     this.s = 0;
/* 151 */     this.t = false;
/* 152 */     this.z = false;
/* 153 */     this.A = false;
/* 154 */     this.C = 0;
/* 155 */     this.D = 10;
/* 156 */     this.E = par1;
/* 157 */     this.d = ew.c(par1 * 4);
/* 158 */     this.e = this.d.asIntBuffer();
/* 159 */     this.f = this.d.asFloatBuffer();
/* 160 */     this.g = this.d.asShortBuffer();
/* 161 */     this.h = new int[par1];
/* 162 */     this.A = (c && (GLContext.getCapabilities()).GL_ARB_vertex_buffer_object);
/* 163 */     if (this.A) {
/*     */       
/* 165 */       this.B = ew.d(this.D);
/* 166 */       ARBVertexBufferObject.glGenBuffersARB(this.B);
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public int a() {
/* 175 */     if (!this.z)
/*     */     {
/* 177 */       throw new IllegalStateException("Not tesselating!");
/*     */     }
/*     */     
/* 180 */     if (this.renderingChunk && this.subTessellators.length > 0) {
/*     */       
/* 182 */       boolean textureChanged = false;
/* 183 */       for (int j = 0; j < this.subTessellators.length; j++) {
/*     */         
/* 185 */         int tex = this.subTextures[j];
/*     */         
/* 187 */         if (tex <= 0) {
/*     */           break;
/*     */         }
/* 190 */         adz tess = this.subTessellators[j];
/* 191 */         if (tess.z) {
/*     */ 
/*     */           
/* 194 */           GL11.glBindTexture(3553, tex);
/*     */           
/* 196 */           tess.a();
/*     */           
/* 198 */           textureChanged = true;
/*     */         } 
/*     */       } 
/* 201 */       if (textureChanged) {
/* 202 */         GL11.glBindTexture(3553, getTerrainTexture());
/*     */       }
/*     */     } 
/* 205 */     this.z = false;
/* 206 */     if (this.i > 0) {
/*     */       
/* 208 */       this.e.clear();
/* 209 */       this.e.put(this.h, 0, this.r);
/* 210 */       this.d.position(0);
/* 211 */       this.d.limit(this.r * 4);
/* 212 */       if (this.A) {
/*     */         
/* 214 */         this.C = (this.C + 1) % this.D;
/* 215 */         ARBVertexBufferObject.glBindBufferARB(34962, this.B.get(this.C));
/* 216 */         ARBVertexBufferObject.glBufferDataARB(34962, this.d, 35040);
/*     */       } 
/* 218 */       if (this.o) {
/*     */         
/* 220 */         if (this.A) {
/*     */           
/* 222 */           GL11.glTexCoordPointer(2, 5126, 32, 12L);
/*     */         } else {
/*     */           
/* 225 */           this.f.position(3);
/* 226 */           GL11.glTexCoordPointer(2, 32, this.f);
/*     */         } 
/* 228 */         GL11.glEnableClientState(32888);
/*     */       } 
/* 230 */       if (this.p) {
/*     */         
/* 232 */         es.b(es.b);
/* 233 */         if (this.A) {
/*     */           
/* 235 */           GL11.glTexCoordPointer(2, 5122, 32, 28L);
/*     */         } else {
/*     */           
/* 238 */           this.g.position(14);
/* 239 */           GL11.glTexCoordPointer(2, 32, this.g);
/*     */         } 
/* 241 */         GL11.glEnableClientState(32888);
/* 242 */         es.b(es.a);
/*     */       } 
/* 244 */       if (this.n) {
/*     */         
/* 246 */         if (this.A) {
/*     */           
/* 248 */           GL11.glColorPointer(4, 5121, 32, 20L);
/*     */         } else {
/*     */           
/* 251 */           this.d.position(20);
/* 252 */           GL11.glColorPointer(4, true, 32, this.d);
/*     */         } 
/* 254 */         GL11.glEnableClientState(32886);
/*     */       } 
/* 256 */       if (this.q) {
/*     */         
/* 258 */         if (this.A) {
/*     */           
/* 260 */           GL11.glNormalPointer(5121, 32, 24L);
/*     */         } else {
/*     */           
/* 263 */           this.d.position(24);
/* 264 */           GL11.glNormalPointer(32, this.d);
/*     */         } 
/* 266 */         GL11.glEnableClientState(32885);
/*     */       } 
/* 268 */       if (this.A) {
/*     */         
/* 270 */         GL11.glVertexPointer(3, 5126, 32, 0L);
/*     */       } else {
/*     */         
/* 273 */         this.f.position(0);
/* 274 */         GL11.glVertexPointer(3, 32, this.f);
/*     */       } 
/* 276 */       GL11.glEnableClientState(32884);
/* 277 */       if (this.u == 7 && b) {
/*     */         
/* 279 */         GL11.glDrawArrays(4, 0, this.i);
/*     */       } else {
/*     */         
/* 282 */         GL11.glDrawArrays(this.u, 0, this.i);
/*     */       } 
/* 284 */       GL11.glDisableClientState(32884);
/* 285 */       if (this.o)
/*     */       {
/* 287 */         GL11.glDisableClientState(32888);
/*     */       }
/* 289 */       if (this.p) {
/*     */         
/* 291 */         es.b(es.b);
/* 292 */         GL11.glDisableClientState(32888);
/* 293 */         es.b(es.a);
/*     */       } 
/* 295 */       if (this.n)
/*     */       {
/* 297 */         GL11.glDisableClientState(32886);
/*     */       }
/* 299 */       if (this.q)
/*     */       {
/* 301 */         GL11.glDisableClientState(32885);
/*     */       }
/*     */     } 
/* 304 */     int i = this.r * 4;
/* 305 */     d();
/* 306 */     return i;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private void d() {
/* 314 */     this.i = 0;
/* 315 */     this.d.clear();
/* 316 */     this.r = 0;
/* 317 */     this.s = 0;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void b() {
/* 325 */     a(7);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void a(int par1) {
/* 333 */     if (this.z)
/*     */     {
/* 335 */       throw new IllegalStateException("Already tesselating!");
/*     */     }
/*     */     
/* 338 */     this.z = true;
/* 339 */     d();
/* 340 */     this.u = par1;
/* 341 */     this.q = false;
/* 342 */     this.n = false;
/* 343 */     this.o = false;
/* 344 */     this.p = false;
/* 345 */     this.t = false;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void a(double par1, double par3) {
/* 355 */     this.o = true;
/* 356 */     this.j = par1;
/* 357 */     this.k = par3;
/*     */   }
/*     */ 
/*     */   
/*     */   public void b(int par1) {
/* 362 */     this.p = true;
/* 363 */     this.l = par1;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void a(float par1, float par2, float par3) {
/* 371 */     a((int)(par1 * 255.0F), (int)(par2 * 255.0F), (int)(par3 * 255.0F));
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void a(float par1, float par2, float par3, float par4) {
/* 379 */     a((int)(par1 * 255.0F), (int)(par2 * 255.0F), (int)(par3 * 255.0F), (int)(par4 * 255.0F));
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void a(int par1, int par2, int par3) {
/* 387 */     a(par1, par2, par3, 255);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void a(int par1, int par2, int par3, int par4) {
/* 395 */     if (this.t) {
/*     */       return;
/*     */     }
/*     */     
/* 399 */     if (par1 > 255)
/*     */     {
/* 401 */       par1 = 255;
/*     */     }
/* 403 */     if (par2 > 255)
/*     */     {
/* 405 */       par2 = 255;
/*     */     }
/* 407 */     if (par3 > 255)
/*     */     {
/* 409 */       par3 = 255;
/*     */     }
/* 411 */     if (par4 > 255)
/*     */     {
/* 413 */       par4 = 255;
/*     */     }
/* 415 */     if (par1 < 0)
/*     */     {
/* 417 */       par1 = 0;
/*     */     }
/* 419 */     if (par2 < 0)
/*     */     {
/* 421 */       par2 = 0;
/*     */     }
/* 423 */     if (par3 < 0)
/*     */     {
/* 425 */       par3 = 0;
/*     */     }
/* 427 */     if (par4 < 0)
/*     */     {
/* 429 */       par4 = 0;
/*     */     }
/* 431 */     this.n = true;
/* 432 */     if (littleEndianByteOrder) {
/*     */       
/* 434 */       this.m = par4 << 24 | par3 << 16 | par2 << 8 | par1;
/*     */     } else {
/*     */       
/* 437 */       this.m = par1 << 24 | par2 << 16 | par3 << 8 | par4;
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void a(double par1, double par3, double par5, double par7, double par9) {
/* 446 */     a(par7, par9);
/* 447 */     a(par1, par3, par5);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void a(double par1, double par3, double par5) {
/* 456 */     if (this.autoGrow && this.r >= this.E - 32) {
/*     */       
/* 458 */       Config.dbg("Expand tessellator buffer, old: " + this.E + ", new: " + (this.E * 2));
/* 459 */       this.E *= 2;
/* 460 */       int[] newRawBuffer = new int[this.E];
/* 461 */       System.arraycopy(this.h, 0, newRawBuffer, 0, this.h.length);
/* 462 */       this.h = newRawBuffer;
/* 463 */       this.d = ew.c(this.E * 4);
/* 464 */       this.e = this.d.asIntBuffer();
/* 465 */       this.f = this.d.asFloatBuffer();
/* 466 */       this.g = this.d.asShortBuffer();
/*     */     } 
/* 468 */     this.s++;
/* 469 */     if (this.u == 7 && b && this.s % 4 == 0)
/*     */     {
/* 471 */       for (int i = 0; i < 2; i++) {
/*     */         
/* 473 */         int j = 8 * (3 - i);
/* 474 */         if (this.o) {
/*     */           
/* 476 */           this.h[this.r + 3] = this.h[this.r - j + 3];
/* 477 */           this.h[this.r + 4] = this.h[this.r - j + 4];
/*     */         } 
/* 479 */         if (this.p)
/*     */         {
/* 481 */           this.h[this.r + 7] = this.h[this.r - j + 7];
/*     */         }
/* 483 */         if (this.n)
/*     */         {
/* 485 */           this.h[this.r + 5] = this.h[this.r - j + 5];
/*     */         }
/* 487 */         this.h[this.r + 0] = this.h[this.r - j + 0];
/* 488 */         this.h[this.r + 1] = this.h[this.r - j + 1];
/* 489 */         this.h[this.r + 2] = this.h[this.r - j + 2];
/* 490 */         this.i++;
/* 491 */         this.r += 8;
/*     */       } 
/*     */     }
/* 494 */     if (this.o) {
/*     */       
/* 496 */       this.h[this.r + 3] = Float.floatToRawIntBits((float)this.j);
/* 497 */       this.h[this.r + 4] = Float.floatToRawIntBits((float)this.k);
/*     */     } 
/* 499 */     if (this.p)
/*     */     {
/* 501 */       this.h[this.r + 7] = this.l;
/*     */     }
/* 503 */     if (this.n)
/*     */     {
/* 505 */       this.h[this.r + 5] = this.m;
/*     */     }
/* 507 */     if (this.q)
/*     */     {
/* 509 */       this.h[this.r + 6] = this.y;
/*     */     }
/* 511 */     this.h[this.r + 0] = Float.floatToRawIntBits((float)(par1 + this.v));
/* 512 */     this.h[this.r + 1] = Float.floatToRawIntBits((float)(par3 + this.w));
/* 513 */     this.h[this.r + 2] = Float.floatToRawIntBits((float)(par5 + this.x));
/* 514 */     this.r += 8;
/* 515 */     this.i++;
/* 516 */     if (!this.autoGrow && this.s % 4 == 0 && this.r >= this.E - 32) {
/*     */       
/* 518 */       a();
/* 519 */       this.z = true;
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void c(int par1) {
/* 528 */     int i = par1 >> 16 & 0xFF;
/* 529 */     int j = par1 >> 8 & 0xFF;
/* 530 */     int k = par1 & 0xFF;
/* 531 */     a(i, j, k);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void a(int par1, int par2) {
/* 539 */     int i = par1 >> 16 & 0xFF;
/* 540 */     int j = par1 >> 8 & 0xFF;
/* 541 */     int k = par1 & 0xFF;
/* 542 */     a(i, j, k, par2);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void c() {
/* 550 */     this.t = true;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void b(float par1, float par2, float par3) {
/* 558 */     this.q = true;
/* 559 */     byte byte0 = (byte)(int)(par1 * 127.0F);
/* 560 */     byte byte1 = (byte)(int)(par2 * 127.0F);
/* 561 */     byte byte2 = (byte)(int)(par3 * 127.0F);
/* 562 */     this.y = byte0 & 0xFF | (byte1 & 0xFF) << 8 | (byte2 & 0xFF) << 16;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void b(double par1, double par3, double par5) {
/* 570 */     this.v = par1;
/* 571 */     this.w = par3;
/* 572 */     this.x = par5;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void c(float par1, float par2, float par3) {
/* 580 */     this.v += par1;
/* 581 */     this.w += par2;
/* 582 */     this.x += par3;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public boolean isRenderingChunk() {
/* 590 */     return this.renderingChunk;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void setRenderingChunk(boolean renderingChunk) {
/* 598 */     if (this.renderingChunk != renderingChunk)
/*     */     {
/*     */       
/* 601 */       for (int i = 0; i < this.subTextures.length; i++)
/*     */       {
/*     */         
/* 604 */         this.subTextures[i] = 0;
/*     */       }
/*     */     }
/*     */     
/* 608 */     this.renderingChunk = renderingChunk;
/*     */     
/* 610 */     if (this.textureUpdateTime != Config.getTextureUpdateTime()) {
/*     */       
/* 612 */       this.terrainTexture = 0;
/* 613 */       this.textureUpdateTime = Config.getTextureUpdateTime();
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public adz getSubTessellator(int tex) {
/* 622 */     adz newTess = getSubTessellatorImpl(tex);
/*     */     
/* 624 */     if (!newTess.z) {
/* 625 */       newTess.a(this.u);
/*     */     }
/* 627 */     newTess.l = this.l;
/* 628 */     newTess.p = this.p;
/* 629 */     newTess.m = this.m;
/* 630 */     newTess.n = this.n;
/* 631 */     newTess.y = this.y;
/* 632 */     newTess.q = this.q;
/* 633 */     newTess.renderingChunk = this.renderingChunk;
/* 634 */     newTess.defaultTexture = false;
/* 635 */     newTess.v = this.v;
/* 636 */     newTess.w = this.w;
/* 637 */     newTess.x = this.x;
/*     */     
/* 639 */     return newTess;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public adz getSubTessellatorImpl(int tex) {
/*     */     int i;
/* 648 */     for (i = 0; i < this.subTextures.length; i++) {
/*     */       
/* 650 */       int subTex = this.subTextures[i];
/* 651 */       if (subTex == tex) {
/*     */         
/* 653 */         adz adz1 = this.subTessellators[i];
/* 654 */         return adz1;
/*     */       } 
/*     */     } 
/*     */     
/* 658 */     for (i = 0; i < this.subTextures.length; i++) {
/*     */       
/* 660 */       int subTex = this.subTextures[i];
/* 661 */       if (subTex <= 0) {
/*     */ 
/*     */         
/* 664 */         adz adz1 = this.subTessellators[i];
/*     */         
/* 666 */         this.subTextures[i] = tex;
/*     */         
/* 668 */         return adz1;
/*     */       } 
/*     */     } 
/*     */     
/* 672 */     adz newTess = new adz();
/* 673 */     newTess.textureID = tex;
/*     */     
/* 675 */     adz[] oldTess = this.subTessellators;
/* 676 */     int[] oldTexs = this.subTextures;
/*     */     
/* 678 */     this.subTessellators = new adz[oldTess.length + 1];
/* 679 */     this.subTextures = new int[oldTexs.length + 1];
/* 680 */     System.arraycopy(oldTess, 0, this.subTessellators, 0, oldTess.length);
/* 681 */     System.arraycopy(oldTexs, 0, this.subTextures, 0, oldTexs.length);
/*     */     
/* 683 */     this.subTessellators[oldTess.length] = newTess;
/* 684 */     this.subTextures[oldTexs.length] = tex;
/*     */     
/* 686 */     Config.dbg("Allocated subtessellator, count: " + this.subTessellators.length);
/*     */     
/* 688 */     return newTess;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private int getTerrainTexture() {
/* 696 */     if (this.terrainTexture == 0) {
/* 697 */       this.terrainTexture = (Config.getMinecraft()).p.b("/terrain.png");
/*     */     }
/* 699 */     return this.terrainTexture;
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\adz.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */