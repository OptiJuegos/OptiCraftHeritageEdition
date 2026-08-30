/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ public class nq
/*    */   implements amd
/*    */ {
/* 12 */   private wc a = r.a();
/*    */   
/*    */   private double b;
/*    */   
/*    */   public void a(double par1, double par3, double par5) {
/* 17 */     this.b = par1;
/* 18 */     this.c = par3;
/* 19 */     this.d = par5;
/*    */   }
/*    */ 
/*    */   
/*    */   private double c;
/*    */   private double d;
/*    */   
/*    */   public boolean a(double par1, double par3, double par5, double par7, double par9, double par11) {
/* 27 */     return this.a.a(par1 - this.b, par3 - this.c, par5 - this.d, par7 - this.b, par9 - this.c, par11 - this.d);
/*    */   }
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */   
/*    */   public boolean a(wu par1AxisAlignedBB) {
/* 35 */     return a(par1AxisAlignedBB.a, par1AxisAlignedBB.b, par1AxisAlignedBB.c, par1AxisAlignedBB.d, par1AxisAlignedBB.e, par1AxisAlignedBB.f);
/*    */   }
/*    */ 
/*    */ 
/*    */   
/*    */   public boolean isBoxInFrustumFully(double minX, double minY, double minZ, double maxX, double maxY, double maxZ) {
/* 41 */     return this.a.isBoxInFrustumFully(minX - this.b, minY - this.c, minZ - this.d, maxX - this.b, maxY - this.c, maxZ - this.d);
/*    */   }
/*    */ 
/*    */ 
/*    */   
/*    */   public boolean isBoundingBoxInFrustumFully(wu aab) {
/* 47 */     return isBoxInFrustumFully(aab.a, aab.b, aab.c, aab.d, aab.e, aab.f);
/*    */   }
/*    */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\nq.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */