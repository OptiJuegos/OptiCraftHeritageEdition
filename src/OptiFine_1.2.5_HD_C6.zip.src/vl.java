/*      */ import net.minecraft.client.Minecraft;
/*      */ import org.lwjgl.opengl.GL11;
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
/*      */ public class vl
/*      */ {
/*      */   public ali a;
/*      */   public int d;
/*      */   public boolean e;
/*      */   public boolean f;
/*      */   public static boolean b = true;
/*      */   public static boolean cfgGrassFix = true;
/*      */   public boolean c;
/*      */   public int g;
/*      */   public int h;
/*      */   public int i;
/*      */   public int j;
/*      */   public int k;
/*      */   public int l;
/*      */   public boolean m;
/*      */   public float n;
/*      */   public float o;
/*      */   public float p;
/*      */   public float q;
/*      */   public float r;
/*      */   public float s;
/*      */   public float t;
/*      */   public float u;
/*      */   public float v;
/*      */   public float w;
/*      */   public float x;
/*      */   public float y;
/*      */   public float z;
/*      */   public float A;
/*      */   public float B;
/*      */   public float C;
/*      */   public float D;
/*      */   public float E;
/*      */   public float F;
/*      */   public float G;
/*      */   public float H;
/*      */   public float I;
/*      */   public float J;
/*      */   public float K;
/*      */   public float L;
/*      */   public float M;
/*      */   public float N;
/*      */   public int O;
/*      */   public int P;
/*      */   public int Q;
/*      */   public int R;
/*      */   public int S;
/*      */   public int T;
/*      */   public int U;
/*      */   public int V;
/*      */   public int W;
/*      */   public int X;
/*      */   public int Y;
/*      */   public int Z;
/*      */   public int aa;
/*      */   public int ab;
/*      */   public int ac;
/*      */   public int ad;
/*      */   public int ae;
/*      */   public int af;
/*      */   public int ag;
/*      */   public int ah;
/*      */   public int ai;
/*      */   public int aj;
/*      */   public int ak;
/*      */   public int al;
/*      */   public int am;
/*      */   public float an;
/*      */   public float ao;
/*      */   public float ap;
/*      */   public float aq;
/*      */   public float ar;
/*      */   public float as;
/*      */   public float at;
/*      */   public float au;
/*      */   public float av;
/*      */   public float aw;
/*      */   public float ax;
/*      */   public float ay;
/*      */   public boolean az;
/*      */   public boolean aA;
/*      */   public boolean aB;
/*      */   public boolean aC;
/*      */   public boolean aD;
/*      */   public boolean aE;
/*      */   public boolean aF;
/*      */   public boolean aG;
/*      */   public boolean aH;
/*      */   public boolean aI;
/*      */   public boolean aJ;
/*      */   public boolean aK;
/*      */   public boolean aoLightValuesCalculated;
/*  256 */   public float aoLightValueOpaque = 0.2F;
/*      */ 
/*      */ 
/*      */   
/*      */   public vl(ali par1IBlockAccess) {
/*  261 */     this.d = -1;
/*  262 */     this.e = false;
/*  263 */     this.f = false;
/*  264 */     this.c = true;
/*  265 */     this.g = 0;
/*  266 */     this.h = 0;
/*  267 */     this.i = 0;
/*  268 */     this.j = 0;
/*  269 */     this.k = 0;
/*  270 */     this.l = 0;
/*  271 */     this.ai = 1;
/*  272 */     this.a = par1IBlockAccess;
/*      */     
/*  274 */     this.aoLightValueOpaque = 1.0F - Config.getAmbientOcclusionLevel() * 0.8F;
/*      */   }
/*      */ 
/*      */   
/*      */   public vl() {
/*  279 */     this.d = -1;
/*  280 */     this.e = false;
/*  281 */     this.f = false;
/*  282 */     this.c = true;
/*  283 */     this.g = 0;
/*  284 */     this.h = 0;
/*  285 */     this.i = 0;
/*  286 */     this.j = 0;
/*  287 */     this.k = 0;
/*  288 */     this.l = 0;
/*  289 */     this.ai = 1;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a() {
/*  297 */     this.d = -1;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(pb par1Block, int par2, int par3, int par4, int par5) {
/*  305 */     this.d = par5;
/*  306 */     b(par1Block, par2, par3, par4);
/*  307 */     this.d = -1;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(pb par1Block, int par2, int par3, int par4) {
/*  315 */     this.f = true;
/*  316 */     b(par1Block, par2, par3, par4);
/*  317 */     this.f = false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean b(pb par1Block, int par2, int par3, int par4) {
/*  325 */     int i = par1Block.d();
/*  326 */     par1Block.a(this.a, par2, par3, par4);
/*      */     
/*  328 */     if (Config.isBetterSnow())
/*      */     {
/*  330 */       if (par1Block == pb.aD)
/*      */       {
/*  332 */         if (hasSnowNeighbours(par2, par3, par4)) {
/*  333 */           o(pb.aS, par2, par3, par4);
/*      */         }
/*      */       }
/*      */     }
/*  337 */     if (i == 0)
/*      */     {
/*  339 */       return o(par1Block, par2, par3, par4);
/*      */     }
/*  341 */     if (i == 4)
/*      */     {
/*  343 */       return n(par1Block, par2, par3, par4);
/*      */     }
/*  345 */     if (i == 13)
/*      */     {
/*  347 */       return p(par1Block, par2, par3, par4);
/*      */     }
/*  349 */     if (i == 1)
/*      */     {
/*  351 */       return j(par1Block, par2, par3, par4);
/*      */     }
/*  353 */     if (i == 19)
/*      */     {
/*  355 */       return k(par1Block, par2, par3, par4);
/*      */     }
/*  357 */     if (i == 23)
/*      */     {
/*  359 */       return m(par1Block, par2, par3, par4);
/*      */     }
/*  361 */     if (i == 6)
/*      */     {
/*  363 */       return l(par1Block, par2, par3, par4);
/*      */     }
/*  365 */     if (i == 2)
/*      */     {
/*  367 */       return c(par1Block, par2, par3, par4);
/*      */     }
/*  369 */     if (i == 3)
/*      */     {
/*  371 */       return f(par1Block, par2, par3, par4);
/*      */     }
/*  373 */     if (i == 5)
/*      */     {
/*  375 */       return g(par1Block, par2, par3, par4);
/*      */     }
/*  377 */     if (i == 8)
/*      */     {
/*  379 */       return h(par1Block, par2, par3, par4);
/*      */     }
/*  381 */     if (i == 7)
/*      */     {
/*  383 */       return r(par1Block, par2, par3, par4);
/*      */     }
/*  385 */     if (i == 9)
/*      */     {
/*  387 */       return a((aez)par1Block, par2, par3, par4);
/*      */     }
/*  389 */     if (i == 10)
/*      */     {
/*  391 */       return q(par1Block, par2, par3, par4);
/*      */     }
/*  393 */     if (i == 27)
/*      */     {
/*  395 */       return a((de)par1Block, par2, par3, par4);
/*      */     }
/*  397 */     if (i == 11)
/*      */     {
/*  399 */       return a((abg)par1Block, par2, par3, par4);
/*      */     }
/*  401 */     if (i == 12)
/*      */     {
/*  403 */       return e(par1Block, par2, par3, par4);
/*      */     }
/*  405 */     if (i == 14)
/*      */     {
/*  407 */       return t(par1Block, par2, par3, par4);
/*      */     }
/*  409 */     if (i == 15)
/*      */     {
/*  411 */       return u(par1Block, par2, par3, par4);
/*      */     }
/*  413 */     if (i == 16)
/*      */     {
/*  415 */       return b(par1Block, par2, par3, par4, false);
/*      */     }
/*  417 */     if (i == 17)
/*      */     {
/*  419 */       return c(par1Block, par2, par3, par4, true);
/*      */     }
/*  421 */     if (i == 18)
/*      */     {
/*  423 */       return a((akq)par1Block, par2, par3, par4);
/*      */     }
/*  425 */     if (i == 20)
/*      */     {
/*  427 */       return i(par1Block, par2, par3, par4);
/*      */     }
/*  429 */     if (i == 21)
/*      */     {
/*  431 */       return a((ajf)par1Block, par2, par3, par4);
/*      */     }
/*  433 */     if (i == 24)
/*      */     {
/*  435 */       return a((xm)par1Block, par2, par3, par4);
/*      */     }
/*  437 */     if (i == 25)
/*      */     {
/*  439 */       return a((xc)par1Block, par2, par3, par4);
/*      */     }
/*  441 */     if (i == 26)
/*      */     {
/*  443 */       return s(par1Block, par2, par3, par4);
/*      */     }
/*      */     
/*  446 */     if (Reflector.hasClass(0))
/*      */     {
/*  448 */       return Reflector.callBoolean(0, new Object[] { this, this.a, Integer.valueOf(par2), Integer.valueOf(par3), Integer.valueOf(par4), par1Block, Integer.valueOf(i) });
/*      */     }
/*      */     
/*  451 */     return false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private boolean hasSnowNeighbours(int x, int y, int z) {
/*  462 */     int snowId = pb.aS.bO;
/*  463 */     if (this.a.a(x - 1, y, z) == snowId || this.a.a(x + 1, y, z) == snowId || this.a.a(x, y, z - 1) == snowId || this.a.a(x, y, z + 1) == snowId)
/*      */     {
/*      */ 
/*      */ 
/*      */ 
/*      */       
/*  469 */       return this.a.g(x, y - 1, z);
/*      */     }
/*      */     
/*  472 */     return false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean s(pb par1Block, int par2, int par3, int par4) {
/*  481 */     int i = this.a.e(par2, par3, par4);
/*  482 */     int j = i & 0x3;
/*  483 */     if (j == 0) {
/*      */       
/*  485 */       this.k = 3;
/*      */     }
/*  487 */     else if (j == 3) {
/*      */       
/*  489 */       this.k = 1;
/*      */     }
/*  491 */     else if (j == 1) {
/*      */       
/*  493 */       this.k = 2;
/*      */     } 
/*  495 */     if (!amj.e(i)) {
/*      */       
/*  497 */       par1Block.a(0.0F, 0.0F, 0.0F, 1.0F, 0.8125F, 1.0F);
/*  498 */       o(par1Block, par2, par3, par4);
/*  499 */       par1Block.h();
/*  500 */       this.k = 0;
/*  501 */       return true;
/*      */     } 
/*      */     
/*  504 */     par1Block.a(0.0F, 0.0F, 0.0F, 1.0F, 0.8125F, 1.0F);
/*  505 */     o(par1Block, par2, par3, par4);
/*  506 */     this.d = 174;
/*  507 */     par1Block.a(0.25F, 0.8125F, 0.25F, 0.75F, 1.0F, 0.75F);
/*  508 */     o(par1Block, par2, par3, par4);
/*  509 */     a();
/*  510 */     par1Block.h();
/*  511 */     this.k = 0;
/*  512 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean t(pb par1Block, int par2, int par3, int par4) {
/*  522 */     adz tessellator = adz.a;
/*  523 */     int i = this.a.e(par2, par3, par4);
/*  524 */     int j = pm.a(i);
/*  525 */     boolean flag = pm.e(i);
/*  526 */     float f = 0.5F;
/*  527 */     float f1 = 1.0F;
/*  528 */     float f2 = 0.8F;
/*  529 */     float f3 = 0.6F;
/*  530 */     float f4 = f1;
/*  531 */     float f5 = f1;
/*  532 */     float f6 = f1;
/*  533 */     float f7 = f;
/*  534 */     float f8 = f2;
/*  535 */     float f9 = f3;
/*  536 */     float f10 = f;
/*  537 */     float f11 = f2;
/*  538 */     float f12 = f3;
/*  539 */     float f13 = f;
/*  540 */     float f14 = f2;
/*  541 */     float f15 = f3;
/*  542 */     int k = par1Block.d(this.a, par2, par3, par4);
/*  543 */     tessellator.b(k);
/*  544 */     tessellator.a(f7, f10, f13);
/*  545 */     int l = par1Block.d(this.a, par2, par3, par4, 0);
/*  546 */     int i1 = (l & 0xF) << 4;
/*  547 */     int j1 = l & 0xF0;
/*  548 */     double d = (i1 / 256.0F);
/*  549 */     double d1 = ((i1 + 16) - 0.01D) / 256.0D;
/*  550 */     double d2 = (j1 / 256.0F);
/*  551 */     double d3 = ((j1 + 16) - 0.01D) / 256.0D;
/*  552 */     double d4 = par2 + par1Block.bV;
/*  553 */     double d5 = par2 + par1Block.bY;
/*  554 */     double d6 = par3 + par1Block.bW + 0.1875D;
/*  555 */     double d7 = par4 + par1Block.bX;
/*  556 */     double d8 = par4 + par1Block.ca;
/*  557 */     tessellator.a(d4, d6, d8, d, d3);
/*  558 */     tessellator.a(d4, d6, d7, d, d2);
/*  559 */     tessellator.a(d5, d6, d7, d1, d2);
/*  560 */     tessellator.a(d5, d6, d8, d1, d3);
/*  561 */     tessellator.b(par1Block.d(this.a, par2, par3 + 1, par4));
/*  562 */     tessellator.a(f4, f5, f6);
/*  563 */     l = par1Block.d(this.a, par2, par3, par4, 1);
/*  564 */     i1 = (l & 0xF) << 4;
/*  565 */     j1 = l & 0xF0;
/*  566 */     d = (i1 / 256.0F);
/*  567 */     d1 = ((i1 + 16) - 0.01D) / 256.0D;
/*  568 */     d2 = (j1 / 256.0F);
/*  569 */     d3 = ((j1 + 16) - 0.01D) / 256.0D;
/*  570 */     d4 = d;
/*  571 */     d5 = d1;
/*  572 */     d6 = d2;
/*  573 */     d7 = d2;
/*  574 */     d8 = d;
/*  575 */     double d9 = d1;
/*  576 */     double d10 = d3;
/*  577 */     double d11 = d3;
/*  578 */     if (j == 0) {
/*      */       
/*  580 */       d5 = d;
/*  581 */       d6 = d3;
/*  582 */       d8 = d1;
/*  583 */       d11 = d2;
/*      */     }
/*  585 */     else if (j == 2) {
/*      */       
/*  587 */       d4 = d1;
/*  588 */       d7 = d3;
/*  589 */       d9 = d;
/*  590 */       d10 = d2;
/*      */     }
/*  592 */     else if (j == 3) {
/*      */       
/*  594 */       d4 = d1;
/*  595 */       d7 = d3;
/*  596 */       d9 = d;
/*  597 */       d10 = d2;
/*  598 */       d5 = d;
/*  599 */       d6 = d3;
/*  600 */       d8 = d1;
/*  601 */       d11 = d2;
/*      */     } 
/*  603 */     double d12 = par2 + par1Block.bV;
/*  604 */     double d13 = par2 + par1Block.bY;
/*  605 */     double d14 = par3 + par1Block.bZ;
/*  606 */     double d15 = par4 + par1Block.bX;
/*  607 */     double d16 = par4 + par1Block.ca;
/*  608 */     tessellator.a(d13, d14, d16, d8, d10);
/*  609 */     tessellator.a(d13, d14, d15, d4, d6);
/*  610 */     tessellator.a(d12, d14, d15, d5, d7);
/*  611 */     tessellator.a(d12, d14, d16, d9, d11);
/*  612 */     l = aav.c[j];
/*  613 */     if (flag)
/*      */     {
/*  615 */       l = aav.c[aav.e[j]];
/*      */     }
/*  617 */     i1 = 4;
/*  618 */     switch (j) {
/*      */       
/*      */       case 0:
/*  621 */         i1 = 5;
/*      */         break;
/*      */       
/*      */       case 3:
/*  625 */         i1 = 2;
/*      */         break;
/*      */       
/*      */       case 1:
/*  629 */         i1 = 3;
/*      */         break;
/*      */     } 
/*  632 */     if (l != 2 && (this.f || par1Block.a(this.a, par2, par3, par4 - 1, 2))) {
/*      */       
/*  634 */       tessellator.b((par1Block.bX <= 0.0D) ? par1Block.d(this.a, par2, par3, par4 - 1) : k);
/*  635 */       tessellator.a(f8, f11, f14);
/*  636 */       this.e = (i1 == 2);
/*  637 */       c(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 2));
/*      */     } 
/*  639 */     if (l != 3 && (this.f || par1Block.a(this.a, par2, par3, par4 + 1, 3))) {
/*      */       
/*  641 */       tessellator.b((par1Block.ca >= 1.0D) ? par1Block.d(this.a, par2, par3, par4 + 1) : k);
/*  642 */       tessellator.a(f8, f11, f14);
/*  643 */       this.e = (i1 == 3);
/*  644 */       d(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 3));
/*      */     } 
/*  646 */     if (l != 4 && (this.f || par1Block.a(this.a, par2 - 1, par3, par4, 4))) {
/*      */       
/*  648 */       tessellator.b((par1Block.bX <= 0.0D) ? par1Block.d(this.a, par2 - 1, par3, par4) : k);
/*  649 */       tessellator.a(f9, f12, f15);
/*  650 */       this.e = (i1 == 4);
/*  651 */       e(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 4));
/*      */     } 
/*  653 */     if (l != 5 && (this.f || par1Block.a(this.a, par2 + 1, par3, par4, 5))) {
/*      */       
/*  655 */       tessellator.b((par1Block.ca >= 1.0D) ? par1Block.d(this.a, par2 + 1, par3, par4) : k);
/*  656 */       tessellator.a(f9, f12, f15);
/*  657 */       this.e = (i1 == 5);
/*  658 */       f(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 5));
/*      */     } 
/*  660 */     this.e = false;
/*  661 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean a(xc par1BlockBrewingStand, int par2, int par3, int par4) {
/*  670 */     par1BlockBrewingStand.a(0.4375F, 0.0F, 0.4375F, 0.5625F, 0.875F, 0.5625F);
/*  671 */     o((pb)par1BlockBrewingStand, par2, par3, par4);
/*  672 */     this.d = 156;
/*  673 */     par1BlockBrewingStand.a(0.5625F, 0.0F, 0.3125F, 0.9375F, 0.125F, 0.6875F);
/*  674 */     o((pb)par1BlockBrewingStand, par2, par3, par4);
/*  675 */     par1BlockBrewingStand.a(0.125F, 0.0F, 0.0625F, 0.5F, 0.125F, 0.4375F);
/*  676 */     o((pb)par1BlockBrewingStand, par2, par3, par4);
/*  677 */     par1BlockBrewingStand.a(0.125F, 0.0F, 0.5625F, 0.5F, 0.125F, 0.9375F);
/*  678 */     o((pb)par1BlockBrewingStand, par2, par3, par4);
/*  679 */     a();
/*  680 */     adz tessellator = adz.a;
/*  681 */     tessellator.b(par1BlockBrewingStand.d(this.a, par2, par3, par4));
/*  682 */     float f = 1.0F;
/*  683 */     int i = par1BlockBrewingStand.c(this.a, par2, par3, par4);
/*  684 */     float f1 = (i >> 16 & 0xFF) / 255.0F;
/*  685 */     float f2 = (i >> 8 & 0xFF) / 255.0F;
/*  686 */     float f3 = (i & 0xFF) / 255.0F;
/*  687 */     if (lr.a) {
/*      */       
/*  689 */       float f4 = (f1 * 30.0F + f2 * 59.0F + f3 * 11.0F) / 100.0F;
/*  690 */       float f5 = (f1 * 30.0F + f2 * 70.0F) / 100.0F;
/*  691 */       float f6 = (f1 * 30.0F + f3 * 70.0F) / 100.0F;
/*  692 */       f1 = f4;
/*  693 */       f2 = f5;
/*  694 */       f3 = f6;
/*      */     } 
/*  696 */     tessellator.a(f * f1, f * f2, f * f3);
/*  697 */     int j = par1BlockBrewingStand.a(0, 0);
/*  698 */     if (this.d >= 0)
/*      */     {
/*  700 */       j = this.d;
/*      */     }
/*  702 */     int k = (j & 0xF) << 4;
/*  703 */     int l = j & 0xF0;
/*  704 */     double d = (l / 256.0F);
/*  705 */     double d1 = ((l + 15.99F) / 256.0F);
/*  706 */     int i1 = this.a.e(par2, par3, par4);
/*  707 */     for (int j1 = 0; j1 < 3; j1++) {
/*      */       
/*  709 */       double d2 = j1 * Math.PI * 2.0D / 3.0D + 1.5707963267948966D;
/*  710 */       double d3 = ((k + 8.0F) / 256.0F);
/*  711 */       double d4 = ((k + 15.99F) / 256.0F);
/*  712 */       if ((i1 & 1 << j1) != 0) {
/*      */         
/*  714 */         d3 = ((k + 7.99F) / 256.0F);
/*  715 */         d4 = ((k + 0.0F) / 256.0F);
/*      */       } 
/*  717 */       double d5 = par2 + 0.5D;
/*  718 */       double d6 = par2 + 0.5D + Math.sin(d2) * 8.0D / 16.0D;
/*  719 */       double d7 = par4 + 0.5D;
/*  720 */       double d8 = par4 + 0.5D + Math.cos(d2) * 8.0D / 16.0D;
/*  721 */       tessellator.a(d5, (par3 + 1), d7, d3, d);
/*  722 */       tessellator.a(d5, (par3 + 0), d7, d3, d1);
/*  723 */       tessellator.a(d6, (par3 + 0), d8, d4, d1);
/*  724 */       tessellator.a(d6, (par3 + 1), d8, d4, d);
/*  725 */       tessellator.a(d6, (par3 + 1), d8, d4, d);
/*  726 */       tessellator.a(d6, (par3 + 0), d8, d4, d1);
/*  727 */       tessellator.a(d5, (par3 + 0), d7, d3, d1);
/*  728 */       tessellator.a(d5, (par3 + 1), d7, d3, d);
/*      */     } 
/*      */     
/*  731 */     par1BlockBrewingStand.h();
/*  732 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean a(xm par1BlockCauldron, int par2, int par3, int par4) {
/*  741 */     o((pb)par1BlockCauldron, par2, par3, par4);
/*  742 */     adz tessellator = adz.a;
/*  743 */     tessellator.b(par1BlockCauldron.d(this.a, par2, par3, par4));
/*  744 */     float f = 1.0F;
/*  745 */     int i = par1BlockCauldron.c(this.a, par2, par3, par4);
/*  746 */     float f1 = (i >> 16 & 0xFF) / 255.0F;
/*  747 */     float f2 = (i >> 8 & 0xFF) / 255.0F;
/*  748 */     float f3 = (i & 0xFF) / 255.0F;
/*  749 */     if (lr.a) {
/*      */       
/*  751 */       float f4 = (f1 * 30.0F + f2 * 59.0F + f3 * 11.0F) / 100.0F;
/*  752 */       float f5 = (f1 * 30.0F + f2 * 70.0F) / 100.0F;
/*  753 */       float f7 = (f1 * 30.0F + f3 * 70.0F) / 100.0F;
/*  754 */       f1 = f4;
/*  755 */       f2 = f5;
/*  756 */       f3 = f7;
/*      */     } 
/*  758 */     tessellator.a(f * f1, f * f2, f * f3);
/*  759 */     char c = '';
/*  760 */     float f6 = 0.125F;
/*  761 */     f((pb)par1BlockCauldron, (par2 - 1.0F + f6), par3, par4, c);
/*  762 */     e((pb)par1BlockCauldron, (par2 + 1.0F - f6), par3, par4, c);
/*  763 */     d((pb)par1BlockCauldron, par2, par3, (par4 - 1.0F + f6), c);
/*  764 */     c((pb)par1BlockCauldron, par2, par3, (par4 + 1.0F - f6), c);
/*  765 */     char c1 = '';
/*  766 */     b((pb)par1BlockCauldron, par2, (par3 - 1.0F + 0.25F), par4, c1);
/*  767 */     a((pb)par1BlockCauldron, par2, (par3 + 1.0F - 0.75F), par4, c1);
/*  768 */     int j = this.a.e(par2, par3, par4);
/*  769 */     if (j > 0) {
/*      */       
/*  771 */       char c2 = 'Í';
/*  772 */       if (j > 3)
/*      */       {
/*  774 */         j = 3;
/*      */       }
/*      */       
/*  777 */       int wc = CustomColorizer.getFluidColor(pb.B, this.a, par2, par3, par4);
/*      */       
/*  779 */       float wr = (wc >> 16 & 0xFF) / 255.0F;
/*  780 */       float wg = (wc >> 8 & 0xFF) / 255.0F;
/*  781 */       float wb = (wc & 0xFF) / 255.0F;
/*  782 */       tessellator.a(wr, wg, wb);
/*      */       
/*  784 */       b((pb)par1BlockCauldron, par2, (par3 - 1.0F + (6.0F + j * 3.0F) / 16.0F), par4, c2);
/*      */     } 
/*  786 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean c(pb par1Block, int par2, int par3, int par4) {
/*  794 */     int i = this.a.e(par2, par3, par4);
/*  795 */     adz tessellator = adz.a;
/*  796 */     tessellator.b(par1Block.d(this.a, par2, par3, par4));
/*  797 */     tessellator.a(1.0F, 1.0F, 1.0F);
/*  798 */     double d = 0.4000000059604645D;
/*  799 */     double d1 = 0.5D - d;
/*  800 */     double d2 = 0.20000000298023224D;
/*  801 */     if (i == 1) {
/*      */       
/*  803 */       a(par1Block, par2 - d1, par3 + d2, par4, -d, 0.0D);
/*      */     }
/*  805 */     else if (i == 2) {
/*      */       
/*  807 */       a(par1Block, par2 + d1, par3 + d2, par4, d, 0.0D);
/*      */     }
/*  809 */     else if (i == 3) {
/*      */       
/*  811 */       a(par1Block, par2, par3 + d2, par4 - d1, 0.0D, -d);
/*      */     }
/*  813 */     else if (i == 4) {
/*      */       
/*  815 */       a(par1Block, par2, par3 + d2, par4 + d1, 0.0D, d);
/*      */     } else {
/*      */       
/*  818 */       a(par1Block, par2, par3, par4, 0.0D, 0.0D);
/*      */       
/*  820 */       if (par1Block != pb.aq)
/*      */       {
/*  822 */         if (Config.isBetterSnow())
/*      */         {
/*  824 */           if (hasSnowNeighbours(par2, par3, par4))
/*  825 */             o(pb.aS, par2, par3, par4); 
/*      */         }
/*      */       }
/*      */     } 
/*  829 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean u(pb par1Block, int par2, int par3, int par4) {
/*  838 */     int i = this.a.e(par2, par3, par4);
/*  839 */     int j = i & 0x3;
/*  840 */     int k = (i & 0xC) >> 2;
/*  841 */     o(par1Block, par2, par3, par4);
/*  842 */     adz tessellator = adz.a;
/*  843 */     tessellator.b(par1Block.d(this.a, par2, par3, par4));
/*  844 */     tessellator.a(1.0F, 1.0F, 1.0F);
/*  845 */     double d = -0.1875D;
/*  846 */     double d1 = 0.0D;
/*  847 */     double d2 = 0.0D;
/*  848 */     double d3 = 0.0D;
/*  849 */     double d4 = 0.0D;
/*  850 */     switch (j) {
/*      */       
/*      */       case 0:
/*  853 */         d4 = -0.3125D;
/*  854 */         d2 = qx.a[k];
/*      */         break;
/*      */       
/*      */       case 2:
/*  858 */         d4 = 0.3125D;
/*  859 */         d2 = -qx.a[k];
/*      */         break;
/*      */       
/*      */       case 3:
/*  863 */         d3 = -0.3125D;
/*  864 */         d1 = qx.a[k];
/*      */         break;
/*      */       
/*      */       case 1:
/*  868 */         d3 = 0.3125D;
/*  869 */         d1 = -qx.a[k];
/*      */         break;
/*      */     } 
/*  872 */     a(par1Block, par2 + d1, par3 + d, par4 + d2, 0.0D, 0.0D);
/*  873 */     a(par1Block, par2 + d3, par3 + d, par4 + d4, 0.0D, 0.0D);
/*  874 */     int l = par1Block.a_(1);
/*  875 */     int i1 = (l & 0xF) << 4;
/*  876 */     int j1 = l & 0xF0;
/*  877 */     double d5 = (i1 / 256.0F);
/*  878 */     double d6 = ((i1 + 15.99F) / 256.0F);
/*  879 */     double d7 = (j1 / 256.0F);
/*  880 */     double d8 = ((j1 + 15.99F) / 256.0F);
/*  881 */     double d9 = 0.125D;
/*  882 */     double d10 = (par2 + 1);
/*  883 */     double d11 = (par2 + 1);
/*  884 */     double d12 = (par2 + 0);
/*  885 */     double d13 = (par2 + 0);
/*  886 */     double d14 = (par4 + 0);
/*  887 */     double d15 = (par4 + 1);
/*  888 */     double d16 = (par4 + 1);
/*  889 */     double d17 = (par4 + 0);
/*  890 */     double d18 = par3 + d9;
/*      */ 
/*      */     
/*  893 */     d10 = d11 = (par2 + 0);
/*  894 */     d12 = d13 = (par2 + 1);
/*  895 */     d14 = d17 = (par4 + 1);
/*  896 */     d15 = d16 = (par4 + 0);
/*      */ 
/*      */ 
/*      */     
/*  900 */     d10 = d13 = (par2 + 0);
/*  901 */     d11 = d12 = (par2 + 1);
/*  902 */     d14 = d15 = (par4 + 0);
/*  903 */     d16 = d17 = (par4 + 1);
/*      */     
/*  905 */     if (j == 1) {
/*      */       
/*  907 */       d10 = d13 = (par2 + 1);
/*  908 */       d11 = d12 = (par2 + 0);
/*  909 */       d14 = d15 = (par4 + 1);
/*  910 */       d16 = d17 = (par4 + 0);
/*      */     } 
/*  912 */     tessellator.a(d13, d18, d17, d5, d7);
/*  913 */     tessellator.a(d12, d18, d16, d5, d8);
/*  914 */     tessellator.a(d11, d18, d15, d6, d8);
/*  915 */     tessellator.a(d10, d18, d14, d6, d7);
/*  916 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void d(pb par1Block, int par2, int par3, int par4) {
/*  924 */     this.f = true;
/*  925 */     b(par1Block, par2, par3, par4, true);
/*  926 */     this.f = false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean b(pb par1Block, int par2, int par3, int par4, boolean par5) {
/*  935 */     int i = this.a.e(par2, par3, par4);
/*  936 */     boolean flag = (par5 || (i & 0x8) != 0);
/*  937 */     int j = abc.e(i);
/*  938 */     if (flag) {
/*      */       
/*  940 */       switch (j) {
/*      */         
/*      */         case 0:
/*  943 */           this.g = 3;
/*  944 */           this.h = 3;
/*  945 */           this.i = 3;
/*  946 */           this.j = 3;
/*  947 */           par1Block.a(0.0F, 0.25F, 0.0F, 1.0F, 1.0F, 1.0F);
/*      */           break;
/*      */         
/*      */         case 1:
/*  951 */           par1Block.a(0.0F, 0.0F, 0.0F, 1.0F, 0.75F, 1.0F);
/*      */           break;
/*      */         
/*      */         case 2:
/*  955 */           this.i = 1;
/*  956 */           this.j = 2;
/*  957 */           par1Block.a(0.0F, 0.0F, 0.25F, 1.0F, 1.0F, 1.0F);
/*      */           break;
/*      */         
/*      */         case 3:
/*  961 */           this.i = 2;
/*  962 */           this.j = 1;
/*  963 */           this.k = 3;
/*  964 */           this.l = 3;
/*  965 */           par1Block.a(0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 0.75F);
/*      */           break;
/*      */         
/*      */         case 4:
/*  969 */           this.g = 1;
/*  970 */           this.h = 2;
/*  971 */           this.k = 2;
/*  972 */           this.l = 1;
/*  973 */           par1Block.a(0.25F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F);
/*      */           break;
/*      */         
/*      */         case 5:
/*  977 */           this.g = 2;
/*  978 */           this.h = 1;
/*  979 */           this.k = 1;
/*  980 */           this.l = 2;
/*  981 */           par1Block.a(0.0F, 0.0F, 0.0F, 0.75F, 1.0F, 1.0F);
/*      */           break;
/*      */       } 
/*  984 */       o(par1Block, par2, par3, par4);
/*  985 */       this.g = 0;
/*  986 */       this.h = 0;
/*  987 */       this.i = 0;
/*  988 */       this.j = 0;
/*  989 */       this.k = 0;
/*  990 */       this.l = 0;
/*  991 */       par1Block.a(0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F);
/*      */     } else {
/*      */       
/*  994 */       switch (j) {
/*      */         
/*      */         case 0:
/*  997 */           this.g = 3;
/*  998 */           this.h = 3;
/*  999 */           this.i = 3;
/* 1000 */           this.j = 3;
/*      */           break;
/*      */         
/*      */         case 2:
/* 1004 */           this.i = 1;
/* 1005 */           this.j = 2;
/*      */           break;
/*      */         
/*      */         case 3:
/* 1009 */           this.i = 2;
/* 1010 */           this.j = 1;
/* 1011 */           this.k = 3;
/* 1012 */           this.l = 3;
/*      */           break;
/*      */         
/*      */         case 4:
/* 1016 */           this.g = 1;
/* 1017 */           this.h = 2;
/* 1018 */           this.k = 2;
/* 1019 */           this.l = 1;
/*      */           break;
/*      */         
/*      */         case 5:
/* 1023 */           this.g = 2;
/* 1024 */           this.h = 1;
/* 1025 */           this.k = 1;
/* 1026 */           this.l = 2;
/*      */           break;
/*      */       } 
/* 1029 */       o(par1Block, par2, par3, par4);
/* 1030 */       this.g = 0;
/* 1031 */       this.h = 0;
/* 1032 */       this.i = 0;
/* 1033 */       this.j = 0;
/* 1034 */       this.k = 0;
/* 1035 */       this.l = 0;
/*      */     } 
/* 1037 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(double par1, double par3, double par5, double par7, double par9, double par11, float par13, double par14) {
/* 1046 */     int i = 108;
/* 1047 */     if (this.d >= 0)
/*      */     {
/* 1049 */       i = this.d;
/*      */     }
/* 1051 */     int j = (i & 0xF) << 4;
/* 1052 */     int k = i & 0xF0;
/* 1053 */     adz tessellator = adz.a;
/* 1054 */     double d = ((j + 0) / 256.0F);
/* 1055 */     double d1 = ((k + 0) / 256.0F);
/* 1056 */     double d2 = (j + par14 - 0.01D) / 256.0D;
/* 1057 */     double d3 = ((k + 4.0F) - 0.01D) / 256.0D;
/* 1058 */     tessellator.a(par13, par13, par13);
/* 1059 */     tessellator.a(par1, par7, par9, d2, d1);
/* 1060 */     tessellator.a(par1, par5, par9, d, d1);
/* 1061 */     tessellator.a(par3, par5, par11, d, d3);
/* 1062 */     tessellator.a(par3, par7, par11, d2, d3);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void b(double par1, double par3, double par5, double par7, double par9, double par11, float par13, double par14) {
/* 1071 */     int i = 108;
/* 1072 */     if (this.d >= 0)
/*      */     {
/* 1074 */       i = this.d;
/*      */     }
/* 1076 */     int j = (i & 0xF) << 4;
/* 1077 */     int k = i & 0xF0;
/* 1078 */     adz tessellator = adz.a;
/* 1079 */     double d = ((j + 0) / 256.0F);
/* 1080 */     double d1 = ((k + 0) / 256.0F);
/* 1081 */     double d2 = (j + par14 - 0.01D) / 256.0D;
/* 1082 */     double d3 = ((k + 4.0F) - 0.01D) / 256.0D;
/* 1083 */     tessellator.a(par13, par13, par13);
/* 1084 */     tessellator.a(par1, par5, par11, d2, d1);
/* 1085 */     tessellator.a(par1, par5, par9, d, d1);
/* 1086 */     tessellator.a(par3, par7, par9, d, d3);
/* 1087 */     tessellator.a(par3, par7, par11, d2, d3);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void c(double par1, double par3, double par5, double par7, double par9, double par11, float par13, double par14) {
/* 1096 */     int i = 108;
/* 1097 */     if (this.d >= 0)
/*      */     {
/* 1099 */       i = this.d;
/*      */     }
/* 1101 */     int j = (i & 0xF) << 4;
/* 1102 */     int k = i & 0xF0;
/* 1103 */     adz tessellator = adz.a;
/* 1104 */     double d = ((j + 0) / 256.0F);
/* 1105 */     double d1 = ((k + 0) / 256.0F);
/* 1106 */     double d2 = (j + par14 - 0.01D) / 256.0D;
/* 1107 */     double d3 = ((k + 4.0F) - 0.01D) / 256.0D;
/* 1108 */     tessellator.a(par13, par13, par13);
/* 1109 */     tessellator.a(par3, par5, par9, d2, d1);
/* 1110 */     tessellator.a(par1, par5, par9, d, d1);
/* 1111 */     tessellator.a(par1, par7, par11, d, d3);
/* 1112 */     tessellator.a(par3, par7, par11, d2, d3);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(pb par1Block, int par2, int par3, int par4, boolean par5) {
/* 1120 */     this.f = true;
/* 1121 */     c(par1Block, par2, par3, par4, par5);
/* 1122 */     this.f = false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean c(pb par1Block, int par2, int par3, int par4, boolean par5) {
/* 1131 */     int i = this.a.e(par2, par3, par4);
/* 1132 */     int j = sq.h(i);
/* 1133 */     float f = par1Block.e(this.a, par2, par3, par4);
/* 1134 */     float f1 = par5 ? 1.0F : 0.5F;
/* 1135 */     double d = par5 ? 16.0D : 8.0D;
/* 1136 */     switch (j) {
/*      */       
/*      */       case 0:
/* 1139 */         this.g = 3;
/* 1140 */         this.h = 3;
/* 1141 */         this.i = 3;
/* 1142 */         this.j = 3;
/* 1143 */         par1Block.a(0.0F, 0.0F, 0.0F, 1.0F, 0.25F, 1.0F);
/* 1144 */         o(par1Block, par2, par3, par4);
/* 1145 */         a((par2 + 0.375F), (par2 + 0.625F), (par3 + 0.25F), (par3 + 0.25F + f1), (par4 + 0.625F), (par4 + 0.625F), f * 0.8F, d);
/* 1146 */         a((par2 + 0.625F), (par2 + 0.375F), (par3 + 0.25F), (par3 + 0.25F + f1), (par4 + 0.375F), (par4 + 0.375F), f * 0.8F, d);
/* 1147 */         a((par2 + 0.375F), (par2 + 0.375F), (par3 + 0.25F), (par3 + 0.25F + f1), (par4 + 0.375F), (par4 + 0.625F), f * 0.6F, d);
/* 1148 */         a((par2 + 0.625F), (par2 + 0.625F), (par3 + 0.25F), (par3 + 0.25F + f1), (par4 + 0.625F), (par4 + 0.375F), f * 0.6F, d);
/*      */         break;
/*      */       
/*      */       case 1:
/* 1152 */         par1Block.a(0.0F, 0.75F, 0.0F, 1.0F, 1.0F, 1.0F);
/* 1153 */         o(par1Block, par2, par3, par4);
/* 1154 */         a((par2 + 0.375F), (par2 + 0.625F), (par3 - 0.25F + 1.0F - f1), (par3 - 0.25F + 1.0F), (par4 + 0.625F), (par4 + 0.625F), f * 0.8F, d);
/* 1155 */         a((par2 + 0.625F), (par2 + 0.375F), (par3 - 0.25F + 1.0F - f1), (par3 - 0.25F + 1.0F), (par4 + 0.375F), (par4 + 0.375F), f * 0.8F, d);
/* 1156 */         a((par2 + 0.375F), (par2 + 0.375F), (par3 - 0.25F + 1.0F - f1), (par3 - 0.25F + 1.0F), (par4 + 0.375F), (par4 + 0.625F), f * 0.6F, d);
/* 1157 */         a((par2 + 0.625F), (par2 + 0.625F), (par3 - 0.25F + 1.0F - f1), (par3 - 0.25F + 1.0F), (par4 + 0.625F), (par4 + 0.375F), f * 0.6F, d);
/*      */         break;
/*      */       
/*      */       case 2:
/* 1161 */         this.i = 1;
/* 1162 */         this.j = 2;
/* 1163 */         par1Block.a(0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 0.25F);
/* 1164 */         o(par1Block, par2, par3, par4);
/* 1165 */         b((par2 + 0.375F), (par2 + 0.375F), (par3 + 0.625F), (par3 + 0.375F), (par4 + 0.25F), (par4 + 0.25F + f1), f * 0.6F, d);
/* 1166 */         b((par2 + 0.625F), (par2 + 0.625F), (par3 + 0.375F), (par3 + 0.625F), (par4 + 0.25F), (par4 + 0.25F + f1), f * 0.6F, d);
/* 1167 */         b((par2 + 0.375F), (par2 + 0.625F), (par3 + 0.375F), (par3 + 0.375F), (par4 + 0.25F), (par4 + 0.25F + f1), f * 0.5F, d);
/* 1168 */         b((par2 + 0.625F), (par2 + 0.375F), (par3 + 0.625F), (par3 + 0.625F), (par4 + 0.25F), (par4 + 0.25F + f1), f, d);
/*      */         break;
/*      */       
/*      */       case 3:
/* 1172 */         this.i = 2;
/* 1173 */         this.j = 1;
/* 1174 */         this.k = 3;
/* 1175 */         this.l = 3;
/* 1176 */         par1Block.a(0.0F, 0.0F, 0.75F, 1.0F, 1.0F, 1.0F);
/* 1177 */         o(par1Block, par2, par3, par4);
/* 1178 */         b((par2 + 0.375F), (par2 + 0.375F), (par3 + 0.625F), (par3 + 0.375F), (par4 - 0.25F + 1.0F - f1), (par4 - 0.25F + 1.0F), f * 0.6F, d);
/* 1179 */         b((par2 + 0.625F), (par2 + 0.625F), (par3 + 0.375F), (par3 + 0.625F), (par4 - 0.25F + 1.0F - f1), (par4 - 0.25F + 1.0F), f * 0.6F, d);
/* 1180 */         b((par2 + 0.375F), (par2 + 0.625F), (par3 + 0.375F), (par3 + 0.375F), (par4 - 0.25F + 1.0F - f1), (par4 - 0.25F + 1.0F), f * 0.5F, d);
/* 1181 */         b((par2 + 0.625F), (par2 + 0.375F), (par3 + 0.625F), (par3 + 0.625F), (par4 - 0.25F + 1.0F - f1), (par4 - 0.25F + 1.0F), f, d);
/*      */         break;
/*      */       
/*      */       case 4:
/* 1185 */         this.g = 1;
/* 1186 */         this.h = 2;
/* 1187 */         this.k = 2;
/* 1188 */         this.l = 1;
/* 1189 */         par1Block.a(0.0F, 0.0F, 0.0F, 0.25F, 1.0F, 1.0F);
/* 1190 */         o(par1Block, par2, par3, par4);
/* 1191 */         c((par2 + 0.25F), (par2 + 0.25F + f1), (par3 + 0.375F), (par3 + 0.375F), (par4 + 0.625F), (par4 + 0.375F), f * 0.5F, d);
/* 1192 */         c((par2 + 0.25F), (par2 + 0.25F + f1), (par3 + 0.625F), (par3 + 0.625F), (par4 + 0.375F), (par4 + 0.625F), f, d);
/* 1193 */         c((par2 + 0.25F), (par2 + 0.25F + f1), (par3 + 0.375F), (par3 + 0.625F), (par4 + 0.375F), (par4 + 0.375F), f * 0.6F, d);
/* 1194 */         c((par2 + 0.25F), (par2 + 0.25F + f1), (par3 + 0.625F), (par3 + 0.375F), (par4 + 0.625F), (par4 + 0.625F), f * 0.6F, d);
/*      */         break;
/*      */       
/*      */       case 5:
/* 1198 */         this.g = 2;
/* 1199 */         this.h = 1;
/* 1200 */         this.k = 1;
/* 1201 */         this.l = 2;
/* 1202 */         par1Block.a(0.75F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F);
/* 1203 */         o(par1Block, par2, par3, par4);
/* 1204 */         c((par2 - 0.25F + 1.0F - f1), (par2 - 0.25F + 1.0F), (par3 + 0.375F), (par3 + 0.375F), (par4 + 0.625F), (par4 + 0.375F), f * 0.5F, d);
/* 1205 */         c((par2 - 0.25F + 1.0F - f1), (par2 - 0.25F + 1.0F), (par3 + 0.625F), (par3 + 0.625F), (par4 + 0.375F), (par4 + 0.625F), f, d);
/* 1206 */         c((par2 - 0.25F + 1.0F - f1), (par2 - 0.25F + 1.0F), (par3 + 0.375F), (par3 + 0.625F), (par4 + 0.375F), (par4 + 0.375F), f * 0.6F, d);
/* 1207 */         c((par2 - 0.25F + 1.0F - f1), (par2 - 0.25F + 1.0F), (par3 + 0.625F), (par3 + 0.375F), (par4 + 0.625F), (par4 + 0.625F), f * 0.6F, d);
/*      */         break;
/*      */     } 
/* 1210 */     this.g = 0;
/* 1211 */     this.h = 0;
/* 1212 */     this.i = 0;
/* 1213 */     this.j = 0;
/* 1214 */     this.k = 0;
/* 1215 */     this.l = 0;
/* 1216 */     par1Block.a(0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F);
/* 1217 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean e(pb par1Block, int par2, int par3, int par4) {
/* 1225 */     int i = this.a.e(par2, par3, par4);
/* 1226 */     int j = i & 0x7;
/* 1227 */     boolean flag = ((i & 0x8) > 0);
/* 1228 */     adz tessellator = adz.a;
/* 1229 */     boolean flag1 = (this.d >= 0);
/* 1230 */     if (!flag1)
/*      */     {
/* 1232 */       this.d = pb.w.bN;
/*      */     }
/* 1234 */     float f = 0.25F;
/* 1235 */     float f1 = 0.1875F;
/* 1236 */     float f2 = 0.1875F;
/* 1237 */     if (j == 5) {
/*      */       
/* 1239 */       par1Block.a(0.5F - f1, 0.0F, 0.5F - f, 0.5F + f1, f2, 0.5F + f);
/*      */     }
/* 1241 */     else if (j == 6) {
/*      */       
/* 1243 */       par1Block.a(0.5F - f, 0.0F, 0.5F - f1, 0.5F + f, f2, 0.5F + f1);
/*      */     }
/* 1245 */     else if (j == 4) {
/*      */       
/* 1247 */       par1Block.a(0.5F - f1, 0.5F - f, 1.0F - f2, 0.5F + f1, 0.5F + f, 1.0F);
/*      */     }
/* 1249 */     else if (j == 3) {
/*      */       
/* 1251 */       par1Block.a(0.5F - f1, 0.5F - f, 0.0F, 0.5F + f1, 0.5F + f, f2);
/*      */     }
/* 1253 */     else if (j == 2) {
/*      */       
/* 1255 */       par1Block.a(1.0F - f2, 0.5F - f, 0.5F - f1, 1.0F, 0.5F + f, 0.5F + f1);
/*      */     }
/* 1257 */     else if (j == 1) {
/*      */       
/* 1259 */       par1Block.a(0.0F, 0.5F - f, 0.5F - f1, f2, 0.5F + f, 0.5F + f1);
/*      */     } 
/* 1261 */     o(par1Block, par2, par3, par4);
/* 1262 */     if (!flag1)
/*      */     {
/* 1264 */       this.d = -1;
/*      */     }
/* 1266 */     tessellator.b(par1Block.d(this.a, par2, par3, par4));
/* 1267 */     float f3 = 1.0F;
/* 1268 */     if (pb.q[par1Block.bO] > 0)
/*      */     {
/* 1270 */       f3 = 1.0F;
/*      */     }
/* 1272 */     tessellator.a(f3, f3, f3);
/* 1273 */     int k = par1Block.a_(0);
/* 1274 */     if (this.d >= 0)
/*      */     {
/* 1276 */       k = this.d;
/*      */     }
/* 1278 */     int l = (k & 0xF) << 4;
/* 1279 */     int i1 = k & 0xF0;
/* 1280 */     float f4 = l / 256.0F;
/* 1281 */     float f5 = (l + 15.99F) / 256.0F;
/* 1282 */     float f6 = i1 / 256.0F;
/* 1283 */     float f7 = (i1 + 15.99F) / 256.0F;
/* 1284 */     bo[] avec3d = new bo[8];
/* 1285 */     float f8 = 0.0625F;
/* 1286 */     float f9 = 0.0625F;
/* 1287 */     float f10 = 0.625F;
/* 1288 */     avec3d[0] = bo.b(-f8, 0.0D, -f9);
/* 1289 */     avec3d[1] = bo.b(f8, 0.0D, -f9);
/* 1290 */     avec3d[2] = bo.b(f8, 0.0D, f9);
/* 1291 */     avec3d[3] = bo.b(-f8, 0.0D, f9);
/* 1292 */     avec3d[4] = bo.b(-f8, f10, -f9);
/* 1293 */     avec3d[5] = bo.b(f8, f10, -f9);
/* 1294 */     avec3d[6] = bo.b(f8, f10, f9);
/* 1295 */     avec3d[7] = bo.b(-f8, f10, f9);
/* 1296 */     for (int j1 = 0; j1 < 8; j1++) {
/*      */       
/* 1298 */       if (flag) {
/*      */         
/* 1300 */         (avec3d[j1]).c -= 0.0625D;
/* 1301 */         avec3d[j1].a(0.69813174F);
/*      */       } else {
/*      */         
/* 1304 */         (avec3d[j1]).c += 0.0625D;
/* 1305 */         avec3d[j1].a(-0.69813174F);
/*      */       } 
/* 1307 */       if (j == 6)
/*      */       {
/* 1309 */         avec3d[j1].b(1.5707964F);
/*      */       }
/* 1311 */       if (j < 5) {
/*      */         
/* 1313 */         (avec3d[j1]).b -= 0.375D;
/* 1314 */         avec3d[j1].a(1.5707964F);
/* 1315 */         if (j == 4)
/*      */         {
/* 1317 */           avec3d[j1].b(0.0F);
/*      */         }
/* 1319 */         if (j == 3)
/*      */         {
/* 1321 */           avec3d[j1].b(3.1415927F);
/*      */         }
/* 1323 */         if (j == 2)
/*      */         {
/* 1325 */           avec3d[j1].b(1.5707964F);
/*      */         }
/* 1327 */         if (j == 1)
/*      */         {
/* 1329 */           avec3d[j1].b(-1.5707964F);
/*      */         }
/* 1331 */         (avec3d[j1]).a += par2 + 0.5D;
/* 1332 */         (avec3d[j1]).b += (par3 + 0.5F);
/* 1333 */         (avec3d[j1]).c += par4 + 0.5D;
/*      */       } else {
/*      */         
/* 1336 */         (avec3d[j1]).a += par2 + 0.5D;
/* 1337 */         (avec3d[j1]).b += (par3 + 0.125F);
/* 1338 */         (avec3d[j1]).c += par4 + 0.5D;
/*      */       } 
/*      */     } 
/*      */     
/* 1342 */     bo vec3d = null;
/* 1343 */     bo vec3d1 = null;
/* 1344 */     bo vec3d2 = null;
/* 1345 */     bo vec3d3 = null;
/* 1346 */     for (int k1 = 0; k1 < 6; k1++) {
/*      */       
/* 1348 */       if (k1 == 0) {
/*      */         
/* 1350 */         f4 = (l + 7) / 256.0F;
/* 1351 */         f5 = ((l + 9) - 0.01F) / 256.0F;
/* 1352 */         f6 = (i1 + 6) / 256.0F;
/* 1353 */         f7 = ((i1 + 8) - 0.01F) / 256.0F;
/*      */       }
/* 1355 */       else if (k1 == 2) {
/*      */         
/* 1357 */         f4 = (l + 7) / 256.0F;
/* 1358 */         f5 = ((l + 9) - 0.01F) / 256.0F;
/* 1359 */         f6 = (i1 + 6) / 256.0F;
/* 1360 */         f7 = ((i1 + 16) - 0.01F) / 256.0F;
/*      */       } 
/* 1362 */       if (k1 == 0) {
/*      */         
/* 1364 */         vec3d = avec3d[0];
/* 1365 */         vec3d1 = avec3d[1];
/* 1366 */         vec3d2 = avec3d[2];
/* 1367 */         vec3d3 = avec3d[3];
/*      */       }
/* 1369 */       else if (k1 == 1) {
/*      */         
/* 1371 */         vec3d = avec3d[7];
/* 1372 */         vec3d1 = avec3d[6];
/* 1373 */         vec3d2 = avec3d[5];
/* 1374 */         vec3d3 = avec3d[4];
/*      */       }
/* 1376 */       else if (k1 == 2) {
/*      */         
/* 1378 */         vec3d = avec3d[1];
/* 1379 */         vec3d1 = avec3d[0];
/* 1380 */         vec3d2 = avec3d[4];
/* 1381 */         vec3d3 = avec3d[5];
/*      */       }
/* 1383 */       else if (k1 == 3) {
/*      */         
/* 1385 */         vec3d = avec3d[2];
/* 1386 */         vec3d1 = avec3d[1];
/* 1387 */         vec3d2 = avec3d[5];
/* 1388 */         vec3d3 = avec3d[6];
/*      */       }
/* 1390 */       else if (k1 == 4) {
/*      */         
/* 1392 */         vec3d = avec3d[3];
/* 1393 */         vec3d1 = avec3d[2];
/* 1394 */         vec3d2 = avec3d[6];
/* 1395 */         vec3d3 = avec3d[7];
/*      */       }
/* 1397 */       else if (k1 == 5) {
/*      */         
/* 1399 */         vec3d = avec3d[0];
/* 1400 */         vec3d1 = avec3d[3];
/* 1401 */         vec3d2 = avec3d[7];
/* 1402 */         vec3d3 = avec3d[4];
/*      */       } 
/* 1404 */       tessellator.a(vec3d.a, vec3d.b, vec3d.c, f4, f7);
/* 1405 */       tessellator.a(vec3d1.a, vec3d1.b, vec3d1.c, f5, f7);
/* 1406 */       tessellator.a(vec3d2.a, vec3d2.b, vec3d2.c, f5, f6);
/* 1407 */       tessellator.a(vec3d3.a, vec3d3.b, vec3d3.c, f4, f6);
/*      */     } 
/*      */     
/* 1410 */     if (Config.isBetterSnow())
/*      */     {
/* 1412 */       if (hasSnowNeighbours(par2, par3, par4)) {
/* 1413 */         o(pb.aS, par2, par3, par4);
/*      */       }
/*      */     }
/* 1416 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean f(pb par1Block, int par2, int par3, int par4) {
/* 1424 */     adz tessellator = adz.a;
/* 1425 */     int i = par1Block.a_(0);
/* 1426 */     if (this.d >= 0)
/*      */     {
/* 1428 */       i = this.d;
/*      */     }
/* 1430 */     tessellator.a(1.0F, 1.0F, 1.0F);
/* 1431 */     tessellator.b(par1Block.d(this.a, par2, par3, par4));
/* 1432 */     int j = (i & 0xF) << 4;
/* 1433 */     int k = i & 0xF0;
/* 1434 */     double d = (j / 256.0F);
/* 1435 */     double d2 = ((j + 15.99F) / 256.0F);
/* 1436 */     double d4 = (k / 256.0F);
/* 1437 */     double d6 = ((k + 15.99F) / 256.0F);
/* 1438 */     float f = 1.4F;
/* 1439 */     if (this.a.h(par2, par3 - 1, par4) || pb.ar.g(this.a, par2, par3 - 1, par4)) {
/*      */       
/* 1441 */       double d8 = par2 + 0.5D + 0.2D;
/* 1442 */       double d9 = par2 + 0.5D - 0.2D;
/* 1443 */       double d12 = par4 + 0.5D + 0.2D;
/* 1444 */       double d14 = par4 + 0.5D - 0.2D;
/* 1445 */       double d16 = par2 + 0.5D - 0.3D;
/* 1446 */       double d18 = par2 + 0.5D + 0.3D;
/* 1447 */       double d20 = par4 + 0.5D - 0.3D;
/* 1448 */       double d22 = par4 + 0.5D + 0.3D;
/* 1449 */       tessellator.a(d16, (par3 + f), (par4 + 1), d2, d4);
/* 1450 */       tessellator.a(d8, (par3 + 0), (par4 + 1), d2, d6);
/* 1451 */       tessellator.a(d8, (par3 + 0), (par4 + 0), d, d6);
/* 1452 */       tessellator.a(d16, (par3 + f), (par4 + 0), d, d4);
/* 1453 */       tessellator.a(d18, (par3 + f), (par4 + 0), d2, d4);
/* 1454 */       tessellator.a(d9, (par3 + 0), (par4 + 0), d2, d6);
/* 1455 */       tessellator.a(d9, (par3 + 0), (par4 + 1), d, d6);
/* 1456 */       tessellator.a(d18, (par3 + f), (par4 + 1), d, d4);
/* 1457 */       d = (j / 256.0F);
/* 1458 */       d2 = ((j + 15.99F) / 256.0F);
/* 1459 */       d4 = ((k + 16) / 256.0F);
/* 1460 */       d6 = ((k + 15.99F + 16.0F) / 256.0F);
/* 1461 */       tessellator.a((par2 + 1), (par3 + f), d22, d2, d4);
/* 1462 */       tessellator.a((par2 + 1), (par3 + 0), d14, d2, d6);
/* 1463 */       tessellator.a((par2 + 0), (par3 + 0), d14, d, d6);
/* 1464 */       tessellator.a((par2 + 0), (par3 + f), d22, d, d4);
/* 1465 */       tessellator.a((par2 + 0), (par3 + f), d20, d2, d4);
/* 1466 */       tessellator.a((par2 + 0), (par3 + 0), d12, d2, d6);
/* 1467 */       tessellator.a((par2 + 1), (par3 + 0), d12, d, d6);
/* 1468 */       tessellator.a((par2 + 1), (par3 + f), d20, d, d4);
/* 1469 */       d8 = par2 + 0.5D - 0.5D;
/* 1470 */       d9 = par2 + 0.5D + 0.5D;
/* 1471 */       d12 = par4 + 0.5D - 0.5D;
/* 1472 */       d14 = par4 + 0.5D + 0.5D;
/* 1473 */       d16 = par2 + 0.5D - 0.4D;
/* 1474 */       d18 = par2 + 0.5D + 0.4D;
/* 1475 */       d20 = par4 + 0.5D - 0.4D;
/* 1476 */       d22 = par4 + 0.5D + 0.4D;
/* 1477 */       tessellator.a(d16, (par3 + f), (par4 + 0), d, d4);
/* 1478 */       tessellator.a(d8, (par3 + 0), (par4 + 0), d, d6);
/* 1479 */       tessellator.a(d8, (par3 + 0), (par4 + 1), d2, d6);
/* 1480 */       tessellator.a(d16, (par3 + f), (par4 + 1), d2, d4);
/* 1481 */       tessellator.a(d18, (par3 + f), (par4 + 1), d, d4);
/* 1482 */       tessellator.a(d9, (par3 + 0), (par4 + 1), d, d6);
/* 1483 */       tessellator.a(d9, (par3 + 0), (par4 + 0), d2, d6);
/* 1484 */       tessellator.a(d18, (par3 + f), (par4 + 0), d2, d4);
/* 1485 */       d = (j / 256.0F);
/* 1486 */       d2 = ((j + 15.99F) / 256.0F);
/* 1487 */       d4 = (k / 256.0F);
/* 1488 */       d6 = ((k + 15.99F) / 256.0F);
/* 1489 */       tessellator.a((par2 + 0), (par3 + f), d22, d, d4);
/* 1490 */       tessellator.a((par2 + 0), (par3 + 0), d14, d, d6);
/* 1491 */       tessellator.a((par2 + 1), (par3 + 0), d14, d2, d6);
/* 1492 */       tessellator.a((par2 + 1), (par3 + f), d22, d2, d4);
/* 1493 */       tessellator.a((par2 + 1), (par3 + f), d20, d, d4);
/* 1494 */       tessellator.a((par2 + 1), (par3 + 0), d12, d, d6);
/* 1495 */       tessellator.a((par2 + 0), (par3 + 0), d12, d2, d6);
/* 1496 */       tessellator.a((par2 + 0), (par3 + f), d20, d2, d4);
/*      */     } else {
/*      */       
/* 1499 */       float f2 = 0.2F;
/* 1500 */       float f3 = 0.0625F;
/* 1501 */       if ((par2 + par3 + par4 & 0x1) == 1) {
/*      */         
/* 1503 */         d = (j / 256.0F);
/* 1504 */         d2 = ((j + 15.99F) / 256.0F);
/* 1505 */         d4 = ((k + 16) / 256.0F);
/* 1506 */         d6 = ((k + 15.99F + 16.0F) / 256.0F);
/*      */       } 
/* 1508 */       if ((par2 / 2 + par3 / 2 + par4 / 2 & 0x1) == 1) {
/*      */         
/* 1510 */         double d10 = d2;
/* 1511 */         d2 = d;
/* 1512 */         d = d10;
/*      */       } 
/* 1514 */       if (pb.ar.g(this.a, par2 - 1, par3, par4)) {
/*      */         
/* 1516 */         tessellator.a((par2 + f2), (par3 + f + f3), (par4 + 1), d2, d4);
/* 1517 */         tessellator.a((par2 + 0), ((par3 + 0) + f3), (par4 + 1), d2, d6);
/* 1518 */         tessellator.a((par2 + 0), ((par3 + 0) + f3), (par4 + 0), d, d6);
/* 1519 */         tessellator.a((par2 + f2), (par3 + f + f3), (par4 + 0), d, d4);
/* 1520 */         tessellator.a((par2 + f2), (par3 + f + f3), (par4 + 0), d, d4);
/* 1521 */         tessellator.a((par2 + 0), ((par3 + 0) + f3), (par4 + 0), d, d6);
/* 1522 */         tessellator.a((par2 + 0), ((par3 + 0) + f3), (par4 + 1), d2, d6);
/* 1523 */         tessellator.a((par2 + f2), (par3 + f + f3), (par4 + 1), d2, d4);
/*      */       } 
/* 1525 */       if (pb.ar.g(this.a, par2 + 1, par3, par4)) {
/*      */         
/* 1527 */         tessellator.a(((par2 + 1) - f2), (par3 + f + f3), (par4 + 0), d, d4);
/* 1528 */         tessellator.a((par2 + 1 - 0), ((par3 + 0) + f3), (par4 + 0), d, d6);
/* 1529 */         tessellator.a((par2 + 1 - 0), ((par3 + 0) + f3), (par4 + 1), d2, d6);
/* 1530 */         tessellator.a(((par2 + 1) - f2), (par3 + f + f3), (par4 + 1), d2, d4);
/* 1531 */         tessellator.a(((par2 + 1) - f2), (par3 + f + f3), (par4 + 1), d2, d4);
/* 1532 */         tessellator.a((par2 + 1 - 0), ((par3 + 0) + f3), (par4 + 1), d2, d6);
/* 1533 */         tessellator.a((par2 + 1 - 0), ((par3 + 0) + f3), (par4 + 0), d, d6);
/* 1534 */         tessellator.a(((par2 + 1) - f2), (par3 + f + f3), (par4 + 0), d, d4);
/*      */       } 
/* 1536 */       if (pb.ar.g(this.a, par2, par3, par4 - 1)) {
/*      */         
/* 1538 */         tessellator.a((par2 + 0), (par3 + f + f3), (par4 + f2), d2, d4);
/* 1539 */         tessellator.a((par2 + 0), ((par3 + 0) + f3), (par4 + 0), d2, d6);
/* 1540 */         tessellator.a((par2 + 1), ((par3 + 0) + f3), (par4 + 0), d, d6);
/* 1541 */         tessellator.a((par2 + 1), (par3 + f + f3), (par4 + f2), d, d4);
/* 1542 */         tessellator.a((par2 + 1), (par3 + f + f3), (par4 + f2), d, d4);
/* 1543 */         tessellator.a((par2 + 1), ((par3 + 0) + f3), (par4 + 0), d, d6);
/* 1544 */         tessellator.a((par2 + 0), ((par3 + 0) + f3), (par4 + 0), d2, d6);
/* 1545 */         tessellator.a((par2 + 0), (par3 + f + f3), (par4 + f2), d2, d4);
/*      */       } 
/* 1547 */       if (pb.ar.g(this.a, par2, par3, par4 + 1)) {
/*      */         
/* 1549 */         tessellator.a((par2 + 1), (par3 + f + f3), ((par4 + 1) - f2), d, d4);
/* 1550 */         tessellator.a((par2 + 1), ((par3 + 0) + f3), (par4 + 1 - 0), d, d6);
/* 1551 */         tessellator.a((par2 + 0), ((par3 + 0) + f3), (par4 + 1 - 0), d2, d6);
/* 1552 */         tessellator.a((par2 + 0), (par3 + f + f3), ((par4 + 1) - f2), d2, d4);
/* 1553 */         tessellator.a((par2 + 0), (par3 + f + f3), ((par4 + 1) - f2), d2, d4);
/* 1554 */         tessellator.a((par2 + 0), ((par3 + 0) + f3), (par4 + 1 - 0), d2, d6);
/* 1555 */         tessellator.a((par2 + 1), ((par3 + 0) + f3), (par4 + 1 - 0), d, d6);
/* 1556 */         tessellator.a((par2 + 1), (par3 + f + f3), ((par4 + 1) - f2), d, d4);
/*      */       } 
/* 1558 */       if (pb.ar.g(this.a, par2, par3 + 1, par4)) {
/*      */         
/* 1560 */         double d11 = par2 + 0.5D + 0.5D;
/* 1561 */         double d13 = par2 + 0.5D - 0.5D;
/* 1562 */         double d15 = par4 + 0.5D + 0.5D;
/* 1563 */         double d17 = par4 + 0.5D - 0.5D;
/* 1564 */         double d19 = par2 + 0.5D - 0.5D;
/* 1565 */         double d21 = par2 + 0.5D + 0.5D;
/* 1566 */         double d23 = par4 + 0.5D - 0.5D;
/* 1567 */         double d24 = par4 + 0.5D + 0.5D;
/* 1568 */         double d1 = (j / 256.0F);
/* 1569 */         double d3 = ((j + 15.99F) / 256.0F);
/* 1570 */         double d5 = (k / 256.0F);
/* 1571 */         double d7 = ((k + 15.99F) / 256.0F);
/* 1572 */         par3++;
/* 1573 */         float f1 = -0.2F;
/* 1574 */         if ((par2 + par3 + par4 & 0x1) == 0) {
/*      */           
/* 1576 */           tessellator.a(d19, (par3 + f1), (par4 + 0), d3, d5);
/* 1577 */           tessellator.a(d11, (par3 + 0), (par4 + 0), d3, d7);
/* 1578 */           tessellator.a(d11, (par3 + 0), (par4 + 1), d1, d7);
/* 1579 */           tessellator.a(d19, (par3 + f1), (par4 + 1), d1, d5);
/* 1580 */           d1 = (j / 256.0F);
/* 1581 */           d3 = ((j + 15.99F) / 256.0F);
/* 1582 */           d5 = ((k + 16) / 256.0F);
/* 1583 */           d7 = ((k + 15.99F + 16.0F) / 256.0F);
/* 1584 */           tessellator.a(d21, (par3 + f1), (par4 + 1), d3, d5);
/* 1585 */           tessellator.a(d13, (par3 + 0), (par4 + 1), d3, d7);
/* 1586 */           tessellator.a(d13, (par3 + 0), (par4 + 0), d1, d7);
/* 1587 */           tessellator.a(d21, (par3 + f1), (par4 + 0), d1, d5);
/*      */         } else {
/*      */           
/* 1590 */           tessellator.a((par2 + 0), (par3 + f1), d24, d3, d5);
/* 1591 */           tessellator.a((par2 + 0), (par3 + 0), d17, d3, d7);
/* 1592 */           tessellator.a((par2 + 1), (par3 + 0), d17, d1, d7);
/* 1593 */           tessellator.a((par2 + 1), (par3 + f1), d24, d1, d5);
/* 1594 */           d1 = (j / 256.0F);
/* 1595 */           d3 = ((j + 15.99F) / 256.0F);
/* 1596 */           d5 = ((k + 16) / 256.0F);
/* 1597 */           d7 = ((k + 15.99F + 16.0F) / 256.0F);
/* 1598 */           tessellator.a((par2 + 1), (par3 + f1), d23, d3, d5);
/* 1599 */           tessellator.a((par2 + 1), (par3 + 0), d15, d3, d7);
/* 1600 */           tessellator.a((par2 + 0), (par3 + 0), d15, d1, d7);
/* 1601 */           tessellator.a((par2 + 0), (par3 + f1), d23, d1, d5);
/*      */         } 
/*      */       } 
/*      */     } 
/* 1605 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean g(pb par1Block, int par2, int par3, int par4) {
/* 1613 */     adz tessellator = adz.a;
/* 1614 */     int i = this.a.e(par2, par3, par4);
/* 1615 */     int j = par1Block.a(1, i);
/* 1616 */     if (this.d >= 0)
/*      */     {
/* 1618 */       j = this.d;
/*      */     }
/* 1620 */     tessellator.b(par1Block.d(this.a, par2, par3, par4));
/* 1621 */     float f = 1.0F;
/* 1622 */     float f1 = i / 15.0F;
/* 1623 */     float f2 = f1 * 0.6F + 0.4F;
/* 1624 */     if (i == 0)
/*      */     {
/* 1626 */       f2 = 0.3F;
/*      */     }
/* 1628 */     float f3 = f1 * f1 * 0.7F - 0.5F;
/* 1629 */     float f4 = f1 * f1 * 0.6F - 0.7F;
/* 1630 */     if (f3 < 0.0F)
/*      */     {
/* 1632 */       f3 = 0.0F;
/*      */     }
/* 1634 */     if (f4 < 0.0F)
/*      */     {
/* 1636 */       f4 = 0.0F;
/*      */     }
/*      */     
/* 1639 */     int rsColor = CustomColorizer.getRedstoneColor(i);
/* 1640 */     if (rsColor != -1) {
/*      */       
/* 1642 */       int red = rsColor >> 16 & 0xFF;
/* 1643 */       int green = rsColor >> 8 & 0xFF;
/* 1644 */       int blue = rsColor & 0xFF;
/* 1645 */       f2 = red / 255.0F;
/* 1646 */       f3 = green / 255.0F;
/* 1647 */       f4 = blue / 255.0F;
/*      */     } 
/*      */     
/* 1650 */     tessellator.a(f2, f3, f4);
/* 1651 */     int k = (j & 0xF) << 4;
/* 1652 */     int l = j & 0xF0;
/* 1653 */     double d = (k / 256.0F);
/* 1654 */     double d2 = ((k + 15.99F) / 256.0F);
/* 1655 */     double d4 = (l / 256.0F);
/* 1656 */     double d6 = ((l + 15.99F) / 256.0F);
/* 1657 */     boolean flag = (ahi.e(this.a, par2 - 1, par3, par4, 1) || (!this.a.h(par2 - 1, par3, par4) && ahi.e(this.a, par2 - 1, par3 - 1, par4, -1)));
/* 1658 */     boolean flag1 = (ahi.e(this.a, par2 + 1, par3, par4, 3) || (!this.a.h(par2 + 1, par3, par4) && ahi.e(this.a, par2 + 1, par3 - 1, par4, -1)));
/* 1659 */     boolean flag2 = (ahi.e(this.a, par2, par3, par4 - 1, 2) || (!this.a.h(par2, par3, par4 - 1) && ahi.e(this.a, par2, par3 - 1, par4 - 1, -1)));
/* 1660 */     boolean flag3 = (ahi.e(this.a, par2, par3, par4 + 1, 0) || (!this.a.h(par2, par3, par4 + 1) && ahi.e(this.a, par2, par3 - 1, par4 + 1, -1)));
/* 1661 */     if (!this.a.h(par2, par3 + 1, par4)) {
/*      */       
/* 1663 */       if (this.a.h(par2 - 1, par3, par4) && ahi.e(this.a, par2 - 1, par3 + 1, par4, -1))
/*      */       {
/* 1665 */         flag = true;
/*      */       }
/* 1667 */       if (this.a.h(par2 + 1, par3, par4) && ahi.e(this.a, par2 + 1, par3 + 1, par4, -1))
/*      */       {
/* 1669 */         flag1 = true;
/*      */       }
/* 1671 */       if (this.a.h(par2, par3, par4 - 1) && ahi.e(this.a, par2, par3 + 1, par4 - 1, -1))
/*      */       {
/* 1673 */         flag2 = true;
/*      */       }
/* 1675 */       if (this.a.h(par2, par3, par4 + 1) && ahi.e(this.a, par2, par3 + 1, par4 + 1, -1))
/*      */       {
/* 1677 */         flag3 = true;
/*      */       }
/*      */     } 
/* 1680 */     float f5 = (par2 + 0);
/* 1681 */     float f6 = (par2 + 1);
/* 1682 */     float f7 = (par4 + 0);
/* 1683 */     float f8 = (par4 + 1);
/* 1684 */     byte byte0 = 0;
/* 1685 */     if ((flag || flag1) && !flag2 && !flag3)
/*      */     {
/* 1687 */       byte0 = 1;
/*      */     }
/* 1689 */     if ((flag2 || flag3) && !flag1 && !flag)
/*      */     {
/* 1691 */       byte0 = 2;
/*      */     }
/* 1693 */     if (byte0 != 0) {
/*      */       
/* 1695 */       d = ((k + 16) / 256.0F);
/* 1696 */       d2 = (((k + 16) + 15.99F) / 256.0F);
/* 1697 */       d4 = (l / 256.0F);
/* 1698 */       d6 = ((l + 15.99F) / 256.0F);
/*      */     } 
/* 1700 */     if (byte0 == 0) {
/*      */       
/* 1702 */       if (!flag)
/*      */       {
/* 1704 */         f5 += 0.3125F;
/*      */       }
/* 1706 */       if (!flag)
/*      */       {
/* 1708 */         d += 0.01953125D;
/*      */       }
/* 1710 */       if (!flag1)
/*      */       {
/* 1712 */         f6 -= 0.3125F;
/*      */       }
/* 1714 */       if (!flag1)
/*      */       {
/* 1716 */         d2 -= 0.01953125D;
/*      */       }
/* 1718 */       if (!flag2)
/*      */       {
/* 1720 */         f7 += 0.3125F;
/*      */       }
/* 1722 */       if (!flag2)
/*      */       {
/* 1724 */         d4 += 0.01953125D;
/*      */       }
/* 1726 */       if (!flag3)
/*      */       {
/* 1728 */         f8 -= 0.3125F;
/*      */       }
/* 1730 */       if (!flag3)
/*      */       {
/* 1732 */         d6 -= 0.01953125D;
/*      */       }
/* 1734 */       tessellator.a(f6, par3 + 0.015625D, f8, d2, d6);
/* 1735 */       tessellator.a(f6, par3 + 0.015625D, f7, d2, d4);
/* 1736 */       tessellator.a(f5, par3 + 0.015625D, f7, d, d4);
/* 1737 */       tessellator.a(f5, par3 + 0.015625D, f8, d, d6);
/* 1738 */       tessellator.a(f, f, f);
/* 1739 */       tessellator.a(f6, par3 + 0.015625D, f8, d2, d6 + 0.0625D);
/* 1740 */       tessellator.a(f6, par3 + 0.015625D, f7, d2, d4 + 0.0625D);
/* 1741 */       tessellator.a(f5, par3 + 0.015625D, f7, d, d4 + 0.0625D);
/* 1742 */       tessellator.a(f5, par3 + 0.015625D, f8, d, d6 + 0.0625D);
/*      */     }
/* 1744 */     else if (byte0 == 1) {
/*      */       
/* 1746 */       tessellator.a(f6, par3 + 0.015625D, f8, d2, d6);
/* 1747 */       tessellator.a(f6, par3 + 0.015625D, f7, d2, d4);
/* 1748 */       tessellator.a(f5, par3 + 0.015625D, f7, d, d4);
/* 1749 */       tessellator.a(f5, par3 + 0.015625D, f8, d, d6);
/* 1750 */       tessellator.a(f, f, f);
/* 1751 */       tessellator.a(f6, par3 + 0.015625D, f8, d2, d6 + 0.0625D);
/* 1752 */       tessellator.a(f6, par3 + 0.015625D, f7, d2, d4 + 0.0625D);
/* 1753 */       tessellator.a(f5, par3 + 0.015625D, f7, d, d4 + 0.0625D);
/* 1754 */       tessellator.a(f5, par3 + 0.015625D, f8, d, d6 + 0.0625D);
/*      */     }
/* 1756 */     else if (byte0 == 2) {
/*      */       
/* 1758 */       tessellator.a(f6, par3 + 0.015625D, f8, d2, d6);
/* 1759 */       tessellator.a(f6, par3 + 0.015625D, f7, d, d6);
/* 1760 */       tessellator.a(f5, par3 + 0.015625D, f7, d, d4);
/* 1761 */       tessellator.a(f5, par3 + 0.015625D, f8, d2, d4);
/* 1762 */       tessellator.a(f, f, f);
/* 1763 */       tessellator.a(f6, par3 + 0.015625D, f8, d2, d6 + 0.0625D);
/* 1764 */       tessellator.a(f6, par3 + 0.015625D, f7, d, d6 + 0.0625D);
/* 1765 */       tessellator.a(f5, par3 + 0.015625D, f7, d, d4 + 0.0625D);
/* 1766 */       tessellator.a(f5, par3 + 0.015625D, f8, d2, d4 + 0.0625D);
/*      */     } 
/* 1768 */     if (!this.a.h(par2, par3 + 1, par4)) {
/*      */       
/* 1770 */       double d1 = ((k + 16) / 256.0F);
/* 1771 */       double d3 = (((k + 16) + 15.99F) / 256.0F);
/* 1772 */       double d5 = (l / 256.0F);
/* 1773 */       double d7 = ((l + 15.99F) / 256.0F);
/* 1774 */       if (this.a.h(par2 - 1, par3, par4) && this.a.a(par2 - 1, par3 + 1, par4) == pb.av.bO) {
/*      */         
/* 1776 */         tessellator.a(f * f2, f * f3, f * f4);
/* 1777 */         tessellator.a(par2 + 0.015625D, ((par3 + 1) + 0.021875F), (par4 + 1), d3, d5);
/* 1778 */         tessellator.a(par2 + 0.015625D, (par3 + 0), (par4 + 1), d1, d5);
/* 1779 */         tessellator.a(par2 + 0.015625D, (par3 + 0), (par4 + 0), d1, d7);
/* 1780 */         tessellator.a(par2 + 0.015625D, ((par3 + 1) + 0.021875F), (par4 + 0), d3, d7);
/* 1781 */         tessellator.a(f, f, f);
/* 1782 */         tessellator.a(par2 + 0.015625D, ((par3 + 1) + 0.021875F), (par4 + 1), d3, d5 + 0.0625D);
/* 1783 */         tessellator.a(par2 + 0.015625D, (par3 + 0), (par4 + 1), d1, d5 + 0.0625D);
/* 1784 */         tessellator.a(par2 + 0.015625D, (par3 + 0), (par4 + 0), d1, d7 + 0.0625D);
/* 1785 */         tessellator.a(par2 + 0.015625D, ((par3 + 1) + 0.021875F), (par4 + 0), d3, d7 + 0.0625D);
/*      */       } 
/* 1787 */       if (this.a.h(par2 + 1, par3, par4) && this.a.a(par2 + 1, par3 + 1, par4) == pb.av.bO) {
/*      */         
/* 1789 */         tessellator.a(f * f2, f * f3, f * f4);
/* 1790 */         tessellator.a((par2 + 1) - 0.015625D, (par3 + 0), (par4 + 1), d1, d7);
/* 1791 */         tessellator.a((par2 + 1) - 0.015625D, ((par3 + 1) + 0.021875F), (par4 + 1), d3, d7);
/* 1792 */         tessellator.a((par2 + 1) - 0.015625D, ((par3 + 1) + 0.021875F), (par4 + 0), d3, d5);
/* 1793 */         tessellator.a((par2 + 1) - 0.015625D, (par3 + 0), (par4 + 0), d1, d5);
/* 1794 */         tessellator.a(f, f, f);
/* 1795 */         tessellator.a((par2 + 1) - 0.015625D, (par3 + 0), (par4 + 1), d1, d7 + 0.0625D);
/* 1796 */         tessellator.a((par2 + 1) - 0.015625D, ((par3 + 1) + 0.021875F), (par4 + 1), d3, d7 + 0.0625D);
/* 1797 */         tessellator.a((par2 + 1) - 0.015625D, ((par3 + 1) + 0.021875F), (par4 + 0), d3, d5 + 0.0625D);
/* 1798 */         tessellator.a((par2 + 1) - 0.015625D, (par3 + 0), (par4 + 0), d1, d5 + 0.0625D);
/*      */       } 
/* 1800 */       if (this.a.h(par2, par3, par4 - 1) && this.a.a(par2, par3 + 1, par4 - 1) == pb.av.bO) {
/*      */         
/* 1802 */         tessellator.a(f * f2, f * f3, f * f4);
/* 1803 */         tessellator.a((par2 + 1), (par3 + 0), par4 + 0.015625D, d1, d7);
/* 1804 */         tessellator.a((par2 + 1), ((par3 + 1) + 0.021875F), par4 + 0.015625D, d3, d7);
/* 1805 */         tessellator.a((par2 + 0), ((par3 + 1) + 0.021875F), par4 + 0.015625D, d3, d5);
/* 1806 */         tessellator.a((par2 + 0), (par3 + 0), par4 + 0.015625D, d1, d5);
/* 1807 */         tessellator.a(f, f, f);
/* 1808 */         tessellator.a((par2 + 1), (par3 + 0), par4 + 0.015625D, d1, d7 + 0.0625D);
/* 1809 */         tessellator.a((par2 + 1), ((par3 + 1) + 0.021875F), par4 + 0.015625D, d3, d7 + 0.0625D);
/* 1810 */         tessellator.a((par2 + 0), ((par3 + 1) + 0.021875F), par4 + 0.015625D, d3, d5 + 0.0625D);
/* 1811 */         tessellator.a((par2 + 0), (par3 + 0), par4 + 0.015625D, d1, d5 + 0.0625D);
/*      */       } 
/* 1813 */       if (this.a.h(par2, par3, par4 + 1) && this.a.a(par2, par3 + 1, par4 + 1) == pb.av.bO) {
/*      */         
/* 1815 */         tessellator.a(f * f2, f * f3, f * f4);
/* 1816 */         tessellator.a((par2 + 1), ((par3 + 1) + 0.021875F), (par4 + 1) - 0.015625D, d3, d5);
/* 1817 */         tessellator.a((par2 + 1), (par3 + 0), (par4 + 1) - 0.015625D, d1, d5);
/* 1818 */         tessellator.a((par2 + 0), (par3 + 0), (par4 + 1) - 0.015625D, d1, d7);
/* 1819 */         tessellator.a((par2 + 0), ((par3 + 1) + 0.021875F), (par4 + 1) - 0.015625D, d3, d7);
/* 1820 */         tessellator.a(f, f, f);
/* 1821 */         tessellator.a((par2 + 1), ((par3 + 1) + 0.021875F), (par4 + 1) - 0.015625D, d3, d5 + 0.0625D);
/* 1822 */         tessellator.a((par2 + 1), (par3 + 0), (par4 + 1) - 0.015625D, d1, d5 + 0.0625D);
/* 1823 */         tessellator.a((par2 + 0), (par3 + 0), (par4 + 1) - 0.015625D, d1, d7 + 0.0625D);
/* 1824 */         tessellator.a((par2 + 0), ((par3 + 1) + 0.021875F), (par4 + 1) - 0.015625D, d3, d7 + 0.0625D);
/*      */       } 
/*      */     } 
/*      */     
/* 1828 */     if (Config.isBetterSnow())
/*      */     {
/* 1830 */       if (hasSnowNeighbours(par2, par3, par4)) {
/*      */         
/* 1832 */         double oldMaxY = pb.aS.bZ;
/* 1833 */         pb.aS.bZ = 0.01D;
/* 1834 */         o(pb.aS, par2, par3, par4);
/* 1835 */         pb.aS.bZ = oldMaxY;
/*      */       } 
/*      */     }
/*      */     
/* 1839 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean a(aez par1BlockRail, int par2, int par3, int par4) {
/* 1847 */     adz tessellator = adz.a;
/* 1848 */     int i = this.a.e(par2, par3, par4);
/* 1849 */     int j = par1BlockRail.a(0, i);
/* 1850 */     if (this.d >= 0)
/*      */     {
/* 1852 */       j = this.d;
/*      */     }
/* 1854 */     if (par1BlockRail.v())
/*      */     {
/* 1856 */       i &= 0x7;
/*      */     }
/* 1858 */     tessellator.b(par1BlockRail.d(this.a, par2, par3, par4));
/* 1859 */     tessellator.a(1.0F, 1.0F, 1.0F);
/* 1860 */     int k = (j & 0xF) << 4;
/* 1861 */     int l = j & 0xF0;
/* 1862 */     double d = (k / 256.0F);
/* 1863 */     double d1 = ((k + 15.99F) / 256.0F);
/* 1864 */     double d2 = (l / 256.0F);
/* 1865 */     double d3 = ((l + 15.99F) / 256.0F);
/* 1866 */     double d4 = 0.0625D;
/* 1867 */     double d5 = (par2 + 1);
/* 1868 */     double d6 = (par2 + 1);
/* 1869 */     double d7 = (par2 + 0);
/* 1870 */     double d8 = (par2 + 0);
/* 1871 */     double d9 = (par4 + 0);
/* 1872 */     double d10 = (par4 + 1);
/* 1873 */     double d11 = (par4 + 1);
/* 1874 */     double d12 = (par4 + 0);
/* 1875 */     double d13 = par3 + d4;
/* 1876 */     double d14 = par3 + d4;
/* 1877 */     double d15 = par3 + d4;
/* 1878 */     double d16 = par3 + d4;
/*      */ 
/*      */     
/* 1881 */     d5 = d8 = (par2 + 1);
/* 1882 */     d6 = d7 = (par2 + 0);
/* 1883 */     d9 = d10 = (par4 + 1);
/* 1884 */     d11 = d12 = (par4 + 0);
/*      */ 
/*      */ 
/*      */     
/* 1888 */     d5 = d6 = (par2 + 0);
/* 1889 */     d7 = d8 = (par2 + 1);
/* 1890 */     d9 = d12 = (par4 + 1);
/* 1891 */     d10 = d11 = (par4 + 0);
/*      */     
/* 1893 */     if (i == 9) {
/*      */       
/* 1895 */       d5 = d8 = (par2 + 0);
/* 1896 */       d6 = d7 = (par2 + 1);
/* 1897 */       d9 = d10 = (par4 + 0);
/* 1898 */       d11 = d12 = (par4 + 1);
/*      */     } 
/* 1900 */     if (i == 2 || i == 4) {
/*      */       
/* 1902 */       d13++;
/* 1903 */       d16++;
/*      */     }
/* 1905 */     else if (i == 3 || i == 5) {
/*      */       
/* 1907 */       d14++;
/* 1908 */       d15++;
/*      */     } 
/* 1910 */     tessellator.a(d5, d13, d9, d1, d2);
/* 1911 */     tessellator.a(d6, d14, d10, d1, d3);
/* 1912 */     tessellator.a(d7, d15, d11, d, d3);
/* 1913 */     tessellator.a(d8, d16, d12, d, d2);
/* 1914 */     tessellator.a(d8, d16, d12, d, d2);
/* 1915 */     tessellator.a(d7, d15, d11, d, d3);
/* 1916 */     tessellator.a(d6, d14, d10, d1, d3);
/* 1917 */     tessellator.a(d5, d13, d9, d1, d2);
/*      */     
/* 1919 */     if (Config.isBetterSnow())
/*      */     {
/* 1921 */       if (hasSnowNeighbours(par2, par3, par4)) {
/*      */         
/* 1923 */         double oldMaxY = pb.aS.bZ;
/* 1924 */         pb.aS.bZ = 0.05D;
/* 1925 */         o(pb.aS, par2, par3, par4);
/* 1926 */         pb.aS.bZ = oldMaxY;
/*      */       } 
/*      */     }
/* 1929 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean h(pb par1Block, int par2, int par3, int par4) {
/* 1937 */     adz tessellator = adz.a;
/* 1938 */     int i = par1Block.a_(0);
/* 1939 */     if (this.d >= 0)
/*      */     {
/* 1941 */       i = this.d;
/*      */     }
/* 1943 */     tessellator.b(par1Block.d(this.a, par2, par3, par4));
/* 1944 */     float f = 1.0F;
/* 1945 */     tessellator.a(f, f, f);
/* 1946 */     f = ((i & 0xF) << 4);
/* 1947 */     int j = i & 0xF0;
/* 1948 */     double d = (f / 256.0F);
/* 1949 */     double d1 = ((f + 15.99F) / 256.0F);
/* 1950 */     double d2 = (j / 256.0F);
/* 1951 */     double d3 = ((j + 15.99F) / 256.0F);
/* 1952 */     int k = this.a.e(par2, par3, par4);
/* 1953 */     double d4 = 0.0D;
/* 1954 */     double d5 = 0.05000000074505806D;
/* 1955 */     if (k == 5) {
/*      */       
/* 1957 */       tessellator.a(par2 + d5, (par3 + 1) + d4, (par4 + 1) + d4, d, d2);
/* 1958 */       tessellator.a(par2 + d5, (par3 + 0) - d4, (par4 + 1) + d4, d, d3);
/* 1959 */       tessellator.a(par2 + d5, (par3 + 0) - d4, (par4 + 0) - d4, d1, d3);
/* 1960 */       tessellator.a(par2 + d5, (par3 + 1) + d4, (par4 + 0) - d4, d1, d2);
/*      */     } 
/* 1962 */     if (k == 4) {
/*      */       
/* 1964 */       tessellator.a((par2 + 1) - d5, (par3 + 0) - d4, (par4 + 1) + d4, d1, d3);
/* 1965 */       tessellator.a((par2 + 1) - d5, (par3 + 1) + d4, (par4 + 1) + d4, d1, d2);
/* 1966 */       tessellator.a((par2 + 1) - d5, (par3 + 1) + d4, (par4 + 0) - d4, d, d2);
/* 1967 */       tessellator.a((par2 + 1) - d5, (par3 + 0) - d4, (par4 + 0) - d4, d, d3);
/*      */     } 
/* 1969 */     if (k == 3) {
/*      */       
/* 1971 */       tessellator.a((par2 + 1) + d4, (par3 + 0) - d4, par4 + d5, d1, d3);
/* 1972 */       tessellator.a((par2 + 1) + d4, (par3 + 1) + d4, par4 + d5, d1, d2);
/* 1973 */       tessellator.a((par2 + 0) - d4, (par3 + 1) + d4, par4 + d5, d, d2);
/* 1974 */       tessellator.a((par2 + 0) - d4, (par3 + 0) - d4, par4 + d5, d, d3);
/*      */     } 
/* 1976 */     if (k == 2) {
/*      */       
/* 1978 */       tessellator.a((par2 + 1) + d4, (par3 + 1) + d4, (par4 + 1) - d5, d, d2);
/* 1979 */       tessellator.a((par2 + 1) + d4, (par3 + 0) - d4, (par4 + 1) - d5, d, d3);
/* 1980 */       tessellator.a((par2 + 0) - d4, (par3 + 0) - d4, (par4 + 1) - d5, d1, d3);
/* 1981 */       tessellator.a((par2 + 0) - d4, (par3 + 1) + d4, (par4 + 1) - d5, d1, d2);
/*      */     } 
/* 1983 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean i(pb par1Block, int par2, int par3, int par4) {
/* 1991 */     adz tessellator = adz.a;
/* 1992 */     int i = par1Block.a_(0);
/* 1993 */     if (this.d >= 0)
/*      */     {
/* 1995 */       i = this.d;
/*      */     }
/*      */     
/* 1998 */     if (Config.isConnectedTextures() && this.d < 0) {
/*      */ 
/*      */       
/* 2001 */       int tex = ConnectedTextures.getConnectedTexture(this.a, par1Block, par2, par3, par4, -1, i);
/* 2002 */       if (tex >= 0) {
/*      */         
/* 2004 */         int ctmTex = tex / 256;
/* 2005 */         tessellator = adz.a.getSubTessellator(ctmTex);
/* 2006 */         i = tex % 256;
/*      */       } 
/*      */     } 
/* 2009 */     float f = 1.0F;
/* 2010 */     tessellator.b(par1Block.d(this.a, par2, par3, par4));
/*      */     
/* 2012 */     int j = CustomColorizer.getColorMultiplier(par1Block, this.a, par2, par3, par4);
/*      */     
/* 2014 */     float f1 = (j >> 16 & 0xFF) / 255.0F;
/* 2015 */     float d = (j >> 8 & 0xFF) / 255.0F;
/* 2016 */     float f2 = (j & 0xFF) / 255.0F;
/* 2017 */     tessellator.a(f * f1, f * d, f * f2);
/* 2018 */     j = (i & 0xF) << 4;
/* 2019 */     f1 = (i & 0xF0);
/* 2020 */     d = j / 256.0F;
/* 2021 */     double d1 = ((j + 15.99F) / 256.0F);
/* 2022 */     double d2 = (f1 / 256.0F);
/* 2023 */     double d3 = ((f1 + 15.99F) / 256.0F);
/* 2024 */     double d4 = 0.05000000074505806D;
/* 2025 */     int k = this.a.e(par2, par3, par4);
/* 2026 */     if ((k & 0x2) != 0) {
/*      */       
/* 2028 */       tessellator.a(par2 + d4, (par3 + 1), (par4 + 1), d, d2);
/* 2029 */       tessellator.a(par2 + d4, (par3 + 0), (par4 + 1), d, d3);
/* 2030 */       tessellator.a(par2 + d4, (par3 + 0), (par4 + 0), d1, d3);
/* 2031 */       tessellator.a(par2 + d4, (par3 + 1), (par4 + 0), d1, d2);
/* 2032 */       tessellator.a(par2 + d4, (par3 + 1), (par4 + 0), d1, d2);
/* 2033 */       tessellator.a(par2 + d4, (par3 + 0), (par4 + 0), d1, d3);
/* 2034 */       tessellator.a(par2 + d4, (par3 + 0), (par4 + 1), d, d3);
/* 2035 */       tessellator.a(par2 + d4, (par3 + 1), (par4 + 1), d, d2);
/*      */     } 
/* 2037 */     if ((k & 0x8) != 0) {
/*      */       
/* 2039 */       tessellator.a((par2 + 1) - d4, (par3 + 0), (par4 + 1), d1, d3);
/* 2040 */       tessellator.a((par2 + 1) - d4, (par3 + 1), (par4 + 1), d1, d2);
/* 2041 */       tessellator.a((par2 + 1) - d4, (par3 + 1), (par4 + 0), d, d2);
/* 2042 */       tessellator.a((par2 + 1) - d4, (par3 + 0), (par4 + 0), d, d3);
/* 2043 */       tessellator.a((par2 + 1) - d4, (par3 + 0), (par4 + 0), d, d3);
/* 2044 */       tessellator.a((par2 + 1) - d4, (par3 + 1), (par4 + 0), d, d2);
/* 2045 */       tessellator.a((par2 + 1) - d4, (par3 + 1), (par4 + 1), d1, d2);
/* 2046 */       tessellator.a((par2 + 1) - d4, (par3 + 0), (par4 + 1), d1, d3);
/*      */     } 
/* 2048 */     if ((k & 0x4) != 0) {
/*      */       
/* 2050 */       tessellator.a((par2 + 1), (par3 + 0), par4 + d4, d1, d3);
/* 2051 */       tessellator.a((par2 + 1), (par3 + 1), par4 + d4, d1, d2);
/* 2052 */       tessellator.a((par2 + 0), (par3 + 1), par4 + d4, d, d2);
/* 2053 */       tessellator.a((par2 + 0), (par3 + 0), par4 + d4, d, d3);
/* 2054 */       tessellator.a((par2 + 0), (par3 + 0), par4 + d4, d, d3);
/* 2055 */       tessellator.a((par2 + 0), (par3 + 1), par4 + d4, d, d2);
/* 2056 */       tessellator.a((par2 + 1), (par3 + 1), par4 + d4, d1, d2);
/* 2057 */       tessellator.a((par2 + 1), (par3 + 0), par4 + d4, d1, d3);
/*      */     } 
/* 2059 */     if ((k & 0x1) != 0) {
/*      */       
/* 2061 */       tessellator.a((par2 + 1), (par3 + 1), (par4 + 1) - d4, d, d2);
/* 2062 */       tessellator.a((par2 + 1), (par3 + 0), (par4 + 1) - d4, d, d3);
/* 2063 */       tessellator.a((par2 + 0), (par3 + 0), (par4 + 1) - d4, d1, d3);
/* 2064 */       tessellator.a((par2 + 0), (par3 + 1), (par4 + 1) - d4, d1, d2);
/* 2065 */       tessellator.a((par2 + 0), (par3 + 1), (par4 + 1) - d4, d1, d2);
/* 2066 */       tessellator.a((par2 + 0), (par3 + 0), (par4 + 1) - d4, d1, d3);
/* 2067 */       tessellator.a((par2 + 1), (par3 + 0), (par4 + 1) - d4, d, d3);
/* 2068 */       tessellator.a((par2 + 1), (par3 + 1), (par4 + 1) - d4, d, d2);
/*      */     } 
/* 2070 */     if (this.a.h(par2, par3 + 1, par4)) {
/*      */       
/* 2072 */       tessellator.a((par2 + 1), (par3 + 1) - d4, (par4 + 0), d, d2);
/* 2073 */       tessellator.a((par2 + 1), (par3 + 1) - d4, (par4 + 1), d, d3);
/* 2074 */       tessellator.a((par2 + 0), (par3 + 1) - d4, (par4 + 1), d1, d3);
/* 2075 */       tessellator.a((par2 + 0), (par3 + 1) - d4, (par4 + 0), d1, d2);
/*      */     } 
/* 2077 */     return true;
/*      */   }
/*      */ 
/*      */   
/*      */   public boolean a(akq par1BlockPane, int par2, int par3, int par4) {
/* 2082 */     int i = this.a.b();
/* 2083 */     adz tessellator = adz.a;
/*      */     
/* 2085 */     boolean connected = (par1BlockPane == pb.bq && ConnectedTextures.isConnectedGlassPanes());
/* 2086 */     adz tessellatorFront = tessellator;
/* 2087 */     adz tessellatorSide = tessellator;
/*      */     
/* 2089 */     tessellator.b(par1BlockPane.d(this.a, par2, par3, par4));
/* 2090 */     float f = 1.0F;
/* 2091 */     int j = par1BlockPane.c(this.a, par2, par3, par4);
/* 2092 */     float f1 = (j >> 16 & 0xFF) / 255.0F;
/* 2093 */     float f2 = (j >> 8 & 0xFF) / 255.0F;
/* 2094 */     float f3 = (j & 0xFF) / 255.0F;
/* 2095 */     if (lr.a) {
/*      */       
/* 2097 */       float f4 = (f1 * 30.0F + f2 * 59.0F + f3 * 11.0F) / 100.0F;
/* 2098 */       float f5 = (f1 * 30.0F + f2 * 70.0F) / 100.0F;
/* 2099 */       float f6 = (f1 * 30.0F + f3 * 70.0F) / 100.0F;
/* 2100 */       f1 = f4;
/* 2101 */       f2 = f5;
/* 2102 */       f3 = f6;
/*      */     } 
/* 2104 */     tessellator.a(f * f1, f * f2, f * f3);
/* 2105 */     int k = 0;
/* 2106 */     int l = 0;
/* 2107 */     if (this.d >= 0) {
/*      */       
/* 2109 */       k = this.d;
/* 2110 */       l = this.d;
/*      */       
/* 2112 */       connected = false;
/*      */     } else {
/*      */       
/* 2115 */       int i1 = this.a.e(par2, par3, par4);
/* 2116 */       k = par1BlockPane.a(0, i1);
/* 2117 */       l = par1BlockPane.v();
/*      */       
/* 2119 */       if (connected) {
/*      */         
/* 2121 */         int ctmTex = (Config.getMinecraft()).p.b("/ctm.png");
/* 2122 */         tessellatorFront = adz.a.getSubTessellator(ctmTex);
/*      */         
/* 2124 */         k = 0;
/*      */       } 
/*      */     } 
/*      */     
/* 2128 */     int kr = k;
/* 2129 */     int kz = k;
/* 2130 */     int kzr = k;
/*      */     
/* 2132 */     int blockIdXp = this.a.a(par2 + 1, par3, par4);
/* 2133 */     int blockIdXn = this.a.a(par2 - 1, par3, par4);
/* 2134 */     int blockIdZp = this.a.a(par2, par3, par4 + 1);
/* 2135 */     int blockIdZn = this.a.a(par2, par3, par4 - 1);
/*      */     
/* 2137 */     if (connected) {
/*      */       
/* 2139 */       int glassPaneId = pb.bq.bO;
/* 2140 */       int blockIdYp = this.a.a(par2, par3 + 1, par4);
/* 2141 */       int blockIdYn = this.a.a(par2, par3 - 1, par4);
/*      */       
/* 2143 */       boolean linkXp = (blockIdXp == glassPaneId);
/* 2144 */       boolean linkXn = (blockIdXn == glassPaneId);
/* 2145 */       boolean linkYp = (blockIdYp == glassPaneId);
/* 2146 */       boolean linkYn = (blockIdYn == glassPaneId);
/* 2147 */       boolean linkZp = (blockIdZp == glassPaneId);
/* 2148 */       boolean linkZn = (blockIdZn == glassPaneId);
/*      */       
/* 2150 */       k = getGlassPaneTexture(linkXp, linkXn, linkYp, linkYn);
/* 2151 */       kr = getReverseGlassPaneTexture(k);
/*      */       
/* 2153 */       kz = getGlassPaneTexture(linkZp, linkZn, linkYp, linkYn);
/* 2154 */       kzr = getReverseGlassPaneTexture(kz);
/*      */     } 
/*      */     
/* 2157 */     int j1 = (k & 0xF) << 4;
/* 2158 */     int k1 = k & 0xF0;
/* 2159 */     double d = (j1 / 256.0F);
/* 2160 */     double d1 = ((j1 + 7.99F) / 256.0F);
/* 2161 */     double d2 = ((j1 + 15.99F) / 256.0F);
/* 2162 */     double d3 = (k1 / 256.0F);
/* 2163 */     double d4 = ((k1 + 15.99F) / 256.0F);
/*      */     
/* 2165 */     int j1r = (kr & 0xF) << 4;
/* 2166 */     int k1r = kr & 0xF0;
/* 2167 */     double dr = (j1r / 256.0F);
/* 2168 */     double d1r = ((j1r + 7.99F) / 256.0F);
/* 2169 */     double d2r = ((j1r + 15.99F) / 256.0F);
/* 2170 */     double d3r = (k1r / 256.0F);
/* 2171 */     double d4r = ((k1r + 15.99F) / 256.0F);
/*      */     
/* 2173 */     int j1z = (kz & 0xF) << 4;
/* 2174 */     int k1z = kz & 0xF0;
/* 2175 */     double dz = (j1z / 256.0F);
/* 2176 */     double d1z = ((j1z + 7.99F) / 256.0F);
/* 2177 */     double d2z = ((j1z + 15.99F) / 256.0F);
/* 2178 */     double d3z = (k1z / 256.0F);
/* 2179 */     double d4z = ((k1z + 15.99F) / 256.0F);
/*      */     
/* 2181 */     int j1zr = (kzr & 0xF) << 4;
/* 2182 */     int k1zr = kzr & 0xF0;
/* 2183 */     double dzr = (j1zr / 256.0F);
/* 2184 */     double d1zr = ((j1zr + 7.99F) / 256.0F);
/* 2185 */     double d2zr = ((j1zr + 15.99F) / 256.0F);
/* 2186 */     double d3zr = (k1zr / 256.0F);
/* 2187 */     double d4zr = ((k1zr + 15.99F) / 256.0F);
/*      */     
/* 2189 */     int l1 = (l & 0xF) << 4;
/* 2190 */     int i2 = l & 0xF0;
/* 2191 */     double d5 = ((l1 + 7) / 256.0F);
/* 2192 */     double d6 = ((l1 + 8.99F) / 256.0F);
/* 2193 */     double d7 = (i2 / 256.0F);
/* 2194 */     double d8 = ((i2 + 8) / 256.0F);
/* 2195 */     double d9 = ((i2 + 15.99F) / 256.0F);
/* 2196 */     double d10 = par2;
/* 2197 */     double d11 = par2 + 0.5D;
/* 2198 */     double d12 = (par2 + 1);
/* 2199 */     double d13 = par4;
/* 2200 */     double d14 = par4 + 0.5D;
/* 2201 */     double d15 = (par4 + 1);
/* 2202 */     double d16 = par2 + 0.5D - 0.0625D;
/* 2203 */     double d17 = par2 + 0.5D + 0.0625D;
/* 2204 */     double d18 = par4 + 0.5D - 0.0625D;
/* 2205 */     double d19 = par4 + 0.5D + 0.0625D;
/*      */     
/* 2207 */     boolean flag = par1BlockPane.e(this.a.a(par2, par3, par4 - 1));
/* 2208 */     boolean flag1 = par1BlockPane.e(this.a.a(par2, par3, par4 + 1));
/* 2209 */     boolean flag2 = par1BlockPane.e(this.a.a(par2 - 1, par3, par4));
/* 2210 */     boolean flag3 = par1BlockPane.e(this.a.a(par2 + 1, par3, par4));
/* 2211 */     boolean flag4 = par1BlockPane.a(this.a, par2, par3 + 1, par4, 1);
/* 2212 */     boolean flag5 = par1BlockPane.a(this.a, par2, par3 - 1, par4, 0);
/*      */     
/* 2214 */     if ((flag2 && flag3) || (!flag2 && !flag3 && !flag && !flag1)) {
/*      */ 
/*      */ 
/*      */       
/* 2218 */       tessellator = tessellatorFront;
/*      */       
/* 2220 */       tessellator.a(d10, (par3 + 1), d14, d, d3);
/* 2221 */       tessellator.a(d10, (par3 + 0), d14, d, d4);
/* 2222 */       tessellator.a(d12, (par3 + 0), d14, d2, d4);
/* 2223 */       tessellator.a(d12, (par3 + 1), d14, d2, d3);
/*      */       
/* 2225 */       tessellator.a(d12, (par3 + 1), d14, dr, d3r);
/* 2226 */       tessellator.a(d12, (par3 + 0), d14, dr, d4r);
/* 2227 */       tessellator.a(d10, (par3 + 0), d14, d2r, d4r);
/* 2228 */       tessellator.a(d10, (par3 + 1), d14, d2r, d3r);
/*      */       
/* 2230 */       tessellator = tessellatorSide;
/*      */       
/* 2232 */       if (flag4) {
/*      */         
/* 2234 */         tessellator.a(d10, (par3 + 1) + 0.01D, d19, d6, d9);
/* 2235 */         tessellator.a(d12, (par3 + 1) + 0.01D, d19, d6, d7);
/* 2236 */         tessellator.a(d12, (par3 + 1) + 0.01D, d18, d5, d7);
/* 2237 */         tessellator.a(d10, (par3 + 1) + 0.01D, d18, d5, d9);
/* 2238 */         tessellator.a(d12, (par3 + 1) + 0.01D, d19, d6, d9);
/* 2239 */         tessellator.a(d10, (par3 + 1) + 0.01D, d19, d6, d7);
/* 2240 */         tessellator.a(d10, (par3 + 1) + 0.01D, d18, d5, d7);
/* 2241 */         tessellator.a(d12, (par3 + 1) + 0.01D, d18, d5, d9);
/*      */       } else {
/*      */         
/* 2244 */         if (par3 < i - 1 && this.a.i(par2 - 1, par3 + 1, par4)) {
/*      */           
/* 2246 */           tessellator.a(d10, (par3 + 1) + 0.01D, d19, d6, d8);
/* 2247 */           tessellator.a(d11, (par3 + 1) + 0.01D, d19, d6, d9);
/* 2248 */           tessellator.a(d11, (par3 + 1) + 0.01D, d18, d5, d9);
/* 2249 */           tessellator.a(d10, (par3 + 1) + 0.01D, d18, d5, d8);
/* 2250 */           tessellator.a(d11, (par3 + 1) + 0.01D, d19, d6, d8);
/* 2251 */           tessellator.a(d10, (par3 + 1) + 0.01D, d19, d6, d9);
/* 2252 */           tessellator.a(d10, (par3 + 1) + 0.01D, d18, d5, d9);
/* 2253 */           tessellator.a(d11, (par3 + 1) + 0.01D, d18, d5, d8);
/*      */         } 
/* 2255 */         if (par3 < i - 1 && this.a.i(par2 + 1, par3 + 1, par4)) {
/*      */           
/* 2257 */           tessellator.a(d11, (par3 + 1) + 0.01D, d19, d6, d7);
/* 2258 */           tessellator.a(d12, (par3 + 1) + 0.01D, d19, d6, d8);
/* 2259 */           tessellator.a(d12, (par3 + 1) + 0.01D, d18, d5, d8);
/* 2260 */           tessellator.a(d11, (par3 + 1) + 0.01D, d18, d5, d7);
/* 2261 */           tessellator.a(d12, (par3 + 1) + 0.01D, d19, d6, d7);
/* 2262 */           tessellator.a(d11, (par3 + 1) + 0.01D, d19, d6, d8);
/* 2263 */           tessellator.a(d11, (par3 + 1) + 0.01D, d18, d5, d8);
/* 2264 */           tessellator.a(d12, (par3 + 1) + 0.01D, d18, d5, d7);
/*      */         } 
/*      */       } 
/* 2267 */       if (flag5) {
/*      */         
/* 2269 */         tessellator.a(d10, par3 - 0.01D, d19, d6, d9);
/* 2270 */         tessellator.a(d12, par3 - 0.01D, d19, d6, d7);
/* 2271 */         tessellator.a(d12, par3 - 0.01D, d18, d5, d7);
/* 2272 */         tessellator.a(d10, par3 - 0.01D, d18, d5, d9);
/* 2273 */         tessellator.a(d12, par3 - 0.01D, d19, d6, d9);
/* 2274 */         tessellator.a(d10, par3 - 0.01D, d19, d6, d7);
/* 2275 */         tessellator.a(d10, par3 - 0.01D, d18, d5, d7);
/* 2276 */         tessellator.a(d12, par3 - 0.01D, d18, d5, d9);
/*      */       } else {
/*      */         
/* 2279 */         if (par3 > 1 && this.a.i(par2 - 1, par3 - 1, par4)) {
/*      */           
/* 2281 */           tessellator.a(d10, par3 - 0.01D, d19, d6, d8);
/* 2282 */           tessellator.a(d11, par3 - 0.01D, d19, d6, d9);
/* 2283 */           tessellator.a(d11, par3 - 0.01D, d18, d5, d9);
/* 2284 */           tessellator.a(d10, par3 - 0.01D, d18, d5, d8);
/* 2285 */           tessellator.a(d11, par3 - 0.01D, d19, d6, d8);
/* 2286 */           tessellator.a(d10, par3 - 0.01D, d19, d6, d9);
/* 2287 */           tessellator.a(d10, par3 - 0.01D, d18, d5, d9);
/* 2288 */           tessellator.a(d11, par3 - 0.01D, d18, d5, d8);
/*      */         } 
/* 2290 */         if (par3 > 1 && this.a.i(par2 + 1, par3 - 1, par4))
/*      */         {
/* 2292 */           tessellator.a(d11, par3 - 0.01D, d19, d6, d7);
/* 2293 */           tessellator.a(d12, par3 - 0.01D, d19, d6, d8);
/* 2294 */           tessellator.a(d12, par3 - 0.01D, d18, d5, d8);
/* 2295 */           tessellator.a(d11, par3 - 0.01D, d18, d5, d7);
/* 2296 */           tessellator.a(d12, par3 - 0.01D, d19, d6, d7);
/* 2297 */           tessellator.a(d11, par3 - 0.01D, d19, d6, d8);
/* 2298 */           tessellator.a(d11, par3 - 0.01D, d18, d5, d8);
/* 2299 */           tessellator.a(d12, par3 - 0.01D, d18, d5, d7);
/*      */         }
/*      */       
/*      */       } 
/* 2303 */     } else if (flag2 && !flag3) {
/*      */ 
/*      */ 
/*      */       
/* 2307 */       tessellator = tessellatorFront;
/*      */       
/* 2309 */       tessellator.a(d10, (par3 + 1), d14, d, d3);
/* 2310 */       tessellator.a(d10, (par3 + 0), d14, d, d4);
/* 2311 */       tessellator.a(d11, (par3 + 0), d14, d1, d4);
/* 2312 */       tessellator.a(d11, (par3 + 1), d14, d1, d3);
/*      */       
/* 2314 */       tessellator.a(d11, (par3 + 1), d14, d1r, d3r);
/* 2315 */       tessellator.a(d11, (par3 + 0), d14, d1r, d4r);
/* 2316 */       tessellator.a(d10, (par3 + 0), d14, d2r, d4r);
/* 2317 */       tessellator.a(d10, (par3 + 1), d14, d2r, d3r);
/*      */       
/* 2319 */       tessellator = tessellatorSide;
/*      */       
/* 2321 */       if (!flag1 && !flag) {
/*      */         
/* 2323 */         tessellator.a(d11, (par3 + 1), d19, d5, d7);
/* 2324 */         tessellator.a(d11, (par3 + 0), d19, d5, d9);
/* 2325 */         tessellator.a(d11, (par3 + 0), d18, d6, d9);
/* 2326 */         tessellator.a(d11, (par3 + 1), d18, d6, d7);
/* 2327 */         tessellator.a(d11, (par3 + 1), d18, d5, d7);
/* 2328 */         tessellator.a(d11, (par3 + 0), d18, d5, d9);
/* 2329 */         tessellator.a(d11, (par3 + 0), d19, d6, d9);
/* 2330 */         tessellator.a(d11, (par3 + 1), d19, d6, d7);
/*      */       } 
/* 2332 */       if (flag4 || (par3 < i - 1 && this.a.i(par2 - 1, par3 + 1, par4))) {
/*      */         
/* 2334 */         tessellator.a(d10, (par3 + 1) + 0.01D, d19, d6, d8);
/* 2335 */         tessellator.a(d11, (par3 + 1) + 0.01D, d19, d6, d9);
/* 2336 */         tessellator.a(d11, (par3 + 1) + 0.01D, d18, d5, d9);
/* 2337 */         tessellator.a(d10, (par3 + 1) + 0.01D, d18, d5, d8);
/* 2338 */         tessellator.a(d11, (par3 + 1) + 0.01D, d19, d6, d8);
/* 2339 */         tessellator.a(d10, (par3 + 1) + 0.01D, d19, d6, d9);
/* 2340 */         tessellator.a(d10, (par3 + 1) + 0.01D, d18, d5, d9);
/* 2341 */         tessellator.a(d11, (par3 + 1) + 0.01D, d18, d5, d8);
/*      */       } 
/* 2343 */       if (flag5 || (par3 > 1 && this.a.i(par2 - 1, par3 - 1, par4)))
/*      */       {
/* 2345 */         tessellator.a(d10, par3 - 0.01D, d19, d6, d8);
/* 2346 */         tessellator.a(d11, par3 - 0.01D, d19, d6, d9);
/* 2347 */         tessellator.a(d11, par3 - 0.01D, d18, d5, d9);
/* 2348 */         tessellator.a(d10, par3 - 0.01D, d18, d5, d8);
/* 2349 */         tessellator.a(d11, par3 - 0.01D, d19, d6, d8);
/* 2350 */         tessellator.a(d10, par3 - 0.01D, d19, d6, d9);
/* 2351 */         tessellator.a(d10, par3 - 0.01D, d18, d5, d9);
/* 2352 */         tessellator.a(d11, par3 - 0.01D, d18, d5, d8);
/*      */       }
/*      */     
/* 2355 */     } else if (!flag2 && flag3) {
/*      */ 
/*      */ 
/*      */       
/* 2359 */       tessellator = tessellatorFront;
/*      */       
/* 2361 */       tessellator.a(d11, (par3 + 1), d14, d1, d3);
/* 2362 */       tessellator.a(d11, (par3 + 0), d14, d1, d4);
/* 2363 */       tessellator.a(d12, (par3 + 0), d14, d2, d4);
/* 2364 */       tessellator.a(d12, (par3 + 1), d14, d2, d3);
/*      */       
/* 2366 */       tessellator.a(d12, (par3 + 1), d14, dr, d3r);
/* 2367 */       tessellator.a(d12, (par3 + 0), d14, dr, d4r);
/* 2368 */       tessellator.a(d11, (par3 + 0), d14, d1r, d4r);
/* 2369 */       tessellator.a(d11, (par3 + 1), d14, d1r, d3r);
/*      */       
/* 2371 */       tessellator = tessellatorSide;
/*      */       
/* 2373 */       if (!flag1 && !flag) {
/*      */         
/* 2375 */         tessellator.a(d11, (par3 + 1), d18, d5, d7);
/* 2376 */         tessellator.a(d11, (par3 + 0), d18, d5, d9);
/* 2377 */         tessellator.a(d11, (par3 + 0), d19, d6, d9);
/* 2378 */         tessellator.a(d11, (par3 + 1), d19, d6, d7);
/* 2379 */         tessellator.a(d11, (par3 + 1), d19, d5, d7);
/* 2380 */         tessellator.a(d11, (par3 + 0), d19, d5, d9);
/* 2381 */         tessellator.a(d11, (par3 + 0), d18, d6, d9);
/* 2382 */         tessellator.a(d11, (par3 + 1), d18, d6, d7);
/*      */       } 
/* 2384 */       if (flag4 || (par3 < i - 1 && this.a.i(par2 + 1, par3 + 1, par4))) {
/*      */         
/* 2386 */         tessellator.a(d11, (par3 + 1) + 0.01D, d19, d6, d7);
/* 2387 */         tessellator.a(d12, (par3 + 1) + 0.01D, d19, d6, d8);
/* 2388 */         tessellator.a(d12, (par3 + 1) + 0.01D, d18, d5, d8);
/* 2389 */         tessellator.a(d11, (par3 + 1) + 0.01D, d18, d5, d7);
/* 2390 */         tessellator.a(d12, (par3 + 1) + 0.01D, d19, d6, d7);
/* 2391 */         tessellator.a(d11, (par3 + 1) + 0.01D, d19, d6, d8);
/* 2392 */         tessellator.a(d11, (par3 + 1) + 0.01D, d18, d5, d8);
/* 2393 */         tessellator.a(d12, (par3 + 1) + 0.01D, d18, d5, d7);
/*      */       } 
/* 2395 */       if (flag5 || (par3 > 1 && this.a.i(par2 + 1, par3 - 1, par4))) {
/*      */         
/* 2397 */         tessellator.a(d11, par3 - 0.01D, d19, d6, d7);
/* 2398 */         tessellator.a(d12, par3 - 0.01D, d19, d6, d8);
/* 2399 */         tessellator.a(d12, par3 - 0.01D, d18, d5, d8);
/* 2400 */         tessellator.a(d11, par3 - 0.01D, d18, d5, d7);
/* 2401 */         tessellator.a(d12, par3 - 0.01D, d19, d6, d7);
/* 2402 */         tessellator.a(d11, par3 - 0.01D, d19, d6, d8);
/* 2403 */         tessellator.a(d11, par3 - 0.01D, d18, d5, d8);
/* 2404 */         tessellator.a(d12, par3 - 0.01D, d18, d5, d7);
/*      */       } 
/*      */     } 
/* 2407 */     if ((flag && flag1) || (!flag2 && !flag3 && !flag && !flag1)) {
/*      */ 
/*      */ 
/*      */       
/* 2411 */       tessellator = tessellatorFront;
/*      */       
/* 2413 */       tessellator.a(d11, (par3 + 1), d15, dzr, d3zr);
/* 2414 */       tessellator.a(d11, (par3 + 0), d15, dzr, d4zr);
/* 2415 */       tessellator.a(d11, (par3 + 0), d13, d2zr, d4zr);
/* 2416 */       tessellator.a(d11, (par3 + 1), d13, d2zr, d3zr);
/*      */       
/* 2418 */       tessellator.a(d11, (par3 + 1), d13, dz, d3z);
/* 2419 */       tessellator.a(d11, (par3 + 0), d13, dz, d4z);
/* 2420 */       tessellator.a(d11, (par3 + 0), d15, d2z, d4z);
/* 2421 */       tessellator.a(d11, (par3 + 1), d15, d2z, d3z);
/*      */       
/* 2423 */       tessellator = tessellatorSide;
/*      */       
/* 2425 */       if (flag4) {
/*      */         
/* 2427 */         tessellator.a(d17, (par3 + 1), d15, d6, d9);
/* 2428 */         tessellator.a(d17, (par3 + 1), d13, d6, d7);
/* 2429 */         tessellator.a(d16, (par3 + 1), d13, d5, d7);
/* 2430 */         tessellator.a(d16, (par3 + 1), d15, d5, d9);
/* 2431 */         tessellator.a(d17, (par3 + 1), d13, d6, d9);
/* 2432 */         tessellator.a(d17, (par3 + 1), d15, d6, d7);
/* 2433 */         tessellator.a(d16, (par3 + 1), d15, d5, d7);
/* 2434 */         tessellator.a(d16, (par3 + 1), d13, d5, d9);
/*      */       } else {
/*      */         
/* 2437 */         if (par3 < i - 1 && this.a.i(par2, par3 + 1, par4 - 1)) {
/*      */           
/* 2439 */           tessellator.a(d16, (par3 + 1), d13, d6, d7);
/* 2440 */           tessellator.a(d16, (par3 + 1), d14, d6, d8);
/* 2441 */           tessellator.a(d17, (par3 + 1), d14, d5, d8);
/* 2442 */           tessellator.a(d17, (par3 + 1), d13, d5, d7);
/* 2443 */           tessellator.a(d16, (par3 + 1), d14, d6, d7);
/* 2444 */           tessellator.a(d16, (par3 + 1), d13, d6, d8);
/* 2445 */           tessellator.a(d17, (par3 + 1), d13, d5, d8);
/* 2446 */           tessellator.a(d17, (par3 + 1), d14, d5, d7);
/*      */         } 
/* 2448 */         if (par3 < i - 1 && this.a.i(par2, par3 + 1, par4 + 1)) {
/*      */           
/* 2450 */           tessellator.a(d16, (par3 + 1), d14, d5, d8);
/* 2451 */           tessellator.a(d16, (par3 + 1), d15, d5, d9);
/* 2452 */           tessellator.a(d17, (par3 + 1), d15, d6, d9);
/* 2453 */           tessellator.a(d17, (par3 + 1), d14, d6, d8);
/* 2454 */           tessellator.a(d16, (par3 + 1), d15, d5, d8);
/* 2455 */           tessellator.a(d16, (par3 + 1), d14, d5, d9);
/* 2456 */           tessellator.a(d17, (par3 + 1), d14, d6, d9);
/* 2457 */           tessellator.a(d17, (par3 + 1), d15, d6, d8);
/*      */         } 
/*      */       } 
/* 2460 */       if (flag5) {
/*      */         
/* 2462 */         tessellator.a(d17, par3, d15, d6, d9);
/* 2463 */         tessellator.a(d17, par3, d13, d6, d7);
/* 2464 */         tessellator.a(d16, par3, d13, d5, d7);
/* 2465 */         tessellator.a(d16, par3, d15, d5, d9);
/* 2466 */         tessellator.a(d17, par3, d13, d6, d9);
/* 2467 */         tessellator.a(d17, par3, d15, d6, d7);
/* 2468 */         tessellator.a(d16, par3, d15, d5, d7);
/* 2469 */         tessellator.a(d16, par3, d13, d5, d9);
/*      */       } else {
/*      */         
/* 2472 */         if (par3 > 1 && this.a.i(par2, par3 - 1, par4 - 1)) {
/*      */           
/* 2474 */           tessellator.a(d16, par3, d13, d6, d7);
/* 2475 */           tessellator.a(d16, par3, d14, d6, d8);
/* 2476 */           tessellator.a(d17, par3, d14, d5, d8);
/* 2477 */           tessellator.a(d17, par3, d13, d5, d7);
/* 2478 */           tessellator.a(d16, par3, d14, d6, d7);
/* 2479 */           tessellator.a(d16, par3, d13, d6, d8);
/* 2480 */           tessellator.a(d17, par3, d13, d5, d8);
/* 2481 */           tessellator.a(d17, par3, d14, d5, d7);
/*      */         } 
/* 2483 */         if (par3 > 1 && this.a.i(par2, par3 - 1, par4 + 1))
/*      */         {
/* 2485 */           tessellator.a(d16, par3, d14, d5, d8);
/* 2486 */           tessellator.a(d16, par3, d15, d5, d9);
/* 2487 */           tessellator.a(d17, par3, d15, d6, d9);
/* 2488 */           tessellator.a(d17, par3, d14, d6, d8);
/* 2489 */           tessellator.a(d16, par3, d15, d5, d8);
/* 2490 */           tessellator.a(d16, par3, d14, d5, d9);
/* 2491 */           tessellator.a(d17, par3, d14, d6, d9);
/* 2492 */           tessellator.a(d17, par3, d15, d6, d8);
/*      */         }
/*      */       
/*      */       } 
/* 2496 */     } else if (flag && !flag1) {
/*      */ 
/*      */ 
/*      */       
/* 2500 */       tessellator = tessellatorFront;
/*      */       
/* 2502 */       tessellator.a(d11, (par3 + 1), d13, dz, d3z);
/* 2503 */       tessellator.a(d11, (par3 + 0), d13, dz, d4z);
/* 2504 */       tessellator.a(d11, (par3 + 0), d14, d1z, d4z);
/* 2505 */       tessellator.a(d11, (par3 + 1), d14, d1z, d3z);
/*      */       
/* 2507 */       tessellator.a(d11, (par3 + 1), d14, d1zr, d3zr);
/* 2508 */       tessellator.a(d11, (par3 + 0), d14, d1zr, d4zr);
/* 2509 */       tessellator.a(d11, (par3 + 0), d13, d2zr, d4zr);
/* 2510 */       tessellator.a(d11, (par3 + 1), d13, d2zr, d3zr);
/*      */       
/* 2512 */       tessellator = tessellatorSide;
/*      */       
/* 2514 */       if (!flag3 && !flag2) {
/*      */         
/* 2516 */         tessellator.a(d16, (par3 + 1), d14, d5, d7);
/* 2517 */         tessellator.a(d16, (par3 + 0), d14, d5, d9);
/* 2518 */         tessellator.a(d17, (par3 + 0), d14, d6, d9);
/* 2519 */         tessellator.a(d17, (par3 + 1), d14, d6, d7);
/* 2520 */         tessellator.a(d17, (par3 + 1), d14, d5, d7);
/* 2521 */         tessellator.a(d17, (par3 + 0), d14, d5, d9);
/* 2522 */         tessellator.a(d16, (par3 + 0), d14, d6, d9);
/* 2523 */         tessellator.a(d16, (par3 + 1), d14, d6, d7);
/*      */       } 
/* 2525 */       if (flag4 || (par3 < i - 1 && this.a.i(par2, par3 + 1, par4 - 1))) {
/*      */         
/* 2527 */         tessellator.a(d16, (par3 + 1), d13, d6, d7);
/* 2528 */         tessellator.a(d16, (par3 + 1), d14, d6, d8);
/* 2529 */         tessellator.a(d17, (par3 + 1), d14, d5, d8);
/* 2530 */         tessellator.a(d17, (par3 + 1), d13, d5, d7);
/* 2531 */         tessellator.a(d16, (par3 + 1), d14, d6, d7);
/* 2532 */         tessellator.a(d16, (par3 + 1), d13, d6, d8);
/* 2533 */         tessellator.a(d17, (par3 + 1), d13, d5, d8);
/* 2534 */         tessellator.a(d17, (par3 + 1), d14, d5, d7);
/*      */       } 
/* 2536 */       if (flag5 || (par3 > 1 && this.a.i(par2, par3 - 1, par4 - 1)))
/*      */       {
/* 2538 */         tessellator.a(d16, par3, d13, d6, d7);
/* 2539 */         tessellator.a(d16, par3, d14, d6, d8);
/* 2540 */         tessellator.a(d17, par3, d14, d5, d8);
/* 2541 */         tessellator.a(d17, par3, d13, d5, d7);
/* 2542 */         tessellator.a(d16, par3, d14, d6, d7);
/* 2543 */         tessellator.a(d16, par3, d13, d6, d8);
/* 2544 */         tessellator.a(d17, par3, d13, d5, d8);
/* 2545 */         tessellator.a(d17, par3, d14, d5, d7);
/*      */       }
/*      */     
/* 2548 */     } else if (!flag && flag1) {
/*      */ 
/*      */ 
/*      */       
/* 2552 */       tessellator = tessellatorFront;
/*      */       
/* 2554 */       tessellator.a(d11, (par3 + 1), d14, d1z, d3z);
/* 2555 */       tessellator.a(d11, (par3 + 0), d14, d1z, d4z);
/* 2556 */       tessellator.a(d11, (par3 + 0), d15, d2z, d4z);
/* 2557 */       tessellator.a(d11, (par3 + 1), d15, d2z, d3z);
/*      */       
/* 2559 */       tessellator.a(d11, (par3 + 1), d15, dzr, d3zr);
/* 2560 */       tessellator.a(d11, (par3 + 0), d15, dzr, d4zr);
/* 2561 */       tessellator.a(d11, (par3 + 0), d14, d1zr, d4zr);
/* 2562 */       tessellator.a(d11, (par3 + 1), d14, d1zr, d3zr);
/*      */       
/* 2564 */       tessellator = tessellatorSide;
/*      */       
/* 2566 */       if (!flag3 && !flag2) {
/*      */         
/* 2568 */         tessellator.a(d17, (par3 + 1), d14, d5, d7);
/* 2569 */         tessellator.a(d17, (par3 + 0), d14, d5, d9);
/* 2570 */         tessellator.a(d16, (par3 + 0), d14, d6, d9);
/* 2571 */         tessellator.a(d16, (par3 + 1), d14, d6, d7);
/* 2572 */         tessellator.a(d16, (par3 + 1), d14, d5, d7);
/* 2573 */         tessellator.a(d16, (par3 + 0), d14, d5, d9);
/* 2574 */         tessellator.a(d17, (par3 + 0), d14, d6, d9);
/* 2575 */         tessellator.a(d17, (par3 + 1), d14, d6, d7);
/*      */       } 
/* 2577 */       if (flag4 || (par3 < i - 1 && this.a.i(par2, par3 + 1, par4 + 1))) {
/*      */         
/* 2579 */         tessellator.a(d16, (par3 + 1), d14, d5, d8);
/* 2580 */         tessellator.a(d16, (par3 + 1), d15, d5, d9);
/* 2581 */         tessellator.a(d17, (par3 + 1), d15, d6, d9);
/* 2582 */         tessellator.a(d17, (par3 + 1), d14, d6, d8);
/* 2583 */         tessellator.a(d16, (par3 + 1), d15, d5, d8);
/* 2584 */         tessellator.a(d16, (par3 + 1), d14, d5, d9);
/* 2585 */         tessellator.a(d17, (par3 + 1), d14, d6, d9);
/* 2586 */         tessellator.a(d17, (par3 + 1), d15, d6, d8);
/*      */       } 
/* 2588 */       if (flag5 || (par3 > 1 && this.a.i(par2, par3 - 1, par4 + 1))) {
/*      */         
/* 2590 */         tessellator.a(d16, par3, d14, d5, d8);
/* 2591 */         tessellator.a(d16, par3, d15, d5, d9);
/* 2592 */         tessellator.a(d17, par3, d15, d6, d9);
/* 2593 */         tessellator.a(d17, par3, d14, d6, d8);
/* 2594 */         tessellator.a(d16, par3, d15, d5, d8);
/* 2595 */         tessellator.a(d16, par3, d14, d5, d9);
/* 2596 */         tessellator.a(d17, par3, d14, d6, d9);
/* 2597 */         tessellator.a(d17, par3, d15, d6, d8);
/*      */       } 
/*      */     } 
/*      */     
/* 2601 */     if (Config.isBetterSnow())
/*      */     {
/* 2603 */       if (hasSnowNeighbours(par2, par3, par4))
/*      */       {
/* 2605 */         o(pb.aS, par2, par3, par4);
/*      */       }
/*      */     }
/*      */     
/* 2609 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private int getReverseGlassPaneTexture(int texNum) {
/* 2618 */     int col = texNum % 16;
/* 2619 */     if (col == 1)
/* 2620 */       return texNum + 2; 
/* 2621 */     if (col == 3) {
/* 2622 */       return texNum - 2;
/*      */     }
/* 2624 */     return texNum;
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
/*      */   
/*      */   private int getGlassPaneTexture(boolean linkP, boolean linkN, boolean linkYp, boolean linkYn) {
/* 2638 */     if (linkN && linkP) {
/*      */ 
/*      */       
/* 2641 */       if (linkYp) {
/*      */         
/* 2643 */         if (linkYn)
/*      */         {
/*      */           
/* 2646 */           return 34;
/*      */         }
/*      */ 
/*      */ 
/*      */         
/* 2651 */         return 50;
/*      */       } 
/*      */ 
/*      */ 
/*      */       
/* 2656 */       if (linkYn)
/*      */       {
/*      */         
/* 2659 */         return 18;
/*      */       }
/*      */ 
/*      */ 
/*      */       
/* 2664 */       return 2;
/*      */     } 
/*      */ 
/*      */     
/* 2668 */     if (linkN && !linkP) {
/*      */ 
/*      */       
/* 2671 */       if (linkYp) {
/*      */         
/* 2673 */         if (linkYn)
/*      */         {
/*      */           
/* 2676 */           return 35;
/*      */         }
/*      */ 
/*      */ 
/*      */         
/* 2681 */         return 51;
/*      */       } 
/*      */ 
/*      */ 
/*      */       
/* 2686 */       if (linkYn)
/*      */       {
/*      */         
/* 2689 */         return 19;
/*      */       }
/*      */ 
/*      */ 
/*      */       
/* 2694 */       return 3;
/*      */     } 
/*      */ 
/*      */     
/* 2698 */     if (!linkN && linkP) {
/*      */ 
/*      */       
/* 2701 */       if (linkYp) {
/*      */         
/* 2703 */         if (linkYn)
/*      */         {
/*      */           
/* 2706 */           return 33;
/*      */         }
/*      */ 
/*      */ 
/*      */         
/* 2711 */         return 49;
/*      */       } 
/*      */ 
/*      */ 
/*      */       
/* 2716 */       if (linkYn)
/*      */       {
/*      */         
/* 2719 */         return 17;
/*      */       }
/*      */ 
/*      */ 
/*      */       
/* 2724 */       return 1;
/*      */     } 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */     
/* 2731 */     if (linkYp) {
/*      */       
/* 2733 */       if (linkYn)
/*      */       {
/*      */         
/* 2736 */         return 32;
/*      */       }
/*      */ 
/*      */ 
/*      */       
/* 2741 */       return 48;
/*      */     } 
/*      */ 
/*      */ 
/*      */     
/* 2746 */     if (linkYn)
/*      */     {
/*      */       
/* 2749 */       return 16;
/*      */     }
/*      */ 
/*      */ 
/*      */     
/* 2754 */     return 0;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean j(pb par1Block, int par2, int par3, int par4) {
/* 2765 */     adz tessellator = adz.a;
/* 2766 */     tessellator.b(par1Block.d(this.a, par2, par3, par4));
/* 2767 */     float f = 1.0F;
/*      */     
/* 2769 */     int i = CustomColorizer.getColorMultiplier(par1Block, this.a, par2, par3, par4);
/*      */     
/* 2771 */     float f1 = (i >> 16 & 0xFF) / 255.0F;
/* 2772 */     float f2 = (i >> 8 & 0xFF) / 255.0F;
/* 2773 */     float f3 = (i & 0xFF) / 255.0F;
/* 2774 */     if (lr.a) {
/*      */       
/* 2776 */       float f4 = (f1 * 30.0F + f2 * 59.0F + f3 * 11.0F) / 100.0F;
/* 2777 */       float f5 = (f1 * 30.0F + f2 * 70.0F) / 100.0F;
/* 2778 */       float f6 = (f1 * 30.0F + f3 * 70.0F) / 100.0F;
/* 2779 */       f1 = f4;
/* 2780 */       f2 = f5;
/* 2781 */       f3 = f6;
/*      */     } 
/* 2783 */     tessellator.a(f * f1, f * f2, f * f3);
/* 2784 */     double d = par2;
/* 2785 */     double d1 = par3;
/* 2786 */     double d2 = par4;
/* 2787 */     if (par1Block == pb.X) {
/*      */       
/* 2789 */       long l = (par2 * 3129871) ^ par4 * 116129781L ^ par3;
/* 2790 */       l = l * l * 42317861L + l * 11L;
/* 2791 */       d += (((float)(l >> 16L & 0xFL) / 15.0F) - 0.5D) * 0.5D;
/* 2792 */       d1 += (((float)(l >> 20L & 0xFL) / 15.0F) - 1.0D) * 0.2D;
/* 2793 */       d2 += (((float)(l >> 24L & 0xFL) / 15.0F) - 0.5D) * 0.5D;
/*      */     } 
/* 2795 */     a(par1Block, this.a.e(par2, par3, par4), d, d1, d2);
/*      */     
/* 2797 */     if (Config.isBetterSnow())
/*      */     {
/* 2799 */       if (hasSnowNeighbours(par2, par3, par4))
/*      */       {
/* 2801 */         o(pb.aS, par2, par3, par4);
/*      */       }
/*      */     }
/* 2804 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean k(pb par1Block, int par2, int par3, int par4) {
/* 2812 */     aei blockstem = (aei)par1Block;
/* 2813 */     adz tessellator = adz.a;
/* 2814 */     tessellator.b(blockstem.d(this.a, par2, par3, par4));
/* 2815 */     float f = 1.0F;
/*      */     
/* 2817 */     int i = CustomColorizer.getStemColorMultiplier(blockstem, this.a, par2, par3, par4);
/*      */     
/* 2819 */     float f1 = (i >> 16 & 0xFF) / 255.0F;
/* 2820 */     float f2 = (i >> 8 & 0xFF) / 255.0F;
/* 2821 */     float f3 = (i & 0xFF) / 255.0F;
/* 2822 */     if (lr.a) {
/*      */       
/* 2824 */       float f4 = (f1 * 30.0F + f2 * 59.0F + f3 * 11.0F) / 100.0F;
/* 2825 */       float f5 = (f1 * 30.0F + f2 * 70.0F) / 100.0F;
/* 2826 */       float f6 = (f1 * 30.0F + f3 * 70.0F) / 100.0F;
/* 2827 */       f1 = f4;
/* 2828 */       f2 = f5;
/* 2829 */       f3 = f6;
/*      */     } 
/* 2831 */     tessellator.a(f * f1, f * f2, f * f3);
/* 2832 */     blockstem.a(this.a, par2, par3, par4);
/* 2833 */     int j = blockstem.g(this.a, par2, par3, par4);
/* 2834 */     if (j < 0) {
/*      */       
/* 2836 */       a((pb)blockstem, this.a.e(par2, par3, par4), blockstem.bZ, par2, par3, par4);
/*      */     } else {
/*      */       
/* 2839 */       a((pb)blockstem, this.a.e(par2, par3, par4), 0.5D, par2, par3, par4);
/* 2840 */       a((pb)blockstem, this.a.e(par2, par3, par4), j, blockstem.bZ, par2, par3, par4);
/*      */     } 
/* 2842 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean l(pb par1Block, int par2, int par3, int par4) {
/* 2850 */     adz tessellator = adz.a;
/* 2851 */     tessellator.b(par1Block.d(this.a, par2, par3, par4));
/* 2852 */     tessellator.a(1.0F, 1.0F, 1.0F);
/* 2853 */     b(par1Block, this.a.e(par2, par3, par4), par2, (par3 - 0.0625F), par4);
/* 2854 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(pb par1Block, double par2, double par4, double par6, double par8, double par10) {
/* 2862 */     adz tessellator = adz.a;
/* 2863 */     int i = par1Block.a_(0);
/* 2864 */     if (this.d >= 0)
/*      */     {
/* 2866 */       i = this.d;
/*      */     }
/* 2868 */     int j = (i & 0xF) << 4;
/* 2869 */     int k = i & 0xF0;
/* 2870 */     float f = j / 256.0F;
/* 2871 */     float f1 = (j + 15.99F) / 256.0F;
/* 2872 */     float f2 = k / 256.0F;
/* 2873 */     float f3 = (k + 15.99F) / 256.0F;
/* 2874 */     double d = f + 0.02734375D;
/* 2875 */     double d1 = f2 + 0.0234375D;
/* 2876 */     double d2 = f + 0.03515625D;
/* 2877 */     double d3 = f2 + 0.03125D;
/* 2878 */     par2 += 0.5D;
/* 2879 */     par6 += 0.5D;
/* 2880 */     double d4 = par2 - 0.5D;
/* 2881 */     double d5 = par2 + 0.5D;
/* 2882 */     double d6 = par6 - 0.5D;
/* 2883 */     double d7 = par6 + 0.5D;
/* 2884 */     double d8 = 0.0625D;
/* 2885 */     double d9 = 0.625D;
/* 2886 */     tessellator.a(par2 + par8 * (1.0D - d9) - d8, par4 + d9, par6 + par10 * (1.0D - d9) - d8, d, d1);
/* 2887 */     tessellator.a(par2 + par8 * (1.0D - d9) - d8, par4 + d9, par6 + par10 * (1.0D - d9) + d8, d, d3);
/* 2888 */     tessellator.a(par2 + par8 * (1.0D - d9) + d8, par4 + d9, par6 + par10 * (1.0D - d9) + d8, d2, d3);
/* 2889 */     tessellator.a(par2 + par8 * (1.0D - d9) + d8, par4 + d9, par6 + par10 * (1.0D - d9) - d8, d2, d1);
/* 2890 */     tessellator.a(par2 - d8, par4 + 1.0D, d6, f, f2);
/* 2891 */     tessellator.a(par2 - d8 + par8, par4 + 0.0D, d6 + par10, f, f3);
/* 2892 */     tessellator.a(par2 - d8 + par8, par4 + 0.0D, d7 + par10, f1, f3);
/* 2893 */     tessellator.a(par2 - d8, par4 + 1.0D, d7, f1, f2);
/* 2894 */     tessellator.a(par2 + d8, par4 + 1.0D, d7, f, f2);
/* 2895 */     tessellator.a(par2 + par8 + d8, par4 + 0.0D, d7 + par10, f, f3);
/* 2896 */     tessellator.a(par2 + par8 + d8, par4 + 0.0D, d6 + par10, f1, f3);
/* 2897 */     tessellator.a(par2 + d8, par4 + 1.0D, d6, f1, f2);
/* 2898 */     tessellator.a(d4, par4 + 1.0D, par6 + d8, f, f2);
/* 2899 */     tessellator.a(d4 + par8, par4 + 0.0D, par6 + d8 + par10, f, f3);
/* 2900 */     tessellator.a(d5 + par8, par4 + 0.0D, par6 + d8 + par10, f1, f3);
/* 2901 */     tessellator.a(d5, par4 + 1.0D, par6 + d8, f1, f2);
/* 2902 */     tessellator.a(d5, par4 + 1.0D, par6 - d8, f, f2);
/* 2903 */     tessellator.a(d5 + par8, par4 + 0.0D, par6 - d8 + par10, f, f3);
/* 2904 */     tessellator.a(d4 + par8, par4 + 0.0D, par6 - d8 + par10, f1, f3);
/* 2905 */     tessellator.a(d4, par4 + 1.0D, par6 - d8, f1, f2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(pb par1Block, int par2, double par3, double par5, double par7) {
/* 2913 */     adz tessellator = adz.a;
/* 2914 */     int i = par1Block.a(0, par2);
/* 2915 */     if (this.d >= 0)
/*      */     {
/* 2917 */       i = this.d;
/*      */     }
/*      */     
/* 2920 */     if (Config.isConnectedTextures() && this.d < 0) {
/*      */ 
/*      */       
/* 2923 */       int tex = ConnectedTextures.getConnectedTexture(this.a, par1Block, (int)par3, (int)par5, (int)par7, -1, i);
/* 2924 */       if (tex >= 0) {
/*      */         
/* 2926 */         int ctmTex = tex / 256;
/* 2927 */         tessellator = adz.a.getSubTessellator(ctmTex);
/* 2928 */         i = tex % 256;
/*      */       } 
/*      */     } 
/* 2931 */     int j = (i & 0xF) << 4;
/* 2932 */     int k = i & 0xF0;
/* 2933 */     double d = (j / 256.0F);
/* 2934 */     double d1 = ((j + 15.99F) / 256.0F);
/* 2935 */     double d2 = (k / 256.0F);
/* 2936 */     double d3 = ((k + 15.99F) / 256.0F);
/* 2937 */     double d4 = par3 + 0.5D - 0.45D;
/* 2938 */     double d5 = par3 + 0.5D + 0.45D;
/* 2939 */     double d6 = par7 + 0.5D - 0.45D;
/* 2940 */     double d7 = par7 + 0.5D + 0.45D;
/* 2941 */     tessellator.a(d4, par5 + 1.0D, d6, d, d2);
/* 2942 */     tessellator.a(d4, par5 + 0.0D, d6, d, d3);
/* 2943 */     tessellator.a(d5, par5 + 0.0D, d7, d1, d3);
/* 2944 */     tessellator.a(d5, par5 + 1.0D, d7, d1, d2);
/* 2945 */     tessellator.a(d5, par5 + 1.0D, d7, d, d2);
/* 2946 */     tessellator.a(d5, par5 + 0.0D, d7, d, d3);
/* 2947 */     tessellator.a(d4, par5 + 0.0D, d6, d1, d3);
/* 2948 */     tessellator.a(d4, par5 + 1.0D, d6, d1, d2);
/* 2949 */     tessellator.a(d4, par5 + 1.0D, d7, d, d2);
/* 2950 */     tessellator.a(d4, par5 + 0.0D, d7, d, d3);
/* 2951 */     tessellator.a(d5, par5 + 0.0D, d6, d1, d3);
/* 2952 */     tessellator.a(d5, par5 + 1.0D, d6, d1, d2);
/* 2953 */     tessellator.a(d5, par5 + 1.0D, d6, d, d2);
/* 2954 */     tessellator.a(d5, par5 + 0.0D, d6, d, d3);
/* 2955 */     tessellator.a(d4, par5 + 0.0D, d7, d1, d3);
/* 2956 */     tessellator.a(d4, par5 + 1.0D, d7, d1, d2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(pb par1Block, int par2, double par3, double par5, double par7, double par9) {
/* 2964 */     adz tessellator = adz.a;
/* 2965 */     int i = par1Block.a(0, par2);
/* 2966 */     if (this.d >= 0)
/*      */     {
/* 2968 */       i = this.d;
/*      */     }
/* 2970 */     int j = (i & 0xF) << 4;
/* 2971 */     int k = i & 0xF0;
/* 2972 */     double d = (j / 256.0F);
/* 2973 */     double d1 = ((j + 15.99F) / 256.0F);
/* 2974 */     double d2 = (k / 256.0F);
/* 2975 */     double d3 = (k + 15.989999771118164D * par3) / 256.0D;
/* 2976 */     double d4 = par5 + 0.5D - 0.44999998807907104D;
/* 2977 */     double d5 = par5 + 0.5D + 0.44999998807907104D;
/* 2978 */     double d6 = par9 + 0.5D - 0.44999998807907104D;
/* 2979 */     double d7 = par9 + 0.5D + 0.44999998807907104D;
/* 2980 */     tessellator.a(d4, par7 + par3, d6, d, d2);
/* 2981 */     tessellator.a(d4, par7 + 0.0D, d6, d, d3);
/* 2982 */     tessellator.a(d5, par7 + 0.0D, d7, d1, d3);
/* 2983 */     tessellator.a(d5, par7 + par3, d7, d1, d2);
/* 2984 */     tessellator.a(d5, par7 + par3, d7, d, d2);
/* 2985 */     tessellator.a(d5, par7 + 0.0D, d7, d, d3);
/* 2986 */     tessellator.a(d4, par7 + 0.0D, d6, d1, d3);
/* 2987 */     tessellator.a(d4, par7 + par3, d6, d1, d2);
/* 2988 */     tessellator.a(d4, par7 + par3, d7, d, d2);
/* 2989 */     tessellator.a(d4, par7 + 0.0D, d7, d, d3);
/* 2990 */     tessellator.a(d5, par7 + 0.0D, d6, d1, d3);
/* 2991 */     tessellator.a(d5, par7 + par3, d6, d1, d2);
/* 2992 */     tessellator.a(d5, par7 + par3, d6, d, d2);
/* 2993 */     tessellator.a(d5, par7 + 0.0D, d6, d, d3);
/* 2994 */     tessellator.a(d4, par7 + 0.0D, d7, d1, d3);
/* 2995 */     tessellator.a(d4, par7 + par3, d7, d1, d2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean m(pb par1Block, int par2, int par3, int par4) {
/* 3003 */     adz tessellator = adz.a;
/* 3004 */     int i = par1Block.bN;
/* 3005 */     if (this.d >= 0)
/*      */     {
/* 3007 */       i = this.d;
/*      */     }
/*      */     
/* 3010 */     if (Config.isConnectedTextures() && this.d < 0) {
/*      */       
/* 3012 */       int tex = ConnectedTextures.getConnectedTexture(this.a, par1Block, par2, par3, par4, -1, i);
/* 3013 */       if (tex >= 0) {
/*      */         
/* 3015 */         int ctmTex = tex / 256;
/* 3016 */         tessellator = adz.a.getSubTessellator(ctmTex);
/* 3017 */         i = tex % 256;
/*      */       } 
/*      */     } 
/* 3020 */     int j = (i & 0xF) << 4;
/* 3021 */     int k = i & 0xF0;
/* 3022 */     float f = 0.015625F;
/* 3023 */     double d = (j / 256.0F);
/* 3024 */     double d1 = ((j + 15.99F) / 256.0F);
/* 3025 */     double d2 = (k / 256.0F);
/* 3026 */     double d3 = ((k + 15.99F) / 256.0F);
/* 3027 */     long l = (par2 * 3129871) ^ par4 * 116129781L ^ par3;
/* 3028 */     l = l * l * 42317861L + l * 11L;
/* 3029 */     int i1 = (int)(l >> 16L & 0x3L);
/* 3030 */     tessellator.b(par1Block.d(this.a, par2, par3, par4));
/* 3031 */     float f1 = par2 + 0.5F;
/* 3032 */     float f2 = par4 + 0.5F;
/* 3033 */     float f3 = (i1 & 0x1) * 0.5F * (1 - i1 / 2 % 2 * 2);
/* 3034 */     float f4 = (i1 + 1 & 0x1) * 0.5F * (1 - (i1 + 1) / 2 % 2 * 2);
/*      */     
/* 3036 */     int col = CustomColorizer.getLilypadColor();
/*      */     
/* 3038 */     tessellator.c(col);
/* 3039 */     tessellator.a((f1 + f3 - f4), (par3 + f), (f2 + f3 + f4), d, d2);
/* 3040 */     tessellator.a((f1 + f3 + f4), (par3 + f), (f2 - f3 + f4), d1, d2);
/* 3041 */     tessellator.a((f1 - f3 + f4), (par3 + f), (f2 - f3 - f4), d1, d3);
/* 3042 */     tessellator.a((f1 - f3 - f4), (par3 + f), (f2 + f3 - f4), d, d3);
/* 3043 */     tessellator.c((col & 0xFEFEFE) >> 1);
/* 3044 */     tessellator.a((f1 - f3 - f4), (par3 + f), (f2 + f3 - f4), d, d3);
/* 3045 */     tessellator.a((f1 - f3 + f4), (par3 + f), (f2 - f3 - f4), d1, d3);
/* 3046 */     tessellator.a((f1 + f3 + f4), (par3 + f), (f2 - f3 + f4), d1, d2);
/* 3047 */     tessellator.a((f1 + f3 - f4), (par3 + f), (f2 + f3 + f4), d, d2);
/* 3048 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(pb par1Block, int par2, int par3, double par4, double par6, double par8, double par10) {
/* 3056 */     adz tessellator = adz.a;
/* 3057 */     int i = par1Block.a(0, par2) + 16;
/* 3058 */     if (this.d >= 0)
/*      */     {
/* 3060 */       i = this.d;
/*      */     }
/* 3062 */     int j = (i & 0xF) << 4;
/* 3063 */     int k = i & 0xF0;
/* 3064 */     double d = (j / 256.0F);
/* 3065 */     double d1 = ((j + 15.99F) / 256.0F);
/* 3066 */     double d2 = (k / 256.0F);
/* 3067 */     double d3 = (k + 15.989999771118164D * par4) / 256.0D;
/* 3068 */     double d4 = par6 + 0.5D - 0.5D;
/* 3069 */     double d5 = par6 + 0.5D + 0.5D;
/* 3070 */     double d6 = par10 + 0.5D - 0.5D;
/* 3071 */     double d7 = par10 + 0.5D + 0.5D;
/* 3072 */     double d8 = par6 + 0.5D;
/* 3073 */     double d9 = par10 + 0.5D;
/* 3074 */     if ((par3 + 1) / 2 % 2 == 1) {
/*      */       
/* 3076 */       double d10 = d1;
/* 3077 */       d1 = d;
/* 3078 */       d = d10;
/*      */     } 
/* 3080 */     if (par3 < 2) {
/*      */       
/* 3082 */       tessellator.a(d4, par8 + par4, d9, d, d2);
/* 3083 */       tessellator.a(d4, par8 + 0.0D, d9, d, d3);
/* 3084 */       tessellator.a(d5, par8 + 0.0D, d9, d1, d3);
/* 3085 */       tessellator.a(d5, par8 + par4, d9, d1, d2);
/* 3086 */       tessellator.a(d5, par8 + par4, d9, d1, d2);
/* 3087 */       tessellator.a(d5, par8 + 0.0D, d9, d1, d3);
/* 3088 */       tessellator.a(d4, par8 + 0.0D, d9, d, d3);
/* 3089 */       tessellator.a(d4, par8 + par4, d9, d, d2);
/*      */     } else {
/*      */       
/* 3092 */       tessellator.a(d8, par8 + par4, d7, d, d2);
/* 3093 */       tessellator.a(d8, par8 + 0.0D, d7, d, d3);
/* 3094 */       tessellator.a(d8, par8 + 0.0D, d6, d1, d3);
/* 3095 */       tessellator.a(d8, par8 + par4, d6, d1, d2);
/* 3096 */       tessellator.a(d8, par8 + par4, d6, d1, d2);
/* 3097 */       tessellator.a(d8, par8 + 0.0D, d6, d1, d3);
/* 3098 */       tessellator.a(d8, par8 + 0.0D, d7, d, d3);
/* 3099 */       tessellator.a(d8, par8 + par4, d7, d, d2);
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void b(pb par1Block, int par2, double par3, double par5, double par7) {
/* 3108 */     adz tessellator = adz.a;
/* 3109 */     int i = par1Block.a(0, par2);
/* 3110 */     if (this.d >= 0)
/*      */     {
/* 3112 */       i = this.d;
/*      */     }
/* 3114 */     int j = (i & 0xF) << 4;
/* 3115 */     int k = i & 0xF0;
/* 3116 */     double d = (j / 256.0F);
/* 3117 */     double d1 = ((j + 15.99F) / 256.0F);
/* 3118 */     double d2 = (k / 256.0F);
/* 3119 */     double d3 = ((k + 15.99F) / 256.0F);
/* 3120 */     double d4 = par3 + 0.5D - 0.25D;
/* 3121 */     double d5 = par3 + 0.5D + 0.25D;
/* 3122 */     double d6 = par7 + 0.5D - 0.5D;
/* 3123 */     double d7 = par7 + 0.5D + 0.5D;
/* 3124 */     tessellator.a(d4, par5 + 1.0D, d6, d, d2);
/* 3125 */     tessellator.a(d4, par5 + 0.0D, d6, d, d3);
/* 3126 */     tessellator.a(d4, par5 + 0.0D, d7, d1, d3);
/* 3127 */     tessellator.a(d4, par5 + 1.0D, d7, d1, d2);
/* 3128 */     tessellator.a(d4, par5 + 1.0D, d7, d, d2);
/* 3129 */     tessellator.a(d4, par5 + 0.0D, d7, d, d3);
/* 3130 */     tessellator.a(d4, par5 + 0.0D, d6, d1, d3);
/* 3131 */     tessellator.a(d4, par5 + 1.0D, d6, d1, d2);
/* 3132 */     tessellator.a(d5, par5 + 1.0D, d7, d, d2);
/* 3133 */     tessellator.a(d5, par5 + 0.0D, d7, d, d3);
/* 3134 */     tessellator.a(d5, par5 + 0.0D, d6, d1, d3);
/* 3135 */     tessellator.a(d5, par5 + 1.0D, d6, d1, d2);
/* 3136 */     tessellator.a(d5, par5 + 1.0D, d6, d, d2);
/* 3137 */     tessellator.a(d5, par5 + 0.0D, d6, d, d3);
/* 3138 */     tessellator.a(d5, par5 + 0.0D, d7, d1, d3);
/* 3139 */     tessellator.a(d5, par5 + 1.0D, d7, d1, d2);
/* 3140 */     d4 = par3 + 0.5D - 0.5D;
/* 3141 */     d5 = par3 + 0.5D + 0.5D;
/* 3142 */     d6 = par7 + 0.5D - 0.25D;
/* 3143 */     d7 = par7 + 0.5D + 0.25D;
/* 3144 */     tessellator.a(d4, par5 + 1.0D, d6, d, d2);
/* 3145 */     tessellator.a(d4, par5 + 0.0D, d6, d, d3);
/* 3146 */     tessellator.a(d5, par5 + 0.0D, d6, d1, d3);
/* 3147 */     tessellator.a(d5, par5 + 1.0D, d6, d1, d2);
/* 3148 */     tessellator.a(d5, par5 + 1.0D, d6, d, d2);
/* 3149 */     tessellator.a(d5, par5 + 0.0D, d6, d, d3);
/* 3150 */     tessellator.a(d4, par5 + 0.0D, d6, d1, d3);
/* 3151 */     tessellator.a(d4, par5 + 1.0D, d6, d1, d2);
/* 3152 */     tessellator.a(d5, par5 + 1.0D, d7, d, d2);
/* 3153 */     tessellator.a(d5, par5 + 0.0D, d7, d, d3);
/* 3154 */     tessellator.a(d4, par5 + 0.0D, d7, d1, d3);
/* 3155 */     tessellator.a(d4, par5 + 1.0D, d7, d1, d2);
/* 3156 */     tessellator.a(d4, par5 + 1.0D, d7, d, d2);
/* 3157 */     tessellator.a(d4, par5 + 0.0D, d7, d, d3);
/* 3158 */     tessellator.a(d5, par5 + 0.0D, d7, d1, d3);
/* 3159 */     tessellator.a(d5, par5 + 1.0D, d7, d1, d2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean n(pb par1Block, int par2, int par3, int par4) {
/* 3167 */     adz tessellator = adz.a;
/*      */     
/* 3169 */     int i = CustomColorizer.getFluidColor(par1Block, this.a, par2, par3, par4);
/*      */     
/* 3171 */     float f = (i >> 16 & 0xFF) / 255.0F;
/* 3172 */     float f1 = (i >> 8 & 0xFF) / 255.0F;
/* 3173 */     float f2 = (i & 0xFF) / 255.0F;
/* 3174 */     boolean flag = par1Block.a(this.a, par2, par3 + 1, par4, 1);
/* 3175 */     boolean flag1 = par1Block.a(this.a, par2, par3 - 1, par4, 0);
/* 3176 */     boolean[] aflag = new boolean[4];
/* 3177 */     aflag[0] = par1Block.a(this.a, par2, par3, par4 - 1, 2);
/* 3178 */     aflag[1] = par1Block.a(this.a, par2, par3, par4 + 1, 3);
/* 3179 */     aflag[2] = par1Block.a(this.a, par2 - 1, par3, par4, 4);
/* 3180 */     aflag[3] = par1Block.a(this.a, par2 + 1, par3, par4, 5);
/* 3181 */     if (!flag && !flag1 && !aflag[0] && !aflag[1] && !aflag[2] && !aflag[3])
/*      */     {
/* 3183 */       return false;
/*      */     }
/* 3185 */     boolean flag2 = false;
/* 3186 */     float f3 = 0.5F;
/* 3187 */     float f4 = 1.0F;
/* 3188 */     float f5 = 0.8F;
/* 3189 */     float f6 = 0.6F;
/* 3190 */     double d = 0.0D;
/* 3191 */     double d1 = 1.0D;
/* 3192 */     acn material = par1Block.cd;
/* 3193 */     int j = this.a.e(par2, par3, par4);
/* 3194 */     double d2 = a(par2, par3, par4, material);
/* 3195 */     double d3 = a(par2, par3, par4 + 1, material);
/* 3196 */     double d4 = a(par2 + 1, par3, par4 + 1, material);
/* 3197 */     double d5 = a(par2 + 1, par3, par4, material);
/*      */     
/* 3199 */     double d6 = 0.0010000000474974513D;
/* 3200 */     if (this.f || flag) {
/*      */       
/* 3202 */       flag2 = true;
/* 3203 */       int k = par1Block.a(1, j);
/* 3204 */       float f8 = (float)agv.a(this.a, par2, par3, par4, material);
/* 3205 */       if (f8 > -999.0F)
/*      */       {
/* 3207 */         k = par1Block.a(2, j);
/*      */       }
/* 3209 */       d2 -= d6;
/* 3210 */       d3 -= d6;
/* 3211 */       d4 -= d6;
/* 3212 */       d5 -= d6;
/* 3213 */       int j1 = (k & 0xF) << 4;
/* 3214 */       int l1 = k & 0xF0;
/* 3215 */       double d7 = (j1 + 8.0D) / 256.0D;
/* 3216 */       double d8 = (l1 + 8.0D) / 256.0D;
/* 3217 */       if (f8 < -999.0F) {
/*      */         
/* 3219 */         f8 = 0.0F;
/*      */       } else {
/*      */         
/* 3222 */         d7 = ((j1 + 16) / 256.0F);
/* 3223 */         d8 = ((l1 + 16) / 256.0F);
/*      */       } 
/* 3225 */       double d10 = (gk.a(f8) * 8.0F) / 256.0D;
/* 3226 */       double d12 = (gk.b(f8) * 8.0F) / 256.0D;
/* 3227 */       tessellator.b(par1Block.d(this.a, par2, par3, par4));
/* 3228 */       float f9 = 1.0F;
/* 3229 */       tessellator.a(f4 * f9 * f, f4 * f9 * f1, f4 * f9 * f2);
/* 3230 */       tessellator.a((par2 + 0), par3 + d2, (par4 + 0), d7 - d12 - d10, d8 - d12 + d10);
/* 3231 */       tessellator.a((par2 + 0), par3 + d3, (par4 + 1), d7 - d12 + d10, d8 + d12 + d10);
/* 3232 */       tessellator.a((par2 + 1), par3 + d4, (par4 + 1), d7 + d12 + d10, d8 + d12 - d10);
/* 3233 */       tessellator.a((par2 + 1), par3 + d5, (par4 + 0), d7 + d12 - d10, d8 - d12 - d10);
/*      */     } 
/* 3235 */     if (this.f || flag1) {
/*      */       
/* 3237 */       tessellator.b(par1Block.d(this.a, par2, par3 - 1, par4));
/* 3238 */       float f7 = 1.0F;
/*      */       
/* 3240 */       tessellator.a(f3 * f7 * f, f3 * f7 * f1, f3 * f7 * f2);
/* 3241 */       a(par1Block, par2, par3 + d6, par4, par1Block.a_(0));
/* 3242 */       flag2 = true;
/*      */     } 
/* 3244 */     for (int l = 0; l < 4; l++) {
/*      */       
/* 3246 */       int i1 = par2;
/* 3247 */       int k1 = par3;
/* 3248 */       int i2 = par4;
/* 3249 */       if (l == 0)
/*      */       {
/* 3251 */         i2--;
/*      */       }
/* 3253 */       if (l == 1)
/*      */       {
/* 3255 */         i2++;
/*      */       }
/* 3257 */       if (l == 2)
/*      */       {
/* 3259 */         i1--;
/*      */       }
/* 3261 */       if (l == 3)
/*      */       {
/* 3263 */         i1++;
/*      */       }
/* 3265 */       int j2 = par1Block.a(l + 2, j);
/* 3266 */       int k2 = (j2 & 0xF) << 4;
/* 3267 */       int l2 = j2 & 0xF0;
/* 3268 */       if (this.f || aflag[l]) {
/*      */         double d9, d11, d13, d14, d15, d16;
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */         
/* 3278 */         if (l == 0) {
/*      */           
/* 3280 */           d9 = d2;
/* 3281 */           d11 = d5;
/* 3282 */           d13 = par2;
/* 3283 */           d15 = (par2 + 1);
/* 3284 */           d14 = par4 + d6;
/* 3285 */           d16 = par4 + d6;
/*      */         }
/* 3287 */         else if (l == 1) {
/*      */           
/* 3289 */           d9 = d4;
/* 3290 */           d11 = d3;
/* 3291 */           d13 = (par2 + 1);
/* 3292 */           d15 = par2;
/* 3293 */           d14 = (par4 + 1) - d6;
/* 3294 */           d16 = (par4 + 1) - d6;
/*      */         }
/* 3296 */         else if (l == 2) {
/*      */           
/* 3298 */           d9 = d3;
/* 3299 */           d11 = d2;
/* 3300 */           d13 = par2 + d6;
/* 3301 */           d15 = par2 + d6;
/* 3302 */           d14 = (par4 + 1);
/* 3303 */           d16 = par4;
/*      */         } else {
/*      */           
/* 3306 */           d9 = d5;
/* 3307 */           d11 = d4;
/* 3308 */           d13 = (par2 + 1) - d6;
/* 3309 */           d15 = (par2 + 1) - d6;
/* 3310 */           d14 = par4;
/* 3311 */           d16 = (par4 + 1);
/*      */         } 
/* 3313 */         flag2 = true;
/* 3314 */         double d17 = ((k2 + 0) / 256.0F);
/* 3315 */         double d18 = ((k2 + 16) - 0.01D) / 256.0D;
/* 3316 */         double d19 = (l2 + (1.0D - d9) * 16.0D) / 256.0D;
/* 3317 */         double d20 = (l2 + (1.0D - d11) * 16.0D) / 256.0D;
/* 3318 */         double d21 = ((l2 + 16) - 0.01D) / 256.0D;
/* 3319 */         tessellator.b(par1Block.d(this.a, i1, k1, i2));
/* 3320 */         float f10 = 1.0F;
/* 3321 */         if (l < 2) {
/*      */           
/* 3323 */           f10 *= f5;
/*      */         } else {
/*      */           
/* 3326 */           f10 *= f6;
/*      */         } 
/* 3328 */         tessellator.a(f4 * f10 * f, f4 * f10 * f1, f4 * f10 * f2);
/* 3329 */         tessellator.a(d13, par3 + d9, d14, d17, d19);
/* 3330 */         tessellator.a(d15, par3 + d11, d16, d18, d20);
/* 3331 */         tessellator.a(d15, (par3 + 0), d16, d18, d21);
/* 3332 */         tessellator.a(d13, (par3 + 0), d14, d17, d21);
/*      */       } 
/*      */     } 
/* 3335 */     par1Block.bW = d;
/* 3336 */     par1Block.bZ = d1;
/* 3337 */     return flag2;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public float a(int par1, int par2, int par3, acn par4Material) {
/* 3346 */     int i = 0;
/* 3347 */     float f = 0.0F;
/* 3348 */     for (int j = 0; j < 4; j++) {
/*      */       
/* 3350 */       int k = par1 - (j & 0x1);
/* 3351 */       int l = par2;
/* 3352 */       int i1 = par3 - (j >> 1 & 0x1);
/* 3353 */       if (this.a.f(k, l + 1, i1) == par4Material)
/*      */       {
/* 3355 */         return 1.0F;
/*      */       }
/* 3357 */       acn material = this.a.f(k, l, i1);
/* 3358 */       if (material == par4Material) {
/*      */         
/* 3360 */         int j1 = this.a.e(k, l, i1);
/* 3361 */         if (j1 >= 8 || j1 == 0) {
/*      */           
/* 3363 */           f += agv.e(j1) * 10.0F;
/* 3364 */           i += 10;
/*      */         } 
/* 3366 */         f += agv.e(j1);
/* 3367 */         i++;
/*      */       
/*      */       }
/* 3370 */       else if (!material.a()) {
/*      */         
/* 3372 */         f++;
/* 3373 */         i++;
/*      */       } 
/*      */     } 
/*      */     
/* 3377 */     return 1.0F - f / i;
/*      */   }
/*      */ 
/*      */   
/*      */   public void a(pb par1Block, xd par2World, int par3, int par4, int par5) {
/* 3382 */     float f = 0.5F;
/* 3383 */     float f1 = 1.0F;
/* 3384 */     float f2 = 0.8F;
/* 3385 */     float f3 = 0.6F;
/* 3386 */     adz tessellator = adz.a;
/* 3387 */     tessellator.b();
/* 3388 */     tessellator.b(par1Block.d((ali)par2World, par3, par4, par5));
/* 3389 */     float f4 = 1.0F;
/* 3390 */     float f5 = 1.0F;
/* 3391 */     if (f5 < f4)
/*      */     {
/* 3393 */       f5 = f4;
/*      */     }
/* 3395 */     tessellator.a(f * f5, f * f5, f * f5);
/* 3396 */     a(par1Block, -0.5D, -0.5D, -0.5D, par1Block.a_(0));
/* 3397 */     f5 = 1.0F;
/* 3398 */     if (f5 < f4)
/*      */     {
/* 3400 */       f5 = f4;
/*      */     }
/* 3402 */     tessellator.a(f1 * f5, f1 * f5, f1 * f5);
/* 3403 */     b(par1Block, -0.5D, -0.5D, -0.5D, par1Block.a_(1));
/* 3404 */     f5 = 1.0F;
/* 3405 */     if (f5 < f4)
/*      */     {
/* 3407 */       f5 = f4;
/*      */     }
/* 3409 */     tessellator.a(f2 * f5, f2 * f5, f2 * f5);
/* 3410 */     c(par1Block, -0.5D, -0.5D, -0.5D, par1Block.a_(2));
/* 3411 */     f5 = 1.0F;
/* 3412 */     if (f5 < f4)
/*      */     {
/* 3414 */       f5 = f4;
/*      */     }
/* 3416 */     tessellator.a(f2 * f5, f2 * f5, f2 * f5);
/* 3417 */     d(par1Block, -0.5D, -0.5D, -0.5D, par1Block.a_(3));
/* 3418 */     f5 = 1.0F;
/* 3419 */     if (f5 < f4)
/*      */     {
/* 3421 */       f5 = f4;
/*      */     }
/* 3423 */     tessellator.a(f3 * f5, f3 * f5, f3 * f5);
/* 3424 */     e(par1Block, -0.5D, -0.5D, -0.5D, par1Block.a_(4));
/* 3425 */     f5 = 1.0F;
/* 3426 */     if (f5 < f4)
/*      */     {
/* 3428 */       f5 = f4;
/*      */     }
/* 3430 */     tessellator.a(f3 * f5, f3 * f5, f3 * f5);
/* 3431 */     f(par1Block, -0.5D, -0.5D, -0.5D, par1Block.a_(5));
/* 3432 */     tessellator.a();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean o(pb par1Block, int par2, int par3, int par4) {
/* 3441 */     int i = CustomColorizer.getColorMultiplier(par1Block, this.a, par2, par3, par4);
/*      */     
/* 3443 */     float f = (i >> 16 & 0xFF) / 255.0F;
/* 3444 */     float f1 = (i >> 8 & 0xFF) / 255.0F;
/* 3445 */     float f2 = (i & 0xFF) / 255.0F;
/* 3446 */     if (lr.a) {
/*      */       
/* 3448 */       float f3 = (f * 30.0F + f1 * 59.0F + f2 * 11.0F) / 100.0F;
/* 3449 */       float f4 = (f * 30.0F + f1 * 70.0F) / 100.0F;
/* 3450 */       float f5 = (f * 30.0F + f2 * 70.0F) / 100.0F;
/* 3451 */       f = f3;
/* 3452 */       f1 = f4;
/* 3453 */       f2 = f5;
/*      */     } 
/* 3455 */     if (Minecraft.t() && pb.q[par1Block.bO] == 0)
/*      */     {
/* 3457 */       return a(par1Block, par2, par3, par4, f, f1, f2);
/*      */     }
/*      */     
/* 3460 */     return b(par1Block, par2, par3, par4, f, f1, f2);
/*      */   }
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean a(pb par1Block, int par2, int par3, int par4, float par5, float par6, float par7) {
/* 3466 */     this.m = true;
/*      */     
/* 3468 */     boolean defaultTexture = adz.a.defaultTexture;
/*      */     
/* 3470 */     boolean betterGrass = (Config.isBetterGrass() && defaultTexture);
/*      */     
/* 3472 */     if (par1Block.getClass() == xj.class) {
/* 3473 */       this.ai = 0;
/*      */     } else {
/* 3475 */       this.ai = 1;
/*      */     } 
/* 3477 */     boolean flag = false;
/* 3478 */     boolean flag1 = true;
/* 3479 */     boolean flag2 = true;
/* 3480 */     boolean flag3 = true;
/* 3481 */     boolean flag4 = true;
/* 3482 */     boolean flag5 = true;
/* 3483 */     boolean flag6 = true;
/*      */     
/* 3485 */     this.aoLightValuesCalculated = false;
/*      */     
/* 3487 */     int i = par1Block.d(this.a, par2, par3, par4);
/*      */     
/* 3489 */     adz tessellator = adz.a;
/* 3490 */     tessellator.b(983055);
/*      */     
/* 3492 */     if (par1Block.bN == 3)
/*      */     {
/* 3494 */       flag1 = flag3 = flag4 = flag5 = flag6 = false;
/*      */     }
/* 3496 */     if (this.d >= 0)
/*      */     {
/* 3498 */       flag1 = flag3 = flag4 = flag5 = flag6 = false;
/*      */     }
/* 3500 */     if (this.f || par1Block.a(this.a, par2, par3 - 1, par4, 0)) {
/*      */       float f1, f8, f15, f22;
/*      */       
/* 3503 */       if (!this.aoLightValuesCalculated) {
/* 3504 */         calculateAoLightValues(par1Block, par2, par3, par4);
/*      */       }
/* 3506 */       int k = i;
/* 3507 */       if (par1Block.bW <= 0.0D)
/*      */       {
/* 3509 */         k = par1Block.d(this.a, par2, par3 - 1, par4);
/*      */       }
/*      */ 
/*      */ 
/*      */ 
/*      */       
/* 3515 */       if (this.ai > 0) {
/*      */         
/* 3517 */         if (par1Block.bW <= 0.0D)
/*      */         {
/* 3519 */           par3--;
/*      */         }
/* 3521 */         this.P = par1Block.d(this.a, par2 - 1, par3, par4);
/* 3522 */         this.R = par1Block.d(this.a, par2, par3, par4 - 1);
/* 3523 */         this.S = par1Block.d(this.a, par2, par3, par4 + 1);
/* 3524 */         this.U = par1Block.d(this.a, par2 + 1, par3, par4);
/* 3525 */         this.v = getAmbientOcclusionLightValue(this.a, par2 - 1, par3, par4);
/* 3526 */         this.x = getAmbientOcclusionLightValue(this.a, par2, par3, par4 - 1);
/* 3527 */         this.y = getAmbientOcclusionLightValue(this.a, par2, par3, par4 + 1);
/* 3528 */         this.A = getAmbientOcclusionLightValue(this.a, par2 + 1, par3, par4);
/* 3529 */         if (this.aH || this.aJ) {
/*      */           
/* 3531 */           this.u = getAmbientOcclusionLightValue(this.a, par2 - 1, par3, par4 - 1);
/* 3532 */           this.O = par1Block.d(this.a, par2 - 1, par3, par4 - 1);
/*      */         } else {
/*      */           
/* 3535 */           this.u = this.v;
/* 3536 */           this.O = this.P;
/*      */         } 
/* 3538 */         if (this.aK || this.aJ) {
/*      */           
/* 3540 */           this.w = getAmbientOcclusionLightValue(this.a, par2 - 1, par3, par4 + 1);
/* 3541 */           this.Q = par1Block.d(this.a, par2 - 1, par3, par4 + 1);
/*      */         } else {
/*      */           
/* 3544 */           this.w = this.v;
/* 3545 */           this.Q = this.P;
/*      */         } 
/* 3547 */         if (this.aH || this.aI) {
/*      */           
/* 3549 */           this.z = getAmbientOcclusionLightValue(this.a, par2 + 1, par3, par4 - 1);
/* 3550 */           this.T = par1Block.d(this.a, par2 + 1, par3, par4 - 1);
/*      */         } else {
/*      */           
/* 3553 */           this.z = this.A;
/* 3554 */           this.T = this.U;
/*      */         } 
/* 3556 */         if (this.aK || this.aI) {
/*      */           
/* 3558 */           this.B = getAmbientOcclusionLightValue(this.a, par2 + 1, par3, par4 + 1);
/* 3559 */           this.V = par1Block.d(this.a, par2 + 1, par3, par4 + 1);
/*      */         } else {
/*      */           
/* 3562 */           this.B = this.A;
/* 3563 */           this.V = this.U;
/*      */         } 
/* 3565 */         if (par1Block.bW <= 0.0D)
/*      */         {
/* 3567 */           par3++;
/*      */         }
/* 3569 */         f1 = (this.w + this.v + this.y + this.p) / 4.0F;
/* 3570 */         f22 = (this.y + this.p + this.B + this.A) / 4.0F;
/* 3571 */         f15 = (this.p + this.x + this.A + this.z) / 4.0F;
/* 3572 */         f8 = (this.v + this.u + this.p + this.x) / 4.0F;
/* 3573 */         this.aj = a(this.Q, this.P, this.S, k);
/* 3574 */         this.am = a(this.S, this.V, this.U, k);
/* 3575 */         this.al = a(this.R, this.U, this.T, k);
/* 3576 */         this.ak = a(this.P, this.O, this.R, k);
/*      */       } else {
/*      */         
/* 3579 */         f1 = f8 = f15 = f22 = this.p;
/* 3580 */         this.aj = this.ak = this.al = this.am = k;
/*      */       } 
/* 3582 */       this.an = this.ao = this.ap = this.aq = (flag1 ? par5 : 1.0F) * 0.5F;
/* 3583 */       this.ar = this.as = this.at = this.au = (flag1 ? par6 : 1.0F) * 0.5F;
/* 3584 */       this.av = this.aw = this.ax = this.ay = (flag1 ? par7 : 1.0F) * 0.5F;
/* 3585 */       this.an *= f1;
/* 3586 */       this.ar *= f1;
/* 3587 */       this.av *= f1;
/* 3588 */       this.ao *= f8;
/* 3589 */       this.as *= f8;
/* 3590 */       this.aw *= f8;
/* 3591 */       this.ap *= f15;
/* 3592 */       this.at *= f15;
/* 3593 */       this.ax *= f15;
/* 3594 */       this.aq *= f22;
/* 3595 */       this.au *= f22;
/* 3596 */       this.ay *= f22;
/* 3597 */       a(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 0));
/* 3598 */       flag = true;
/*      */     } 
/* 3600 */     if (this.f || par1Block.a(this.a, par2, par3 + 1, par4, 1)) {
/*      */       float f2, f9, f16, f23;
/*      */       
/* 3603 */       if (!this.aoLightValuesCalculated) {
/* 3604 */         calculateAoLightValues(par1Block, par2, par3, par4);
/*      */       }
/* 3606 */       int j1 = i;
/* 3607 */       if (par1Block.bZ >= 1.0D)
/*      */       {
/* 3609 */         j1 = par1Block.d(this.a, par2, par3 + 1, par4);
/*      */       }
/*      */ 
/*      */ 
/*      */ 
/*      */       
/* 3615 */       if (this.ai > 0) {
/*      */         
/* 3617 */         if (par1Block.bZ >= 1.0D)
/*      */         {
/* 3619 */           par3++;
/*      */         }
/* 3621 */         this.X = par1Block.d(this.a, par2 - 1, par3, par4);
/* 3622 */         this.ab = par1Block.d(this.a, par2 + 1, par3, par4);
/* 3623 */         this.Z = par1Block.d(this.a, par2, par3, par4 - 1);
/* 3624 */         this.ac = par1Block.d(this.a, par2, par3, par4 + 1);
/* 3625 */         this.D = getAmbientOcclusionLightValue(this.a, par2 - 1, par3, par4);
/* 3626 */         this.H = getAmbientOcclusionLightValue(this.a, par2 + 1, par3, par4);
/* 3627 */         this.F = getAmbientOcclusionLightValue(this.a, par2, par3, par4 - 1);
/* 3628 */         this.I = getAmbientOcclusionLightValue(this.a, par2, par3, par4 + 1);
/* 3629 */         if (this.az || this.aB) {
/*      */           
/* 3631 */           this.C = getAmbientOcclusionLightValue(this.a, par2 - 1, par3, par4 - 1);
/* 3632 */           this.W = par1Block.d(this.a, par2 - 1, par3, par4 - 1);
/*      */         } else {
/*      */           
/* 3635 */           this.C = this.D;
/* 3636 */           this.W = this.X;
/*      */         } 
/* 3638 */         if (this.az || this.aA) {
/*      */           
/* 3640 */           this.G = getAmbientOcclusionLightValue(this.a, par2 + 1, par3, par4 - 1);
/* 3641 */           this.aa = par1Block.d(this.a, par2 + 1, par3, par4 - 1);
/*      */         } else {
/*      */           
/* 3644 */           this.G = this.H;
/* 3645 */           this.aa = this.ab;
/*      */         } 
/* 3647 */         if (this.aC || this.aB) {
/*      */           
/* 3649 */           this.E = getAmbientOcclusionLightValue(this.a, par2 - 1, par3, par4 + 1);
/* 3650 */           this.Y = par1Block.d(this.a, par2 - 1, par3, par4 + 1);
/*      */         } else {
/*      */           
/* 3653 */           this.E = this.D;
/* 3654 */           this.Y = this.X;
/*      */         } 
/* 3656 */         if (this.aC || this.aA) {
/*      */           
/* 3658 */           this.J = getAmbientOcclusionLightValue(this.a, par2 + 1, par3, par4 + 1);
/* 3659 */           this.ad = par1Block.d(this.a, par2 + 1, par3, par4 + 1);
/*      */         } else {
/*      */           
/* 3662 */           this.J = this.H;
/* 3663 */           this.ad = this.ab;
/*      */         } 
/* 3665 */         if (par1Block.bZ >= 1.0D)
/*      */         {
/* 3667 */           par3--;
/*      */         }
/* 3669 */         f23 = (this.E + this.D + this.I + this.s) / 4.0F;
/* 3670 */         f2 = (this.I + this.s + this.J + this.H) / 4.0F;
/* 3671 */         f9 = (this.s + this.F + this.H + this.G) / 4.0F;
/* 3672 */         f16 = (this.D + this.C + this.s + this.F) / 4.0F;
/* 3673 */         this.am = a(this.Y, this.X, this.ac, j1);
/* 3674 */         this.aj = a(this.ac, this.ad, this.ab, j1);
/* 3675 */         this.ak = a(this.Z, this.ab, this.aa, j1);
/* 3676 */         this.al = a(this.X, this.W, this.Z, j1);
/*      */       } else {
/*      */         
/* 3679 */         f2 = f9 = f16 = f23 = this.s;
/* 3680 */         this.aj = this.ak = this.al = this.am = j1;
/*      */       } 
/* 3682 */       this.an = this.ao = this.ap = this.aq = flag2 ? par5 : 1.0F;
/* 3683 */       this.ar = this.as = this.at = this.au = flag2 ? par6 : 1.0F;
/* 3684 */       this.av = this.aw = this.ax = this.ay = flag2 ? par7 : 1.0F;
/* 3685 */       this.an *= f2;
/* 3686 */       this.ar *= f2;
/* 3687 */       this.av *= f2;
/* 3688 */       this.ao *= f9;
/* 3689 */       this.as *= f9;
/* 3690 */       this.aw *= f9;
/* 3691 */       this.ap *= f16;
/* 3692 */       this.at *= f16;
/* 3693 */       this.ax *= f16;
/* 3694 */       this.aq *= f23;
/* 3695 */       this.au *= f23;
/* 3696 */       this.ay *= f23;
/* 3697 */       b(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 1));
/* 3698 */       flag = true;
/*      */     } 
/* 3700 */     if (this.f || par1Block.a(this.a, par2, par3, par4 - 1, 2)) {
/*      */       float f3, f10, f17, f24;
/*      */       
/* 3703 */       if (!this.aoLightValuesCalculated) {
/* 3704 */         calculateAoLightValues(par1Block, par2, par3, par4);
/*      */       }
/* 3706 */       int l = i;
/* 3707 */       if (par1Block.bX <= 0.0D)
/*      */       {
/* 3709 */         l = par1Block.d(this.a, par2, par3, par4 - 1);
/*      */       }
/*      */ 
/*      */ 
/*      */ 
/*      */       
/* 3715 */       if (this.ai > 0) {
/*      */         
/* 3717 */         if (par1Block.bX <= 0.0D)
/*      */         {
/* 3719 */           par4--;
/*      */         }
/* 3721 */         this.K = getAmbientOcclusionLightValue(this.a, par2 - 1, par3, par4);
/* 3722 */         this.x = getAmbientOcclusionLightValue(this.a, par2, par3 - 1, par4);
/* 3723 */         this.F = getAmbientOcclusionLightValue(this.a, par2, par3 + 1, par4);
/* 3724 */         this.L = getAmbientOcclusionLightValue(this.a, par2 + 1, par3, par4);
/* 3725 */         this.ae = par1Block.d(this.a, par2 - 1, par3, par4);
/* 3726 */         this.R = par1Block.d(this.a, par2, par3 - 1, par4);
/* 3727 */         this.Z = par1Block.d(this.a, par2, par3 + 1, par4);
/* 3728 */         this.af = par1Block.d(this.a, par2 + 1, par3, par4);
/* 3729 */         if (this.aD || this.aH) {
/*      */           
/* 3731 */           this.u = getAmbientOcclusionLightValue(this.a, par2 - 1, par3 - 1, par4);
/* 3732 */           this.O = par1Block.d(this.a, par2 - 1, par3 - 1, par4);
/*      */         } else {
/*      */           
/* 3735 */           this.u = this.K;
/* 3736 */           this.O = this.ae;
/*      */         } 
/* 3738 */         if (this.aD || this.az) {
/*      */           
/* 3740 */           this.C = getAmbientOcclusionLightValue(this.a, par2 - 1, par3 + 1, par4);
/* 3741 */           this.W = par1Block.d(this.a, par2 - 1, par3 + 1, par4);
/*      */         } else {
/*      */           
/* 3744 */           this.C = this.K;
/* 3745 */           this.W = this.ae;
/*      */         } 
/* 3747 */         if (this.aG || this.aH) {
/*      */           
/* 3749 */           this.z = getAmbientOcclusionLightValue(this.a, par2 + 1, par3 - 1, par4);
/* 3750 */           this.T = par1Block.d(this.a, par2 + 1, par3 - 1, par4);
/*      */         } else {
/*      */           
/* 3753 */           this.z = this.L;
/* 3754 */           this.T = this.af;
/*      */         } 
/* 3756 */         if (this.aG || this.az) {
/*      */           
/* 3758 */           this.G = getAmbientOcclusionLightValue(this.a, par2 + 1, par3 + 1, par4);
/* 3759 */           this.aa = par1Block.d(this.a, par2 + 1, par3 + 1, par4);
/*      */         } else {
/*      */           
/* 3762 */           this.G = this.L;
/* 3763 */           this.aa = this.af;
/*      */         } 
/* 3765 */         if (par1Block.bX <= 0.0D)
/*      */         {
/* 3767 */           par4++;
/*      */         }
/* 3769 */         f3 = (this.K + this.C + this.q + this.F) / 4.0F;
/* 3770 */         f10 = (this.q + this.F + this.L + this.G) / 4.0F;
/* 3771 */         f17 = (this.x + this.q + this.z + this.L) / 4.0F;
/* 3772 */         f24 = (this.u + this.K + this.x + this.q) / 4.0F;
/* 3773 */         this.aj = a(this.ae, this.W, this.Z, l);
/* 3774 */         this.ak = a(this.Z, this.af, this.aa, l);
/* 3775 */         this.al = a(this.R, this.T, this.af, l);
/* 3776 */         this.am = a(this.O, this.ae, this.R, l);
/*      */       } else {
/*      */         
/* 3779 */         f3 = f10 = f17 = f24 = this.q;
/* 3780 */         this.aj = this.ak = this.al = this.am = l;
/*      */       } 
/* 3782 */       this.an = this.ao = this.ap = this.aq = (flag3 ? par5 : 1.0F) * 0.8F;
/* 3783 */       this.ar = this.as = this.at = this.au = (flag3 ? par6 : 1.0F) * 0.8F;
/* 3784 */       this.av = this.aw = this.ax = this.ay = (flag3 ? par7 : 1.0F) * 0.8F;
/* 3785 */       this.an *= f3;
/* 3786 */       this.ar *= f3;
/* 3787 */       this.av *= f3;
/* 3788 */       this.ao *= f10;
/* 3789 */       this.as *= f10;
/* 3790 */       this.aw *= f10;
/* 3791 */       this.ap *= f17;
/* 3792 */       this.at *= f17;
/* 3793 */       this.ax *= f17;
/* 3794 */       this.aq *= f24;
/* 3795 */       this.au *= f24;
/* 3796 */       this.ay *= f24;
/* 3797 */       int l1 = par1Block.d(this.a, par2, par3, par4, 2);
/*      */       
/* 3799 */       if (betterGrass) {
/* 3800 */         l1 = fixAoSideGrassTexture(l1, par2, par3, par4, 2, par5, par6, par7);
/*      */       }
/* 3802 */       c(par1Block, par2, par3, par4, l1);
/*      */       
/* 3804 */       if (defaultTexture && b && l1 == 3 && this.d < 0) {
/*      */         
/* 3806 */         this.an *= par5;
/* 3807 */         this.ao *= par5;
/* 3808 */         this.ap *= par5;
/* 3809 */         this.aq *= par5;
/* 3810 */         this.ar *= par6;
/* 3811 */         this.as *= par6;
/* 3812 */         this.at *= par6;
/* 3813 */         this.au *= par6;
/* 3814 */         this.av *= par7;
/* 3815 */         this.aw *= par7;
/* 3816 */         this.ax *= par7;
/* 3817 */         this.ay *= par7;
/* 3818 */         c(par1Block, par2, par3, par4, 38);
/*      */       } 
/* 3820 */       flag = true;
/*      */     } 
/* 3822 */     if (this.f || par1Block.a(this.a, par2, par3, par4 + 1, 3)) {
/*      */       float f4, f11, f18, f25;
/*      */       
/* 3825 */       if (!this.aoLightValuesCalculated) {
/* 3826 */         calculateAoLightValues(par1Block, par2, par3, par4);
/*      */       }
/* 3828 */       int k1 = i;
/* 3829 */       if (par1Block.ca >= 1.0D)
/*      */       {
/* 3831 */         k1 = par1Block.d(this.a, par2, par3, par4 + 1);
/*      */       }
/*      */ 
/*      */ 
/*      */ 
/*      */       
/* 3837 */       if (this.ai > 0) {
/*      */         
/* 3839 */         if (par1Block.ca >= 1.0D)
/*      */         {
/* 3841 */           par4++;
/*      */         }
/* 3843 */         this.M = getAmbientOcclusionLightValue(this.a, par2 - 1, par3, par4);
/* 3844 */         this.N = getAmbientOcclusionLightValue(this.a, par2 + 1, par3, par4);
/* 3845 */         this.y = getAmbientOcclusionLightValue(this.a, par2, par3 - 1, par4);
/* 3846 */         this.I = getAmbientOcclusionLightValue(this.a, par2, par3 + 1, par4);
/* 3847 */         this.ag = par1Block.d(this.a, par2 - 1, par3, par4);
/* 3848 */         this.ah = par1Block.d(this.a, par2 + 1, par3, par4);
/* 3849 */         this.S = par1Block.d(this.a, par2, par3 - 1, par4);
/* 3850 */         this.ac = par1Block.d(this.a, par2, par3 + 1, par4);
/* 3851 */         if (this.aF || this.aK) {
/*      */           
/* 3853 */           this.w = getAmbientOcclusionLightValue(this.a, par2 - 1, par3 - 1, par4);
/* 3854 */           this.Q = par1Block.d(this.a, par2 - 1, par3 - 1, par4);
/*      */         } else {
/*      */           
/* 3857 */           this.w = this.M;
/* 3858 */           this.Q = this.ag;
/*      */         } 
/* 3860 */         if (this.aF || this.aC) {
/*      */           
/* 3862 */           this.E = getAmbientOcclusionLightValue(this.a, par2 - 1, par3 + 1, par4);
/* 3863 */           this.Y = par1Block.d(this.a, par2 - 1, par3 + 1, par4);
/*      */         } else {
/*      */           
/* 3866 */           this.E = this.M;
/* 3867 */           this.Y = this.ag;
/*      */         } 
/* 3869 */         if (this.aE || this.aK) {
/*      */           
/* 3871 */           this.B = getAmbientOcclusionLightValue(this.a, par2 + 1, par3 - 1, par4);
/* 3872 */           this.V = par1Block.d(this.a, par2 + 1, par3 - 1, par4);
/*      */         } else {
/*      */           
/* 3875 */           this.B = this.N;
/* 3876 */           this.V = this.ah;
/*      */         } 
/* 3878 */         if (this.aE || this.aC) {
/*      */           
/* 3880 */           this.J = getAmbientOcclusionLightValue(this.a, par2 + 1, par3 + 1, par4);
/* 3881 */           this.ad = par1Block.d(this.a, par2 + 1, par3 + 1, par4);
/*      */         } else {
/*      */           
/* 3884 */           this.J = this.N;
/* 3885 */           this.ad = this.ah;
/*      */         } 
/* 3887 */         if (par1Block.ca >= 1.0D)
/*      */         {
/* 3889 */           par4--;
/*      */         }
/* 3891 */         f4 = (this.M + this.E + this.t + this.I) / 4.0F;
/* 3892 */         f25 = (this.t + this.I + this.N + this.J) / 4.0F;
/* 3893 */         f18 = (this.y + this.t + this.B + this.N) / 4.0F;
/* 3894 */         f11 = (this.w + this.M + this.y + this.t) / 4.0F;
/* 3895 */         this.aj = a(this.ag, this.Y, this.ac, k1);
/* 3896 */         this.am = a(this.ac, this.ah, this.ad, k1);
/* 3897 */         this.al = a(this.S, this.V, this.ah, k1);
/* 3898 */         this.ak = a(this.Q, this.ag, this.S, k1);
/*      */       } else {
/*      */         
/* 3901 */         f4 = f11 = f18 = f25 = this.t;
/* 3902 */         this.aj = this.ak = this.al = this.am = k1;
/*      */       } 
/* 3904 */       this.an = this.ao = this.ap = this.aq = (flag4 ? par5 : 1.0F) * 0.8F;
/* 3905 */       this.ar = this.as = this.at = this.au = (flag4 ? par6 : 1.0F) * 0.8F;
/* 3906 */       this.av = this.aw = this.ax = this.ay = (flag4 ? par7 : 1.0F) * 0.8F;
/* 3907 */       this.an *= f4;
/* 3908 */       this.ar *= f4;
/* 3909 */       this.av *= f4;
/* 3910 */       this.ao *= f11;
/* 3911 */       this.as *= f11;
/* 3912 */       this.aw *= f11;
/* 3913 */       this.ap *= f18;
/* 3914 */       this.at *= f18;
/* 3915 */       this.ax *= f18;
/* 3916 */       this.aq *= f25;
/* 3917 */       this.au *= f25;
/* 3918 */       this.ay *= f25;
/* 3919 */       int i2 = par1Block.d(this.a, par2, par3, par4, 3);
/*      */       
/* 3921 */       if (betterGrass) {
/* 3922 */         i2 = fixAoSideGrassTexture(i2, par2, par3, par4, 3, par5, par6, par7);
/*      */       }
/* 3924 */       d(par1Block, par2, par3, par4, i2);
/*      */       
/* 3926 */       if (defaultTexture && b && i2 == 3 && this.d < 0) {
/*      */         
/* 3928 */         this.an *= par5;
/* 3929 */         this.ao *= par5;
/* 3930 */         this.ap *= par5;
/* 3931 */         this.aq *= par5;
/* 3932 */         this.ar *= par6;
/* 3933 */         this.as *= par6;
/* 3934 */         this.at *= par6;
/* 3935 */         this.au *= par6;
/* 3936 */         this.av *= par7;
/* 3937 */         this.aw *= par7;
/* 3938 */         this.ax *= par7;
/* 3939 */         this.ay *= par7;
/* 3940 */         d(par1Block, par2, par3, par4, 38);
/*      */       } 
/* 3942 */       flag = true;
/*      */     } 
/* 3944 */     if (this.f || par1Block.a(this.a, par2 - 1, par3, par4, 4)) {
/*      */       float f5, f12, f19, f26;
/*      */       
/* 3947 */       if (!this.aoLightValuesCalculated) {
/* 3948 */         calculateAoLightValues(par1Block, par2, par3, par4);
/*      */       }
/* 3950 */       int j = i;
/* 3951 */       if (par1Block.bV <= 0.0D)
/*      */       {
/* 3953 */         j = par1Block.d(this.a, par2 - 1, par3, par4);
/*      */       }
/*      */ 
/*      */ 
/*      */ 
/*      */       
/* 3959 */       if (this.ai > 0) {
/*      */         
/* 3961 */         if (par1Block.bV <= 0.0D)
/*      */         {
/* 3963 */           par2--;
/*      */         }
/* 3965 */         this.v = getAmbientOcclusionLightValue(this.a, par2, par3 - 1, par4);
/* 3966 */         this.K = getAmbientOcclusionLightValue(this.a, par2, par3, par4 - 1);
/* 3967 */         this.M = getAmbientOcclusionLightValue(this.a, par2, par3, par4 + 1);
/* 3968 */         this.D = getAmbientOcclusionLightValue(this.a, par2, par3 + 1, par4);
/* 3969 */         this.P = par1Block.d(this.a, par2, par3 - 1, par4);
/* 3970 */         this.ae = par1Block.d(this.a, par2, par3, par4 - 1);
/* 3971 */         this.ag = par1Block.d(this.a, par2, par3, par4 + 1);
/* 3972 */         this.X = par1Block.d(this.a, par2, par3 + 1, par4);
/* 3973 */         if (this.aD || this.aJ) {
/*      */           
/* 3975 */           this.u = getAmbientOcclusionLightValue(this.a, par2, par3 - 1, par4 - 1);
/* 3976 */           this.O = par1Block.d(this.a, par2, par3 - 1, par4 - 1);
/*      */         } else {
/*      */           
/* 3979 */           this.u = this.K;
/* 3980 */           this.O = this.ae;
/*      */         } 
/* 3982 */         if (this.aF || this.aJ) {
/*      */           
/* 3984 */           this.w = getAmbientOcclusionLightValue(this.a, par2, par3 - 1, par4 + 1);
/* 3985 */           this.Q = par1Block.d(this.a, par2, par3 - 1, par4 + 1);
/*      */         } else {
/*      */           
/* 3988 */           this.w = this.M;
/* 3989 */           this.Q = this.ag;
/*      */         } 
/* 3991 */         if (this.aD || this.aB) {
/*      */           
/* 3993 */           this.C = getAmbientOcclusionLightValue(this.a, par2, par3 + 1, par4 - 1);
/* 3994 */           this.W = par1Block.d(this.a, par2, par3 + 1, par4 - 1);
/*      */         } else {
/*      */           
/* 3997 */           this.C = this.K;
/* 3998 */           this.W = this.ae;
/*      */         } 
/* 4000 */         if (this.aF || this.aB) {
/*      */           
/* 4002 */           this.E = getAmbientOcclusionLightValue(this.a, par2, par3 + 1, par4 + 1);
/* 4003 */           this.Y = par1Block.d(this.a, par2, par3 + 1, par4 + 1);
/*      */         } else {
/*      */           
/* 4006 */           this.E = this.M;
/* 4007 */           this.Y = this.ag;
/*      */         } 
/* 4009 */         if (par1Block.bV <= 0.0D)
/*      */         {
/* 4011 */           par2++;
/*      */         }
/* 4013 */         f26 = (this.v + this.w + this.o + this.M) / 4.0F;
/* 4014 */         f5 = (this.o + this.M + this.D + this.E) / 4.0F;
/* 4015 */         f12 = (this.K + this.o + this.C + this.D) / 4.0F;
/* 4016 */         f19 = (this.u + this.v + this.K + this.o) / 4.0F;
/* 4017 */         this.am = a(this.P, this.Q, this.ag, j);
/* 4018 */         this.aj = a(this.ag, this.X, this.Y, j);
/* 4019 */         this.ak = a(this.ae, this.W, this.X, j);
/* 4020 */         this.al = a(this.O, this.P, this.ae, j);
/*      */       } else {
/*      */         
/* 4023 */         f5 = f12 = f19 = f26 = this.o;
/* 4024 */         this.aj = this.ak = this.al = this.am = j;
/*      */       } 
/* 4026 */       this.an = this.ao = this.ap = this.aq = (flag5 ? par5 : 1.0F) * 0.6F;
/* 4027 */       this.ar = this.as = this.at = this.au = (flag5 ? par6 : 1.0F) * 0.6F;
/* 4028 */       this.av = this.aw = this.ax = this.ay = (flag5 ? par7 : 1.0F) * 0.6F;
/* 4029 */       this.an *= f5;
/* 4030 */       this.ar *= f5;
/* 4031 */       this.av *= f5;
/* 4032 */       this.ao *= f12;
/* 4033 */       this.as *= f12;
/* 4034 */       this.aw *= f12;
/* 4035 */       this.ap *= f19;
/* 4036 */       this.at *= f19;
/* 4037 */       this.ax *= f19;
/* 4038 */       this.aq *= f26;
/* 4039 */       this.au *= f26;
/* 4040 */       this.ay *= f26;
/* 4041 */       int j2 = par1Block.d(this.a, par2, par3, par4, 4);
/*      */       
/* 4043 */       if (betterGrass) {
/* 4044 */         j2 = fixAoSideGrassTexture(j2, par2, par3, par4, 4, par5, par6, par7);
/*      */       }
/* 4046 */       e(par1Block, par2, par3, par4, j2);
/*      */       
/* 4048 */       if (defaultTexture && b && j2 == 3 && this.d < 0) {
/*      */         
/* 4050 */         this.an *= par5;
/* 4051 */         this.ao *= par5;
/* 4052 */         this.ap *= par5;
/* 4053 */         this.aq *= par5;
/* 4054 */         this.ar *= par6;
/* 4055 */         this.as *= par6;
/* 4056 */         this.at *= par6;
/* 4057 */         this.au *= par6;
/* 4058 */         this.av *= par7;
/* 4059 */         this.aw *= par7;
/* 4060 */         this.ax *= par7;
/* 4061 */         this.ay *= par7;
/* 4062 */         e(par1Block, par2, par3, par4, 38);
/*      */       } 
/* 4064 */       flag = true;
/*      */     } 
/* 4066 */     if (this.f || par1Block.a(this.a, par2 + 1, par3, par4, 5)) {
/*      */       float f6, f13, f20, f27;
/*      */       
/* 4069 */       if (!this.aoLightValuesCalculated) {
/* 4070 */         calculateAoLightValues(par1Block, par2, par3, par4);
/*      */       }
/* 4072 */       int i1 = i;
/* 4073 */       if (par1Block.bY >= 1.0D)
/*      */       {
/* 4075 */         i1 = par1Block.d(this.a, par2 + 1, par3, par4);
/*      */       }
/*      */ 
/*      */ 
/*      */ 
/*      */       
/* 4081 */       if (this.ai > 0) {
/*      */         
/* 4083 */         if (par1Block.bY >= 1.0D)
/*      */         {
/* 4085 */           par2++;
/*      */         }
/* 4087 */         this.A = getAmbientOcclusionLightValue(this.a, par2, par3 - 1, par4);
/* 4088 */         this.L = getAmbientOcclusionLightValue(this.a, par2, par3, par4 - 1);
/* 4089 */         this.N = getAmbientOcclusionLightValue(this.a, par2, par3, par4 + 1);
/* 4090 */         this.H = getAmbientOcclusionLightValue(this.a, par2, par3 + 1, par4);
/* 4091 */         this.U = par1Block.d(this.a, par2, par3 - 1, par4);
/* 4092 */         this.af = par1Block.d(this.a, par2, par3, par4 - 1);
/* 4093 */         this.ah = par1Block.d(this.a, par2, par3, par4 + 1);
/* 4094 */         this.ab = par1Block.d(this.a, par2, par3 + 1, par4);
/* 4095 */         if (this.aI || this.aG) {
/*      */           
/* 4097 */           this.z = getAmbientOcclusionLightValue(this.a, par2, par3 - 1, par4 - 1);
/* 4098 */           this.T = par1Block.d(this.a, par2, par3 - 1, par4 - 1);
/*      */         } else {
/*      */           
/* 4101 */           this.z = this.L;
/* 4102 */           this.T = this.af;
/*      */         } 
/* 4104 */         if (this.aI || this.aE) {
/*      */           
/* 4106 */           this.B = getAmbientOcclusionLightValue(this.a, par2, par3 - 1, par4 + 1);
/* 4107 */           this.V = par1Block.d(this.a, par2, par3 - 1, par4 + 1);
/*      */         } else {
/*      */           
/* 4110 */           this.B = this.N;
/* 4111 */           this.V = this.ah;
/*      */         } 
/* 4113 */         if (this.aA || this.aG) {
/*      */           
/* 4115 */           this.G = getAmbientOcclusionLightValue(this.a, par2, par3 + 1, par4 - 1);
/* 4116 */           this.aa = par1Block.d(this.a, par2, par3 + 1, par4 - 1);
/*      */         } else {
/*      */           
/* 4119 */           this.G = this.L;
/* 4120 */           this.aa = this.af;
/*      */         } 
/* 4122 */         if (this.aA || this.aE) {
/*      */           
/* 4124 */           this.J = getAmbientOcclusionLightValue(this.a, par2, par3 + 1, par4 + 1);
/* 4125 */           this.ad = par1Block.d(this.a, par2, par3 + 1, par4 + 1);
/*      */         } else {
/*      */           
/* 4128 */           this.J = this.N;
/* 4129 */           this.ad = this.ah;
/*      */         } 
/* 4131 */         if (par1Block.bY >= 1.0D)
/*      */         {
/* 4133 */           par2--;
/*      */         }
/* 4135 */         f6 = (this.A + this.B + this.r + this.N) / 4.0F;
/* 4136 */         f27 = (this.r + this.N + this.H + this.J) / 4.0F;
/* 4137 */         f20 = (this.L + this.r + this.G + this.H) / 4.0F;
/* 4138 */         f13 = (this.z + this.A + this.L + this.r) / 4.0F;
/* 4139 */         this.aj = a(this.U, this.V, this.ah, i1);
/* 4140 */         this.am = a(this.ah, this.ab, this.ad, i1);
/* 4141 */         this.al = a(this.af, this.aa, this.ab, i1);
/* 4142 */         this.ak = a(this.T, this.U, this.af, i1);
/*      */       } else {
/*      */         
/* 4145 */         f6 = f13 = f20 = f27 = this.r;
/* 4146 */         this.aj = this.ak = this.al = this.am = i1;
/*      */       } 
/* 4148 */       this.an = this.ao = this.ap = this.aq = (flag6 ? par5 : 1.0F) * 0.6F;
/* 4149 */       this.ar = this.as = this.at = this.au = (flag6 ? par6 : 1.0F) * 0.6F;
/* 4150 */       this.av = this.aw = this.ax = this.ay = (flag6 ? par7 : 1.0F) * 0.6F;
/* 4151 */       this.an *= f6;
/* 4152 */       this.ar *= f6;
/* 4153 */       this.av *= f6;
/* 4154 */       this.ao *= f13;
/* 4155 */       this.as *= f13;
/* 4156 */       this.aw *= f13;
/* 4157 */       this.ap *= f20;
/* 4158 */       this.at *= f20;
/* 4159 */       this.ax *= f20;
/* 4160 */       this.aq *= f27;
/* 4161 */       this.au *= f27;
/* 4162 */       this.ay *= f27;
/* 4163 */       int k2 = par1Block.d(this.a, par2, par3, par4, 5);
/*      */       
/* 4165 */       if (betterGrass) {
/* 4166 */         k2 = fixAoSideGrassTexture(k2, par2, par3, par4, 5, par5, par6, par7);
/*      */       }
/* 4168 */       f(par1Block, par2, par3, par4, k2);
/*      */       
/* 4170 */       if (defaultTexture && b && k2 == 3 && this.d < 0) {
/*      */         
/* 4172 */         this.an *= par5;
/* 4173 */         this.ao *= par5;
/* 4174 */         this.ap *= par5;
/* 4175 */         this.aq *= par5;
/* 4176 */         this.ar *= par6;
/* 4177 */         this.as *= par6;
/* 4178 */         this.at *= par6;
/* 4179 */         this.au *= par6;
/* 4180 */         this.av *= par7;
/* 4181 */         this.aw *= par7;
/* 4182 */         this.ax *= par7;
/* 4183 */         this.ay *= par7;
/* 4184 */         f(par1Block, par2, par3, par4, 38);
/*      */       } 
/* 4186 */       flag = true;
/*      */     } 
/* 4188 */     this.m = false;
/* 4189 */     return flag;
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
/*      */   private void calculateAoLightValues(pb block, int i, int j, int k) {
/* 4202 */     this.o = getAmbientOcclusionLightValue(this.a, i - 1, j, k);
/* 4203 */     this.p = getAmbientOcclusionLightValue(this.a, i, j - 1, k);
/* 4204 */     this.q = getAmbientOcclusionLightValue(this.a, i, j, k - 1);
/* 4205 */     this.r = getAmbientOcclusionLightValue(this.a, i + 1, j, k);
/* 4206 */     this.s = getAmbientOcclusionLightValue(this.a, i, j + 1, k);
/* 4207 */     this.t = getAmbientOcclusionLightValue(this.a, i, j, k + 1);
/*      */     
/* 4209 */     this.aA = pb.p[this.a.a(i + 1, j + 1, k)];
/* 4210 */     this.aI = pb.p[this.a.a(i + 1, j - 1, k)];
/* 4211 */     this.aE = pb.p[this.a.a(i + 1, j, k + 1)];
/* 4212 */     this.aG = pb.p[this.a.a(i + 1, j, k - 1)];
/* 4213 */     this.aB = pb.p[this.a.a(i - 1, j + 1, k)];
/* 4214 */     this.aJ = pb.p[this.a.a(i - 1, j - 1, k)];
/* 4215 */     this.aD = pb.p[this.a.a(i - 1, j, k - 1)];
/* 4216 */     this.aF = pb.p[this.a.a(i - 1, j, k + 1)];
/* 4217 */     this.aC = pb.p[this.a.a(i, j + 1, k + 1)];
/* 4218 */     this.az = pb.p[this.a.a(i, j + 1, k - 1)];
/* 4219 */     this.aK = pb.p[this.a.a(i, j - 1, k + 1)];
/* 4220 */     this.aH = pb.p[this.a.a(i, j - 1, k - 1)];
/*      */     
/* 4222 */     this.aoLightValuesCalculated = true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private float getAmbientOcclusionLightValue(ali iBlockAccess, int i, int j, int k) {
/* 4231 */     pb block = pb.m[iBlockAccess.a(i, j, k)];
/* 4232 */     if (block == null) {
/* 4233 */       return 1.0F;
/*      */     }
/* 4235 */     if (block.getClass() == xj.class) {
/* 4236 */       return 1.0F;
/*      */     }
/* 4238 */     return (block.cd.c() && block.b()) ? this.aoLightValueOpaque : 1.0F;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private int fixAoSideGrassTexture(int tex, int x, int y, int z, int side, float f, float f1, float f2) {
/* 4249 */     if (tex == 3 || tex == 77) {
/*      */ 
/*      */       
/* 4252 */       tex = Config.getSideGrassTexture(this.a, x, y, z, side, tex);
/*      */       
/* 4254 */       if (tex == 0) {
/*      */ 
/*      */         
/* 4257 */         this.an *= f;
/* 4258 */         this.ao *= f;
/* 4259 */         this.ap *= f;
/* 4260 */         this.aq *= f;
/* 4261 */         this.ar *= f1;
/* 4262 */         this.as *= f1;
/* 4263 */         this.at *= f1;
/* 4264 */         this.au *= f1;
/* 4265 */         this.av *= f2;
/* 4266 */         this.aw *= f2;
/* 4267 */         this.ax *= f2;
/* 4268 */         this.ay *= f2;
/*      */       } 
/*      */     } 
/*      */     
/* 4272 */     if (tex == 68)
/*      */     {
/*      */       
/* 4275 */       tex = Config.getSideSnowGrassTexture(this.a, x, y, z, side);
/*      */     }
/* 4277 */     return tex;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public int a(int par1, int par2, int par3, int par4) {
/* 4285 */     if (par1 == 0)
/*      */     {
/* 4287 */       par1 = par4;
/*      */     }
/* 4289 */     if (par2 == 0)
/*      */     {
/* 4291 */       par2 = par4;
/*      */     }
/* 4293 */     if (par3 == 0)
/*      */     {
/* 4295 */       par3 = par4;
/*      */     }
/* 4297 */     return par1 + par2 + par3 + par4 >> 2 & 0xFF00FF;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean b(pb par1Block, int par2, int par3, int par4, float par5, float par6, float par7) {
/* 4305 */     this.m = false;
/*      */     
/* 4307 */     boolean defaultTexture = adz.a.defaultTexture;
/*      */     
/* 4309 */     boolean betterGrass = (Config.isBetterGrass() && defaultTexture);
/*      */     
/* 4311 */     adz tessellator = adz.a;
/* 4312 */     boolean flag = false;
/* 4313 */     float f = 0.5F;
/* 4314 */     float f1 = 1.0F;
/* 4315 */     float f2 = 0.8F;
/* 4316 */     float f3 = 0.6F;
/* 4317 */     float f4 = f1 * par5;
/* 4318 */     float f5 = f1 * par6;
/* 4319 */     float f6 = f1 * par7;
/* 4320 */     float f7 = f;
/* 4321 */     float f8 = f2;
/* 4322 */     float f9 = f3;
/* 4323 */     float f10 = f;
/* 4324 */     float f11 = f2;
/* 4325 */     float f12 = f3;
/* 4326 */     float f13 = f;
/* 4327 */     float f14 = f2;
/* 4328 */     float f15 = f3;
/* 4329 */     if (par1Block != pb.u) {
/*      */       
/* 4331 */       f7 *= par5;
/* 4332 */       f8 *= par5;
/* 4333 */       f9 *= par5;
/* 4334 */       f10 *= par6;
/* 4335 */       f11 *= par6;
/* 4336 */       f12 *= par6;
/* 4337 */       f13 *= par7;
/* 4338 */       f14 *= par7;
/* 4339 */       f15 *= par7;
/*      */     } 
/* 4341 */     int i = par1Block.d(this.a, par2, par3, par4);
/* 4342 */     if (this.f || par1Block.a(this.a, par2, par3 - 1, par4, 0)) {
/*      */       
/* 4344 */       tessellator.b((par1Block.bW <= 0.0D) ? par1Block.d(this.a, par2, par3 - 1, par4) : i);
/* 4345 */       tessellator.a(f7, f10, f13);
/* 4346 */       a(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 0));
/* 4347 */       flag = true;
/*      */     } 
/* 4349 */     if (this.f || par1Block.a(this.a, par2, par3 + 1, par4, 1)) {
/*      */       
/* 4351 */       tessellator.b((par1Block.bZ >= 1.0D) ? par1Block.d(this.a, par2, par3 + 1, par4) : i);
/* 4352 */       tessellator.a(f4, f5, f6);
/* 4353 */       b(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 1));
/* 4354 */       flag = true;
/*      */     } 
/* 4356 */     if (this.f || par1Block.a(this.a, par2, par3, par4 - 1, 2)) {
/*      */       
/* 4358 */       tessellator.b((par1Block.bX <= 0.0D) ? par1Block.d(this.a, par2, par3, par4 - 1) : i);
/* 4359 */       tessellator.a(f8, f11, f14);
/* 4360 */       int j = par1Block.d(this.a, par2, par3, par4, 2);
/*      */       
/* 4362 */       if (betterGrass) {
/*      */ 
/*      */         
/* 4365 */         if (j == 3 || j == 77) {
/*      */           
/* 4367 */           j = Config.getSideGrassTexture(this.a, par2, par3, par4, 2, j);
/*      */           
/* 4369 */           if (j == 0)
/*      */           {
/*      */             
/* 4372 */             tessellator.a(f8 * par5, f11 * par6, f14 * par7);
/*      */           }
/*      */         } 
/*      */         
/* 4376 */         if (j == 68)
/*      */         {
/*      */           
/* 4379 */           j = Config.getSideSnowGrassTexture(this.a, par2, par3, par4, 2);
/*      */         }
/*      */       } 
/*      */       
/* 4383 */       c(par1Block, par2, par3, par4, j);
/* 4384 */       if (defaultTexture && b && j == 3 && this.d < 0) {
/*      */         
/* 4386 */         tessellator.a(f8 * par5, f11 * par6, f14 * par7);
/* 4387 */         c(par1Block, par2, par3, par4, 38);
/*      */       } 
/* 4389 */       flag = true;
/*      */     } 
/* 4391 */     if (this.f || par1Block.a(this.a, par2, par3, par4 + 1, 3)) {
/*      */       
/* 4393 */       tessellator.b((par1Block.ca >= 1.0D) ? par1Block.d(this.a, par2, par3, par4 + 1) : i);
/* 4394 */       tessellator.a(f8, f11, f14);
/* 4395 */       int k = par1Block.d(this.a, par2, par3, par4, 3);
/*      */       
/* 4397 */       if (betterGrass) {
/*      */         
/* 4399 */         if (k == 3 || k == 77) {
/*      */ 
/*      */           
/* 4402 */           k = Config.getSideGrassTexture(this.a, par2, par3, par4, 3, k);
/*      */           
/* 4404 */           if (k == 0)
/*      */           {
/*      */             
/* 4407 */             tessellator.a(f8 * par5, f11 * par6, f14 * par7);
/*      */           }
/*      */         } 
/*      */         
/* 4411 */         if (k == 68)
/*      */         {
/*      */           
/* 4414 */           k = Config.getSideSnowGrassTexture(this.a, par2, par3, par4, 3);
/*      */         }
/*      */       } 
/*      */       
/* 4418 */       d(par1Block, par2, par3, par4, k);
/* 4419 */       if (defaultTexture && b && k == 3 && this.d < 0) {
/*      */         
/* 4421 */         tessellator.a(f8 * par5, f11 * par6, f14 * par7);
/* 4422 */         d(par1Block, par2, par3, par4, 38);
/*      */       } 
/* 4424 */       flag = true;
/*      */     } 
/* 4426 */     if (this.f || par1Block.a(this.a, par2 - 1, par3, par4, 4)) {
/*      */       
/* 4428 */       tessellator.b((par1Block.bV <= 0.0D) ? par1Block.d(this.a, par2 - 1, par3, par4) : i);
/* 4429 */       tessellator.a(f9, f12, f15);
/* 4430 */       int l = par1Block.d(this.a, par2, par3, par4, 4);
/*      */       
/* 4432 */       if (betterGrass) {
/*      */         
/* 4434 */         if (l == 3 || l == 77) {
/*      */ 
/*      */           
/* 4437 */           l = Config.getSideGrassTexture(this.a, par2, par3, par4, 4, l);
/*      */           
/* 4439 */           if (l == 0)
/*      */           {
/*      */             
/* 4442 */             tessellator.a(f8 * par5, f11 * par6, f14 * par7);
/*      */           }
/*      */         } 
/*      */         
/* 4446 */         if (l == 68)
/*      */         {
/*      */           
/* 4449 */           l = Config.getSideSnowGrassTexture(this.a, par2, par3, par4, 4);
/*      */         }
/*      */       } 
/*      */       
/* 4453 */       e(par1Block, par2, par3, par4, l);
/* 4454 */       if (defaultTexture && b && l == 3 && this.d < 0) {
/*      */         
/* 4456 */         tessellator.a(f9 * par5, f12 * par6, f15 * par7);
/* 4457 */         e(par1Block, par2, par3, par4, 38);
/*      */       } 
/* 4459 */       flag = true;
/*      */     } 
/* 4461 */     if (this.f || par1Block.a(this.a, par2 + 1, par3, par4, 5)) {
/*      */       
/* 4463 */       tessellator.b((par1Block.bY >= 1.0D) ? par1Block.d(this.a, par2 + 1, par3, par4) : i);
/* 4464 */       tessellator.a(f9, f12, f15);
/* 4465 */       int i1 = par1Block.d(this.a, par2, par3, par4, 5);
/*      */       
/* 4467 */       if (betterGrass) {
/*      */         
/* 4469 */         if (i1 == 3 || i1 == 77) {
/*      */ 
/*      */           
/* 4472 */           i1 = Config.getSideGrassTexture(this.a, par2, par3, par4, 5, i1);
/*      */           
/* 4474 */           if (i1 == 0)
/*      */           {
/*      */             
/* 4477 */             tessellator.a(f8 * par5, f11 * par6, f14 * par7);
/*      */           }
/*      */         } 
/*      */         
/* 4481 */         if (i1 == 68)
/*      */         {
/*      */           
/* 4484 */           i1 = Config.getSideSnowGrassTexture(this.a, par2, par3, par4, 5);
/*      */         }
/*      */       } 
/*      */       
/* 4488 */       f(par1Block, par2, par3, par4, i1);
/* 4489 */       if (defaultTexture && b && i1 == 3 && this.d < 0) {
/*      */         
/* 4491 */         tessellator.a(f9 * par5, f12 * par6, f15 * par7);
/* 4492 */         f(par1Block, par2, par3, par4, 38);
/*      */       } 
/* 4494 */       flag = true;
/*      */     } 
/* 4496 */     return flag;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean p(pb par1Block, int par2, int par3, int par4) {
/* 4504 */     int i = par1Block.c(this.a, par2, par3, par4);
/* 4505 */     float f = (i >> 16 & 0xFF) / 255.0F;
/* 4506 */     float f1 = (i >> 8 & 0xFF) / 255.0F;
/* 4507 */     float f2 = (i & 0xFF) / 255.0F;
/* 4508 */     if (lr.a) {
/*      */       
/* 4510 */       float f3 = (f * 30.0F + f1 * 59.0F + f2 * 11.0F) / 100.0F;
/* 4511 */       float f4 = (f * 30.0F + f1 * 70.0F) / 100.0F;
/* 4512 */       float f5 = (f * 30.0F + f2 * 70.0F) / 100.0F;
/* 4513 */       f = f3;
/* 4514 */       f1 = f4;
/* 4515 */       f2 = f5;
/*      */     } 
/* 4517 */     return c(par1Block, par2, par3, par4, f, f1, f2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean c(pb par1Block, int par2, int par3, int par4, float par5, float par6, float par7) {
/* 4525 */     adz tessellator = adz.a;
/* 4526 */     boolean flag = false;
/* 4527 */     float f = 0.5F;
/* 4528 */     float f1 = 1.0F;
/* 4529 */     float f2 = 0.8F;
/* 4530 */     float f3 = 0.6F;
/* 4531 */     float f4 = f * par5;
/* 4532 */     float f5 = f1 * par5;
/* 4533 */     float f6 = f2 * par5;
/* 4534 */     float f7 = f3 * par5;
/* 4535 */     float f8 = f * par6;
/* 4536 */     float f9 = f1 * par6;
/* 4537 */     float f10 = f2 * par6;
/* 4538 */     float f11 = f3 * par6;
/* 4539 */     float f12 = f * par7;
/* 4540 */     float f13 = f1 * par7;
/* 4541 */     float f14 = f2 * par7;
/* 4542 */     float f15 = f3 * par7;
/* 4543 */     float f16 = 0.0625F;
/* 4544 */     int i = par1Block.d(this.a, par2, par3, par4);
/* 4545 */     if (this.f || par1Block.a(this.a, par2, par3 - 1, par4, 0)) {
/*      */       
/* 4547 */       tessellator.b((par1Block.bW <= 0.0D) ? par1Block.d(this.a, par2, par3 - 1, par4) : i);
/* 4548 */       tessellator.a(f4, f8, f12);
/* 4549 */       a(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 0));
/* 4550 */       flag = true;
/*      */     } 
/* 4552 */     if (this.f || par1Block.a(this.a, par2, par3 + 1, par4, 1)) {
/*      */       
/* 4554 */       tessellator.b((par1Block.bZ >= 1.0D) ? par1Block.d(this.a, par2, par3 + 1, par4) : i);
/* 4555 */       tessellator.a(f5, f9, f13);
/* 4556 */       b(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 1));
/* 4557 */       flag = true;
/*      */     } 
/* 4559 */     if (this.f || par1Block.a(this.a, par2, par3, par4 - 1, 2)) {
/*      */       
/* 4561 */       tessellator.b((par1Block.bX <= 0.0D) ? par1Block.d(this.a, par2, par3, par4 - 1) : i);
/* 4562 */       tessellator.a(f6, f10, f14);
/* 4563 */       tessellator.c(0.0F, 0.0F, f16);
/* 4564 */       c(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 2));
/* 4565 */       tessellator.c(0.0F, 0.0F, -f16);
/* 4566 */       flag = true;
/*      */     } 
/* 4568 */     if (this.f || par1Block.a(this.a, par2, par3, par4 + 1, 3)) {
/*      */       
/* 4570 */       tessellator.b((par1Block.ca >= 1.0D) ? par1Block.d(this.a, par2, par3, par4 + 1) : i);
/* 4571 */       tessellator.a(f6, f10, f14);
/* 4572 */       tessellator.c(0.0F, 0.0F, -f16);
/* 4573 */       d(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 3));
/* 4574 */       tessellator.c(0.0F, 0.0F, f16);
/* 4575 */       flag = true;
/*      */     } 
/* 4577 */     if (this.f || par1Block.a(this.a, par2 - 1, par3, par4, 4)) {
/*      */       
/* 4579 */       tessellator.b((par1Block.bV <= 0.0D) ? par1Block.d(this.a, par2 - 1, par3, par4) : i);
/* 4580 */       tessellator.a(f7, f11, f15);
/* 4581 */       tessellator.c(f16, 0.0F, 0.0F);
/* 4582 */       e(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 4));
/* 4583 */       tessellator.c(-f16, 0.0F, 0.0F);
/* 4584 */       flag = true;
/*      */     } 
/* 4586 */     if (this.f || par1Block.a(this.a, par2 + 1, par3, par4, 5)) {
/*      */       
/* 4588 */       tessellator.b((par1Block.bY >= 1.0D) ? par1Block.d(this.a, par2 + 1, par3, par4) : i);
/* 4589 */       tessellator.a(f7, f11, f15);
/* 4590 */       tessellator.c(-f16, 0.0F, 0.0F);
/* 4591 */       f(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 5));
/* 4592 */       tessellator.c(f16, 0.0F, 0.0F);
/* 4593 */       flag = true;
/*      */     } 
/* 4595 */     return flag;
/*      */   }
/*      */ 
/*      */   
/*      */   public boolean a(abg par1BlockFence, int par2, int par3, int par4) {
/* 4600 */     boolean flag = false;
/* 4601 */     float f = 0.375F;
/* 4602 */     float f1 = 0.625F;
/* 4603 */     par1BlockFence.a(f, 0.0F, f, f1, 1.0F, f1);
/* 4604 */     o((pb)par1BlockFence, par2, par3, par4);
/* 4605 */     flag = true;
/* 4606 */     boolean flag1 = false;
/* 4607 */     boolean flag2 = false;
/* 4608 */     if (par1BlockFence.g(this.a, par2 - 1, par3, par4) || par1BlockFence.g(this.a, par2 + 1, par3, par4))
/*      */     {
/* 4610 */       flag1 = true;
/*      */     }
/* 4612 */     if (par1BlockFence.g(this.a, par2, par3, par4 - 1) || par1BlockFence.g(this.a, par2, par3, par4 + 1))
/*      */     {
/* 4614 */       flag2 = true;
/*      */     }
/* 4616 */     boolean flag3 = par1BlockFence.g(this.a, par2 - 1, par3, par4);
/* 4617 */     boolean flag4 = par1BlockFence.g(this.a, par2 + 1, par3, par4);
/* 4618 */     boolean flag5 = par1BlockFence.g(this.a, par2, par3, par4 - 1);
/* 4619 */     boolean flag6 = par1BlockFence.g(this.a, par2, par3, par4 + 1);
/* 4620 */     if (!flag1 && !flag2)
/*      */     {
/* 4622 */       flag1 = true;
/*      */     }
/* 4624 */     f = 0.4375F;
/* 4625 */     f1 = 0.5625F;
/* 4626 */     float f2 = 0.75F;
/* 4627 */     float f3 = 0.9375F;
/* 4628 */     float f4 = flag3 ? 0.0F : f;
/* 4629 */     float f5 = flag4 ? 1.0F : f1;
/* 4630 */     float f6 = flag5 ? 0.0F : f;
/* 4631 */     float f7 = flag6 ? 1.0F : f1;
/* 4632 */     if (flag1) {
/*      */       
/* 4634 */       par1BlockFence.a(f4, f2, f, f5, f3, f1);
/* 4635 */       o((pb)par1BlockFence, par2, par3, par4);
/* 4636 */       flag = true;
/*      */     } 
/* 4638 */     if (flag2) {
/*      */       
/* 4640 */       par1BlockFence.a(f, f2, f6, f1, f3, f7);
/* 4641 */       o((pb)par1BlockFence, par2, par3, par4);
/* 4642 */       flag = true;
/*      */     } 
/* 4644 */     f2 = 0.375F;
/* 4645 */     f3 = 0.5625F;
/* 4646 */     if (flag1) {
/*      */       
/* 4648 */       par1BlockFence.a(f4, f2, f, f5, f3, f1);
/* 4649 */       o((pb)par1BlockFence, par2, par3, par4);
/* 4650 */       flag = true;
/*      */     } 
/* 4652 */     if (flag2) {
/*      */       
/* 4654 */       par1BlockFence.a(f, f2, f6, f1, f3, f7);
/* 4655 */       o((pb)par1BlockFence, par2, par3, par4);
/* 4656 */       flag = true;
/*      */     } 
/* 4658 */     par1BlockFence.a(this.a, par2, par3, par4);
/*      */     
/* 4660 */     if (Config.isBetterSnow())
/*      */     {
/* 4662 */       if (hasSnowNeighbours(par2, par3, par4))
/*      */       {
/* 4664 */         o(pb.aS, par2, par3, par4);
/*      */       }
/*      */     }
/*      */     
/* 4668 */     return flag;
/*      */   }
/*      */ 
/*      */   
/*      */   public boolean a(de par1BlockDragonEgg, int par2, int par3, int par4) {
/* 4673 */     boolean flag = false;
/* 4674 */     int i = 0;
/* 4675 */     for (int j = 0; j < 8; j++) {
/*      */       
/* 4677 */       int k = 0;
/* 4678 */       byte byte0 = 1;
/* 4679 */       if (j == 0)
/*      */       {
/* 4681 */         k = 2;
/*      */       }
/* 4683 */       if (j == 1)
/*      */       {
/* 4685 */         k = 3;
/*      */       }
/* 4687 */       if (j == 2)
/*      */       {
/* 4689 */         k = 4;
/*      */       }
/* 4691 */       if (j == 3) {
/*      */         
/* 4693 */         k = 5;
/* 4694 */         byte0 = 2;
/*      */       } 
/* 4696 */       if (j == 4) {
/*      */         
/* 4698 */         k = 6;
/* 4699 */         byte0 = 3;
/*      */       } 
/* 4701 */       if (j == 5) {
/*      */         
/* 4703 */         k = 7;
/* 4704 */         byte0 = 5;
/*      */       } 
/* 4706 */       if (j == 6) {
/*      */         
/* 4708 */         k = 6;
/* 4709 */         byte0 = 2;
/*      */       } 
/* 4711 */       if (j == 7)
/*      */       {
/* 4713 */         k = 3;
/*      */       }
/* 4715 */       float f = k / 16.0F;
/* 4716 */       float f1 = 1.0F - i / 16.0F;
/* 4717 */       float f2 = 1.0F - (i + byte0) / 16.0F;
/* 4718 */       i += byte0;
/* 4719 */       par1BlockDragonEgg.a(0.5F - f, f2, 0.5F - f, 0.5F + f, f1, 0.5F + f);
/* 4720 */       o((pb)par1BlockDragonEgg, par2, par3, par4);
/*      */     } 
/*      */     
/* 4723 */     flag = true;
/* 4724 */     par1BlockDragonEgg.a(0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F);
/* 4725 */     return flag;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean a(ajf par1BlockFenceGate, int par2, int par3, int par4) {
/* 4733 */     boolean flag = true;
/* 4734 */     int i = this.a.e(par2, par3, par4);
/* 4735 */     boolean flag1 = ajf.e(i);
/* 4736 */     int j = w.a(i);
/* 4737 */     if (j == 3 || j == 1) {
/*      */       
/* 4739 */       float f = 0.4375F;
/* 4740 */       float f4 = 0.5625F;
/* 4741 */       float f8 = 0.0F;
/* 4742 */       float f12 = 0.125F;
/* 4743 */       par1BlockFenceGate.a(f, 0.3125F, f8, f4, 1.0F, f12);
/* 4744 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4745 */       f8 = 0.875F;
/* 4746 */       f12 = 1.0F;
/* 4747 */       par1BlockFenceGate.a(f, 0.3125F, f8, f4, 1.0F, f12);
/* 4748 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/*      */     } else {
/*      */       
/* 4751 */       float f1 = 0.0F;
/* 4752 */       float f5 = 0.125F;
/* 4753 */       float f9 = 0.4375F;
/* 4754 */       float f13 = 0.5625F;
/* 4755 */       par1BlockFenceGate.a(f1, 0.3125F, f9, f5, 1.0F, f13);
/* 4756 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4757 */       f1 = 0.875F;
/* 4758 */       f5 = 1.0F;
/* 4759 */       par1BlockFenceGate.a(f1, 0.3125F, f9, f5, 1.0F, f13);
/* 4760 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/*      */     } 
/* 4762 */     if (!flag1) {
/*      */       
/* 4764 */       if (j == 3 || j == 1) {
/*      */         
/* 4766 */         float f2 = 0.4375F;
/* 4767 */         float f6 = 0.5625F;
/* 4768 */         float f10 = 0.375F;
/* 4769 */         float f14 = 0.5F;
/* 4770 */         par1BlockFenceGate.a(f2, 0.375F, f10, f6, 0.9375F, f14);
/* 4771 */         o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4772 */         f10 = 0.5F;
/* 4773 */         f14 = 0.625F;
/* 4774 */         par1BlockFenceGate.a(f2, 0.375F, f10, f6, 0.9375F, f14);
/* 4775 */         o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4776 */         f10 = 0.625F;
/* 4777 */         f14 = 0.875F;
/* 4778 */         par1BlockFenceGate.a(f2, 0.375F, f10, f6, 0.5625F, f14);
/* 4779 */         o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4780 */         par1BlockFenceGate.a(f2, 0.75F, f10, f6, 0.9375F, f14);
/* 4781 */         o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4782 */         f10 = 0.125F;
/* 4783 */         f14 = 0.375F;
/* 4784 */         par1BlockFenceGate.a(f2, 0.375F, f10, f6, 0.5625F, f14);
/* 4785 */         o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4786 */         par1BlockFenceGate.a(f2, 0.75F, f10, f6, 0.9375F, f14);
/* 4787 */         o((pb)par1BlockFenceGate, par2, par3, par4);
/*      */       } else {
/*      */         
/* 4790 */         float f3 = 0.375F;
/* 4791 */         float f7 = 0.5F;
/* 4792 */         float f11 = 0.4375F;
/* 4793 */         float f15 = 0.5625F;
/* 4794 */         par1BlockFenceGate.a(f3, 0.375F, f11, f7, 0.9375F, f15);
/* 4795 */         o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4796 */         f3 = 0.5F;
/* 4797 */         f7 = 0.625F;
/* 4798 */         par1BlockFenceGate.a(f3, 0.375F, f11, f7, 0.9375F, f15);
/* 4799 */         o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4800 */         f3 = 0.625F;
/* 4801 */         f7 = 0.875F;
/* 4802 */         par1BlockFenceGate.a(f3, 0.375F, f11, f7, 0.5625F, f15);
/* 4803 */         o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4804 */         par1BlockFenceGate.a(f3, 0.75F, f11, f7, 0.9375F, f15);
/* 4805 */         o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4806 */         f3 = 0.125F;
/* 4807 */         f7 = 0.375F;
/* 4808 */         par1BlockFenceGate.a(f3, 0.375F, f11, f7, 0.5625F, f15);
/* 4809 */         o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4810 */         par1BlockFenceGate.a(f3, 0.75F, f11, f7, 0.9375F, f15);
/* 4811 */         o((pb)par1BlockFenceGate, par2, par3, par4);
/*      */       }
/*      */     
/* 4814 */     } else if (j == 3) {
/*      */       
/* 4816 */       par1BlockFenceGate.a(0.8125F, 0.375F, 0.0F, 0.9375F, 0.9375F, 0.125F);
/* 4817 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4818 */       par1BlockFenceGate.a(0.8125F, 0.375F, 0.875F, 0.9375F, 0.9375F, 1.0F);
/* 4819 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4820 */       par1BlockFenceGate.a(0.5625F, 0.375F, 0.0F, 0.8125F, 0.5625F, 0.125F);
/* 4821 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4822 */       par1BlockFenceGate.a(0.5625F, 0.375F, 0.875F, 0.8125F, 0.5625F, 1.0F);
/* 4823 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4824 */       par1BlockFenceGate.a(0.5625F, 0.75F, 0.0F, 0.8125F, 0.9375F, 0.125F);
/* 4825 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4826 */       par1BlockFenceGate.a(0.5625F, 0.75F, 0.875F, 0.8125F, 0.9375F, 1.0F);
/* 4827 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/*      */     }
/* 4829 */     else if (j == 1) {
/*      */       
/* 4831 */       par1BlockFenceGate.a(0.0625F, 0.375F, 0.0F, 0.1875F, 0.9375F, 0.125F);
/* 4832 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4833 */       par1BlockFenceGate.a(0.0625F, 0.375F, 0.875F, 0.1875F, 0.9375F, 1.0F);
/* 4834 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4835 */       par1BlockFenceGate.a(0.1875F, 0.375F, 0.0F, 0.4375F, 0.5625F, 0.125F);
/* 4836 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4837 */       par1BlockFenceGate.a(0.1875F, 0.375F, 0.875F, 0.4375F, 0.5625F, 1.0F);
/* 4838 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4839 */       par1BlockFenceGate.a(0.1875F, 0.75F, 0.0F, 0.4375F, 0.9375F, 0.125F);
/* 4840 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4841 */       par1BlockFenceGate.a(0.1875F, 0.75F, 0.875F, 0.4375F, 0.9375F, 1.0F);
/* 4842 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/*      */     }
/* 4844 */     else if (j == 0) {
/*      */       
/* 4846 */       par1BlockFenceGate.a(0.0F, 0.375F, 0.8125F, 0.125F, 0.9375F, 0.9375F);
/* 4847 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4848 */       par1BlockFenceGate.a(0.875F, 0.375F, 0.8125F, 1.0F, 0.9375F, 0.9375F);
/* 4849 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4850 */       par1BlockFenceGate.a(0.0F, 0.375F, 0.5625F, 0.125F, 0.5625F, 0.8125F);
/* 4851 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4852 */       par1BlockFenceGate.a(0.875F, 0.375F, 0.5625F, 1.0F, 0.5625F, 0.8125F);
/* 4853 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4854 */       par1BlockFenceGate.a(0.0F, 0.75F, 0.5625F, 0.125F, 0.9375F, 0.8125F);
/* 4855 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4856 */       par1BlockFenceGate.a(0.875F, 0.75F, 0.5625F, 1.0F, 0.9375F, 0.8125F);
/* 4857 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/*      */     }
/* 4859 */     else if (j == 2) {
/*      */       
/* 4861 */       par1BlockFenceGate.a(0.0F, 0.375F, 0.0625F, 0.125F, 0.9375F, 0.1875F);
/* 4862 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4863 */       par1BlockFenceGate.a(0.875F, 0.375F, 0.0625F, 1.0F, 0.9375F, 0.1875F);
/* 4864 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4865 */       par1BlockFenceGate.a(0.0F, 0.375F, 0.1875F, 0.125F, 0.5625F, 0.4375F);
/* 4866 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4867 */       par1BlockFenceGate.a(0.875F, 0.375F, 0.1875F, 1.0F, 0.5625F, 0.4375F);
/* 4868 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4869 */       par1BlockFenceGate.a(0.0F, 0.75F, 0.1875F, 0.125F, 0.9375F, 0.4375F);
/* 4870 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/* 4871 */       par1BlockFenceGate.a(0.875F, 0.75F, 0.1875F, 1.0F, 0.9375F, 0.4375F);
/* 4872 */       o((pb)par1BlockFenceGate, par2, par3, par4);
/*      */     } 
/* 4874 */     par1BlockFenceGate.a(0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F);
/* 4875 */     return flag;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean q(pb par1Block, int par2, int par3, int par4) {
/* 4883 */     int i = this.a.e(par2, par3, par4);
/* 4884 */     int j = i & 0x3;
/* 4885 */     float f = 0.0F;
/* 4886 */     float f1 = 0.5F;
/* 4887 */     float f2 = 0.5F;
/* 4888 */     float f3 = 1.0F;
/* 4889 */     if ((i & 0x4) != 0) {
/*      */       
/* 4891 */       f = 0.5F;
/* 4892 */       f1 = 1.0F;
/* 4893 */       f2 = 0.0F;
/* 4894 */       f3 = 0.5F;
/*      */     } 
/* 4896 */     par1Block.a(0.0F, f, 0.0F, 1.0F, f1, 1.0F);
/* 4897 */     o(par1Block, par2, par3, par4);
/* 4898 */     if (j == 0) {
/*      */       
/* 4900 */       par1Block.a(0.5F, f2, 0.0F, 1.0F, f3, 1.0F);
/* 4901 */       o(par1Block, par2, par3, par4);
/*      */     }
/* 4903 */     else if (j == 1) {
/*      */       
/* 4905 */       par1Block.a(0.0F, f2, 0.0F, 0.5F, f3, 1.0F);
/* 4906 */       o(par1Block, par2, par3, par4);
/*      */     }
/* 4908 */     else if (j == 2) {
/*      */       
/* 4910 */       par1Block.a(0.0F, f2, 0.5F, 1.0F, f3, 1.0F);
/* 4911 */       o(par1Block, par2, par3, par4);
/*      */     }
/* 4913 */     else if (j == 3) {
/*      */       
/* 4915 */       par1Block.a(0.0F, f2, 0.0F, 1.0F, f3, 0.5F);
/* 4916 */       o(par1Block, par2, par3, par4);
/*      */     } 
/* 4918 */     par1Block.a(0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F);
/* 4919 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean r(pb par1Block, int par2, int par3, int par4) {
/* 4927 */     adz tessellator = adz.a;
/* 4928 */     acb blockdoor = (acb)par1Block;
/* 4929 */     boolean flag = false;
/* 4930 */     float f = 0.5F;
/* 4931 */     float f1 = 1.0F;
/* 4932 */     float f2 = 0.8F;
/* 4933 */     float f3 = 0.6F;
/* 4934 */     int i = par1Block.d(this.a, par2, par3, par4);
/* 4935 */     tessellator.b((par1Block.bW <= 0.0D) ? par1Block.d(this.a, par2, par3 - 1, par4) : i);
/* 4936 */     tessellator.a(f, f, f);
/* 4937 */     a(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 0));
/* 4938 */     flag = true;
/* 4939 */     tessellator.b((par1Block.bZ >= 1.0D) ? par1Block.d(this.a, par2, par3 + 1, par4) : i);
/* 4940 */     tessellator.a(f1, f1, f1);
/* 4941 */     b(par1Block, par2, par3, par4, par1Block.d(this.a, par2, par3, par4, 1));
/* 4942 */     flag = true;
/* 4943 */     tessellator.b((par1Block.bX <= 0.0D) ? par1Block.d(this.a, par2, par3, par4 - 1) : i);
/* 4944 */     tessellator.a(f2, f2, f2);
/* 4945 */     int j = par1Block.d(this.a, par2, par3, par4, 2);
/* 4946 */     if (j < 0) {
/*      */       
/* 4948 */       this.e = true;
/* 4949 */       j = -j;
/*      */     } 
/* 4951 */     c(par1Block, par2, par3, par4, j);
/* 4952 */     flag = true;
/* 4953 */     this.e = false;
/* 4954 */     tessellator.b((par1Block.ca >= 1.0D) ? par1Block.d(this.a, par2, par3, par4 + 1) : i);
/* 4955 */     tessellator.a(f2, f2, f2);
/* 4956 */     j = par1Block.d(this.a, par2, par3, par4, 3);
/* 4957 */     if (j < 0) {
/*      */       
/* 4959 */       this.e = true;
/* 4960 */       j = -j;
/*      */     } 
/* 4962 */     d(par1Block, par2, par3, par4, j);
/* 4963 */     flag = true;
/* 4964 */     this.e = false;
/* 4965 */     tessellator.b((par1Block.bV <= 0.0D) ? par1Block.d(this.a, par2 - 1, par3, par4) : i);
/* 4966 */     tessellator.a(f3, f3, f3);
/* 4967 */     j = par1Block.d(this.a, par2, par3, par4, 4);
/* 4968 */     if (j < 0) {
/*      */       
/* 4970 */       this.e = true;
/* 4971 */       j = -j;
/*      */     } 
/* 4973 */     e(par1Block, par2, par3, par4, j);
/* 4974 */     flag = true;
/* 4975 */     this.e = false;
/* 4976 */     tessellator.b((par1Block.bY >= 1.0D) ? par1Block.d(this.a, par2 + 1, par3, par4) : i);
/* 4977 */     tessellator.a(f3, f3, f3);
/* 4978 */     j = par1Block.d(this.a, par2, par3, par4, 5);
/* 4979 */     if (j < 0) {
/*      */       
/* 4981 */       this.e = true;
/* 4982 */       j = -j;
/*      */     } 
/* 4984 */     f(par1Block, par2, par3, par4, j);
/* 4985 */     flag = true;
/* 4986 */     this.e = false;
/* 4987 */     return flag;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(pb par1Block, double par2, double par4, double par6, int par8) {
/* 4995 */     adz tessellator = adz.a;
/* 4996 */     if (this.d >= 0)
/*      */     {
/* 4998 */       par8 = this.d;
/*      */     }
/*      */     
/* 5001 */     if (Config.isConnectedTextures() && this.d < 0) {
/*      */       
/* 5003 */       int tex = ConnectedTextures.getConnectedTexture(this.a, par1Block, (int)par2, (int)par4, (int)par6, 0, par8);
/* 5004 */       if (tex >= 0) {
/*      */         
/* 5006 */         int ctmTex = tex / 256;
/* 5007 */         tessellator = adz.a.getSubTessellator(ctmTex);
/* 5008 */         par8 = tex % 256;
/*      */       } 
/*      */     } 
/*      */     
/* 5012 */     if (Config.isNaturalTextures() && this.d < 0) {
/*      */       
/* 5014 */       NaturalProperties nps = NaturalTextures.getNaturalProperties(tessellator.textureID, par8);
/* 5015 */       if (nps != null) {
/*      */         
/* 5017 */         int rand = Config.getRandom((int)par2, (int)par4, (int)par6, 0);
/* 5018 */         if (nps.rotation > 1) {
/* 5019 */           this.l = rand & 0x3;
/*      */         }
/* 5021 */         if (nps.rotation == 2) {
/* 5022 */           this.l = this.l / 2 * 3;
/*      */         }
/* 5024 */         if (nps.flip) {
/* 5025 */           this.e = ((rand & 0x4) != 0);
/*      */         }
/*      */       } 
/*      */     } 
/* 5029 */     double minX = par1Block.bV;
/* 5030 */     double maxX = par1Block.bY;
/* 5031 */     double minZ = par1Block.bX;
/* 5032 */     double maxZ = par1Block.ca;
/* 5033 */     if (minX < 0.0D || maxX > 1.0D) {
/*      */       
/* 5035 */       minX = 0.0D;
/* 5036 */       maxX = 1.0D;
/*      */     } 
/* 5038 */     if (minZ < 0.0D || maxZ > 1.0D) {
/*      */       
/* 5040 */       minZ = 0.0D;
/* 5041 */       maxZ = 1.0D;
/*      */     } 
/*      */     
/* 5044 */     int i = (par8 & 0xF) << 4;
/* 5045 */     int j = par8 & 0xF0;
/* 5046 */     double d = (i + minX * 16.0D) / 256.0D;
/* 5047 */     double d1 = (i + maxX * 16.0D - 0.01D) / 256.0D;
/* 5048 */     double d2 = (j + minZ * 16.0D) / 256.0D;
/* 5049 */     double d3 = (j + maxZ * 16.0D - 0.01D) / 256.0D;
/*      */     
/* 5051 */     if (this.e) {
/*      */       
/* 5053 */       double tmp = d;
/* 5054 */       d = d1;
/* 5055 */       d1 = tmp;
/*      */     } 
/* 5057 */     double d4 = d1;
/* 5058 */     double d5 = d;
/* 5059 */     double d6 = d2;
/* 5060 */     double d7 = d3;
/* 5061 */     if (this.l == 2) {
/*      */       
/* 5063 */       d = (i + minZ * 16.0D) / 256.0D;
/* 5064 */       d2 = ((j + 16) - maxX * 16.0D) / 256.0D;
/* 5065 */       d1 = (i + maxZ * 16.0D - 0.01D) / 256.0D;
/* 5066 */       d3 = ((j + 16) - minX * 16.0D - 0.01D) / 256.0D;
/*      */       
/* 5068 */       if (this.e) {
/*      */         
/* 5070 */         double tmp = d;
/* 5071 */         d = d1;
/* 5072 */         d1 = tmp;
/*      */       } 
/* 5074 */       d4 = d1;
/* 5075 */       d5 = d;
/* 5076 */       d6 = d2;
/* 5077 */       d7 = d3;
/* 5078 */       d4 = d;
/* 5079 */       d5 = d1;
/* 5080 */       d2 = d3;
/* 5081 */       d3 = d6;
/*      */     }
/* 5083 */     else if (this.l == 1) {
/*      */       
/* 5085 */       d = ((i + 16) - maxZ * 16.0D) / 256.0D;
/* 5086 */       d2 = (j + minX * 16.0D) / 256.0D;
/* 5087 */       d1 = ((i + 16) - minZ * 16.0D - 0.01D) / 256.0D;
/* 5088 */       d3 = (j + maxX * 16.0D - 0.01D) / 256.0D;
/*      */       
/* 5090 */       if (this.e) {
/*      */         
/* 5092 */         double tmp = d;
/* 5093 */         d = d1;
/* 5094 */         d1 = tmp;
/*      */       } 
/* 5096 */       d4 = d1;
/* 5097 */       d5 = d;
/* 5098 */       d6 = d2;
/* 5099 */       d7 = d3;
/* 5100 */       d = d4;
/* 5101 */       d1 = d5;
/* 5102 */       d6 = d3;
/* 5103 */       d7 = d2;
/*      */     }
/* 5105 */     else if (this.l == 3) {
/*      */       
/* 5107 */       d = ((i + 16) - minX * 16.0D - 0.01D) / 256.0D;
/* 5108 */       d1 = ((i + 16) - maxX * 16.0D) / 256.0D;
/* 5109 */       d2 = ((j + 16) - minZ * 16.0D - 0.01D) / 256.0D;
/* 5110 */       d3 = ((j + 16) - maxZ * 16.0D) / 256.0D;
/*      */       
/* 5112 */       if (this.e) {
/*      */         
/* 5114 */         double tmp = d;
/* 5115 */         d = d1;
/* 5116 */         d1 = tmp;
/*      */       } 
/* 5118 */       d4 = d1;
/* 5119 */       d5 = d;
/* 5120 */       d6 = d2;
/* 5121 */       d7 = d3;
/*      */     } 
/*      */     
/* 5124 */     this.l = 0;
/* 5125 */     this.e = false;
/*      */     
/* 5127 */     double d8 = par2 + par1Block.bV;
/* 5128 */     double d9 = par2 + par1Block.bY;
/* 5129 */     double d10 = par4 + par1Block.bW;
/* 5130 */     double d11 = par6 + par1Block.bX;
/* 5131 */     double d12 = par6 + par1Block.ca;
/* 5132 */     if (this.m) {
/*      */       
/* 5134 */       tessellator.a(this.an, this.ar, this.av);
/* 5135 */       tessellator.b(this.aj);
/* 5136 */       tessellator.a(d8, d10, d12, d5, d7);
/* 5137 */       tessellator.a(this.ao, this.as, this.aw);
/* 5138 */       tessellator.b(this.ak);
/* 5139 */       tessellator.a(d8, d10, d11, d, d2);
/* 5140 */       tessellator.a(this.ap, this.at, this.ax);
/* 5141 */       tessellator.b(this.al);
/* 5142 */       tessellator.a(d9, d10, d11, d4, d6);
/* 5143 */       tessellator.a(this.aq, this.au, this.ay);
/* 5144 */       tessellator.b(this.am);
/* 5145 */       tessellator.a(d9, d10, d12, d1, d3);
/*      */     } else {
/*      */       
/* 5148 */       tessellator.a(d8, d10, d12, d5, d7);
/* 5149 */       tessellator.a(d8, d10, d11, d, d2);
/* 5150 */       tessellator.a(d9, d10, d11, d4, d6);
/* 5151 */       tessellator.a(d9, d10, d12, d1, d3);
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void b(pb par1Block, double par2, double par4, double par6, int par8) {
/* 5160 */     adz tessellator = adz.a;
/* 5161 */     if (this.d >= 0)
/*      */     {
/* 5163 */       par8 = this.d;
/*      */     }
/*      */     
/* 5166 */     if (Config.isConnectedTextures() && this.d < 0) {
/*      */       
/* 5168 */       int tex = ConnectedTextures.getConnectedTexture(this.a, par1Block, (int)par2, (int)par4, (int)par6, 1, par8);
/* 5169 */       if (tex >= 0) {
/*      */         
/* 5171 */         int ctmTex = tex / 256;
/* 5172 */         tessellator = adz.a.getSubTessellator(ctmTex);
/* 5173 */         par8 = tex % 256;
/*      */       } 
/*      */     } 
/*      */     
/* 5177 */     if (Config.isNaturalTextures() && this.d < 0) {
/*      */       
/* 5179 */       NaturalProperties nps = NaturalTextures.getNaturalProperties(tessellator.textureID, par8);
/* 5180 */       if (nps != null) {
/*      */         
/* 5182 */         int rand = Config.getRandom((int)par2, (int)par4, (int)par6, 1);
/* 5183 */         if (nps.rotation > 1) {
/* 5184 */           this.k = rand & 0x3;
/*      */         }
/* 5186 */         if (nps.rotation == 2) {
/* 5187 */           this.k = this.k / 2 * 3;
/*      */         }
/* 5189 */         if (nps.flip) {
/* 5190 */           this.e = ((rand & 0x4) != 0);
/*      */         }
/*      */       } 
/*      */     } 
/* 5194 */     double minX = par1Block.bV;
/* 5195 */     double maxX = par1Block.bY;
/* 5196 */     double minZ = par1Block.bX;
/* 5197 */     double maxZ = par1Block.ca;
/* 5198 */     if (minX < 0.0D || maxX > 1.0D) {
/*      */       
/* 5200 */       minX = 0.0D;
/* 5201 */       maxX = 1.0D;
/*      */     } 
/* 5203 */     if (minZ < 0.0D || maxZ > 1.0D) {
/*      */       
/* 5205 */       minZ = 0.0D;
/* 5206 */       maxZ = 1.0D;
/*      */     } 
/*      */     
/* 5209 */     int i = (par8 & 0xF) << 4;
/* 5210 */     int j = par8 & 0xF0;
/* 5211 */     double d = (i + minX * 16.0D) / 256.0D;
/* 5212 */     double d1 = (i + maxX * 16.0D - 0.01D) / 256.0D;
/* 5213 */     double d2 = (j + minZ * 16.0D) / 256.0D;
/* 5214 */     double d3 = (j + maxZ * 16.0D - 0.01D) / 256.0D;
/*      */     
/* 5216 */     if (this.e) {
/*      */       
/* 5218 */       double tmp = d;
/* 5219 */       d = d1;
/* 5220 */       d1 = tmp;
/*      */     } 
/* 5222 */     double d4 = d1;
/* 5223 */     double d5 = d;
/* 5224 */     double d6 = d2;
/* 5225 */     double d7 = d3;
/* 5226 */     if (this.k == 1) {
/*      */       
/* 5228 */       d = (i + minZ * 16.0D) / 256.0D;
/* 5229 */       d2 = ((j + 16) - maxX * 16.0D) / 256.0D;
/* 5230 */       d1 = (i + maxZ * 16.0D - 0.01D) / 256.0D;
/* 5231 */       d3 = ((j + 16) - minX * 16.0D - 0.01D) / 256.0D;
/*      */       
/* 5233 */       if (this.e) {
/*      */         
/* 5235 */         double tmp = d;
/* 5236 */         d = d1;
/* 5237 */         d1 = tmp;
/*      */       } 
/* 5239 */       d4 = d1;
/* 5240 */       d5 = d;
/* 5241 */       d6 = d2;
/* 5242 */       d7 = d3;
/* 5243 */       d4 = d;
/* 5244 */       d5 = d1;
/* 5245 */       d2 = d3;
/* 5246 */       d3 = d6;
/*      */     }
/* 5248 */     else if (this.k == 2) {
/*      */       
/* 5250 */       d = ((i + 16) - maxZ * 16.0D) / 256.0D;
/* 5251 */       d2 = (j + minX * 16.0D) / 256.0D;
/* 5252 */       d1 = ((i + 16) - minZ * 16.0D - 0.01D) / 256.0D;
/* 5253 */       d3 = (j + maxX * 16.0D - 0.01D) / 256.0D;
/*      */       
/* 5255 */       if (this.e) {
/*      */         
/* 5257 */         double tmp = d;
/* 5258 */         d = d1;
/* 5259 */         d1 = tmp;
/*      */       } 
/* 5261 */       d4 = d1;
/* 5262 */       d5 = d;
/* 5263 */       d6 = d2;
/* 5264 */       d7 = d3;
/* 5265 */       d = d4;
/* 5266 */       d1 = d5;
/* 5267 */       d6 = d3;
/* 5268 */       d7 = d2;
/*      */     }
/* 5270 */     else if (this.k == 3) {
/*      */       
/* 5272 */       d = ((i + 16) - minX * 16.0D - 0.01D) / 256.0D;
/* 5273 */       d1 = ((i + 16) - maxX * 16.0D) / 256.0D;
/* 5274 */       d2 = ((j + 16) - minZ * 16.0D - 0.01D) / 256.0D;
/* 5275 */       d3 = ((j + 16) - maxZ * 16.0D) / 256.0D;
/*      */       
/* 5277 */       if (this.e) {
/*      */         
/* 5279 */         double tmp = d;
/* 5280 */         d = d1;
/* 5281 */         d1 = tmp;
/*      */       } 
/* 5283 */       d4 = d1;
/* 5284 */       d5 = d;
/* 5285 */       d6 = d2;
/* 5286 */       d7 = d3;
/*      */     } 
/*      */     
/* 5289 */     this.k = 0;
/* 5290 */     this.e = false;
/*      */     
/* 5292 */     double d8 = par2 + par1Block.bV;
/* 5293 */     double d9 = par2 + par1Block.bY;
/* 5294 */     double d10 = par4 + par1Block.bZ;
/* 5295 */     double d11 = par6 + par1Block.bX;
/* 5296 */     double d12 = par6 + par1Block.ca;
/* 5297 */     if (this.m) {
/*      */       
/* 5299 */       tessellator.a(this.an, this.ar, this.av);
/* 5300 */       tessellator.b(this.aj);
/* 5301 */       tessellator.a(d9, d10, d12, d1, d3);
/* 5302 */       tessellator.a(this.ao, this.as, this.aw);
/* 5303 */       tessellator.b(this.ak);
/* 5304 */       tessellator.a(d9, d10, d11, d4, d6);
/* 5305 */       tessellator.a(this.ap, this.at, this.ax);
/* 5306 */       tessellator.b(this.al);
/* 5307 */       tessellator.a(d8, d10, d11, d, d2);
/* 5308 */       tessellator.a(this.aq, this.au, this.ay);
/* 5309 */       tessellator.b(this.am);
/* 5310 */       tessellator.a(d8, d10, d12, d5, d7);
/*      */     } else {
/*      */       
/* 5313 */       tessellator.a(d9, d10, d12, d1, d3);
/* 5314 */       tessellator.a(d9, d10, d11, d4, d6);
/* 5315 */       tessellator.a(d8, d10, d11, d, d2);
/* 5316 */       tessellator.a(d8, d10, d12, d5, d7);
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void c(pb par1Block, double par2, double par4, double par6, int par8) {
/* 5325 */     adz tessellator = adz.a;
/* 5326 */     if (this.d >= 0)
/*      */     {
/* 5328 */       par8 = this.d;
/*      */     }
/*      */     
/* 5331 */     if (Config.isConnectedTextures() && this.d < 0) {
/*      */       
/* 5333 */       int tex = ConnectedTextures.getConnectedTexture(this.a, par1Block, (int)par2, (int)par4, (int)par6, 2, par8);
/* 5334 */       if (tex >= 0) {
/*      */         
/* 5336 */         int ctmTex = tex / 256;
/* 5337 */         tessellator = adz.a.getSubTessellator(ctmTex);
/* 5338 */         par8 = tex % 256;
/*      */       } 
/*      */     } 
/*      */     
/* 5342 */     if (Config.isNaturalTextures() && this.d < 0) {
/*      */       
/* 5344 */       NaturalProperties nps = NaturalTextures.getNaturalProperties(tessellator.textureID, par8);
/* 5345 */       if (nps != null) {
/*      */         
/* 5347 */         int rand = Config.getRandom((int)par2, (int)par4, (int)par6, 2);
/* 5348 */         if (nps.rotation > 1) {
/* 5349 */           this.g = rand & 0x3;
/*      */         }
/* 5351 */         if (nps.rotation == 2) {
/* 5352 */           this.g = this.g / 2 * 3;
/*      */         }
/* 5354 */         if (nps.flip) {
/* 5355 */           this.e = ((rand & 0x4) != 0);
/*      */         }
/*      */       } 
/*      */     } 
/* 5359 */     double minX = par1Block.bV;
/* 5360 */     double maxX = par1Block.bY;
/* 5361 */     double minY = par1Block.bW;
/* 5362 */     double maxY = par1Block.bZ;
/* 5363 */     if (minX < 0.0D || maxX > 1.0D) {
/*      */       
/* 5365 */       minX = 0.0D;
/* 5366 */       maxX = 1.0D;
/*      */     } 
/* 5368 */     if (minY < 0.0D || maxY > 1.0D) {
/*      */       
/* 5370 */       minY = 0.0D;
/* 5371 */       maxY = 1.0D;
/*      */     } 
/*      */     
/* 5374 */     int i = (par8 & 0xF) << 4;
/* 5375 */     int j = par8 & 0xF0;
/* 5376 */     double d = (i + minX * 16.0D) / 256.0D;
/* 5377 */     double d1 = (i + maxX * 16.0D - 0.01D) / 256.0D;
/* 5378 */     double d2 = ((j + 16) - maxY * 16.0D) / 256.0D;
/* 5379 */     double d3 = ((j + 16) - minY * 16.0D - 0.01D) / 256.0D;
/* 5380 */     if (this.e) {
/*      */       
/* 5382 */       double d4 = d;
/* 5383 */       d = d1;
/* 5384 */       d1 = d4;
/*      */     } 
/* 5386 */     double d5 = d1;
/* 5387 */     double d6 = d;
/* 5388 */     double d7 = d2;
/* 5389 */     double d8 = d3;
/* 5390 */     if (this.g == 2) {
/*      */       
/* 5392 */       d = (i + minY * 16.0D) / 256.0D;
/* 5393 */       d2 = ((j + 16) - minX * 16.0D - 0.01D) / 256.0D;
/* 5394 */       d1 = (i + maxY * 16.0D - 0.01D) / 256.0D;
/* 5395 */       d3 = ((j + 16) - maxX * 16.0D) / 256.0D;
/*      */       
/* 5397 */       if (this.e) {
/*      */         
/* 5399 */         double tmp = d;
/* 5400 */         d = d1;
/* 5401 */         d1 = tmp;
/*      */       } 
/* 5403 */       d5 = d1;
/* 5404 */       d6 = d;
/* 5405 */       d7 = d2;
/* 5406 */       d8 = d3;
/* 5407 */       d5 = d;
/* 5408 */       d6 = d1;
/* 5409 */       d2 = d3;
/* 5410 */       d3 = d7;
/*      */     }
/* 5412 */     else if (this.g == 1) {
/*      */       
/* 5414 */       d = ((i + 16) - maxY * 16.0D) / 256.0D;
/* 5415 */       d2 = (j + maxX * 16.0D - 0.01D) / 256.0D;
/* 5416 */       d1 = ((i + 16) - minY * 16.0D - 0.01D) / 256.0D;
/* 5417 */       d3 = (j + minX * 16.0D) / 256.0D;
/*      */       
/* 5419 */       if (this.e) {
/*      */         
/* 5421 */         double tmp = d;
/* 5422 */         d = d1;
/* 5423 */         d1 = tmp;
/*      */       } 
/* 5425 */       d5 = d1;
/* 5426 */       d6 = d;
/* 5427 */       d7 = d2;
/* 5428 */       d8 = d3;
/* 5429 */       d = d5;
/* 5430 */       d1 = d6;
/* 5431 */       d7 = d3;
/* 5432 */       d8 = d2;
/*      */     }
/* 5434 */     else if (this.g == 3) {
/*      */       
/* 5436 */       d = ((i + 16) - minX * 16.0D - 0.01D) / 256.0D;
/* 5437 */       d1 = ((i + 16) - maxX * 16.0D) / 256.0D;
/* 5438 */       d2 = (j + maxY * 16.0D - 0.01D) / 256.0D;
/* 5439 */       d3 = (j + minY * 16.0D) / 256.0D;
/*      */       
/* 5441 */       if (this.e) {
/*      */         
/* 5443 */         double tmp = d;
/* 5444 */         d = d1;
/* 5445 */         d1 = tmp;
/*      */       } 
/* 5447 */       d5 = d1;
/* 5448 */       d6 = d;
/* 5449 */       d7 = d2;
/* 5450 */       d8 = d3;
/*      */     } 
/*      */     
/* 5453 */     this.g = 0;
/* 5454 */     this.e = false;
/*      */     
/* 5456 */     double d9 = par2 + par1Block.bV;
/* 5457 */     double d10 = par2 + par1Block.bY;
/* 5458 */     double d11 = par4 + par1Block.bW;
/* 5459 */     double d12 = par4 + par1Block.bZ;
/* 5460 */     double d13 = par6 + par1Block.bX;
/* 5461 */     if (this.m) {
/*      */       
/* 5463 */       tessellator.a(this.an, this.ar, this.av);
/* 5464 */       tessellator.b(this.aj);
/* 5465 */       tessellator.a(d9, d12, d13, d5, d7);
/* 5466 */       tessellator.a(this.ao, this.as, this.aw);
/* 5467 */       tessellator.b(this.ak);
/* 5468 */       tessellator.a(d10, d12, d13, d, d2);
/* 5469 */       tessellator.a(this.ap, this.at, this.ax);
/* 5470 */       tessellator.b(this.al);
/* 5471 */       tessellator.a(d10, d11, d13, d6, d8);
/* 5472 */       tessellator.a(this.aq, this.au, this.ay);
/* 5473 */       tessellator.b(this.am);
/* 5474 */       tessellator.a(d9, d11, d13, d1, d3);
/*      */     } else {
/*      */       
/* 5477 */       tessellator.a(d9, d12, d13, d5, d7);
/* 5478 */       tessellator.a(d10, d12, d13, d, d2);
/* 5479 */       tessellator.a(d10, d11, d13, d6, d8);
/* 5480 */       tessellator.a(d9, d11, d13, d1, d3);
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void d(pb par1Block, double par2, double par4, double par6, int par8) {
/* 5489 */     adz tessellator = adz.a;
/* 5490 */     if (this.d >= 0)
/*      */     {
/* 5492 */       par8 = this.d;
/*      */     }
/*      */     
/* 5495 */     if (Config.isConnectedTextures() && this.d < 0) {
/*      */       
/* 5497 */       int tex = ConnectedTextures.getConnectedTexture(this.a, par1Block, (int)par2, (int)par4, (int)par6, 3, par8);
/* 5498 */       if (tex >= 0) {
/*      */         
/* 5500 */         int ctmTex = tex / 256;
/* 5501 */         tessellator = adz.a.getSubTessellator(ctmTex);
/* 5502 */         par8 = tex % 256;
/*      */       } 
/*      */     } 
/*      */     
/* 5506 */     if (Config.isNaturalTextures() && this.d < 0) {
/*      */       
/* 5508 */       NaturalProperties nps = NaturalTextures.getNaturalProperties(tessellator.textureID, par8);
/* 5509 */       if (nps != null) {
/*      */         
/* 5511 */         int rand = Config.getRandom((int)par2, (int)par4, (int)par6, 3);
/* 5512 */         if (nps.rotation > 1) {
/* 5513 */           this.h = rand & 0x3;
/*      */         }
/* 5515 */         if (nps.rotation == 2) {
/* 5516 */           this.h = this.h / 2 * 3;
/*      */         }
/* 5518 */         if (nps.flip) {
/* 5519 */           this.e = ((rand & 0x4) != 0);
/*      */         }
/*      */       } 
/*      */     } 
/* 5523 */     double minX = par1Block.bV;
/* 5524 */     double maxX = par1Block.bY;
/* 5525 */     double minY = par1Block.bW;
/* 5526 */     double maxY = par1Block.bZ;
/* 5527 */     if (minX < 0.0D || maxX > 1.0D) {
/*      */       
/* 5529 */       minX = 0.0D;
/* 5530 */       maxX = 1.0D;
/*      */     } 
/* 5532 */     if (minY < 0.0D || maxY > 1.0D) {
/*      */       
/* 5534 */       minY = 0.0D;
/* 5535 */       maxY = 1.0D;
/*      */     } 
/*      */     
/* 5538 */     int i = (par8 & 0xF) << 4;
/* 5539 */     int j = par8 & 0xF0;
/* 5540 */     double d = (i + minX * 16.0D) / 256.0D;
/* 5541 */     double d1 = (i + maxX * 16.0D - 0.01D) / 256.0D;
/* 5542 */     double d2 = ((j + 16) - maxY * 16.0D) / 256.0D;
/* 5543 */     double d3 = ((j + 16) - minY * 16.0D - 0.01D) / 256.0D;
/* 5544 */     if (this.e) {
/*      */       
/* 5546 */       double d4 = d;
/* 5547 */       d = d1;
/* 5548 */       d1 = d4;
/*      */     } 
/* 5550 */     double d5 = d1;
/* 5551 */     double d6 = d;
/* 5552 */     double d7 = d2;
/* 5553 */     double d8 = d3;
/* 5554 */     if (this.h == 1) {
/*      */       
/* 5556 */       d = (i + minY * 16.0D) / 256.0D;
/* 5557 */       d3 = ((j + 16) - minX * 16.0D - 0.01D) / 256.0D;
/* 5558 */       d1 = (i + maxY * 16.0D - 0.01D) / 256.0D;
/* 5559 */       d2 = ((j + 16) - maxX * 16.0D) / 256.0D;
/*      */       
/* 5561 */       if (this.e) {
/*      */         
/* 5563 */         double tmp = d;
/* 5564 */         d = d1;
/* 5565 */         d1 = tmp;
/*      */       } 
/* 5567 */       d5 = d1;
/* 5568 */       d6 = d;
/* 5569 */       d7 = d2;
/* 5570 */       d8 = d3;
/* 5571 */       d5 = d;
/* 5572 */       d6 = d1;
/* 5573 */       d2 = d3;
/* 5574 */       d3 = d7;
/*      */     }
/* 5576 */     else if (this.h == 2) {
/*      */       
/* 5578 */       d = ((i + 16) - maxY * 16.0D) / 256.0D;
/* 5579 */       d2 = (j + minX * 16.0D) / 256.0D;
/* 5580 */       d1 = ((i + 16) - minY * 16.0D - 0.01D) / 256.0D;
/* 5581 */       d3 = (j + maxX * 16.0D - 0.01D) / 256.0D;
/*      */       
/* 5583 */       if (this.e) {
/*      */         
/* 5585 */         double tmp = d;
/* 5586 */         d = d1;
/* 5587 */         d1 = tmp;
/*      */       } 
/* 5589 */       d5 = d1;
/* 5590 */       d6 = d;
/* 5591 */       d7 = d2;
/* 5592 */       d8 = d3;
/* 5593 */       d = d5;
/* 5594 */       d1 = d6;
/* 5595 */       d7 = d3;
/* 5596 */       d8 = d2;
/*      */     }
/* 5598 */     else if (this.h == 3) {
/*      */       
/* 5600 */       d = ((i + 16) - minX * 16.0D - 0.01D) / 256.0D;
/* 5601 */       d1 = ((i + 16) - maxX * 16.0D) / 256.0D;
/* 5602 */       d2 = (j + maxY * 16.0D - 0.01D) / 256.0D;
/* 5603 */       d3 = (j + minY * 16.0D) / 256.0D;
/*      */       
/* 5605 */       if (this.e) {
/*      */         
/* 5607 */         double tmp = d;
/* 5608 */         d = d1;
/* 5609 */         d1 = tmp;
/*      */       } 
/* 5611 */       d5 = d1;
/* 5612 */       d6 = d;
/* 5613 */       d7 = d2;
/* 5614 */       d8 = d3;
/*      */     } 
/*      */     
/* 5617 */     this.h = 0;
/* 5618 */     this.e = false;
/*      */     
/* 5620 */     double d9 = par2 + par1Block.bV;
/* 5621 */     double d10 = par2 + par1Block.bY;
/* 5622 */     double d11 = par4 + par1Block.bW;
/* 5623 */     double d12 = par4 + par1Block.bZ;
/* 5624 */     double d13 = par6 + par1Block.ca;
/* 5625 */     if (this.m) {
/*      */       
/* 5627 */       tessellator.a(this.an, this.ar, this.av);
/* 5628 */       tessellator.b(this.aj);
/* 5629 */       tessellator.a(d9, d12, d13, d, d2);
/* 5630 */       tessellator.a(this.ao, this.as, this.aw);
/* 5631 */       tessellator.b(this.ak);
/* 5632 */       tessellator.a(d9, d11, d13, d6, d8);
/* 5633 */       tessellator.a(this.ap, this.at, this.ax);
/* 5634 */       tessellator.b(this.al);
/* 5635 */       tessellator.a(d10, d11, d13, d1, d3);
/* 5636 */       tessellator.a(this.aq, this.au, this.ay);
/* 5637 */       tessellator.b(this.am);
/* 5638 */       tessellator.a(d10, d12, d13, d5, d7);
/*      */     } else {
/*      */       
/* 5641 */       tessellator.a(d9, d12, d13, d, d2);
/* 5642 */       tessellator.a(d9, d11, d13, d6, d8);
/* 5643 */       tessellator.a(d10, d11, d13, d1, d3);
/* 5644 */       tessellator.a(d10, d12, d13, d5, d7);
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void e(pb par1Block, double par2, double par4, double par6, int par8) {
/* 5653 */     adz tessellator = adz.a;
/* 5654 */     if (this.d >= 0)
/*      */     {
/* 5656 */       par8 = this.d;
/*      */     }
/*      */     
/* 5659 */     if (Config.isConnectedTextures() && this.d < 0) {
/*      */       
/* 5661 */       int tex = ConnectedTextures.getConnectedTexture(this.a, par1Block, (int)par2, (int)par4, (int)par6, 4, par8);
/* 5662 */       if (tex >= 0) {
/*      */         
/* 5664 */         int ctmTex = tex / 256;
/* 5665 */         tessellator = adz.a.getSubTessellator(ctmTex);
/* 5666 */         par8 = tex % 256;
/*      */       } 
/*      */     } 
/*      */     
/* 5670 */     if (Config.isNaturalTextures() && this.d < 0) {
/*      */       
/* 5672 */       NaturalProperties nps = NaturalTextures.getNaturalProperties(tessellator.textureID, par8);
/* 5673 */       if (nps != null) {
/*      */         
/* 5675 */         int rand = Config.getRandom((int)par2, (int)par4, (int)par6, 4);
/* 5676 */         if (nps.rotation > 1) {
/* 5677 */           this.j = rand & 0x3;
/*      */         }
/* 5679 */         if (nps.rotation == 2) {
/* 5680 */           this.j = this.j / 2 * 3;
/*      */         }
/* 5682 */         if (nps.flip) {
/* 5683 */           this.e = ((rand & 0x4) != 0);
/*      */         }
/*      */       } 
/*      */     } 
/* 5687 */     double minZ = par1Block.bX;
/* 5688 */     double maxZ = par1Block.ca;
/* 5689 */     double minY = par1Block.bW;
/* 5690 */     double maxY = par1Block.bZ;
/* 5691 */     if (minZ < 0.0D || maxZ > 1.0D) {
/*      */       
/* 5693 */       minZ = 0.0D;
/* 5694 */       maxZ = 1.0D;
/*      */     } 
/* 5696 */     if (minY < 0.0D || maxY > 1.0D) {
/*      */       
/* 5698 */       minY = 0.0D;
/* 5699 */       maxY = 1.0D;
/*      */     } 
/*      */     
/* 5702 */     int i = (par8 & 0xF) << 4;
/* 5703 */     int j = par8 & 0xF0;
/* 5704 */     double d = (i + minZ * 16.0D) / 256.0D;
/* 5705 */     double d1 = (i + maxZ * 16.0D - 0.01D) / 256.0D;
/* 5706 */     double d2 = ((j + 16) - maxY * 16.0D) / 256.0D;
/* 5707 */     double d3 = ((j + 16) - minY * 16.0D - 0.01D) / 256.0D;
/* 5708 */     if (this.e) {
/*      */       
/* 5710 */       double d4 = d;
/* 5711 */       d = d1;
/* 5712 */       d1 = d4;
/*      */     } 
/* 5714 */     double d5 = d1;
/* 5715 */     double d6 = d;
/* 5716 */     double d7 = d2;
/* 5717 */     double d8 = d3;
/* 5718 */     if (this.j == 1) {
/*      */       
/* 5720 */       d = (i + minY * 16.0D) / 256.0D;
/* 5721 */       d2 = ((j + 16) - maxZ * 16.0D) / 256.0D;
/* 5722 */       d1 = (i + maxY * 16.0D - 0.01D) / 256.0D;
/* 5723 */       d3 = ((j + 16) - minZ * 16.0D - 0.01D) / 256.0D;
/*      */       
/* 5725 */       if (this.e) {
/*      */         
/* 5727 */         double tmp = d;
/* 5728 */         d = d1;
/* 5729 */         d1 = tmp;
/*      */       } 
/* 5731 */       d5 = d1;
/* 5732 */       d6 = d;
/* 5733 */       d7 = d2;
/* 5734 */       d8 = d3;
/* 5735 */       d5 = d;
/* 5736 */       d6 = d1;
/* 5737 */       d2 = d3;
/* 5738 */       d3 = d7;
/*      */     }
/* 5740 */     else if (this.j == 2) {
/*      */       
/* 5742 */       d = ((i + 16) - maxY * 16.0D) / 256.0D;
/* 5743 */       d2 = (j + minZ * 16.0D) / 256.0D;
/* 5744 */       d1 = ((i + 16) - minY * 16.0D - 0.01D) / 256.0D;
/* 5745 */       d3 = (j + maxZ * 16.0D - 0.01D) / 256.0D;
/*      */       
/* 5747 */       if (this.e) {
/*      */         
/* 5749 */         double tmp = d;
/* 5750 */         d = d1;
/* 5751 */         d1 = tmp;
/*      */       } 
/* 5753 */       d5 = d1;
/* 5754 */       d6 = d;
/* 5755 */       d7 = d2;
/* 5756 */       d8 = d3;
/* 5757 */       d = d5;
/* 5758 */       d1 = d6;
/* 5759 */       d7 = d3;
/* 5760 */       d8 = d2;
/*      */     }
/* 5762 */     else if (this.j == 3) {
/*      */       
/* 5764 */       d = ((i + 16) - minZ * 16.0D - 0.01D) / 256.0D;
/* 5765 */       d1 = ((i + 16) - maxZ * 16.0D) / 256.0D;
/* 5766 */       d2 = (j + maxY * 16.0D - 0.01D) / 256.0D;
/* 5767 */       d3 = (j + minY * 16.0D) / 256.0D;
/*      */       
/* 5769 */       if (this.e) {
/*      */         
/* 5771 */         double tmp = d;
/* 5772 */         d = d1;
/* 5773 */         d1 = tmp;
/*      */       } 
/* 5775 */       d5 = d1;
/* 5776 */       d6 = d;
/* 5777 */       d7 = d2;
/* 5778 */       d8 = d3;
/*      */     } 
/*      */     
/* 5781 */     this.j = 0;
/* 5782 */     this.e = false;
/*      */     
/* 5784 */     double d9 = par2 + par1Block.bV;
/* 5785 */     double d10 = par4 + par1Block.bW;
/* 5786 */     double d11 = par4 + par1Block.bZ;
/* 5787 */     double d12 = par6 + par1Block.bX;
/* 5788 */     double d13 = par6 + par1Block.ca;
/* 5789 */     if (this.m) {
/*      */       
/* 5791 */       tessellator.a(this.an, this.ar, this.av);
/* 5792 */       tessellator.b(this.aj);
/* 5793 */       tessellator.a(d9, d11, d13, d5, d7);
/* 5794 */       tessellator.a(this.ao, this.as, this.aw);
/* 5795 */       tessellator.b(this.ak);
/* 5796 */       tessellator.a(d9, d11, d12, d, d2);
/* 5797 */       tessellator.a(this.ap, this.at, this.ax);
/* 5798 */       tessellator.b(this.al);
/* 5799 */       tessellator.a(d9, d10, d12, d6, d8);
/* 5800 */       tessellator.a(this.aq, this.au, this.ay);
/* 5801 */       tessellator.b(this.am);
/* 5802 */       tessellator.a(d9, d10, d13, d1, d3);
/*      */     } else {
/*      */       
/* 5805 */       tessellator.a(d9, d11, d13, d5, d7);
/* 5806 */       tessellator.a(d9, d11, d12, d, d2);
/* 5807 */       tessellator.a(d9, d10, d12, d6, d8);
/* 5808 */       tessellator.a(d9, d10, d13, d1, d3);
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void f(pb par1Block, double par2, double par4, double par6, int par8) {
/* 5817 */     adz tessellator = adz.a;
/* 5818 */     if (this.d >= 0)
/*      */     {
/* 5820 */       par8 = this.d;
/*      */     }
/*      */     
/* 5823 */     if (Config.isConnectedTextures() && this.d < 0) {
/*      */       
/* 5825 */       int tex = ConnectedTextures.getConnectedTexture(this.a, par1Block, (int)par2, (int)par4, (int)par6, 5, par8);
/* 5826 */       if (tex >= 0) {
/*      */         
/* 5828 */         int ctmTex = tex / 256;
/* 5829 */         tessellator = adz.a.getSubTessellator(ctmTex);
/* 5830 */         par8 = tex % 256;
/*      */       } 
/*      */     } 
/*      */     
/* 5834 */     if (Config.isNaturalTextures() && this.d < 0) {
/*      */       
/* 5836 */       NaturalProperties nps = NaturalTextures.getNaturalProperties(tessellator.textureID, par8);
/* 5837 */       if (nps != null) {
/*      */         
/* 5839 */         int rand = Config.getRandom((int)par2, (int)par4, (int)par6, 5);
/* 5840 */         if (nps.rotation > 1) {
/* 5841 */           this.i = rand & 0x3;
/*      */         }
/* 5843 */         if (nps.rotation == 2) {
/* 5844 */           this.i = this.i / 2 * 3;
/*      */         }
/* 5846 */         if (nps.flip) {
/* 5847 */           this.e = ((rand & 0x4) != 0);
/*      */         }
/*      */       } 
/*      */     } 
/* 5851 */     double minZ = par1Block.bX;
/* 5852 */     double maxZ = par1Block.ca;
/* 5853 */     double minY = par1Block.bW;
/* 5854 */     double maxY = par1Block.bZ;
/* 5855 */     if (minZ < 0.0D || maxZ > 1.0D) {
/*      */       
/* 5857 */       minZ = 0.0D;
/* 5858 */       maxZ = 1.0D;
/*      */     } 
/* 5860 */     if (minY < 0.0D || maxY > 1.0D) {
/*      */       
/* 5862 */       minY = 0.0D;
/* 5863 */       maxY = 1.0D;
/*      */     } 
/*      */     
/* 5866 */     int i = (par8 & 0xF) << 4;
/* 5867 */     int j = par8 & 0xF0;
/* 5868 */     double d = (i + minZ * 16.0D) / 256.0D;
/* 5869 */     double d1 = (i + maxZ * 16.0D - 0.01D) / 256.0D;
/* 5870 */     double d2 = ((j + 16) - maxY * 16.0D) / 256.0D;
/* 5871 */     double d3 = ((j + 16) - minY * 16.0D - 0.01D) / 256.0D;
/* 5872 */     if (this.e) {
/*      */       
/* 5874 */       double d4 = d;
/* 5875 */       d = d1;
/* 5876 */       d1 = d4;
/*      */     } 
/* 5878 */     double d5 = d1;
/* 5879 */     double d6 = d;
/* 5880 */     double d7 = d2;
/* 5881 */     double d8 = d3;
/* 5882 */     if (this.i == 2) {
/*      */       
/* 5884 */       d = (i + minY * 16.0D) / 256.0D;
/* 5885 */       d2 = ((j + 16) - minZ * 16.0D - 0.01D) / 256.0D;
/* 5886 */       d1 = (i + maxY * 16.0D - 0.01D) / 256.0D;
/* 5887 */       d3 = ((j + 16) - maxZ * 16.0D) / 256.0D;
/*      */       
/* 5889 */       if (this.e) {
/*      */         
/* 5891 */         double tmp = d;
/* 5892 */         d = d1;
/* 5893 */         d1 = tmp;
/*      */       } 
/* 5895 */       d5 = d1;
/* 5896 */       d6 = d;
/* 5897 */       d7 = d2;
/* 5898 */       d8 = d3;
/* 5899 */       d5 = d;
/* 5900 */       d6 = d1;
/* 5901 */       d2 = d3;
/* 5902 */       d3 = d7;
/*      */     }
/* 5904 */     else if (this.i == 1) {
/*      */       
/* 5906 */       d = ((i + 16) - maxY * 16.0D) / 256.0D;
/* 5907 */       d2 = (j + maxZ * 16.0D - 0.01D) / 256.0D;
/* 5908 */       d1 = ((i + 16) - minY * 16.0D - 0.01D) / 256.0D;
/* 5909 */       d3 = (j + minZ * 16.0D) / 256.0D;
/*      */       
/* 5911 */       if (this.e) {
/*      */         
/* 5913 */         double tmp = d;
/* 5914 */         d = d1;
/* 5915 */         d1 = tmp;
/*      */       } 
/* 5917 */       d5 = d1;
/* 5918 */       d6 = d;
/* 5919 */       d7 = d2;
/* 5920 */       d8 = d3;
/* 5921 */       d = d5;
/* 5922 */       d1 = d6;
/* 5923 */       d7 = d3;
/* 5924 */       d8 = d2;
/*      */     }
/* 5926 */     else if (this.i == 3) {
/*      */       
/* 5928 */       d = ((i + 16) - minZ * 16.0D - 0.01D) / 256.0D;
/* 5929 */       d1 = ((i + 16) - maxZ * 16.0D) / 256.0D;
/* 5930 */       d2 = (j + maxY * 16.0D - 0.01D) / 256.0D;
/* 5931 */       d3 = (j + minY * 16.0D) / 256.0D;
/*      */       
/* 5933 */       if (this.e) {
/*      */         
/* 5935 */         double tmp = d;
/* 5936 */         d = d1;
/* 5937 */         d1 = tmp;
/*      */       } 
/* 5939 */       d5 = d1;
/* 5940 */       d6 = d;
/* 5941 */       d7 = d2;
/* 5942 */       d8 = d3;
/*      */     } 
/*      */     
/* 5945 */     this.i = 0;
/* 5946 */     this.e = false;
/*      */     
/* 5948 */     double d9 = par2 + par1Block.bY;
/* 5949 */     double d10 = par4 + par1Block.bW;
/* 5950 */     double d11 = par4 + par1Block.bZ;
/* 5951 */     double d12 = par6 + par1Block.bX;
/* 5952 */     double d13 = par6 + par1Block.ca;
/* 5953 */     if (this.m) {
/*      */       
/* 5955 */       tessellator.a(this.an, this.ar, this.av);
/* 5956 */       tessellator.b(this.aj);
/* 5957 */       tessellator.a(d9, d10, d13, d6, d8);
/* 5958 */       tessellator.a(this.ao, this.as, this.aw);
/* 5959 */       tessellator.b(this.ak);
/* 5960 */       tessellator.a(d9, d10, d12, d1, d3);
/* 5961 */       tessellator.a(this.ap, this.at, this.ax);
/* 5962 */       tessellator.b(this.al);
/* 5963 */       tessellator.a(d9, d11, d12, d5, d7);
/* 5964 */       tessellator.a(this.aq, this.au, this.ay);
/* 5965 */       tessellator.b(this.am);
/* 5966 */       tessellator.a(d9, d11, d13, d, d2);
/*      */     } else {
/*      */       
/* 5969 */       tessellator.a(d9, d10, d13, d6, d8);
/* 5970 */       tessellator.a(d9, d10, d12, d1, d3);
/* 5971 */       tessellator.a(d9, d11, d12, d5, d7);
/* 5972 */       tessellator.a(d9, d11, d13, d, d2);
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(pb par1Block, int par2, float par3) {
/* 5981 */     adz tessellator = adz.a;
/* 5982 */     boolean flag = (par1Block.bO == pb.u.bO);
/* 5983 */     if (this.c) {
/*      */       
/* 5985 */       int i = par1Block.d(par2);
/* 5986 */       if (flag)
/*      */       {
/* 5988 */         i = 16777215;
/*      */       }
/* 5990 */       float f = (i >> 16 & 0xFF) / 255.0F;
/* 5991 */       float f2 = (i >> 8 & 0xFF) / 255.0F;
/* 5992 */       float f6 = (i & 0xFF) / 255.0F;
/* 5993 */       GL11.glColor4f(f * par3, f2 * par3, f6 * par3, 1.0F);
/*      */     } 
/* 5995 */     int j = par1Block.d();
/* 5996 */     if (j == 0 || j == 16) {
/*      */       
/* 5998 */       if (j == 16)
/*      */       {
/* 6000 */         par2 = 1;
/*      */       }
/* 6002 */       par1Block.h();
/* 6003 */       GL11.glTranslatef(-0.5F, -0.5F, -0.5F);
/* 6004 */       tessellator.b();
/* 6005 */       tessellator.b(0.0F, -1.0F, 0.0F);
/* 6006 */       a(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a(0, par2));
/* 6007 */       tessellator.a();
/* 6008 */       if (flag && this.c) {
/*      */         
/* 6010 */         int k = par1Block.d(par2);
/* 6011 */         float f3 = (k >> 16 & 0xFF) / 255.0F;
/* 6012 */         float f7 = (k >> 8 & 0xFF) / 255.0F;
/* 6013 */         float f8 = (k & 0xFF) / 255.0F;
/* 6014 */         GL11.glColor4f(f3 * par3, f7 * par3, f8 * par3, 1.0F);
/*      */       } 
/* 6016 */       tessellator.b();
/* 6017 */       tessellator.b(0.0F, 1.0F, 0.0F);
/* 6018 */       b(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a(1, par2));
/* 6019 */       tessellator.a();
/* 6020 */       if (flag && this.c)
/*      */       {
/* 6022 */         GL11.glColor4f(par3, par3, par3, 1.0F);
/*      */       }
/* 6024 */       tessellator.b();
/* 6025 */       tessellator.b(0.0F, 0.0F, -1.0F);
/* 6026 */       c(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a(2, par2));
/* 6027 */       tessellator.a();
/* 6028 */       tessellator.b();
/* 6029 */       tessellator.b(0.0F, 0.0F, 1.0F);
/* 6030 */       d(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a(3, par2));
/* 6031 */       tessellator.a();
/* 6032 */       tessellator.b();
/* 6033 */       tessellator.b(-1.0F, 0.0F, 0.0F);
/* 6034 */       e(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a(4, par2));
/* 6035 */       tessellator.a();
/* 6036 */       tessellator.b();
/* 6037 */       tessellator.b(1.0F, 0.0F, 0.0F);
/* 6038 */       f(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a(5, par2));
/* 6039 */       tessellator.a();
/* 6040 */       GL11.glTranslatef(0.5F, 0.5F, 0.5F);
/*      */     }
/* 6042 */     else if (j == 1) {
/*      */       
/* 6044 */       tessellator.b();
/* 6045 */       tessellator.b(0.0F, -1.0F, 0.0F);
/* 6046 */       a(par1Block, par2, -0.5D, -0.5D, -0.5D);
/* 6047 */       tessellator.a();
/*      */     }
/* 6049 */     else if (j == 19) {
/*      */       
/* 6051 */       tessellator.b();
/* 6052 */       tessellator.b(0.0F, -1.0F, 0.0F);
/* 6053 */       par1Block.h();
/* 6054 */       a(par1Block, par2, par1Block.bZ, -0.5D, -0.5D, -0.5D);
/* 6055 */       tessellator.a();
/*      */     }
/* 6057 */     else if (j == 23) {
/*      */       
/* 6059 */       tessellator.b();
/* 6060 */       tessellator.b(0.0F, -1.0F, 0.0F);
/* 6061 */       par1Block.h();
/* 6062 */       tessellator.a();
/*      */     }
/* 6064 */     else if (j == 13) {
/*      */       
/* 6066 */       par1Block.h();
/* 6067 */       GL11.glTranslatef(-0.5F, -0.5F, -0.5F);
/* 6068 */       float f1 = 0.0625F;
/* 6069 */       tessellator.b();
/* 6070 */       tessellator.b(0.0F, -1.0F, 0.0F);
/* 6071 */       a(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(0));
/* 6072 */       tessellator.a();
/* 6073 */       tessellator.b();
/* 6074 */       tessellator.b(0.0F, 1.0F, 0.0F);
/* 6075 */       b(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(1));
/* 6076 */       tessellator.a();
/* 6077 */       tessellator.b();
/* 6078 */       tessellator.b(0.0F, 0.0F, -1.0F);
/* 6079 */       tessellator.c(0.0F, 0.0F, f1);
/* 6080 */       c(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(2));
/* 6081 */       tessellator.c(0.0F, 0.0F, -f1);
/* 6082 */       tessellator.a();
/* 6083 */       tessellator.b();
/* 6084 */       tessellator.b(0.0F, 0.0F, 1.0F);
/* 6085 */       tessellator.c(0.0F, 0.0F, -f1);
/* 6086 */       d(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(3));
/* 6087 */       tessellator.c(0.0F, 0.0F, f1);
/* 6088 */       tessellator.a();
/* 6089 */       tessellator.b();
/* 6090 */       tessellator.b(-1.0F, 0.0F, 0.0F);
/* 6091 */       tessellator.c(f1, 0.0F, 0.0F);
/* 6092 */       e(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(4));
/* 6093 */       tessellator.c(-f1, 0.0F, 0.0F);
/* 6094 */       tessellator.a();
/* 6095 */       tessellator.b();
/* 6096 */       tessellator.b(1.0F, 0.0F, 0.0F);
/* 6097 */       tessellator.c(-f1, 0.0F, 0.0F);
/* 6098 */       f(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(5));
/* 6099 */       tessellator.c(f1, 0.0F, 0.0F);
/* 6100 */       tessellator.a();
/* 6101 */       GL11.glTranslatef(0.5F, 0.5F, 0.5F);
/*      */     }
/* 6103 */     else if (j == 22) {
/*      */       
/* 6105 */       df.a.a(par1Block, par2, par3);
/* 6106 */       GL11.glEnable(32826);
/*      */     }
/* 6108 */     else if (j == 6) {
/*      */       
/* 6110 */       tessellator.b();
/* 6111 */       tessellator.b(0.0F, -1.0F, 0.0F);
/* 6112 */       b(par1Block, par2, -0.5D, -0.5D, -0.5D);
/* 6113 */       tessellator.a();
/*      */     }
/* 6115 */     else if (j == 2) {
/*      */       
/* 6117 */       tessellator.b();
/* 6118 */       tessellator.b(0.0F, -1.0F, 0.0F);
/* 6119 */       a(par1Block, -0.5D, -0.5D, -0.5D, 0.0D, 0.0D);
/* 6120 */       tessellator.a();
/*      */     }
/* 6122 */     else if (j == 10) {
/*      */       
/* 6124 */       for (int l = 0; l < 2; l++)
/*      */       {
/* 6126 */         if (l == 0)
/*      */         {
/* 6128 */           par1Block.a(0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 0.5F);
/*      */         }
/* 6130 */         if (l == 1)
/*      */         {
/* 6132 */           par1Block.a(0.0F, 0.0F, 0.5F, 1.0F, 0.5F, 1.0F);
/*      */         }
/* 6134 */         GL11.glTranslatef(-0.5F, -0.5F, -0.5F);
/* 6135 */         tessellator.b();
/* 6136 */         tessellator.b(0.0F, -1.0F, 0.0F);
/* 6137 */         a(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(0));
/* 6138 */         tessellator.a();
/* 6139 */         tessellator.b();
/* 6140 */         tessellator.b(0.0F, 1.0F, 0.0F);
/* 6141 */         b(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(1));
/* 6142 */         tessellator.a();
/* 6143 */         tessellator.b();
/* 6144 */         tessellator.b(0.0F, 0.0F, -1.0F);
/* 6145 */         c(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(2));
/* 6146 */         tessellator.a();
/* 6147 */         tessellator.b();
/* 6148 */         tessellator.b(0.0F, 0.0F, 1.0F);
/* 6149 */         d(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(3));
/* 6150 */         tessellator.a();
/* 6151 */         tessellator.b();
/* 6152 */         tessellator.b(-1.0F, 0.0F, 0.0F);
/* 6153 */         e(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(4));
/* 6154 */         tessellator.a();
/* 6155 */         tessellator.b();
/* 6156 */         tessellator.b(1.0F, 0.0F, 0.0F);
/* 6157 */         f(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(5));
/* 6158 */         tessellator.a();
/* 6159 */         GL11.glTranslatef(0.5F, 0.5F, 0.5F);
/*      */       }
/*      */     
/* 6162 */     } else if (j == 27) {
/*      */       
/* 6164 */       int i1 = 0;
/* 6165 */       GL11.glTranslatef(-0.5F, -0.5F, -0.5F);
/* 6166 */       tessellator.b();
/* 6167 */       for (int l1 = 0; l1 < 8; l1++) {
/*      */         
/* 6169 */         int i2 = 0;
/* 6170 */         byte byte0 = 1;
/* 6171 */         if (l1 == 0)
/*      */         {
/* 6173 */           i2 = 2;
/*      */         }
/* 6175 */         if (l1 == 1)
/*      */         {
/* 6177 */           i2 = 3;
/*      */         }
/* 6179 */         if (l1 == 2)
/*      */         {
/* 6181 */           i2 = 4;
/*      */         }
/* 6183 */         if (l1 == 3) {
/*      */           
/* 6185 */           i2 = 5;
/* 6186 */           byte0 = 2;
/*      */         } 
/* 6188 */         if (l1 == 4) {
/*      */           
/* 6190 */           i2 = 6;
/* 6191 */           byte0 = 3;
/*      */         } 
/* 6193 */         if (l1 == 5) {
/*      */           
/* 6195 */           i2 = 7;
/* 6196 */           byte0 = 5;
/*      */         } 
/* 6198 */         if (l1 == 6) {
/*      */           
/* 6200 */           i2 = 6;
/* 6201 */           byte0 = 2;
/*      */         } 
/* 6203 */         if (l1 == 7)
/*      */         {
/* 6205 */           i2 = 3;
/*      */         }
/* 6207 */         float f9 = i2 / 16.0F;
/* 6208 */         float f10 = 1.0F - i1 / 16.0F;
/* 6209 */         float f11 = 1.0F - (i1 + byte0) / 16.0F;
/* 6210 */         i1 += byte0;
/* 6211 */         par1Block.a(0.5F - f9, f11, 0.5F - f9, 0.5F + f9, f10, 0.5F + f9);
/* 6212 */         tessellator.b(0.0F, -1.0F, 0.0F);
/* 6213 */         a(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(0));
/* 6214 */         tessellator.b(0.0F, 1.0F, 0.0F);
/* 6215 */         b(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(1));
/* 6216 */         tessellator.b(0.0F, 0.0F, -1.0F);
/* 6217 */         c(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(2));
/* 6218 */         tessellator.b(0.0F, 0.0F, 1.0F);
/* 6219 */         d(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(3));
/* 6220 */         tessellator.b(-1.0F, 0.0F, 0.0F);
/* 6221 */         e(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(4));
/* 6222 */         tessellator.b(1.0F, 0.0F, 0.0F);
/* 6223 */         f(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(5));
/*      */       } 
/*      */       
/* 6226 */       tessellator.a();
/* 6227 */       GL11.glTranslatef(0.5F, 0.5F, 0.5F);
/* 6228 */       par1Block.a(0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F);
/*      */     }
/* 6230 */     else if (j == 11) {
/*      */       
/* 6232 */       for (int j1 = 0; j1 < 4; j1++) {
/*      */         
/* 6234 */         float f4 = 0.125F;
/* 6235 */         if (j1 == 0)
/*      */         {
/* 6237 */           par1Block.a(0.5F - f4, 0.0F, 0.0F, 0.5F + f4, 1.0F, f4 * 2.0F);
/*      */         }
/* 6239 */         if (j1 == 1)
/*      */         {
/* 6241 */           par1Block.a(0.5F - f4, 0.0F, 1.0F - f4 * 2.0F, 0.5F + f4, 1.0F, 1.0F);
/*      */         }
/* 6243 */         f4 = 0.0625F;
/* 6244 */         if (j1 == 2)
/*      */         {
/* 6246 */           par1Block.a(0.5F - f4, 1.0F - f4 * 3.0F, -f4 * 2.0F, 0.5F + f4, 1.0F - f4, 1.0F + f4 * 2.0F);
/*      */         }
/* 6248 */         if (j1 == 3)
/*      */         {
/* 6250 */           par1Block.a(0.5F - f4, 0.5F - f4 * 3.0F, -f4 * 2.0F, 0.5F + f4, 0.5F - f4, 1.0F + f4 * 2.0F);
/*      */         }
/* 6252 */         GL11.glTranslatef(-0.5F, -0.5F, -0.5F);
/* 6253 */         tessellator.b();
/* 6254 */         tessellator.b(0.0F, -1.0F, 0.0F);
/* 6255 */         a(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(0));
/* 6256 */         tessellator.a();
/* 6257 */         tessellator.b();
/* 6258 */         tessellator.b(0.0F, 1.0F, 0.0F);
/* 6259 */         b(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(1));
/* 6260 */         tessellator.a();
/* 6261 */         tessellator.b();
/* 6262 */         tessellator.b(0.0F, 0.0F, -1.0F);
/* 6263 */         c(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(2));
/* 6264 */         tessellator.a();
/* 6265 */         tessellator.b();
/* 6266 */         tessellator.b(0.0F, 0.0F, 1.0F);
/* 6267 */         d(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(3));
/* 6268 */         tessellator.a();
/* 6269 */         tessellator.b();
/* 6270 */         tessellator.b(-1.0F, 0.0F, 0.0F);
/* 6271 */         e(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(4));
/* 6272 */         tessellator.a();
/* 6273 */         tessellator.b();
/* 6274 */         tessellator.b(1.0F, 0.0F, 0.0F);
/* 6275 */         f(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(5));
/* 6276 */         tessellator.a();
/* 6277 */         GL11.glTranslatef(0.5F, 0.5F, 0.5F);
/*      */       } 
/*      */       
/* 6280 */       par1Block.a(0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F);
/*      */     }
/* 6282 */     else if (j == 21) {
/*      */       
/* 6284 */       for (int k1 = 0; k1 < 3; k1++) {
/*      */         
/* 6286 */         float f5 = 0.0625F;
/* 6287 */         if (k1 == 0)
/*      */         {
/* 6289 */           par1Block.a(0.5F - f5, 0.3F, 0.0F, 0.5F + f5, 1.0F, f5 * 2.0F);
/*      */         }
/* 6291 */         if (k1 == 1)
/*      */         {
/* 6293 */           par1Block.a(0.5F - f5, 0.3F, 1.0F - f5 * 2.0F, 0.5F + f5, 1.0F, 1.0F);
/*      */         }
/* 6295 */         f5 = 0.0625F;
/* 6296 */         if (k1 == 2)
/*      */         {
/* 6298 */           par1Block.a(0.5F - f5, 0.5F, 0.0F, 0.5F + f5, 1.0F - f5, 1.0F);
/*      */         }
/* 6300 */         GL11.glTranslatef(-0.5F, -0.5F, -0.5F);
/* 6301 */         tessellator.b();
/* 6302 */         tessellator.b(0.0F, -1.0F, 0.0F);
/* 6303 */         a(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(0));
/* 6304 */         tessellator.a();
/* 6305 */         tessellator.b();
/* 6306 */         tessellator.b(0.0F, 1.0F, 0.0F);
/* 6307 */         b(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(1));
/* 6308 */         tessellator.a();
/* 6309 */         tessellator.b();
/* 6310 */         tessellator.b(0.0F, 0.0F, -1.0F);
/* 6311 */         c(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(2));
/* 6312 */         tessellator.a();
/* 6313 */         tessellator.b();
/* 6314 */         tessellator.b(0.0F, 0.0F, 1.0F);
/* 6315 */         d(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(3));
/* 6316 */         tessellator.a();
/* 6317 */         tessellator.b();
/* 6318 */         tessellator.b(-1.0F, 0.0F, 0.0F);
/* 6319 */         e(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(4));
/* 6320 */         tessellator.a();
/* 6321 */         tessellator.b();
/* 6322 */         tessellator.b(1.0F, 0.0F, 0.0F);
/* 6323 */         f(par1Block, 0.0D, 0.0D, 0.0D, par1Block.a_(5));
/* 6324 */         tessellator.a();
/* 6325 */         GL11.glTranslatef(0.5F, 0.5F, 0.5F);
/*      */       } 
/*      */       
/* 6328 */       par1Block.a(0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F);
/*      */ 
/*      */ 
/*      */     
/*      */     }
/* 6333 */     else if (Reflector.hasClass(0)) {
/* 6334 */       Reflector.callVoid(1, new Object[] { this, par1Block, Integer.valueOf(par2), Integer.valueOf(j) });
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean a(int par0) {
/* 6343 */     if (par0 == 0)
/*      */     {
/* 6345 */       return true;
/*      */     }
/* 6347 */     if (par0 == 13)
/*      */     {
/* 6349 */       return true;
/*      */     }
/* 6351 */     if (par0 == 10)
/*      */     {
/* 6353 */       return true;
/*      */     }
/* 6355 */     if (par0 == 11)
/*      */     {
/* 6357 */       return true;
/*      */     }
/* 6359 */     if (par0 == 27)
/*      */     {
/* 6361 */       return true;
/*      */     }
/* 6363 */     if (par0 == 22)
/*      */     {
/* 6365 */       return true;
/*      */     }
/* 6367 */     if (par0 == 21)
/*      */     {
/* 6369 */       return true;
/*      */     }
/* 6371 */     if (par0 == 16)
/*      */     {
/* 6373 */       return true;
/*      */     }
/*      */     
/* 6376 */     if (Reflector.hasClass(0)) {
/* 6377 */       return Reflector.callBoolean(2, new Object[] { Integer.valueOf(par0) });
/*      */     }
/* 6379 */     return false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/* 6385 */   public static float[][] redstoneColors = new float[16][]; static {
/* 6386 */     for (int i = 0; i < redstoneColors.length; i++) {
/*      */       
/* 6388 */       float f = i / 15.0F;
/* 6389 */       float f1 = f * 0.6F + 0.4F;
/* 6390 */       if (i == 0)
/*      */       {
/* 6392 */         f = 0.0F;
/*      */       }
/* 6394 */       float f2 = f * f * 0.7F - 0.5F;
/* 6395 */       float f3 = f * f * 0.6F - 0.7F;
/* 6396 */       if (f2 < 0.0F)
/*      */       {
/* 6398 */         f2 = 0.0F;
/*      */       }
/* 6400 */       if (f3 < 0.0F)
/*      */       {
/* 6402 */         f3 = 0.0F;
/*      */       }
/* 6404 */       (new float[3])[0] = f1; (new float[3])[1] = f2; (new float[3])[2] = f3; redstoneColors[i] = new float[3];
/*      */     } 
/*      */   }
/*      */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\vl.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */