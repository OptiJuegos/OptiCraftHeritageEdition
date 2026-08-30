/*     */ import java.nio.ByteBuffer;
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
/*     */ public class CustomAnimation
/*     */ {
/*  15 */   private String imagePath = null;
/*  16 */   public byte[] imageBytes = null;
/*  17 */   public int frameWidth = 0;
/*  18 */   public int frameHeight = 0;
/*  19 */   public CustomAnimationFrame[] frames = null;
/*  20 */   public int activeFrame = 0;
/*     */   
/*  22 */   public String destTexture = null;
/*  23 */   public int destX = 0;
/*  24 */   public int destY = 0;
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public CustomAnimation(String imagePath, byte[] data, int frameWidth, int frameHeight, Properties props, int durDef) {
/*  31 */     this.imagePath = imagePath;
/*  32 */     this.imageBytes = data;
/*  33 */     this.frameWidth = frameWidth;
/*  34 */     this.frameHeight = frameHeight;
/*     */     
/*  36 */     int frameLen = frameWidth * frameHeight * 4;
/*     */     
/*  38 */     if (data.length % frameLen != 0) {
/*  39 */       Config.dbg("Invalid animated texture length: " + data.length + ", frameWidth: " + frameHeight + ", frameHeight: " + frameHeight);
/*     */     }
/*  41 */     int numFrames = data.length / frameLen;
/*  42 */     if (props.get("tile.0") != null)
/*     */     {
/*  44 */       for (int j = 0; props.get("tile." + j) != null; j++)
/*     */       {
/*  46 */         numFrames = j + 1;
/*     */       }
/*     */     }
/*     */     
/*  50 */     String durationDefStr = (String)props.get("duration");
/*  51 */     int durationDef = Config.parseInt(durationDefStr, durDef);
/*     */     
/*  53 */     this.frames = new CustomAnimationFrame[numFrames];
/*  54 */     for (int i = 0; i < this.frames.length; i++) {
/*     */       
/*  56 */       String indexStr = (String)props.get("tile." + i);
/*  57 */       int index = Config.parseInt(indexStr, i);
/*  58 */       String durationStr = (String)props.get("duration." + i);
/*  59 */       int duration = Config.parseInt(durationStr, durationDef);
/*  60 */       CustomAnimationFrame frm = new CustomAnimationFrame(index, duration);
/*  61 */       this.frames[i] = frm;
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public boolean nextFrame() {
/*  70 */     if (this.frames.length <= 0) {
/*  71 */       return false;
/*     */     }
/*  73 */     if (this.activeFrame >= this.frames.length) {
/*  74 */       this.activeFrame = 0;
/*     */     }
/*  76 */     CustomAnimationFrame frame = this.frames[this.activeFrame];
/*     */     
/*  78 */     frame.counter++;
/*     */     
/*  80 */     if (frame.counter < frame.duration) {
/*  81 */       return false;
/*     */     }
/*  83 */     frame.counter = 0;
/*     */     
/*  85 */     this.activeFrame++;
/*     */     
/*  87 */     if (this.activeFrame >= this.frames.length) {
/*  88 */       this.activeFrame = 0;
/*     */     }
/*  90 */     return true;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public int getActiveFrameIndex() {
/*  98 */     if (this.frames.length <= 0) {
/*  99 */       return 0;
/*     */     }
/* 101 */     if (this.activeFrame >= this.frames.length) {
/* 102 */       this.activeFrame = 0;
/*     */     }
/* 104 */     CustomAnimationFrame frame = this.frames[this.activeFrame];
/*     */     
/* 106 */     return frame.index;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public int getFrameCount() {
/* 113 */     return this.frames.length;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public boolean updateCustomTexture(ByteBuffer imgData, boolean animated, boolean dynamicTexturesUpdated, StringBuffer dataIdBuf) {
/* 121 */     if (this.imageBytes == null) {
/* 122 */       return false;
/*     */     }
/* 124 */     if (!animated && dynamicTexturesUpdated)
/*     */     {
/*     */       
/* 127 */       return true;
/*     */     }
/*     */     
/* 130 */     if (!nextFrame())
/*     */     {
/*     */       
/* 133 */       return true;
/*     */     }
/*     */     
/* 136 */     int imgLen = this.frameWidth * this.frameHeight * 4;
/*     */     
/* 138 */     if (this.imageBytes.length < imgLen) {
/* 139 */       return false;
/*     */     }
/* 141 */     int imgCount = getFrameCount();
/* 142 */     int imgNum = getActiveFrameIndex();
/* 143 */     int offset = 0;
/*     */     
/* 145 */     if (animated) {
/* 146 */       offset = imgLen * imgNum;
/*     */     }
/* 148 */     imgData.clear();
/* 149 */     imgData.put(this.imageBytes, offset, imgLen);
/* 150 */     imgData.position(0).limit(imgLen);
/*     */     
/* 152 */     dataIdBuf.append(this.imagePath);
/* 153 */     dataIdBuf.append(":");
/* 154 */     dataIdBuf.append(imgNum);
/*     */     
/* 156 */     return true;
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\CustomAnimation.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */