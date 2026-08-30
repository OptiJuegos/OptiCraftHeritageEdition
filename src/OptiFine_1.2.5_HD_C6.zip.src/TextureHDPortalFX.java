/*     */ import java.util.Random;
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ public class TextureHDPortalFX
/*     */   extends tt
/*     */   implements TextureHDFX
/*     */ {
/*     */   private int tileWidth;
/*     */   private int tickCounter;
/*     */   private byte[][] buffer;
/*     */   
/*     */   public TextureHDPortalFX() {
/*  16 */     super(pb.be.bN);
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */     
/* 134 */     this.tileWidth = 0;
/*     */     this.tileWidth = 16;
/*     */     this.tickCounter = 0;
/*     */     setup();
/*     */   }
/*     */   
/*     */   public void setTileWidth(int tileWidth) {
/*     */     if (tileWidth > Config.getMaxDynamicTileWidth())
/*     */       tileWidth = Config.getMaxDynamicTileWidth(); 
/*     */     this.tileWidth = tileWidth;
/*     */     setup();
/*     */     this.tickCounter = 0;
/*     */   }
/*     */   
/*     */   public void setTexturePackBase(h tpb) {}
/*     */   
/*     */   private void setup() {
/*     */     this.f = new byte[this.tileWidth * this.tileWidth * 4];
/*     */     this.buffer = new byte[32][this.tileWidth * this.tileWidth * 4];
/*     */     Random random = new Random(100L);
/*     */     for (int i = 0; i < 32; i++) {
/*     */       for (int x = 0; x < this.tileWidth; x++) {
/*     */         for (int y = 0; y < this.tileWidth; y++) {
/*     */           float f = 0.0F;
/*     */           for (int l = 0; l < 2; l++) {
/*     */             float f1 = (l * this.tileWidth / 2);
/*     */             float f2 = (l * this.tileWidth / 2);
/*     */             float f3 = (x - f1) / this.tileWidth * 2.0F;
/*     */             float f4 = (y - f2) / this.tileWidth * 2.0F;
/*     */             if (f3 < -1.0F)
/*     */               f3 += 2.0F; 
/*     */             if (f3 >= 1.0F)
/*     */               f3 -= 2.0F; 
/*     */             if (f4 < -1.0F)
/*     */               f4 += 2.0F; 
/*     */             if (f4 >= 1.0F)
/*     */               f4 -= 2.0F; 
/*     */             float f5 = f3 * f3 + f4 * f4;
/*     */             float f6 = (float)Math.atan2(f4, f3) + (i / 32.0F * 3.141593F * 2.0F - f5 * 10.0F + (l * 2)) * (l * 2 - 1);
/*     */             f6 = (gk.a(f6) + 1.0F) / 2.0F;
/*     */             f6 /= f5 + 1.0F;
/*     */             f += f6 * 0.5F;
/*     */           } 
/*     */           f += random.nextFloat() * 0.1F;
/*     */           int i1 = (int)(f * 100.0F + 155.0F);
/*     */           int j1 = (int)(f * f * 200.0F + 55.0F);
/*     */           int k1 = (int)(f * f * f * f * 255.0F);
/*     */           int l1 = (int)(f * 100.0F + 155.0F);
/*     */           int pos = y * this.tileWidth + x;
/*     */           this.buffer[i][pos * 4 + 0] = (byte)j1;
/*     */           this.buffer[i][pos * 4 + 1] = (byte)k1;
/*     */           this.buffer[i][pos * 4 + 2] = (byte)i1;
/*     */           this.buffer[i][pos * 4 + 3] = (byte)l1;
/*     */         } 
/*     */       } 
/*     */     } 
/*     */   }
/*     */   
/*     */   public void a() {
/*     */     if (!Config.isAnimatedPortal())
/*     */       this.f = null; 
/*     */     if (this.f == null)
/*     */       return; 
/*     */     this.tickCounter++;
/*     */     byte[] abyte0 = this.buffer[this.tickCounter & 0x1F];
/*     */     for (int i = 0; i < this.tileWidth * this.tileWidth; i++) {
/*     */       int j = abyte0[i * 4 + 0] & 0xFF;
/*     */       int k = abyte0[i * 4 + 1] & 0xFF;
/*     */       int l = abyte0[i * 4 + 2] & 0xFF;
/*     */       int i1 = abyte0[i * 4 + 3] & 0xFF;
/*     */       if (this.h) {
/*     */         int j1 = (j * 30 + k * 59 + l * 11) / 100;
/*     */         int k1 = (j * 30 + k * 70) / 100;
/*     */         int l1 = (j * 30 + l * 70) / 100;
/*     */         j = j1;
/*     */         k = k1;
/*     */         l = l1;
/*     */       } 
/*     */       this.f[i * 4 + 0] = (byte)j;
/*     */       this.f[i * 4 + 1] = (byte)k;
/*     */       this.f[i * 4 + 2] = (byte)l;
/*     */       this.f[i * 4 + 3] = (byte)i1;
/*     */     } 
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\TextureHDPortalFX.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */