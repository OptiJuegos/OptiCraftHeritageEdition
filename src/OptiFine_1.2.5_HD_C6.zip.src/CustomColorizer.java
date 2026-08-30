/*      */ import java.io.IOException;
/*      */ import java.io.InputStream;
/*      */ import java.util.Arrays;
/*      */ import java.util.HashMap;
/*      */ import java.util.Iterator;
/*      */ import java.util.Map;
/*      */ import java.util.Properties;
/*      */ import java.util.Random;
/*      */ import java.util.Set;
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ public class CustomColorizer
/*      */ {
/*   27 */   private static int[] grassColors = null;
/*      */   
/*   29 */   private static int[] waterColors = null;
/*      */   
/*   31 */   private static int[] foliageColors = null;
/*   32 */   private static int[] foliagePineColors = null;
/*   33 */   private static int[] foliageBirchColors = null;
/*      */   
/*   35 */   private static int[] swampFoliageColors = null;
/*   36 */   private static int[] swampGrassColors = null;
/*      */ 
/*      */   
/*   39 */   private static int[][] blockPalettes = (int[][])null;
/*      */   
/*   41 */   private static int[][] paletteColors = (int[][])null;
/*      */   
/*   43 */   private static int[] skyColors = null;
/*      */   
/*   45 */   private static int[] fogColors = null;
/*      */   
/*   47 */   private static int[] underwaterColors = null;
/*      */   
/*   49 */   private static float[][][] lightMapsColorsRgb = (float[][][])null;
/*      */   
/*   51 */   private static float[][] sunRgbs = new float[16][3];
/*   52 */   private static float[][] torchRgbs = new float[16][3];
/*      */   
/*   54 */   private static int[] redstoneColors = null;
/*      */   
/*   56 */   private static int[] stemColors = null;
/*      */ 
/*      */ 
/*      */   
/*   60 */   private static int[] myceliumParticleColors = null;
/*      */   
/*   62 */   private static int particleWaterColor = -1;
/*   63 */   private static int particlePortalColor = -1;
/*   64 */   private static int lilyPadColor = -1;
/*   65 */   private static bo fogColorNether = null;
/*   66 */   private static bo fogColorEnd = null;
/*   67 */   private static bo skyColorEnd = null;
/*      */   
/*      */   private static final int TYPE_NONE = 0;
/*      */   
/*      */   private static final int TYPE_GRASS = 1;
/*      */   private static final int TYPE_FOLIAGE = 2;
/*   73 */   private static Random random = new Random();
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static void update(aaw re) {
/*   80 */     grassColors = null;
/*   81 */     waterColors = null;
/*   82 */     foliageColors = null;
/*   83 */     foliageBirchColors = null;
/*   84 */     foliagePineColors = null;
/*   85 */     swampGrassColors = null;
/*   86 */     swampFoliageColors = null;
/*   87 */     skyColors = null;
/*   88 */     fogColors = null;
/*   89 */     underwaterColors = null;
/*   90 */     redstoneColors = null;
/*   91 */     stemColors = null;
/*   92 */     myceliumParticleColors = null;
/*   93 */     lightMapsColorsRgb = (float[][][])null;
/*      */     
/*   95 */     lilyPadColor = -1;
/*   96 */     particleWaterColor = -1;
/*   97 */     particlePortalColor = -1;
/*   98 */     fogColorNether = null;
/*   99 */     fogColorEnd = null;
/*  100 */     skyColorEnd = null;
/*  101 */     blockPalettes = (int[][])null;
/*  102 */     paletteColors = (int[][])null;
/*      */     
/*  104 */     grassColors = getCustomColors("/misc/grasscolor.png", re, 65536);
/*  105 */     foliageColors = getCustomColors("/misc/foliagecolor.png", re, 65536);
/*      */     
/*  107 */     waterColors = getCustomColors("/misc/watercolorX.png", re, 65536);
/*      */     
/*  109 */     if (!Config.isCustomColors()) {
/*      */       return;
/*      */     }
/*  112 */     foliagePineColors = getCustomColors("/misc/pinecolor.png", re, 65536);
/*  113 */     foliageBirchColors = getCustomColors("/misc/birchcolor.png", re, 65536);
/*      */     
/*  115 */     swampGrassColors = getCustomColors("/misc/swampgrasscolor.png", re, 65536);
/*  116 */     swampFoliageColors = getCustomColors("/misc/swampfoliagecolor.png", re, 65536);
/*      */     
/*  118 */     skyColors = getCustomColors("/misc/skycolor0.png", re, 65536);
/*  119 */     fogColors = getCustomColors("/misc/fogcolor0.png", re, 65536);
/*  120 */     underwaterColors = getCustomColors("/misc/underwatercolor.png", re, 65536);
/*      */     
/*  122 */     redstoneColors = getCustomColors("/misc/redstonecolor.png", re, 16);
/*  123 */     stemColors = getCustomColors("/misc/stemcolor.png", re, 8);
/*      */     
/*  125 */     myceliumParticleColors = getCustomColors("/misc/myceliumparticlecolor.png", re, -1);
/*      */     
/*  127 */     int[][] lightMapsColors = new int[3][];
/*  128 */     lightMapsColorsRgb = new float[3][][];
/*  129 */     for (int i = 0; i < lightMapsColors.length; i++) {
/*      */       
/*  131 */       lightMapsColors[i] = getCustomColors("/environment/lightmap" + (i - 1) + ".png", re, -1);
/*  132 */       if (lightMapsColors[i] != null) {
/*  133 */         lightMapsColorsRgb[i] = toRgb(lightMapsColors[i]);
/*      */       }
/*      */     } 
/*  136 */     readColorProperties("/color.properties", re);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static float[][] toRgb(int[] cols) {
/*  144 */     float[][] colsRgb = new float[cols.length][3];
/*  145 */     for (int i = 0; i < cols.length; i++) {
/*      */       
/*  147 */       int col = cols[i];
/*  148 */       float rf = (col >> 16 & 0xFF) / 255.0F;
/*  149 */       float gf = (col >> 8 & 0xFF) / 255.0F;
/*  150 */       float bf = (col & 0xFF) / 255.0F;
/*  151 */       float[] colRgb = colsRgb[i];
/*  152 */       colRgb[0] = rf;
/*  153 */       colRgb[1] = gf;
/*  154 */       colRgb[2] = bf;
/*      */     } 
/*      */     
/*  157 */     return colsRgb;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static void readColorProperties(String fileName, aaw re) {
/*  165 */     InputStream in = (re.getTexturePack()).a.a(fileName);
/*  166 */     if (in == null) {
/*      */       return;
/*      */     }
/*      */     try {
/*  170 */       Config.log("Loading " + fileName);
/*  171 */       Properties props = new Properties();
/*  172 */       props.load(in);
/*      */       
/*  174 */       lilyPadColor = readColor(props, "lilypad");
/*  175 */       particleWaterColor = readColor(props, new String[] { "particle.water", "drop.water" });
/*  176 */       particlePortalColor = readColor(props, "particle.portal");
/*      */       
/*  178 */       fogColorNether = readColorVec3D(props, "fog.nether");
/*  179 */       fogColorEnd = readColorVec3D(props, "fog.end");
/*  180 */       skyColorEnd = readColorVec3D(props, "sky.end");
/*      */       
/*  182 */       readCustomPalettes(props, re);
/*      */     }
/*  184 */     catch (IOException e) {
/*      */       
/*  186 */       e.printStackTrace();
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static void readCustomPalettes(Properties props, aaw re) {
/*  196 */     blockPalettes = new int[256][1];
/*  197 */     for (int i = 0; i < 256; i++)
/*      */     {
/*  199 */       blockPalettes[i][0] = -1;
/*      */     }
/*      */     
/*  202 */     String palettePrefix = "palette.block.";
/*  203 */     Map<Object, Object> map = new HashMap<Object, Object>();
/*  204 */     Set<Object> keys = props.keySet();
/*  205 */     for (Iterator<String> iter = keys.iterator(); iter.hasNext(); ) {
/*      */       
/*  207 */       String key = iter.next();
/*  208 */       String value = props.getProperty(key);
/*  209 */       if (!key.startsWith(palettePrefix)) {
/*      */         continue;
/*      */       }
/*  212 */       map.put(key, value);
/*      */     } 
/*      */     
/*  215 */     String[] propNames = (String[])map.keySet().toArray((Object[])new String[map.size()]);
/*  216 */     paletteColors = new int[propNames.length][];
/*  217 */     for (int j = 0; j < propNames.length; j++) {
/*      */       
/*  219 */       String name = propNames[j];
/*  220 */       String value = props.getProperty(name);
/*  221 */       Config.log("Block palette: " + name + " = " + value);
/*  222 */       String path = name.substring(palettePrefix.length());
/*  223 */       int[] colors = getCustomColors(path, re, 65536);
/*  224 */       paletteColors[j] = colors;
/*      */       
/*  226 */       String[] indexStrs = Config.tokenize(value, " ,;");
/*  227 */       for (int ix = 0; ix < indexStrs.length; ix++) {
/*      */         
/*  229 */         String blockStr = indexStrs[ix];
/*      */         
/*  231 */         int metadata = -1;
/*  232 */         if (blockStr.contains(":")) {
/*      */           
/*  234 */           String[] blockStrs = Config.tokenize(blockStr, ":");
/*  235 */           blockStr = blockStrs[0];
/*  236 */           String metadataStr = blockStrs[1];
/*  237 */           metadata = Config.parseInt(metadataStr, -1);
/*      */           
/*  239 */           if (metadata < 0 || metadata > 15) {
/*      */             
/*  241 */             Config.log("Invalid block metadata: " + blockStr + " in palette: " + name);
/*      */             
/*      */             continue;
/*      */           } 
/*      */         } 
/*  246 */         int blockIndex = Config.parseInt(blockStr, -1);
/*  247 */         if (blockIndex < 0 || blockIndex > 255) {
/*      */           
/*  249 */           Config.log("Invalid block index: " + blockIndex + " in palette: " + name);
/*      */ 
/*      */         
/*      */         }
/*  253 */         else if (blockIndex != pb.u.bO && blockIndex != pb.X.bO && blockIndex != pb.K.bO && blockIndex != pb.bu.bO) {
/*      */ 
/*      */ 
/*      */           
/*  257 */           if (metadata == -1) {
/*      */ 
/*      */             
/*  260 */             blockPalettes[blockIndex][0] = j;
/*      */           
/*      */           }
/*      */           else {
/*      */             
/*  265 */             if ((blockPalettes[blockIndex]).length < 16) {
/*      */ 
/*      */               
/*  268 */               blockPalettes[blockIndex] = new int[16];
/*  269 */               Arrays.fill(blockPalettes[blockIndex], -1);
/*      */             } 
/*      */             
/*  272 */             blockPalettes[blockIndex][metadata] = j;
/*      */           } 
/*      */         } 
/*      */         continue;
/*      */       } 
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static int readColor(Properties props, String[] names) {
/*  285 */     for (int i = 0; i < names.length; i++) {
/*      */       
/*  287 */       String name = names[i];
/*  288 */       int col = readColor(props, name);
/*  289 */       if (col >= 0) {
/*  290 */         return col;
/*      */       }
/*      */     } 
/*  293 */     return -1;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static int readColor(Properties props, String name) {
/*  303 */     String str = props.getProperty(name);
/*  304 */     if (str == null) {
/*  305 */       return -1;
/*      */     }
/*      */     
/*      */     try {
/*  309 */       int val = Integer.parseInt(str, 16) & 0xFFFFFF;
/*  310 */       Config.log("Custom color: " + name + " = " + str);
/*      */       
/*  312 */       return val;
/*      */     }
/*  314 */     catch (NumberFormatException e) {
/*      */       
/*  316 */       Config.log("Invalid custom color: " + name + " = " + str);
/*  317 */       return -1;
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static bo readColorVec3D(Properties props, String name) {
/*  327 */     int col = readColor(props, name);
/*  328 */     if (col < 0) {
/*  329 */       return null;
/*      */     }
/*  331 */     int red = col >> 16 & 0xFF;
/*  332 */     int green = col >> 8 & 0xFF;
/*  333 */     int blue = col & 0xFF;
/*      */     
/*  335 */     float redF = red / 255.0F;
/*  336 */     float greenF = green / 255.0F;
/*  337 */     float blueF = blue / 255.0F;
/*      */     
/*  339 */     return bo.a(redF, greenF, blueF);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static int[] getCustomColors(String path, aaw re, int length) {
/*  350 */     InputStream in = (re.getTexturePack()).a.a(path);
/*  351 */     if (in == null) {
/*  352 */       return null;
/*      */     }
/*  354 */     int[] colors = re.a(path);
/*  355 */     if (colors == null) {
/*  356 */       return null;
/*      */     }
/*  358 */     if (length > 0 && colors.length != length) {
/*      */       
/*  360 */       Config.log("Invalid custom colors length: " + colors.length + ", path: " + path);
/*  361 */       return null;
/*      */     } 
/*      */     
/*  364 */     Config.log("Loading custom colors: " + path);
/*      */     
/*  366 */     return colors;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getColorMultiplier(pb block, ali blockAccess, int x, int y, int z) {
/*  374 */     if (foliageBirchColors == null && foliagePineColors == null && swampGrassColors == null && swampFoliageColors == null && blockPalettes == null && Config.isSwampColors() && Config.isSmoothBiomes())
/*      */     {
/*      */       
/*  377 */       return block.c(blockAccess, x, y, z);
/*      */     }
/*      */     
/*  380 */     int[] colors = null;
/*  381 */     int[] swampColors = null;
/*      */     
/*  383 */     if (blockPalettes != null) {
/*      */       
/*  385 */       int blockId = block.bO;
/*  386 */       if (blockId >= 0 && blockId < 256) {
/*      */         
/*  388 */         int[] metadataPals = blockPalettes[blockId];
/*  389 */         int paletteIx = -1;
/*      */         
/*  391 */         if (metadataPals.length > 1) {
/*      */ 
/*      */           
/*  394 */           int i = blockAccess.e(x, y, z);
/*  395 */           paletteIx = metadataPals[i];
/*      */         
/*      */         }
/*      */         else {
/*      */           
/*  400 */           paletteIx = metadataPals[0];
/*      */         } 
/*  402 */         if (paletteIx >= 0) {
/*  403 */           colors = paletteColors[paletteIx];
/*      */         }
/*      */       } 
/*  406 */       if (colors != null) {
/*      */ 
/*      */         
/*  409 */         if (Config.isSmoothBiomes()) {
/*  410 */           return getSmoothColorMultiplier(block, blockAccess, x, y, z, colors, swampColors, 0, 0);
/*      */         }
/*  412 */         return getCustomColor(colors, blockAccess, x, y, z);
/*      */       } 
/*      */     } 
/*      */     
/*  416 */     boolean useSwampColors = Config.isSwampColors();
/*  417 */     boolean smoothColors = false;
/*  418 */     int type = 0;
/*  419 */     int metadata = 0;
/*      */     
/*  421 */     if (block == pb.u || block == pb.X) {
/*      */ 
/*      */       
/*  424 */       type = 1;
/*  425 */       smoothColors = Config.isSmoothBiomes();
/*      */       
/*  427 */       colors = grassColors;
/*      */       
/*  429 */       if (useSwampColors) {
/*  430 */         swampColors = swampGrassColors;
/*      */       } else {
/*  432 */         swampColors = colors;
/*      */       } 
/*  434 */     } else if (block == pb.K) {
/*      */ 
/*      */       
/*  437 */       type = 2;
/*  438 */       smoothColors = Config.isSmoothBiomes();
/*      */       
/*  440 */       metadata = blockAccess.e(x, y, z);
/*  441 */       if ((metadata & 0x3) == 1) {
/*      */ 
/*      */         
/*  444 */         colors = foliagePineColors;
/*      */       }
/*  446 */       else if ((metadata & 0x3) == 2) {
/*      */ 
/*      */         
/*  449 */         colors = foliageBirchColors;
/*      */       
/*      */       }
/*      */       else {
/*      */         
/*  454 */         colors = foliageColors;
/*      */         
/*  456 */         if (useSwampColors) {
/*  457 */           swampColors = swampFoliageColors;
/*      */         } else {
/*  459 */           swampColors = colors;
/*      */         } 
/*      */       } 
/*  462 */     } else if (block == pb.bu) {
/*      */ 
/*      */       
/*  465 */       type = 2;
/*  466 */       smoothColors = Config.isSmoothBiomes();
/*      */       
/*  468 */       colors = foliageColors;
/*      */       
/*  470 */       if (useSwampColors) {
/*  471 */         swampColors = swampFoliageColors;
/*      */       } else {
/*  473 */         swampColors = colors;
/*      */       } 
/*      */     } 
/*  476 */     if (smoothColors) {
/*  477 */       return getSmoothColorMultiplier(block, blockAccess, x, y, z, colors, swampColors, type, metadata);
/*      */     }
/*  479 */     if (swampColors != colors)
/*      */     {
/*      */ 
/*      */       
/*  483 */       if (blockAccess.a(x, z) == abn.h) {
/*  484 */         colors = swampColors;
/*      */       }
/*      */     }
/*  487 */     if (colors != null)
/*      */     {
/*      */       
/*  490 */       return getCustomColor(colors, blockAccess, x, y, z);
/*      */     }
/*      */     
/*  493 */     return block.c(blockAccess, x, y, z);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static int getSmoothColorMultiplier(pb block, ali blockAccess, int x, int y, int z, int[] colors, int[] swampColors, int type, int metadata) {
/*  501 */     int sumRed = 0;
/*  502 */     int sumGreen = 0;
/*  503 */     int sumBlue = 0;
/*  504 */     for (int ix = x - 1; ix <= x + 1; ix++) {
/*      */       
/*  506 */       for (int iz = z - 1; iz <= z + 1; iz++) {
/*      */         
/*  508 */         int[] cols = colors;
/*      */         
/*  510 */         if (swampColors != cols)
/*      */         {
/*      */ 
/*      */           
/*  514 */           if (blockAccess.a(ix, iz) == abn.h) {
/*  515 */             cols = swampColors;
/*      */           }
/*      */         }
/*  518 */         int col = 0;
/*  519 */         if (cols == null) {
/*      */           
/*  521 */           switch (type) {
/*      */ 
/*      */             
/*      */             case 1:
/*  525 */               col = blockAccess.a(ix, iz).k();
/*      */               break;
/*      */             case 2:
/*  528 */               if ((metadata & 0x3) == 1) {
/*  529 */                 col = gu.a(); break;
/*  530 */               }  if ((metadata & 0x3) == 2) {
/*  531 */                 col = gu.b();
/*      */                 break;
/*      */               } 
/*  534 */               col = blockAccess.a(ix, iz).l();
/*      */               break;
/*      */             default:
/*  537 */               col = block.c(blockAccess, ix, y, iz);
/*      */               break;
/*      */           } 
/*      */         } else {
/*  541 */           col = getCustomColor(cols, blockAccess, ix, y, iz);
/*      */         } 
/*  543 */         sumRed += col >> 16 & 0xFF;
/*  544 */         sumGreen += col >> 8 & 0xFF;
/*  545 */         sumBlue += col & 0xFF;
/*      */       } 
/*      */     } 
/*      */     
/*  549 */     int r = sumRed / 9;
/*  550 */     int g = sumGreen / 9;
/*  551 */     int b = sumBlue / 9;
/*      */     
/*  553 */     return r << 16 | g << 8 | b;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getFluidColor(pb block, ali blockAccess, int x, int y, int z) {
/*  566 */     if (block.cd != acn.g) {
/*  567 */       return block.c(blockAccess, x, y, z);
/*      */     }
/*  569 */     if (waterColors != null) {
/*      */       
/*  571 */       if (Config.isSmoothBiomes()) {
/*  572 */         return getSmoothColor(waterColors, blockAccess, x, y, z, 3, 1);
/*      */       }
/*  574 */       return getCustomColor(waterColors, blockAccess, x, y, z);
/*      */     } 
/*      */     
/*  577 */     if (!Config.isSwampColors()) {
/*  578 */       return 16777215;
/*      */     }
/*  580 */     return block.c(blockAccess, x, y, z);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static int getCustomColor(int[] colors, ali blockAccess, int x, int y, int z) {
/*  592 */     abn bgb = blockAccess.a(x, z);
/*  593 */     double temperature = gk.a(bgb.j(), 0.0F, 1.0F);
/*  594 */     double rainfall = gk.a(bgb.i(), 0.0F, 1.0F);
/*      */     
/*  596 */     rainfall *= temperature;
/*  597 */     int cx = (int)((1.0D - temperature) * 255.0D);
/*  598 */     int cy = (int)((1.0D - rainfall) * 255.0D);
/*  599 */     return colors[cy << 8 | cx] & 0xFFFFFF;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static void updatePortalFX(rt fx) {
/*  606 */     if (particlePortalColor < 0) {
/*      */       return;
/*      */     }
/*  609 */     int col = particlePortalColor;
/*  610 */     int red = col >> 16 & 0xFF;
/*  611 */     int green = col >> 8 & 0xFF;
/*  612 */     int blue = col & 0xFF;
/*      */     
/*  614 */     float redF = red / 255.0F;
/*  615 */     float greenF = green / 255.0F;
/*  616 */     float blueF = blue / 255.0F;
/*      */     
/*  618 */     fx.ao = redF;
/*  619 */     fx.ap = greenF;
/*  620 */     fx.aq = blueF;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static void updateMyceliumFX(rt fx) {
/*  627 */     if (myceliumParticleColors == null) {
/*      */       return;
/*      */     }
/*  630 */     int col = myceliumParticleColors[random.nextInt(myceliumParticleColors.length)];
/*      */     
/*  632 */     int red = col >> 16 & 0xFF;
/*  633 */     int green = col >> 8 & 0xFF;
/*  634 */     int blue = col & 0xFF;
/*      */     
/*  636 */     float redF = red / 255.0F;
/*  637 */     float greenF = green / 255.0F;
/*  638 */     float blueF = blue / 255.0F;
/*      */     
/*  640 */     fx.ao = redF;
/*  641 */     fx.ap = greenF;
/*  642 */     fx.aq = blueF;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static void updateReddustFX(rt fx, ali blockAccess, double x, double y, double z) {
/*  650 */     if (redstoneColors == null) {
/*      */       return;
/*      */     }
/*  653 */     int level = blockAccess.e((int)x, (int)y, (int)z);
/*      */     
/*  655 */     int col = getRedstoneColor(level);
/*  656 */     if (col == -1) {
/*      */       return;
/*      */     }
/*  659 */     int red = col >> 16 & 0xFF;
/*  660 */     int green = col >> 8 & 0xFF;
/*  661 */     int blue = col & 0xFF;
/*      */     
/*  663 */     float redF = red / 255.0F;
/*  664 */     float greenF = green / 255.0F;
/*  665 */     float blueF = blue / 255.0F;
/*      */     
/*  667 */     fx.ao = redF;
/*  668 */     fx.ap = greenF;
/*  669 */     fx.aq = blueF;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getRedstoneColor(int level) {
/*  678 */     if (redstoneColors == null) {
/*  679 */       return -1;
/*      */     }
/*  681 */     if (level < 0 || level > 15) {
/*  682 */       return -1;
/*      */     }
/*  684 */     return redstoneColors[level] & 0xFFFFFF;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static void updateWaterFX(rt fx, ali blockAccess) {
/*  692 */     if (waterColors == null) {
/*      */       return;
/*      */     }
/*  695 */     int x = (int)fx.o;
/*  696 */     int y = (int)fx.p;
/*  697 */     int z = (int)fx.q;
/*      */     
/*  699 */     int col = getFluidColor(pb.B, blockAccess, x, y, z);
/*  700 */     int red = col >> 16 & 0xFF;
/*  701 */     int green = col >> 8 & 0xFF;
/*  702 */     int blue = col & 0xFF;
/*      */     
/*  704 */     float redF = red / 255.0F;
/*  705 */     float greenF = green / 255.0F;
/*  706 */     float blueF = blue / 255.0F;
/*      */     
/*  708 */     if (particleWaterColor >= 0) {
/*      */       
/*  710 */       int redDrop = particleWaterColor >> 16 & 0xFF;
/*  711 */       int greenDrop = particleWaterColor >> 8 & 0xFF;
/*  712 */       int blueDrop = particleWaterColor & 0xFF;
/*      */       
/*  714 */       redF *= redDrop / 255.0F;
/*  715 */       greenF *= greenDrop / 255.0F;
/*  716 */       blueF *= blueDrop / 255.0F;
/*      */     } 
/*      */     
/*  719 */     fx.ao = redF;
/*  720 */     fx.ap = greenF;
/*  721 */     fx.aq = blueF;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getLilypadColor() {
/*  728 */     if (lilyPadColor < 0) {
/*  729 */       return pb.bz.i();
/*      */     }
/*  731 */     return lilyPadColor;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static bo getFogColorNether(bo col) {
/*  738 */     if (fogColorNether == null) {
/*  739 */       return col;
/*      */     }
/*  741 */     return fogColorNether;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static bo getFogColorEnd(bo col) {
/*  748 */     if (fogColorEnd == null) {
/*  749 */       return col;
/*      */     }
/*  751 */     return fogColorEnd;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static bo getSkyColorEnd(bo col) {
/*  758 */     if (skyColorEnd == null) {
/*  759 */       return col;
/*      */     }
/*  761 */     return skyColorEnd;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static bo getSkyColor(bo skyColor3d, ali blockAccess, double x, double y, double z) {
/*  768 */     if (skyColors == null) {
/*  769 */       return skyColor3d;
/*      */     }
/*  771 */     int col = getSmoothColor(skyColors, blockAccess, x, y, z, 10, 1);
/*      */     
/*  773 */     int red = col >> 16 & 0xFF;
/*  774 */     int green = col >> 8 & 0xFF;
/*  775 */     int blue = col & 0xFF;
/*      */     
/*  777 */     float redF = red / 255.0F;
/*  778 */     float greenF = green / 255.0F;
/*  779 */     float blueF = blue / 255.0F;
/*      */ 
/*      */     
/*  782 */     float cRed = (float)skyColor3d.a / 0.5F;
/*      */     
/*  784 */     float cGreen = (float)skyColor3d.b / 0.66275F;
/*      */     
/*  786 */     float cBlue = (float)skyColor3d.c;
/*      */     
/*  788 */     redF *= cRed;
/*  789 */     greenF *= cGreen;
/*  790 */     blueF *= cBlue;
/*      */     
/*  792 */     return bo.a(redF, greenF, blueF);
/*      */   }
/*      */ 
/*      */ 
/*      */   
/*      */   public static bo getFogColor(bo fogColor3d, ali blockAccess, double x, double y, double z) {
/*  798 */     if (fogColors == null) {
/*  799 */       return fogColor3d;
/*      */     }
/*  801 */     int col = getSmoothColor(fogColors, blockAccess, x, y, z, 10, 1);
/*      */     
/*  803 */     int red = col >> 16 & 0xFF;
/*  804 */     int green = col >> 8 & 0xFF;
/*  805 */     int blue = col & 0xFF;
/*      */     
/*  807 */     float redF = red / 255.0F;
/*  808 */     float greenF = green / 255.0F;
/*  809 */     float blueF = blue / 255.0F;
/*      */ 
/*      */     
/*  812 */     float cRed = (float)fogColor3d.a / 0.753F;
/*      */     
/*  814 */     float cGreen = (float)fogColor3d.b / 0.8471F;
/*      */     
/*  816 */     float cBlue = (float)fogColor3d.c;
/*      */     
/*  818 */     redF *= cRed;
/*  819 */     greenF *= cGreen;
/*  820 */     blueF *= cBlue;
/*      */     
/*  822 */     return bo.a(redF, greenF, blueF);
/*      */   }
/*      */ 
/*      */ 
/*      */   
/*      */   public static bo getUnderwaterColor(ali blockAccess, double x, double y, double z) {
/*  828 */     if (underwaterColors == null) {
/*  829 */       return null;
/*      */     }
/*  831 */     int col = getSmoothColor(underwaterColors, blockAccess, x, y, z, 10, 1);
/*      */     
/*  833 */     int red = col >> 16 & 0xFF;
/*  834 */     int green = col >> 8 & 0xFF;
/*  835 */     int blue = col & 0xFF;
/*      */     
/*  837 */     float redF = red / 255.0F;
/*  838 */     float greenF = green / 255.0F;
/*  839 */     float blueF = blue / 255.0F;
/*      */     
/*  841 */     return bo.a(redF, greenF, blueF);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getSmoothColor(int[] colors, ali blockAccess, double x, double y, double z, int samples, int step) {
/*  848 */     if (colors == null) {
/*  849 */       return -1;
/*      */     }
/*  851 */     int x0 = (int)Math.floor(x);
/*  852 */     int y0 = (int)Math.floor(y);
/*  853 */     int z0 = (int)Math.floor(z);
/*  854 */     int n = samples * step / 2;
/*  855 */     int sumRed = 0;
/*  856 */     int sumGreen = 0;
/*  857 */     int sumBlue = 0;
/*  858 */     int count = 0; int ix;
/*  859 */     for (ix = x0 - n; ix <= x0 + n; ix += step) {
/*      */       int iz;
/*  861 */       for (iz = z0 - n; iz <= z0 + n; iz += step) {
/*      */ 
/*      */         
/*  864 */         int col = getCustomColor(colors, blockAccess, ix, y0, iz);
/*  865 */         sumRed += col >> 16 & 0xFF;
/*  866 */         sumGreen += col >> 8 & 0xFF;
/*  867 */         sumBlue += col & 0xFF;
/*  868 */         count++;
/*      */       } 
/*      */     } 
/*      */     
/*  872 */     int r = sumRed / count;
/*  873 */     int g = sumGreen / count;
/*  874 */     int b = sumBlue / count;
/*      */     
/*  876 */     return r << 16 | g << 8 | b;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int mixColors(int c1, int c2, float w1) {
/*  883 */     if (w1 <= 0.0F)
/*  884 */       return c2; 
/*  885 */     if (w1 >= 1.0F) {
/*  886 */       return c1;
/*      */     }
/*  888 */     float w2 = 1.0F - w1;
/*      */     
/*  890 */     int r1 = c1 >> 16 & 0xFF;
/*  891 */     int g1 = c1 >> 8 & 0xFF;
/*  892 */     int b1 = c1 & 0xFF;
/*      */     
/*  894 */     int r2 = c2 >> 16 & 0xFF;
/*  895 */     int g2 = c2 >> 8 & 0xFF;
/*  896 */     int b2 = c2 & 0xFF;
/*      */     
/*  898 */     int r = (int)(r1 * w1 + r2 * w2);
/*  899 */     int g = (int)(g1 * w1 + g2 * w2);
/*  900 */     int b = (int)(b1 * w1 + b2 * w2);
/*      */     
/*  902 */     return r << 16 | g << 8 | b;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static int averageColor(int c1, int c2) {
/*  909 */     int r1 = c1 >> 16 & 0xFF;
/*  910 */     int g1 = c1 >> 8 & 0xFF;
/*  911 */     int b1 = c1 & 0xFF;
/*      */     
/*  913 */     int r2 = c2 >> 16 & 0xFF;
/*  914 */     int g2 = c2 >> 8 & 0xFF;
/*  915 */     int b2 = c2 & 0xFF;
/*      */     
/*  917 */     int r = (r1 + r2) / 2;
/*  918 */     int g = (g1 + g2) / 2;
/*  919 */     int b = (b1 + b2) / 2;
/*      */     
/*  921 */     return r << 16 | g << 8 | b;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getStemColorMultiplier(aei blockStem, ali blockAccess, int x, int y, int z) {
/*  932 */     if (stemColors == null) {
/*  933 */       return blockStem.c(blockAccess, x, y, z);
/*      */     }
/*  935 */     int level = blockAccess.e(x, y, z);
/*  936 */     if (level < 0)
/*  937 */       level = 0; 
/*  938 */     if (level >= stemColors.length) {
/*  939 */       level = stemColors.length - 1;
/*      */     }
/*  941 */     return stemColors[level];
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean updateLightmap(xd world, lr entityRenderer, int[] lmColors) {
/*  950 */     if (world == null)
/*  951 */       return false; 
/*  952 */     if (lightMapsColorsRgb == null) {
/*  953 */       return false;
/*      */     }
/*  955 */     if (!Config.isCustomColors()) {
/*  956 */       return false;
/*      */     }
/*  958 */     int worldType = world.t.g;
/*  959 */     if (worldType < -1 || worldType > 1) {
/*  960 */       return false;
/*      */     }
/*  962 */     int lightMapIndex = worldType + 1;
/*  963 */     float[][] lightMapRgb = lightMapsColorsRgb[lightMapIndex];
/*  964 */     if (lightMapRgb == null) {
/*  965 */       return false;
/*      */     }
/*  967 */     int height = 32;
/*  968 */     int width = lightMapRgb.length / height;
/*  969 */     if (width < 16) {
/*      */       
/*  971 */       Config.dbg("Invalid lightmap width: " + width + " for: /environment/lightmap" + worldType + ".png");
/*  972 */       lightMapsColorsRgb[lightMapIndex] = (float[][])null;
/*  973 */       return false;
/*      */     } 
/*      */     
/*  976 */     float sun = 1.1666666F * (world.b(1.0F) - 0.2F);
/*  977 */     if (world.n > 0)
/*  978 */       sun = 1.0F; 
/*  979 */     sun = Config.limitTo1(sun);
/*  980 */     float sunX = sun * (width - 1);
/*  981 */     float torchX = Config.limitTo1(entityRenderer.e + 0.5F) * (width - 1);
/*  982 */     float gamma = Config.limitTo1((Config.getMinecraft()).A.O);
/*  983 */     boolean hasGamma = (gamma > 1.0E-4F);
/*      */     
/*  985 */     getLightMapColumn(lightMapRgb, sunX, 0, width, sunRgbs);
/*  986 */     getLightMapColumn(lightMapRgb, torchX, 16 * width, width, torchRgbs);
/*      */     
/*  988 */     float[] rgb = new float[3];
/*      */     
/*  990 */     for (int is = 0; is < 16; is++) {
/*      */       
/*  992 */       for (int it = 0; it < 16; it++) {
/*      */         
/*  994 */         for (int ic = 0; ic < 3; ic++) {
/*      */           
/*  996 */           float comp = Config.limitTo1(sunRgbs[is][ic] + torchRgbs[it][ic]);
/*  997 */           if (hasGamma) {
/*      */             
/*  999 */             float cg = 1.0F - comp;
/* 1000 */             cg = 1.0F - cg * cg * cg * cg;
/* 1001 */             comp = gamma * cg + (1.0F - gamma) * comp;
/*      */           } 
/* 1003 */           rgb[ic] = comp;
/*      */         } 
/* 1005 */         int r = (int)(rgb[0] * 255.0F);
/* 1006 */         int g = (int)(rgb[1] * 255.0F);
/* 1007 */         int b = (int)(rgb[2] * 255.0F);
/*      */         
/* 1009 */         lmColors[is * 16 + it] = 0xFF000000 | r << 16 | g << 8 | b;
/*      */       } 
/*      */     } 
/*      */     
/* 1013 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static void getLightMapColumn(float[][] origMap, float x, int offset, int width, float[][] colRgb) {
/* 1025 */     int xLow = (int)Math.floor(x);
/* 1026 */     int xHigh = (int)Math.ceil(x);
/* 1027 */     if (xLow == xHigh) {
/*      */ 
/*      */       
/* 1030 */       for (int i = 0; i < 16; i++) {
/*      */         
/* 1032 */         float[] rgbLow = origMap[offset + i * width + xLow];
/* 1033 */         float[] rgb = colRgb[i];
/* 1034 */         for (int j = 0; j < 3; j++)
/*      */         {
/* 1036 */           rgb[j] = rgbLow[j];
/*      */         }
/*      */       } 
/*      */       
/*      */       return;
/*      */     } 
/* 1042 */     float dLow = 1.0F - x - xLow;
/* 1043 */     float dHigh = 1.0F - xHigh - x;
/*      */     
/* 1045 */     for (int y = 0; y < 16; y++) {
/*      */       
/* 1047 */       float[] rgbLow = origMap[offset + y * width + xLow];
/* 1048 */       float[] rgbHigh = origMap[offset + y * width + xHigh];
/* 1049 */       float[] rgb = colRgb[y];
/*      */       
/* 1051 */       for (int i = 0; i < 3; i++)
/*      */       {
/* 1053 */         rgb[i] = rgbLow[i] * dLow + rgbHigh[i] * dHigh;
/*      */       }
/*      */     } 
/*      */   }
/*      */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\CustomColorizer.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */