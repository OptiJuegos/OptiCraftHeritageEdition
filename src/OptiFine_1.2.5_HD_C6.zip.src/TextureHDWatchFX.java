/*     */ import java.awt.image.BufferedImage;
/*     */ import java.io.IOException;
/*     */ import javax.imageio.ImageIO;
/*     */ import net.minecraft.client.Minecraft;
/*     */ 
/*     */ 
/*     */ public class TextureHDWatchFX
/*     */   extends tt
/*     */   implements TextureHDFX
/*     */ {
/*     */   private Minecraft mc;
/*     */   private int tileWidth;
/*     */   private h texturePackBase;
/*     */   private int[] watchIconImageData;
/*     */   private int[] dialImageData;
/*     */   private byte[] watchBaseData;
/*     */   private byte[] dialBaseData;
/*     */   private double showAngle;
/*     */   private double angleDiff;
/*     */   
/*     */   public TextureHDWatchFX(Minecraft minecraft) {
/*  22 */     super(yr.aS.b(0));
/*  23 */     this.mc = minecraft;
/*  24 */     this.tileWidth = 16;
/*  25 */     setup();
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void setTileWidth(int tileWidth) {
/*  33 */     this.tileWidth = tileWidth;
/*  34 */     setup();
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public void setTexturePackBase(h tpb) {
/*  41 */     this.texturePackBase = tpb;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private void setup() {
/*  48 */     this.f = new byte[this.tileWidth * this.tileWidth * 4];
/*  49 */     this.watchIconImageData = new int[this.tileWidth * this.tileWidth];
/*  50 */     this.dialImageData = new int[this.tileWidth * this.tileWidth];
/*  51 */     this.k = 1;
/*     */     
/*     */     try {
/*  54 */       BufferedImage bufferedimage = ImageIO.read(Minecraft.class.getResource("/gui/items.png"));
/*  55 */       if (this.texturePackBase != null) {
/*  56 */         bufferedimage = ImageIO.read(this.texturePackBase.a("/gui/items.png"));
/*     */       }
/*  58 */       this.tileWidth = bufferedimage.getWidth() / 16;
/*  59 */       this.f = new byte[this.tileWidth * this.tileWidth * 4];
/*  60 */       this.watchIconImageData = new int[this.tileWidth * this.tileWidth];
/*  61 */       this.dialImageData = new int[this.tileWidth * this.tileWidth];
/*     */       
/*  63 */       int x = this.g % 16 * this.tileWidth;
/*  64 */       int y = this.g / 16 * this.tileWidth;
/*  65 */       bufferedimage.getRGB(x, y, this.tileWidth, this.tileWidth, this.watchIconImageData, 0, this.tileWidth);
/*  66 */       bufferedimage = ImageIO.read(Minecraft.class.getResource("/misc/dial.png"));
/*  67 */       if (this.texturePackBase != null)
/*  68 */         bufferedimage = ImageIO.read(this.texturePackBase.a("/misc/dial.png")); 
/*  69 */       if (bufferedimage.getWidth() != this.tileWidth)
/*  70 */         bufferedimage = aaw.scaleBufferedImage(bufferedimage, this.tileWidth, this.tileWidth); 
/*  71 */       bufferedimage.getRGB(0, 0, this.tileWidth, this.tileWidth, this.dialImageData, 0, this.tileWidth);
/*     */       
/*  73 */       this.watchBaseData = new byte[this.watchIconImageData.length * 4];
/*  74 */       this.dialBaseData = new byte[this.dialImageData.length * 4];
/*     */       
/*  76 */       int tileWidth2 = this.tileWidth * this.tileWidth; int i;
/*  77 */       for (i = 0; i < tileWidth2; i++) {
/*     */         
/*  79 */         int a = this.watchIconImageData[i] >> 24 & 0xFF;
/*  80 */         int r = this.watchIconImageData[i] >> 16 & 0xFF;
/*  81 */         int g = this.watchIconImageData[i] >> 8 & 0xFF;
/*  82 */         int b = this.watchIconImageData[i] >> 0 & 0xFF;
/*  83 */         if (r == b && g == 0 && b > 0)
/*     */         {
/*  85 */           int t = 0;
/*     */         }
/*  87 */         this.watchBaseData[i * 4 + 0] = (byte)r;
/*  88 */         this.watchBaseData[i * 4 + 1] = (byte)g;
/*  89 */         this.watchBaseData[i * 4 + 2] = (byte)b;
/*  90 */         this.watchBaseData[i * 4 + 3] = (byte)a;
/*     */       } 
/*     */       
/*  93 */       for (i = 0; i < tileWidth2; i++)
/*     */       {
/*  95 */         int a = this.dialImageData[i] >> 24 & 0xFF;
/*  96 */         int r = this.dialImageData[i] >> 16 & 0xFF;
/*  97 */         int g = this.dialImageData[i] >> 8 & 0xFF;
/*  98 */         int b = this.dialImageData[i] >> 0 & 0xFF;
/*  99 */         this.dialBaseData[i * 4 + 0] = (byte)r;
/* 100 */         this.dialBaseData[i * 4 + 1] = (byte)g;
/* 101 */         this.dialBaseData[i * 4 + 2] = (byte)b;
/* 102 */         this.dialBaseData[i * 4 + 3] = (byte)a;
/*     */       }
/*     */     
/* 105 */     } catch (IOException ioexception) {
/*     */       
/* 107 */       ioexception.printStackTrace();
/*     */     } 
/*     */   }
/*     */ 
/*     */   
/*     */   public void a() {
/* 113 */     double d = 0.0D;
/* 114 */     if (this.mc.f != null && this.mc.h != null) {
/*     */       
/* 116 */       float f = this.mc.f.c(1.0F);
/* 117 */       d = (-f * 3.141593F * 2.0F);
/* 118 */       if (!this.mc.f.t.e())
/*     */       {
/* 120 */         d = Math.random() * 3.1415927410125732D * 2.0D;
/*     */       }
/*     */     } 
/*     */     double d1;
/* 124 */     for (d1 = d - this.showAngle; d1 < -3.141592653589793D; d1 += 6.283185307179586D);
/* 125 */     for (; d1 >= Math.PI; d1 -= 6.283185307179586D);
/* 126 */     if (d1 < -1.0D)
/*     */     {
/* 128 */       d1 = -1.0D;
/*     */     }
/* 130 */     if (d1 > 1.0D)
/*     */     {
/* 132 */       d1 = 1.0D;
/*     */     }
/* 134 */     this.angleDiff += d1 * 0.1D;
/* 135 */     this.angleDiff *= 0.8D;
/* 136 */     this.showAngle += this.angleDiff;
/* 137 */     double d2 = Math.sin(this.showAngle);
/* 138 */     double d3 = Math.cos(this.showAngle);
/* 139 */     int tileWidth2 = this.tileWidth * this.tileWidth;
/* 140 */     int widthMask = this.tileWidth - 1;
/* 141 */     double widthMaskD = widthMask;
/* 142 */     for (int i = 0; i < tileWidth2; i++) {
/*     */       
/* 144 */       int i4 = i * 4;
/* 145 */       int r = this.watchBaseData[i4 + 0] & 0xFF;
/* 146 */       int g = this.watchBaseData[i4 + 1] & 0xFF;
/* 147 */       int b = this.watchBaseData[i4 + 2] & 0xFF;
/* 148 */       int a = this.watchBaseData[i4 + 3] & 0xFF;
/*     */       
/* 150 */       if (r == b && g == 0 && b > 0) {
/*     */         
/* 152 */         double d4 = -((i % this.tileWidth) / widthMaskD - 0.5D);
/* 153 */         double d5 = (i / this.tileWidth) / widthMaskD - 0.5D;
/* 154 */         int i2 = r;
/* 155 */         int dx = (int)((d4 * d3 + d5 * d2 + 0.5D) * this.tileWidth);
/* 156 */         int dy = (int)((d5 * d3 - d4 * d2 + 0.5D) * this.tileWidth);
/* 157 */         int pos = (dx & widthMask) + (dy & widthMask) * this.tileWidth;
/* 158 */         int pos4 = pos * 4;
/* 159 */         r = (this.dialBaseData[pos4 + 0] & 0xFF) * i2 / 255;
/* 160 */         g = (this.dialBaseData[pos4 + 1] & 0xFF) * i2 / 255;
/* 161 */         b = (this.dialBaseData[pos4 + 2] & 0xFF) * i2 / 255;
/* 162 */         a = this.dialBaseData[pos4 + 3] & 0xFF;
/*     */       } 
/* 164 */       if (this.h) {
/*     */         
/* 166 */         int j1 = (r * 30 + g * 59 + b * 11) / 100;
/* 167 */         int k1 = (r * 30 + g * 70) / 100;
/* 168 */         int l1 = (r * 30 + b * 70) / 100;
/* 169 */         r = j1;
/* 170 */         g = k1;
/* 171 */         b = l1;
/*     */       } 
/* 173 */       this.f[i4 + 0] = (byte)r;
/* 174 */       this.f[i4 + 1] = (byte)g;
/* 175 */       this.f[i4 + 2] = (byte)b;
/* 176 */       this.f[i4 + 3] = (byte)a;
/*     */     } 
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\TextureHDWatchFX.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */