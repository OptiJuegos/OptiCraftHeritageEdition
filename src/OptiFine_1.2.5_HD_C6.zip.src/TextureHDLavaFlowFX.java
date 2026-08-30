/*     */ 
/*     */ 
/*     */ public class TextureHDLavaFlowFX
/*     */   extends tt
/*     */   implements TextureHDFX
/*     */ {
/*     */   private h texturePackBase;
/*     */   private int tileWidth;
/*     */   protected float[] buf1;
/*     */   protected float[] buf2;
/*     */   protected float[] buf3;
/*     */   protected float[] buf4;
/*     */   int tickCounter;
/*     */   
/*     */   public TextureHDLavaFlowFX() {
/*  16 */     super(pb.C.bN + 1);
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */     
/* 131 */     this.tileWidth = 0;
/*     */     this.tileWidth = 16;
/*     */     this.f = new byte[this.tileWidth * this.tileWidth * 4];
/*     */     this.buf1 = new float[this.tileWidth * this.tileWidth];
/*     */     this.buf2 = new float[this.tileWidth * this.tileWidth];
/*     */     this.buf3 = new float[this.tileWidth * this.tileWidth];
/*     */     this.buf4 = new float[this.tileWidth * this.tileWidth];
/*     */     this.tickCounter = 0;
/*     */     this.j = 2;
/*     */   }
/*     */   
/*     */   public void setTileWidth(int tileWidth) {
/*     */     if (tileWidth > Config.getMaxDynamicTileWidth())
/*     */       tileWidth = Config.getMaxDynamicTileWidth(); 
/*     */     this.tileWidth = tileWidth;
/*     */     this.f = new byte[tileWidth * tileWidth * 4];
/*     */     this.buf1 = new float[tileWidth * tileWidth];
/*     */     this.buf2 = new float[tileWidth * tileWidth];
/*     */     this.buf3 = new float[tileWidth * tileWidth];
/*     */     this.buf4 = new float[tileWidth * tileWidth];
/*     */   }
/*     */   
/*     */   public void setTexturePackBase(h tpb) {
/*     */     this.texturePackBase = tpb;
/*     */   }
/*     */   
/*     */   public void a() {
/*     */     if (!Config.isAnimatedLava())
/*     */       this.f = null; 
/*     */     if (this.f == null)
/*     */       return; 
/*     */     this.tickCounter++;
/*     */     int widthMask = this.tileWidth - 1;
/*     */     for (int x = 0; x < this.tileWidth; x++) {
/*     */       for (int y = 0; y < this.tileWidth; y++) {
/*     */         float f = 0.0F;
/*     */         int l = (int)(gk.a(y * 3.141593F * 2.0F / 16.0F) * 1.2F);
/*     */         int i1 = (int)(gk.a(x * 3.141593F * 2.0F / 16.0F) * 1.2F);
/*     */         for (int k1 = x - 1; k1 <= x + 1; k1++) {
/*     */           for (int i2 = y - 1; i2 <= y + 1; i2++) {
/*     */             int k2 = k1 + l & widthMask;
/*     */             int i3 = i2 + i1 & widthMask;
/*     */             f += this.buf1[k2 + i3 * this.tileWidth];
/*     */           } 
/*     */         } 
/*     */         this.buf2[x + y * this.tileWidth] = f / 10.0F + (this.buf3[(x + 0 & widthMask) + (y + 0 & widthMask) * this.tileWidth] + this.buf3[(x + 1 & widthMask) + (y + 0 & widthMask) * this.tileWidth] + this.buf3[(x + 1 & widthMask) + (y + 1 & widthMask) * this.tileWidth] + this.buf3[(x + 0 & widthMask) + (y + 1 & widthMask) * this.tileWidth]) / 4.0F * 0.8F;
/*     */         this.buf3[x + y * this.tileWidth] = this.buf3[x + y * this.tileWidth] + this.buf4[x + y * this.tileWidth] * 0.01F;
/*     */         if (this.buf3[x + y * this.tileWidth] < 0.0F)
/*     */           this.buf3[x + y * this.tileWidth] = 0.0F; 
/*     */         this.buf4[x + y * this.tileWidth] = this.buf4[x + y * this.tileWidth] - 0.06F;
/*     */         if (Math.random() < 0.005D)
/*     */           this.buf4[x + y * this.tileWidth] = 1.5F; 
/*     */       } 
/*     */     } 
/*     */     float[] af = this.buf2;
/*     */     this.buf2 = this.buf1;
/*     */     this.buf1 = af;
/*     */     int widthMask2 = this.tileWidth * this.tileWidth - 1;
/*     */     for (int k = 0; k < this.tileWidth * this.tileWidth; k++) {
/*     */       float f1 = this.buf1[k - this.tickCounter / 3 * this.tileWidth & widthMask2] * 2.0F;
/*     */       if (f1 > 1.0F)
/*     */         f1 = 1.0F; 
/*     */       if (f1 < 0.0F)
/*     */         f1 = 0.0F; 
/*     */       float f2 = f1;
/*     */       int j1 = (int)(f2 * 100.0F + 155.0F);
/*     */       int l1 = (int)(f2 * f2 * 255.0F);
/*     */       int j2 = (int)(f2 * f2 * f2 * f2 * 128.0F);
/*     */       if (this.h) {
/*     */         int l2 = (j1 * 30 + l1 * 59 + j2 * 11) / 100;
/*     */         int j3 = (j1 * 30 + l1 * 70) / 100;
/*     */         int k3 = (j1 * 30 + j2 * 70) / 100;
/*     */         j1 = l2;
/*     */         l1 = j3;
/*     */         j2 = k3;
/*     */       } 
/*     */       this.f[k * 4 + 0] = (byte)j1;
/*     */       this.f[k * 4 + 1] = (byte)l1;
/*     */       this.f[k * 4 + 2] = (byte)j2;
/*     */       this.f[k * 4 + 3] = -1;
/*     */     } 
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\TextureHDLavaFlowFX.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */