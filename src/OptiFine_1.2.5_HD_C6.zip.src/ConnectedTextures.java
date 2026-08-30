/*     */ import java.awt.Dimension;
/*     */ import java.io.IOException;
/*     */ import java.io.InputStream;
/*     */ import java.util.ArrayList;
/*     */ import java.util.List;
/*     */ import java.util.Properties;
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
/*     */ public class ConnectedTextures
/*     */ {
/*  19 */   private static ConnectedProperties[][] blockProperties = (ConnectedProperties[][])null;
/*  20 */   private static ConnectedProperties[][] terrainProperties = (ConnectedProperties[][])null;
/*     */   
/*     */   private static boolean matchingCtmPng = false;
/*     */   
/*     */   private static final int BOTTOM = 0;
/*     */   private static final int TOP = 1;
/*     */   private static final int EAST = 2;
/*     */   private static final int WEST = 3;
/*     */   private static final int NORTH = 4;
/*     */   private static final int SOUTH = 5;
/*  30 */   private static final String[] propSuffixes = new String[] { "", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z" };
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static void update(aaw re) {
/*  39 */     blockProperties = (ConnectedProperties[][])null;
/*  40 */     terrainProperties = (ConnectedProperties[][])null;
/*  41 */     matchingCtmPng = false;
/*     */ 
/*     */ 
/*     */     
/*  45 */     blockProperties = readConnectedProperties("/ctm/block", 256, re, 1);
/*  46 */     terrainProperties = readConnectedProperties("/ctm/terrain", 256, re, 2);
/*     */     
/*  48 */     matchingCtmPng = getMatchingCtmPng(re);
/*  49 */     Config.dbg("MatchingCtmPng: " + matchingCtmPng);
/*     */ 
/*     */     
/*  52 */     if (blockProperties == null && terrainProperties == null && matchingCtmPng) {
/*     */       
/*  54 */       Config.dbg("Registering default ConnectedTextures");
/*     */       
/*  56 */       blockProperties = new ConnectedProperties[256][];
/*     */       
/*  58 */       blockProperties[pb.M.bO] = new ConnectedProperties[1];
/*  59 */       blockProperties[pb.M.bO][0] = makeDefaultProperties("ctm", re);
/*     */       
/*  61 */       blockProperties[pb.an.bO] = new ConnectedProperties[1];
/*  62 */       blockProperties[pb.an.bO][0] = makeDefaultProperties("horizontal", re);
/*     */       
/*  64 */       terrainProperties = new ConnectedProperties[256][];
/*     */       
/*  66 */       terrainProperties[pb.Q.bN] = new ConnectedProperties[1];
/*  67 */       terrainProperties[pb.Q.bN][0] = makeDefaultProperties("top", re);
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static ConnectedProperties[][] readConnectedProperties(String prefix, int num, aaw re, int defConnect) {
/*  76 */     ConnectedProperties[][] cps = (ConnectedProperties[][])null;
/*     */     
/*  78 */     for (int i = 0; i < num; i++) {
/*     */ 
/*     */       
/*  81 */       List<ConnectedProperties> listIndexProps = new ArrayList();
/*     */       
/*  83 */       for (int is = 0; is < propSuffixes.length; is++) {
/*     */         
/*  85 */         String suff = propSuffixes[is];
/*  86 */         String path = prefix + i + suff + ".properties";
/*  87 */         InputStream in = re.l.a.a(path);
/*  88 */         if (in == null) {
/*     */           break;
/*     */         }
/*     */ 
/*     */ 
/*     */ 
/*     */         
/*     */         try {
/*  96 */           Properties props = new Properties();
/*  97 */           props.load(in);
/*     */           
/*  99 */           Config.dbg("Connected texture: " + path);
/*     */           
/* 101 */           ConnectedProperties cp = new ConnectedProperties(props);
/*     */           
/* 103 */           if (cp.connect == 0) {
/* 104 */             cp.connect = defConnect;
/*     */           }
/* 106 */           if (cp.isValid(path)) {
/*     */ 
/*     */             
/* 109 */             cp.textureId = re.b(cp.source);
/*     */             
/* 111 */             listIndexProps.add(cp);
/*     */             
/* 113 */             in.close();
/*     */           } 
/* 115 */         } catch (IOException e) {
/*     */           
/* 117 */           e.printStackTrace();
/*     */         } 
/*     */       } 
/*     */       
/* 121 */       if (listIndexProps.size() > 0) {
/*     */ 
/*     */         
/* 124 */         if (cps == null) {
/* 125 */           cps = new ConnectedProperties[num][0];
/*     */         }
/* 127 */         cps[i] = listIndexProps.<ConnectedProperties>toArray(new ConnectedProperties[listIndexProps.size()]);
/*     */       } 
/*     */     } 
/*     */     
/* 131 */     return cps;
/*     */   }
/*     */ 
/*     */ 
/*     */   
/*     */   public static int getConnectedTexture(ali blockAccess, pb block, int x, int y, int z, int side, int tileNum) {
/* 137 */     if (blockAccess == null) {
/* 138 */       return -1;
/*     */     }
/* 140 */     if (terrainProperties != null && adz.a.defaultTexture)
/*     */     {
/* 142 */       if (tileNum >= 0 && tileNum < terrainProperties.length) {
/*     */         
/* 144 */         ConnectedProperties[] cps = terrainProperties[tileNum];
/* 145 */         if (cps != null) {
/*     */           
/* 147 */           int texNum = getConnectedTexture(cps, blockAccess, block, x, y, z, side, tileNum);
/* 148 */           if (texNum >= 0) {
/* 149 */             return texNum;
/*     */           }
/*     */         } 
/*     */       } 
/*     */     }
/* 154 */     if (blockProperties != null) {
/*     */       
/* 156 */       int blockId = block.bO;
/* 157 */       if (blockId >= 0 && blockId < blockProperties.length) {
/*     */         
/* 159 */         ConnectedProperties[] cps = blockProperties[blockId];
/* 160 */         if (cps != null) {
/*     */           
/* 162 */           int texNum = getConnectedTexture(cps, blockAccess, block, x, y, z, side, tileNum);
/* 163 */           if (texNum >= 0) {
/* 164 */             return texNum;
/*     */           }
/*     */         } 
/*     */       } 
/*     */     } 
/* 169 */     return -1;
/*     */   }
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
/*     */   private static int getConnectedTexture(ConnectedProperties[] cps, ali blockAccess, pb block, int x, int y, int z, int side, int tileNum) {
/* 183 */     for (int i = 0; i < cps.length; i++) {
/*     */       
/* 185 */       ConnectedProperties cp = cps[i];
/* 186 */       if (cp != null) {
/*     */         
/* 188 */         int texNum = getConnectedTexture(cp, blockAccess, block, x, y, z, side, tileNum);
/* 189 */         if (texNum >= 0)
/* 190 */           return texNum; 
/*     */       } 
/*     */     } 
/* 193 */     return -1;
/*     */   }
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
/*     */   private static int getConnectedTexture(ConnectedProperties cp, ali blockAccess, pb block, int x, int y, int z, int side, int tileNum) {
/* 208 */     if (side >= 0)
/*     */     {
/* 210 */       if ((1 << side & cp.faces) == 0) {
/* 211 */         return -1;
/*     */       }
/*     */     }
/* 214 */     if (cp.metadata != null) {
/*     */       
/* 216 */       int[] mds = cp.metadata;
/* 217 */       int md = blockAccess.e(x, y, z);
/* 218 */       boolean metadataFound = false;
/* 219 */       for (int i = 0; i < mds.length; i++) {
/*     */         
/* 221 */         if (mds[i] == md) {
/*     */           
/* 223 */           metadataFound = true;
/*     */           break;
/*     */         } 
/*     */       } 
/* 227 */       if (!metadataFound) {
/* 228 */         return -1;
/*     */       }
/*     */     } 
/* 231 */     switch (cp.method) {
/*     */       
/*     */       case 1:
/* 234 */         return getConnectedTextureCtm(cp, blockAccess, block, x, y, z, side, tileNum);
/*     */       case 2:
/* 236 */         return getConnectedTextureHorizontal(cp, blockAccess, block, x, y, z, side, tileNum);
/*     */       case 6:
/* 238 */         return getConnectedTextureVertical(cp, blockAccess, block, x, y, z, side, tileNum);
/*     */       case 3:
/* 240 */         return getConnectedTextureTop(cp, blockAccess, block, x, y, z, side, tileNum);
/*     */       case 4:
/* 242 */         return getConnectedTextureRandom(cp, x, y, z, side);
/*     */       case 5:
/* 244 */         return getConnectedTextureRepeat(cp, x, y, z, side);
/*     */     } 
/*     */     
/* 247 */     return -1;
/*     */   }
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
/*     */   private static int getConnectedTextureRandom(ConnectedProperties cp, int x, int y, int z, int side) {
/* 263 */     int face = side / cp.symmetry * cp.symmetry;
/*     */     
/* 265 */     int rand = Config.getRandom(x, y, z, side) & Integer.MAX_VALUE;
/*     */     
/* 267 */     int index = 0;
/*     */     
/* 269 */     if (cp.weights == null) {
/*     */ 
/*     */       
/* 272 */       index = rand % cp.tiles.length;
/*     */     
/*     */     }
/*     */     else {
/*     */       
/* 277 */       int randWeight = rand % cp.sumAllWeights;
/*     */       
/* 279 */       int[] sumWeights = cp.sumWeights;
/* 280 */       for (int i = 0; i < sumWeights.length; i++) {
/*     */         
/* 282 */         if (randWeight < sumWeights[i]) {
/*     */           
/* 284 */           index = i;
/*     */           
/*     */           break;
/*     */         } 
/*     */       } 
/*     */     } 
/* 290 */     return cp.textureId * 256 + cp.tiles[index];
/*     */   }
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
/*     */   private static int getConnectedTextureRepeat(ConnectedProperties cp, int x, int y, int z, int side) {
/* 305 */     int nx = 0;
/* 306 */     int ny = 0;
/* 307 */     switch (side) {
/*     */       
/*     */       case 0:
/* 310 */         nx = x;
/* 311 */         ny = z;
/*     */         break;
/*     */       case 1:
/* 314 */         nx = x;
/* 315 */         ny = z;
/*     */         break;
/*     */       case 2:
/* 318 */         nx = -x - 1;
/* 319 */         ny = -y;
/*     */         break;
/*     */       case 3:
/* 322 */         nx = x;
/* 323 */         ny = -y;
/*     */         break;
/*     */       case 4:
/* 326 */         nx = z;
/* 327 */         ny = -y;
/*     */         break;
/*     */       case 5:
/* 330 */         nx = -z - 1;
/* 331 */         ny = -y;
/*     */         break;
/*     */     } 
/*     */     
/* 335 */     nx %= cp.width;
/* 336 */     ny %= cp.height;
/*     */     
/* 338 */     if (nx < 0)
/* 339 */       nx += cp.width; 
/* 340 */     if (ny < 0) {
/* 341 */       ny += cp.height;
/*     */     }
/* 343 */     int index = ny * cp.width + nx;
/*     */     
/* 345 */     return cp.textureId * 256 + cp.tiles[index];
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static int getConnectedTextureCtm(ConnectedProperties cp, ali blockAccess, pb block, int x, int y, int z, int side, int tileNum) {
/* 356 */     boolean[] borders = new boolean[6];
/*     */ 
/*     */     
/* 359 */     int id = block.bO;
/* 360 */     switch (side) {
/*     */       
/*     */       case 0:
/*     */       case 1:
/* 364 */         borders[0] = isNeighbour(cp, blockAccess, x - 1, y, z, id, side, tileNum);
/* 365 */         borders[1] = isNeighbour(cp, blockAccess, x + 1, y, z, id, side, tileNum);
/* 366 */         borders[2] = isNeighbour(cp, blockAccess, x, y, z + 1, id, side, tileNum);
/* 367 */         borders[3] = isNeighbour(cp, blockAccess, x, y, z - 1, id, side, tileNum);
/*     */         break;
/*     */       case 2:
/* 370 */         borders[0] = isNeighbour(cp, blockAccess, x + 1, y, z, id, side, tileNum);
/* 371 */         borders[1] = isNeighbour(cp, blockAccess, x - 1, y, z, id, side, tileNum);
/* 372 */         borders[2] = isNeighbour(cp, blockAccess, x, y - 1, z, id, side, tileNum);
/* 373 */         borders[3] = isNeighbour(cp, blockAccess, x, y + 1, z, id, side, tileNum);
/*     */         break;
/*     */       case 3:
/* 376 */         borders[0] = isNeighbour(cp, blockAccess, x - 1, y, z, id, side, tileNum);
/* 377 */         borders[1] = isNeighbour(cp, blockAccess, x + 1, y, z, id, side, tileNum);
/* 378 */         borders[2] = isNeighbour(cp, blockAccess, x, y - 1, z, id, side, tileNum);
/* 379 */         borders[3] = isNeighbour(cp, blockAccess, x, y + 1, z, id, side, tileNum);
/*     */         break;
/*     */       case 4:
/* 382 */         borders[0] = isNeighbour(cp, blockAccess, x, y, z - 1, id, side, tileNum);
/* 383 */         borders[1] = isNeighbour(cp, blockAccess, x, y, z + 1, id, side, tileNum);
/* 384 */         borders[2] = isNeighbour(cp, blockAccess, x, y - 1, z, id, side, tileNum);
/* 385 */         borders[3] = isNeighbour(cp, blockAccess, x, y + 1, z, id, side, tileNum);
/*     */         break;
/*     */       case 5:
/* 388 */         borders[0] = isNeighbour(cp, blockAccess, x, y, z + 1, id, side, tileNum);
/* 389 */         borders[1] = isNeighbour(cp, blockAccess, x, y, z - 1, id, side, tileNum);
/* 390 */         borders[2] = isNeighbour(cp, blockAccess, x, y - 1, z, id, side, tileNum);
/* 391 */         borders[3] = isNeighbour(cp, blockAccess, x, y + 1, z, id, side, tileNum);
/*     */         break;
/*     */     } 
/*     */     
/* 395 */     int index = 0;
/*     */     
/* 397 */     if ((borders[0] & (!borders[1] ? 1 : 0) & (!borders[2] ? 1 : 0) & (!borders[3] ? 1 : 0)) != 0) {
/* 398 */       index = 3;
/* 399 */     } else if (((!borders[0] ? 1 : 0) & borders[1] & (!borders[2] ? 1 : 0) & (!borders[3] ? 1 : 0)) != 0) {
/* 400 */       index = 1;
/* 401 */     } else if (((!borders[0] ? 1 : 0) & (!borders[1] ? 1 : 0) & borders[2] & (!borders[3] ? 1 : 0)) != 0) {
/* 402 */       index = 12;
/* 403 */     } else if (((!borders[0] ? 1 : 0) & (!borders[1] ? 1 : 0) & (!borders[2] ? 1 : 0) & borders[3]) != 0) {
/* 404 */       index = 36;
/* 405 */     } else if ((borders[0] & borders[1] & (!borders[2] ? 1 : 0) & (!borders[3] ? 1 : 0)) != 0) {
/* 406 */       index = 2;
/* 407 */     } else if (((!borders[0] ? 1 : 0) & (!borders[1] ? 1 : 0) & borders[2] & borders[3]) != 0) {
/* 408 */       index = 24;
/* 409 */     } else if ((borders[0] & (!borders[1] ? 1 : 0) & borders[2] & (!borders[3] ? 1 : 0)) != 0) {
/* 410 */       index = 15;
/* 411 */     } else if ((borders[0] & (!borders[1] ? 1 : 0) & (!borders[2] ? 1 : 0) & borders[3]) != 0) {
/* 412 */       index = 39;
/* 413 */     } else if (((!borders[0] ? 1 : 0) & borders[1] & borders[2] & (!borders[3] ? 1 : 0)) != 0) {
/* 414 */       index = 13;
/* 415 */     } else if (((!borders[0] ? 1 : 0) & borders[1] & (!borders[2] ? 1 : 0) & borders[3]) != 0) {
/* 416 */       index = 37;
/* 417 */     } else if (((!borders[0] ? 1 : 0) & borders[1] & borders[2] & borders[3]) != 0) {
/* 418 */       index = 25;
/* 419 */     } else if ((borders[0] & (!borders[1] ? 1 : 0) & borders[2] & borders[3]) != 0) {
/* 420 */       index = 27;
/* 421 */     } else if ((borders[0] & borders[1] & (!borders[2] ? 1 : 0) & borders[3]) != 0) {
/* 422 */       index = 38;
/* 423 */     } else if ((borders[0] & borders[1] & borders[2] & (!borders[3] ? 1 : 0)) != 0) {
/* 424 */       index = 14;
/* 425 */     } else if ((borders[0] & borders[1] & borders[2] & borders[3]) != 0) {
/* 426 */       index = 26;
/*     */     } 
/* 428 */     if (!Config.isConnectedTexturesFancy()) {
/* 429 */       return cp.textureId * 256 + cp.tiles[index];
/*     */     }
/* 431 */     boolean[] edges = new boolean[6];
/* 432 */     switch (side) {
/*     */       
/*     */       case 0:
/*     */       case 1:
/* 436 */         edges[0] = !isNeighbour(cp, blockAccess, x + 1, y, z + 1, id, side, tileNum);
/* 437 */         edges[1] = !isNeighbour(cp, blockAccess, x - 1, y, z + 1, id, side, tileNum);
/* 438 */         edges[2] = !isNeighbour(cp, blockAccess, x + 1, y, z - 1, id, side, tileNum);
/* 439 */         edges[3] = !isNeighbour(cp, blockAccess, x - 1, y, z - 1, id, side, tileNum);
/*     */         break;
/*     */       case 2:
/* 442 */         edges[0] = !isNeighbour(cp, blockAccess, x - 1, y - 1, z, id, side, tileNum);
/* 443 */         edges[1] = !isNeighbour(cp, blockAccess, x + 1, y - 1, z, id, side, tileNum);
/* 444 */         edges[2] = !isNeighbour(cp, blockAccess, x - 1, y + 1, z, id, side, tileNum);
/* 445 */         edges[3] = !isNeighbour(cp, blockAccess, x + 1, y + 1, z, id, side, tileNum);
/*     */         break;
/*     */       case 3:
/* 448 */         edges[0] = !isNeighbour(cp, blockAccess, x + 1, y - 1, z, id, side, tileNum);
/* 449 */         edges[1] = !isNeighbour(cp, blockAccess, x - 1, y - 1, z, id, side, tileNum);
/* 450 */         edges[2] = !isNeighbour(cp, blockAccess, x + 1, y + 1, z, id, side, tileNum);
/* 451 */         edges[3] = !isNeighbour(cp, blockAccess, x - 1, y + 1, z, id, side, tileNum);
/*     */         break;
/*     */       case 4:
/* 454 */         edges[0] = !isNeighbour(cp, blockAccess, x, y - 1, z + 1, id, side, tileNum);
/* 455 */         edges[1] = !isNeighbour(cp, blockAccess, x, y - 1, z - 1, id, side, tileNum);
/* 456 */         edges[2] = !isNeighbour(cp, blockAccess, x, y + 1, z + 1, id, side, tileNum);
/* 457 */         edges[3] = !isNeighbour(cp, blockAccess, x, y + 1, z - 1, id, side, tileNum);
/*     */         break;
/*     */       case 5:
/* 460 */         edges[0] = !isNeighbour(cp, blockAccess, x, y - 1, z - 1, id, side, tileNum);
/* 461 */         edges[1] = !isNeighbour(cp, blockAccess, x, y - 1, z + 1, id, side, tileNum);
/* 462 */         edges[2] = !isNeighbour(cp, blockAccess, x, y + 1, z - 1, id, side, tileNum);
/* 463 */         edges[3] = !isNeighbour(cp, blockAccess, x, y + 1, z + 1, id, side, tileNum);
/*     */         break;
/*     */     } 
/* 466 */     if (index == 13 && edges[0])
/* 467 */       index = 4; 
/* 468 */     if (index == 15 && edges[1])
/* 469 */       index = 5; 
/* 470 */     if (index == 37 && edges[2])
/* 471 */       index = 16; 
/* 472 */     if (index == 39 && edges[3]) {
/* 473 */       index = 17;
/*     */     }
/* 475 */     if (index == 14 && edges[0] && edges[1])
/* 476 */       index = 7; 
/* 477 */     if (index == 25 && edges[0] && edges[2])
/* 478 */       index = 6; 
/* 479 */     if (index == 27 && edges[3] && edges[1])
/* 480 */       index = 19; 
/* 481 */     if (index == 38 && edges[3] && edges[2]) {
/* 482 */       index = 18;
/*     */     }
/* 484 */     if (index == 14 && !edges[0] && edges[1])
/* 485 */       index = 31; 
/* 486 */     if (index == 25 && edges[0] && !edges[2])
/* 487 */       index = 30; 
/* 488 */     if (index == 27 && !edges[3] && edges[1])
/* 489 */       index = 41; 
/* 490 */     if (index == 38 && edges[3] && !edges[2]) {
/* 491 */       index = 40;
/*     */     }
/* 493 */     if (index == 14 && edges[0] && !edges[1])
/* 494 */       index = 29; 
/* 495 */     if (index == 25 && !edges[0] && edges[2])
/* 496 */       index = 28; 
/* 497 */     if (index == 27 && edges[3] && !edges[1])
/* 498 */       index = 43; 
/* 499 */     if (index == 38 && !edges[3] && edges[2]) {
/* 500 */       index = 42;
/*     */     }
/* 502 */     if (index == 26 && edges[0] && edges[1] && edges[2] && edges[3]) {
/* 503 */       index = 46;
/*     */     }
/* 505 */     if (index == 26 && !edges[0] && edges[1] && edges[2] && edges[3])
/* 506 */       index = 9; 
/* 507 */     if (index == 26 && edges[0] && !edges[1] && edges[2] && edges[3])
/* 508 */       index = 21; 
/* 509 */     if (index == 26 && edges[0] && edges[1] && !edges[2] && edges[3])
/* 510 */       index = 8; 
/* 511 */     if (index == 26 && edges[0] && edges[1] && edges[2] && !edges[3]) {
/* 512 */       index = 20;
/*     */     }
/* 514 */     if (index == 26 && edges[0] && edges[1] && !edges[2] && !edges[3])
/* 515 */       index = 11; 
/* 516 */     if (index == 26 && !edges[0] && !edges[1] && edges[2] && edges[3])
/* 517 */       index = 22; 
/* 518 */     if (index == 26 && !edges[0] && edges[1] && !edges[2] && edges[3])
/* 519 */       index = 23; 
/* 520 */     if (index == 26 && edges[0] && !edges[1] && edges[2] && !edges[3]) {
/* 521 */       index = 10;
/*     */     }
/* 523 */     if (index == 26 && edges[0] && !edges[1] && !edges[2] && edges[3])
/* 524 */       index = 34; 
/* 525 */     if (index == 26 && !edges[0] && edges[1] && edges[2] && !edges[3]) {
/* 526 */       index = 35;
/*     */     }
/* 528 */     if (index == 26 && edges[0] && !edges[1] && !edges[2] && !edges[3])
/* 529 */       index = 32; 
/* 530 */     if (index == 26 && !edges[0] && edges[1] && !edges[2] && !edges[3])
/* 531 */       index = 33; 
/* 532 */     if (index == 26 && !edges[0] && !edges[1] && edges[2] && !edges[3])
/* 533 */       index = 44; 
/* 534 */     if (index == 26 && !edges[0] && !edges[1] && !edges[2] && edges[3]) {
/* 535 */       index = 45;
/*     */     }
/* 537 */     return cp.textureId * 256 + cp.tiles[index];
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static boolean isNeighbour(ConnectedProperties cp, ali iblockaccess, int x, int y, int z, int id, int side, int tileNum) {
/* 544 */     int blockId = iblockaccess.a(x, y, z);
/*     */     
/* 546 */     if (cp.connect == 2) {
/*     */       
/* 548 */       pb neighbourBlock = pb.m[blockId];
/* 549 */       if (neighbourBlock == null) {
/* 550 */         return false;
/*     */       }
/* 552 */       int neighbourTileNum = neighbourBlock.d(iblockaccess, x, y, z, side);
/* 553 */       return (neighbourTileNum == tileNum);
/*     */     } 
/*     */     
/* 556 */     return (blockId == id);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static int getConnectedTextureHorizontal(ConnectedProperties cp, ali blockAccess, pb block, int x, int y, int z, int side, int tileNum) {
/* 564 */     if (side == 0 || side == 1) {
/* 565 */       return -1;
/*     */     }
/* 567 */     boolean left = false;
/* 568 */     boolean right = false;
/*     */ 
/*     */     
/* 571 */     int id = block.bO;
/*     */     
/* 573 */     switch (side) {
/*     */       
/*     */       case 2:
/* 576 */         left = isNeighbour(cp, blockAccess, x + 1, y, z, id, side, tileNum);
/* 577 */         right = isNeighbour(cp, blockAccess, x - 1, y, z, id, side, tileNum);
/*     */         break;
/*     */       case 3:
/* 580 */         left = isNeighbour(cp, blockAccess, x - 1, y, z, id, side, tileNum);
/* 581 */         right = isNeighbour(cp, blockAccess, x + 1, y, z, id, side, tileNum);
/*     */         break;
/*     */       case 4:
/* 584 */         left = isNeighbour(cp, blockAccess, x, y, z - 1, id, side, tileNum);
/* 585 */         right = isNeighbour(cp, blockAccess, x, y, z + 1, id, side, tileNum);
/*     */         break;
/*     */       case 5:
/* 588 */         left = isNeighbour(cp, blockAccess, x, y, z + 1, id, side, tileNum);
/* 589 */         right = isNeighbour(cp, blockAccess, x, y, z - 1, id, side, tileNum);
/*     */         break;
/*     */     } 
/*     */     
/* 593 */     int index = 3;
/*     */     
/* 595 */     if (left) {
/*     */       
/* 597 */       if (right)
/*     */       {
/*     */         
/* 600 */         index = 1;
/*     */       
/*     */       }
/*     */       else
/*     */       {
/* 605 */         index = 2;
/*     */       
/*     */       }
/*     */     
/*     */     }
/* 610 */     else if (right) {
/*     */ 
/*     */       
/* 613 */       index = 0;
/*     */     
/*     */     }
/*     */     else {
/*     */       
/* 618 */       index = 3;
/*     */     } 
/*     */ 
/*     */     
/* 622 */     return cp.textureId * 256 + cp.tiles[index];
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static int getConnectedTextureVertical(ConnectedProperties cp, ali blockAccess, pb block, int x, int y, int z, int side, int tileNum) {
/* 630 */     if (side == 0 || side == 1) {
/* 631 */       return -1;
/*     */     }
/* 633 */     int id = block.bO;
/* 634 */     boolean bottom = isNeighbour(cp, blockAccess, x, y - 1, z, id, side, tileNum);
/* 635 */     boolean top = isNeighbour(cp, blockAccess, x, y + 1, z, id, side, tileNum);
/*     */ 
/*     */     
/* 638 */     int index = 3;
/*     */     
/* 640 */     if (bottom) {
/*     */       
/* 642 */       if (top)
/*     */       {
/*     */         
/* 645 */         index = 1;
/*     */       
/*     */       }
/*     */       else
/*     */       {
/* 650 */         index = 2;
/*     */       
/*     */       }
/*     */     
/*     */     }
/* 655 */     else if (top) {
/*     */ 
/*     */       
/* 658 */       index = 0;
/*     */     
/*     */     }
/*     */     else {
/*     */       
/* 663 */       index = 3;
/*     */     } 
/*     */ 
/*     */     
/* 667 */     return cp.textureId * 256 + cp.tiles[index];
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static int getConnectedTextureTop(ConnectedProperties cp, ali blockAccess, pb block, int x, int y, int z, int side, int tileNum) {
/* 675 */     if (side == 0 || side == 1) {
/* 676 */       return -1;
/*     */     }
/*     */     
/* 679 */     int blockId = block.bO;
/*     */     
/* 681 */     if (isNeighbour(cp, blockAccess, x, y + 1, z, blockId, side, tileNum))
/*     */     {
/*     */       
/* 684 */       return cp.textureId * 256 + cp.tiles[0];
/*     */     }
/*     */     
/* 687 */     return -1;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static boolean isConnectedGlassPanes() {
/* 695 */     return (Config.isConnectedTextures() && matchingCtmPng);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static boolean getMatchingCtmPng(aaw re) {
/* 704 */     Dimension dimCtm = re.getTextureDimensions(re.b("/ctm.png"));
/* 705 */     if (dimCtm == null)
/* 706 */       return false; 
/* 707 */     Dimension dimTerrain = re.getTextureDimensions(re.b("/terrain.png"));
/* 708 */     if (dimTerrain == null) {
/* 709 */       return false;
/*     */     }
/* 711 */     return (dimCtm.width == dimTerrain.width && dimCtm.height == dimTerrain.height);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static ConnectedProperties makeDefaultProperties(String methodStr, aaw re) {
/* 718 */     Properties props = new Properties();
/* 719 */     props.put("source", "/ctm.png");
/* 720 */     props.put("method", methodStr);
/* 721 */     ConnectedProperties cp = new ConnectedProperties(props);
/* 722 */     cp.isValid("(default)");
/*     */     
/* 724 */     cp.textureId = re.b(cp.source);
/*     */     
/* 726 */     return cp;
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\ConnectedTextures.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */