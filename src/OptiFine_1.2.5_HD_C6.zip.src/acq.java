/*      */ import java.util.Collection;
/*      */ import java.util.HashMap;
/*      */ import java.util.Iterator;
/*      */ import java.util.List;
/*      */ import java.util.Random;
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
/*      */ public abstract class acq
/*      */   extends nn
/*      */ {
/*      */   public int ba;
/*      */   public float bb;
/*      */   public float bc;
/*      */   public float bd;
/*      */   public float be;
/*      */   public float bf;
/*      */   public float bg;
/*      */   protected float bh;
/*      */   protected float bi;
/*      */   protected float bj;
/*      */   protected float bk;
/*      */   protected boolean bl;
/*      */   protected String bm;
/*      */   protected boolean bn;
/*      */   protected float bo;
/*      */   protected String bp;
/*      */   protected float bq;
/*      */   protected int br;
/*      */   protected float bs;
/*      */   public float bt;
/*      */   public float bu;
/*      */   public float bv;
/*      */   public float bw;
/*      */   protected int bx;
/*      */   public int by;
/*      */   public int bz;
/*      */   private int a;
/*      */   public int bA;
/*      */   public int bB;
/*      */   public float bC;
/*      */   public int bD;
/*      */   public int bE;
/*      */   public float bF;
/*      */   public float bG;
/*      */   protected boolean bH;
/*      */   protected int bI;
/*      */   public int bJ;
/*      */   public float bK;
/*      */   public float bL;
/*      */   public float bM;
/*      */   public float bN;
/*      */   protected yw bO;
/*      */   protected int bP;
/*      */   private acq b;
/*      */   private int c;
/*      */   private acq d;
/*      */   public int bQ;
/*      */   public int bR;
/*      */   protected HashMap bS;
/*      */   private boolean e;
/*      */   private int am;
/*      */   private vz an;
/*      */   private j ao;
/*      */   private aej ap;
/*      */   private vh aq;
/*      */   private rh ar;
/*      */   protected nb bT;
/*      */   protected nb bU;
/*      */   private acq as;
/*      */   private abk at;
/*      */   private float au;
/*      */   private uh av;
/*      */   private float aw;
/*      */   protected int bV;
/*      */   protected double bW;
/*      */   protected double bX;
/*      */   protected double bY;
/*      */   protected double bZ;
/*      */   protected double ca;
/*      */   float cb;
/*      */   protected int cc;
/*      */   protected int cd;
/*      */   protected float ce;
/*      */   protected float cf;
/*      */   protected float cg;
/*      */   protected boolean ch;
/*      */   protected float ci;
/*      */   protected float cj;
/*      */   private int ax;
/*      */   private nn ay;
/*      */   protected int ck;
/*  185 */   public int persistentId = this.U.nextInt(2147483647);
/*      */ 
/*      */   
/*      */   public acq(xd par1World) {
/*  189 */     super(par1World);
/*  190 */     this.ba = 20;
/*  191 */     this.bd = 0.0F;
/*  192 */     this.be = 0.0F;
/*  193 */     this.bf = 0.0F;
/*  194 */     this.bg = 0.0F;
/*  195 */     this.bl = true;
/*  196 */     this.bm = "/mob/char.png";
/*  197 */     this.bn = true;
/*  198 */     this.bo = 0.0F;
/*  199 */     this.bp = null;
/*  200 */     this.bq = 1.0F;
/*  201 */     this.br = 0;
/*  202 */     this.bs = 0.0F;
/*  203 */     this.bt = 0.1F;
/*  204 */     this.bu = 0.02F;
/*  205 */     this.bC = 0.0F;
/*  206 */     this.bD = 0;
/*  207 */     this.bE = 0;
/*  208 */     this.bH = false;
/*  209 */     this.bJ = -1;
/*  210 */     this.bK = (float)(Math.random() * 0.8999999761581421D + 0.10000000149011612D);
/*  211 */     this.bO = null;
/*  212 */     this.bP = 0;
/*  213 */     this.b = null;
/*  214 */     this.c = 0;
/*  215 */     this.d = null;
/*  216 */     this.bQ = 0;
/*  217 */     this.bR = 0;
/*  218 */     this.bS = new HashMap<Object, Object>();
/*  219 */     this.e = true;
/*  220 */     this.bT = new nb();
/*  221 */     this.bU = new nb();
/*  222 */     this.av = new uh(0, 0, 0);
/*  223 */     this.aw = -1.0F;
/*  224 */     this.cb = 0.0F;
/*  225 */     this.cc = 0;
/*  226 */     this.cd = 0;
/*  227 */     this.ch = false;
/*  228 */     this.ci = 0.0F;
/*  229 */     this.cj = 0.7F;
/*  230 */     this.ax = 0;
/*  231 */     this.ck = 0;
/*  232 */     this.bx = d();
/*  233 */     this.h = true;
/*  234 */     this.an = new vz(this);
/*  235 */     this.ao = new j(this);
/*  236 */     this.ap = new aej(this);
/*  237 */     this.aq = new vh(this);
/*  238 */     this.ar = new rh(this, par1World, 16.0F);
/*  239 */     this.at = new abk(this);
/*  240 */     this.bc = (float)(Math.random() + 1.0D) * 0.01F;
/*  241 */     d(this.o, this.p, this.q);
/*  242 */     this.bb = (float)Math.random() * 12398.0F;
/*  243 */     this.u = (float)(Math.random() * Math.PI * 2.0D);
/*  244 */     this.bf = this.u;
/*  245 */     this.R = 0.5F;
/*      */   }
/*      */ 
/*      */   
/*      */   public vz aJ() {
/*  250 */     return this.an;
/*      */   }
/*      */ 
/*      */   
/*      */   public j aK() {
/*  255 */     return this.ao;
/*      */   }
/*      */ 
/*      */   
/*      */   public aej aL() {
/*  260 */     return this.ap;
/*      */   }
/*      */ 
/*      */   
/*      */   public rh aM() {
/*  265 */     return this.ar;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public abk aN() {
/*  273 */     return this.at;
/*      */   }
/*      */ 
/*      */   
/*      */   public Random aO() {
/*  278 */     return this.U;
/*      */   }
/*      */ 
/*      */   
/*      */   public acq aP() {
/*  283 */     return this.b;
/*      */   }
/*      */ 
/*      */   
/*      */   public acq aQ() {
/*  288 */     return this.d;
/*      */   }
/*      */ 
/*      */   
/*      */   public void l(nn par1Entity) {
/*  293 */     if (par1Entity instanceof acq)
/*      */     {
/*  295 */       this.d = (acq)par1Entity;
/*      */     }
/*      */   }
/*      */ 
/*      */   
/*      */   public int aR() {
/*  301 */     return this.cd;
/*      */   }
/*      */ 
/*      */   
/*      */   public void f(float par1) {
/*  306 */     this.bf = par1;
/*      */   }
/*      */ 
/*      */   
/*      */   public float aS() {
/*  311 */     return this.au;
/*      */   }
/*      */ 
/*      */   
/*      */   public void g(float par1) {
/*  316 */     this.au = par1;
/*  317 */     h(par1);
/*      */   }
/*      */ 
/*      */   
/*      */   public boolean c(nn par1Entity) {
/*  322 */     l(par1Entity);
/*  323 */     return false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public acq aT() {
/*  331 */     return this.as;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void c(acq par1EntityLiving) {
/*  339 */     this.as = par1EntityLiving;
/*      */     
/*  341 */     if (Reflector.hasClass(9)) {
/*  342 */       Reflector.callVoid(90, new Object[] { this, par1EntityLiving });
/*      */     }
/*      */   }
/*      */   
/*      */   public boolean a(Class<yd> par1Class) {
/*  347 */     return (yd.class != par1Class && ui.class != par1Class);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void v() {}
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean aU() {
/*  363 */     return f(gk.c(this.o), gk.c(this.p), gk.c(this.q));
/*      */   }
/*      */ 
/*      */   
/*      */   public boolean f(int par1, int par2, int par3) {
/*  368 */     if (this.aw == -1.0F)
/*      */     {
/*  370 */       return true;
/*      */     }
/*      */     
/*  373 */     return (this.av.c(par1, par2, par3) < this.aw * this.aw);
/*      */   }
/*      */ 
/*      */ 
/*      */   
/*      */   public void b(int par1, int par2, int par3, int par4) {
/*  379 */     this.av.a(par1, par2, par3);
/*  380 */     this.aw = par4;
/*      */   }
/*      */ 
/*      */   
/*      */   public uh aV() {
/*  385 */     return this.av;
/*      */   }
/*      */ 
/*      */   
/*      */   public float aW() {
/*  390 */     return this.aw;
/*      */   }
/*      */ 
/*      */   
/*      */   public void aX() {
/*  395 */     this.aw = -1.0F;
/*      */   }
/*      */ 
/*      */   
/*      */   public boolean aY() {
/*  400 */     return (this.aw != -1.0F);
/*      */   }
/*      */ 
/*      */   
/*      */   public void a(acq par1EntityLiving) {
/*  405 */     this.b = par1EntityLiving;
/*  406 */     this.c = (this.b == null) ? 0 : 60;
/*      */     
/*  408 */     if (Reflector.hasClass(9)) {
/*  409 */       Reflector.callVoid(90, new Object[] { this, par1EntityLiving });
/*      */     }
/*      */   }
/*      */   
/*      */   protected void b() {
/*  414 */     this.ac.a(8, Integer.valueOf(this.am));
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean m(nn par1Entity) {
/*  422 */     return (this.k.a(bo.b(this.o, this.p + I(), this.q), bo.b(par1Entity.o, par1Entity.p + par1Entity.I(), par1Entity.q)) == null);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public String v_() {
/*  430 */     return this.bm;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean l_() {
/*  438 */     return !this.G;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean d_() {
/*  446 */     return !this.G;
/*      */   }
/*      */ 
/*      */   
/*      */   public float I() {
/*  451 */     return this.J * 0.85F;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public int j() {
/*  459 */     return 80;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void aZ() {
/*  467 */     String s = m();
/*  468 */     if (s != null)
/*      */     {
/*  470 */       this.k.a(this, s, C_(), af());
/*      */     }
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void B() {
/*  479 */     this.bv = this.bw;
/*  480 */     super.B();
/*  481 */     lv.a("mobBaseTick");
/*  482 */     if (M() && this.U.nextInt(1000) < this.a++) {
/*      */       
/*  484 */       this.a = -j();
/*  485 */       aZ();
/*      */     } 
/*  487 */     if (M() && N())
/*      */     {
/*  489 */       if (!a(md.e, 1));
/*      */     }
/*  491 */     if (F() || this.k.F)
/*      */     {
/*  493 */       D();
/*      */     }
/*  495 */     if (M() && a(acn.g) && !r_() && !this.bS.containsKey(Integer.valueOf(aad.o.H))) {
/*      */       
/*  497 */       g(b(Y()));
/*  498 */       if (Y() == -20) {
/*      */         
/*  500 */         g(0);
/*  501 */         for (int i = 0; i < 8; i++) {
/*      */           
/*  503 */           float f = this.U.nextFloat() - this.U.nextFloat();
/*  504 */           float f1 = this.U.nextFloat() - this.U.nextFloat();
/*  505 */           float f2 = this.U.nextFloat() - this.U.nextFloat();
/*  506 */           this.k.a("bubble", this.o + f, this.p + f1, this.q + f2, this.r, this.s, this.t);
/*      */         } 
/*      */         
/*  509 */         a(md.f, 2);
/*      */       } 
/*  511 */       D();
/*      */     } else {
/*      */       
/*  514 */       g(300);
/*      */     } 
/*  516 */     this.bF = this.bG;
/*  517 */     if (this.bE > 0)
/*      */     {
/*  519 */       this.bE--;
/*      */     }
/*  521 */     if (this.bA > 0)
/*      */     {
/*  523 */       this.bA--;
/*      */     }
/*  525 */     if (this.Y > 0)
/*      */     {
/*  527 */       this.Y--;
/*      */     }
/*  529 */     if (this.bx <= 0)
/*      */     {
/*  531 */       w_();
/*      */     }
/*  533 */     if (this.bP > 0) {
/*      */       
/*  535 */       this.bP--;
/*      */     } else {
/*      */       
/*  538 */       this.bO = null;
/*      */     } 
/*  540 */     if (this.d != null && !this.d.M())
/*      */     {
/*  542 */       this.d = null;
/*      */     }
/*  544 */     if (this.b != null)
/*      */     {
/*  546 */       if (!this.b.M()) {
/*      */         
/*  548 */         a((acq)null);
/*      */       }
/*  550 */       else if (this.c > 0) {
/*      */         
/*  552 */         this.c--;
/*      */       } else {
/*      */         
/*  555 */         a((acq)null);
/*      */       } 
/*      */     }
/*  558 */     be();
/*  559 */     this.bk = this.bj;
/*  560 */     this.be = this.bd;
/*  561 */     this.bg = this.bf;
/*  562 */     this.w = this.u;
/*  563 */     this.x = this.v;
/*  564 */     lv.b();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected void w_() {
/*  572 */     this.bD++;
/*  573 */     if (this.bD == 20) {
/*      */       
/*  575 */       if (!this.k.F && (this.bP > 0 || aH()) && !bi())
/*      */       {
/*  577 */         for (int k = b(this.bO); k > 0; ) {
/*      */           
/*  579 */           int m = bz.b(k);
/*  580 */           k -= m;
/*  581 */           this.k.a((nn)new bz(this.k, this.o, this.p, this.q, m));
/*      */         } 
/*      */       }
/*  584 */       bc();
/*  585 */       A();
/*  586 */       for (int i = 0; i < 20; i++) {
/*      */         
/*  588 */         double d = this.U.nextGaussian() * 0.02D;
/*  589 */         double d1 = this.U.nextGaussian() * 0.02D;
/*  590 */         double d2 = this.U.nextGaussian() * 0.02D;
/*  591 */         this.k.a("explode", this.o + (this.U.nextFloat() * this.I * 2.0F) - this.I, this.p + (this.U.nextFloat() * this.J), this.q + (this.U.nextFloat() * this.I * 2.0F) - this.I, d, d1, d2);
/*      */       } 
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected int b(int par1) {
/*  601 */     return par1 - 1;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected int b(yw par1EntityPlayer) {
/*  609 */     return this.bI;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected boolean aH() {
/*  617 */     return false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void ba() {
/*  625 */     for (int i = 0; i < 20; i++) {
/*      */       
/*  627 */       double d = this.U.nextGaussian() * 0.02D;
/*  628 */       double d1 = this.U.nextGaussian() * 0.02D;
/*  629 */       double d2 = this.U.nextGaussian() * 0.02D;
/*  630 */       double d3 = 10.0D;
/*  631 */       this.k.a("explode", this.o + (this.U.nextFloat() * this.I * 2.0F) - this.I - d * d3, this.p + (this.U.nextFloat() * this.J) - d1 * d3, this.q + (this.U.nextFloat() * this.I * 2.0F) - this.I - d2 * d3, d, d1, d2);
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void O() {
/*  640 */     super.O();
/*  641 */     this.bh = this.bi;
/*  642 */     this.bi = 0.0F;
/*  643 */     this.M = 0.0F;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(double par1, double par3, double par5, float par7, float par8, int par9) {
/*  652 */     this.H = 0.0F;
/*  653 */     this.bW = par1;
/*  654 */     this.bX = par3;
/*  655 */     this.bY = par5;
/*  656 */     this.bZ = par7;
/*  657 */     this.ca = par8;
/*  658 */     this.bV = par9;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void J_() {
/*  667 */     if (Reflector.hasClass(9)) {
/*      */       
/*  669 */       boolean ret = Reflector.callBoolean(91, new Object[] { this });
/*  670 */       if (ret) {
/*      */         return;
/*      */       }
/*      */     } 
/*  674 */     super.J_();
/*  675 */     if (this.bQ > 0) {
/*      */       
/*  677 */       if (this.bR <= 0)
/*      */       {
/*  679 */         this.bR = 60;
/*      */       }
/*  681 */       this.bR--;
/*  682 */       if (this.bR <= 0)
/*      */       {
/*  684 */         this.bQ--;
/*      */       }
/*      */     } 
/*  687 */     e();
/*  688 */     double d = this.o - this.l;
/*  689 */     double d1 = this.q - this.n;
/*  690 */     float f = gk.a(d * d + d1 * d1);
/*  691 */     float f1 = this.bd;
/*  692 */     float f2 = 0.0F;
/*  693 */     this.bh = this.bi;
/*  694 */     float f3 = 0.0F;
/*  695 */     if (f > 0.05F) {
/*      */       
/*  697 */       f3 = 1.0F;
/*  698 */       f2 = f * 3.0F;
/*  699 */       f1 = (float)Math.atan2(d1, d) * 180.0F / 3.1415927F - 90.0F;
/*      */     } 
/*  701 */     if (this.bw > 0.0F)
/*      */     {
/*  703 */       f1 = this.u;
/*      */     }
/*  705 */     if (!this.z)
/*      */     {
/*  707 */       f3 = 0.0F;
/*      */     }
/*  709 */     this.bi += (f3 - this.bi) * 0.3F;
/*  710 */     if (b_()) {
/*      */       
/*  712 */       this.aq.a();
/*      */     } else {
/*      */       float f4;
/*      */       
/*  716 */       for (f4 = f1 - this.bd; f4 < -180.0F; f4 += 360.0F);
/*  717 */       for (; f4 >= 180.0F; f4 -= 360.0F);
/*  718 */       this.bd += f4 * 0.3F;
/*      */       float f5;
/*  720 */       for (f5 = this.u - this.bd; f5 < -180.0F; f5 += 360.0F);
/*  721 */       for (; f5 >= 180.0F; f5 -= 360.0F);
/*  722 */       boolean flag = (f5 < -90.0F || f5 >= 90.0F);
/*  723 */       if (f5 < -75.0F)
/*      */       {
/*  725 */         f5 = -75.0F;
/*      */       }
/*  727 */       if (f5 >= 75.0F)
/*      */       {
/*  729 */         f5 = 75.0F;
/*      */       }
/*  731 */       this.bd = this.u - f5;
/*  732 */       if (f5 * f5 > 2500.0F)
/*      */       {
/*  734 */         this.bd += f5 * 0.2F;
/*      */       }
/*  736 */       if (flag)
/*      */       {
/*  738 */         f2 *= -1.0F;
/*      */       }
/*      */     } 
/*  741 */     for (; this.u - this.w < -180.0F; this.w -= 360.0F);
/*  742 */     for (; this.u - this.w >= 180.0F; this.w += 360.0F);
/*  743 */     for (; this.bd - this.be < -180.0F; this.be -= 360.0F);
/*  744 */     for (; this.bd - this.be >= 180.0F; this.be += 360.0F);
/*  745 */     for (; this.v - this.x < -180.0F; this.x -= 360.0F);
/*  746 */     for (; this.v - this.x >= 180.0F; this.x += 360.0F);
/*  747 */     for (; this.bf - this.bg < -180.0F; this.bg -= 360.0F);
/*  748 */     for (; this.bf - this.bg >= 180.0F; this.bg += 360.0F);
/*  749 */     this.bj += f2;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected void a(float par1, float par2) {
/*  757 */     super.a(par1, par2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void k(int par1) {
/*  765 */     if (this.bx <= 0) {
/*      */       return;
/*      */     }
/*      */     
/*  769 */     this.bx += par1;
/*  770 */     if (this.bx > d())
/*      */     {
/*  772 */       this.bx = d();
/*      */     }
/*  774 */     this.Y = this.ba / 2;
/*      */   }
/*      */ 
/*      */   
/*      */   public abstract int d();
/*      */   
/*      */   public int bb() {
/*  781 */     return this.bx;
/*      */   }
/*      */ 
/*      */   
/*      */   public void l(int par1) {
/*  786 */     this.bx = par1;
/*  787 */     if (par1 > d())
/*      */     {
/*  789 */       par1 = d();
/*      */     }
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean a(md par1DamageSource, int par2) {
/*  799 */     if (Reflector.hasClass(9)) {
/*      */       
/*  801 */       boolean ret = Reflector.callBoolean(92, new Object[] { this, par1DamageSource, Integer.valueOf(par2) });
/*  802 */       if (ret) {
/*  803 */         return false;
/*      */       }
/*      */     } 
/*  806 */     if (this.k.F)
/*      */     {
/*  808 */       return false;
/*      */     }
/*  810 */     this.cd = 0;
/*  811 */     if (this.bx <= 0)
/*      */     {
/*  813 */       return false;
/*      */     }
/*  815 */     if (par1DamageSource.k() && a(aad.n))
/*      */     {
/*  817 */       return false;
/*      */     }
/*  819 */     this.bM = 1.5F;
/*  820 */     boolean flag = true;
/*  821 */     if (this.Y > this.ba / 2.0F) {
/*      */       
/*  823 */       if (par2 <= this.cc)
/*      */       {
/*  825 */         return false;
/*      */       }
/*  827 */       c(par1DamageSource, par2 - this.cc);
/*  828 */       this.cc = par2;
/*  829 */       flag = false;
/*      */     } else {
/*      */       
/*  832 */       this.cc = par2;
/*  833 */       this.by = this.bx;
/*  834 */       this.Y = this.ba;
/*  835 */       c(par1DamageSource, par2);
/*  836 */       this.bA = this.bB = 10;
/*      */     } 
/*  838 */     this.bC = 0.0F;
/*  839 */     nn entity = par1DamageSource.a();
/*  840 */     if (entity != null) {
/*      */       
/*  842 */       if (entity instanceof acq)
/*      */       {
/*  844 */         a((acq)entity);
/*      */       }
/*  846 */       if (entity instanceof yw) {
/*      */         
/*  848 */         this.bP = 60;
/*  849 */         this.bO = (yw)entity;
/*      */       }
/*  851 */       else if (entity instanceof yo) {
/*      */         
/*  853 */         yo entitywolf = (yo)entity;
/*  854 */         if (entitywolf.G_()) {
/*      */           
/*  856 */           this.bP = 60;
/*  857 */           this.bO = null;
/*      */         } 
/*      */       } 
/*      */     } 
/*  861 */     if (flag) {
/*      */       
/*  863 */       this.k.a(this, (byte)2);
/*  864 */       K();
/*  865 */       if (entity != null) {
/*      */         
/*  867 */         double d = entity.o - this.o;
/*      */         double d1;
/*  869 */         for (d1 = entity.q - this.q; d * d + d1 * d1 < 1.0E-4D; d1 = (Math.random() - Math.random()) * 0.01D)
/*      */         {
/*  871 */           d = (Math.random() - Math.random()) * 0.01D;
/*      */         }
/*      */         
/*  874 */         this.bC = (float)(Math.atan2(d1, d) * 180.0D / Math.PI) - this.u;
/*  875 */         a(entity, par2, d, d1);
/*      */       } else {
/*      */         
/*  878 */         this.bC = ((int)(Math.random() * 2.0D) * 180);
/*      */       } 
/*      */     } 
/*  881 */     if (this.bx <= 0) {
/*      */       
/*  883 */       if (flag)
/*      */       {
/*  885 */         this.k.a(this, o(), C_(), af());
/*      */       }
/*  887 */       a(par1DamageSource);
/*      */     }
/*  889 */     else if (flag) {
/*      */       
/*  891 */       this.k.a(this, n(), C_(), af());
/*      */     } 
/*  893 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private float af() {
/*  901 */     if (bi())
/*      */     {
/*  903 */       return (this.U.nextFloat() - this.U.nextFloat()) * 0.2F + 1.5F;
/*      */     }
/*      */     
/*  906 */     return (this.U.nextFloat() - this.U.nextFloat()) * 0.2F + 1.0F;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void k() {
/*  915 */     this.bA = this.bB = 10;
/*  916 */     this.bC = 0.0F;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public int au() {
/*  924 */     return 0;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected void h(int i) {}
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected int d(md par1DamageSource, int par2) {
/*  936 */     if (!par1DamageSource.e()) {
/*      */       
/*  938 */       int i = 25 - au();
/*  939 */       int k = par2 * i + this.bz;
/*  940 */       h(par2);
/*  941 */       par2 = k / 25;
/*  942 */       this.bz = k % 25;
/*      */     } 
/*  944 */     return par2;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected int b(md par1DamageSource, int par2) {
/*  952 */     if (a(aad.m)) {
/*      */       
/*  954 */       int i = (b(aad.m).c() + 1) * 5;
/*  955 */       int m = 25 - i;
/*  956 */       int k = par2 * m + this.bz;
/*  957 */       par2 = k / 25;
/*  958 */       this.bz = k % 25;
/*      */     } 
/*  960 */     return par2;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected void c(md par1DamageSource, int par2) {
/*  970 */     if (Reflector.hasClass(9)) {
/*      */       
/*  972 */       par2 = Reflector.callInt(93, new Object[] { this, par1DamageSource, Integer.valueOf(par2) });
/*  973 */       if (par2 == 0) {
/*      */         return;
/*      */       }
/*      */     } 
/*  977 */     par2 = d(par1DamageSource, par2);
/*  978 */     par2 = b(par1DamageSource, par2);
/*  979 */     this.bx -= par2;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected float C_() {
/*  987 */     return 1.0F;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected String m() {
/*  995 */     return null;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected String n() {
/* 1003 */     return "damage.hurtflesh";
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected String o() {
/* 1011 */     return "damage.hurtflesh";
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(nn par1Entity, int par2, double par3, double par5) {
/* 1019 */     this.al = true;
/* 1020 */     float f = gk.a(par3 * par3 + par5 * par5);
/* 1021 */     float f1 = 0.4F;
/* 1022 */     this.r /= 2.0D;
/* 1023 */     this.s /= 2.0D;
/* 1024 */     this.t /= 2.0D;
/* 1025 */     this.r -= par3 / f * f1;
/* 1026 */     this.s += f1;
/* 1027 */     this.t -= par5 / f * f1;
/* 1028 */     if (this.s > 0.4000000059604645D)
/*      */     {
/* 1030 */       this.s = 0.4000000059604645D;
/*      */     }
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(md par1DamageSource) {
/* 1040 */     if (Reflector.hasClass(9)) {
/*      */       
/* 1042 */       boolean ret = Reflector.callBoolean(94, new Object[] { this, par1DamageSource });
/* 1043 */       if (ret) {
/*      */         return;
/*      */       }
/*      */     } 
/* 1047 */     nn entity = par1DamageSource.a();
/* 1048 */     if (this.br >= 0 && entity != null)
/*      */     {
/* 1050 */       entity.a(this, this.br);
/*      */     }
/* 1052 */     if (entity != null)
/*      */     {
/* 1054 */       entity.b(this);
/*      */     }
/* 1056 */     this.bH = true;
/* 1057 */     if (!this.k.F) {
/*      */       
/* 1059 */       int i = 0;
/* 1060 */       if (entity instanceof yw)
/*      */       {
/* 1062 */         i = ais.f(((yw)entity).ap);
/*      */       }
/*      */       
/* 1065 */       List listCapturedDrops = null;
/* 1066 */       if (Reflector.hasClass(9)) {
/*      */         
/* 1068 */         Reflector.setFieldValue(this, 100, Boolean.TRUE);
/* 1069 */         listCapturedDrops = (List)Reflector.getFieldValue(this, 101);
/* 1070 */         Reflector.callVoid(listCapturedDrops, 110, null);
/*      */       } 
/*      */       
/* 1073 */       int k = 0;
/*      */       
/* 1075 */       if (!bi()) {
/*      */         
/* 1077 */         a((this.bP > 0), i);
/* 1078 */         if (this.bP > 0) {
/*      */           
/* 1080 */           k = this.U.nextInt(200) - i;
/* 1081 */           if (k < 5)
/*      */           {
/* 1083 */             j_((k > 0) ? 0 : 1);
/*      */           }
/*      */         } 
/*      */       } 
/*      */       
/* 1088 */       if (Reflector.hasClass(9)) {
/*      */         
/* 1090 */         Reflector.setFieldValue(this, 100, Boolean.FALSE);
/* 1091 */         Reflector.callVoid(95, new Object[] { this, par1DamageSource, listCapturedDrops, Integer.valueOf(i), Boolean.valueOf((this.bP > 0)), Integer.valueOf(k) });
/*      */ 
/*      */         
/* 1094 */         for (Iterator<fq> iter = listCapturedDrops.iterator(); iter.hasNext(); ) {
/*      */           
/* 1096 */           fq item = iter.next();
/* 1097 */           this.k.a((nn)item);
/*      */         } 
/*      */       } 
/*      */     } 
/* 1101 */     this.k.a(this, (byte)3);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected void j_(int i) {}
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected void a(boolean par1, int par2) {
/* 1113 */     int i = f();
/* 1114 */     if (i > 0) {
/*      */       
/* 1116 */       int m = this.U.nextInt(3);
/* 1117 */       if (par2 > 0)
/*      */       {
/* 1119 */         m += this.U.nextInt(par2 + 1);
/*      */       }
/* 1121 */       for (int k = 0; k < m; k++)
/*      */       {
/* 1123 */         b(i, 1);
/*      */       }
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected int f() {
/* 1133 */     return 0;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected void e(float par1) {
/* 1142 */     if (Reflector.hasClass(9)) {
/*      */       
/* 1144 */       boolean ret = Reflector.callBoolean(96, new Object[] { this, Float.valueOf(par1) });
/* 1145 */       if (ret) {
/*      */         return;
/*      */       }
/*      */     } 
/* 1149 */     super.e(par1);
/* 1150 */     int i = (int)Math.ceil((par1 - 3.0F));
/* 1151 */     if (i > 0) {
/*      */       
/* 1153 */       if (i > 4) {
/*      */         
/* 1155 */         this.k.a(this, "damage.fallbig", 1.0F, 1.0F);
/*      */       } else {
/*      */         
/* 1158 */         this.k.a(this, "damage.fallsmall", 1.0F, 1.0F);
/*      */       } 
/* 1160 */       a(md.i, i);
/* 1161 */       int k = this.k.a(gk.c(this.o), gk.c(this.p - 0.20000000298023224D - this.H), gk.c(this.q));
/* 1162 */       if (k > 0) {
/*      */         
/* 1164 */         vj stepsound = (pb.m[k]).cb;
/* 1165 */         this.k.a(this, stepsound.d(), stepsound.b() * 0.5F, stepsound.c() * 0.75F);
/*      */       } 
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a_(float par1, float par2) {
/* 1175 */     if (H()) {
/*      */       
/* 1177 */       double d = this.p;
/* 1178 */       a(par1, par2, b_() ? 0.04F : 0.02F);
/* 1179 */       b(this.r, this.s, this.t);
/* 1180 */       this.r *= 0.800000011920929D;
/* 1181 */       this.s *= 0.800000011920929D;
/* 1182 */       this.t *= 0.800000011920929D;
/* 1183 */       this.s -= 0.02D;
/* 1184 */       if (this.A && e(this.r, this.s + 0.6000000238418579D - this.p + d, this.t))
/*      */       {
/* 1186 */         this.s = 0.30000001192092896D;
/*      */       }
/*      */     }
/* 1189 */     else if (J()) {
/*      */       
/* 1191 */       double d1 = this.p;
/* 1192 */       a(par1, par2, 0.02F);
/* 1193 */       b(this.r, this.s, this.t);
/* 1194 */       this.r *= 0.5D;
/* 1195 */       this.s *= 0.5D;
/* 1196 */       this.t *= 0.5D;
/* 1197 */       this.s -= 0.02D;
/* 1198 */       if (this.A && e(this.r, this.s + 0.6000000238418579D - this.p + d1, this.t))
/*      */       {
/* 1200 */         this.s = 0.30000001192092896D;
/*      */       }
/*      */     } else {
/*      */       
/* 1204 */       float f2, f = 0.91F;
/* 1205 */       if (this.z) {
/*      */         
/* 1207 */         f = 0.5460001F;
/* 1208 */         int i = this.k.a(gk.c(this.o), gk.c(this.y.b) - 1, gk.c(this.q));
/* 1209 */         if (i > 0)
/*      */         {
/* 1211 */           f = (pb.m[i]).ce * 0.91F;
/*      */         }
/*      */       } 
/* 1214 */       float f1 = 0.1627714F / f * f * f;
/*      */       
/* 1216 */       if (this.z) {
/*      */         
/* 1218 */         if (b_()) {
/*      */           
/* 1220 */           f2 = aS();
/*      */         } else {
/*      */           
/* 1223 */           f2 = this.bt;
/*      */         } 
/* 1225 */         f2 *= f1;
/*      */       } else {
/*      */         
/* 1228 */         f2 = this.bu;
/*      */       } 
/* 1230 */       a(par1, par2, f2);
/* 1231 */       f = 0.91F;
/* 1232 */       if (this.z) {
/*      */         
/* 1234 */         f = 0.5460001F;
/* 1235 */         int i = this.k.a(gk.c(this.o), gk.c(this.y.b) - 1, gk.c(this.q));
/* 1236 */         if (i > 0)
/*      */         {
/* 1238 */           f = (pb.m[i]).ce * 0.91F;
/*      */         }
/*      */       } 
/* 1241 */       if (p()) {
/*      */         
/* 1243 */         float f3 = 0.15F;
/* 1244 */         if (this.r < -f3)
/*      */         {
/* 1246 */           this.r = -f3;
/*      */         }
/* 1248 */         if (this.r > f3)
/*      */         {
/* 1250 */           this.r = f3;
/*      */         }
/* 1252 */         if (this.t < -f3)
/*      */         {
/* 1254 */           this.t = -f3;
/*      */         }
/* 1256 */         if (this.t > f3)
/*      */         {
/* 1258 */           this.t = f3;
/*      */         }
/* 1260 */         this.M = 0.0F;
/* 1261 */         if (this.s < -0.15D)
/*      */         {
/* 1263 */           this.s = -0.15D;
/*      */         }
/* 1265 */         boolean flag = (V() && this instanceof yw);
/* 1266 */         if (flag && this.s < 0.0D)
/*      */         {
/* 1268 */           this.s = 0.0D;
/*      */         }
/*      */       } 
/* 1271 */       b(this.r, this.s, this.t);
/* 1272 */       if (this.A && p())
/*      */       {
/* 1274 */         this.s = 0.2D;
/*      */       }
/* 1276 */       this.s -= 0.08D;
/* 1277 */       this.s *= 0.9800000190734863D;
/* 1278 */       this.r *= f;
/* 1279 */       this.t *= f;
/*      */     } 
/* 1281 */     this.bL = this.bM;
/* 1282 */     double d2 = this.o - this.l;
/* 1283 */     double d3 = this.q - this.n;
/* 1284 */     float f4 = gk.a(d2 * d2 + d3 * d3) * 4.0F;
/* 1285 */     if (f4 > 1.0F)
/*      */     {
/* 1287 */       f4 = 1.0F;
/*      */     }
/* 1289 */     this.bM += (f4 - this.bM) * 0.4F;
/* 1290 */     this.bN += this.bM;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean p() {
/* 1298 */     int i = gk.c(this.o);
/* 1299 */     int m = gk.c(this.y.b);
/* 1300 */     int k = gk.c(this.q);
/* 1301 */     int l = this.k.a(i, m, k);
/*      */     
/* 1303 */     if (Reflector.hasMethod(50)) {
/*      */       
/* 1305 */       pb block = pb.m[l];
/* 1306 */       if (block != null) {
/* 1307 */         return Reflector.callBoolean(block, 50, new Object[] { this.k, Integer.valueOf(i), Integer.valueOf(m), Integer.valueOf(k) });
/*      */       }
/*      */     } 
/* 1310 */     return (l == pb.aF.bO || l == pb.bu.bO);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void b(ady par1NBTTagCompound) {
/* 1318 */     par1NBTTagCompound.a("Health", (short)this.bx);
/* 1319 */     par1NBTTagCompound.a("HurtTime", (short)this.bA);
/* 1320 */     par1NBTTagCompound.a("DeathTime", (short)this.bD);
/* 1321 */     par1NBTTagCompound.a("AttackTime", (short)this.bE);
/* 1322 */     if (!this.bS.isEmpty()) {
/*      */       
/* 1324 */       no nbttaglist = new no();
/*      */       
/* 1326 */       for (Iterator<alg> iterator = this.bS.values().iterator(); iterator.hasNext(); nbttaglist.a((gh)nbttagcompound)) {
/*      */         
/* 1328 */         alg potioneffect = iterator.next();
/* 1329 */         ady nbttagcompound = new ady();
/* 1330 */         nbttagcompound.a("Id", (byte)potioneffect.a());
/* 1331 */         nbttagcompound.a("Amplifier", (byte)potioneffect.c());
/* 1332 */         nbttagcompound.a("Duration", potioneffect.b());
/*      */       } 
/*      */       
/* 1335 */       par1NBTTagCompound.a("ActiveEffects", (gh)nbttaglist);
/*      */     } 
/*      */     
/* 1338 */     par1NBTTagCompound.a("PersistentId", this.persistentId);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(ady par1NBTTagCompound) {
/* 1346 */     if (this.bx < -32768)
/*      */     {
/* 1348 */       this.bx = -32768;
/*      */     }
/* 1350 */     this.bx = par1NBTTagCompound.e("Health");
/* 1351 */     if (!par1NBTTagCompound.c("Health"))
/*      */     {
/* 1353 */       this.bx = d();
/*      */     }
/* 1355 */     this.bA = par1NBTTagCompound.e("HurtTime");
/* 1356 */     this.bD = par1NBTTagCompound.e("DeathTime");
/* 1357 */     this.bE = par1NBTTagCompound.e("AttackTime");
/* 1358 */     if (par1NBTTagCompound.c("ActiveEffects")) {
/*      */       
/* 1360 */       no nbttaglist = par1NBTTagCompound.n("ActiveEffects");
/* 1361 */       for (int i = 0; i < nbttaglist.d(); i++) {
/*      */         
/* 1363 */         ady nbttagcompound = (ady)nbttaglist.a(i);
/* 1364 */         byte byte0 = nbttagcompound.d("Id");
/* 1365 */         byte byte1 = nbttagcompound.d("Amplifier");
/* 1366 */         int k = nbttagcompound.f("Duration");
/* 1367 */         this.bS.put(Integer.valueOf(byte0), new alg(byte0, k, byte1));
/*      */       } 
/*      */     } 
/*      */     
/* 1371 */     this.persistentId = par1NBTTagCompound.f("PersistentId");
/* 1372 */     if (this.persistentId == 0) {
/* 1373 */       this.persistentId = this.U.nextInt(2147483647);
/*      */     }
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean M() {
/* 1381 */     return (!this.G && this.bx > 0);
/*      */   }
/*      */ 
/*      */   
/*      */   public boolean r_() {
/* 1386 */     return false;
/*      */   }
/*      */ 
/*      */   
/*      */   public void h(float par1) {
/* 1391 */     this.cf = par1;
/*      */   }
/*      */ 
/*      */   
/*      */   public void i(boolean par1) {
/* 1396 */     this.ch = par1;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void e() {
/* 1405 */     if (this.ax > 0)
/*      */     {
/* 1407 */       this.ax--;
/*      */     }
/* 1409 */     if (this.bV > 0) {
/*      */       
/* 1411 */       double d = this.o + (this.bW - this.o) / this.bV;
/* 1412 */       double d1 = this.p + (this.bX - this.p) / this.bV;
/* 1413 */       double d2 = this.q + (this.bY - this.q) / this.bV;
/*      */       double d3;
/* 1415 */       for (d3 = this.bZ - this.u; d3 < -180.0D; d3 += 360.0D);
/* 1416 */       for (; d3 >= 180.0D; d3 -= 360.0D);
/* 1417 */       this.u = (float)(this.u + d3 / this.bV);
/* 1418 */       this.v = (float)(this.v + (this.ca - this.v) / this.bV);
/* 1419 */       this.bV--;
/* 1420 */       d(d, d1, d2);
/* 1421 */       b(this.u, this.v);
/* 1422 */       List<wu> list1 = this.k.a(this, this.y.e(0.03125D, 0.0D, 0.03125D));
/* 1423 */       if (list1.size() > 0) {
/*      */         
/* 1425 */         double d4 = 0.0D;
/* 1426 */         for (int i = 0; i < list1.size(); i++) {
/*      */           
/* 1428 */           wu axisalignedbb = list1.get(i);
/* 1429 */           if (axisalignedbb.e > d4)
/*      */           {
/* 1431 */             d4 = axisalignedbb.e;
/*      */           }
/*      */         } 
/*      */         
/* 1435 */         d1 += d4 - this.y.b;
/* 1436 */         d(d, d1, d2);
/*      */       } 
/*      */     } 
/* 1439 */     lv.a("ai");
/* 1440 */     if (aq()) {
/*      */       
/* 1442 */       this.ch = false;
/* 1443 */       this.ce = 0.0F;
/* 1444 */       this.cf = 0.0F;
/* 1445 */       this.cg = 0.0F;
/*      */     }
/* 1447 */     else if (H_()) {
/*      */       
/* 1449 */       if (b_()) {
/*      */         
/* 1451 */         lv.a("newAi");
/* 1452 */         s_();
/* 1453 */         lv.b();
/*      */       } else {
/*      */         
/* 1456 */         lv.a("oldAi");
/* 1457 */         y_();
/* 1458 */         lv.b();
/* 1459 */         this.bf = this.u;
/*      */       } 
/*      */     } 
/* 1462 */     lv.b();
/* 1463 */     boolean flag = H();
/* 1464 */     boolean flag1 = J();
/* 1465 */     if (this.ch) {
/*      */       
/* 1467 */       if (flag) {
/*      */         
/* 1469 */         this.s += 0.03999999910593033D;
/*      */       }
/* 1471 */       else if (flag1) {
/*      */         
/* 1473 */         this.s += 0.03999999910593033D;
/*      */       }
/* 1475 */       else if (this.z && this.ax == 0) {
/*      */         
/* 1477 */         aD();
/* 1478 */         this.ax = 10;
/*      */       } 
/*      */     } else {
/*      */       
/* 1482 */       this.ax = 0;
/*      */     } 
/* 1484 */     this.ce *= 0.98F;
/* 1485 */     this.cf *= 0.98F;
/* 1486 */     this.cg *= 0.9F;
/* 1487 */     float f = this.bt;
/* 1488 */     this.bt *= at();
/* 1489 */     a_(this.ce, this.cf);
/* 1490 */     this.bt = f;
/* 1491 */     lv.a("push");
/* 1492 */     List<nn> list = this.k.b(this, this.y.b(0.20000000298023224D, 0.0D, 0.20000000298023224D));
/* 1493 */     if (list != null && list.size() > 0)
/*      */     {
/* 1495 */       for (int i = 0; i < list.size(); i++) {
/*      */         
/* 1497 */         nn entity = list.get(i);
/* 1498 */         if (entity.d_())
/*      */         {
/* 1500 */           entity.g(this);
/*      */         }
/*      */       } 
/*      */     }
/* 1504 */     lv.b();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected boolean b_() {
/* 1512 */     return false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected boolean H_() {
/* 1520 */     return !this.k.F;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected boolean aq() {
/* 1528 */     return (this.bx <= 0);
/*      */   }
/*      */ 
/*      */   
/*      */   public boolean ao() {
/* 1533 */     return false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected void aD() {
/* 1541 */     this.s = 0.41999998688697815D;
/* 1542 */     if (a(aad.j))
/*      */     {
/* 1544 */       this.s += ((b(aad.j).c() + 1) * 0.1F);
/*      */     }
/* 1546 */     if (W()) {
/*      */       
/* 1548 */       float f = this.u * 0.01745329F;
/* 1549 */       this.r -= (gk.a(f) * 0.2F);
/* 1550 */       this.t += (gk.b(f) * 0.2F);
/*      */     } 
/* 1552 */     this.al = true;
/*      */     
/* 1554 */     if (Reflector.hasClass(9)) {
/* 1555 */       Reflector.callVoid(97, new Object[] { this });
/*      */     }
/*      */   }
/*      */ 
/*      */ 
/*      */   
/*      */   protected boolean c_() {
/* 1562 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected void w() {
/* 1570 */     yw entityplayer = this.k.a(this, -1.0D);
/* 1571 */     if (entityplayer != null) {
/*      */       
/* 1573 */       double d = ((nn)entityplayer).o - this.o;
/* 1574 */       double d1 = ((nn)entityplayer).p - this.p;
/* 1575 */       double d2 = ((nn)entityplayer).q - this.q;
/* 1576 */       double d3 = d * d + d1 * d1 + d2 * d2;
/* 1577 */       if (c_() && d3 > 16384.0D)
/*      */       {
/* 1579 */         A();
/*      */       }
/* 1581 */       if (this.cd > 600 && this.U.nextInt(800) == 0 && d3 > 1024.0D && c_()) {
/*      */         
/* 1583 */         A();
/*      */       }
/* 1585 */       else if (d3 < 1024.0D) {
/*      */         
/* 1587 */         this.cd = 0;
/*      */       } 
/*      */     } 
/*      */   }
/*      */ 
/*      */   
/*      */   protected void s_() {
/* 1594 */     this.cd++;
/* 1595 */     lv.a("checkDespawn");
/* 1596 */     w();
/* 1597 */     lv.b();
/* 1598 */     lv.a("sensing");
/* 1599 */     this.at.a();
/* 1600 */     lv.b();
/* 1601 */     lv.a("targetSelector");
/* 1602 */     this.bU.a();
/* 1603 */     lv.b();
/* 1604 */     lv.a("goalSelector");
/* 1605 */     this.bT.a();
/* 1606 */     lv.b();
/* 1607 */     lv.a("navigation");
/* 1608 */     this.ar.d();
/* 1609 */     lv.b();
/* 1610 */     lv.a("mob tick");
/* 1611 */     g();
/* 1612 */     lv.b();
/* 1613 */     lv.a("controls");
/* 1614 */     this.ao.c();
/* 1615 */     this.an.a();
/* 1616 */     this.ap.b();
/* 1617 */     lv.b();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected void g() {}
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected void y_() {
/* 1629 */     this.cd++;
/* 1630 */     w();
/* 1631 */     this.ce = 0.0F;
/* 1632 */     this.cf = 0.0F;
/* 1633 */     float f = 8.0F;
/* 1634 */     if (this.U.nextFloat() < 0.02F) {
/*      */       
/* 1636 */       yw entityplayer = this.k.a(this, f);
/* 1637 */       if (entityplayer != null) {
/*      */         
/* 1639 */         this.ay = (nn)entityplayer;
/* 1640 */         this.ck = 10 + this.U.nextInt(20);
/*      */       } else {
/*      */         
/* 1643 */         this.cg = (this.U.nextFloat() - 0.5F) * 20.0F;
/*      */       } 
/*      */     } 
/* 1646 */     if (this.ay != null) {
/*      */       
/* 1648 */       a(this.ay, 10.0F, ak());
/* 1649 */       if (this.ck-- <= 0 || this.ay.G || this.ay.f(this) > (f * f))
/*      */       {
/* 1651 */         this.ay = null;
/*      */       }
/*      */     } else {
/*      */       
/* 1655 */       if (this.U.nextFloat() < 0.05F)
/*      */       {
/* 1657 */         this.cg = (this.U.nextFloat() - 0.5F) * 20.0F;
/*      */       }
/* 1659 */       this.u += this.cg;
/* 1660 */       this.v = this.ci;
/*      */     } 
/* 1662 */     boolean flag = H();
/* 1663 */     boolean flag1 = J();
/* 1664 */     if (flag || flag1)
/*      */     {
/* 1666 */       this.ch = (this.U.nextFloat() < 0.8F);
/*      */     }
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public int ak() {
/* 1676 */     return 40;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void a(nn par1Entity, float par2, float par3) {
/* 1684 */     double d1, d = par1Entity.o - this.o;
/* 1685 */     double d2 = par1Entity.q - this.q;
/*      */     
/* 1687 */     if (par1Entity instanceof acq) {
/*      */       
/* 1689 */       acq entityliving = (acq)par1Entity;
/* 1690 */       d1 = this.p + I() - entityliving.p + entityliving.I();
/*      */     } else {
/*      */       
/* 1693 */       d1 = (par1Entity.y.b + par1Entity.y.e) / 2.0D - this.p + I();
/*      */     } 
/* 1695 */     double d3 = gk.a(d * d + d2 * d2);
/* 1696 */     float f = (float)(Math.atan2(d2, d) * 180.0D / Math.PI) - 90.0F;
/* 1697 */     float f1 = (float)-(Math.atan2(d1, d3) * 180.0D / Math.PI);
/* 1698 */     this.v = -b(this.v, f1, par3);
/* 1699 */     this.u = b(this.u, f, par2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private float b(float par1, float par2, float par3) {
/*      */     float f;
/* 1708 */     for (f = par2 - par1; f < -180.0F; f += 360.0F);
/* 1709 */     for (; f >= 180.0F; f -= 360.0F);
/* 1710 */     if (f > par3)
/*      */     {
/* 1712 */       f = par3;
/*      */     }
/* 1714 */     if (f < -par3)
/*      */     {
/* 1716 */       f = -par3;
/*      */     }
/* 1718 */     return par1 + f;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void bc() {}
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean i() {
/* 1733 */     return (this.k.a(this.y) && this.k.a(this, this.y).size() == 0 && !this.k.b(this.y));
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected void E() {
/* 1741 */     a(md.j, 4);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public float i(float par1) {
/* 1749 */     float f = this.bw - this.bv;
/* 1750 */     if (f < 0.0F)
/*      */     {
/* 1752 */       f++;
/*      */     }
/* 1754 */     return this.bv + f * par1;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public bo j(float par1) {
/* 1762 */     if (par1 == 1.0F)
/*      */     {
/* 1764 */       return bo.b(this.o, this.p, this.q);
/*      */     }
/*      */     
/* 1767 */     double d = this.l + (this.o - this.l) * par1;
/* 1768 */     double d1 = this.m + (this.p - this.m) * par1;
/* 1769 */     double d2 = this.n + (this.q - this.n) * par1;
/* 1770 */     return bo.b(d, d1, d2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public bo Q() {
/* 1779 */     return k(1.0F);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public bo k(float par1) {
/* 1787 */     if (par1 == 1.0F) {
/*      */       
/* 1789 */       float f = gk.b(-this.u * 0.01745329F - 3.1415927F);
/* 1790 */       float f2 = gk.a(-this.u * 0.01745329F - 3.1415927F);
/* 1791 */       float f4 = -gk.b(-this.v * 0.01745329F);
/* 1792 */       float f6 = gk.a(-this.v * 0.01745329F);
/* 1793 */       return bo.b((f2 * f4), f6, (f * f4));
/*      */     } 
/*      */     
/* 1796 */     float f1 = this.x + (this.v - this.x) * par1;
/* 1797 */     float f3 = this.w + (this.u - this.w) * par1;
/* 1798 */     float f5 = gk.b(-f3 * 0.01745329F - 3.1415927F);
/* 1799 */     float f7 = gk.a(-f3 * 0.01745329F - 3.1415927F);
/* 1800 */     float f8 = -gk.b(-f1 * 0.01745329F);
/* 1801 */     float f9 = gk.a(-f1 * 0.01745329F);
/* 1802 */     return bo.b((f7 * f8), f9, (f5 * f8));
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public float bd() {
/* 1811 */     return 1.0F;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public pl a(double par1, float par3) {
/* 1819 */     bo vec3d = j(par3);
/* 1820 */     bo vec3d1 = k(par3);
/* 1821 */     bo vec3d2 = vec3d.c(vec3d1.a * par1, vec3d1.b * par1, vec3d1.c * par1);
/* 1822 */     return this.k.a(vec3d, vec3d2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public int ac() {
/* 1830 */     return 4;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public aan ae() {
/* 1838 */     return null;
/*      */   }
/*      */ 
/*      */   
/*      */   public void a(byte par1) {
/* 1843 */     if (par1 == 2) {
/*      */       
/* 1845 */       this.bM = 1.5F;
/* 1846 */       this.Y = this.ba;
/* 1847 */       this.bA = this.bB = 10;
/* 1848 */       this.bC = 0.0F;
/* 1849 */       this.k.a(this, n(), C_(), (this.U.nextFloat() - this.U.nextFloat()) * 0.2F + 1.0F);
/* 1850 */       a(md.k, 0);
/*      */     }
/* 1852 */     else if (par1 == 3) {
/*      */       
/* 1854 */       this.k.a(this, o(), C_(), (this.U.nextFloat() - this.U.nextFloat()) * 0.2F + 1.0F);
/* 1855 */       this.bx = 0;
/* 1856 */       a(md.k);
/*      */     } else {
/*      */       
/* 1859 */       super.a(par1);
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean az() {
/* 1868 */     return false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public int b(aan par1ItemStack, int par2) {
/* 1876 */     return par1ItemStack.b();
/*      */   }
/*      */ 
/*      */   
/*      */   protected void be() {
/* 1881 */     Iterator<Integer> iterator = this.bS.keySet().iterator();
/*      */ 
/*      */     
/* 1884 */     while (iterator.hasNext()) {
/*      */ 
/*      */ 
/*      */       
/* 1888 */       Integer integer = iterator.next();
/* 1889 */       alg potioneffect = (alg)this.bS.get(integer);
/* 1890 */       if (!potioneffect.a(this) && !this.k.F) {
/*      */         
/* 1892 */         iterator.remove();
/* 1893 */         e(potioneffect);
/*      */       } 
/*      */     } 
/* 1896 */     if (this.e) {
/*      */       
/* 1898 */       if (!this.k.F)
/*      */       {
/* 1900 */         if (!this.bS.isEmpty()) {
/*      */           
/* 1902 */           int i = hx.a(this.bS.values());
/* 1903 */           this.ac.b(8, Integer.valueOf(i));
/*      */         } else {
/*      */           
/* 1906 */           this.ac.b(8, Integer.valueOf(0));
/*      */         } 
/*      */       }
/* 1909 */       this.e = false;
/*      */     } 
/* 1911 */     if (this.U.nextBoolean()) {
/*      */       
/* 1913 */       int i = this.ac.c(8);
/* 1914 */       if (i > 0) {
/*      */         
/* 1916 */         double d = (i >> 16 & 0xFF) / 255.0D;
/* 1917 */         double d1 = (i >> 8 & 0xFF) / 255.0D;
/* 1918 */         double d2 = (i >> 0 & 0xFF) / 255.0D;
/* 1919 */         this.k.a("mobSpell", this.o + (this.U.nextDouble() - 0.5D) * this.I, this.p + this.U.nextDouble() * this.J - this.H, this.q + (this.U.nextDouble() - 0.5D) * this.I, d, d1, d2);
/*      */       } 
/*      */     } 
/*      */   }
/*      */ 
/*      */   
/*      */   public void bf() {
/* 1926 */     Iterator<Integer> iterator = this.bS.keySet().iterator();
/*      */ 
/*      */     
/* 1929 */     while (iterator.hasNext()) {
/*      */ 
/*      */ 
/*      */       
/* 1933 */       Integer integer = iterator.next();
/* 1934 */       alg potioneffect = (alg)this.bS.get(integer);
/* 1935 */       if (!this.k.F) {
/*      */         
/* 1937 */         iterator.remove();
/* 1938 */         e(potioneffect);
/*      */       } 
/*      */     } 
/*      */   }
/*      */ 
/*      */   
/*      */   public Collection bg() {
/* 1945 */     return this.bS.values();
/*      */   }
/*      */ 
/*      */   
/*      */   public boolean a(aad par1Potion) {
/* 1950 */     return this.bS.containsKey(Integer.valueOf(par1Potion.H));
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public alg b(aad par1Potion) {
/* 1958 */     return (alg)this.bS.get(Integer.valueOf(par1Potion.H));
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void b(alg par1PotionEffect) {
/* 1966 */     if (!a(par1PotionEffect)) {
/*      */       return;
/*      */     }
/*      */     
/* 1970 */     if (this.bS.containsKey(Integer.valueOf(par1PotionEffect.a()))) {
/*      */       
/* 1972 */       ((alg)this.bS.get(Integer.valueOf(par1PotionEffect.a()))).a(par1PotionEffect);
/* 1973 */       d((alg)this.bS.get(Integer.valueOf(par1PotionEffect.a())));
/*      */     } else {
/*      */       
/* 1976 */       this.bS.put(Integer.valueOf(par1PotionEffect.a()), par1PotionEffect);
/* 1977 */       c(par1PotionEffect);
/*      */     } 
/*      */   }
/*      */ 
/*      */   
/*      */   public boolean a(alg par1PotionEffect) {
/* 1983 */     if (s() == bk.b) {
/*      */       
/* 1985 */       int i = par1PotionEffect.a();
/* 1986 */       if (i == aad.l.H || i == aad.u.H)
/*      */       {
/* 1988 */         return false;
/*      */       }
/*      */     } 
/* 1991 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean bh() {
/* 1999 */     return (s() == bk.b);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void m(int par1) {
/* 2007 */     this.bS.remove(Integer.valueOf(par1));
/*      */   }
/*      */ 
/*      */   
/*      */   protected void c(alg par1PotionEffect) {
/* 2012 */     this.e = true;
/*      */   }
/*      */ 
/*      */   
/*      */   protected void d(alg par1PotionEffect) {
/* 2017 */     this.e = true;
/*      */   }
/*      */ 
/*      */   
/*      */   protected void e(alg par1PotionEffect) {
/* 2022 */     this.e = true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   protected float at() {
/* 2031 */     float f = 1.0F;
/* 2032 */     if (a(aad.c))
/*      */     {
/* 2034 */       f *= 1.0F + 0.2F * (b(aad.c).c() + 1);
/*      */     }
/* 2036 */     if (a(aad.d))
/*      */     {
/* 2038 */       f *= 1.0F - 0.15F * (b(aad.d).c() + 1);
/*      */     }
/* 2040 */     return f;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void j(double par1, double par3, double par5) {
/* 2048 */     c(par1, par3, par5, this.u, this.v);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public boolean bi() {
/* 2056 */     return false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public bk s() {
/* 2064 */     return bk.a;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void c(aan par1ItemStack) {
/* 2072 */     this.k.a(this, "random.break", 0.8F, 0.8F + this.k.r.nextFloat() * 0.4F);
/* 2073 */     for (int i = 0; i < 5; i++) {
/*      */       
/* 2075 */       bo vec3d = bo.b((this.U.nextFloat() - 0.5D) * 0.1D, Math.random() * 0.1D + 0.1D, 0.0D);
/* 2076 */       vec3d.a(-this.v * 3.1415927F / 180.0F);
/* 2077 */       vec3d.b(-this.u * 3.1415927F / 180.0F);
/* 2078 */       bo vec3d1 = bo.b((this.U.nextFloat() - 0.5D) * 0.3D, -this.U.nextFloat() * 0.6D - 0.3D, 0.6D);
/* 2079 */       vec3d1.a(-this.v * 3.1415927F / 180.0F);
/* 2080 */       vec3d1.b(-this.u * 3.1415927F / 180.0F);
/* 2081 */       vec3d1 = vec3d1.c(this.o, this.p + I(), this.q);
/* 2082 */       this.k.a("iconcrack_" + (par1ItemStack.a()).bQ, vec3d1.a, vec3d1.b, vec3d1.c, vec3d.a, vec3d.b + 0.05D, vec3d.c);
/*      */     } 
/*      */   }
/*      */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\acq.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */