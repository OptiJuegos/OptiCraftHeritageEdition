/*     */ 
/*     */ public class TextureHDWaterFX
/*     */   extends tt
/*     */   implements TextureHDFX
/*     */ {
/*     */   private h texturePackBase;
/*     */   private int tileWidth;
/*     */   protected float[] buf1;
/*     */   protected float[] buf2;
/*     */   protected float[] buf3;
/*     */   protected float[] buf4;
/*     */   private int tickCounter;
/*     */   
/*     */   public TextureHDWaterFX() {
/*  15 */     super(pb.A.bN);
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */     
/* 130 */     this.tileWidth = 0;
/*     */     this.tileWidth = 16;
/*     */     this.f = new byte[this.tileWidth * this.tileWidth * 4];
/*     */     this.buf1 = new float[this.tileWidth * this.tileWidth];
/*     */     this.buf2 = new float[this.tileWidth * this.tileWidth];
/*     */     this.buf3 = new float[this.tileWidth * this.tileWidth];
/*     */     this.buf4 = new float[this.tileWidth * this.tileWidth];
/*     */     this.tickCounter = 0;
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
/*     */     this.tickCounter = 0;
/*     */   }
/*     */   
/*     */   public void setTexturePackBase(h tpb) {
/*     */     this.texturePackBase = tpb;
/*     */   }
/*     */   
/*     */   public void a() {
/*     */     if (!Config.isAnimatedWater())
/*     */       this.f = null; 
/*     */     if (this.f == null)
/*     */       return; 
/*     */     this.tickCounter++;
/*     */     int widthMask = this.tileWidth - 1;
/*     */     for (int y = 0; y < this.tileWidth; y++) {
/*     */       for (int x = 0; x < this.tileWidth; x++) {
/*     */         float f = 0.0F;
/*     */         for (int s = y - 1; s <= y + 1; s++) {
/*     */           int tx = s & widthMask;
/*     */           int ty = x & widthMask;
/*     */           f += this.buf1[tx + ty * this.tileWidth];
/*     */         } 
/*     */         this.buf2[y + x * this.tileWidth] = f / 3.3F + this.buf3[y + x * this.tileWidth] * 0.8F;
/*     */       } 
/*     */     } 
/*     */     for (int j = 0; j < this.tileWidth; j++) {
/*     */       for (int l = 0; l < this.tileWidth; l++) {
/*     */         this.buf3[j + l * this.tileWidth] = this.buf3[j + l * this.tileWidth] + this.buf4[j + l * this.tileWidth] * 0.05F;
/*     */         if (this.buf3[j + l * this.tileWidth] < 0.0F)
/*     */           this.buf3[j + l * this.tileWidth] = 0.0F; 
/*     */         this.buf4[j + l * this.tileWidth] = this.buf4[j + l * this.tileWidth] - 0.1F;
/*     */         if (Math.random() < 0.05D)
/*     */           this.buf4[j + l * this.tileWidth] = 0.5F; 
/*     */       } 
/*     */     } 
/*     */     float[] af = this.buf2;
/*     */     this.buf2 = this.buf1;
/*     */     this.buf1 = af;
/*     */     for (int i = 0; i < this.tileWidth * this.tileWidth; i++) {
/*     */       float f1 = this.buf1[i];
/*     */       if (f1 > 1.0F)
/*     */         f1 = 1.0F; 
/*     */       if (f1 < 0.0F)
/*     */         f1 = 0.0F; 
/*     */       float f2 = f1 * f1;
/*     */       int r = (int)(32.0F + f2 * 32.0F);
/*     */       int g = (int)(50.0F + f2 * 64.0F);
/*     */       int b = 255;
/*     */       int a = (int)(146.0F + f2 * 50.0F);
/*     */       if (this.h) {
/*     */         int i3 = (r * 30 + g * 59 + b * 11) / 100;
/*     */         int j3 = (r * 30 + g * 70) / 100;
/*     */         int k3 = (r * 30 + b * 70) / 100;
/*     */         r = i3;
/*     */         g = j3;
/*     */         b = k3;
/*     */       } 
/*     */       this.f[i * 4 + 0] = (byte)r;
/*     */       this.f[i * 4 + 1] = (byte)g;
/*     */       this.f[i * 4 + 2] = (byte)b;
/*     */       this.f[i * 4 + 3] = (byte)a;
/*     */     } 
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\TextureHDWaterFX.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */