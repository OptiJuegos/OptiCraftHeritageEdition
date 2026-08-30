/*     */ import java.util.ArrayList;
/*     */ import java.util.Arrays;
/*     */ import java.util.Collections;
/*     */ import java.util.HashMap;
/*     */ import java.util.Iterator;
/*     */ import java.util.List;
/*     */ import java.util.Map;
/*     */ 
/*     */ public class lv
/*     */ {
/*     */   public static boolean a = false;
/*  12 */   private static List b = new ArrayList();
/*     */ 
/*     */   
/*  15 */   private static List c = new ArrayList();
/*     */ 
/*     */   
/*  18 */   private static String d = "";
/*     */ 
/*     */   
/*  21 */   private static Map e = new HashMap<Object, Object>();
/*     */   
/*     */   public static boolean profilerGlobalEnabled = true;
/*  24 */   private static boolean profilerLocalEnabled = profilerGlobalEnabled;
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static void a() {
/*  35 */     e.clear();
/*  36 */     profilerLocalEnabled = profilerGlobalEnabled;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static void a(String par0Str) {
/*  44 */     if (!profilerLocalEnabled)
/*     */       return; 
/*  46 */     if (!a) {
/*     */       return;
/*     */     }
/*     */     
/*  50 */     if (d.length() > 0)
/*     */     {
/*  52 */       d += ".";
/*     */     }
/*  54 */     d += par0Str;
/*  55 */     b.add(d);
/*  56 */     c.add(Long.valueOf(System.nanoTime()));
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static void b() {
/*  64 */     if (!profilerLocalEnabled)
/*     */       return; 
/*  66 */     if (!a) {
/*     */       return;
/*     */     }
/*     */     
/*  70 */     long l = System.nanoTime();
/*  71 */     long l1 = ((Long)c.remove(c.size() - 1)).longValue();
/*  72 */     b.remove(b.size() - 1);
/*  73 */     long l2 = l - l1;
/*  74 */     if (e.containsKey(d)) {
/*     */       
/*  76 */       e.put(d, Long.valueOf(((Long)e.get(d)).longValue() + l2));
/*     */     } else {
/*     */       
/*  79 */       e.put(d, Long.valueOf(l2));
/*     */     } 
/*  81 */     d = (b.size() <= 0) ? "" : b.get(b.size() - 1);
/*  82 */     if (l2 > 100000000L);
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
/*     */   public static List b(String par0Str) {
/*  94 */     profilerLocalEnabled = profilerGlobalEnabled;
/*  95 */     if (!profilerLocalEnabled)
/*  96 */       return new ArrayList(Arrays.asList((Object[])new aaf[] { new aaf("root", 0.0D, 0.0D) })); 
/*  97 */     if (!a)
/*     */     {
/*  99 */       return null;
/*     */     }
/* 101 */     String s = par0Str;
/* 102 */     long l = e.containsKey("root") ? ((Long)e.get("root")).longValue() : 0L;
/* 103 */     long l1 = e.containsKey(par0Str) ? ((Long)e.get(par0Str)).longValue() : -1L;
/* 104 */     ArrayList<aaf> arraylist = new ArrayList();
/* 105 */     if (par0Str.length() > 0)
/*     */     {
/* 107 */       par0Str = par0Str + ".";
/*     */     }
/* 109 */     long l2 = 0L;
/* 110 */     Iterator<String> iterator = e.keySet().iterator();
/*     */ 
/*     */     
/* 113 */     while (iterator.hasNext()) {
/*     */ 
/*     */ 
/*     */       
/* 117 */       String s1 = iterator.next();
/* 118 */       if (s1.length() > par0Str.length() && s1.startsWith(par0Str) && s1.indexOf(".", par0Str.length() + 1) < 0)
/*     */       {
/* 120 */         l2 += ((Long)e.get(s1)).longValue();
/*     */       }
/*     */     } 
/* 123 */     float f = (float)l2;
/* 124 */     if (l2 < l1)
/*     */     {
/* 126 */       l2 = l1;
/*     */     }
/* 128 */     if (l < l2)
/*     */     {
/* 130 */       l = l2;
/*     */     }
/* 132 */     Iterator<String> iterator1 = e.keySet().iterator();
/*     */ 
/*     */     
/* 135 */     while (iterator1.hasNext()) {
/*     */ 
/*     */ 
/*     */       
/* 139 */       String s2 = iterator1.next();
/* 140 */       if (s2.length() > par0Str.length() && s2.startsWith(par0Str) && s2.indexOf(".", par0Str.length() + 1) < 0) {
/*     */         
/* 142 */         long l3 = ((Long)e.get(s2)).longValue();
/* 143 */         double d = l3 * 100.0D / l2;
/* 144 */         double d1 = l3 * 100.0D / l;
/* 145 */         String s4 = s2.substring(par0Str.length());
/* 146 */         arraylist.add(new aaf(s4, d, d1));
/*     */       } 
/*     */     } 
/*     */     
/* 150 */     for (Iterator<String> iterator2 = e.keySet().iterator(); iterator2.hasNext(); e.put(s3, Long.valueOf(((Long)e.get(s3)).longValue() * 999L / 1000L)))
/*     */     {
/* 152 */       String s3 = iterator2.next();
/*     */     }
/*     */     
/* 155 */     if ((float)l2 > f)
/*     */     {
/* 157 */       arraylist.add(new aaf("unspecified", ((float)l2 - f) * 100.0D / l2, ((float)l2 - f) * 100.0D / l));
/*     */     }
/* 159 */     Collections.sort(arraylist);
/* 160 */     arraylist.add(0, new aaf(s, 100.0D, l2 * 100.0D / l));
/* 161 */     return arraylist;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static void c(String par0Str) {
/* 169 */     if (!profilerLocalEnabled)
/*     */       return; 
/* 171 */     b();
/* 172 */     a(par0Str);
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\lv.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */