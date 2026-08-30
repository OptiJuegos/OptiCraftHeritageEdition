/*     */ import java.awt.image.BufferedImage;
/*     */ import java.io.IOException;
/*     */ import javax.imageio.ImageIO;
/*     */ import net.minecraft.client.Minecraft;
/*     */ 
/*     */ 
/*     */ public class TextureHDCompassFX
/*     */   extends tt
/*     */   implements TextureHDFX
/*     */ {
/*     */   private Minecraft mc;
/*     */   private int tileWidth;
/*     */   private h texturePackBase;
/*     */   private byte[] baseImageData;
/*     */   private int[] compassIconImageData;
/*     */   private double showAngle;
/*     */   private double angleDiff;
/*     */   
/*     */   public TextureHDCompassFX(Minecraft minecraft) {
/*  20 */     super(yr.aQ.b(0));
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */     
/* 183 */     this.tileWidth = 0;
/* 184 */     this.texturePackBase = null;
/*     */     this.mc = minecraft;
/*     */     this.tileWidth = 16;
/*     */     setup();
/*     */   }
/*     */   
/*     */   public void setTileWidth(int tileWidth) {
/*     */     this.tileWidth = tileWidth;
/*     */     setup();
/*     */   }
/*     */   
/*     */   public void setTexturePackBase(h tpb) {
/*     */     this.texturePackBase = tpb;
/*     */   }
/*     */   
/*     */   private void setup() {
/*     */     this.f = new byte[this.tileWidth * this.tileWidth * 4];
/*     */     this.compassIconImageData = new int[this.tileWidth * this.tileWidth];
/*     */     this.k = 1;
/*     */     try {
/*     */       BufferedImage bufferedimage = ImageIO.read(Minecraft.class.getResource("/gui/items.png"));
/*     */       if (this.texturePackBase != null)
/*     */         bufferedimage = ImageIO.read(this.texturePackBase.a("/gui/items.png")); 
/*     */       this.tileWidth = bufferedimage.getWidth() / 16;
/*     */       this.f = new byte[this.tileWidth * this.tileWidth * 4];
/*     */       this.compassIconImageData = new int[this.tileWidth * this.tileWidth];
/*     */       int x = this.g % 16 * this.tileWidth;
/*     */       int y = this.g / 16 * this.tileWidth;
/*     */       bufferedimage.getRGB(x, y, this.tileWidth, this.tileWidth, this.compassIconImageData, 0, this.tileWidth);
/*     */       this.baseImageData = new byte[this.f.length];
/*     */       int tileWidth2 = this.tileWidth * this.tileWidth;
/*     */       for (int i = 0; i < tileWidth2; i++) {
/*     */         int j = this.compassIconImageData[i] >> 24 & 0xFF;
/*     */         int k = this.compassIconImageData[i] >> 16 & 0xFF;
/*     */         int l = this.compassIconImageData[i] >> 8 & 0xFF;
/*     */         int i1 = this.compassIconImageData[i] >> 0 & 0xFF;
/*     */         if (this.h) {
/*     */           int j1 = (k * 30 + l * 59 + i1 * 11) / 100;
/*     */           int k1 = (k * 30 + l * 70) / 100;
/*     */           int l1 = (k * 30 + i1 * 70) / 100;
/*     */           k = j1;
/*     */           l = k1;
/*     */           i1 = l1;
/*     */         } 
/*     */         this.baseImageData[i * 4 + 0] = (byte)k;
/*     */         this.baseImageData[i * 4 + 1] = (byte)l;
/*     */         this.baseImageData[i * 4 + 2] = (byte)i1;
/*     */         this.baseImageData[i * 4 + 3] = (byte)j;
/*     */       } 
/*     */     } catch (IOException ioexception) {
/*     */       ioexception.printStackTrace();
/*     */     } 
/*     */   }
/*     */   
/*     */   public void a() {
/*     */     int tileWidth2 = this.tileWidth * this.tileWidth;
/*     */     double cx = (this.tileWidth / 2) + 0.5D;
/*     */     double cy = (this.tileWidth / 2) - 0.5D;
/*     */     double needleLen = 0.3D * (this.tileWidth / 16);
/*     */     System.arraycopy(this.baseImageData, 0, this.f, 0, this.f.length);
/*     */     double d = 0.0D;
/*     */     if (this.mc.f != null && this.mc.h != null) {
/*     */       uh chunkcoordinates = this.mc.f.x();
/*     */       double d2 = chunkcoordinates.a - this.mc.h.o;
/*     */       double d4 = chunkcoordinates.c - this.mc.h.q;
/*     */       d = (this.mc.h.u - 90.0F) * Math.PI / 180.0D - Math.atan2(d4, d2);
/*     */       if (!this.mc.f.t.e())
/*     */         d = Math.random() * 3.1415927410125732D * 2.0D; 
/*     */     } 
/*     */     double d1;
/*     */     for (d1 = d - this.showAngle; d1 < -3.141592653589793D; d1 += 6.283185307179586D);
/*     */     for (; d1 >= Math.PI; d1 -= 6.283185307179586D);
/*     */     if (d1 < -1.0D)
/*     */       d1 = -1.0D; 
/*     */     if (d1 > 1.0D)
/*     */       d1 = 1.0D; 
/*     */     this.angleDiff += d1 * 0.1D;
/*     */     this.angleDiff *= 0.8D;
/*     */     this.showAngle += this.angleDiff;
/*     */     double d3 = Math.sin(this.showAngle);
/*     */     double d5 = Math.cos(this.showAngle);
/*     */     for (int i2 = -4; i2 <= 4; i2++) {
/*     */       int k2 = (int)(cx + d5 * i2 * needleLen);
/*     */       int i3 = (int)(cy - d3 * i2 * needleLen * 0.5D);
/*     */       int k3 = i3 * this.tileWidth + k2;
/*     */       int i4 = 100;
/*     */       int k4 = 100;
/*     */       int i5 = 100;
/*     */       char c = 'ÿ';
/*     */       if (this.h) {
/*     */         int k5 = (i4 * 30 + k4 * 59 + i5 * 11) / 100;
/*     */         int i6 = (i4 * 30 + k4 * 70) / 100;
/*     */         int k6 = (i4 * 30 + i5 * 70) / 100;
/*     */         i4 = k5;
/*     */         k4 = i6;
/*     */         i5 = k6;
/*     */       } 
/*     */       int k34 = k3 * 4;
/*     */       this.f[k34 + 0] = (byte)i4;
/*     */       this.f[k34 + 1] = (byte)k4;
/*     */       this.f[k34 + 2] = (byte)i5;
/*     */       this.f[k34 + 3] = (byte)c;
/*     */     } 
/*     */     for (int j2 = -8; j2 <= 16; j2++) {
/*     */       int l2 = (int)(cx + d3 * j2 * needleLen);
/*     */       int j3 = (int)(cy + d5 * j2 * needleLen * 0.5D);
/*     */       int l3 = j3 * this.tileWidth + l2;
/*     */       int j4 = (j2 < 0) ? 100 : 255;
/*     */       int l4 = (j2 < 0) ? 100 : 20;
/*     */       int j5 = (j2 < 0) ? 100 : 20;
/*     */       char c1 = 'ÿ';
/*     */       if (this.h) {
/*     */         int l5 = (j4 * 30 + l4 * 59 + j5 * 11) / 100;
/*     */         int j6 = (j4 * 30 + l4 * 70) / 100;
/*     */         int l6 = (j4 * 30 + j5 * 70) / 100;
/*     */         j4 = l5;
/*     */         l4 = j6;
/*     */         j5 = l6;
/*     */       } 
/*     */       int l34 = l3 * 4;
/*     */       this.f[l34 + 0] = (byte)j4;
/*     */       this.f[l34 + 1] = (byte)l4;
/*     */       this.f[l34 + 2] = (byte)j5;
/*     */       this.f[l34 + 3] = (byte)c1;
/*     */     } 
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\TextureHDCompassFX.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */