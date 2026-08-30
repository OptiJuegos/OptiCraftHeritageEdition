/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ public class wc
/*     */ {
/*  12 */   public float[][] a = new float[16][16];
/*  13 */   public float[] b = new float[16];
/*  14 */   public float[] c = new float[16];
/*  15 */   public float[] d = new float[16];
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public boolean a(double minX, double minY, double minZ, double maxX, double maxY, double maxZ) {
/*  36 */     for (int i = 0; i < 6; i++) {
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */       
/*  50 */       float minXf = (float)minX;
/*  51 */       float minYf = (float)minY;
/*  52 */       float minZf = (float)minZ;
/*  53 */       float maxXf = (float)maxX;
/*  54 */       float maxYf = (float)maxY;
/*  55 */       float maxZf = (float)maxZ;
/*     */       
/*  57 */       if (this.a[i][0] * minXf + this.a[i][1] * minYf + this.a[i][2] * minZf + this.a[i][3] <= 0.0F && this.a[i][0] * maxXf + this.a[i][1] * minYf + this.a[i][2] * minZf + this.a[i][3] <= 0.0F && this.a[i][0] * minXf + this.a[i][1] * maxYf + this.a[i][2] * minZf + this.a[i][3] <= 0.0F && this.a[i][0] * maxXf + this.a[i][1] * maxYf + this.a[i][2] * minZf + this.a[i][3] <= 0.0F && this.a[i][0] * minXf + this.a[i][1] * minYf + this.a[i][2] * maxZf + this.a[i][3] <= 0.0F && this.a[i][0] * maxXf + this.a[i][1] * minYf + this.a[i][2] * maxZf + this.a[i][3] <= 0.0F && this.a[i][0] * minXf + this.a[i][1] * maxYf + this.a[i][2] * maxZf + this.a[i][3] <= 0.0F && this.a[i][0] * maxXf + this.a[i][1] * maxYf + this.a[i][2] * maxZf + this.a[i][3] <= 0.0F)
/*     */       {
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */         
/*  66 */         return false;
/*     */       }
/*     */     } 
/*  69 */     return true;
/*     */   }
/*     */ 
/*     */   
/*     */   public boolean isBoxInFrustumFully(double minX, double minY, double minZ, double maxX, double maxY, double maxZ) {
/*  74 */     for (int i = 0; i < 6; i++) {
/*     */       
/*  76 */       float minXf = (float)minX;
/*  77 */       float minYf = (float)minY;
/*  78 */       float minZf = (float)minZ;
/*  79 */       float maxXf = (float)maxX;
/*  80 */       float maxYf = (float)maxY;
/*  81 */       float maxZf = (float)maxZ;
/*     */       
/*  83 */       if (i < 4) {
/*     */ 
/*     */         
/*  86 */         if (this.a[i][0] * minXf + this.a[i][1] * minYf + this.a[i][2] * minZf + this.a[i][3] <= 0.0F || this.a[i][0] * maxXf + this.a[i][1] * minYf + this.a[i][2] * minZf + this.a[i][3] <= 0.0F || this.a[i][0] * minXf + this.a[i][1] * maxYf + this.a[i][2] * minZf + this.a[i][3] <= 0.0F || this.a[i][0] * maxXf + this.a[i][1] * maxYf + this.a[i][2] * minZf + this.a[i][3] <= 0.0F || this.a[i][0] * minXf + this.a[i][1] * minYf + this.a[i][2] * maxZf + this.a[i][3] <= 0.0F || this.a[i][0] * maxXf + this.a[i][1] * minYf + this.a[i][2] * maxZf + this.a[i][3] <= 0.0F || this.a[i][0] * minXf + this.a[i][1] * maxYf + this.a[i][2] * maxZf + this.a[i][3] <= 0.0F || this.a[i][0] * maxXf + this.a[i][1] * maxYf + this.a[i][2] * maxZf + this.a[i][3] <= 0.0F)
/*     */         {
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */           
/*  95 */           return false;
/*     */ 
/*     */         
/*     */         }
/*     */       
/*     */       }
/* 101 */       else if (this.a[i][0] * minXf + this.a[i][1] * minYf + this.a[i][2] * minZf + this.a[i][3] <= 0.0F && this.a[i][0] * maxXf + this.a[i][1] * minYf + this.a[i][2] * minZf + this.a[i][3] <= 0.0F && this.a[i][0] * minXf + this.a[i][1] * maxYf + this.a[i][2] * minZf + this.a[i][3] <= 0.0F && this.a[i][0] * maxXf + this.a[i][1] * maxYf + this.a[i][2] * minZf + this.a[i][3] <= 0.0F && this.a[i][0] * minXf + this.a[i][1] * minYf + this.a[i][2] * maxZf + this.a[i][3] <= 0.0F && this.a[i][0] * maxXf + this.a[i][1] * minYf + this.a[i][2] * maxZf + this.a[i][3] <= 0.0F && this.a[i][0] * minXf + this.a[i][1] * maxYf + this.a[i][2] * maxZf + this.a[i][3] <= 0.0F && this.a[i][0] * maxXf + this.a[i][1] * maxYf + this.a[i][2] * maxZf + this.a[i][3] <= 0.0F) {
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */         
/* 110 */         return false;
/*     */       } 
/*     */     } 
/*     */     
/* 114 */     return true;
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\wc.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */