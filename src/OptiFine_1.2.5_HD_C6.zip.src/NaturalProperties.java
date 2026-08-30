/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ public class NaturalProperties
/*    */ {
/* 16 */   public int rotation = 1;
/*    */ 
/*    */   
/*    */   public boolean flip = false;
/*    */ 
/*    */ 
/*    */   
/*    */   public NaturalProperties(String type) {
/* 24 */     if (type.equals("4")) {
/*    */       
/* 26 */       this.rotation = 4;
/*    */       return;
/*    */     } 
/* 29 */     if (type.equals("2")) {
/*    */       
/* 31 */       this.rotation = 2;
/*    */       return;
/*    */     } 
/* 34 */     if (type.equals("F")) {
/*    */       
/* 36 */       this.flip = true;
/*    */       return;
/*    */     } 
/* 39 */     if (type.equals("4F")) {
/*    */       
/* 41 */       this.rotation = 4;
/* 42 */       this.flip = true;
/*    */       return;
/*    */     } 
/* 45 */     if (type.equals("2F")) {
/*    */       
/* 47 */       this.rotation = 2;
/* 48 */       this.flip = true;
/*    */       
/*    */       return;
/*    */     } 
/* 52 */     Config.dbg("Unknown natural texture type: " + type);
/*    */   }
/*    */ 
/*    */ 
/*    */   
/*    */   public boolean isValid() {
/* 58 */     if (this.rotation == 2 || this.rotation == 4)
/* 59 */       return true; 
/* 60 */     if (this.flip) {
/* 61 */       return true;
/*    */     }
/* 63 */     return false;
/*    */   }
/*    */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\NaturalProperties.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */