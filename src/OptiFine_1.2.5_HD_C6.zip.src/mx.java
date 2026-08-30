/*     */ import java.awt.Graphics;
/*     */ import java.awt.image.BufferedImage;
/*     */ import java.awt.image.DataBufferInt;
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ public class mx
/*     */   implements js
/*     */ {
/*     */   private int[] a;
/*     */   private int b;
/*     */   private int c;
/*     */   
/*     */   public BufferedImage a(BufferedImage par1BufferedImage) {
/*  18 */     if (par1BufferedImage == null)
/*     */     {
/*  20 */       return null;
/*     */     }
/*  22 */     this.b = 64;
/*  23 */     this.c = 32;
/*     */     
/*  25 */     BufferedImage srcImg = par1BufferedImage;
/*  26 */     while (this.b < srcImg.getWidth() || this.c < srcImg.getHeight()) {
/*     */       
/*  28 */       this.b *= 2;
/*  29 */       this.c *= 2;
/*     */     } 
/*  31 */     BufferedImage bufferedimage = new BufferedImage(this.b, this.c, 2);
/*  32 */     Graphics g = bufferedimage.getGraphics();
/*  33 */     g.drawImage(par1BufferedImage, 0, 0, null);
/*  34 */     g.dispose();
/*  35 */     this.a = ((DataBufferInt)bufferedimage.getRaster().getDataBuffer()).getData();
/*     */     
/*  37 */     int w = this.b;
/*  38 */     int h = this.c;
/*     */     
/*  40 */     b(0, 0, w / 2, h / 2);
/*  41 */     a(w / 2, 0, w, h);
/*  42 */     b(0, h / 2, w, h);
/*  43 */     boolean flag = false;
/*  44 */     for (int i = w / 2; i < w; i++) {
/*     */       
/*  46 */       for (int k = 0; k < h / 2; k++) {
/*     */         
/*  48 */         int i1 = this.a[i + k * w];
/*  49 */         if ((i1 >> 24 & 0xFF) < 128)
/*     */         {
/*  51 */           flag = true;
/*     */         }
/*     */       } 
/*     */     } 
/*     */     
/*  56 */     if (!flag)
/*     */     {
/*  58 */       for (int j = w / 2; j < w; j++) {
/*     */         
/*  60 */         for (int l = 0; l < h / 2; l++) {
/*     */           
/*  62 */           int j1 = this.a[j + l * w];
/*     */           
/*  64 */           if ((j1 >> 24 & 0xFF) < 128)
/*     */           {
/*  66 */             boolean flag1 = true;
/*     */           }
/*     */         } 
/*     */       } 
/*     */     }
/*  71 */     return bufferedimage;
/*     */   }
/*     */ 
/*     */   
/*     */   private void a(int par1, int par2, int par3, int par4) {
/*  76 */     if (c(par1, par2, par3, par4)) {
/*     */       return;
/*     */     }
/*     */     
/*  80 */     for (int i = par1; i < par3; i++) {
/*     */       
/*  82 */       for (int j = par2; j < par4; j++)
/*     */       {
/*  84 */         this.a[i + j * this.b] = this.a[i + j * this.b] & 0xFFFFFF;
/*     */       }
/*     */     } 
/*     */   }
/*     */ 
/*     */   
/*     */   private void b(int par1, int par2, int par3, int par4) {
/*  91 */     for (int i = par1; i < par3; i++) {
/*     */       
/*  93 */       for (int j = par2; j < par4; j++)
/*     */       {
/*  95 */         this.a[i + j * this.b] = this.a[i + j * this.b] | 0xFF000000;
/*     */       }
/*     */     } 
/*     */   }
/*     */ 
/*     */   
/*     */   private boolean c(int par1, int par2, int par3, int par4) {
/* 102 */     for (int i = par1; i < par3; i++) {
/*     */       
/* 104 */       for (int j = par2; j < par4; j++) {
/*     */         
/* 106 */         int k = this.a[i + j * this.b];
/* 107 */         if ((k >> 24 & 0xFF) < 128)
/*     */         {
/* 109 */           return true;
/*     */         }
/*     */       } 
/*     */     } 
/*     */     
/* 114 */     return false;
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\mx.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */