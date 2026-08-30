/*     */ import java.util.Random;
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ public class TextureHDFlamesFX
/*     */   extends tt
/*     */   implements TextureHDFX
/*     */ {
/*     */   private int tileWidth;
/*     */   private int fireHeight;
/*     */   protected float[] buf1;
/*     */   protected float[] buf2;
/*     */   private Random random;
/*     */   
/*     */   public TextureHDFlamesFX(int i) {
/*  17 */     super(pb.ar.bN + i * 16);
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
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
/* 132 */     this.fireHeight = 0;
/*     */ 
/*     */     
/* 135 */     this.random = new Random();
/*     */     this.tileWidth = 16;
/*     */     this.fireHeight = this.tileWidth + this.tileWidth / 4;
/*     */     this.f = new byte[this.tileWidth * this.tileWidth * 4];
/*     */     this.buf1 = new float[this.tileWidth * this.fireHeight];
/*     */     this.buf2 = new float[this.tileWidth * this.fireHeight];
/*     */   }
/*     */   
/*     */   public void setTileWidth(int tileWidth) {
/*     */     if (tileWidth > Config.getMaxDynamicTileWidth())
/*     */       tileWidth = Config.getMaxDynamicTileWidth(); 
/*     */     this.tileWidth = tileWidth;
/*     */     this.fireHeight = tileWidth + tileWidth / 4;
/*     */     this.f = new byte[tileWidth * tileWidth * 4];
/*     */     this.buf1 = new float[tileWidth * this.fireHeight];
/*     */     this.buf2 = new float[tileWidth * this.fireHeight];
/*     */   }
/*     */   
/*     */   public void setTexturePackBase(h tpb) {}
/*     */   
/*     */   public void a() {
/*     */     if (!Config.isAnimatedFire())
/*     */       this.f = null; 
/*     */     if (this.f == null)
/*     */       return; 
/*     */     float kFire = 1.01F + 0.8F / this.tileWidth;
/*     */     float kDensity = 3.0F + this.tileWidth / 16.0F;
/*     */     for (int x = 0; x < this.tileWidth; x++) {
/*     */       for (int y = 0; y < this.fireHeight; y++) {
/*     */         int l = this.fireHeight - this.tileWidth / 8;
/*     */         float f1 = this.buf1[x + (y + 1) % this.fireHeight * this.tileWidth] * l;
/*     */         for (int i1 = x - 1; i1 <= x + 1; i1++) {
/*     */           for (int k1 = y; k1 <= y + 1; k1++) {
/*     */             int i2 = i1;
/*     */             int k2 = k1;
/*     */             if (i2 >= 0 && k2 >= 0 && i2 < this.tileWidth && k2 < this.fireHeight)
/*     */               f1 += this.buf1[i2 + k2 * this.tileWidth]; 
/*     */             l++;
/*     */           } 
/*     */         } 
/*     */         this.buf2[x + y * this.tileWidth] = f1 / l * kFire;
/*     */         if (y >= this.fireHeight - this.tileWidth / 16)
/*     */           this.buf2[x + y * this.tileWidth] = this.random.nextFloat() * this.random.nextFloat() * this.random.nextFloat() * kDensity + this.random.nextFloat() * 0.1F + 0.2F; 
/*     */       } 
/*     */     } 
/*     */     float[] af = this.buf2;
/*     */     this.buf2 = this.buf1;
/*     */     this.buf1 = af;
/*     */     int tileWidth2 = this.tileWidth * this.tileWidth;
/*     */     for (int k = 0; k < tileWidth2; k++) {
/*     */       float f = this.buf1[k] * 1.8F;
/*     */       if (f > 1.0F)
/*     */         f = 1.0F; 
/*     */       if (f < 0.0F)
/*     */         f = 0.0F; 
/*     */       float f2 = f;
/*     */       int j1 = (int)(f2 * 155.0F + 100.0F);
/*     */       int l1 = (int)(f2 * f2 * 255.0F);
/*     */       int j2 = (int)(f2 * f2 * f2 * f2 * f2 * f2 * f2 * f2 * f2 * f2 * 255.0F);
/*     */       char c = 'ÿ';
/*     */       if (f2 < 0.5F)
/*     */         c = Character.MIN_VALUE; 
/*     */       f2 = (f2 - 0.5F) * 2.0F;
/*     */       if (this.h) {
/*     */         int l2 = (j1 * 30 + l1 * 59 + j2 * 11) / 100;
/*     */         int i3 = (j1 * 30 + l1 * 70) / 100;
/*     */         int j3 = (j1 * 30 + j2 * 70) / 100;
/*     */         j1 = l2;
/*     */         l1 = i3;
/*     */         j2 = j3;
/*     */       } 
/*     */       int k4 = k * 4;
/*     */       this.f[k4 + 0] = (byte)j1;
/*     */       this.f[k4 + 1] = (byte)l1;
/*     */       this.f[k4 + 2] = (byte)j2;
/*     */       this.f[k4 + 3] = (byte)c;
/*     */     } 
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\TextureHDFlamesFX.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */