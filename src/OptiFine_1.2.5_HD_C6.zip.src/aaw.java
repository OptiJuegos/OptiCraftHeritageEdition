/*      */ import java.awt.Color;
/*      */ import java.awt.Dimension;
/*      */ import java.awt.Graphics;
/*      */ import java.awt.Graphics2D;
/*      */ import java.awt.RenderingHints;
/*      */ import java.awt.image.BufferedImage;
/*      */ import java.io.File;
/*      */ import java.io.FileInputStream;
/*      */ import java.io.IOException;
/*      */ import java.io.InputStream;
/*      */ import java.nio.ByteBuffer;
/*      */ import java.nio.IntBuffer;
/*      */ import java.util.ArrayList;
/*      */ import java.util.Enumeration;
/*      */ import java.util.HashMap;
/*      */ import java.util.IdentityHashMap;
/*      */ import java.util.Iterator;
/*      */ import java.util.List;
/*      */ import java.util.Map;
/*      */ import java.util.Properties;
/*      */ import java.util.logging.Logger;
/*      */ import java.util.zip.ZipEntry;
/*      */ import java.util.zip.ZipFile;
/*      */ import javax.imageio.ImageIO;
/*      */ import net.minecraft.client.Minecraft;
/*      */ import org.lwjgl.opengl.GL11;
/*      */ import org.lwjgl.opengl.GLContext;
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
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ public class aaw
/*      */ {
/*      */   public static boolean a = true;
/*      */   private HashMap d;
/*      */   private HashMap e;
/*      */   private abh f;
/*      */   private IntBuffer g;
/*      */   private ByteBuffer h;
/*      */   public List i;
/*      */   private Map j;
/*      */   private hu k;
/*      */   public boolean b;
/*      */   public boolean c;
/*      */   public gi l;
/*      */   private BufferedImage m;
/*      */   private int n;
/*   78 */   public int terrainTextureId = -1;
/*   79 */   public int guiItemsTextureId = -1;
/*   80 */   public int ctmTextureId = -1;
/*      */   
/*      */   private boolean hdTexturesInstalled = false;
/*      */   
/*   84 */   private Map textureDimensionsMap = new HashMap<Object, Object>();
/*      */   
/*   86 */   private Map textureDataMap = new HashMap<Object, Object>();
/*      */   
/*   88 */   private int tickCounter = 0;
/*      */   
/*      */   private ByteBuffer[] mipImageDatas;
/*      */   
/*      */   private boolean dynamicTexturesUpdated = false;
/*      */   
/*   94 */   private Map textureFxMap = new IdentityHashMap<Object, Object>();
/*      */   
/*   96 */   private Map mipDataBufsMap = new HashMap<Object, Object>();
/*      */   
/*   98 */   private Map customAnimationMap = new HashMap<Object, Object>();
/*      */   
/*  100 */   private CustomAnimation[] textureAnimations = null;
/*      */   
/*  102 */   public static Logger log = Logger.getAnonymousLogger();
/*      */ 
/*      */ 
/*      */   
/*      */   public aaw(gi par1TexturePackList, hu par2GameSettings) {
/*  107 */     this.d = new HashMap<Object, Object>();
/*  108 */     this.e = new HashMap<Object, Object>();
/*  109 */     this.f = new abh();
/*  110 */     this.g = ew.d(1);
/*      */     
/*  112 */     allocateImageData(256, 256);
/*      */     
/*  114 */     this.i = new ArrayList();
/*  115 */     this.j = new HashMap<Object, Object>();
/*  116 */     this.b = false;
/*  117 */     this.c = false;
/*  118 */     this.m = new BufferedImage(64, 64, 2);
/*  119 */     this.n = 16;
/*  120 */     this.l = par1TexturePackList;
/*  121 */     this.k = par2GameSettings;
/*  122 */     Graphics g = this.m.getGraphics();
/*  123 */     g.setColor(Color.WHITE);
/*  124 */     g.fillRect(0, 0, 64, 64);
/*  125 */     g.setColor(Color.BLACK);
/*  126 */     g.drawString("missingtex", 1, 10);
/*  127 */     g.dispose();
/*      */   }
/*      */ 
/*      */   
/*      */   public int[] a(String par1Str) {
/*  132 */     h texturepackbase = this.l.a;
/*  133 */     int[] ai = (int[])this.e.get(par1Str);
/*  134 */     if (ai != null)
/*      */     {
/*  136 */       return ai;
/*      */     }
/*      */     
/*      */     try {
/*  140 */       int[] ai1 = null;
/*  141 */       if (par1Str.startsWith("##")) {
/*      */         
/*  143 */         ai1 = b(c(a(texturepackbase.a(par1Str.substring(2)))));
/*      */       }
/*  145 */       else if (par1Str.startsWith("%clamp%")) {
/*      */         
/*  147 */         this.b = true;
/*  148 */         ai1 = b(a(texturepackbase.a(par1Str.substring(7))));
/*  149 */         this.b = false;
/*      */       }
/*  151 */       else if (par1Str.startsWith("%blur%")) {
/*      */         
/*  153 */         this.c = true;
/*  154 */         this.b = true;
/*  155 */         ai1 = b(a(texturepackbase.a(par1Str.substring(6))));
/*  156 */         this.b = false;
/*  157 */         this.c = false;
/*      */       } else {
/*      */         
/*  160 */         InputStream inputstream = texturepackbase.a(par1Str);
/*  161 */         if (inputstream == null) {
/*      */           
/*  163 */           ai1 = b(this.m);
/*      */         } else {
/*      */           
/*  166 */           ai1 = b(a(inputstream));
/*      */         } 
/*      */       } 
/*  169 */       this.e.put(par1Str, ai1);
/*  170 */       return ai1;
/*      */     }
/*  172 */     catch (IOException ioexception) {
/*      */       
/*  174 */       ioexception.printStackTrace();
/*      */       
/*  176 */       int[] ai2 = b(this.m);
/*  177 */       this.e.put(par1Str, ai2);
/*  178 */       return ai2;
/*      */     } 
/*      */   }
/*      */   
/*      */   private int[] b(BufferedImage par1BufferedImage) {
/*  183 */     int i = par1BufferedImage.getWidth();
/*  184 */     int j = par1BufferedImage.getHeight();
/*  185 */     int[] ai = new int[i * j];
/*  186 */     par1BufferedImage.getRGB(0, 0, i, j, ai, 0, i);
/*  187 */     return ai;
/*      */   }
/*      */ 
/*      */   
/*      */   private int[] a(BufferedImage par1BufferedImage, int[] par2ArrayOfInteger) {
/*  192 */     int i = par1BufferedImage.getWidth();
/*  193 */     int j = par1BufferedImage.getHeight();
/*  194 */     par1BufferedImage.getRGB(0, 0, i, j, par2ArrayOfInteger, 0, i);
/*  195 */     return par2ArrayOfInteger;
/*      */   }
/*      */ 
/*      */   
/*      */   public int b(String par1Str) {
/*  200 */     h texturepackbase = this.l.a;
/*  201 */     Integer integer = (Integer)this.d.get(par1Str);
/*  202 */     if (integer != null)
/*      */     {
/*  204 */       return integer.intValue();
/*      */     }
/*      */ 
/*      */     
/*      */     try {
/*  209 */       if (Reflector.hasClass(1)) {
/*  210 */         Reflector.callVoid(18, new Object[] { par1Str });
/*      */       }
/*  212 */       this.g.clear();
/*  213 */       ew.a(this.g);
/*      */       
/*  215 */       if (adz.renderingWorldRenderer) {
/*  216 */         System.out.printf("Warning: Texture %s not preloaded, will cause render glitches!\n", new Object[] { par1Str });
/*      */       }
/*  218 */       int i = this.g.get(0);
/*      */       
/*  220 */       Config.dbg("setupTexture: \"" + par1Str + "\", id: " + i);
/*      */       
/*  222 */       if (par1Str.startsWith("##")) {
/*      */         
/*  224 */         a(c(a(texturepackbase.a(par1Str.substring(2)))), i);
/*      */       }
/*  226 */       else if (par1Str.startsWith("%clamp%")) {
/*      */         
/*  228 */         this.b = true;
/*  229 */         a(a(texturepackbase.a(par1Str.substring(7))), i);
/*  230 */         this.b = false;
/*      */       }
/*  232 */       else if (par1Str.startsWith("%blur%")) {
/*      */         
/*  234 */         this.c = true;
/*  235 */         a(a(texturepackbase.a(par1Str.substring(6))), i);
/*  236 */         this.c = false;
/*      */       }
/*  238 */       else if (par1Str.startsWith("%blurclamp%")) {
/*      */         
/*  240 */         this.c = true;
/*  241 */         this.b = true;
/*  242 */         a(a(texturepackbase.a(par1Str.substring(11))), i);
/*  243 */         this.c = false;
/*  244 */         this.b = false;
/*      */       } else {
/*      */         
/*  247 */         InputStream inputstream = texturepackbase.a(par1Str);
/*  248 */         if (inputstream == null) {
/*      */           
/*  250 */           a(this.m, i);
/*      */         }
/*      */         else {
/*      */           
/*  254 */           if (par1Str.equals("/terrain.png"))
/*  255 */             this.terrainTextureId = i; 
/*  256 */           if (par1Str.equals("/gui/items.png"))
/*  257 */             this.guiItemsTextureId = i; 
/*  258 */           if (par1Str.equals("/ctm.png")) {
/*  259 */             this.ctmTextureId = i;
/*      */           }
/*  261 */           a(a(inputstream), i);
/*      */         } 
/*      */       } 
/*  264 */       this.d.put(par1Str, Integer.valueOf(i));
/*      */       
/*  266 */       if (Reflector.hasClass(1)) {
/*  267 */         Reflector.callVoid(19, new Object[] { par1Str, Integer.valueOf(i) });
/*      */       }
/*  269 */       return i;
/*      */     }
/*  271 */     catch (Exception exception) {
/*      */       
/*  273 */       exception.printStackTrace();
/*      */       
/*  275 */       ew.a(this.g);
/*  276 */       int j = this.g.get(0);
/*  277 */       a(this.m, j);
/*  278 */       this.d.put(par1Str, Integer.valueOf(j));
/*  279 */       return j;
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private BufferedImage c(BufferedImage par1BufferedImage) {
/*  288 */     int i = par1BufferedImage.getWidth() / 16;
/*  289 */     BufferedImage bufferedimage = new BufferedImage(16, par1BufferedImage.getHeight() * i, 2);
/*  290 */     Graphics g = bufferedimage.getGraphics();
/*  291 */     for (int j = 0; j < i; j++)
/*      */     {
/*  293 */       g.drawImage(par1BufferedImage, -j * 16, j * par1BufferedImage.getHeight(), null);
/*      */     }
/*      */     
/*  296 */     g.dispose();
/*  297 */     return bufferedimage;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public int a(BufferedImage par1BufferedImage) {
/*  305 */     this.g.clear();
/*  306 */     ew.a(this.g);
/*  307 */     int i = this.g.get(0);
/*  308 */     a(par1BufferedImage, i);
/*  309 */     this.f.a(i, par1BufferedImage);
/*  310 */     return i;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(BufferedImage par1BufferedImage, int par2) {
/*  318 */     GL11.glBindTexture(3553, par2);
/*      */     
/*  320 */     boolean mipmapsActive = (a && Config.isUseMipmaps());
/*  321 */     if (mipmapsActive && par2 != this.guiItemsTextureId) {
/*      */ 
/*      */       
/*  324 */       int mipmapType = Config.getMipmapType();
/*  325 */       GL11.glTexParameteri(3553, 10241, mipmapType);
/*  326 */       GL11.glTexParameteri(3553, 10240, 9728);
/*      */       
/*  328 */       if ((GLContext.getCapabilities()).OpenGL12)
/*      */       {
/*  330 */         GL11.glTexParameteri(3553, 33084, 0);
/*  331 */         int mipmapLevel = Config.getMipmapLevel();
/*      */         
/*  333 */         if (mipmapLevel >= 4) {
/*      */           
/*  335 */           int minDim = Math.min(par1BufferedImage.getWidth(), par1BufferedImage.getHeight());
/*      */           
/*  337 */           mipmapLevel = getMaxMipmapLevel(minDim) - 4;
/*      */           
/*  339 */           if (mipmapLevel < 0)
/*  340 */             mipmapLevel = 0; 
/*      */         } 
/*  342 */         GL11.glTexParameteri(3553, 33085, mipmapLevel);
/*      */       
/*      */       }
/*      */     
/*      */     }
/*      */     else {
/*      */       
/*  349 */       GL11.glTexParameteri(3553, 10241, 9728);
/*  350 */       GL11.glTexParameteri(3553, 10240, 9728);
/*      */     } 
/*  352 */     if (this.c) {
/*      */       
/*  354 */       GL11.glTexParameteri(3553, 10241, 9729);
/*  355 */       GL11.glTexParameteri(3553, 10240, 9729);
/*      */     } 
/*  357 */     if (this.b) {
/*      */       
/*  359 */       GL11.glTexParameteri(3553, 10242, 10496);
/*  360 */       GL11.glTexParameteri(3553, 10243, 10496);
/*      */     } else {
/*      */       
/*  363 */       GL11.glTexParameteri(3553, 10242, 10497);
/*  364 */       GL11.glTexParameteri(3553, 10243, 10497);
/*      */     } 
/*  366 */     int width = par1BufferedImage.getWidth();
/*  367 */     int height = par1BufferedImage.getHeight();
/*      */     
/*  369 */     setTextureDimension(par2, new Dimension(width, height));
/*      */     
/*  371 */     if (Reflector.hasClass(7)) {
/*  372 */       Reflector.callVoid(70, new Object[] { Integer.valueOf(par2), Integer.valueOf(width), Integer.valueOf(height), this.i });
/*      */     }
/*  374 */     int[] ai = new int[width * height];
/*  375 */     byte[] byteBuf = new byte[width * height * 4];
/*  376 */     par1BufferedImage.getRGB(0, 0, width, height, ai, 0, width);
/*      */     
/*  378 */     int[] bgColors = new int[256];
/*  379 */     if (mipmapsActive && isTerrainTexture(par2))
/*      */     {
/*  381 */       for (int ty = 0; ty < 16; ty++) {
/*      */         
/*  383 */         for (int tx = 0; tx < 16; tx++)
/*      */         {
/*  385 */           bgColors[ty * 16 + tx] = getAverageOpaqueColor(ai, tx, ty, width, height);
/*      */         }
/*      */       } 
/*      */     }
/*      */     
/*  390 */     for (int k = 0; k < ai.length; k++) {
/*      */       
/*  392 */       int alpha = ai[k] >> 24 & 0xFF;
/*  393 */       int red = ai[k] >> 16 & 0xFF;
/*  394 */       int green = ai[k] >> 8 & 0xFF;
/*  395 */       int blue = ai[k] & 0xFF;
/*  396 */       if (this.k != null && this.k.g) {
/*      */         
/*  398 */         int i3 = (red * 30 + green * 59 + blue * 11) / 100;
/*  399 */         int k3 = (red * 30 + green * 70) / 100;
/*  400 */         int i4 = (red * 30 + blue * 70) / 100;
/*  401 */         red = i3;
/*  402 */         green = k3;
/*  403 */         blue = i4;
/*      */       } 
/*      */ 
/*      */ 
/*      */       
/*  408 */       if (alpha == 0)
/*      */       {
/*  410 */         if (isTerrainTexture(par2)) {
/*      */           
/*  412 */           red = 255;
/*  413 */           green = 255;
/*  414 */           blue = 255;
/*      */           
/*  416 */           if (mipmapsActive) {
/*      */             
/*  418 */             int px = k % width;
/*  419 */             int py = k / width;
/*      */             
/*  421 */             int tx = px / width / 16;
/*  422 */             int ty = py / height / 16;
/*      */             
/*  424 */             int bgCol = bgColors[ty * 16 + tx];
/*  425 */             if (bgCol != 0)
/*      */             {
/*  427 */               red = bgCol >> 16 & 0xFF;
/*  428 */               green = bgCol >> 8 & 0xFF;
/*  429 */               blue = bgCol & 0xFF;
/*      */             }
/*      */           
/*      */           } 
/*      */         } else {
/*      */           
/*  435 */           red = 0;
/*  436 */           green = 0;
/*  437 */           blue = 0;
/*      */         } 
/*      */       }
/*      */       
/*  441 */       byteBuf[k * 4 + 0] = (byte)red;
/*  442 */       byteBuf[k * 4 + 1] = (byte)green;
/*  443 */       byteBuf[k * 4 + 2] = (byte)blue;
/*  444 */       byteBuf[k * 4 + 3] = (byte)alpha;
/*      */     } 
/*      */     
/*  447 */     checkImageDataSize(width, height);
/*  448 */     this.h.clear();
/*  449 */     this.h.put(byteBuf);
/*  450 */     this.h.position(0).limit(byteBuf.length);
/*  451 */     GL11.glTexImage2D(3553, 0, 6408, width, height, 0, 6408, 5121, this.h);
/*  452 */     if (a)
/*      */     {
/*      */       
/*  455 */       generateMipMaps(this.h, width, height);
/*      */     }
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
/*      */   private int getAverageOpaqueColor(int[] ai, int tx, int ty, int width, int height) {
/*  469 */     int tileWidth = width / 16;
/*  470 */     int tileHeight = height / 16;
/*  471 */     int startPos = ty * tileHeight * width + tx * tileWidth;
/*      */     
/*  473 */     long redSum = 0L;
/*  474 */     long greenSum = 0L;
/*  475 */     long blueSum = 0L;
/*  476 */     long count = 0L;
/*      */     
/*  478 */     for (int py = 0; py < tileHeight; py++) {
/*      */       
/*  480 */       for (int px = 0; px < tileWidth; px++) {
/*      */         
/*  482 */         int k = startPos + py * width + px;
/*      */         
/*  484 */         int i = ai[k] >> 24 & 0xFF;
/*  485 */         if (i != 0) {
/*      */ 
/*      */           
/*  488 */           int red = ai[k] >> 16 & 0xFF;
/*  489 */           int green = ai[k] >> 8 & 0xFF;
/*  490 */           int blue = ai[k] & 0xFF;
/*      */           
/*  492 */           redSum += red;
/*  493 */           greenSum += green;
/*  494 */           blueSum += blue;
/*  495 */           count++;
/*      */         } 
/*      */       } 
/*      */     } 
/*  499 */     if (count <= 0L) {
/*  500 */       return 0;
/*      */     }
/*  502 */     int alpha = 255;
/*  503 */     int redAvg = (int)(redSum / count);
/*  504 */     int greenAvg = (int)(greenSum / count);
/*  505 */     int blueAvg = (int)(blueSum / count);
/*      */     
/*  507 */     return alpha << 24 | redAvg << 16 | greenAvg << 8 | blueAvg;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private boolean isTerrainTexture(int i) {
/*  516 */     if (i == this.terrainTextureId)
/*  517 */       return true; 
/*  518 */     if (i == this.ctmTextureId) {
/*  519 */       return true;
/*      */     }
/*  521 */     return false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void generateMipMaps(ByteBuffer data, int width, int height) {
/*  530 */     ByteBuffer parMipData = data;
/*      */     
/*  532 */     for (int level = 1; level <= 16; level++) {
/*      */       
/*  534 */       int parWidth = width >> level - 1;
/*  535 */       int mipWidth = width >> level;
/*  536 */       int mipHeight = height >> level;
/*      */       
/*  538 */       if (mipWidth <= 0 || mipHeight <= 0) {
/*      */         break;
/*      */       }
/*      */       
/*  542 */       ByteBuffer mipData = this.mipImageDatas[level - 1];
/*  543 */       mipData.limit(mipWidth * mipHeight * 4);
/*  544 */       for (int mipX = 0; mipX < mipWidth; mipX++) {
/*      */         
/*  546 */         for (int mipY = 0; mipY < mipHeight; mipY++) {
/*      */           
/*  548 */           int p1 = parMipData.getInt((mipX * 2 + 0 + (mipY * 2 + 0) * parWidth) * 4);
/*  549 */           int p2 = parMipData.getInt((mipX * 2 + 1 + (mipY * 2 + 0) * parWidth) * 4);
/*  550 */           int p3 = parMipData.getInt((mipX * 2 + 1 + (mipY * 2 + 1) * parWidth) * 4);
/*  551 */           int p4 = parMipData.getInt((mipX * 2 + 0 + (mipY * 2 + 1) * parWidth) * 4);
/*  552 */           int pixel = alphaBlend(p1, p2, p3, p4);
/*  553 */           mipData.putInt((mipX + mipY * mipWidth) * 4, pixel);
/*      */         } 
/*      */       } 
/*  556 */       mipData.rewind();
/*  557 */       GL11.glTexImage2D(3553, level, 6408, mipWidth, mipHeight, 0, 6408, 5121, mipData);
/*  558 */       parMipData = mipData;
/*      */     } 
/*      */   }
/*      */ 
/*      */   
/*      */   public void a(int[] par1ArrayOfInteger, int par2, int par3, int par4) {
/*  564 */     GL11.glBindTexture(3553, par4);
/*      */     
/*  566 */     if (a && Config.isUseMipmaps()) {
/*      */       
/*  568 */       GL11.glTexParameteri(3553, 10241, 9986);
/*  569 */       GL11.glTexParameteri(3553, 10240, 9728);
/*      */     } else {
/*      */       
/*  572 */       GL11.glTexParameteri(3553, 10241, 9728);
/*  573 */       GL11.glTexParameteri(3553, 10240, 9728);
/*      */     } 
/*  575 */     if (this.c) {
/*      */       
/*  577 */       GL11.glTexParameteri(3553, 10241, 9729);
/*  578 */       GL11.glTexParameteri(3553, 10240, 9729);
/*      */     } 
/*  580 */     if (this.b) {
/*      */       
/*  582 */       GL11.glTexParameteri(3553, 10242, 10496);
/*  583 */       GL11.glTexParameteri(3553, 10243, 10496);
/*      */     } else {
/*      */       
/*  586 */       GL11.glTexParameteri(3553, 10242, 10497);
/*  587 */       GL11.glTexParameteri(3553, 10243, 10497);
/*      */     } 
/*  589 */     byte[] abyte0 = new byte[par2 * par3 * 4];
/*  590 */     for (int i = 0; i < par1ArrayOfInteger.length; i++) {
/*      */       
/*  592 */       int j = par1ArrayOfInteger[i] >> 24 & 0xFF;
/*  593 */       int k = par1ArrayOfInteger[i] >> 16 & 0xFF;
/*  594 */       int l = par1ArrayOfInteger[i] >> 8 & 0xFF;
/*  595 */       int i1 = par1ArrayOfInteger[i] & 0xFF;
/*  596 */       if (this.k != null && this.k.g) {
/*      */         
/*  598 */         int j1 = (k * 30 + l * 59 + i1 * 11) / 100;
/*  599 */         int k1 = (k * 30 + l * 70) / 100;
/*  600 */         int l1 = (k * 30 + i1 * 70) / 100;
/*  601 */         k = j1;
/*  602 */         l = k1;
/*  603 */         i1 = l1;
/*      */       } 
/*  605 */       abyte0[i * 4 + 0] = (byte)k;
/*  606 */       abyte0[i * 4 + 1] = (byte)l;
/*  607 */       abyte0[i * 4 + 2] = (byte)i1;
/*  608 */       abyte0[i * 4 + 3] = (byte)j;
/*      */     } 
/*  610 */     this.h.clear();
/*  611 */     this.h.put(abyte0);
/*  612 */     this.h.position(0).limit(abyte0.length);
/*  613 */     GL11.glTexSubImage2D(3553, 0, 0, 0, par2, par3, 6408, 5121, this.h);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(int par1) {
/*  621 */     this.f.d(par1);
/*  622 */     this.g.clear();
/*  623 */     this.g.put(par1);
/*  624 */     this.g.flip();
/*  625 */     GL11.glDeleteTextures(this.g);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public int a(String par1Str, String par2Str) {
/*  636 */     if (Config.isRandomMobs()) {
/*      */       
/*  638 */       int rmTex = RandomMobs.getTexture(par1Str, par2Str);
/*  639 */       if (rmTex >= 0) {
/*  640 */         return rmTex;
/*      */       }
/*      */     } 
/*  643 */     dm threaddownloadimagedata = (dm)this.j.get(par1Str);
/*  644 */     if (threaddownloadimagedata != null && threaddownloadimagedata.a != null && !threaddownloadimagedata.d) {
/*      */       
/*  646 */       if (threaddownloadimagedata.c < 0) {
/*      */         
/*  648 */         threaddownloadimagedata.c = a(threaddownloadimagedata.a);
/*      */       } else {
/*      */         
/*  651 */         a(threaddownloadimagedata.a, threaddownloadimagedata.c);
/*      */       } 
/*  653 */       threaddownloadimagedata.d = true;
/*      */     } 
/*  655 */     if (threaddownloadimagedata == null || threaddownloadimagedata.c < 0) {
/*      */       
/*  657 */       if (par2Str == null)
/*      */       {
/*  659 */         return -1;
/*      */       }
/*      */       
/*  662 */       return b(par2Str);
/*      */     } 
/*      */ 
/*      */     
/*  666 */     return threaddownloadimagedata.c;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public dm a(String par1Str, js par2ImageBuffer) {
/*  676 */     dm threaddownloadimagedata = (dm)this.j.get(par1Str);
/*  677 */     if (threaddownloadimagedata == null) {
/*      */       
/*  679 */       this.j.put(par1Str, new dm(par1Str, par2ImageBuffer));
/*      */     } else {
/*      */       
/*  682 */       threaddownloadimagedata.b++;
/*      */     } 
/*  684 */     return threaddownloadimagedata;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void c(String par1Str) {
/*  692 */     dm threaddownloadimagedata = (dm)this.j.get(par1Str);
/*  693 */     if (threaddownloadimagedata != null) {
/*      */       
/*  695 */       threaddownloadimagedata.b--;
/*  696 */       if (threaddownloadimagedata.b == 0) {
/*      */         
/*  698 */         if (threaddownloadimagedata.c >= 0)
/*      */         {
/*  700 */           a(threaddownloadimagedata.c);
/*      */         }
/*  702 */         this.j.remove(par1Str);
/*      */       } 
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(tt par1TextureFX) {
/*  710 */     if (Reflector.hasClass(7)) {
/*  711 */       Reflector.callVoid(71, new Object[] { par1TextureFX });
/*      */     }
/*  713 */     int newTexId = getTextureId(par1TextureFX);
/*  714 */     for (int i = 0; i < this.i.size(); i++) {
/*      */       
/*  716 */       tt fx = this.i.get(i);
/*  717 */       int texId = getTextureId(fx);
/*  718 */       if (texId == newTexId && fx.g == par1TextureFX.g) {
/*      */         
/*  720 */         this.i.remove(i);
/*  721 */         i--;
/*  722 */         Config.log("TextureFX removed: " + fx + ", texId: " + texId + ", index: " + fx.g);
/*      */       } 
/*      */     } 
/*      */     
/*  726 */     if (par1TextureFX instanceof TextureHDFX) {
/*      */       
/*  728 */       TextureHDFX texHdFx = (TextureHDFX)par1TextureFX;
/*      */       
/*  730 */       texHdFx.setTexturePackBase(this.l.a);
/*      */       
/*  732 */       Dimension dim = getTextureDimensions(newTexId);
/*  733 */       if (dim != null) {
/*  734 */         texHdFx.setTileWidth(dim.width / 16);
/*      */       }
/*      */     } 
/*  737 */     this.i.add(par1TextureFX);
/*  738 */     par1TextureFX.a();
/*  739 */     Config.log("TextureFX registered: " + par1TextureFX + ", texId: " + newTexId + ", index: " + par1TextureFX.g);
/*      */     
/*  741 */     this.dynamicTexturesUpdated = false;
/*      */   }
/*      */ 
/*      */   
/*      */   private int getTextureId(tt fx) {
/*  746 */     Integer texId = (Integer)this.textureFxMap.get(fx);
/*  747 */     if (texId != null) {
/*  748 */       return texId.intValue();
/*      */     }
/*  750 */     int oldTexId = getBoundTexture();
/*      */     
/*  752 */     fx.a(this);
/*      */     
/*  754 */     int texIdInt = getBoundTexture();
/*      */     
/*  756 */     b(oldTexId);
/*      */     
/*  758 */     this.textureFxMap.put(fx, new Integer(texIdInt));
/*      */ 
/*      */ 
/*      */     
/*  762 */     return texIdInt;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private int getBoundTexture() {
/*  772 */     int texId = GL11.glGetInteger(32873);
/*  773 */     return texId;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void generateMipMapsSub(int xOffset, int yOffset, int width, int height, ByteBuffer data, int numTiles, boolean fastColor, String dataId) {
/*  783 */     ByteBuffer parMipData = data;
/*  784 */     byte[][] mipDataBufs = (byte[][])null;
/*  785 */     if (dataId.length() > 0) {
/*      */       
/*  787 */       mipDataBufs = (byte[][])this.mipDataBufsMap.get(dataId);
/*      */       
/*  789 */       if (mipDataBufs == null) {
/*      */         
/*  791 */         mipDataBufs = new byte[17][];
/*      */         
/*  793 */         this.mipDataBufsMap.put(dataId, mipDataBufs);
/*      */       } 
/*      */     } 
/*      */     
/*  797 */     for (int level = 1; level <= 16; level++) {
/*      */       
/*  799 */       int parWidth = width >> level - 1;
/*  800 */       int mipWidth = width >> level;
/*  801 */       int mipHeight = height >> level;
/*  802 */       int xMipOffset = xOffset >> level;
/*  803 */       int yMipOffset = yOffset >> level;
/*      */       
/*  805 */       if (mipWidth <= 0 || mipHeight <= 0) {
/*      */         break;
/*      */       }
/*      */       
/*  809 */       ByteBuffer mipData = this.mipImageDatas[level - 1];
/*      */       
/*  811 */       mipData.limit(mipWidth * mipHeight * 4);
/*      */       
/*  813 */       byte[] mipDataBuf = null;
/*  814 */       if (mipDataBufs != null) {
/*  815 */         mipDataBuf = mipDataBufs[level];
/*      */       }
/*  817 */       if (mipDataBuf != null && mipDataBuf.length != mipWidth * mipHeight * 4) {
/*  818 */         mipDataBuf = null;
/*      */       }
/*  820 */       if (mipDataBuf == null) {
/*      */         
/*  822 */         if (mipDataBufs != null)
/*  823 */           mipDataBuf = new byte[mipWidth * mipHeight * 4]; 
/*  824 */         for (int mipX = 0; mipX < mipWidth; mipX++) {
/*      */           
/*  826 */           for (int mipY = 0; mipY < mipHeight; mipY++) {
/*      */             
/*  828 */             int pixel, p1 = parMipData.getInt((mipX * 2 + 0 + (mipY * 2 + 0) * parWidth) * 4);
/*  829 */             int p2 = parMipData.getInt((mipX * 2 + 1 + (mipY * 2 + 0) * parWidth) * 4);
/*  830 */             int p3 = parMipData.getInt((mipX * 2 + 1 + (mipY * 2 + 1) * parWidth) * 4);
/*  831 */             int p4 = parMipData.getInt((mipX * 2 + 0 + (mipY * 2 + 1) * parWidth) * 4);
/*      */ 
/*      */             
/*  834 */             if (fastColor) {
/*  835 */               pixel = averageColor(averageColor(p1, p2), averageColor(p3, p4));
/*      */             } else {
/*  837 */               pixel = alphaBlend(p1, p2, p3, p4);
/*      */             } 
/*  839 */             mipData.putInt((mipX + mipY * mipWidth) * 4, pixel);
/*      */           } 
/*      */         } 
/*  842 */         if (mipDataBufs != null) {
/*      */ 
/*      */           
/*  845 */           mipData.rewind();
/*  846 */           mipData.get(mipDataBuf);
/*  847 */           mipDataBufs[level] = mipDataBuf;
/*      */         } 
/*      */       } 
/*      */       
/*  851 */       if (mipDataBuf != null) {
/*      */ 
/*      */         
/*  854 */         mipData.rewind();
/*  855 */         mipData.put(mipDataBuf);
/*      */       } 
/*      */ 
/*      */       
/*  859 */       mipData.rewind();
/*  860 */       for (int ix = 0; ix < numTiles; ix++) {
/*      */         
/*  862 */         for (int iy = 0; iy < numTiles; iy++) {
/*      */           
/*  864 */           int dx = ix * mipWidth;
/*  865 */           int dy = iy * mipHeight;
/*  866 */           GL11.glTexSubImage2D(3553, level, xMipOffset + dx, yMipOffset + dy, mipWidth, mipHeight, 6408, 5121, mipData);
/*      */         } 
/*      */       } 
/*      */       
/*  870 */       parMipData = mipData;
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a() {
/*  878 */     boolean mipmapsActive = (a && Config.isUseMipmaps());
/*      */     
/*  880 */     checkHdTextures();
/*      */     
/*  882 */     this.tickCounter++;
/*      */     
/*  884 */     this.terrainTextureId = b("/terrain.png");
/*  885 */     this.guiItemsTextureId = b("/gui/items.png");
/*  886 */     this.ctmTextureId = b("/ctm.png");
/*      */     
/*  888 */     StringBuffer dataIdBuf = new StringBuffer();
/*      */     
/*  890 */     int boundTextureId = -1;
/*      */     
/*  892 */     for (int i = 0; i < this.i.size(); i++) {
/*      */       
/*  894 */       tt texturefx = this.i.get(i);
/*  895 */       texturefx.h = this.k.g;
/*      */       
/*  897 */       if (texturefx.getClass().getName().equals("ModTextureStatic") && this.dynamicTexturesUpdated) {
/*      */         continue;
/*      */       }
/*      */ 
/*      */ 
/*      */       
/*  903 */       int tid = getTextureId(texturefx);
/*  904 */       Dimension dim = getTextureDimensions(tid);
/*  905 */       if (dim == null)
/*  906 */         throw new IllegalArgumentException("Unknown dimensions for texture id: " + tid); 
/*  907 */       int tileWidth = dim.width / 16;
/*  908 */       int tileHeight = dim.height / 16;
/*      */       
/*  910 */       checkImageDataSize(dim.width, dim.height);
/*  911 */       this.h.limit(0);
/*      */       
/*  913 */       dataIdBuf.setLength(0);
/*      */       
/*  915 */       boolean customOk = updateCustomTexture(texturefx, tid, this.h, dim.width / 16, dataIdBuf);
/*      */       
/*  917 */       if (customOk && this.h.limit() <= 0) {
/*      */         continue;
/*      */       }
/*      */ 
/*      */ 
/*      */       
/*  923 */       if (this.h.limit() <= 0) {
/*      */         
/*  925 */         boolean defaultOk = updateDefaultTexture(texturefx, tid, this.h, dim.width / 16, dataIdBuf);
/*      */         
/*  927 */         if (defaultOk && this.h.limit() <= 0) {
/*      */           continue;
/*      */         }
/*      */       } 
/*      */ 
/*      */ 
/*      */       
/*  934 */       if (this.h.limit() <= 0) {
/*      */ 
/*      */         
/*  937 */         texturefx.a();
/*      */         
/*  939 */         if (Reflector.hasClass(7))
/*      */         {
/*  941 */           if (!Reflector.callBoolean(72, new Object[] { texturefx })) {
/*      */             continue;
/*      */           }
/*      */         }
/*  945 */         if (texturefx.f == null) {
/*      */           continue;
/*      */         }
/*  948 */         int targetDataLen = tileWidth * tileHeight * 4;
/*  949 */         if (texturefx.f.length == targetDataLen) {
/*      */           
/*  951 */           this.h.clear();
/*  952 */           this.h.put(texturefx.f);
/*  953 */           this.h.position(0).limit(texturefx.f.length);
/*      */         }
/*      */         else {
/*      */           
/*  957 */           copyScaled(texturefx.f, this.h, tileWidth);
/*      */         } 
/*      */       } 
/*      */       
/*  961 */       if (tid != boundTextureId) {
/*      */         
/*  963 */         texturefx.a(this);
/*  964 */         boundTextureId = tid;
/*      */       } 
/*      */       
/*  967 */       boolean fastColor = scalesWithFastColor(texturefx);
/*      */       
/*  969 */       for (int ix = 0; ix < texturefx.j; ix++) {
/*      */         
/*  971 */         for (int iy = 0; iy < texturefx.j; iy++) {
/*      */ 
/*      */           
/*  974 */           int xOffset = texturefx.g % 16 * tileWidth + ix * tileWidth;
/*  975 */           int yOffset = texturefx.g / 16 * tileHeight + iy * tileHeight;
/*  976 */           GL11.glTexSubImage2D(3553, 0, xOffset, yOffset, tileWidth, tileHeight, 6408, 5121, this.h);
/*      */           
/*  978 */           if (mipmapsActive && tid != this.guiItemsTextureId) {
/*      */ 
/*      */             
/*  981 */             String dataId = dataIdBuf.toString();
/*      */ 
/*      */             
/*  984 */             if (ix == 0 && iy == 0)
/*  985 */               generateMipMapsSub(xOffset, yOffset, tileWidth, tileHeight, this.h, texturefx.j, fastColor, dataId); 
/*      */           } 
/*      */         } 
/*      */       } 
/*      */       continue;
/*      */     } 
/*  991 */     if (this.textureAnimations != null) {
/*      */ 
/*      */       
/*  994 */       boolean animatedTextures = this.k.ofAnimatedTextures;
/*      */       
/*  996 */       for (int j = 0; j < this.textureAnimations.length; j++) {
/*      */         
/*  998 */         CustomAnimation anim = this.textureAnimations[j];
/*  999 */         int tid = b(anim.destTexture);
/* 1000 */         if (tid >= 0) {
/*      */           
/* 1002 */           Dimension dim = getTextureDimensions(tid);
/* 1003 */           if (dim != null) {
/*      */ 
/*      */             
/* 1006 */             checkImageDataSize(dim.width, dim.height);
/* 1007 */             this.h.limit(0);
/*      */             
/* 1009 */             dataIdBuf.setLength(0);
/*      */             
/* 1011 */             boolean ok = anim.updateCustomTexture(this.h, animatedTextures, this.dynamicTexturesUpdated, dataIdBuf);
/* 1012 */             if (!ok || this.h.limit() > 0)
/*      */             {
/*      */ 
/*      */ 
/*      */               
/* 1017 */               if (this.h.limit() > 0) {
/*      */ 
/*      */                 
/* 1020 */                 b(tid);
/* 1021 */                 GL11.glTexSubImage2D(3553, 0, anim.destX, anim.destY, anim.frameWidth, anim.frameHeight, 6408, 5121, this.h);
/*      */               }  } 
/*      */           } 
/*      */         } 
/*      */       } 
/* 1026 */     }  this.dynamicTexturesUpdated = true;
/*      */   }
/*      */ 
/*      */ 
/*      */   
/*      */   private int averageColor(int i, int j) {
/* 1032 */     int k = (i & 0xFF000000) >> 24 & 0xFF;
/* 1033 */     int l = (j & 0xFF000000) >> 24 & 0xFF;
/* 1034 */     return (k + l >> 1 << 24) + ((i & 0xFEFEFE) + (j & 0xFEFEFE) >> 1);
/*      */   }
/*      */ 
/*      */ 
/*      */   
/*      */   private int alphaBlend(int c1, int c2, int c3, int c4) {
/* 1040 */     int cx1 = a(c1, c2);
/* 1041 */     int cx2 = a(c3, c4);
/* 1042 */     int cx = a(cx1, cx2);
/* 1043 */     return cx;
/*      */   }
/*      */ 
/*      */   
/*      */   private int a(int c1, int c2) {
/* 1048 */     int a1 = (c1 & 0xFF000000) >> 24 & 0xFF;
/* 1049 */     int a2 = (c2 & 0xFF000000) >> 24 & 0xFF;
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */     
/* 1055 */     int ax = (a1 + a2) / 2;
/*      */     
/* 1057 */     if (a1 == 0 && a2 == 0) {
/*      */       
/* 1059 */       a1 = 1;
/* 1060 */       a2 = 1;
/*      */     
/*      */     }
/*      */     else {
/*      */       
/* 1065 */       if (a1 == 0) {
/*      */         
/* 1067 */         c1 = c2;
/* 1068 */         ax /= 2;
/*      */       } 
/*      */       
/* 1071 */       if (a2 == 0) {
/*      */         
/* 1073 */         c2 = c1;
/* 1074 */         ax /= 2;
/*      */       } 
/*      */     } 
/*      */     
/* 1078 */     int r1 = (c1 >> 16 & 0xFF) * a1;
/* 1079 */     int g1 = (c1 >> 8 & 0xFF) * a1;
/* 1080 */     int b1 = (c1 & 0xFF) * a1;
/*      */     
/* 1082 */     int r2 = (c2 >> 16 & 0xFF) * a2;
/* 1083 */     int g2 = (c2 >> 8 & 0xFF) * a2;
/* 1084 */     int b2 = (c2 & 0xFF) * a2;
/*      */     
/* 1086 */     int rx = (r1 + r2) / (a1 + a2);
/* 1087 */     int gx = (g1 + g2) / (a1 + a2);
/* 1088 */     int bx = (b1 + b2) / (a1 + a2);
/*      */     
/* 1090 */     return ax << 24 | rx << 16 | gx << 8 | bx;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void b() {
/* 1100 */     this.textureDataMap.clear();
/* 1101 */     this.textureFxMap.clear();
/* 1102 */     this.dynamicTexturesUpdated = false;
/* 1103 */     Config.setTextureUpdateTime(System.currentTimeMillis());
/* 1104 */     RandomMobs.resetTextures();
/* 1105 */     this.mipDataBufsMap.clear();
/* 1106 */     this.customAnimationMap.clear();
/*      */     
/* 1108 */     h texturepackbase = this.l.a;
/*      */ 
/*      */     
/* 1111 */     for (Iterator<Integer> iterator = this.f.b().iterator(); iterator.hasNext(); a(bufferedimage, i)) {
/*      */       
/* 1113 */       int i = ((Integer)iterator.next()).intValue();
/* 1114 */       BufferedImage bufferedimage = (BufferedImage)this.f.a(i);
/*      */     } 
/*      */     
/* 1117 */     for (Iterator<dm> iterator1 = this.j.values().iterator(); iterator1.hasNext(); ) {
/*      */       
/* 1119 */       dm threaddownloadimagedata = iterator1.next();
/* 1120 */       threaddownloadimagedata.d = false;
/*      */     } 
/*      */     
/* 1123 */     for (Iterator<String> iterator2 = this.d.keySet().iterator(); iterator2.hasNext(); ) {
/*      */       
/* 1125 */       String s = iterator2.next();
/*      */       
/*      */       try {
/*      */         BufferedImage bufferedimage1;
/* 1129 */         if (s.startsWith("##")) {
/*      */           
/* 1131 */           bufferedimage1 = c(a(texturepackbase.a(s.substring(2))));
/*      */         }
/* 1133 */         else if (s.startsWith("%clamp%")) {
/*      */           
/* 1135 */           this.b = true;
/* 1136 */           bufferedimage1 = a(texturepackbase.a(s.substring(7)));
/*      */         }
/* 1138 */         else if (s.startsWith("%blur%")) {
/*      */           
/* 1140 */           this.c = true;
/* 1141 */           bufferedimage1 = a(texturepackbase.a(s.substring(6)));
/*      */         }
/* 1143 */         else if (s.startsWith("%blurclamp%")) {
/*      */           
/* 1145 */           this.c = true;
/* 1146 */           this.b = true;
/* 1147 */           bufferedimage1 = a(texturepackbase.a(s.substring(11)));
/*      */         } else {
/*      */           
/* 1150 */           bufferedimage1 = a(texturepackbase.a(s));
/*      */         } 
/* 1152 */         int j = ((Integer)this.d.get(s)).intValue();
/* 1153 */         a(bufferedimage1, j);
/* 1154 */         this.c = false;
/* 1155 */         this.b = false;
/*      */       }
/* 1157 */       catch (Exception e) {
/*      */         
/* 1159 */         if (!"input == null!".equals(e.getMessage())) {
/* 1160 */           e.printStackTrace();
/*      */         }
/*      */       } 
/*      */     } 
/* 1164 */     for (Iterator<String> iterator3 = this.e.keySet().iterator(); iterator3.hasNext(); ) {
/*      */       
/* 1166 */       String s1 = iterator3.next();
/*      */       
/*      */       try {
/*      */         BufferedImage bufferedimage2;
/* 1170 */         if (s1.startsWith("##")) {
/*      */           
/* 1172 */           bufferedimage2 = c(a(texturepackbase.a(s1.substring(2))));
/*      */         }
/* 1174 */         else if (s1.startsWith("%clamp%")) {
/*      */           
/* 1176 */           this.b = true;
/* 1177 */           bufferedimage2 = a(texturepackbase.a(s1.substring(7)));
/*      */         }
/* 1179 */         else if (s1.startsWith("%blur%")) {
/*      */           
/* 1181 */           this.c = true;
/* 1182 */           bufferedimage2 = a(texturepackbase.a(s1.substring(6)));
/*      */         } else {
/*      */           
/* 1185 */           bufferedimage2 = a(texturepackbase.a(s1));
/*      */         } 
/* 1187 */         a(bufferedimage2, (int[])this.e.get(s1));
/* 1188 */         this.c = false;
/* 1189 */         this.b = false;
/*      */       }
/* 1191 */       catch (Exception e) {
/*      */         
/* 1193 */         if (!"input == null!".equals(e.getMessage())) {
/* 1194 */           e.printStackTrace();
/*      */         }
/*      */       } 
/*      */     } 
/* 1198 */     registerCustomTexturesFX();
/* 1199 */     CustomColorizer.update(this);
/* 1200 */     ConnectedTextures.update(this);
/* 1201 */     NaturalTextures.update(this);
/*      */     
/* 1203 */     if (Reflector.hasClass(7)) {
/* 1204 */       Reflector.callVoid(73, new Object[] { this, texturepackbase, this.i });
/*      */     }
/* 1206 */     a();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private BufferedImage a(InputStream par1InputStream) throws IOException {
/* 1214 */     BufferedImage bufferedimage = ImageIO.read(par1InputStream);
/* 1215 */     par1InputStream.close();
/* 1216 */     return bufferedimage;
/*      */   }
/*      */ 
/*      */   
/*      */   public void b(int par1) {
/* 1221 */     if (par1 < 0) {
/*      */       return;
/*      */     }
/*      */ 
/*      */     
/* 1226 */     GL11.glBindTexture(3553, par1);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void setTextureDimension(int texId, Dimension dim) {
/* 1237 */     this.textureDimensionsMap.put(new Integer(texId), dim);
/*      */ 
/*      */ 
/*      */     
/* 1241 */     if (texId == this.terrainTextureId)
/*      */     {
/* 1243 */       Config.setIconWidthTerrain(dim.width / 16);
/*      */     }
/* 1245 */     if (texId == this.guiItemsTextureId)
/*      */     {
/* 1247 */       Config.setIconWidthItems(dim.width / 16);
/*      */     }
/*      */     
/* 1250 */     updateDinamicTextures(texId, dim);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public Dimension getTextureDimensions(int id) {
/* 1257 */     Dimension dim = (Dimension)this.textureDimensionsMap.get(new Integer(id));
/*      */ 
/*      */ 
/*      */     
/* 1261 */     return dim;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void updateDinamicTextures(int texId, Dimension dim) {
/* 1270 */     for (int i = 0; i < this.i.size(); i++) {
/*      */       
/* 1272 */       tt fx = this.i.get(i);
/*      */       
/* 1274 */       int fxTexId = getTextureId(fx);
/* 1275 */       if (fxTexId == texId)
/*      */       {
/*      */         
/* 1278 */         if (fx instanceof TextureHDFX) {
/*      */           
/* 1280 */           TextureHDFX texHD = (TextureHDFX)fx;
/*      */           
/* 1282 */           texHD.setTexturePackBase(this.l.a);
/* 1283 */           texHD.setTileWidth(dim.width / 16);
/* 1284 */           texHD.a();
/*      */         } 
/*      */       }
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean updateCustomTexture(tt texturefx, int texId, ByteBuffer imgData, int tileWidth, StringBuffer dataIdBuf) {
/* 1297 */     if (texId == this.terrainTextureId) {
/*      */ 
/*      */       
/* 1300 */       if (texturefx.g == pb.B.bN) {
/*      */         
/* 1302 */         if (Config.isGeneratedWater())
/* 1303 */           return false; 
/* 1304 */         return updateCustomTexture(texturefx, "/custom_water_still.png", imgData, tileWidth, Config.isAnimatedWater(), 1, dataIdBuf);
/*      */       } 
/* 1306 */       if (texturefx.g == pb.B.bN + 1) {
/*      */         
/* 1308 */         if (Config.isGeneratedWater())
/* 1309 */           return false; 
/* 1310 */         return updateCustomTexture(texturefx, "/custom_water_flowing.png", imgData, tileWidth, Config.isAnimatedWater(), 1, dataIdBuf);
/*      */       } 
/*      */       
/* 1313 */       if (texturefx.g == pb.D.bN) {
/*      */         
/* 1315 */         if (Config.isGeneratedLava())
/* 1316 */           return false; 
/* 1317 */         return updateCustomTexture(texturefx, "/custom_lava_still.png", imgData, tileWidth, Config.isAnimatedLava(), 1, dataIdBuf);
/*      */       } 
/* 1319 */       if (texturefx.g == pb.D.bN + 1) {
/*      */         
/* 1321 */         if (Config.isGeneratedLava())
/* 1322 */           return false; 
/* 1323 */         return updateCustomTexture(texturefx, "/custom_lava_flowing.png", imgData, tileWidth, Config.isAnimatedLava(), 1, dataIdBuf);
/*      */       } 
/*      */       
/* 1326 */       if (texturefx.g == pb.be.bN) {
/* 1327 */         return updateCustomTexture(texturefx, "/custom_portal.png", imgData, tileWidth, Config.isAnimatedPortal(), 1, dataIdBuf);
/*      */       }
/* 1329 */       if (texturefx.g == pb.ar.bN)
/* 1330 */         return updateCustomTexture(texturefx, "/custom_fire_n_s.png", imgData, tileWidth, Config.isAnimatedFire(), 1, dataIdBuf); 
/* 1331 */       if (texturefx.g == pb.ar.bN + 16) {
/* 1332 */         return updateCustomTexture(texturefx, "/custom_fire_e_w.png", imgData, tileWidth, Config.isAnimatedFire(), 1, dataIdBuf);
/*      */       }
/* 1334 */       if (Config.isAnimatedTerrain()) {
/* 1335 */         return updateCustomTexture(texturefx, "/custom_terrain_" + texturefx.g + ".png", imgData, tileWidth, Config.isAnimatedTerrain(), 1, dataIdBuf);
/*      */       }
/*      */     } 
/* 1338 */     if (texId == this.guiItemsTextureId)
/*      */     {
/*      */       
/* 1341 */       if (Config.isAnimatedItems()) {
/* 1342 */         return updateCustomTexture(texturefx, "/custom_item_" + texturefx.g + ".png", imgData, tileWidth, Config.isAnimatedTerrain(), 1, dataIdBuf);
/*      */       }
/*      */     }
/* 1345 */     return false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private boolean updateDefaultTexture(tt texturefx, int texId, ByteBuffer imgData, int tileWidth, StringBuffer dataIdBuf) {
/* 1356 */     if (texId != this.terrainTextureId) {
/* 1357 */       return false;
/*      */     }
/* 1359 */     if (this.l.a instanceof lc) {
/* 1360 */       return false;
/*      */     }
/* 1362 */     if (texturefx.g == pb.B.bN) {
/*      */       
/* 1364 */       if (Config.isGeneratedWater())
/* 1365 */         return false; 
/* 1366 */       return updateDefaultTexture(texturefx, imgData, tileWidth, false, 1, dataIdBuf);
/*      */     } 
/* 1368 */     if (texturefx.g == pb.B.bN + 1) {
/*      */       
/* 1370 */       if (Config.isGeneratedWater())
/* 1371 */         return false; 
/* 1372 */       return updateDefaultTexture(texturefx, imgData, tileWidth, Config.isAnimatedWater(), 1, dataIdBuf);
/*      */     } 
/*      */     
/* 1375 */     if (texturefx.g == pb.D.bN) {
/*      */       
/* 1377 */       if (Config.isGeneratedLava())
/* 1378 */         return false; 
/* 1379 */       return updateDefaultTexture(texturefx, imgData, tileWidth, false, 1, dataIdBuf);
/*      */     } 
/* 1381 */     if (texturefx.g == pb.D.bN + 1) {
/*      */       
/* 1383 */       if (Config.isGeneratedLava())
/* 1384 */         return false; 
/* 1385 */       return updateDefaultTexture(texturefx, imgData, tileWidth, Config.isAnimatedLava(), 3, dataIdBuf);
/*      */     } 
/*      */     
/* 1388 */     return false;
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
/*      */   private boolean updateDefaultTexture(tt texturefx, ByteBuffer imgData, int tileWidth, boolean scrolling, int scrollDiv, StringBuffer dataIdBuf) {
/* 1401 */     int iconIndex = texturefx.g;
/*      */     
/* 1403 */     if (!scrolling && this.dynamicTexturesUpdated) {
/* 1404 */       return true;
/*      */     }
/* 1406 */     byte[] tileData = getTerrainIconData(iconIndex, tileWidth, dataIdBuf);
/* 1407 */     if (tileData == null) {
/* 1408 */       return false;
/*      */     }
/* 1410 */     imgData.clear();
/* 1411 */     int imgLen = tileData.length;
/* 1412 */     if (scrolling) {
/*      */       
/* 1414 */       int movNum = tileWidth - this.tickCounter / scrollDiv % tileWidth;
/* 1415 */       int offset = movNum * tileWidth * 4;
/* 1416 */       imgData.put(tileData, offset, imgLen - offset);
/* 1417 */       imgData.put(tileData, 0, offset);
/*      */       
/* 1419 */       dataIdBuf.append(":");
/* 1420 */       dataIdBuf.append(movNum);
/*      */     }
/*      */     else {
/*      */       
/* 1424 */       imgData.put(tileData, 0, imgLen);
/*      */     } 
/*      */ 
/*      */     
/* 1428 */     imgData.position(0).limit(imgLen);
/*      */     
/* 1430 */     return true;
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
/*      */   private boolean updateCustomTexture(tt texturefx, String imagePath, ByteBuffer imgData, int tileWidth, boolean animated, int animDiv, StringBuffer dataIdBuf) {
/* 1442 */     int tileHeight = tileWidth;
/* 1443 */     CustomAnimation anim = getCustomAnimation(imagePath, tileWidth, tileHeight, animDiv);
/* 1444 */     if (anim == null) {
/* 1445 */       return false;
/*      */     }
/* 1447 */     return anim.updateCustomTexture(imgData, animated, this.dynamicTexturesUpdated, dataIdBuf);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private CustomAnimation getCustomAnimation(String imagePath, int tileWidth, int tileHeight, int durMul) {
/* 1458 */     CustomAnimation anim = (CustomAnimation)this.customAnimationMap.get(imagePath);
/* 1459 */     if (anim == null) {
/*      */ 
/*      */       
/* 1462 */       if (this.customAnimationMap.containsKey(imagePath)) {
/* 1463 */         return null;
/*      */       }
/* 1465 */       byte[] data = getCustomTextureData(imagePath, tileWidth);
/* 1466 */       if (data == null) {
/*      */ 
/*      */         
/* 1469 */         this.customAnimationMap.put(imagePath, null);
/*      */         
/* 1471 */         return null;
/*      */       } 
/*      */       
/* 1474 */       Properties props = new Properties();
/* 1475 */       String propName = makePropertiesName(imagePath);
/* 1476 */       if (propName != null) {
/*      */         
/*      */         try {
/*      */           
/* 1480 */           InputStream propIn = this.l.a.a(propName);
/* 1481 */           if (propIn == null)
/* 1482 */             propIn = this.l.a.a("/anim" + propName); 
/* 1483 */           if (propIn != null) {
/* 1484 */             props.load(propIn);
/*      */           }
/* 1486 */         } catch (IOException e) {
/*      */           
/* 1488 */           e.printStackTrace();
/*      */         } 
/*      */       }
/* 1491 */       anim = new CustomAnimation(imagePath, data, tileWidth, tileHeight, props, durMul);
/*      */       
/* 1493 */       this.customAnimationMap.put(imagePath, anim);
/*      */     } 
/*      */     
/* 1496 */     return anim;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private String makePropertiesName(String imagePath) {
/* 1505 */     if (!imagePath.endsWith(".png")) {
/* 1506 */       return null;
/*      */     }
/* 1508 */     int pos = imagePath.lastIndexOf(".png");
/* 1509 */     if (pos < 0) {
/* 1510 */       return null;
/*      */     }
/* 1512 */     String propsName = imagePath.substring(0, pos) + ".properties";
/*      */     
/* 1514 */     return propsName;
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
/*      */   private byte[] getTerrainIconData(int tileNum, int tileWidth, StringBuffer dataIdBuf) {
/* 1526 */     String tileIdStr = "Tile-" + tileNum;
/* 1527 */     byte[] tileData = getCustomTextureData(tileIdStr, tileWidth);
/* 1528 */     if (tileData != null) {
/*      */       
/* 1530 */       dataIdBuf.append(tileIdStr);
/* 1531 */       return tileData;
/*      */     } 
/*      */     
/* 1534 */     byte[] terrainData = getCustomTextureData("/terrain.png", tileWidth * 16);
/* 1535 */     if (terrainData == null) {
/* 1536 */       return null;
/*      */     }
/* 1538 */     tileData = new byte[tileWidth * tileWidth * 4];
/*      */     
/* 1540 */     int tx = tileNum % 16;
/* 1541 */     int ty = tileNum / 16;
/*      */     
/* 1543 */     int xMin = tx * tileWidth;
/* 1544 */     int yMin = ty * tileWidth;
/*      */     
/* 1546 */     int xMax = xMin + tileWidth;
/* 1547 */     int yMax = yMin + tileWidth;
/*      */     
/* 1549 */     for (int y = 0; y < tileWidth; y++) {
/*      */       
/* 1551 */       int ys = yMin + y;
/* 1552 */       for (int x = 0; x < tileWidth; x++) {
/*      */         
/* 1554 */         int xs = xMin + x;
/* 1555 */         int posSrc = 4 * (xs + ys * tileWidth * 16);
/* 1556 */         int posDst = 4 * (x + y * tileWidth);
/* 1557 */         tileData[posDst + 0] = terrainData[posSrc + 0];
/* 1558 */         tileData[posDst + 1] = terrainData[posSrc + 1];
/* 1559 */         tileData[posDst + 2] = terrainData[posSrc + 2];
/* 1560 */         tileData[posDst + 3] = terrainData[posSrc + 3];
/*      */       } 
/*      */     } 
/*      */     
/* 1564 */     setCustomTextureData(tileIdStr, tileData);
/*      */     
/* 1566 */     dataIdBuf.append(tileIdStr);
/*      */     
/* 1568 */     return tileData;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public byte[] getCustomTextureData(String imagePath, int tileWidth) {
/* 1577 */     byte[] imageBytes = (byte[])this.textureDataMap.get(imagePath);
/* 1578 */     if (imageBytes == null) {
/*      */ 
/*      */       
/* 1581 */       if (this.textureDataMap.containsKey(imagePath))
/*      */       {
/*      */         
/* 1584 */         return null;
/*      */       }
/*      */       
/* 1587 */       imageBytes = loadImage(imagePath, tileWidth);
/*      */       
/* 1589 */       if (imageBytes == null) {
/* 1590 */         imageBytes = loadImage("/anim" + imagePath, tileWidth);
/*      */       }
/* 1592 */       this.textureDataMap.put(imagePath, imageBytes);
/*      */     } 
/* 1594 */     return imageBytes;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void setCustomTextureData(String imagePath, byte[] data) {
/* 1603 */     this.textureDataMap.put(imagePath, data);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private byte[] loadImage(String name, int targetWidth) {
/*      */     try {
/* 1615 */       h texturePackBase = this.l.a;
/* 1616 */       if (texturePackBase == null)
/* 1617 */         return null; 
/* 1618 */       InputStream in = texturePackBase.a(name);
/* 1619 */       if (in == null)
/* 1620 */         return null; 
/* 1621 */       BufferedImage image = a(in);
/* 1622 */       if (image == null)
/* 1623 */         return null; 
/* 1624 */       if (targetWidth > 0 && image.getWidth() != targetWidth) {
/*      */         
/* 1626 */         double aspectHW = (image.getHeight() / image.getWidth());
/* 1627 */         int targetHeight = (int)(targetWidth * aspectHW);
/* 1628 */         image = scaleBufferedImage(image, targetWidth, targetHeight);
/*      */       } 
/*      */       
/* 1631 */       int width = image.getWidth();
/* 1632 */       int height = image.getHeight();
/* 1633 */       int[] ai = new int[width * height];
/* 1634 */       byte[] byteBuf = new byte[width * height * 4];
/* 1635 */       image.getRGB(0, 0, width, height, ai, 0, width);
/* 1636 */       for (int l = 0; l < ai.length; l++) {
/*      */         
/* 1638 */         int alpha = ai[l] >> 24 & 0xFF;
/* 1639 */         int red = ai[l] >> 16 & 0xFF;
/* 1640 */         int green = ai[l] >> 8 & 0xFF;
/* 1641 */         int blue = ai[l] & 0xFF;
/*      */         
/* 1643 */         if (this.k != null && this.k.g) {
/*      */           
/* 1645 */           int j3 = (red * 30 + green * 59 + blue * 11) / 100;
/* 1646 */           int l3 = (red * 30 + green * 70) / 100;
/* 1647 */           int j4 = (red * 30 + blue * 70) / 100;
/* 1648 */           red = j3;
/* 1649 */           green = l3;
/* 1650 */           blue = j4;
/*      */         } 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */         
/* 1661 */         byteBuf[l * 4 + 0] = (byte)red;
/* 1662 */         byteBuf[l * 4 + 1] = (byte)green;
/* 1663 */         byteBuf[l * 4 + 2] = (byte)blue;
/* 1664 */         byteBuf[l * 4 + 3] = (byte)alpha;
/*      */       } 
/* 1666 */       return byteBuf;
/*      */     }
/* 1668 */     catch (Exception e) {
/*      */       
/* 1670 */       e.printStackTrace();
/* 1671 */       return null;
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static BufferedImage scaleBufferedImage(BufferedImage image, int width, int height) {
/* 1683 */     BufferedImage scaledImage = new BufferedImage(width, height, 2);
/* 1684 */     Graphics2D gr = scaledImage.createGraphics();
/*      */     
/* 1686 */     gr.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_BILINEAR);
/* 1687 */     gr.drawImage(image, 0, 0, width, height, null);
/* 1688 */     return scaledImage;
/*      */   }
/*      */ 
/*      */   
/*      */   private void checkImageDataSize(int width, int height) {
/* 1693 */     if (this.h != null) {
/*      */       
/* 1695 */       int len = width * height * 4;
/* 1696 */       if (this.h.capacity() >= len) {
/*      */         return;
/*      */       }
/*      */     } 
/* 1700 */     allocateImageData(width, height);
/*      */   }
/*      */ 
/*      */   
/*      */   private void allocateImageData(int width, int height) {
/* 1705 */     int imgLen = width * height * 4;
/* 1706 */     this.h = ew.c(imgLen);
/*      */     
/* 1708 */     List<ByteBuffer> list = new ArrayList();
/* 1709 */     int mipWidth = width / 2;
/* 1710 */     int mipHeight = height / 2;
/* 1711 */     while (mipWidth > 0 && mipHeight > 0) {
/*      */       
/* 1713 */       int mipLen = mipWidth * mipHeight * 4;
/* 1714 */       ByteBuffer buf = ew.c(mipLen);
/* 1715 */       list.add(buf);
/*      */       
/* 1717 */       mipWidth /= 2;
/* 1718 */       mipHeight /= 2;
/*      */     } 
/*      */     
/* 1721 */     this.mipImageDatas = list.<ByteBuffer>toArray(new ByteBuffer[list.size()]);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void checkHdTextures() {
/* 1729 */     if (this.hdTexturesInstalled) {
/*      */       return;
/*      */     }
/* 1732 */     Minecraft mc = Config.getMinecraft();
/* 1733 */     if (mc == null) {
/*      */       return;
/*      */     }
/* 1736 */     this.hdTexturesInstalled = true;
/*      */     
/* 1738 */     a(new TextureHDLavaFX());
/* 1739 */     a(new TextureHDWaterFX());
/* 1740 */     a(new TextureHDPortalFX());
/* 1741 */     a(new TextureHDWaterFlowFX());
/* 1742 */     a(new TextureHDLavaFlowFX());
/* 1743 */     a(new TextureHDFlamesFX(0));
/* 1744 */     a(new TextureHDFlamesFX(1));
/*      */     
/* 1746 */     a(new TextureHDCompassFX(mc));
/* 1747 */     a(new TextureHDWatchFX(mc));
/*      */     
/* 1749 */     registerCustomTexturesFX();
/*      */     
/* 1751 */     CustomColorizer.update(this);
/*      */     
/* 1753 */     ConnectedTextures.update(this);
/* 1754 */     NaturalTextures.update(this);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void registerCustomTexturesFX() {
/* 1763 */     tt[] customTextures = getRegisteredTexturesFX(TextureHDCustomFX.class);
/*      */     int i;
/* 1765 */     for (i = 0; i < customTextures.length; i++) {
/*      */       
/* 1767 */       tt fx = customTextures[i];
/* 1768 */       unregisterTextureFX(fx);
/*      */     } 
/*      */ 
/*      */     
/* 1772 */     if (Config.isAnimatedTerrain())
/*      */     {
/*      */       
/* 1775 */       for (i = 0; i < 256; i++)
/*      */       {
/* 1777 */         registerCustomTextureFX("/custom_terrain_" + i + ".png", i, 0);
/*      */       }
/*      */     }
/*      */ 
/*      */     
/* 1782 */     if (Config.isAnimatedItems())
/*      */     {
/*      */       
/* 1785 */       for (i = 0; i < 256; i++)
/*      */       {
/* 1787 */         registerCustomTextureFX("/custom_item_" + i + ".png", i, 1);
/*      */       }
/*      */     }
/*      */     
/* 1791 */     this.textureAnimations = getTextureAnimations();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private CustomAnimation[] getTextureAnimations() {
/* 1799 */     String tpName = this.l.a.a;
/* 1800 */     File dirTexturepacks = new File((Config.getMinecraft()).F, "texturepacks");
/* 1801 */     File tpFile = new File(dirTexturepacks, tpName);
/* 1802 */     if (!tpFile.exists()) {
/* 1803 */       return null;
/*      */     }
/* 1805 */     Properties[] animProps = null;
/* 1806 */     if (tpFile.isFile()) {
/* 1807 */       animProps = getAnimationPropertiesZip(tpFile);
/*      */     } else {
/* 1809 */       animProps = getAnimationPropertiesDir(tpFile);
/*      */     } 
/* 1811 */     if (animProps == null) {
/* 1812 */       return null;
/*      */     }
/* 1814 */     List<CustomAnimation> list = new ArrayList();
/* 1815 */     for (int i = 0; i < animProps.length; i++) {
/*      */       
/* 1817 */       Properties props = animProps[i];
/*      */       
/* 1819 */       CustomAnimation anim = makeTextureAnimation(props);
/*      */       
/* 1821 */       if (anim != null)
/* 1822 */         list.add(anim); 
/*      */     } 
/* 1824 */     CustomAnimation[] anims = list.<CustomAnimation>toArray(new CustomAnimation[list.size()]);
/* 1825 */     return anims;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private CustomAnimation makeTextureAnimation(Properties props) {
/* 1834 */     String texFrom = props.getProperty("from");
/* 1835 */     String texTo = props.getProperty("to");
/* 1836 */     int x = Config.parseInt(props.getProperty("x"), -1);
/* 1837 */     int y = Config.parseInt(props.getProperty("y"), -1);
/* 1838 */     int width = Config.parseInt(props.getProperty("w"), -1);
/* 1839 */     int height = Config.parseInt(props.getProperty("h"), -1);
/*      */     
/* 1841 */     if (texFrom == null || texTo == null)
/* 1842 */       return null; 
/* 1843 */     if (x < 0 || y < 0 || width < 0 || height < 0) {
/* 1844 */       return null;
/*      */     }
/* 1846 */     byte[] imageBytes = getCustomTextureData(texFrom, width);
/* 1847 */     if (imageBytes == null)
/* 1848 */       return null; 
/* 1849 */     CustomAnimation anim = new CustomAnimation(texFrom, imageBytes, width, height, props, 1);
/*      */     
/* 1851 */     anim.destTexture = texTo;
/* 1852 */     anim.destX = x;
/* 1853 */     anim.destY = y;
/*      */     
/* 1855 */     return anim;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private Properties[] getAnimationPropertiesDir(File tpDir) {
/* 1864 */     File dirAnim = new File(tpDir, "anim");
/* 1865 */     if (!dirAnim.exists())
/* 1866 */       return null; 
/* 1867 */     if (!dirAnim.isDirectory()) {
/* 1868 */       return null;
/*      */     }
/* 1870 */     File[] propFiles = dirAnim.listFiles();
/* 1871 */     if (propFiles == null) {
/* 1872 */       return null;
/*      */     }
/*      */     
/*      */     try {
/* 1876 */       List<Properties> list = new ArrayList();
/* 1877 */       for (int i = 0; i < propFiles.length; i++) {
/*      */         
/* 1879 */         File file = propFiles[i];
/* 1880 */         String name = file.getName();
/*      */         
/* 1882 */         if (!name.startsWith("custom_"))
/*      */         {
/*      */           
/* 1885 */           if (name.endsWith(".properties"))
/*      */           {
/*      */             
/* 1888 */             if (file.isFile())
/*      */             {
/*      */               
/* 1891 */               if (file.canRead()) {
/*      */ 
/*      */                 
/* 1894 */                 FileInputStream fin = new FileInputStream(file);
/* 1895 */                 Properties properties = new Properties();
/* 1896 */                 properties.load(fin);
/* 1897 */                 fin.close();
/*      */                 
/* 1899 */                 list.add(properties);
/*      */               }  }  }  } 
/* 1901 */       }  Properties[] props = list.<Properties>toArray(new Properties[list.size()]);
/* 1902 */       return props;
/*      */     }
/* 1904 */     catch (IOException e) {
/*      */       
/* 1906 */       e.printStackTrace();
/* 1907 */       return null;
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private Properties[] getAnimationPropertiesZip(File tpFile) {
/*      */     try {
/* 1919 */       ZipFile zf = new ZipFile(tpFile);
/* 1920 */       Enumeration<? extends ZipEntry> en = zf.entries();
/* 1921 */       List<Properties> list = new ArrayList();
/* 1922 */       while (en.hasMoreElements()) {
/*      */         
/* 1924 */         ZipEntry ze = en.nextElement();
/* 1925 */         String name = ze.getName();
/*      */         
/* 1927 */         if (!name.startsWith("anim/")) {
/*      */           continue;
/*      */         }
/* 1930 */         if (name.startsWith("anim/custom_")) {
/*      */           continue;
/*      */         }
/* 1933 */         if (!name.endsWith(".properties")) {
/*      */           continue;
/*      */         }
/* 1936 */         InputStream in = zf.getInputStream(ze);
/* 1937 */         Properties properties = new Properties();
/* 1938 */         properties.load(in);
/* 1939 */         in.close();
/*      */         
/* 1941 */         list.add(properties);
/*      */       } 
/* 1943 */       Properties[] props = list.<Properties>toArray(new Properties[list.size()]);
/* 1944 */       return props;
/*      */     }
/* 1946 */     catch (IOException e) {
/*      */       
/* 1948 */       e.printStackTrace();
/* 1949 */       return null;
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void unregisterTextureFX(tt texFX) {
/* 1959 */     for (int i = 0; i < this.i.size(); i++) {
/*      */       
/* 1961 */       tt fx = this.i.get(i);
/* 1962 */       if (fx == texFX) {
/*      */ 
/*      */         
/* 1965 */         this.i.remove(i);
/*      */         
/* 1967 */         i--;
/*      */       } 
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private tt[] getRegisteredTexturesFX(Class cls) {
/* 1978 */     List<tt> list = new ArrayList();
/* 1979 */     for (int i = 0; i < this.i.size(); i++) {
/*      */       
/* 1981 */       tt fx = this.i.get(i);
/* 1982 */       if (cls.isAssignableFrom(fx.getClass()))
/* 1983 */         list.add(fx); 
/*      */     } 
/* 1985 */     tt[] texs = list.<tt>toArray(new tt[list.size()]);
/*      */     
/* 1987 */     return texs;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void registerCustomTextureFX(String path, int index, int tileImage) {
/* 1996 */     byte[] data = null;
/* 1997 */     if (tileImage == 0) {
/* 1998 */       data = getCustomTextureData(path, Config.getIconWidthTerrain());
/*      */     } else {
/* 2000 */       data = getCustomTextureData(path, Config.getIconWidthItems());
/* 2001 */     }  if (data == null) {
/*      */       return;
/*      */     }
/* 2004 */     a(new TextureHDCustomFX(index, tileImage));
/*      */   }
/*      */ 
/*      */   
/*      */   private int getMaxMipmapLevel(int size) {
/* 2009 */     int level = 0;
/* 2010 */     while (size > 0) {
/*      */       
/* 2012 */       size /= 2;
/* 2013 */       level++;
/*      */     } 
/* 2015 */     return level - 1;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void copyScaled(byte[] buf, ByteBuffer dstBuf, int dstWidth) {
/* 2025 */     int srcWidth = (int)Math.sqrt((buf.length / 4));
/* 2026 */     int scale = dstWidth / srcWidth;
/* 2027 */     byte[] buf4 = new byte[4];
/* 2028 */     int len = dstWidth * dstWidth;
/*      */     
/* 2030 */     dstBuf.clear();
/*      */     
/* 2032 */     if (scale > 1)
/*      */     {
/* 2034 */       for (int y = 0; y < srcWidth; y++) {
/*      */         
/* 2036 */         int yMul = y * srcWidth;
/* 2037 */         int ty = y * scale;
/* 2038 */         int tyMul = ty * dstWidth;
/* 2039 */         for (int x = 0; x < srcWidth; x++) {
/*      */           
/* 2041 */           int srcPos = (x + yMul) * 4;
/* 2042 */           buf4[0] = buf[srcPos];
/* 2043 */           buf4[1] = buf[srcPos + 1];
/* 2044 */           buf4[2] = buf[srcPos + 2];
/* 2045 */           buf4[3] = buf[srcPos + 3];
/*      */           
/* 2047 */           int tx = x * scale;
/* 2048 */           int dstPosBase = tx + tyMul;
/* 2049 */           for (int tdy = 0; tdy < scale; tdy++) {
/*      */             
/* 2051 */             int dstPosY = dstPosBase + tdy * dstWidth;
/* 2052 */             dstBuf.position(dstPosY * 4);
/* 2053 */             for (int tdx = 0; tdx < scale; tdx++)
/*      */             {
/* 2055 */               dstBuf.put(buf4);
/*      */             }
/*      */           } 
/*      */         } 
/*      */       } 
/*      */     }
/*      */     
/* 2062 */     dstBuf.position(0).limit(dstWidth * dstWidth * 4);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private boolean scalesWithFastColor(tt texturefx) {
/* 2072 */     if (texturefx.getClass().getName().equals("ModTextureStatic")) {
/* 2073 */       return false;
/*      */     }
/* 2075 */     return true;
/*      */   }
/*      */ 
/*      */   
/*      */   public gi getTexturePack() {
/* 2080 */     return this.l;
/*      */   }
/*      */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\aaw.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */