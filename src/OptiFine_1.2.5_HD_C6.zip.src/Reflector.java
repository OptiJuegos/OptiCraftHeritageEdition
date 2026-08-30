/*     */ import java.lang.reflect.Field;
/*     */ import java.lang.reflect.Method;
/*     */ import java.util.HashMap;
/*     */ import java.util.List;
/*     */ import java.util.Map;
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
/*     */ public class Reflector
/*     */ {
/*  18 */   private static Class[] classes = new Class[16];
/*  19 */   private static boolean[] classesChecked = new boolean[16];
/*  20 */   private static Map classNameMap = null;
/*     */   
/*  22 */   private static Method[] methods = new Method[256];
/*  23 */   private static boolean[] methodsChecked = new boolean[256];
/*  24 */   private static Map methodNameMap = null;
/*     */   
/*  26 */   private static Field[] fields = new Field[256];
/*  27 */   private static boolean[] fieldsChecked = new boolean[256];
/*  28 */   private static Map fieldNameMap = null;
/*     */   
/*     */   public static final int ModLoader = 0;
/*     */   
/*     */   public static final int ForgeHooksClient = 1;
/*     */   
/*     */   public static final int MinecraftForgeClient = 2;
/*     */   
/*     */   public static final int LightCache = 3;
/*     */   
/*     */   public static final int BlockCoord = 4;
/*     */   
/*     */   public static final int ForgeBlock = 5;
/*     */   
/*     */   public static final int ItemRenderType = 6;
/*     */   
/*     */   public static final int FMLRender = 7;
/*     */   
/*     */   public static final int ForgeEffectRenderer = 8;
/*     */   
/*     */   public static final int ForgeHooks = 9;
/*     */   
/*     */   public static final int Entity = 10;
/*     */   
/*     */   public static final int List = 11;
/*     */   
/*     */   public static final int ModLoader_renderWorldBlock = 0;
/*     */   
/*     */   public static final int ModLoader_renderInvBlock = 1;
/*     */   
/*     */   public static final int ModLoader_renderBlockIsItemFull3D = 2;
/*     */   public static final int ForgeHooksClient_onBlockHighlight = 10;
/*     */   public static final int ForgeHooksClient_canRenderInPass = 11;
/*     */   public static final int ForgeHooksClient_getTexture = 12;
/*     */   public static final int ForgeHooksClient_beforeRenderPass = 13;
/*     */   public static final int ForgeHooksClient_afterRenderPass = 14;
/*     */   public static final int ForgeHooksClient_beforeBlockRender = 15;
/*     */   public static final int ForgeHooksClient_afterBlockRender = 16;
/*     */   public static final int ForgeHooksClient_onRenderWorldLast = 17;
/*     */   public static final int ForgeHooksClient_onTextureLoadPre = 18;
/*     */   public static final int ForgeHooksClient_onTextureLoad = 19;
/*     */   public static final int MinecraftForgeClient_getItemRenderer = 20;
/*     */   public static final int LightCache_clear = 30;
/*     */   public static final int BlockCoord_resetPool = 40;
/*     */   public static final int ForgeBlock_isLadder = 50;
/*     */   public static final int ForgeBlock_isBed = 51;
/*     */   public static final int FMLRender_setTextureDimensions = 70;
/*     */   public static final int FMLRender_preRegisterEffect = 71;
/*     */   public static final int FMLRender_onUpdateTextureEffect = 72;
/*     */   public static final int FMLRender_onTexturePackChange = 73;
/*     */   public static final int ForgeEffectRenderer_addEffect = 80;
/*     */   public static final int ForgeHooks_onEntityLivingSetAttackTarget = 90;
/*     */   public static final int ForgeHooks_onEntityLivingUpdate = 91;
/*     */   public static final int ForgeHooks_onEntityLivingAttacked = 92;
/*     */   public static final int ForgeHooks_onEntityLivingHurt = 93;
/*     */   public static final int ForgeHooks_onEntityLivingDeath = 94;
/*     */   public static final int ForgeHooks_onEntityLivingDrops = 95;
/*     */   public static final int ForgeHooks_onEntityLivingFall = 96;
/*     */   public static final int ForgeHooks_onEntityLivingJump = 97;
/*     */   public static final int List_clear = 110;
/*     */   public static final int LightCache_cache = 30;
/*     */   public static final int ItemRenderType_EQUIPPED = 60;
/*     */   public static final int Entity_captureDrops = 100;
/*     */   public static final int Entity_capturedDrops = 101;
/*     */   
/*     */   private static Map getClassNameMap() {
/*  94 */     if (classNameMap == null) {
/*     */       
/*  96 */       classNameMap = new HashMap<Object, Object>();
/*  97 */       classNameMap.put(Integer.valueOf(0), "ModLoader");
/*  98 */       classNameMap.put(Integer.valueOf(1), "forge.ForgeHooksClient");
/*  99 */       classNameMap.put(Integer.valueOf(2), "forge.MinecraftForgeClient");
/* 100 */       classNameMap.put(Integer.valueOf(3), "LightCache");
/* 101 */       classNameMap.put(Integer.valueOf(4), "BlockCoord");
/* 102 */       classNameMap.put(Integer.valueOf(5), pb.class);
/* 103 */       classNameMap.put(Integer.valueOf(6), "forge.IItemRenderer$ItemRenderType");
/* 104 */       classNameMap.put(Integer.valueOf(7), "FMLRenderAccessLibrary");
/* 105 */       classNameMap.put(Integer.valueOf(8), cw.class);
/* 106 */       classNameMap.put(Integer.valueOf(9), "forge.ForgeHooks");
/* 107 */       classNameMap.put(Integer.valueOf(10), nn.class);
/* 108 */       classNameMap.put(Integer.valueOf(11), List.class);
/*     */     } 
/* 110 */     return classNameMap;
/*     */   }
/*     */ 
/*     */   
/*     */   private static Map getMethodNameMap() {
/* 115 */     if (methodNameMap == null) {
/*     */       
/* 117 */       methodNameMap = new HashMap<Object, Object>();
/* 118 */       methodNameMap.put(Integer.valueOf(0), "renderWorldBlock");
/* 119 */       methodNameMap.put(Integer.valueOf(1), "renderInvBlock");
/* 120 */       methodNameMap.put(Integer.valueOf(2), "renderBlockIsItemFull3D");
/*     */       
/* 122 */       methodNameMap.put(Integer.valueOf(10), "onBlockHighlight");
/* 123 */       methodNameMap.put(Integer.valueOf(11), "canRenderInPass");
/* 124 */       methodNameMap.put(Integer.valueOf(12), "getTexture");
/* 125 */       methodNameMap.put(Integer.valueOf(13), "beforeRenderPass");
/* 126 */       methodNameMap.put(Integer.valueOf(14), "afterRenderPass");
/* 127 */       methodNameMap.put(Integer.valueOf(15), "beforeBlockRender");
/* 128 */       methodNameMap.put(Integer.valueOf(16), "afterBlockRender");
/* 129 */       methodNameMap.put(Integer.valueOf(17), "onRenderWorldLast");
/* 130 */       methodNameMap.put(Integer.valueOf(18), "onTextureLoadPre");
/* 131 */       methodNameMap.put(Integer.valueOf(19), "onTextureLoad");
/*     */       
/* 133 */       methodNameMap.put(Integer.valueOf(20), "getItemRenderer");
/*     */       
/* 135 */       methodNameMap.put(Integer.valueOf(30), "clear");
/* 136 */       methodNameMap.put(Integer.valueOf(40), "resetPool");
/*     */       
/* 138 */       methodNameMap.put(Integer.valueOf(50), "isLadder");
/* 139 */       methodNameMap.put(Integer.valueOf(51), "isBed");
/*     */       
/* 141 */       methodNameMap.put(Integer.valueOf(70), "setTextureDimensions");
/* 142 */       methodNameMap.put(Integer.valueOf(71), "preRegisterEffect");
/* 143 */       methodNameMap.put(Integer.valueOf(72), "onUpdateTextureEffect");
/* 144 */       methodNameMap.put(Integer.valueOf(73), "onTexturePackChange");
/*     */       
/* 146 */       methodNameMap.put(Integer.valueOf(80), "addEffect");
/*     */       
/* 148 */       methodNameMap.put(Integer.valueOf(90), "onEntityLivingSetAttackTarget");
/* 149 */       methodNameMap.put(Integer.valueOf(91), "onEntityLivingUpdate");
/* 150 */       methodNameMap.put(Integer.valueOf(92), "onEntityLivingAttacked");
/* 151 */       methodNameMap.put(Integer.valueOf(93), "onEntityLivingHurt");
/* 152 */       methodNameMap.put(Integer.valueOf(94), "onEntityLivingDeath");
/* 153 */       methodNameMap.put(Integer.valueOf(95), "onEntityLivingDrops");
/* 154 */       methodNameMap.put(Integer.valueOf(96), "onEntityLivingFall");
/* 155 */       methodNameMap.put(Integer.valueOf(97), "onEntityLivingJump");
/*     */       
/* 157 */       methodNameMap.put(Integer.valueOf(110), "clear");
/*     */     } 
/* 159 */     return methodNameMap;
/*     */   }
/*     */ 
/*     */   
/*     */   private static Map getFieldNameMap() {
/* 164 */     if (fieldNameMap == null) {
/*     */       
/* 166 */       fieldNameMap = new HashMap<Object, Object>();
/* 167 */       fieldNameMap.put(Integer.valueOf(30), "cache");
/* 168 */       fieldNameMap.put(Integer.valueOf(60), "EQUIPPED");
/* 169 */       fieldNameMap.put(Integer.valueOf(100), "captureDrops");
/* 170 */       fieldNameMap.put(Integer.valueOf(101), "capturedDrops");
/*     */     } 
/* 172 */     return fieldNameMap;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static void callVoid(int methodId, Object[] params) {
/*     */     try {
/* 181 */       Method m = getMethod(methodId);
/* 182 */       if (m == null) {
/*     */         return;
/*     */       }
/* 185 */       m.invoke(null, params);
/*     */     }
/* 187 */     catch (Exception e) {
/*     */       
/* 189 */       e.printStackTrace();
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static int callInt(int methodId, Object[] params) {
/* 199 */     Integer val = (Integer)call(methodId, params);
/* 200 */     return val.intValue();
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static String callString(int methodId, Object[] params) {
/* 209 */     return (String)call(methodId, params);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static boolean callBoolean(int methodId, Object[] params) {
/*     */     try {
/* 218 */       Method method = getMethod(methodId);
/* 219 */       if (method == null)
/* 220 */         return false; 
/* 221 */       Boolean retVal = (Boolean)method.invoke(null, params);
/*     */       
/* 223 */       return retVal.booleanValue();
/*     */     }
/* 225 */     catch (Throwable e) {
/*     */       
/* 227 */       e.printStackTrace();
/* 228 */       return false;
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static boolean callBoolean(Object obj, int methodId, Object[] params) {
/*     */     try {
/* 238 */       Method method = getMethod(methodId);
/* 239 */       if (method == null)
/* 240 */         return false; 
/* 241 */       Boolean retVal = (Boolean)method.invoke(obj, params);
/*     */ 
/*     */       
/* 244 */       return retVal.booleanValue();
/*     */     }
/* 246 */     catch (Throwable e) {
/*     */       
/* 248 */       e.printStackTrace();
/* 249 */       return false;
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static Object call(int methodId, Object[] params) {
/*     */     try {
/* 259 */       Method method = getMethod(methodId);
/* 260 */       if (method == null)
/* 261 */         return Boolean.valueOf(false); 
/* 262 */       Object retVal = method.invoke(null, params);
/*     */       
/* 264 */       return retVal;
/*     */     }
/* 266 */     catch (Throwable e) {
/*     */       
/* 268 */       e.printStackTrace();
/* 269 */       return null;
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static void callVoid(Object obj, int methodId, Object[] params) {
/*     */     try {
/* 280 */       if (obj == null)
/*     */         return; 
/* 282 */       Method method = getMethod(methodId);
/* 283 */       if (method == null)
/*     */         return; 
/* 285 */       method.invoke(obj, params);
/*     */     
/*     */     }
/* 288 */     catch (Throwable e) {
/*     */       
/* 290 */       e.printStackTrace();
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static Method getMethod(int methodId) {
/* 299 */     Method m = methods[methodId];
/* 300 */     if (m == null) {
/*     */       
/* 302 */       if (methodsChecked[methodId])
/* 303 */         return null; 
/* 304 */       methodsChecked[methodId] = true;
/*     */       
/* 306 */       m = findMethod(methodId);
/* 307 */       methods[methodId] = m;
/*     */     } 
/* 309 */     return m;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static Method findMethod(int methodId) {
/* 318 */     int classId = methodId / 10;
/* 319 */     Class cls = getClass(classId);
/* 320 */     if (cls == null) {
/* 321 */       return null;
/*     */     }
/* 323 */     String methodName = (String)getMethodNameMap().get(Integer.valueOf(methodId));
/* 324 */     if (methodName == null) {
/*     */       
/* 326 */       Config.log("Method name not found for id: " + methodId);
/* 327 */       return null;
/*     */     } 
/* 329 */     Method[] ms = cls.getMethods();
/* 330 */     for (int i = 0; i < ms.length; ) {
/*     */       
/* 332 */       Method m = ms[i];
/* 333 */       if (!m.getName().equals(methodName)) {
/*     */         i++; continue;
/*     */       } 
/* 336 */       return m;
/*     */     } 
/*     */     
/* 339 */     Config.log("Method not found: " + cls.getName() + "." + methodName);
/* 340 */     return null;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static Field getField(int fieldId) {
/* 348 */     Field f = fields[fieldId];
/* 349 */     if (f == null) {
/*     */       
/* 351 */       if (fieldsChecked[fieldId])
/* 352 */         return null; 
/* 353 */       fieldsChecked[fieldId] = true;
/*     */       
/* 355 */       f = findField(fieldId);
/* 356 */       fields[fieldId] = f;
/*     */     } 
/* 358 */     return f;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static Field findField(int fieldId) {
/* 367 */     int classId = fieldId / 10;
/* 368 */     Class cls = getClass(classId);
/* 369 */     if (cls == null) {
/* 370 */       return null;
/*     */     }
/* 372 */     String fieldName = (String)getFieldNameMap().get(Integer.valueOf(fieldId));
/* 373 */     if (fieldName == null) {
/*     */       
/* 375 */       Config.log("Field name not found for id: " + fieldId);
/* 376 */       return null;
/*     */     } 
/*     */     
/*     */     try {
/* 380 */       Field f = cls.getDeclaredField(fieldName);
/*     */       
/* 382 */       return f;
/*     */     }
/* 384 */     catch (SecurityException e) {
/*     */ 
/*     */       
/* 387 */       e.printStackTrace();
/*     */     }
/* 389 */     catch (NoSuchFieldException e) {
/*     */       
/* 391 */       Config.log("Field not found: " + cls.getName() + "." + fieldName);
/*     */     } 
/*     */     
/* 394 */     return null;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   private static Class getClass(int classId) {
/* 403 */     Class<?> cls = classes[classId];
/* 404 */     if (cls == null) {
/*     */       
/* 406 */       if (classesChecked[classId]) {
/* 407 */         return null;
/*     */       }
/* 409 */       classesChecked[classId] = true;
/*     */       
/* 411 */       Object classValue = getClassNameMap().get(Integer.valueOf(classId));
/* 412 */       if (classValue instanceof Class) {
/*     */         
/* 414 */         cls = (Class)classValue;
/*     */         
/* 416 */         classes[classId] = cls;
/*     */         
/* 418 */         return cls;
/*     */       } 
/*     */       
/* 421 */       String className = (String)classValue;
/* 422 */       if (className == null) {
/*     */         
/* 424 */         Config.log("Class name not found for id: " + classId);
/* 425 */         return null;
/*     */       } 
/*     */ 
/*     */ 
/*     */       
/*     */       try {
/* 431 */         cls = Class.forName(className);
/*     */         
/* 433 */         classes[classId] = cls;
/*     */       }
/* 435 */       catch (ClassNotFoundException e) {
/*     */         
/* 437 */         Config.log("Class not present: " + className);
/*     */       }
/* 439 */       catch (Throwable e) {
/*     */         
/* 441 */         e.printStackTrace();
/*     */       } 
/*     */     } 
/*     */     
/* 445 */     return cls;
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static boolean hasClass(int classId) {
/* 453 */     Class cls = getClass(classId);
/* 454 */     return (cls != null);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static boolean hasMethod(int methodId) {
/* 462 */     Method m = getMethod(methodId);
/* 463 */     return (m != null);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static Object getFieldValue(int fieldId) {
/* 471 */     return getFieldValue(null, fieldId);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static Object getFieldValue(Object obj, int fieldId) {
/*     */     try {
/* 481 */       Field field = getField(fieldId);
/* 482 */       if (field == null) {
/* 483 */         return null;
/*     */       }
/* 485 */       Object value = field.get(obj);
/*     */       
/* 487 */       return value;
/*     */     }
/* 489 */     catch (Throwable e) {
/*     */       
/* 491 */       e.printStackTrace();
/* 492 */       return null;
/*     */     } 
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static void setFieldValue(int fieldId, Object value) {
/* 501 */     setFieldValue(null, fieldId, value);
/*     */   }
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */ 
/*     */   
/*     */   public static void setFieldValue(Object obj, int fieldId, Object value) {
/*     */     try {
/* 511 */       Field field = getField(fieldId);
/* 512 */       if (field == null) {
/*     */         return;
/*     */       }
/* 515 */       field.set(obj, value);
/*     */     
/*     */     }
/* 518 */     catch (Throwable e) {
/*     */       
/* 520 */       e.printStackTrace();
/*     */     } 
/*     */   }
/*     */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\Reflector.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */