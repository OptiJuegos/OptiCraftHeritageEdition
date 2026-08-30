/*     */ import net.minecraft.client.Minecraft;
/*     */ import org.lwjgl.opengl.GL11;
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ public class ItemRendererHD
/*     */   extends mn
/*     */ {
/*     */   private Minecraft minecraft;
/*     */   
/*     */   public ItemRendererHD(Minecraft minecraft) {
/*  22 */     super(minecraft);
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */     
/* 215 */     this.minecraft = null;
/*     */     this.minecraft = minecraft;
/*     */   }
/*     */   
/*     */   public void a(acq entityliving, aan itemstack, int ix) {
/*     */     boolean hasForge = Reflector.hasClass(2);
/*     */     if (hasForge) {
/*     */       Object itemRenderType = Reflector.getFieldValue(60);
/*     */       Object customRenderer = Reflector.call(20, new Object[] { itemstack, itemRenderType });
/*     */       if (customRenderer != null) {
/*     */         super.a(entityliving, itemstack, ix);
/*     */         return;
/*     */       } 
/*     */     } 
/*     */     if (itemstack.c < 256 && vl.a(pb.m[itemstack.c].d())) {
/*     */       super.a(entityliving, itemstack, ix);
/*     */       return;
/*     */     } 
/*     */     int num = Config.getIconWidthTerrain();
/*     */     if (num <= 16) {
/*     */       super.a(entityliving, itemstack, ix);
/*     */       return;
/*     */     } 
/*     */     GL11.glPushMatrix();
/*     */     if (itemstack.c < 256) {
/*     */       String texture = "/terrain.png";
/*     */       if (hasForge)
/*     */         texture = Reflector.callString(12, new Object[] { "/terrain.png", itemstack.a() }); 
/*     */       GL11.glBindTexture(3553, this.minecraft.p.b(texture));
/*     */       num = Config.getIconWidthTerrain();
/*     */     } else {
/*     */       String texture = "/gui/items.png";
/*     */       if (hasForge)
/*     */         texture = Reflector.callString(12, new Object[] { "/gui/items.png", itemstack.a() }); 
/*     */       GL11.glBindTexture(3553, this.minecraft.p.b(texture));
/*     */       num = Config.getIconWidthItems();
/*     */     } 
/*     */     adz tessellator = adz.a;
/*     */     int i = entityliving.b(itemstack, ix);
/*     */     float f = ((i % 16 * 16) + 0.0F) / 256.0F;
/*     */     float f1 = ((i % 16 * 16) + 15.99F) / 256.0F;
/*     */     float f2 = ((i / 16 * 16) + 0.0F) / 256.0F;
/*     */     float f3 = ((i / 16 * 16) + 15.99F) / 256.0F;
/*     */     float f4 = 0.0F;
/*     */     float f5 = 0.3F;
/*     */     GL11.glEnable(32826);
/*     */     GL11.glTranslatef(-f4, -f5, 0.0F);
/*     */     float f6 = 1.5F;
/*     */     GL11.glScalef(f6, f6, f6);
/*     */     GL11.glRotatef(50.0F, 0.0F, 1.0F, 0.0F);
/*     */     GL11.glRotatef(335.0F, 0.0F, 0.0F, 1.0F);
/*     */     GL11.glTranslatef(-0.9375F, -0.0625F, 0.0F);
/*     */     renderItem3D(tessellator, f1, f2, f, f3, num);
/*     */     if (itemstack != null && itemstack.r() && ix == 0) {
/*     */       GL11.glDepthFunc(514);
/*     */       GL11.glDisable(2896);
/*     */       this.minecraft.p.b(this.minecraft.p.b("%blur%/misc/glint.png"));
/*     */       GL11.glEnable(3042);
/*     */       GL11.glBlendFunc(768, 1);
/*     */       float f7 = 0.76F;
/*     */       GL11.glColor4f(0.5F * f7, 0.25F * f7, 0.8F * f7, 1.0F);
/*     */       GL11.glMatrixMode(5890);
/*     */       GL11.glPushMatrix();
/*     */       float f8 = 0.125F;
/*     */       GL11.glScalef(f8, f8, f8);
/*     */       float f9 = (float)(System.currentTimeMillis() % 3000L) / 3000.0F * 8.0F;
/*     */       GL11.glTranslatef(f9, 0.0F, 0.0F);
/*     */       GL11.glRotatef(-50.0F, 0.0F, 0.0F, 1.0F);
/*     */       renderItem3D(tessellator, 0.0F, 0.0F, 1.0F, 1.0F, num);
/*     */       GL11.glPopMatrix();
/*     */       GL11.glPushMatrix();
/*     */       GL11.glScalef(f8, f8, f8);
/*     */       f9 = (float)(System.currentTimeMillis() % 4873L) / 4873.0F * 8.0F;
/*     */       GL11.glTranslatef(-f9, 0.0F, 0.0F);
/*     */       GL11.glRotatef(10.0F, 0.0F, 0.0F, 1.0F);
/*     */       renderItem3D(tessellator, 0.0F, 0.0F, 1.0F, 1.0F, num);
/*     */       GL11.glPopMatrix();
/*     */       GL11.glMatrixMode(5888);
/*     */       GL11.glDisable(3042);
/*     */       GL11.glEnable(2896);
/*     */       GL11.glDepthFunc(515);
/*     */     } 
/*     */     GL11.glDisable(32826);
/*     */     GL11.glPopMatrix();
/*     */   }
/*     */   
/*     */   private void renderItem3D(adz tessellator, float f1, float f2, float f, float f3, int num) {
/*     */     float f4 = 1.0F;
/*     */     float f8 = 0.0625F;
/*     */     tessellator.b();
/*     */     tessellator.b(0.0F, 0.0F, 1.0F);
/*     */     tessellator.a(0.0D, 0.0D, 0.0D, f1, f3);
/*     */     tessellator.a(f4, 0.0D, 0.0D, f, f3);
/*     */     tessellator.a(f4, 1.0D, 0.0D, f, f2);
/*     */     tessellator.a(0.0D, 1.0D, 0.0D, f1, f2);
/*     */     tessellator.a();
/*     */     tessellator.b();
/*     */     tessellator.b(0.0F, 0.0F, -1.0F);
/*     */     tessellator.a(0.0D, 1.0D, (0.0F - f8), f1, f2);
/*     */     tessellator.a(f4, 1.0D, (0.0F - f8), f, f2);
/*     */     tessellator.a(f4, 0.0D, (0.0F - f8), f, f3);
/*     */     tessellator.a(0.0D, 0.0D, (0.0F - f8), f1, f3);
/*     */     tessellator.a();
/*     */     float du = 1.0F / (32 * num);
/*     */     float dz = 1.0F / num;
/*     */     tessellator.b();
/*     */     tessellator.b(-1.0F, 0.0F, 0.0F);
/*     */     for (int j = 0; j < num; j++) {
/*     */       float f9 = j / num * 1.0F;
/*     */       float f13 = f1 + (f - f1) * f9 - du;
/*     */       float f17 = f4 * f9;
/*     */       tessellator.a(f17, 0.0D, (0.0F - f8), f13, f3);
/*     */       tessellator.a(f17, 0.0D, 0.0D, f13, f3);
/*     */       tessellator.a(f17, 1.0D, 0.0D, f13, f2);
/*     */       tessellator.a(f17, 1.0D, (0.0F - f8), f13, f2);
/*     */     } 
/*     */     tessellator.a();
/*     */     tessellator.b();
/*     */     tessellator.b(1.0F, 0.0F, 0.0F);
/*     */     for (int k = 0; k < num; k++) {
/*     */       float f10 = k / num * 1.0F;
/*     */       float f14 = f1 + (f - f1) * f10 - du;
/*     */       float f18 = f4 * f10 + dz;
/*     */       tessellator.a(f18, 1.0D, (0.0F - f8), f14, f2);
/*     */       tessellator.a(f18, 1.0D, 0.0D, f14, f2);
/*     */       tessellator.a(f18, 0.0D, 0.0D, f14, f3);
/*     */       tessellator.a(f18, 0.0D, (0.0F - f8), f14, f3);
/*     */     } 
/*     */     tessellator.a();
/*     */     tessellator.b();
/*     */     tessellator.b(0.0F, 1.0F, 0.0F);
/*     */     for (int l = 0; l < num; l++) {
/*     */       float f11 = l / num * 1.0F;
/*     */       float f15 = f3 + (f2 - f3) * f11 - du;
/*     */       float f19 = f4 * f11 + dz;
/*     */       tessellator.a(0.0D, f19, 0.0D, f1, f15);
/*     */       tessellator.a(f4, f19, 0.0D, f, f15);
/*     */       tessellator.a(f4, f19, (0.0F - f8), f, f15);
/*     */       tessellator.a(0.0D, f19, (0.0F - f8), f1, f15);
/*     */     } 
/*     */     tessellator.a();
/*     */     tessellator.b();
/*     */     tessellator.b(0.0F, -1.0F, 0.0F);
/*     */     for (int i1 = 0; i1 < num; i1++) {
/*     */       float f12 = i1 / num * 1.0F;
/*     */       float f16 = f3 + (f2 - f3) * f12 - du;
/*     */       float f20 = f4 * f12;
/*     */       tessellator.a(f4, f20, 0.0D, f, f16);
/*     */       tessellator.a(0.0D, f20, 0.0D, f1, f16);
/*     */       tessellator.a(0.0D, f20, (0.0F - f8), f1, f16);
/*     */       tessellator.a(f4, f20, (0.0F - f8), f, f16);
/*     */     } 
/*     */     tessellator.a();
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\ItemRendererHD.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */