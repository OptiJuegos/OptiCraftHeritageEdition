/*     */ import java.io.IOException;
/*     */ import java.io.InputStream;
/*     */ import java.util.HashMap;
/*     */ import java.util.List;
/*     */ import java.util.Map;
/*     */ import java.util.Random;
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ public class RandomMobs
/*     */ {
/*  19 */   private static Map textureVariantsMap = new HashMap<Object, Object>();
/*     */   
/*  21 */   private static Random random = new Random();
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static void entityLoaded(nn entity) {
/*  28 */     if (entity.Z != null) {
/*     */       return;
/*     */     }
/*  31 */     if (!(entity instanceof acq)) {
/*     */       return;
/*     */     }
/*  34 */     if (entity instanceof yw) {
/*     */       return;
/*     */     }
/*  37 */     acq el = (acq)entity;
/*     */     
/*  39 */     int randomId = el.persistentId;
/*     */     
/*  41 */     entity.Z = "" + randomId;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static void worldChanged(xd oldWorld, xd newWorld) {
/*  50 */     if (newWorld != null) {
/*     */ 
/*     */       
/*  53 */       List<nn> entityList = newWorld.s();
/*  54 */       for (int e = 0; e < entityList.size(); e++) {
/*     */         
/*  56 */         nn entity = entityList.get(e);
/*  57 */         entityLoaded(entity);
/*     */       } 
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static int getTexture(String skinUrl, String texture) {
/*  70 */     if (texture == null) {
/*  71 */       return -1;
/*     */     }
/*  73 */     if (skinUrl == null)
/*  74 */       return -1; 
/*  75 */     if (skinUrl.length() <= 1) {
/*  76 */       return -1;
/*     */     }
/*  78 */     char ch = skinUrl.charAt(0);
/*  79 */     if (ch < '0' || ch > '9') {
/*  80 */       return -1;
/*     */     }
/*  82 */     int num = Math.abs(skinUrl.hashCode());
/*     */     
/*  84 */     String[] texs = (String[])textureVariantsMap.get(texture);
/*  85 */     if (texs == null) {
/*     */       
/*  87 */       texs = getTextureVariants(texture);
/*  88 */       textureVariantsMap.put(texture, texs);
/*     */     } 
/*  90 */     if (texs == null || texs.length <= 0) {
/*  91 */       return -1;
/*     */     }
/*  93 */     int index = num % texs.length;
/*     */     
/*  95 */     String tex = texs[index];
/*     */     
/*  97 */     return (Config.getMinecraft()).p.b(tex);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static String[] getTextureVariants(String texture) {
/* 105 */     aaw renderEngine = (Config.getMinecraft()).p;
/*     */     
/* 107 */     String[] texs = { texture };
/*     */     
/* 109 */     int pointPos = texture.lastIndexOf('.');
/* 110 */     if (pointPos < 0) {
/* 111 */       return texs;
/*     */     }
/* 113 */     String prefix = texture.substring(0, pointPos);
/* 114 */     String suffix = texture.substring(pointPos);
/*     */     
/* 116 */     int countVariants = getCountTextureVariants(texture, prefix, suffix);
/* 117 */     if (countVariants <= 1) {
/* 118 */       return texs;
/*     */     }
/* 120 */     texs = new String[countVariants];
/* 121 */     texs[0] = texture;
/* 122 */     for (int i = 1; i < texs.length; i++) {
/*     */       
/* 124 */       int texNum = i + 1;
/* 125 */       texs[i] = prefix + texNum + suffix;
/*     */     } 
/*     */     
/* 128 */     Config.dbg("RandomMobs: " + texture + ", variants: " + texs.length);
/*     */     
/* 130 */     return texs;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static int getCountTextureVariants(String texture, String prefix, String suffix) {
/* 139 */     aaw renderEngine = (Config.getMinecraft()).p;
/*     */     
/* 141 */     int maxNum = 1000;
/*     */     
/* 143 */     for (int num = 2; num < maxNum; num++) {
/*     */       
/* 145 */       String variant = prefix + num + suffix;
/*     */ 
/*     */       
/* 148 */       try { InputStream in = renderEngine.l.a.a(variant);
/* 149 */         if (in != null)
/*     */         
/*     */         { 
/* 152 */           in.close();
/*     */ 
/*     */           
/*     */            }
/*     */         
/*     */         else
/*     */         
/*     */         { 
/*     */           
/* 161 */           return num - 1; }  } catch (IOException e) { return num - 1; }
/*     */     
/*     */     } 
/* 164 */     return maxNum;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static void resetTextures() {
/* 171 */     textureVariantsMap.clear();
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\RandomMobs.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */