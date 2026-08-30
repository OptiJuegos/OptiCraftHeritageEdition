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
/*     */ public class ConnectedProperties
/*     */ {
/*  16 */   public int method = 0;
/*  17 */   public String source = null;
/*  18 */   public int[] tiles = null;
/*  19 */   public int connect = 0;
/*  20 */   public int faces = 63;
/*  21 */   public int[] metadata = null;
/*  22 */   public int[] weights = null;
/*  23 */   public int symmetry = 1;
/*  24 */   public int width = 0;
/*  25 */   public int height = 0;
/*     */   
/*  27 */   public int[] sumWeights = null;
/*  28 */   public int sumAllWeights = 0;
/*     */   
/*  30 */   public int textureId = -1;
/*     */   
/*     */   public static final int METHOD_NONE = 0;
/*     */   
/*     */   public static final int METHOD_CTM = 1;
/*     */   
/*     */   public static final int METHOD_HORIZONTAL = 2;
/*     */   
/*     */   public static final int METHOD_TOP = 3;
/*     */   
/*     */   public static final int METHOD_RANDOM = 4;
/*     */   
/*     */   public static final int METHOD_REPEAT = 5;
/*     */   
/*     */   public static final int METHOD_VERTICAL = 6;
/*     */   
/*     */   public static final int CONNECT_NONE = 0;
/*     */   public static final int CONNECT_BLOCK = 1;
/*     */   public static final int CONNECT_TILE = 2;
/*     */   public static final int FACE_NONE = 0;
/*     */   public static final int FACE_BOTTOM = 1;
/*     */   public static final int FACE_TOP = 2;
/*     */   public static final int FACE_EAST = 4;
/*     */   public static final int FACE_WEST = 8;
/*     */   public static final int FACE_NORTH = 16;
/*     */   public static final int FACE_SOUTH = 32;
/*     */   public static final int FACE_SIDES = 60;
/*     */   public static final int FACE_ALL = 63;
/*     */   public static final int SYMMETRY_NONE = 1;
/*     */   public static final int SYMMETRY_OPPOSITE = 2;
/*     */   public static final int SYMMETRY_ALL = 6;
/*     */   
/*     */   public ConnectedProperties(Properties props) {
/*  63 */     this.method = parseMethod(props.getProperty("method"));
/*  64 */     this.source = props.getProperty("source");
/*  65 */     this.tiles = parseInts(props.getProperty("tiles"));
/*  66 */     this.connect = parseConnect(props.getProperty("connect"));
/*  67 */     this.faces = parseFaces(props.getProperty("faces"));
/*  68 */     this.metadata = parseInts(props.getProperty("metadata"));
/*  69 */     this.weights = parseInts(props.getProperty("weights"));
/*  70 */     this.symmetry = parseSymmetry(props.getProperty("symmetry"));
/*  71 */     this.width = parseInt(props.getProperty("width"));
/*  72 */     this.height = parseInt(props.getProperty("height"));
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private int parseInt(String str) {
/*  80 */     if (str == null) {
/*  81 */       return -1;
/*     */     }
/*  83 */     int num = Config.parseInt(str, -1);
/*  84 */     if (num < 0) {
/*  85 */       Config.dbg("Invalid number: " + str);
/*     */     }
/*  87 */     return num;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private int parseSymmetry(String str) {
/*  95 */     if (str == null) {
/*  96 */       return 1;
/*     */     }
/*  98 */     if (str.equals("opposite"))
/*  99 */       return 2; 
/* 100 */     if (str.equals("all")) {
/* 101 */       return 6;
/*     */     }
/* 103 */     Config.dbg("Unknown symmetry: " + str);
/*     */     
/* 105 */     return 1;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private int parseFaces(String str) {
/* 114 */     if (str == null) {
/* 115 */       return 63;
/*     */     }
/* 117 */     String[] faceStrs = Config.tokenize(str, " ,");
/* 118 */     int facesMask = 0;
/* 119 */     for (int i = 0; i < faceStrs.length; i++) {
/*     */       
/* 121 */       String faceStr = faceStrs[i];
/* 122 */       int faceMask = parseFace(faceStr);
/*     */       
/* 124 */       facesMask |= faceMask;
/*     */     } 
/* 126 */     return facesMask;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private int parseFace(String str) {
/* 134 */     if (str.equals("bottom"))
/* 135 */       return 1; 
/* 136 */     if (str.equals("top")) {
/* 137 */       return 2;
/*     */     }
/*     */     
/* 140 */     if (str.equals("north")) {
/* 141 */       return 4;
/*     */     }
/* 143 */     if (str.equals("south")) {
/* 144 */       return 8;
/*     */     }
/* 146 */     if (str.equals("east")) {
/* 147 */       return 32;
/*     */     }
/* 149 */     if (str.equals("west")) {
/* 150 */       return 16;
/*     */     }
/* 152 */     if (str.equals("sides"))
/* 153 */       return 60; 
/* 154 */     if (str.equals("all")) {
/* 155 */       return 63;
/*     */     }
/* 157 */     Config.dbg("Unknown face: " + str);
/*     */     
/* 159 */     return 0;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private int parseConnect(String str) {
/* 167 */     if (str == null) {
/* 168 */       return 0;
/*     */     }
/* 170 */     if (str.equals("block"))
/* 171 */       return 1; 
/* 172 */     if (str.equals("tile")) {
/* 173 */       return 2;
/*     */     }
/* 175 */     Config.dbg("Unknown connect: " + str);
/*     */     
/* 177 */     return 0;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private int[] parseInts(String str) {
/* 185 */     if (str == null) {
/* 186 */       return null;
/*     */     }
/* 188 */     List<Integer> list = new ArrayList();
/* 189 */     String[] intStrs = Config.tokenize(str, " ,");
/* 190 */     for (int i = 0; i < intStrs.length; i++) {
/*     */       
/* 192 */       String intStr = intStrs[i];
/*     */       
/* 194 */       if (intStr.contains("-")) {
/*     */         
/* 196 */         String[] subStrs = Config.tokenize(intStr, "-");
/* 197 */         if (subStrs.length != 2) {
/*     */           
/* 199 */           Config.dbg("Invalid interval: " + intStr + ", when parsing: " + str);
/*     */         } else {
/*     */           
/* 202 */           int min = Config.parseInt(subStrs[0], -1);
/* 203 */           int max = Config.parseInt(subStrs[1], -1);
/* 204 */           if (min < 0 || max < 0 || min > max) {
/*     */             
/* 206 */             Config.dbg("Invalid interval: " + intStr + ", when parsing: " + str);
/*     */           }
/*     */           else {
/*     */             
/* 210 */             for (int n = min; n <= max; n++)
/*     */             {
/* 212 */               list.add(Integer.valueOf(n));
/*     */             }
/*     */           } 
/*     */         } 
/*     */       } else {
/*     */         
/* 218 */         int val = Config.parseInt(intStr, -1);
/* 219 */         if (val < 0) {
/*     */           
/* 221 */           Config.dbg("Invalid number: " + intStr + ", when parsing: " + str);
/*     */         }
/*     */         else {
/*     */           
/* 225 */           list.add(Integer.valueOf(val));
/*     */         } 
/*     */       } 
/* 228 */     }  int[] ints = new int[list.size()];
/* 229 */     for (int j = 0; j < ints.length; j++)
/*     */     {
/* 231 */       ints[j] = ((Integer)list.get(j)).intValue();
/*     */     }
/*     */     
/* 234 */     return ints;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private int parseMethod(String str) {
/* 242 */     if (str == null) {
/* 243 */       return 1;
/*     */     }
/* 245 */     if (str.equals("ctm"))
/* 246 */       return 1; 
/* 247 */     if (str.equals("horizontal"))
/* 248 */       return 2; 
/* 249 */     if (str.equals("vertical"))
/* 250 */       return 6; 
/* 251 */     if (str.equals("top"))
/* 252 */       return 3; 
/* 253 */     if (str.equals("random"))
/* 254 */       return 4; 
/* 255 */     if (str.equals("repeat")) {
/* 256 */       return 5;
/*     */     }
/* 258 */     Config.dbg("Unknown method: " + str);
/*     */     
/* 260 */     return 0;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public boolean isValid(String path) {
/* 268 */     if (this.source == null) {
/*     */       
/* 270 */       Config.dbg("No source texture: " + path);
/* 271 */       return false;
/*     */     } 
/*     */     
/* 274 */     if (this.method == 0) {
/*     */       
/* 276 */       Config.dbg("No method: " + path);
/* 277 */       return false;
/*     */     } 
/*     */     
/* 280 */     if (this.tiles != null)
/*     */     {
/* 282 */       for (int i = 0; i < this.tiles.length; i++) {
/*     */         
/* 284 */         int tileNum = this.tiles[i];
/* 285 */         if (tileNum < 0 || tileNum > 255) {
/*     */           
/* 287 */           Config.dbg("Invalid tile: " + tileNum + ", in " + path);
/* 288 */           return false;
/*     */         } 
/*     */       } 
/*     */     }
/*     */     
/* 293 */     switch (this.method) {
/*     */       
/*     */       case 1:
/* 296 */         return isValidCtm(path);
/*     */       case 2:
/* 298 */         return isValidHorizontal(path);
/*     */       case 6:
/* 300 */         return isValidVertical(path);
/*     */       case 3:
/* 302 */         return isValidTop(path);
/*     */       case 4:
/* 304 */         return isValidRandom(path);
/*     */       case 5:
/* 306 */         return isValidRepeat(path);
/*     */     } 
/*     */     
/* 309 */     Config.dbg("Unknown method: " + path);
/* 310 */     return false;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private boolean isValidCtm(String path) {
/* 319 */     if (this.tiles == null) {
/* 320 */       this.tiles = parseInts("0-11 16-27 32-43 48-59");
/*     */     }
/* 322 */     if (this.tiles.length != 48) {
/*     */       
/* 324 */       Config.dbg("Invalid tiles, must be exactly 48: " + path);
/* 325 */       return false;
/*     */     } 
/*     */     
/* 328 */     return true;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private boolean isValidHorizontal(String path) {
/* 337 */     if (this.tiles == null) {
/* 338 */       this.tiles = parseInts("12-15");
/*     */     }
/* 340 */     if (this.tiles.length != 4) {
/*     */       
/* 342 */       Config.dbg("Invalid tiles, must be exactly 4: " + path);
/* 343 */       return false;
/*     */     } 
/*     */     
/* 346 */     return true;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private boolean isValidVertical(String path) {
/* 356 */     if (this.tiles == null) {
/*     */       
/* 358 */       Config.dbg("No tiles defined for vertical: " + path);
/* 359 */       return false;
/*     */     } 
/*     */     
/* 362 */     if (this.tiles.length != 4) {
/*     */       
/* 364 */       Config.dbg("Invalid tiles, must be exactly 4: " + path);
/* 365 */       return false;
/*     */     } 
/*     */     
/* 368 */     return true;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private boolean isValidRandom(String path) {
/* 377 */     if (this.tiles == null) {
/*     */       
/* 379 */       Config.dbg("Tiles not defined: " + path);
/* 380 */       return false;
/*     */     } 
/*     */     
/* 383 */     if (this.weights != null && this.weights.length != this.tiles.length) {
/*     */       
/* 385 */       Config.dbg("Number of weights must equal the number of tiles: " + path);
/* 386 */       return false;
/*     */     } 
/*     */     
/* 389 */     if (this.weights != null) {
/*     */       
/* 391 */       this.sumWeights = new int[this.weights.length];
/* 392 */       int sum = 0;
/* 393 */       for (int i = 0; i < this.weights.length; i++) {
/*     */         
/* 395 */         sum += this.weights[i];
/* 396 */         this.sumWeights[i] = sum;
/*     */       } 
/* 398 */       this.sumAllWeights = sum;
/*     */     } 
/*     */ 
/*     */     
/* 402 */     return true;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private boolean isValidRepeat(String path) {
/* 411 */     if (this.tiles == null) {
/*     */       
/* 413 */       Config.dbg("Tiles not defined: " + path);
/* 414 */       return false;
/*     */     } 
/*     */     
/* 417 */     if (this.width < 0 || this.width > 16) {
/*     */       
/* 419 */       Config.dbg("Invalid width: " + path);
/* 420 */       return false;
/*     */     } 
/*     */     
/* 423 */     if (this.height < 0 || this.height > 16) {
/*     */       
/* 425 */       Config.dbg("Invalid height: " + path);
/* 426 */       return false;
/*     */     } 
/*     */     
/* 429 */     if (this.tiles.length != this.width * this.height) {
/*     */       
/* 431 */       Config.dbg("Number of tiles does not equal width x height: " + path);
/* 432 */       return false;
/*     */     } 
/*     */     
/* 435 */     return true;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private boolean isValidTop(String path) {
/* 444 */     if (this.tiles == null) {
/* 445 */       this.tiles = parseInts("66");
/*     */     }
/* 447 */     if (this.tiles.length != 1) {
/*     */       
/* 449 */       Config.dbg("Invalid tiles, must be exactly 1: " + path);
/* 450 */       return false;
/*     */     } 
/*     */     
/* 453 */     return true;
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\ConnectedProperties.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */