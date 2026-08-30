/*      */ import java.awt.image.BufferedImage;
/*      */ import java.io.IOException;
/*      */ import java.io.InputStream;
/*      */ import java.text.Bidi;
/*      */ import java.util.Arrays;
/*      */ import java.util.Iterator;
/*      */ import java.util.List;
/*      */ import java.util.Properties;
/*      */ import java.util.Random;
/*      */ import java.util.Set;
/*      */ import java.util.regex.Pattern;
/*      */ import javax.imageio.ImageIO;
/*      */ import org.lwjgl.opengl.GL11;
/*      */ 
/*      */ 
/*      */ 
/*      */ public class nl
/*      */ {
/*   19 */   private static final Pattern r = Pattern.compile("(?i)\\u00A7[0-9A-FK-OR]");
/*      */ 
/*      */   
/*      */   private float[] d;
/*      */ 
/*      */   
/*      */   public int a;
/*      */ 
/*      */   
/*      */   public int b;
/*      */ 
/*      */   
/*      */   public Random c;
/*      */ 
/*      */   
/*      */   private byte[] e;
/*      */ 
/*      */   
/*      */   private final int[] f;
/*      */ 
/*      */   
/*      */   private int[] g;
/*      */ 
/*      */   
/*      */   private int h;
/*      */ 
/*      */   
/*      */   private final aaw i;
/*      */ 
/*      */   
/*      */   private float j;
/*      */ 
/*      */   
/*      */   private float k;
/*      */   
/*      */   private boolean l;
/*      */   
/*      */   private boolean m;
/*      */   
/*      */   private float n;
/*      */   
/*      */   private float o;
/*      */   
/*      */   private float p;
/*      */   
/*      */   private float q;
/*      */   
/*      */   private hu gameSettings;
/*      */   
/*      */   private String textureFile;
/*      */   
/*   70 */   private long lastUpdateTime = 0L;
/*      */ 
/*      */ 
/*      */   
/*      */   nl() {
/*   75 */     this.d = new float[256];
/*   76 */     this.a = 0;
/*   77 */     this.b = 8;
/*   78 */     this.c = new Random();
/*   79 */     this.e = new byte[65536];
/*   80 */     this.f = new int[256];
/*   81 */     this.g = new int[32];
/*   82 */     this.i = null;
/*      */   }
/*      */ 
/*      */ 
/*      */   
/*      */   public nl(hu par1GameSettings, String par2Str, aaw par3RenderEngine, boolean par4) {
/*   88 */     this.d = new float[256];
/*   89 */     this.a = 0;
/*   90 */     this.b = 8;
/*   91 */     this.c = new Random();
/*   92 */     this.e = new byte[65536];
/*   93 */     this.f = new int[256];
/*   94 */     this.g = new int[32];
/*   95 */     this.i = par3RenderEngine;
/*   96 */     this.l = par4;
/*      */     
/*   98 */     this.gameSettings = par1GameSettings;
/*   99 */     this.textureFile = par2Str;
/*  100 */     init();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void init() {
/*      */     BufferedImage bufferedimage;
/*  108 */     this.d = new float[256];
/*  109 */     this.a = 0;
/*  110 */     this.e = new byte[65536];
/*      */ 
/*      */ 
/*      */ 
/*      */     
/*      */     try {
/*  116 */       bufferedimage = ImageIO.read(getFontTexturePack().a(this.textureFile));
/*      */       
/*  118 */       InputStream inputstream = getFontTexturePack().a("/font/glyph_sizes.bin");
/*  119 */       inputstream.read(this.e);
/*      */     }
/*  121 */     catch (IOException ioexception) {
/*      */       
/*  123 */       throw new RuntimeException(ioexception);
/*      */     } 
/*  125 */     int imgWidth = bufferedimage.getWidth();
/*  126 */     int imgHeight = bufferedimage.getHeight();
/*      */     
/*  128 */     int charW = imgWidth / 16;
/*  129 */     int charH = imgHeight / 16;
/*      */     
/*  131 */     float kx = imgWidth / 128.0F;
/*      */     
/*  133 */     int[] ai = new int[imgWidth * imgHeight];
/*  134 */     bufferedimage.getRGB(0, 0, imgWidth, imgHeight, ai, 0, imgWidth);
/*  135 */     for (int k = 0; k < 256; k++) {
/*      */       
/*  137 */       int cx = k % 16;
/*  138 */       int cy = k / 16;
/*  139 */       int px = 0;
/*  140 */       for (px = charW - 1; px >= 0; px--) {
/*      */         
/*  142 */         int x = cx * charW + px;
/*  143 */         boolean flag = true;
/*  144 */         for (int py = 0; py < charH && flag; py++) {
/*      */           
/*  146 */           int ypos = (cy * charH + py) * imgWidth;
/*  147 */           int col = ai[x + ypos];
/*  148 */           int alpha = col >> 24 & 0xFF;
/*  149 */           if (alpha > 16)
/*  150 */             flag = false; 
/*      */         } 
/*  152 */         if (!flag)
/*      */           break; 
/*      */       } 
/*  155 */       if (k == 65) {
/*  156 */         k = k;
/*      */       }
/*  158 */       if (k == 32) {
/*  159 */         px = (int)(1.5D * kx);
/*      */       }
/*  161 */       this.d[k] = (px + 1) / kx + 1.0F;
/*      */     } 
/*      */     
/*  164 */     readCustomCharWidths();
/*      */     
/*  166 */     boolean oldUseMipmaps = aaw.a;
/*      */     
/*      */     try {
/*  169 */       aaw.a = false;
/*      */       
/*  171 */       if (this.a <= 0) {
/*  172 */         this.a = this.i.a(bufferedimage);
/*      */       } else {
/*  174 */         this.i.a(bufferedimage, this.a);
/*      */       } 
/*      */     } finally {
/*      */       
/*  178 */       aaw.a = oldUseMipmaps;
/*      */     } 
/*      */     
/*  181 */     for (int l = 0; l < 32; l++) {
/*      */       
/*  183 */       int j1 = (l >> 3 & 0x1) * 85;
/*  184 */       int l1 = (l >> 2 & 0x1) * 170 + j1;
/*  185 */       int j2 = (l >> 1 & 0x1) * 170 + j1;
/*  186 */       int l2 = (l >> 0 & 0x1) * 170 + j1;
/*  187 */       if (l == 6)
/*      */       {
/*  189 */         l1 += 85;
/*      */       }
/*  191 */       if (this.gameSettings.g) {
/*      */         
/*  193 */         int i3 = (l1 * 30 + j2 * 59 + l2 * 11) / 100;
/*  194 */         int k3 = (l1 * 30 + j2 * 70) / 100;
/*  195 */         int i4 = (l1 * 30 + l2 * 70) / 100;
/*  196 */         l1 = i3;
/*  197 */         j2 = k3;
/*  198 */         l2 = i4;
/*      */       } 
/*  200 */       if (l >= 16) {
/*      */         
/*  202 */         l1 /= 4;
/*  203 */         j2 /= 4;
/*  204 */         l2 /= 4;
/*      */       } 
/*  206 */       this.g[l] = (l1 & 0xFF) << 16 | (j2 & 0xFF) << 8 | l2 & 0xFF;
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */   
/*      */   private void readCustomCharWidths() {
/*  213 */     String suffix = ".png";
/*  214 */     if (!this.textureFile.endsWith(suffix))
/*      */       return; 
/*  216 */     String fileName = this.textureFile.substring(0, this.textureFile.length() - suffix.length()) + ".properties";
/*      */     
/*  218 */     InputStream in = getFontTexturePack().a(fileName);
/*  219 */     if (in == null) {
/*      */       return;
/*      */     }
/*      */     try {
/*  223 */       Config.log("Loading " + fileName);
/*  224 */       Properties props = new Properties();
/*  225 */       props.load(in);
/*  226 */       Set<Object> keySet = props.keySet();
/*  227 */       for (Iterator<String> iter = keySet.iterator(); iter.hasNext(); ) {
/*      */         
/*  229 */         String key = iter.next();
/*      */         
/*  231 */         String prefix = "width.";
/*  232 */         if (key.startsWith(prefix)) {
/*      */           
/*  234 */           String numStr = key.substring(prefix.length());
/*  235 */           int num = Config.parseInt(numStr, -1);
/*  236 */           if (num >= 0 && num < this.d.length) {
/*      */             
/*  238 */             String value = props.getProperty(key);
/*  239 */             float width = Config.parseFloat(value, -1.0F);
/*  240 */             if (width >= 0.0F) {
/*  241 */               this.d[num] = width;
/*      */             }
/*      */           } 
/*      */         } 
/*      */       } 
/*  246 */     } catch (IOException e) {
/*      */ 
/*      */       
/*  249 */       e.printStackTrace();
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */   
/*      */   private h getFontTexturePack() {
/*  256 */     if (this.gameSettings.ofCustomFonts)
/*      */     {
/*      */       
/*  259 */       return this.gameSettings.B.E.a;
/*      */     }
/*      */ 
/*      */ 
/*      */     
/*  264 */     return this.gameSettings.B.E.b().get(0);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void checkUpdated() {
/*  272 */     if (Config.getTextureUpdateTime() == this.lastUpdateTime) {
/*      */       return;
/*      */     }
/*  275 */     this.lastUpdateTime = Config.getTextureUpdateTime();
/*      */     
/*  277 */     init();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private float a(int par1, char par2, boolean par3) {
/*  285 */     if (par2 == ' ')
/*      */     {
/*  287 */       return 4.0F;
/*      */     }
/*  289 */     if (par1 > 0 && !this.l)
/*      */     {
/*  291 */       return a(par1 + 32, par3);
/*      */     }
/*      */     
/*  294 */     return a(par2, par3);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private float a(int par1, boolean par2) {
/*  303 */     float f = (par1 % 16 * 8);
/*  304 */     float f1 = (par1 / 16 * 8);
/*  305 */     float f2 = par2 ? 1.0F : 0.0F;
/*  306 */     if (this.h != this.a) {
/*      */       
/*  308 */       GL11.glBindTexture(3553, this.a);
/*  309 */       this.h = this.a;
/*      */     } 
/*  311 */     float f3 = this.d[par1] - 0.01F;
/*  312 */     GL11.glBegin(5);
/*  313 */     GL11.glTexCoord2f(f / 128.0F, f1 / 128.0F);
/*  314 */     GL11.glVertex3f(this.j + f2, this.k, 0.0F);
/*  315 */     GL11.glTexCoord2f(f / 128.0F, (f1 + 7.99F) / 128.0F);
/*  316 */     GL11.glVertex3f(this.j - f2, this.k + 7.99F, 0.0F);
/*  317 */     GL11.glTexCoord2f((f + f3) / 128.0F, f1 / 128.0F);
/*  318 */     GL11.glVertex3f(this.j + f3 + f2, this.k, 0.0F);
/*  319 */     GL11.glTexCoord2f((f + f3) / 128.0F, (f1 + 7.99F) / 128.0F);
/*  320 */     GL11.glVertex3f(this.j + f3 - f2, this.k + 7.99F, 0.0F);
/*  321 */     GL11.glEnd();
/*  322 */     return this.d[par1];
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void a(int par1) {
/*      */     BufferedImage bufferedimage;
/*  330 */     String s = String.format("/font/glyph_%02X.png", new Object[] { Integer.valueOf(par1) });
/*      */ 
/*      */ 
/*      */ 
/*      */     
/*      */     try {
/*  336 */       bufferedimage = ImageIO.read(aaw.class.getResourceAsStream(s));
/*      */     }
/*  338 */     catch (IOException ioexception) {
/*      */       
/*  340 */       throw new RuntimeException(ioexception);
/*      */     } 
/*  342 */     this.f[par1] = this.i.a(bufferedimage);
/*  343 */     this.h = this.f[par1];
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private float a(char par1, boolean par2) {
/*  351 */     if (this.e[par1] == 0)
/*      */     {
/*  353 */       return 0.0F;
/*      */     }
/*  355 */     int i = par1 / 256;
/*  356 */     if (this.f[i] == 0)
/*      */     {
/*  358 */       a(i);
/*      */     }
/*  360 */     if (this.h != this.f[i]) {
/*      */       
/*  362 */       GL11.glBindTexture(3553, this.f[i]);
/*  363 */       this.h = this.f[i];
/*      */     } 
/*  365 */     int j = this.e[par1] >>> 4;
/*  366 */     int k = this.e[par1] & 0xF;
/*  367 */     float f = j;
/*  368 */     float f1 = (k + 1);
/*  369 */     float f2 = (par1 % 16 * 16) + f;
/*  370 */     float f3 = ((par1 & 0xFF) / 16 * 16);
/*  371 */     float f4 = f1 - f - 0.02F;
/*  372 */     float f5 = par2 ? 1.0F : 0.0F;
/*  373 */     GL11.glBegin(5);
/*  374 */     GL11.glTexCoord2f(f2 / 256.0F, f3 / 256.0F);
/*  375 */     GL11.glVertex3f(this.j + f5, this.k, 0.0F);
/*  376 */     GL11.glTexCoord2f(f2 / 256.0F, (f3 + 15.98F) / 256.0F);
/*  377 */     GL11.glVertex3f(this.j - f5, this.k + 7.99F, 0.0F);
/*  378 */     GL11.glTexCoord2f((f2 + f4) / 256.0F, f3 / 256.0F);
/*  379 */     GL11.glVertex3f(this.j + f4 / 2.0F + f5, this.k, 0.0F);
/*  380 */     GL11.glTexCoord2f((f2 + f4) / 256.0F, (f3 + 15.98F) / 256.0F);
/*  381 */     GL11.glVertex3f(this.j + f4 / 2.0F - f5, this.k + 7.99F, 0.0F);
/*  382 */     GL11.glEnd();
/*  383 */     return (f1 - f) / 2.0F + 1.0F;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public int a(String par1Str, int par2, int par3, int par4) {
/*  391 */     if (this.m)
/*      */     {
/*  393 */       par1Str = b(par1Str);
/*      */     }
/*  395 */     int i = a(par1Str, par2 + 1, par3 + 1, par4, true);
/*  396 */     i = Math.max(i, a(par1Str, par2, par3, par4, false));
/*  397 */     return i;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void b(String par1Str, int par2, int par3, int par4) {
/*  405 */     if (this.m)
/*      */     {
/*  407 */       par1Str = b(par1Str);
/*      */     }
/*  409 */     a(par1Str, par2, par3, par4, false);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private String b(String par1Str) {
/*  417 */     if (par1Str == null || !Bidi.requiresBidi(par1Str.toCharArray(), 0, par1Str.length()))
/*      */     {
/*  419 */       return par1Str;
/*      */     }
/*  421 */     Bidi bidi = new Bidi(par1Str, -2);
/*  422 */     byte[] abyte0 = new byte[bidi.getRunCount()];
/*  423 */     String[] as = new String[abyte0.length];
/*  424 */     for (int i = 0; i < abyte0.length; i++) {
/*      */       
/*  426 */       int j = bidi.getRunStart(i);
/*  427 */       int k = bidi.getRunLimit(i);
/*  428 */       int i1 = bidi.getRunLevel(i);
/*  429 */       String s = par1Str.substring(j, k);
/*  430 */       abyte0[i] = (byte)i1;
/*  431 */       as[i] = s;
/*      */     } 
/*      */     
/*  434 */     String[] as1 = (String[])as.clone();
/*  435 */     Bidi.reorderVisually(abyte0, 0, (Object[])as, 0, abyte0.length);
/*  436 */     StringBuilder stringbuilder = new StringBuilder();
/*      */     
/*  438 */     for (int l = 0; l < as.length; l++) {
/*      */       
/*  440 */       byte byte0 = abyte0[l];
/*  441 */       int j1 = 0;
/*      */ 
/*      */       
/*  444 */       while (j1 < as1.length) {
/*      */ 
/*      */ 
/*      */         
/*  448 */         if (as1[j1].equals(as[l])) {
/*      */           
/*  450 */           byte0 = abyte0[j1];
/*      */           break;
/*      */         } 
/*  453 */         j1++;
/*      */       } 
/*  455 */       if ((byte0 & 0x1) == 0) {
/*      */         
/*  457 */         stringbuilder.append(as[l]);
/*      */       } else {
/*      */         
/*  460 */         j1 = as[l].length() - 1;
/*      */ 
/*      */         
/*  463 */         while (j1 >= 0) {
/*      */ 
/*      */ 
/*      */           
/*  467 */           char c = as[l].charAt(j1);
/*  468 */           if (c == '(') {
/*      */             
/*  470 */             c = ')';
/*      */           }
/*  472 */           else if (c == ')') {
/*      */             
/*  474 */             c = '(';
/*      */           } 
/*  476 */           stringbuilder.append(c);
/*  477 */           j1--;
/*      */         } 
/*      */       } 
/*      */     } 
/*  481 */     return stringbuilder.toString();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void a(String par1Str, boolean par2) {
/*  489 */     boolean flag = false;
/*  490 */     boolean flag1 = false;
/*  491 */     boolean flag2 = false;
/*  492 */     boolean flag3 = false;
/*  493 */     boolean flag4 = false;
/*  494 */     for (int i = 0; i < par1Str.length(); i++) {
/*      */       
/*  496 */       char c = par1Str.charAt(i);
/*  497 */       if (c == '§' && i + 1 < par1Str.length()) {
/*      */         
/*  499 */         int j = "0123456789abcdefklmnor".indexOf(par1Str.toLowerCase().charAt(i + 1));
/*  500 */         if (j < 16) {
/*      */           
/*  502 */           flag = false;
/*  503 */           flag1 = false;
/*  504 */           flag4 = false;
/*  505 */           flag3 = false;
/*  506 */           flag2 = false;
/*  507 */           if (j < 0 || j > 15)
/*      */           {
/*  509 */             j = 15;
/*      */           }
/*  511 */           if (par2)
/*      */           {
/*  513 */             j += 16;
/*      */           }
/*  515 */           int l = this.g[j];
/*      */           
/*  517 */           GL11.glColor4f((l >> 16) / 255.0F, (l >> 8 & 0xFF) / 255.0F, (l & 0xFF) / 255.0F, this.q);
/*      */         }
/*  519 */         else if (j == 16) {
/*      */           
/*  521 */           flag = true;
/*      */         }
/*  523 */         else if (j == 17) {
/*      */           
/*  525 */           flag1 = true;
/*      */         }
/*  527 */         else if (j == 18) {
/*      */           
/*  529 */           flag4 = true;
/*      */         }
/*  531 */         else if (j == 19) {
/*      */           
/*  533 */           flag3 = true;
/*      */         }
/*  535 */         else if (j == 20) {
/*      */           
/*  537 */           flag2 = true;
/*      */         }
/*  539 */         else if (j == 21) {
/*      */           
/*  541 */           flag = false;
/*  542 */           flag1 = false;
/*  543 */           flag4 = false;
/*  544 */           flag3 = false;
/*  545 */           flag2 = false;
/*  546 */           GL11.glColor4f(this.n, this.o, this.p, this.q);
/*      */         } 
/*  548 */         i++;
/*      */       } else {
/*      */         
/*  551 */         int k = xn.a.indexOf(c);
/*  552 */         if (flag && k > 0)
/*      */         {
/*      */           while (true) {
/*      */ 
/*      */             
/*  557 */             int i1 = this.c.nextInt(xn.a.length());
/*  558 */             if ((int)this.d[k + 32] == (int)this.d[i1 + 32]) {
/*  559 */               k = i1; break;
/*      */             } 
/*  561 */           }  }  float f = a(k, c, flag2);
/*  562 */         if (flag1) {
/*      */           
/*  564 */           this.j++;
/*  565 */           a(k, c, flag2);
/*  566 */           this.j--;
/*  567 */           f++;
/*      */         } 
/*  569 */         if (flag4) {
/*      */           
/*  571 */           adz tessellator = adz.a;
/*  572 */           GL11.glDisable(3553);
/*  573 */           tessellator.b();
/*  574 */           tessellator.a(this.j, (this.k + (this.b / 2)), 0.0D);
/*  575 */           tessellator.a((this.j + f), (this.k + (this.b / 2)), 0.0D);
/*  576 */           tessellator.a((this.j + f), (this.k + (this.b / 2) - 1.0F), 0.0D);
/*  577 */           tessellator.a(this.j, (this.k + (this.b / 2) - 1.0F), 0.0D);
/*  578 */           tessellator.a();
/*  579 */           GL11.glEnable(3553);
/*      */         } 
/*  581 */         if (flag3) {
/*      */           
/*  583 */           adz tessellator1 = adz.a;
/*  584 */           GL11.glDisable(3553);
/*  585 */           tessellator1.b();
/*  586 */           int j1 = flag3 ? -1 : 0;
/*  587 */           tessellator1.a((this.j + j1), (this.k + this.b), 0.0D);
/*  588 */           tessellator1.a((this.j + f), (this.k + this.b), 0.0D);
/*  589 */           tessellator1.a((this.j + f), (this.k + this.b - 1.0F), 0.0D);
/*  590 */           tessellator1.a((this.j + j1), (this.k + this.b - 1.0F), 0.0D);
/*  591 */           tessellator1.a();
/*  592 */           GL11.glEnable(3553);
/*      */         } 
/*  594 */         this.j += f;
/*      */       } 
/*      */     } 
/*      */   }
/*      */   
/*      */   public int a(String par1Str, int par2, int par3, int par4, boolean par5) {
/*  600 */     if (par1Str != null) {
/*      */       
/*  602 */       this.h = 0;
/*  603 */       if ((par4 & 0xFC000000) == 0)
/*      */       {
/*  605 */         par4 |= 0xFF000000;
/*      */       }
/*  607 */       if (par5)
/*      */       {
/*  609 */         par4 = (par4 & 0xFCFCFC) >> 2 | par4 & 0xFF000000;
/*      */       }
/*  611 */       this.n = (par4 >> 16 & 0xFF) / 255.0F;
/*  612 */       this.o = (par4 >> 8 & 0xFF) / 255.0F;
/*  613 */       this.p = (par4 & 0xFF) / 255.0F;
/*  614 */       this.q = (par4 >> 24 & 0xFF) / 255.0F;
/*  615 */       GL11.glColor4f(this.n, this.o, this.p, this.q);
/*  616 */       this.j = par2;
/*  617 */       this.k = par3;
/*  618 */       a(par1Str, par5);
/*  619 */       return (int)this.j;
/*      */     } 
/*      */     
/*  622 */     return 0;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public int a(String par1Str) {
/*  632 */     checkUpdated();
/*      */     
/*  634 */     if (par1Str == null)
/*      */     {
/*  636 */       return 0;
/*      */     }
/*      */     
/*  639 */     float i = 0.0F;
/*  640 */     boolean flag = false;
/*  641 */     for (int j = 0; j < par1Str.length(); j++) {
/*      */       
/*  643 */       char c = par1Str.charAt(j);
/*      */       
/*  645 */       float k = getCharWidthFloat(c);
/*  646 */       if (k < 0.0F && j < par1Str.length() - 1) {
/*      */         
/*  648 */         char c1 = par1Str.charAt(++j);
/*  649 */         if (c1 == 'l' || c1 == 'L') {
/*      */           
/*  651 */           flag = true;
/*      */         }
/*  653 */         else if (c1 == 'r' || c1 == 'R') {
/*      */           
/*  655 */           flag = false;
/*      */         } 
/*  657 */         k = getCharWidthFloat(c1);
/*      */       } 
/*  659 */       i += k;
/*  660 */       if (flag)
/*      */       {
/*  662 */         i++;
/*      */       }
/*      */     } 
/*      */     
/*  666 */     return Math.round(i);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public int a(char par1) {
/*  675 */     return Math.round(getCharWidthFloat(par1));
/*      */   }
/*      */ 
/*      */ 
/*      */   
/*      */   private float getCharWidthFloat(char par1) {
/*  681 */     if (par1 == '§')
/*      */     {
/*  683 */       return -1.0F;
/*      */     }
/*  685 */     int i = xn.a.indexOf(par1);
/*  686 */     if (i >= 0 && !this.l)
/*      */     {
/*  688 */       return this.d[i + 32];
/*      */     }
/*  690 */     if (this.e[par1] != 0) {
/*      */       
/*  692 */       int j = this.e[par1] >> 4;
/*  693 */       int k = this.e[par1] & 0xF;
/*  694 */       if (k > 7) {
/*      */         
/*  696 */         k = 15;
/*  697 */         j = 0;
/*      */       } 
/*  699 */       return ((++k - j) / 2 + 1);
/*      */     } 
/*      */     
/*  702 */     return 0.0F;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public String a(String par1Str, int par2) {
/*  711 */     return a(par1Str, par2, false);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public String a(String par1Str, int par2, boolean par3) {
/*  719 */     StringBuilder stringbuilder = new StringBuilder();
/*  720 */     float i = 0.0F;
/*  721 */     int j = par3 ? (par1Str.length() - 1) : 0;
/*  722 */     byte byte0 = (byte)(par3 ? -1 : 1);
/*  723 */     boolean flag = false;
/*  724 */     boolean flag1 = false; int k;
/*  725 */     for (k = j; k >= 0 && k < par1Str.length() && i < par2; k += byte0) {
/*      */       
/*  727 */       char c = par1Str.charAt(k);
/*  728 */       float l = getCharWidthFloat(c);
/*  729 */       if (flag) {
/*      */         
/*  731 */         flag = false;
/*  732 */         if (c == 'l' || c == 'L')
/*      */         {
/*  734 */           flag1 = true;
/*      */         }
/*  736 */         else if (c == 'r' || c == 'R')
/*      */         {
/*  738 */           flag1 = false;
/*      */         }
/*      */       
/*  741 */       } else if (l < 0.0F) {
/*      */         
/*  743 */         flag = true;
/*      */       } else {
/*      */         
/*  746 */         i += l;
/*  747 */         if (flag1)
/*      */         {
/*  749 */           i++;
/*      */         }
/*      */       } 
/*  752 */       if (i > par2) {
/*      */         break;
/*      */       }
/*      */       
/*  756 */       if (par3) {
/*      */         
/*  758 */         stringbuilder.insert(0, c);
/*      */       } else {
/*      */         
/*  761 */         stringbuilder.append(c);
/*      */       } 
/*      */     } 
/*      */     
/*  765 */     return stringbuilder.toString();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(String par1Str, int par2, int par3, int par4, int par5) {
/*  774 */     checkUpdated();
/*      */     
/*  776 */     if (this.m)
/*      */     {
/*  778 */       par1Str = b(par1Str);
/*      */     }
/*  780 */     b(par1Str, par2, par3, par4, par5);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void b(String par1Str, int par2, int par3, int par4, int par5) {
/*  788 */     a(par1Str, par2, par3, par4, par5, false);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private void a(String par1Str, int par2, int par3, int par4, int par5, boolean par6) {
/*  798 */     checkUpdated();
/*      */     
/*  800 */     String[] as = par1Str.split("\n");
/*  801 */     if (as.length > 1) {
/*      */       
/*  803 */       for (int i = 0; i < as.length; i++) {
/*      */         
/*  805 */         b(as[i], par2, par3, par4, par5);
/*  806 */         par3 += b(as[i], par4);
/*      */       } 
/*      */       
/*      */       return;
/*      */     } 
/*  811 */     String[] as1 = par1Str.split(" ");
/*  812 */     int j = 0;
/*  813 */     String s = "";
/*      */ 
/*      */     
/*  816 */     while (j < as1.length) {
/*      */       String s1;
/*      */ 
/*      */ 
/*      */       
/*  821 */       for (s1 = s + as1[j++] + " "; j < as1.length && a(s1 + as1[j]) < par4; s1 = s1 + as1[j++] + " ");
/*      */       
/*  823 */       for (; a(s1) > par4; s1 = s + s1.substring(k)) {
/*      */         int k;
/*  825 */         for (k = 0; a(s1.substring(0, k + 1)) <= par4; k++);
/*  826 */         if (s1.substring(0, k).trim().length() > 0) {
/*      */ 
/*      */ 
/*      */           
/*  830 */           String s2 = s1.substring(0, k);
/*  831 */           if (s2.lastIndexOf("§") >= 0)
/*      */           {
/*  833 */             s = "§" + s2.charAt(s2.lastIndexOf("§") + 1);
/*      */           }
/*  835 */           a(s2, par2, par3, par5, par6);
/*  836 */           par3 += this.b;
/*      */         } 
/*      */       } 
/*  839 */       if (a(s1.trim()) > 0) {
/*      */         
/*  841 */         if (s1.lastIndexOf("§") >= 0)
/*      */         {
/*  843 */           s = "§" + s1.charAt(s1.lastIndexOf("§") + 1);
/*      */         }
/*  845 */         a(s1, par2, par3, par5, par6);
/*  846 */         par3 += this.b;
/*      */       } 
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public int b(String par1Str, int par2) {
/*  857 */     checkUpdated();
/*      */     
/*  859 */     String[] as = par1Str.split("\n");
/*  860 */     if (as.length > 1) {
/*      */       
/*  862 */       int i = 0;
/*  863 */       for (int j = 0; j < as.length; j++)
/*      */       {
/*  865 */         i += b(as[j], par2);
/*      */       }
/*      */       
/*  868 */       return i;
/*      */     } 
/*  870 */     String[] as1 = par1Str.split(" ");
/*  871 */     int k = 0;
/*  872 */     int l = 0;
/*      */ 
/*      */     
/*  875 */     while (k < as1.length) {
/*      */       String s;
/*      */ 
/*      */ 
/*      */       
/*  880 */       for (s = as1[k++] + " "; k < as1.length && a(s + as1[k]) < par2; s = s + as1[k++] + " ");
/*      */       
/*  882 */       for (; a(s) > par2; s = s.substring(i1)) {
/*      */         int i1;
/*  884 */         for (i1 = 0; a(s.substring(0, i1 + 1)) <= par2; i1++);
/*  885 */         if (s.substring(0, i1).trim().length() > 0)
/*      */         {
/*  887 */           l += this.b;
/*      */         }
/*      */       } 
/*      */       
/*  891 */       if (s.trim().length() > 0)
/*      */       {
/*  893 */         l += this.b;
/*      */       }
/*      */     } 
/*  896 */     if (l < this.b)
/*      */     {
/*  898 */       l += this.b;
/*      */     }
/*  900 */     return l;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(boolean par1) {
/*  909 */     this.l = par1;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void b(boolean par1) {
/*  917 */     this.m = par1;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public List c(String par1Str, int par2) {
/*  925 */     return Arrays.asList(d(par1Str, par2).split("\n"));
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   String d(String par1Str, int par2) {
/*  933 */     int i = e(par1Str, par2);
/*  934 */     if (par1Str.length() <= i)
/*      */     {
/*  936 */       return par1Str;
/*      */     }
/*      */     
/*  939 */     String s = par1Str.substring(0, i);
/*  940 */     String s1 = c(s) + par1Str.substring(i + ((par1Str.charAt(i) != ' ') ? 0 : 1));
/*  941 */     return s + "\n" + d(s1, par2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private int e(String par1Str, int par2) {
/*  950 */     int i = par1Str.length();
/*  951 */     float j = 0.0F;
/*  952 */     int k = 0;
/*  953 */     int l = -1;
/*  954 */     boolean flag = false;
/*      */ 
/*      */     
/*  957 */     while (k < i) {
/*      */ 
/*      */ 
/*      */       
/*  961 */       char c = par1Str.charAt(k);
/*  962 */       switch (c) {
/*      */         
/*      */         case '§':
/*  965 */           if (k != i) {
/*      */             
/*  967 */             char c1 = par1Str.charAt(++k);
/*  968 */             if (c1 == 'l' || c1 == 'L') {
/*      */               
/*  970 */               flag = true; break;
/*      */             } 
/*  972 */             if (c1 == 'r' || c1 == 'R')
/*      */             {
/*  974 */               flag = false;
/*      */             }
/*      */           } 
/*      */           break;
/*      */         
/*      */         case ' ':
/*  980 */           l = k;
/*      */         
/*      */         default:
/*  983 */           j += getCharWidthFloat(c);
/*  984 */           if (flag)
/*      */           {
/*  986 */             j++;
/*      */           }
/*      */           break;
/*      */       } 
/*  990 */       if (c == '\n') {
/*      */         
/*  992 */         l = ++k;
/*      */         break;
/*      */       } 
/*  995 */       if (j > par2) {
/*      */         break;
/*      */       }
/*      */       
/*  999 */       k++;
/*      */     } 
/* 1001 */     if (k != i && l != -1 && l < k)
/*      */     {
/* 1003 */       return l;
/*      */     }
/*      */     
/* 1006 */     return k;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static boolean b(char par0) {
/* 1015 */     return ((par0 >= '0' && par0 <= '9') || (par0 >= 'a' && par0 <= 'f') || (par0 >= 'A' && par0 <= 'F'));
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static boolean c(char par0) {
/* 1023 */     return ((par0 >= 'k' && par0 <= 'o') || (par0 >= 'K' && par0 <= 'O') || par0 == 'r' || par0 == 'R');
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static String c(String par0Str) {
/* 1031 */     String s = "";
/* 1032 */     int i = -1;
/* 1033 */     int j = par0Str.length();
/*      */ 
/*      */     
/* 1036 */     while ((i = par0Str.indexOf('§', i + 1)) != -1) {
/*      */ 
/*      */ 
/*      */       
/* 1040 */       if (i < j - 1) {
/*      */         
/* 1042 */         char c = par0Str.charAt(i + 1);
/* 1043 */         if (b(c)) {
/*      */           
/* 1045 */           s = "§" + c; continue;
/*      */         } 
/* 1047 */         if (c(c))
/*      */         {
/* 1049 */           s = s + "§" + c;
/*      */         }
/*      */       } 
/*      */     } 
/* 1053 */     return s;
/*      */   }
/*      */ 
/*      */   
/*      */   public static String d(String par0Str) {
/* 1058 */     return r.matcher(par0Str).replaceAll("");
/*      */   }
/*      */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\nl.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */