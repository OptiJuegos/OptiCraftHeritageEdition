/*      */ import java.nio.IntBuffer;
/*      */ import java.util.ArrayList;
/*      */ import java.util.Arrays;
/*      */ import java.util.Comparator;
/*      */ import java.util.List;
/*      */ import java.util.Random;
/*      */ import net.minecraft.client.Minecraft;
/*      */ import org.lwjgl.BufferUtils;
/*      */ import org.lwjgl.input.Mouse;
/*      */ import org.lwjgl.opengl.ARBOcclusionQuery;
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
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ public class l
/*      */   implements afk
/*      */ {
/*      */   public List a;
/*      */   public xd i;
/*      */   public aaw j;
/*      */   private List k;
/*      */   private ct[] l;
/*      */   private ct[] m;
/*      */   private int n;
/*      */   private int o;
/*      */   private int p;
/*      */   private int q;
/*      */   public Minecraft r;
/*      */   public vl s;
/*      */   private IntBuffer t;
/*      */   private boolean u;
/*      */   private int v;
/*      */   private int w;
/*      */   private int x;
/*      */   private int y;
/*      */   private int z;
/*      */   private int A;
/*      */   private int B;
/*      */   private int C;
/*      */   private int D;
/*      */   private int E;
/*      */   private int F;
/*      */   private int G;
/*      */   private int H;
/*      */   private int I;
/*      */   private int J;
/*      */   int[] b;
/*      */   IntBuffer c;
/*      */   private int K;
/*      */   private int L;
/*      */   private int M;
/*      */   private int N;
/*      */   private int O;
/*      */   private int P;
/*      */   private int Q;
/*      */   private IntBuffer glListBuffer;
/*      */   double d;
/*      */   double e;
/*      */   double f;
/*      */   public float g;
/*      */   int h;
/*      */   double prevReposX;
/*      */   double prevReposY;
/*      */   double prevReposZ;
/*      */   private long lastMovedTime;
/*      */   
/*      */   private void f() {
/*      */     Random random = new Random(10842L);
/*      */     adz tessellator = adz.a;
/*      */     tessellator.b();
/*      */     for (int i = 0; i < 1500; i++) {
/*      */       double d = (random.nextFloat() * 2.0F - 1.0F);
/*      */       double d1 = (random.nextFloat() * 2.0F - 1.0F);
/*      */       double d2 = (random.nextFloat() * 2.0F - 1.0F);
/*      */       double d3 = (0.25F + random.nextFloat() * 0.25F);
/*      */       double d4 = d * d + d1 * d1 + d2 * d2;
/*      */       if (d4 < 1.0D && d4 > 0.01D) {
/*      */         d4 = 1.0D / Math.sqrt(d4);
/*      */         d *= d4;
/*      */         d1 *= d4;
/*      */         d2 *= d4;
/*      */         double d5 = d * 100.0D;
/*      */         double d6 = d1 * 100.0D;
/*      */         double d7 = d2 * 100.0D;
/*      */         double d8 = Math.atan2(d, d2);
/*      */         double d9 = Math.sin(d8);
/*      */         double d10 = Math.cos(d8);
/*      */         double d11 = Math.atan2(Math.sqrt(d * d + d2 * d2), d1);
/*      */         double d12 = Math.sin(d11);
/*      */         double d13 = Math.cos(d11);
/*      */         double d14 = random.nextDouble() * Math.PI * 2.0D;
/*      */         double d15 = Math.sin(d14);
/*      */         double d16 = Math.cos(d14);
/*      */         for (int j = 0; j < 4; j++) {
/*      */           double d17 = 0.0D;
/*      */           double d18 = ((j & 0x2) - 1) * d3;
/*      */           double d19 = ((j + 1 & 0x2) - 1) * d3;
/*      */           double d20 = d17;
/*      */           double d21 = d18 * d16 - d19 * d15;
/*      */           double d22 = d19 * d16 + d18 * d15;
/*      */           double d23 = d22;
/*      */           double d24 = d21 * d12 + d20 * d13;
/*      */           double d25 = d20 * d12 - d21 * d13;
/*      */           double d26 = d25 * d9 - d23 * d10;
/*      */           double d27 = d24;
/*      */           double d28 = d23 * d9 + d25 * d10;
/*      */           tessellator.a(d5 + d26, d6 + d27, d7 + d28);
/*      */         } 
/*      */       } 
/*      */     } 
/*      */     tessellator.a();
/*      */   }
/*      */   
/*      */   public void a(xd par1World) {
/*      */     if (this.i != null)
/*      */       this.i.b(this); 
/*      */     this.d = -9999.0D;
/*      */     this.e = -9999.0D;
/*      */     this.f = -9999.0D;
/*      */     ahu.a.a(par1World);
/*      */     this.i = par1World;
/*      */     this.s = new vl((ali)par1World);
/*      */     if (par1World != null) {
/*      */       par1World.a(this);
/*      */       a();
/*      */     } 
/*      */   }
/*      */   
/*      */   public void a() {
/*      */     if (this.i == null)
/*      */       return; 
/*      */     pb.K.b(Config.isTreesFancy());
/*      */     this.F = this.r.A.e;
/*      */     if (this.m != null)
/*      */       for (int m = 0; m < this.m.length; m++)
/*      */         this.m[m].c();  
/*      */     int numBlocks = 64 << 3 - this.F;
/*      */     int numBlocksFar = 512;
/*      */     numBlocks = 2 * this.r.A.ofRenderDistanceFine;
/*      */     if (Config.isLoadChunksFar() && numBlocks < numBlocksFar)
/*      */       numBlocks = numBlocksFar; 
/*      */     numBlocks += Config.getPreloadedChunks() * 2 * 16;
/*      */     int limit = 400;
/*      */     if (this.r.A.ofRenderDistanceFine > 256)
/*      */       limit = 1024; 
/*      */     if (numBlocks > limit)
/*      */       numBlocks = limit; 
/*      */     this.prevReposX = -9999.0D;
/*      */     this.prevReposY = -9999.0D;
/*      */     this.prevReposZ = -9999.0D;
/*      */     this.n = numBlocks / 16 + 1;
/*      */     this.o = 16;
/*      */     this.p = numBlocks / 16 + 1;
/*      */     this.m = new ct[this.n * this.o * this.p];
/*      */     this.l = new ct[this.n * this.o * this.p];
/*      */     int k = 0;
/*      */     int j = 0;
/*      */     this.z = 0;
/*      */     this.A = 0;
/*      */     this.B = 0;
/*      */     this.C = this.n;
/*      */     this.D = this.o;
/*      */     this.E = this.p;
/*      */     for (int i = 0; i < this.k.size(); i++) {
/*      */       ct wr = this.k.get(i);
/*      */       if (wr != null)
/*      */         wr.q = false; 
/*      */     } 
/*      */     this.k.clear();
/*      */     this.a.clear();
/*      */     for (int j1 = 0; j1 < this.n; j1++) {
/*      */       for (int k1 = 0; k1 < this.o; k1++) {
/*      */         for (int l1 = 0; l1 < this.p; l1++) {
/*      */           int wri = (l1 * this.o + k1) * this.n + j1;
/*      */           this.m[wri] = new ct(this.i, this.a, j1 * 16, k1 * 16, l1 * 16, this.q + k);
/*      */           if (this.u)
/*      */             (this.m[wri]).v = this.t.get(j); 
/*      */           (this.m[wri]).u = false;
/*      */           (this.m[wri]).t = true;
/*      */           (this.m[wri]).l = false;
/*      */           (this.m[wri]).s = j++;
/*      */           this.m[wri].f();
/*      */           this.l[wri] = this.m[wri];
/*      */           this.k.add(this.m[wri]);
/*      */           k += 3;
/*      */         } 
/*      */       } 
/*      */     } 
/*      */     if (this.i != null) {
/*      */       vq vq;
/*      */       acq entityliving = this.r.i;
/*      */       if (entityliving == null)
/*      */         vq = this.r.h; 
/*      */       if (vq != null) {
/*      */         c(gk.c(((nn)vq).o), gk.c(((nn)vq).p), gk.c(((nn)vq).q));
/*      */         Arrays.sort(this.l, (Comparator<? super ct>)new gw((nn)vq));
/*      */       } 
/*      */     } 
/*      */     this.G = 2;
/*      */   }
/*      */   
/*      */   public void a(bo par1Vec3D, amd par2ICamera, float par3) {
/*      */     if (this.G > 0) {
/*      */       this.G--;
/*      */       return;
/*      */     } 
/*      */     lv.a("prepare");
/*      */     ach.a.a(this.i, this.j, this.r.q, this.r.i, par3);
/*      */     ahu.a.a(this.i, this.j, this.r.q, this.r.i, this.r.A, par3);
/*      */     ach.a.a();
/*      */     this.H = 0;
/*      */     this.I = 0;
/*      */     this.J = 0;
/*      */     acq entityliving = this.r.i;
/*      */     ahu.b = entityliving.N + (entityliving.o - entityliving.N) * par3;
/*      */     ahu.c = entityliving.O + (entityliving.p - entityliving.O) * par3;
/*      */     ahu.d = entityliving.P + (entityliving.q - entityliving.P) * par3;
/*      */     ach.b = entityliving.N + (entityliving.o - entityliving.N) * par3;
/*      */     ach.c = entityliving.O + (entityliving.p - entityliving.O) * par3;
/*      */     ach.d = entityliving.P + (entityliving.q - entityliving.P) * par3;
/*      */     this.r.u.b(par3);
/*      */     lv.c("global");
/*      */     List<nn> list = this.i.s();
/*      */     this.H = list.size();
/*      */     for (int i = 0; i < this.i.e.size(); i++) {
/*      */       nn entity = this.i.e.get(i);
/*      */       this.I++;
/*      */       if (entity.a(par1Vec3D))
/*      */         ahu.a.a(entity, par3); 
/*      */     } 
/*      */     lv.c("entities");
/*      */     for (int j = 0; j < list.size(); j++) {
/*      */       nn entity1 = list.get(j);
/*      */       if (entity1.a(par1Vec3D) && (entity1.ak || par2ICamera.a(entity1.y)) && (entity1 != this.r.i || this.r.A.E != 0 || this.r.i.az()) && this.i.j(gk.c(entity1.o), 0, gk.c(entity1.q))) {
/*      */         this.I++;
/*      */         ahu.a.a(entity1, par3);
/*      */       } 
/*      */     } 
/*      */     lv.c("tileentities");
/*      */     tf.b();
/*      */     for (int k = 0; k < this.a.size(); k++)
/*      */       ach.a.a(this.a.get(k), par3); 
/*      */     this.r.u.a(par3);
/*      */     lv.b();
/*      */   }
/*      */   
/*      */   public String b() {
/*      */     return "C: " + this.N + "/" + this.K + ". F: " + this.L + ", O: " + this.M + ", E: " + this.O;
/*      */   }
/*      */   
/*      */   public String c() {
/*      */     return "E: " + this.I + "/" + this.H + ". B: " + this.J + ", I: " + (this.H - this.J - this.I) + ", " + Config.getVersion();
/*      */   }
/*      */   
/*      */   private void c(int x, int y, int z) {
/*      */     x -= 8;
/*      */     y -= 8;
/*      */     z -= 8;
/*      */     this.z = Integer.MAX_VALUE;
/*      */     this.A = Integer.MAX_VALUE;
/*      */     this.B = Integer.MAX_VALUE;
/*      */     this.C = Integer.MIN_VALUE;
/*      */     this.D = Integer.MIN_VALUE;
/*      */     this.E = Integer.MIN_VALUE;
/*      */     int blocksWide = this.n * 16;
/*      */     int blocksWide2 = blocksWide / 2;
/*      */     for (int ix = 0; ix < this.n; ix++) {
/*      */       int blockX = ix * 16;
/*      */       int blockXAbs = blockX + blocksWide2 - x;
/*      */       if (blockXAbs < 0)
/*      */         blockXAbs -= blocksWide - 1; 
/*      */       blockXAbs /= blocksWide;
/*      */       blockX -= blockXAbs * blocksWide;
/*      */       if (blockX < this.z)
/*      */         this.z = blockX; 
/*      */       if (blockX > this.C)
/*      */         this.C = blockX; 
/*      */       for (int iz = 0; iz < this.p; iz++) {
/*      */         int blockZ = iz * 16;
/*      */         int blockZAbs = blockZ + blocksWide2 - z;
/*      */         if (blockZAbs < 0)
/*      */           blockZAbs -= blocksWide - 1; 
/*      */         blockZAbs /= blocksWide;
/*      */         blockZ -= blockZAbs * blocksWide;
/*      */         if (blockZ < this.B)
/*      */           this.B = blockZ; 
/*      */         if (blockZ > this.E)
/*      */           this.E = blockZ; 
/*      */         for (int iy = 0; iy < this.o; iy++) {
/*      */           int blockY = iy * 16;
/*      */           if (blockY < this.A)
/*      */             this.A = blockY; 
/*      */           if (blockY > this.D)
/*      */             this.D = blockY; 
/*      */           ct worldrenderer = this.m[(iz * this.o + iy) * this.n + ix];
/*      */           boolean wasNeedingUpdate = worldrenderer.q;
/*      */           worldrenderer.a(blockX, blockY, blockZ);
/*      */           if (!wasNeedingUpdate && worldrenderer.q)
/*      */             this.k.add(worldrenderer); 
/*      */         } 
/*      */       } 
/*      */     } 
/*      */   }
/*      */   
/*      */   public int a(acq player, int renderPass, double partialTicks) {
/*      */     lv.a("sortchunks");
/*      */     if (this.k.size() < 10) {
/*      */       int numRend = 10;
/*      */       for (int i = 0; i < numRend; i++) {
/*      */         this.Q = (this.Q + 1) % this.m.length;
/*      */         ct worldrenderer = this.m[this.Q];
/*      */         if (worldrenderer.q && !this.k.contains(worldrenderer))
/*      */           this.k.add(worldrenderer); 
/*      */       } 
/*      */     } 
/*      */     if (this.r.A.e != this.F)
/*      */       if (!Config.isLoadChunksFar())
/*      */         a();  
/*      */     if (renderPass == 0) {
/*      */       this.K = 0;
/*      */       this.P = 0;
/*      */       this.L = 0;
/*      */       this.M = 0;
/*      */       this.N = 0;
/*      */       this.O = 0;
/*      */     } 
/*      */     double partialX = player.N + (player.o - player.N) * partialTicks;
/*      */     double partialY = player.O + (player.p - player.O) * partialTicks;
/*      */     double partialZ = player.P + (player.q - player.P) * partialTicks;
/*      */     double dSortX = player.o - this.d;
/*      */     double dSortY = player.p - this.e;
/*      */     double dSortZ = player.q - this.f;
/*      */     double distSqSort = dSortX * dSortX + dSortY * dSortY + dSortZ * dSortZ;
/*      */     if (distSqSort > 16.0D) {
/*      */       this.d = player.o;
/*      */       this.e = player.p;
/*      */       this.f = player.q;
/*      */       int preloadedBlocks = Config.getPreloadedChunks() * 16;
/*      */       double dReposX = player.o - this.prevReposX;
/*      */       double dReposY = player.p - this.prevReposY;
/*      */       double dReposZ = player.q - this.prevReposZ;
/*      */       double distSqRepos = dReposX * dReposX + dReposY * dReposY + dReposZ * dReposZ;
/*      */       if (distSqRepos > (preloadedBlocks * preloadedBlocks) + 16.0D) {
/*      */         this.prevReposX = player.o;
/*      */         this.prevReposY = player.p;
/*      */         this.prevReposZ = player.q;
/*      */         c(gk.c(player.o), gk.c(player.p), gk.c(player.q));
/*      */       } 
/*      */       Arrays.sort(this.l, (Comparator<? super ct>)new gw(player));
/*      */       int playerX = (int)player.o;
/*      */       int playerZ = (int)player.q;
/*      */       int step = 2000;
/*      */       if (Math.abs(playerX - ct.globalChunkOffsetX) > step || Math.abs(playerZ - ct.globalChunkOffsetZ) > step) {
/*      */         ct.globalChunkOffsetX = playerX;
/*      */         ct.globalChunkOffsetZ = playerZ;
/*      */         a();
/*      */       } 
/*      */     } 
/*      */     tf.a();
/*      */     if (this.r.A.ofSmoothFps && renderPass == 0)
/*      */       GL11.glFinish(); 
/*      */     int num = 0;
/*      */     int ocReq = 0;
/*      */     if (this.u && this.r.A.h && !this.r.A.g && renderPass == 0) {
/*      */       int firstIndex = 0;
/*      */       int lastIndex = 20;
/*      */       checkOcclusionQueryResult(firstIndex, lastIndex, player.o, player.p, player.q);
/*      */       for (int i = firstIndex; i < lastIndex; i++)
/*      */         (this.l[i]).t = true; 
/*      */       lv.c("render");
/*      */       num += a(firstIndex, lastIndex, renderPass, partialTicks);
/*      */       int endIndex = lastIndex;
/*      */       int stepNum = 0;
/*      */       int step = 30;
/*      */       int switchStep = this.n / 2;
/*      */       while (endIndex < this.l.length) {
/*      */         lv.c("occ");
/*      */         int startIndex = endIndex;
/*      */         if (stepNum < switchStep) {
/*      */           stepNum++;
/*      */         } else {
/*      */           stepNum--;
/*      */         } 
/*      */         endIndex = startIndex + stepNum * step;
/*      */         if (endIndex <= startIndex)
/*      */           endIndex = startIndex + 10; 
/*      */         if (endIndex > this.l.length)
/*      */           endIndex = this.l.length; 
/*      */         GL11.glDisable(3553);
/*      */         GL11.glDisable(2896);
/*      */         GL11.glDisable(3008);
/*      */         GL11.glDisable(2912);
/*      */         GL11.glColorMask(false, false, false, false);
/*      */         GL11.glDepthMask(false);
/*      */         lv.a("check");
/*      */         checkOcclusionQueryResult(startIndex, endIndex, player.o, player.p, player.q);
/*      */         lv.b();
/*      */         GL11.glPushMatrix();
/*      */         float sumTX = 0.0F;
/*      */         float sumTY = 0.0F;
/*      */         float sumTZ = 0.0F;
/*      */         int k = startIndex;
/*      */         while (true) {
/*      */           ct wr;
/*      */           if (k < endIndex) {
/*      */             wr = this.l[k];
/*      */             if (wr.e()) {
/*      */               wr.l = false;
/*      */               continue;
/*      */             } 
/*      */             if (!wr.l)
/*      */               continue; 
/*      */             if (Config.isOcclusionFancy())
/*      */               if (!wr.isInFrustrumFully) {
/*      */                 wr.t = true;
/*      */                 continue;
/*      */               }  
/*      */             if (!wr.l || wr.u)
/*      */               continue; 
/*      */             if (wr.isVisibleFromPosition) {
/*      */               float dx = Math.abs((float)(wr.visibleFromX - player.o));
/*      */               float dy = Math.abs((float)(wr.visibleFromY - player.p));
/*      */               float dz = Math.abs((float)(wr.visibleFromZ - player.q));
/*      */               float distVis = dx + dy + dz;
/*      */               if (distVis < 10.0D + k / 1000.0D) {
/*      */                 wr.t = true;
/*      */               } else {
/*      */                 wr.isVisibleFromPosition = false;
/*      */                 float bbX = (float)(wr.f - partialX);
/*      */                 float bbY = (float)(wr.g - partialY);
/*      */                 float bbZ = (float)(wr.h - partialZ);
/*      */                 float tX = bbX - sumTX;
/*      */                 float tY = bbY - sumTY;
/*      */                 float tZ = bbZ - sumTZ;
/*      */               } 
/*      */               continue;
/*      */             } 
/*      */           } else {
/*      */             break;
/*      */           } 
/*      */           float f1 = (float)(wr.f - partialX);
/*      */           float f2 = (float)(wr.g - partialY);
/*      */           float f3 = (float)(wr.h - partialZ);
/*      */           float f4 = f1 - sumTX;
/*      */           float f5 = f2 - sumTY;
/*      */           float f6 = f3 - sumTZ;
/*      */           k++;
/*      */         } 
/*      */         GL11.glPopMatrix();
/*      */         if (this.r.A.g) {
/*      */           if (lr.b == 0) {
/*      */             GL11.glColorMask(false, true, true, true);
/*      */           } else {
/*      */             GL11.glColorMask(true, false, false, true);
/*      */           } 
/*      */         } else {
/*      */           GL11.glColorMask(true, true, true, true);
/*      */         } 
/*      */         GL11.glDepthMask(true);
/*      */         GL11.glEnable(3553);
/*      */         GL11.glEnable(3008);
/*      */         GL11.glEnable(2912);
/*      */         lv.c("render");
/*      */         num += a(startIndex, endIndex, renderPass, partialTicks);
/*      */       } 
/*      */     } else {
/*      */       lv.c("render");
/*      */       num += a(0, this.l.length, renderPass, partialTicks);
/*      */     } 
/*      */     lv.b();
/*      */     return num;
/*      */   }
/*      */   
/*      */   private void checkOcclusionQueryResult(int startIndex, int endIndex, double px, double py, double pz) {
/*      */     for (int k = startIndex; k < endIndex; k++) {
/*      */       ct wr = this.l[k];
/*      */       if (wr.u) {
/*      */         this.c.clear();
/*      */         ARBOcclusionQuery.glGetQueryObjectuARB(wr.v, 34919, this.c);
/*      */         if (this.c.get(0) != 0) {
/*      */           wr.u = false;
/*      */           this.c.clear();
/*      */           ARBOcclusionQuery.glGetQueryObjectuARB(wr.v, 34918, this.c);
/*      */           boolean wasVisible = wr.t;
/*      */           wr.t = (this.c.get(0) > 0);
/*      */           if (wasVisible && wr.t) {
/*      */             wr.isVisibleFromPosition = true;
/*      */             wr.visibleFromX = px;
/*      */             wr.visibleFromY = py;
/*      */             wr.visibleFromZ = pz;
/*      */           } 
/*      */         } 
/*      */       } 
/*      */     } 
/*      */   }
/*      */   
/*      */   private int a(int startIndex, int endIndex, int renderPass, double partialTicks) {
/*      */     if (Config.isFogOff())
/*      */       GL11.glDisable(2912); 
/*      */     this.glListBuffer.clear();
/*      */     int j = 0;
/*      */     for (int i = startIndex; i < endIndex; i++) {
/*      */       ct wr = this.l[i];
/*      */       if (renderPass == 0) {
/*      */         this.K++;
/*      */         if (wr.m[renderPass]) {
/*      */           this.O++;
/*      */         } else if (!wr.l) {
/*      */           this.L++;
/*      */         } else if (this.u && !wr.t) {
/*      */           this.M++;
/*      */         } else {
/*      */           this.N++;
/*      */         } 
/*      */       } 
/*      */       if (!wr.m[renderPass] && wr.l && (!this.u || wr.t)) {
/*      */         int glCallList = wr.a(renderPass);
/*      */         if (glCallList >= 0) {
/*      */           this.glListBuffer.put(glCallList);
/*      */           j++;
/*      */         } 
/*      */       } 
/*      */     } 
/*      */     this.glListBuffer.flip();
/*      */     acq entityliving = this.r.i;
/*      */     double partialX = entityliving.N + (entityliving.o - entityliving.N) * partialTicks - ct.globalChunkOffsetX;
/*      */     double partialY = entityliving.O + (entityliving.p - entityliving.O) * partialTicks;
/*      */     double partialZ = entityliving.P + (entityliving.q - entityliving.P) * partialTicks - ct.globalChunkOffsetZ;
/*      */     this.r.u.b(partialTicks);
/*      */     GL11.glTranslatef((float)-partialX, (float)-partialY, (float)-partialZ);
/*      */     GL11.glCallLists(this.glListBuffer);
/*      */     GL11.glTranslatef((float)partialX, (float)partialY, (float)partialZ);
/*      */     this.r.u.a(partialTicks);
/*      */     return j;
/*      */   }
/*      */   
/*      */   public void a(int par1, double par2) {}
/*      */   
/*      */   public void d() {
/*      */     this.v++;
/*      */   }
/*      */   
/*      */   public void a(float par1) {
/*      */     if (this.r.f.t.g == 1) {
/*      */       if (!Config.isSkyEnabled())
/*      */         return; 
/*      */       GL11.glDisable(2912);
/*      */       GL11.glDisable(3008);
/*      */       GL11.glEnable(3042);
/*      */       GL11.glBlendFunc(770, 771);
/*      */       tf.a();
/*      */       GL11.glDepthMask(false);
/*      */       this.j.b(this.j.b("/misc/tunnel.png"));
/*      */       adz tessellator = adz.a;
/*      */       for (int i = 0; i < 6; i++) {
/*      */         GL11.glPushMatrix();
/*      */         if (i == 1)
/*      */           GL11.glRotatef(90.0F, 1.0F, 0.0F, 0.0F); 
/*      */         if (i == 2)
/*      */           GL11.glRotatef(-90.0F, 1.0F, 0.0F, 0.0F); 
/*      */         if (i == 3)
/*      */           GL11.glRotatef(180.0F, 1.0F, 0.0F, 0.0F); 
/*      */         if (i == 4)
/*      */           GL11.glRotatef(90.0F, 0.0F, 0.0F, 1.0F); 
/*      */         if (i == 5)
/*      */           GL11.glRotatef(-90.0F, 0.0F, 0.0F, 1.0F); 
/*      */         tessellator.b();
/*      */         tessellator.c(1579032);
/*      */         tessellator.a(-100.0D, -100.0D, -100.0D, 0.0D, 0.0D);
/*      */         tessellator.a(-100.0D, -100.0D, 100.0D, 0.0D, 16.0D);
/*      */         tessellator.a(100.0D, -100.0D, 100.0D, 16.0D, 16.0D);
/*      */         tessellator.a(100.0D, -100.0D, -100.0D, 16.0D, 0.0D);
/*      */         tessellator.a();
/*      */         GL11.glPopMatrix();
/*      */       } 
/*      */       GL11.glDepthMask(true);
/*      */       GL11.glEnable(3553);
/*      */       GL11.glEnable(3008);
/*      */       return;
/*      */     } 
/*      */     if (!this.r.f.t.e())
/*      */       return; 
/*      */     GL11.glDisable(3553);
/*      */     bo vec3d = this.i.a(this.r.i, par1);
/*      */     vec3d = CustomColorizer.getSkyColor(vec3d, (ali)this.r.f, this.r.i.o, this.r.i.p + 1.0D, this.r.i.q);
/*      */     float f = (float)vec3d.a;
/*      */     float f1 = (float)vec3d.b;
/*      */     float f2 = (float)vec3d.c;
/*      */     if (this.r.A.g) {
/*      */       float f3 = (f * 30.0F + f1 * 59.0F + f2 * 11.0F) / 100.0F;
/*      */       float f4 = (f * 30.0F + f1 * 70.0F) / 100.0F;
/*      */       float f5 = (f * 30.0F + f2 * 70.0F) / 100.0F;
/*      */       f = f3;
/*      */       f1 = f4;
/*      */       f2 = f5;
/*      */     } 
/*      */     GL11.glColor3f(f, f1, f2);
/*      */     adz tessellator1 = adz.a;
/*      */     GL11.glDepthMask(false);
/*      */     GL11.glEnable(2912);
/*      */     GL11.glColor3f(f, f1, f2);
/*      */     if (Config.isSkyEnabled())
/*      */       GL11.glCallList(this.x); 
/*      */     GL11.glDisable(2912);
/*      */     GL11.glDisable(3008);
/*      */     GL11.glEnable(3042);
/*      */     GL11.glBlendFunc(770, 771);
/*      */     tf.a();
/*      */     float[] af = this.i.t.a(this.i.c(par1), par1);
/*      */     if (af != null && Config.isSunMoonEnabled()) {
/*      */       GL11.glDisable(3553);
/*      */       GL11.glShadeModel(7425);
/*      */       GL11.glPushMatrix();
/*      */       GL11.glRotatef(90.0F, 1.0F, 0.0F, 0.0F);
/*      */       GL11.glRotatef((gk.a(this.i.e(par1)) >= 0.0F) ? 0.0F : 180.0F, 0.0F, 0.0F, 1.0F);
/*      */       GL11.glRotatef(90.0F, 0.0F, 0.0F, 1.0F);
/*      */       float f6 = af[0];
/*      */       float f8 = af[1];
/*      */       float f11 = af[2];
/*      */       if (this.r.A.g) {
/*      */         float f14 = (f6 * 30.0F + f8 * 59.0F + f11 * 11.0F) / 100.0F;
/*      */         float f17 = (f6 * 30.0F + f8 * 70.0F) / 100.0F;
/*      */         float f20 = (f6 * 30.0F + f11 * 70.0F) / 100.0F;
/*      */         f6 = f14;
/*      */         f8 = f17;
/*      */         f11 = f20;
/*      */       } 
/*      */       tessellator1.a(6);
/*      */       tessellator1.a(f6, f8, f11, af[3]);
/*      */       tessellator1.a(0.0D, 100.0D, 0.0D);
/*      */       int j = 16;
/*      */       tessellator1.a(af[0], af[1], af[2], 0.0F);
/*      */       for (int k = 0; k <= j; k++) {
/*      */         float f21 = k * 3.1415927F * 2.0F / j;
/*      */         float f22 = gk.a(f21);
/*      */         float f23 = gk.b(f21);
/*      */         tessellator1.a((f22 * 120.0F), (f23 * 120.0F), (-f23 * 40.0F * af[3]));
/*      */       } 
/*      */       tessellator1.a();
/*      */       GL11.glPopMatrix();
/*      */       GL11.glShadeModel(7424);
/*      */     } 
/*      */     GL11.glEnable(3553);
/*      */     GL11.glBlendFunc(770, 1);
/*      */     GL11.glPushMatrix();
/*      */     double d = (1.0F - this.i.j(par1));
/*      */     float f7 = 0.0F;
/*      */     float f9 = 0.0F;
/*      */     float f12 = 0.0F;
/*      */     GL11.glColor4f(1.0F, 1.0F, 1.0F, (float)d);
/*      */     GL11.glTranslatef(f7, f9, f12);
/*      */     GL11.glRotatef(-90.0F, 0.0F, 1.0F, 0.0F);
/*      */     GL11.glRotatef(this.i.c(par1) * 360.0F, 1.0F, 0.0F, 0.0F);
/*      */     if (Config.isSunMoonEnabled()) {
/*      */       float f15 = 30.0F;
/*      */       GL11.glBindTexture(3553, this.j.b("/terrain/sun.png"));
/*      */       tessellator1.b();
/*      */       tessellator1.a(-f15, 100.0D, -f15, 0.0D, 0.0D);
/*      */       tessellator1.a(f15, 100.0D, -f15, 1.0D, 0.0D);
/*      */       tessellator1.a(f15, 100.0D, f15, 1.0D, 1.0D);
/*      */       tessellator1.a(-f15, 100.0D, f15, 0.0D, 1.0D);
/*      */       tessellator1.a();
/*      */       f15 = 20.0F;
/*      */       GL11.glBindTexture(3553, this.j.b("/terrain/moon_phases.png"));
/*      */       int i18 = this.i.d(par1);
/*      */       int i = i18 % 4;
/*      */       int i1 = i18 / 4 % 2;
/*      */       float f24 = (i + 0) / 4.0F;
/*      */       float f25 = (i1 + 0) / 2.0F;
/*      */       float f26 = (i + 1) / 4.0F;
/*      */       float f27 = (i1 + 1) / 2.0F;
/*      */       tessellator1.b();
/*      */       tessellator1.a(-f15, -100.0D, f15, f26, f27);
/*      */       tessellator1.a(f15, -100.0D, f15, f24, f27);
/*      */       tessellator1.a(f15, -100.0D, -f15, f24, f25);
/*      */       tessellator1.a(-f15, -100.0D, -f15, f26, f25);
/*      */       tessellator1.a();
/*      */     } 
/*      */     GL11.glDisable(3553);
/*      */     float f18 = (float)(this.i.h(par1) * d);
/*      */     if (f18 > 0.0F && Config.isStarsEnabled()) {
/*      */       GL11.glColor4f(f18, f18, f18, f18);
/*      */       GL11.glCallList(this.w);
/*      */     } 
/*      */     GL11.glColor4f(1.0F, 1.0F, 1.0F, 1.0F);
/*      */     GL11.glDisable(3042);
/*      */     GL11.glEnable(3008);
/*      */     GL11.glEnable(2912);
/*      */     GL11.glPopMatrix();
/*      */     GL11.glDisable(3553);
/*      */     GL11.glColor3f(0.0F, 0.0F, 0.0F);
/*      */     d = (this.r.h.j(par1)).b - this.i.I();
/*      */     if (d < 0.0D && Config.isSkyEnabled()) {
/*      */       GL11.glPushMatrix();
/*      */       GL11.glTranslatef(0.0F, 12.0F, 0.0F);
/*      */       GL11.glCallList(this.y);
/*      */       GL11.glPopMatrix();
/*      */       float f10 = 1.0F;
/*      */       float f13 = -((float)(d + 65.0D));
/*      */       float f16 = -f10;
/*      */       float f19 = f13;
/*      */       tessellator1.b();
/*      */       tessellator1.a(0, 255);
/*      */       tessellator1.a(-f10, f19, f10);
/*      */       tessellator1.a(f10, f19, f10);
/*      */       tessellator1.a(f10, f16, f10);
/*      */       tessellator1.a(-f10, f16, f10);
/*      */       tessellator1.a(-f10, f16, -f10);
/*      */       tessellator1.a(f10, f16, -f10);
/*      */       tessellator1.a(f10, f19, -f10);
/*      */       tessellator1.a(-f10, f19, -f10);
/*      */       tessellator1.a(f10, f16, -f10);
/*      */       tessellator1.a(f10, f16, f10);
/*      */       tessellator1.a(f10, f19, f10);
/*      */       tessellator1.a(f10, f19, -f10);
/*      */       tessellator1.a(-f10, f19, -f10);
/*      */       tessellator1.a(-f10, f19, f10);
/*      */       tessellator1.a(-f10, f16, f10);
/*      */       tessellator1.a(-f10, f16, -f10);
/*      */       tessellator1.a(-f10, f16, -f10);
/*      */       tessellator1.a(-f10, f16, f10);
/*      */       tessellator1.a(f10, f16, f10);
/*      */       tessellator1.a(f10, f16, -f10);
/*      */       tessellator1.a();
/*      */     } 
/*      */     if (this.i.t.c()) {
/*      */       GL11.glColor3f(f * 0.2F + 0.04F, f1 * 0.2F + 0.04F, f2 * 0.6F + 0.1F);
/*      */     } else {
/*      */       GL11.glColor3f(f, f1, f2);
/*      */     } 
/*      */     if (this.r.A.ofRenderDistanceFine <= 64)
/*      */       GL11.glColor3f(this.r.u.n, this.r.u.o, this.r.u.p); 
/*      */     GL11.glPushMatrix();
/*      */     GL11.glTranslatef(0.0F, -((float)(d - 16.0D)), 0.0F);
/*      */     if (Config.isSkyEnabled())
/*      */       GL11.glCallList(this.y); 
/*      */     GL11.glPopMatrix();
/*      */     GL11.glEnable(3553);
/*      */     GL11.glDepthMask(true);
/*      */   }
/*      */   
/*      */   public void b(float par1) {
/*      */     if (!this.r.f.t.e())
/*      */       return; 
/*      */     if (this.r.A.ofClouds == 3)
/*      */       return; 
/*      */     if (Config.isCloudsFancy()) {
/*      */       c(par1);
/*      */       return;
/*      */     } 
/*      */     GL11.glDisable(2884);
/*      */     float f = (float)(this.r.i.O + (this.r.i.p - this.r.i.O) * par1);
/*      */     byte byte0 = 32;
/*      */     int i = 256 / byte0;
/*      */     adz tessellator = adz.a;
/*      */     GL11.glBindTexture(3553, this.j.b("/environment/clouds.png"));
/*      */     GL11.glEnable(3042);
/*      */     GL11.glBlendFunc(770, 771);
/*      */     bo vec3d = this.i.f(par1);
/*      */     float f1 = (float)vec3d.a;
/*      */     float f2 = (float)vec3d.b;
/*      */     float f3 = (float)vec3d.c;
/*      */     if (this.r.A.g) {
/*      */       float f4 = (f1 * 30.0F + f2 * 59.0F + f3 * 11.0F) / 100.0F;
/*      */       float f6 = (f1 * 30.0F + f2 * 70.0F) / 100.0F;
/*      */       float f7 = (f1 * 30.0F + f3 * 70.0F) / 100.0F;
/*      */       f1 = f4;
/*      */       f2 = f6;
/*      */       f3 = f7;
/*      */     } 
/*      */     float f5 = 4.882813E-4F;
/*      */     double d = (this.v + par1);
/*      */     double d1 = this.r.i.l + (this.r.i.o - this.r.i.l) * par1 + d * 0.029999999329447746D;
/*      */     double d2 = this.r.i.n + (this.r.i.q - this.r.i.n) * par1;
/*      */     int j = gk.c(d1 / 2048.0D);
/*      */     int k = gk.c(d2 / 2048.0D);
/*      */     d1 -= (j * 2048);
/*      */     d2 -= (k * 2048);
/*      */     float f8 = this.i.t.f() - f + 0.33F;
/*      */     f8 += this.r.A.ofCloudsHeight * 128.0F;
/*      */     float f9 = (float)(d1 * f5);
/*      */     float f10 = (float)(d2 * f5);
/*      */     tessellator.b();
/*      */     tessellator.a(f1, f2, f3, 0.8F);
/*      */     int m;
/*      */     for (m = -byte0 * i; m < byte0 * i; m += byte0) {
/*      */       int i1;
/*      */       for (i1 = -byte0 * i; i1 < byte0 * i; i1 += byte0) {
/*      */         tessellator.a((m + 0), f8, (i1 + byte0), ((m + 0) * f5 + f9), ((i1 + byte0) * f5 + f10));
/*      */         tessellator.a((m + byte0), f8, (i1 + byte0), ((m + byte0) * f5 + f9), ((i1 + byte0) * f5 + f10));
/*      */         tessellator.a((m + byte0), f8, (i1 + 0), ((m + byte0) * f5 + f9), ((i1 + 0) * f5 + f10));
/*      */         tessellator.a((m + 0), f8, (i1 + 0), ((m + 0) * f5 + f9), ((i1 + 0) * f5 + f10));
/*      */       } 
/*      */     } 
/*      */     tessellator.a();
/*      */     GL11.glColor4f(1.0F, 1.0F, 1.0F, 1.0F);
/*      */     GL11.glDisable(3042);
/*      */     GL11.glEnable(2884);
/*      */   }
/*      */   
/*      */   public boolean a(double par1, double par3, double d, float f) {
/*      */     return false;
/*      */   }
/*      */   
/*      */   public l(Minecraft par1Minecraft, aaw par2RenderEngine) {
/* 2282 */     this.lastMovedTime = System.currentTimeMillis(); this.a = new ArrayList(); this.k = new ArrayList(); this.u = false; this.v = 0; this.F = -1; this.G = 2; this.b = new int[50000]; this.c = ew.d(64); this.glListBuffer = BufferUtils.createIntBuffer(65536); this.d = -9999.0D; this.e = -9999.0D; this.f = -9999.0D; this.h = 0; this.r = par1Minecraft; this.j = par2RenderEngine; byte maxChunkDim = 65; byte maxChunkHeight = 16; this.q = ew.a(maxChunkDim * maxChunkDim * maxChunkHeight * 3); this.u = vm.a(); if (this.u) { this.c.clear(); this.t = ew.d(maxChunkDim * maxChunkDim * maxChunkHeight); this.t.clear(); this.t.position(0); this.t.limit(maxChunkDim * maxChunkDim * maxChunkHeight); ARBOcclusionQuery.glGenQueriesARB(this.t); }  this.w = ew.a(3); GL11.glPushMatrix(); GL11.glNewList(this.w, 4864); f(); GL11.glEndList(); GL11.glPopMatrix(); adz tessellator = adz.a; this.x = this.w + 1; GL11.glNewList(this.x, 4864); byte byte2 = 64; int i = 256 / byte2 + 2; float f = 16.0F; int j; for (j = -byte2 * i; j <= byte2 * i; j += byte2) { int m; for (m = -byte2 * i; m <= byte2 * i; m += byte2) { tessellator.b(); tessellator.a((j + 0), f, (m + 0)); tessellator.a((j + byte2), f, (m + 0)); tessellator.a((j + byte2), f, (m + byte2)); tessellator.a((j + 0), f, (m + byte2)); tessellator.a(); }  }  GL11.glEndList(); this.y = this.w + 2; GL11.glNewList(this.y, 4864); f = -16.0F; tessellator.b(); int k; for (k = -byte2 * i; k <= byte2 * i; k += byte2) { int i1; for (i1 = -byte2 * i; i1 <= byte2 * i; i1 += byte2) { tessellator.a((k + byte2), f, (i1 + 0)); tessellator.a((k + 0), f, (i1 + 0)); tessellator.a((k + 0), f, (i1 + byte2)); tessellator.a((k + byte2), f, (i1 + byte2)); }  }  tessellator.a(); GL11.glEndList(); this.j.a();
/*      */   } public void c(float par1) { GL11.glDisable(2884); float f = (float)(this.r.i.O + (this.r.i.p - this.r.i.O) * par1); adz tessellator = adz.a; float f1 = 12.0F; float f2 = 4.0F; double d = (this.v + par1); double d1 = (this.r.i.l + (this.r.i.o - this.r.i.l) * par1 + d * 0.029999999329447746D) / f1; double d2 = (this.r.i.n + (this.r.i.q - this.r.i.n) * par1) / f1 + 0.33000001311302185D; float f3 = this.i.t.f() - f + 0.33F; f3 += this.r.A.ofCloudsHeight * 128.0F; int i = gk.c(d1 / 2048.0D); int j = gk.c(d2 / 2048.0D); d1 -= (i * 2048); d2 -= (j * 2048); GL11.glBindTexture(3553, this.j.b("/environment/clouds.png")); GL11.glEnable(3042); GL11.glBlendFunc(770, 771); bo vec3d = this.i.f(par1); float f4 = (float)vec3d.a; float f5 = (float)vec3d.b; float f6 = (float)vec3d.c; if (this.r.A.g) { float f7 = (f4 * 30.0F + f5 * 59.0F + f6 * 11.0F) / 100.0F; float f9 = (f4 * 30.0F + f5 * 70.0F) / 100.0F; float f11 = (f4 * 30.0F + f6 * 70.0F) / 100.0F; f4 = f7; f5 = f9; f6 = f11; }  float f8 = (float)(d1 * 0.0D); float f10 = (float)(d2 * 0.0D); float f12 = 0.00390625F; f8 = gk.c(d1) * f12; f10 = gk.c(d2) * f12; float f13 = (float)(d1 - gk.c(d1)); float f14 = (float)(d2 - gk.c(d2)); int k = 8; byte byte0 = 4; float f15 = 9.765625E-4F; GL11.glScalef(f1, 1.0F, f1); for (int m = 0; m < 2; m++) { if (m == 0) { GL11.glColorMask(false, false, false, false); } else if (this.r.A.g) { if (lr.b == 0) { GL11.glColorMask(false, true, true, true); } else { GL11.glColorMask(true, false, false, true); }  } else { GL11.glColorMask(true, true, true, true); }  for (int i1 = -byte0 + 1; i1 <= byte0; i1++) { for (int j1 = -byte0 + 1; j1 <= byte0; j1++) { tessellator.b(); float f16 = (i1 * k); float f17 = (j1 * k); float f18 = f16 - f13; float f19 = f17 - f14; if (f3 > -f2 - 1.0F) { tessellator.a(f4 * 0.7F, f5 * 0.7F, f6 * 0.7F, 0.8F); tessellator.b(0.0F, -1.0F, 0.0F); tessellator.a((f18 + 0.0F), (f3 + 0.0F), (f19 + k), ((f16 + 0.0F) * f12 + f8), ((f17 + k) * f12 + f10)); tessellator.a((f18 + k), (f3 + 0.0F), (f19 + k), ((f16 + k) * f12 + f8), ((f17 + k) * f12 + f10)); tessellator.a((f18 + k), (f3 + 0.0F), (f19 + 0.0F), ((f16 + k) * f12 + f8), ((f17 + 0.0F) * f12 + f10)); tessellator.a((f18 + 0.0F), (f3 + 0.0F), (f19 + 0.0F), ((f16 + 0.0F) * f12 + f8), ((f17 + 0.0F) * f12 + f10)); }  if (f3 <= f2 + 1.0F) { tessellator.a(f4, f5, f6, 0.8F); tessellator.b(0.0F, 1.0F, 0.0F); tessellator.a((f18 + 0.0F), (f3 + f2 - f15), (f19 + k), ((f16 + 0.0F) * f12 + f8), ((f17 + k) * f12 + f10)); tessellator.a((f18 + k), (f3 + f2 - f15), (f19 + k), ((f16 + k) * f12 + f8), ((f17 + k) * f12 + f10)); tessellator.a((f18 + k), (f3 + f2 - f15), (f19 + 0.0F), ((f16 + k) * f12 + f8), ((f17 + 0.0F) * f12 + f10)); tessellator.a((f18 + 0.0F), (f3 + f2 - f15), (f19 + 0.0F), ((f16 + 0.0F) * f12 + f8), ((f17 + 0.0F) * f12 + f10)); }  tessellator.a(f4 * 0.9F, f5 * 0.9F, f6 * 0.9F, 0.8F); if (i1 > -1) { tessellator.b(-1.0F, 0.0F, 0.0F); for (int k1 = 0; k1 < k; k1++) { tessellator.a((f18 + k1 + 0.0F), (f3 + 0.0F), (f19 + k), ((f16 + k1 + 0.5F) * f12 + f8), ((f17 + k) * f12 + f10)); tessellator.a((f18 + k1 + 0.0F), (f3 + f2), (f19 + k), ((f16 + k1 + 0.5F) * f12 + f8), ((f17 + k) * f12 + f10)); tessellator.a((f18 + k1 + 0.0F), (f3 + f2), (f19 + 0.0F), ((f16 + k1 + 0.5F) * f12 + f8), ((f17 + 0.0F) * f12 + f10)); tessellator.a((f18 + k1 + 0.0F), (f3 + 0.0F), (f19 + 0.0F), ((f16 + k1 + 0.5F) * f12 + f8), ((f17 + 0.0F) * f12 + f10)); }  }  if (i1 <= 1) { tessellator.b(1.0F, 0.0F, 0.0F); for (int l1 = 0; l1 < k; l1++) { tessellator.a((f18 + l1 + 1.0F - f15), (f3 + 0.0F), (f19 + k), ((f16 + l1 + 0.5F) * f12 + f8), ((f17 + k) * f12 + f10)); tessellator.a((f18 + l1 + 1.0F - f15), (f3 + f2), (f19 + k), ((f16 + l1 + 0.5F) * f12 + f8), ((f17 + k) * f12 + f10)); tessellator.a((f18 + l1 + 1.0F - f15), (f3 + f2), (f19 + 0.0F), ((f16 + l1 + 0.5F) * f12 + f8), ((f17 + 0.0F) * f12 + f10)); tessellator.a((f18 + l1 + 1.0F - f15), (f3 + 0.0F), (f19 + 0.0F), ((f16 + l1 + 0.5F) * f12 + f8), ((f17 + 0.0F) * f12 + f10)); }  }  tessellator.a(f4 * 0.8F, f5 * 0.8F, f6 * 0.8F, 0.8F); if (j1 > -1) { tessellator.b(0.0F, 0.0F, -1.0F); for (int i2 = 0; i2 < k; i2++) { tessellator.a((f18 + 0.0F), (f3 + f2), (f19 + i2 + 0.0F), ((f16 + 0.0F) * f12 + f8), ((f17 + i2 + 0.5F) * f12 + f10)); tessellator.a((f18 + k), (f3 + f2), (f19 + i2 + 0.0F), ((f16 + k) * f12 + f8), ((f17 + i2 + 0.5F) * f12 + f10)); tessellator.a((f18 + k), (f3 + 0.0F), (f19 + i2 + 0.0F), ((f16 + k) * f12 + f8), ((f17 + i2 + 0.5F) * f12 + f10)); tessellator.a((f18 + 0.0F), (f3 + 0.0F), (f19 + i2 + 0.0F), ((f16 + 0.0F) * f12 + f8), ((f17 + i2 + 0.5F) * f12 + f10)); }  }  if (j1 <= 1) { tessellator.b(0.0F, 0.0F, 1.0F); for (int j2 = 0; j2 < k; j2++) { tessellator.a((f18 + 0.0F), (f3 + f2), (f19 + j2 + 1.0F - f15), ((f16 + 0.0F) * f12 + f8), ((f17 + j2 + 0.5F) * f12 + f10)); tessellator.a((f18 + k), (f3 + f2), (f19 + j2 + 1.0F - f15), ((f16 + k) * f12 + f8), ((f17 + j2 + 0.5F) * f12 + f10)); tessellator.a((f18 + k), (f3 + 0.0F), (f19 + j2 + 1.0F - f15), ((f16 + k) * f12 + f8), ((f17 + j2 + 0.5F) * f12 + f10)); tessellator.a((f18 + 0.0F), (f3 + 0.0F), (f19 + j2 + 1.0F - f15), ((f16 + 0.0F) * f12 + f8), ((f17 + j2 + 0.5F) * f12 + f10)); }  }  tessellator.a(); }  }  }  GL11.glColor4f(1.0F, 1.0F, 1.0F, 1.0F); GL11.glDisable(3042); GL11.glEnable(2884); } public boolean a(acq entityliving, boolean flag) { if (this.k.size() <= 0) return false;  int num = 0; int maxNum = Config.getUpdatesPerFrame(); if (Config.isDynamicUpdates()) if (!isMoving(entityliving)) maxNum *= 3;   int NOT_IN_FRUSTRUM_MUL = 4; int numValid = 0; ct wrBest = null; float distSqBest = Float.MAX_VALUE; int indexBest = -1; for (int i = 0; i < this.k.size(); i++) { ct wr = this.k.get(i); if (wr == null) continue;  numValid++; if (!wr.q) { this.k.set(i, null); continue; }  float distSq = wr.a(entityliving); if (distSq <= 256.0F) if (isActingNow()) { wr.a(); wr.q = false; this.k.set(i, null); num++; continue; }   if (distSq > 256.0F && num >= maxNum) break;  if (!wr.l)
/*      */         distSq *= NOT_IN_FRUSTRUM_MUL;  if (wrBest == null) { wrBest = wr; distSqBest = distSq; indexBest = i; } else if (distSq < distSqBest) { wrBest = wr; distSqBest = distSq; indexBest = i; }  continue; }  if (wrBest != null) { wrBest.a(); wrBest.q = false; this.k.set(indexBest, null); num++; float maxDiffDistSq = distSqBest / 5.0F; for (int j = 0; j < this.k.size(); j++) { if (num >= maxNum)
/*      */           break;  ct wr = this.k.get(j); if (wr != null) { float distSq = wr.a(entityliving); if (!wr.l)
/*      */             distSq *= NOT_IN_FRUSTRUM_MUL;  float diffDistSq = Math.abs(distSq - distSqBest); if (diffDistSq < maxDiffDistSq) { wr.a(); wr.q = false; this.k.set(j, null); num++; }  }  }  }  if (numValid == 0)
/*      */       this.k.clear();  if (this.k.size() > 100 && numValid < this.k.size() * 4 / 5) { int dstIndex = 0; for (int srcIndex = 0; srcIndex < this.k.size(); srcIndex++) { Object wr = this.k.get(srcIndex); if (wr != null) { if (srcIndex != dstIndex)
/*      */             this.k.set(dstIndex, wr);  dstIndex++; }  }  for (int j = this.k.size() - 1; j >= dstIndex; j--)
/*      */         this.k.remove(j);  }  return true; }
/* 2290 */   private boolean isMoving(acq entityliving) { boolean moving = isMovingNow(entityliving);
/* 2291 */     if (moving) {
/*      */       
/* 2293 */       this.lastMovedTime = System.currentTimeMillis();
/* 2294 */       return true;
/*      */     } 
/*      */     
/* 2297 */     return (System.currentTimeMillis() - this.lastMovedTime < 2000L); } public void a(yw par1EntityPlayer, pl par2MovingObjectPosition, int par3, aan par4ItemStack, float par5) { adz tessellator = adz.a; GL11.glEnable(3042); GL11.glEnable(3008); GL11.glBlendFunc(770, 1); GL11.glColor4f(1.0F, 1.0F, 1.0F, (gk.a((float)System.currentTimeMillis() / 100.0F) * 0.2F + 0.4F) * 0.5F); if (par3 == 0) { if (this.g > 0.0F) { GL11.glBlendFunc(774, 768); int i = this.j.b("/terrain.png"); GL11.glBindTexture(3553, i); GL11.glColor4f(1.0F, 1.0F, 1.0F, 0.5F); GL11.glPushMatrix(); int j = this.i.a(par2MovingObjectPosition.b, par2MovingObjectPosition.c, par2MovingObjectPosition.d); pb block = (j <= 0) ? null : pb.m[j]; GL11.glDisable(3008); GL11.glPolygonOffset(-3.0F, -3.0F); GL11.glEnable(32823); double d = par1EntityPlayer.N + (par1EntityPlayer.o - par1EntityPlayer.N) * par5; double d1 = par1EntityPlayer.O + (par1EntityPlayer.p - par1EntityPlayer.O) * par5; double d2 = par1EntityPlayer.P + (par1EntityPlayer.q - par1EntityPlayer.P) * par5; if (block == null) block = pb.t;  GL11.glEnable(3008); tessellator.b(); tessellator.b(-d, -d1, -d2); tessellator.c(); this.s.a(block, par2MovingObjectPosition.b, par2MovingObjectPosition.c, par2MovingObjectPosition.d, 240 + (int)(this.g * 10.0F)); tessellator.a(); tessellator.b(0.0D, 0.0D, 0.0D); GL11.glDisable(3008); GL11.glPolygonOffset(0.0F, 0.0F); GL11.glDisable(32823); GL11.glEnable(3008); GL11.glDepthMask(true); GL11.glPopMatrix(); }  } else if (par4ItemStack != null) { GL11.glBlendFunc(770, 771); float f = gk.a((float)System.currentTimeMillis() / 100.0F) * 0.2F + 0.8F; GL11.glColor4f(f, f, f, gk.a((float)System.currentTimeMillis() / 200.0F) * 0.2F + 0.5F); int k = this.j.b("/terrain.png"); GL11.glBindTexture(3553, k); int i = par2MovingObjectPosition.b; int i1 = par2MovingObjectPosition.c; int j1 = par2MovingObjectPosition.d; if (par2MovingObjectPosition.e == 0) i1--;  if (par2MovingObjectPosition.e == 1) i1++;  if (par2MovingObjectPosition.e == 2) j1--;  if (par2MovingObjectPosition.e == 3) j1++;  if (par2MovingObjectPosition.e == 4) i--;  if (par2MovingObjectPosition.e == 5) i++;  }  GL11.glDisable(3042); GL11.glDisable(3008); }
/*      */   public void b(yw par1EntityPlayer, pl par2MovingObjectPosition, int par3, aan par4ItemStack, float par5) { if (par3 == 0 && par2MovingObjectPosition.a == aat.a) { GL11.glEnable(3042); GL11.glBlendFunc(770, 771); GL11.glColor4f(0.0F, 0.0F, 0.0F, 0.4F); GL11.glLineWidth(2.0F); GL11.glDisable(3553); GL11.glDepthMask(false); float f = 0.002F; int i = this.i.a(par2MovingObjectPosition.b, par2MovingObjectPosition.c, par2MovingObjectPosition.d); if (i > 0) { pb.m[i].a((ali)this.i, par2MovingObjectPosition.b, par2MovingObjectPosition.c, par2MovingObjectPosition.d); double d = par1EntityPlayer.N + (par1EntityPlayer.o - par1EntityPlayer.N) * par5; double d1 = par1EntityPlayer.O + (par1EntityPlayer.p - par1EntityPlayer.O) * par5; double d2 = par1EntityPlayer.P + (par1EntityPlayer.q - par1EntityPlayer.P) * par5; a(pb.m[i].d(this.i, par2MovingObjectPosition.b, par2MovingObjectPosition.c, par2MovingObjectPosition.d).b(f, f, f).c(-d, -d1, -d2)); }  GL11.glDepthMask(true); GL11.glEnable(3553); GL11.glDisable(3042); }  }
/*      */   private void a(wu par1AxisAlignedBB) { adz tessellator = adz.a; tessellator.a(3); tessellator.a(par1AxisAlignedBB.a, par1AxisAlignedBB.b, par1AxisAlignedBB.c); tessellator.a(par1AxisAlignedBB.d, par1AxisAlignedBB.b, par1AxisAlignedBB.c); tessellator.a(par1AxisAlignedBB.d, par1AxisAlignedBB.b, par1AxisAlignedBB.f); tessellator.a(par1AxisAlignedBB.a, par1AxisAlignedBB.b, par1AxisAlignedBB.f); tessellator.a(par1AxisAlignedBB.a, par1AxisAlignedBB.b, par1AxisAlignedBB.c); tessellator.a(); tessellator.a(3); tessellator.a(par1AxisAlignedBB.a, par1AxisAlignedBB.e, par1AxisAlignedBB.c); tessellator.a(par1AxisAlignedBB.d, par1AxisAlignedBB.e, par1AxisAlignedBB.c); tessellator.a(par1AxisAlignedBB.d, par1AxisAlignedBB.e, par1AxisAlignedBB.f); tessellator.a(par1AxisAlignedBB.a, par1AxisAlignedBB.e, par1AxisAlignedBB.f); tessellator.a(par1AxisAlignedBB.a, par1AxisAlignedBB.e, par1AxisAlignedBB.c); tessellator.a(); tessellator.a(1); tessellator.a(par1AxisAlignedBB.a, par1AxisAlignedBB.b, par1AxisAlignedBB.c); tessellator.a(par1AxisAlignedBB.a, par1AxisAlignedBB.e, par1AxisAlignedBB.c); tessellator.a(par1AxisAlignedBB.d, par1AxisAlignedBB.b, par1AxisAlignedBB.c); tessellator.a(par1AxisAlignedBB.d, par1AxisAlignedBB.e, par1AxisAlignedBB.c); tessellator.a(par1AxisAlignedBB.d, par1AxisAlignedBB.b, par1AxisAlignedBB.f); tessellator.a(par1AxisAlignedBB.d, par1AxisAlignedBB.e, par1AxisAlignedBB.f); tessellator.a(par1AxisAlignedBB.a, par1AxisAlignedBB.b, par1AxisAlignedBB.f); tessellator.a(par1AxisAlignedBB.a, par1AxisAlignedBB.e, par1AxisAlignedBB.f); tessellator.a(); }
/*      */   public void a(int par1, int par2, int par3, int par4, int par5, int par6) { int i = gk.a(par1, 16); int j = gk.a(par2, 16); int k = gk.a(par3, 16); int m = gk.a(par4, 16); int i1 = gk.a(par5, 16); int j1 = gk.a(par6, 16); for (int k1 = i; k1 <= m; k1++) { int l1 = k1 % this.n; if (l1 < 0) l1 += this.n;  for (int i2 = j; i2 <= i1; i2++) { int j2 = i2 % this.o; if (j2 < 0) j2 += this.o;  for (int k2 = k; k2 <= j1; k2++) { int l2 = k2 % this.p; if (l2 < 0) l2 += this.p;  int i3 = (l2 * this.o + j2) * this.n + l1; ct worldrenderer = this.m[i3]; if (!worldrenderer.q) { this.k.add(worldrenderer); worldrenderer.f(); }  }  }  }  }
/*      */   public void a(int par1, int par2, int par3) { a(par1 - 1, par2 - 1, par3 - 1, par1 + 1, par2 + 1, par3 + 1); }
/*      */   public void b(int par1, int par2, int par3) { a(par1 - 1, par2 - 1, par3 - 1, par1 + 1, par2 + 1, par3 + 1); }
/*      */   public void b(int par1, int par2, int par3, int par4, int par5, int par6) { a(par1 - 1, par2 - 1, par3 - 1, par4 + 1, par5 + 1, par6 + 1); }
/*      */   public void a(amd par1ICamera, float par2) { for (int i = 0; i < this.m.length; i++) { if (!this.m[i].e()) this.m[i].a(par1ICamera);  }  this.h++; }
/*      */   public void a(String par1Str, int par2, int par3, int par4) { if (par1Str != null) this.r.w.b("C418 - " + par1Str);  this.r.C.a(par1Str, par2, par3, par4, 1.0F, 1.0F); }
/* 2306 */   private boolean isMovingNow(acq entityliving) { double maxDiff = 0.001D;
/*      */     
/* 2308 */     if (entityliving.ch)
/* 2309 */       return true; 
/* 2310 */     if (entityliving.V())
/* 2311 */       return true; 
/* 2312 */     if (entityliving.bv > maxDiff) {
/* 2313 */       return true;
/*      */     }
/* 2315 */     if (this.r.D.a != 0)
/* 2316 */       return true; 
/* 2317 */     if (this.r.D.b != 0) {
/* 2318 */       return true;
/*      */     }
/* 2320 */     if (Math.abs(entityliving.o - entityliving.l) > maxDiff)
/* 2321 */       return true; 
/* 2322 */     if (Math.abs(entityliving.p - entityliving.m) > maxDiff)
/* 2323 */       return true; 
/* 2324 */     if (Math.abs(entityliving.q - entityliving.n) > maxDiff) {
/* 2325 */       return true;
/*      */     }
/* 2327 */     return false; }
/*      */   public void a(String par1Str, double par2, double par4, double par6, float par8, float par9) { float f = 16.0F; if (par8 > 1.0F) f *= par8;  if (this.r.i.f(par2, par4, par6) < (f * f)) this.r.C.b(par1Str, (float)par2, (float)par4, (float)par6, par8, par9);  }
/*      */   public void a(String par1Str, double par2, double par4, double par6, double par8, double par10, double par12) { b(par1Str, par2, par4, par6, par8, par10, par12); }
/*      */   public rt b(String par1Str, double par2, double par4, double par6, double par8, double par10, double par12) { if (this.r == null || this.r.i == null || this.r.j == null) return null;  int i = this.r.A.Q; if (i == 1 && this.i.r.nextInt(3) == 0) i = 2;  double d = this.r.i.o - par2; double d1 = this.r.i.p - par4; double d2 = this.r.i.q - par6; Object obj = null; Object effectObject = null; if (par1Str.equals("hugeexplosion")) { if (Config.isAnimatedExplosion()) this.r.j.a((rt)(obj = new akw(this.i, par2, par4, par6, par8, par10, par12)));  } else if (par1Str.equals("largeexplode")) { if (Config.isAnimatedExplosion()) this.r.j.a((rt)(obj = new im(this.j, this.i, par2, par4, par6, par8, par10, par12)));  }  if (obj != null) return (rt)obj;  double d3 = 16.0D; if (par1Str.equals("crit")) d3 = 196.0D;  if (d * d + d1 * d1 + d2 * d2 > d3 * d3) return null;  if (i > 1) return null;  if (par1Str.equals("bubble")) { obj = new vg(this.i, par2, par4, par6, par8, par10, par12); CustomColorizer.updateWaterFX((rt)obj, (ali)this.i); } else if (par1Str.equals("suspended")) { if (Config.isWaterParticles()) obj = new ky(this.i, par2, par4, par6, par8, par10, par12);  } else if (par1Str.equals("depthsuspend")) { if (Config.isVoidParticles()) obj = new xy(this.i, par2, par4, par6, par8, par10, par12);  } else if (par1Str.equals("townaura")) { obj = new xy(this.i, par2, par4, par6, par8, par10, par12); CustomColorizer.updateMyceliumFX((rt)obj); } else if (par1Str.equals("crit")) { obj = new sk(this.i, par2, par4, par6, par8, par10, par12); } else if (par1Str.equals("magicCrit")) { obj = new sk(this.i, par2, par4, par6, par8, par10, par12); ((rt)obj).b(((rt)obj).o() * 0.3F, ((rt)obj).p() * 0.8F, ((rt)obj).r()); ((rt)obj).c(((rt)obj).s() + 1); } else if (par1Str.equals("smoke")) { if (Config.isAnimatedSmoke()) obj = new alh(this.i, par2, par4, par6, par8, par10, par12);  } else if (par1Str.equals("mobSpell")) { obj = new px(this.i, par2, par4, par6, 0.0D, 0.0D, 0.0D); ((rt)obj).b((float)par8, (float)par10, (float)par12); } else if (par1Str.equals("spell")) { obj = new px(this.i, par2, par4, par6, par8, par10, par12); } else if (par1Str.equals("instantSpell")) { obj = new px(this.i, par2, par4, par6, par8, par10, par12); ((px)obj).b(144); } else if (par1Str.equals("note")) { obj = new ac(this.i, par2, par4, par6, par8, par10, par12); } else if (par1Str.equals("portal")) { if (Config.isPortalParticles()) { obj = new aep(this.i, par2, par4, par6, par8, par10, par12); CustomColorizer.updatePortalFX((rt)obj); }  } else if (par1Str.equals("enchantmenttable")) { obj = new afc(this.i, par2, par4, par6, par8, par10, par12); } else if (par1Str.equals("explode")) { if (Config.isAnimatedExplosion()) obj = new yx(this.i, par2, par4, par6, par8, par10, par12);  } else if (par1Str.equals("flame")) { if (Config.isAnimatedFlame()) obj = new mj(this.i, par2, par4, par6, par8, par10, par12);  } else if (par1Str.equals("lava")) { obj = new ec(this.i, par2, par4, par6); } else if (par1Str.equals("footstep")) { obj = new yq(this.j, this.i, par2, par4, par6); } else if (par1Str.equals("splash")) { obj = new nw(this.i, par2, par4, par6, par8, par10, par12); CustomColorizer.updateWaterFX((rt)obj, (ali)this.i); } else if (par1Str.equals("largesmoke")) { if (Config.isAnimatedSmoke()) obj = new alh(this.i, par2, par4, par6, par8, par10, par12, 2.5F);  } else if (par1Str.equals("cloud")) { obj = new ajb(this.i, par2, par4, par6, par8, par10, par12); } else if (par1Str.equals("reddust")) { if (Config.isAnimatedRedstone()) { obj = new aab(this.i, par2, par4, par6, (float)par8, (float)par10, (float)par12); CustomColorizer.updateReddustFX((rt)obj, (ali)this.i, d, d1, d2); }  } else if (par1Str.equals("snowballpoof")) { obj = new ld(this.i, par2, par4, par6, yr.aD); effectObject = yr.aD; } else if (par1Str.equals("dripWater")) { if (Config.isDrippingWaterLava()) obj = new ga(this.i, par2, par4, par6, acn.g);  } else if (par1Str.equals("dripLava")) { if (Config.isDrippingWaterLava()) obj = new ga(this.i, par2, par4, par6, acn.h);  } else if (par1Str.equals("snowshovel")) { obj = new jk(this.i, par2, par4, par6, par8, par10, par12); } else if (par1Str.equals("slime")) { obj = new ld(this.i, par2, par4, par6, yr.aM); effectObject = yr.aM; } else if (par1Str.equals("heart")) { obj = new abm(this.i, par2, par4, par6, par8, par10, par12); } else if (par1Str.startsWith("iconcrack_")) { int j = Integer.parseInt(par1Str.substring(par1Str.indexOf("_") + 1)); obj = new ld(this.i, par2, par4, par6, par8, par10, par12, yr.e[j]); effectObject = yr.e[j]; } else if (par1Str.startsWith("tilecrack_")) { int k = Integer.parseInt(par1Str.substring(par1Str.indexOf("_") + 1)); obj = new mc(this.i, par2, par4, par6, par8, par10, par12, pb.m[k], 0, 0); effectObject = yr.e[k]; }  if (obj != null) if (Reflector.hasClass(1)) { Reflector.callVoid(this.r.j, 80, new Object[] { obj, effectObject }); } else { this.r.j.a((rt)obj); }   return (rt)obj; }
/*      */   public void a(nn par1Entity) { par1Entity.S(); if (par1Entity.Z != null) if (par1Entity.Z.startsWith("http")) this.j.a(par1Entity.Z, new mx());   if (par1Entity.aa != null) { this.j.a(par1Entity.aa, new mx()); if (par1Entity instanceof yw) { yw player = (yw)par1Entity; dm tdid = this.j.a(player.aa, new mx()); this.j.c(player.aa); String urlStr = "http://optifine.net/capes/" + player.aA + ".png"; ahw tdi = new ahw(tdid, urlStr, new mx()); tdi.start(); if (!Config.isShowCapes())
/*      */           player.aC = "";  }  }  if (Config.isRandomMobs())
/* 2333 */       RandomMobs.entityLoaded(par1Entity);  } private boolean isActingNow() { if (Mouse.isButtonDown(0))
/* 2334 */       return true; 
/* 2335 */     if (Mouse.isButtonDown(1)) {
/* 2336 */       return true;
/*      */     }
/* 2338 */     return false; }
/*      */   public void b(nn par1Entity) { if (par1Entity.Z != null) this.j.c(par1Entity.Z);  if (par1Entity.aa != null) this.j.c(par1Entity.aa);  }
/*      */   public void setAllRenderersVisible() { if (this.m == null) return;  for (int i = 0; i < this.m.length; i++) (this.m[i]).t = true;  }
/*      */   public void a(int i, int j, int k, kw tileentity) {}
/* 2342 */   public void e() { ew.b(this.q); } public void a(yw par1EntityPlayer, int par2, int par3, int par4, int par5, int par6) { int i, m; double d3, d7, d11; int l1; double d, d4, d8; String s; int i1; double d12, d1, d5, d9; String s1; int j1, k1; float f, f1, f2; String s2; int i2, j, k; Random random = this.i.r; switch (par2) { default: return;case 1001: this.i.a(par3, par4, par5, "random.click", 1.0F, 1.2F);case 1000: this.i.a(par3, par4, par5, "random.click", 1.0F, 1.0F);case 1002: this.i.a(par3, par4, par5, "random.bow", 1.0F, 1.2F);case 2000: i = par6 % 3 - 1; m = par6 / 3 % 3 - 1; d3 = par3 + i * 0.6D + 0.5D; d7 = par4 + 0.5D; d11 = par5 + m * 0.6D + 0.5D; for (l1 = 0; l1 < 10; l1++) { double d13 = random.nextDouble() * 0.2D + 0.01D; double d14 = d3 + i * 0.01D + (random.nextDouble() - 0.5D) * m * 0.5D; double d15 = d7 + (random.nextDouble() - 0.5D) * 0.5D; double d17 = d11 + m * 0.01D + (random.nextDouble() - 0.5D) * i * 0.5D; double d19 = i * d13 + random.nextGaussian() * 0.01D; double d21 = -0.03D + random.nextGaussian() * 0.01D; double d23 = m * d13 + random.nextGaussian() * 0.01D; a("smoke", d14, d15, d17, d19, d21, d23); } case 2003: d = par3 + 0.5D; d4 = par4; d8 = par5 + 0.5D; s = "iconcrack_" + yr.bA.bQ; for (i1 = 0; i1 < 8; i1++) a(s, d, d4, d8, random.nextGaussian() * 0.15D, random.nextDouble() * 0.2D, random.nextGaussian() * 0.15D);  for (d12 = 0.0D; d12 < 6.283185307179586D; d12 += 0.15707963267948966D) { a("portal", d + Math.cos(d12) * 5.0D, d4 - 0.4D, d8 + Math.sin(d12) * 5.0D, Math.cos(d12) * -5.0D, 0.0D, Math.sin(d12) * -5.0D); a("portal", d + Math.cos(d12) * 5.0D, d4 - 0.4D, d8 + Math.sin(d12) * 5.0D, Math.cos(d12) * -7.0D, 0.0D, Math.sin(d12) * -7.0D); } case 2002: d1 = par3; d5 = par4; d9 = par5; s1 = "iconcrack_" + yr.bs.bQ; for (j1 = 0; j1 < 8; j1++) a(s1, d1, d5, d9, random.nextGaussian() * 0.15D, random.nextDouble() * 0.2D, random.nextGaussian() * 0.15D);  k1 = yr.bs.b(par6, 0); f = (k1 >> 16 & 0xFF) / 255.0F; f1 = (k1 >> 8 & 0xFF) / 255.0F; f2 = (k1 >> 0 & 0xFF) / 255.0F; s2 = "spell"; if (yr.bs.d(par6)) s2 = "instantSpell";  for (i2 = 0; i2 < 100; i2++) { double d16 = random.nextDouble() * 4.0D; double d18 = random.nextDouble() * Math.PI * 2.0D; double d20 = Math.cos(d18) * d16; double d22 = 0.01D + random.nextDouble() * 0.5D; double d24 = Math.sin(d18) * d16; rt entityfx = b(s2, d1 + d20 * 0.1D, d5 + 0.3D, d9 + d24 * 0.1D, d20, d22, d24); if (entityfx != null) { float f3 = 0.75F + random.nextFloat() * 0.25F; entityfx.b(f * f3, f1 * f3, f2 * f3); entityfx.c((float)d16); }  }  this.i.a(par3 + 0.5D, par4 + 0.5D, par5 + 0.5D, "random.glass", 1.0F, this.i.r.nextFloat() * 0.1F + 0.9F);case 2001: j = par6 & 0xFFF; if (j > 0) { pb block = pb.m[j]; this.r.C.b(block.cb.a(), par3 + 0.5F, par4 + 0.5F, par5 + 0.5F, (block.cb.b() + 1.0F) / 2.0F, block.cb.c() * 0.8F); }  this.r.j.a(par3, par4, par5, par6 & 0xFFF, par6 >> 12 & 0xFF);case 2004: for (k = 0; k < 20; k++) { double d2 = par3 + 0.5D + (this.i.r.nextFloat() - 0.5D) * 2.0D; double d6 = par4 + 0.5D + (this.i.r.nextFloat() - 0.5D) * 2.0D; double d10 = par5 + 0.5D + (this.i.r.nextFloat() - 0.5D) * 2.0D; this.i.a("smoke", d2, d6, d10, 0.0D, 0.0D, 0.0D); this.i.a("flame", d2, d6, d10, 0.0D, 0.0D, 0.0D); } case 1003: if (Math.random() < 0.5D) { this.i.a(par3 + 0.5D, par4 + 0.5D, par5 + 0.5D, "random.door_open", 1.0F, this.i.r.nextFloat() * 0.1F + 0.9F); } else { this.i.a(par3 + 0.5D, par4 + 0.5D, par5 + 0.5D, "random.door_close", 1.0F, this.i.r.nextFloat() * 0.1F + 0.9F); } case 1004: this.i.a((par3 + 0.5F), (par4 + 0.5F), (par5 + 0.5F), "random.fizz", 0.5F, 2.6F + (random.nextFloat() - random.nextFloat()) * 0.8F);case 1005: if (yr.e[par6] instanceof aid) { this.i.a(((aid)yr.e[par6]).a, par3, par4, par5); } else { this.i.a(null, par3, par4, par5); } case 1007: this.i.a(par3 + 0.5D, par4 + 0.5D, par5 + 0.5D, "mob.ghast.charge", 10.0F, (random.nextFloat() - random.nextFloat()) * 0.2F + 1.0F);case 1008: this.i.a(par3 + 0.5D, par4 + 0.5D, par5 + 0.5D, "mob.ghast.fireball", 10.0F, (random.nextFloat() - random.nextFloat()) * 0.2F + 1.0F);case 1010: this.i.a(par3 + 0.5D, par4 + 0.5D, par5 + 0.5D, "mob.zombie.wood", 2.0F, (random.nextFloat() - random.nextFloat()) * 0.2F + 1.0F);case 1012: this.i.a(par3 + 0.5D, par4 + 0.5D, par5 + 0.5D, "mob.zombie.woodbreak", 2.0F, (random.nextFloat() - random.nextFloat()) * 0.2F + 1.0F);case 1011: break; }  this.i.a(par3 + 0.5D, par4 + 0.5D, par5 + 0.5D, "mob.zombie.metal", 2.0F, (random.nextFloat() - random.nextFloat()) * 0.2F + 1.0F); } public int renderAllSortedRenderers(int renderPass, double partialTicks) { return a(0, this.l.length, renderPass, partialTicks); }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public void updateCapes() {
/* 2350 */     if (this.i == null) {
/*      */       return;
/*      */     }
/* 2353 */     boolean showCapes = Config.isShowCapes();
/*      */     
/* 2355 */     List<nn> playerList = this.i.d;
/* 2356 */     for (int i = 0; i < playerList.size(); i++) {
/*      */       
/* 2358 */       nn entity = playerList.get(i);
/* 2359 */       if (entity instanceof yw) {
/*      */         
/* 2361 */         yw player = (yw)entity;
/* 2362 */         if (showCapes) {
/* 2363 */           player.aC = player.aa;
/*      */         } else {
/* 2365 */           player.aC = "";
/*      */         } 
/*      */       } 
/*      */     } 
/*      */   }
/*      */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\l.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */