/*     */ import java.io.InputStream;
/*     */ import java.util.ArrayList;
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ public class NaturalTextures
/*     */ {
/*  15 */   private static aaw renderEngine = null;
/*     */   
/*  17 */   private static NaturalProperties[][] propertiesByTex = new NaturalProperties[0][];
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static void update(aaw re) {
/*  24 */     propertiesByTex = new NaturalProperties[0][];
/*  25 */     renderEngine = re;
/*     */     
/*  27 */     if (!Config.isNaturalTextures()) {
/*     */       return;
/*     */     }
/*  30 */     InputStream in = re.l.a.a("/natural.properties");
/*  31 */     if (in == null) {
/*     */       
/*  33 */       Config.dbg("natural.properties not found");
/*  34 */       propertiesByTex = makeDefaultProperties();
/*     */       
/*     */       return;
/*     */     } 
/*     */     
/*     */     try {
/*  40 */       ArrayList<NaturalProperties[]> list = new ArrayList(1024);
/*  41 */       String configStr = Config.readInputStream(in);
/*  42 */       in.close();
/*  43 */       String[] configLines = Config.tokenize(configStr, "\n\r");
/*  44 */       Config.dbg("Parsing natural.properties");
/*  45 */       for (int i = 0; i < configLines.length; i++) {
/*     */         
/*  47 */         String line = configLines[i].trim();
/*     */         
/*  49 */         if (!line.startsWith("#")) {
/*     */ 
/*     */           
/*  52 */           String[] strs = Config.tokenize(line, "=");
/*  53 */           if (strs.length != 2) {
/*     */             
/*  55 */             Config.dbg("Invalid natural.properties line: " + line);
/*     */           } else {
/*     */             
/*  58 */             String key = strs[0].trim();
/*  59 */             String type = strs[1].trim();
/*     */             
/*  61 */             String[] keyStrs = Config.tokenize(key, ":");
/*  62 */             if (keyStrs.length != 2)
/*     */             
/*  64 */             { Config.dbg("Invalid natural.properties line: " + line); }
/*     */             
/*     */             else
/*     */             
/*  68 */             { String texName = keyStrs[0];
/*  69 */               String tileNumStr = keyStrs[1];
/*  70 */               int tileNum = Config.parseInt(tileNumStr, -1);
/*  71 */               if (tileNum < 0 || tileNum > 255)
/*     */               
/*  73 */               { Config.dbg("Invalid natural.properties line: " + line); }
/*     */               else
/*     */               
/*  76 */               { NaturalProperties props = new NaturalProperties(type);
/*  77 */                 if (props.isValid())
/*     */                 
/*     */                 { 
/*  80 */                   int texNum = re.b(texName);
/*  81 */                   if (texNum >= 0)
/*     */                   
/*     */                   { 
/*  84 */                     while (list.size() <= texNum) {
/*  85 */                       list.add(null);
/*     */                     }
/*  87 */                     NaturalProperties[] propsByTile = list.get(texNum);
/*  88 */                     if (propsByTile == null) {
/*     */                       
/*  90 */                       propsByTile = new NaturalProperties[256];
/*     */                       
/*  92 */                       list.set(texNum, propsByTile);
/*     */                     } 
/*     */                     
/*  95 */                     propsByTile[tileNum] = props; }  }  }  } 
/*     */           } 
/*     */         } 
/*  98 */       }  propertiesByTex = list.<NaturalProperties[]>toArray(new NaturalProperties[list.size()][]);
/*     */     }
/* 100 */     catch (Exception e) {
/*     */       
/* 102 */       e.printStackTrace();
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static NaturalProperties getNaturalProperties(int texId, int tileNum) {
/* 112 */     if (texId < 0) {
/* 113 */       return null;
/*     */     }
/* 115 */     if (texId == 0) {
/* 116 */       texId = renderEngine.terrainTextureId;
/*     */     }
/* 118 */     if (texId > propertiesByTex.length) {
/* 119 */       return null;
/*     */     }
/* 121 */     NaturalProperties[] propsByTile = propertiesByTex[texId];
/* 122 */     if (propsByTile == null) {
/* 123 */       return null;
/*     */     }
/* 125 */     if (tileNum < 0 || tileNum >= propsByTile.length) {
/* 126 */       return null;
/*     */     }
/* 128 */     NaturalProperties props = propsByTile[tileNum];
/*     */     
/* 130 */     return props;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static NaturalProperties[][] makeDefaultProperties() {
/* 139 */     if (!renderEngine.l.a.a.equals("Default")) {
/*     */       
/* 141 */       Config.dbg("Texture pack is not default, ignoring default configuration for Natural Textures.");
/* 142 */       return new NaturalProperties[0][];
/*     */     } 
/* 144 */     Config.dbg("Using default configuration for Natural Textures.");
/*     */     
/* 146 */     NaturalProperties[] terrainProps = new NaturalProperties[256];
/*     */     
/* 148 */     terrainProps[0] = new NaturalProperties("4F");
/*     */     
/* 150 */     terrainProps[1] = new NaturalProperties("2F");
/*     */     
/* 152 */     terrainProps[2] = new NaturalProperties("4F");
/*     */     
/* 154 */     terrainProps[3] = new NaturalProperties("F");
/* 155 */     terrainProps[38] = new NaturalProperties("F");
/*     */     
/* 157 */     terrainProps[6] = new NaturalProperties("F");
/*     */     
/* 159 */     terrainProps[17] = new NaturalProperties("2F");
/*     */     
/* 161 */     terrainProps[18] = new NaturalProperties("4F");
/*     */     
/* 163 */     terrainProps[19] = new NaturalProperties("4");
/*     */     
/* 165 */     terrainProps[20] = new NaturalProperties("2F");
/*     */     
/* 167 */     terrainProps[21] = new NaturalProperties("4F");
/*     */     
/* 169 */     terrainProps[32] = new NaturalProperties("2F");
/* 170 */     terrainProps[33] = new NaturalProperties("2F");
/* 171 */     terrainProps[34] = new NaturalProperties("2F");
/* 172 */     terrainProps[50] = new NaturalProperties("2F");
/* 173 */     terrainProps[51] = new NaturalProperties("2F");
/* 174 */     terrainProps[160] = new NaturalProperties("2F");
/*     */     
/* 176 */     terrainProps[37] = new NaturalProperties("4F");
/*     */     
/* 178 */     terrainProps[52] = new NaturalProperties("2F");
/* 179 */     terrainProps[53] = new NaturalProperties("2F");
/*     */     
/* 181 */     terrainProps[196] = new NaturalProperties("2");
/* 182 */     terrainProps[197] = new NaturalProperties("2");
/*     */     
/* 184 */     terrainProps[66] = new NaturalProperties("4F");
/*     */     
/* 186 */     terrainProps[68] = new NaturalProperties("F");
/*     */     
/* 188 */     terrainProps[70] = new NaturalProperties("2F");
/*     */     
/* 190 */     terrainProps[72] = new NaturalProperties("4F");
/*     */     
/* 192 */     terrainProps[77] = new NaturalProperties("F");
/*     */     
/* 194 */     terrainProps[78] = new NaturalProperties("4F");
/*     */     
/* 196 */     terrainProps[86] = new NaturalProperties("2F");
/*     */     
/* 198 */     terrainProps[87] = new NaturalProperties("2F");
/*     */     
/* 200 */     terrainProps[103] = new NaturalProperties("4F");
/*     */     
/* 202 */     terrainProps[104] = new NaturalProperties("4F");
/*     */     
/* 204 */     terrainProps[105] = new NaturalProperties("4");
/*     */     
/* 206 */     terrainProps[116] = new NaturalProperties("2F");
/*     */     
/* 208 */     terrainProps[117] = new NaturalProperties("F");
/*     */     
/* 210 */     terrainProps[132] = new NaturalProperties("2F");
/* 211 */     terrainProps[133] = new NaturalProperties("2F");
/*     */     
/* 213 */     terrainProps[153] = new NaturalProperties("2F");
/*     */     
/* 215 */     terrainProps[175] = new NaturalProperties("4");
/*     */     
/* 217 */     terrainProps[176] = new NaturalProperties("4");
/*     */     
/* 219 */     terrainProps[208] = new NaturalProperties("4F");
/*     */     
/* 221 */     terrainProps[211] = new NaturalProperties("4F");
/* 222 */     terrainProps[212] = new NaturalProperties("4F");
/*     */     
/* 224 */     int terrainTexId = renderEngine.terrainTextureId;
/* 225 */     NaturalProperties[][] defPropsByTex = new NaturalProperties[terrainTexId + 1][];
/* 226 */     defPropsByTex[terrainTexId] = terrainProps;
/*     */     
/* 228 */     return defPropsByTex;
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\NaturalTextures.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */