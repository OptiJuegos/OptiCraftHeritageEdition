/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ public class TextureHDCustomFX
/*    */   extends tt
/*    */   implements TextureHDFX
/*    */ {
/*    */   private h texturePackBase;
/*    */   private int tileWidth;
/*    */   
/*    */   public TextureHDCustomFX(int index, int tileImage) {
/* 18 */     super(index);
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
/*    */     
/* 46 */     this.tileWidth = 0;
/*    */     this.k = tileImage;
/*    */     this.tileWidth = 16;
/*    */     this.f = null;
/*    */   }
/*    */   
/*    */   public void setTileWidth(int tileWidth) {
/*    */     this.tileWidth = tileWidth;
/*    */   }
/*    */   
/*    */   public void setTexturePackBase(h tpb) {
/*    */     this.texturePackBase = tpb;
/*    */   }
/*    */   
/*    */   public void a() {}
/*    */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\TextureHDCustomFX.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */