/*      */ import java.awt.Dimension;
/*      */ import java.io.BufferedReader;
/*      */ import java.io.File;
/*      */ import java.io.FileInputStream;
/*      */ import java.io.FileOutputStream;
/*      */ import java.io.IOException;
/*      */ import java.io.InputStream;
/*      */ import java.io.InputStreamReader;
/*      */ import java.io.OutputStreamWriter;
/*      */ import java.lang.reflect.Method;
/*      */ import java.util.ArrayList;
/*      */ import java.util.Arrays;
/*      */ import java.util.Comparator;
/*      */ import java.util.Date;
/*      */ import java.util.HashMap;
/*      */ import java.util.List;
/*      */ import java.util.Map;
/*      */ import java.util.StringTokenizer;
/*      */ import net.minecraft.client.Minecraft;
/*      */ import org.lwjgl.LWJGLException;
/*      */ import org.lwjgl.Sys;
/*      */ import org.lwjgl.opengl.Display;
/*      */ import org.lwjgl.opengl.DisplayMode;
/*      */ import org.lwjgl.opengl.GL11;
/*      */ import org.lwjgl.opengl.GLContext;
/*      */ import org.lwjgl.util.glu.GLU;
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ public class Config
/*      */ {
/*      */   public static final String OF_NAME = "OptiFine";
/*      */   public static final String MC_VERSION = "1.2.5";
/*      */   public static final String OF_EDITION = "HD";
/*      */   public static final String OF_RELEASE = "C6";
/*      */   public static final String VERSION = "OptiFine_1.2.5_HD_C6";
/*   49 */   private static String newRelease = null;
/*      */   
/*   51 */   private static hu gameSettings = null;
/*      */   
/*   53 */   private static Minecraft minecraft = null;
/*      */   
/*   55 */   private static int iconWidthTerrain = 16;
/*   56 */   private static int iconWidthItems = 16;
/*      */   
/*   58 */   private static Map foundClassesMap = new HashMap<Object, Object>();
/*      */   
/*   60 */   private static long textureUpdateTime = 0L;
/*      */   
/*   62 */   private static DisplayMode desktopDisplayMode = null;
/*      */   
/*   64 */   private static File logFile = null;
/*      */   
/*   66 */   public static final Boolean DEF_FOG_FANCY = Boolean.valueOf(true);
/*   67 */   public static final Float DEF_FOG_START = Float.valueOf(0.2F);
/*   68 */   public static final Boolean DEF_OPTIMIZE_RENDER_DISTANCE = Boolean.valueOf(false);
/*   69 */   public static final Boolean DEF_OCCLUSION_ENABLED = Boolean.valueOf(false);
/*   70 */   public static final Integer DEF_MIPMAP_LEVEL = Integer.valueOf(0);
/*   71 */   public static final Integer DEF_MIPMAP_TYPE = Integer.valueOf(9984);
/*   72 */   public static final Float DEF_ALPHA_FUNC_LEVEL = Float.valueOf(0.1F);
/*   73 */   public static final Boolean DEF_LOAD_CHUNKS_FAR = Boolean.valueOf(false);
/*   74 */   public static final Integer DEF_PRELOADED_CHUNKS = Integer.valueOf(0);
/*   75 */   public static final Integer DEF_CHUNKS_LIMIT = Integer.valueOf(25);
/*   76 */   public static final Integer DEF_UPDATES_PER_FRAME = Integer.valueOf(3);
/*   77 */   public static final Boolean DEF_DYNAMIC_UPDATES = Boolean.valueOf(false);
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static String getVersion() {
/*   91 */     return "OptiFine_1.2.5_HD_C6";
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static void checkOpenGlCaps() {
/*   99 */     log("");
/*  100 */     log(getVersion());
/*  101 */     log("" + new Date());
/*  102 */     log("OS: " + System.getProperty("os.name") + " (" + System.getProperty("os.arch") + ") version " + System.getProperty("os.version"));
/*  103 */     log("Java: " + System.getProperty("java.version") + ", " + System.getProperty("java.vendor"));
/*  104 */     log("VM: " + System.getProperty("java.vm.name") + " (" + System.getProperty("java.vm.info") + "), " + System.getProperty("java.vm.vendor"));
/*  105 */     log("LWJGL: " + Sys.getVersion());
/*  106 */     log("OpenGL: " + GL11.glGetString(7937) + " version " + GL11.glGetString(7938) + ", " + GL11.glGetString(7936));
/*      */     
/*  108 */     int ver = getOpenGlVersion();
/*  109 */     String verStr = "" + (ver / 10) + "." + (ver % 10);
/*  110 */     log("OpenGL Version: " + verStr);
/*      */     
/*  112 */     if (!(GLContext.getCapabilities()).OpenGL12) {
/*  113 */       log("OpenGL Mipmap levels: Not available (GL12.GL_TEXTURE_MAX_LEVEL)");
/*      */     }
/*  115 */     if (!(GLContext.getCapabilities()).GL_NV_fog_distance) {
/*  116 */       log("OpenGL Fancy fog: Not available (GL_NV_fog_distance)");
/*      */     }
/*  118 */     if (!(GLContext.getCapabilities()).GL_ARB_occlusion_query) {
/*  119 */       log("OpenGL Occlussion culling: Not available (GL_ARB_occlusion_query)");
/*      */     }
/*      */   }
/*      */   
/*      */   public static boolean isFancyFogAvailable() {
/*  124 */     return (GLContext.getCapabilities()).GL_NV_fog_distance;
/*      */   }
/*      */ 
/*      */   
/*      */   public static boolean isOcclusionAvailable() {
/*  129 */     return (GLContext.getCapabilities()).GL_ARB_occlusion_query;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static int getOpenGlVersion() {
/*  137 */     if (!(GLContext.getCapabilities()).OpenGL11)
/*  138 */       return 10; 
/*  139 */     if (!(GLContext.getCapabilities()).OpenGL12)
/*  140 */       return 11; 
/*  141 */     if (!(GLContext.getCapabilities()).OpenGL13)
/*  142 */       return 12; 
/*  143 */     if (!(GLContext.getCapabilities()).OpenGL14)
/*  144 */       return 13; 
/*  145 */     if (!(GLContext.getCapabilities()).OpenGL15) {
/*  146 */       return 14;
/*      */     }
/*  148 */     if (!(GLContext.getCapabilities()).OpenGL20)
/*  149 */       return 15; 
/*  150 */     if (!(GLContext.getCapabilities()).OpenGL21)
/*  151 */       return 20; 
/*  152 */     if (!(GLContext.getCapabilities()).OpenGL30)
/*  153 */       return 21; 
/*  154 */     if (!(GLContext.getCapabilities()).OpenGL31)
/*  155 */       return 30; 
/*  156 */     if (!(GLContext.getCapabilities()).OpenGL32)
/*  157 */       return 31; 
/*  158 */     if (!(GLContext.getCapabilities()).OpenGL33)
/*  159 */       return 32; 
/*  160 */     if (!(GLContext.getCapabilities()).OpenGL40) {
/*  161 */       return 33;
/*      */     }
/*  163 */     return 40;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static void setGameSettings(hu options) {
/*  171 */     if (gameSettings == null) {
/*      */ 
/*      */       
/*  174 */       checkOpenGlCaps();
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */       
/*  190 */       startVersionCheckThread();
/*      */     } 
/*      */     
/*  193 */     gameSettings = options;
/*      */     
/*  195 */     minecraft = gameSettings.B;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static void startVersionCheckThread() {
/*  202 */     VersionCheckThread vct = new VersionCheckThread();
/*  203 */     vct.start();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isUseMipmaps() {
/*  210 */     int mipmapLevel = getMipmapLevel();
/*  211 */     return (mipmapLevel > 0);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getMipmapLevel() {
/*  218 */     if (gameSettings == null) {
/*  219 */       return DEF_MIPMAP_LEVEL.intValue();
/*      */     }
/*  221 */     return gameSettings.ofMipmapLevel;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getMipmapType() {
/*  229 */     if (gameSettings == null) {
/*  230 */       return DEF_MIPMAP_TYPE.intValue();
/*      */     }
/*  232 */     if (gameSettings.ofMipmapLinear) {
/*  233 */       return 9986;
/*      */     }
/*  235 */     return 9984;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isUseAlphaFunc() {
/*  244 */     float alphaFuncLevel = getAlphaFuncLevel();
/*      */     
/*  246 */     return (alphaFuncLevel > DEF_ALPHA_FUNC_LEVEL.floatValue() + 1.0E-5F);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static float getAlphaFuncLevel() {
/*  253 */     return DEF_ALPHA_FUNC_LEVEL.floatValue();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isFogFancy() {
/*  260 */     if (!isFancyFogAvailable()) {
/*  261 */       return false;
/*      */     }
/*  263 */     if (gameSettings == null) {
/*  264 */       return false;
/*      */     }
/*  266 */     return (gameSettings.ofFogType == 2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isFogFast() {
/*  273 */     if (gameSettings == null) {
/*  274 */       return false;
/*      */     }
/*  276 */     return (gameSettings.ofFogType == 1);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isFogOff() {
/*  283 */     if (gameSettings == null) {
/*  284 */       return false;
/*      */     }
/*  286 */     return (gameSettings.ofFogType == 3);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static float getFogStart() {
/*  294 */     if (gameSettings == null) {
/*  295 */       return DEF_FOG_START.floatValue();
/*      */     }
/*  297 */     return gameSettings.ofFogStart;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isOcclusionEnabled() {
/*  304 */     if (gameSettings == null) {
/*  305 */       return DEF_OCCLUSION_ENABLED.booleanValue();
/*      */     }
/*  307 */     return gameSettings.h;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isOcclusionFancy() {
/*  314 */     if (!isOcclusionEnabled()) {
/*  315 */       return false;
/*      */     }
/*  317 */     if (gameSettings == null) {
/*  318 */       return false;
/*      */     }
/*  320 */     return gameSettings.ofOcclusionFancy;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isLoadChunksFar() {
/*  328 */     if (gameSettings == null) {
/*  329 */       return DEF_LOAD_CHUNKS_FAR.booleanValue();
/*      */     }
/*  331 */     return gameSettings.ofLoadFar;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getPreloadedChunks() {
/*  338 */     if (gameSettings == null) {
/*  339 */       return DEF_PRELOADED_CHUNKS.intValue();
/*      */     }
/*  341 */     return gameSettings.ofPreloadedChunks;
/*      */   }
/*      */ 
/*      */   
/*      */   public static void dbg(String s) {
/*  346 */     System.out.println(s);
/*      */   }
/*      */ 
/*      */   
/*      */   public static void log(String s) {
/*  351 */     dbg(s);
/*      */     
/*      */     try {
/*  354 */       if (logFile == null) {
/*      */         
/*  356 */         logFile = new File(Minecraft.b(), "optifog.log");
/*      */         
/*  358 */         logFile.delete();
/*  359 */         logFile.createNewFile();
/*      */       } 
/*      */       
/*  362 */       FileOutputStream fos = new FileOutputStream(logFile, true);
/*  363 */       OutputStreamWriter logFileWriter = new OutputStreamWriter(fos, "ASCII");
/*      */ 
/*      */       
/*      */       try {
/*  367 */         logFileWriter.write(s);
/*  368 */         logFileWriter.write("\n");
/*  369 */         logFileWriter.flush();
/*      */       }
/*      */       finally {
/*      */         
/*  373 */         logFileWriter.close();
/*      */       }
/*      */     
/*  376 */     } catch (IOException e) {
/*      */       
/*  378 */       e.printStackTrace();
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getUpdatesPerFrame() {
/*  386 */     if (gameSettings != null) {
/*  387 */       return gameSettings.ofChunkUpdates;
/*      */     }
/*      */ 
/*      */     
/*  391 */     return 1;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isDynamicUpdates() {
/*  398 */     if (gameSettings != null) {
/*  399 */       return gameSettings.ofChunkUpdatesDynamic;
/*      */     }
/*  401 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isRainFancy() {
/*  408 */     if (gameSettings.ofRain == 0) {
/*  409 */       return gameSettings.j;
/*      */     }
/*  411 */     return (gameSettings.ofRain == 2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isWaterFancy() {
/*  418 */     if (gameSettings.ofWater == 0) {
/*  419 */       return gameSettings.j;
/*      */     }
/*  421 */     return (gameSettings.ofWater == 2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isRainOff() {
/*  428 */     return (gameSettings.ofRain == 3);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isCloudsFancy() {
/*  435 */     if (gameSettings.ofClouds == 0) {
/*  436 */       return gameSettings.j;
/*      */     }
/*  438 */     return (gameSettings.ofClouds == 2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isCloudsOff() {
/*  445 */     return (gameSettings.ofClouds == 3);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isTreesFancy() {
/*  452 */     if (gameSettings.ofTrees == 0) {
/*  453 */       return gameSettings.j;
/*      */     }
/*  455 */     return (gameSettings.ofTrees == 2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isGrassFancy() {
/*  462 */     if (gameSettings.ofGrass == 0) {
/*  463 */       return gameSettings.j;
/*      */     }
/*  465 */     return (gameSettings.ofGrass == 2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int limit(int val, int min, int max) {
/*  472 */     if (val < min)
/*  473 */       return min; 
/*  474 */     if (val > max) {
/*  475 */       return max;
/*      */     }
/*  477 */     return val;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static float limit(float val, float min, float max) {
/*  484 */     if (val < min)
/*  485 */       return min; 
/*  486 */     if (val > max) {
/*  487 */       return max;
/*      */     }
/*  489 */     return val;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static float limitTo1(float val) {
/*  496 */     if (val < 0.0F)
/*  497 */       return 0.0F; 
/*  498 */     if (val > 1.0F) {
/*  499 */       return 1.0F;
/*      */     }
/*  501 */     return val;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isAnimatedWater() {
/*  508 */     if (gameSettings != null) {
/*  509 */       return (gameSettings.ofAnimatedWater != 2);
/*      */     }
/*  511 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isGeneratedWater() {
/*  518 */     if (gameSettings != null) {
/*  519 */       return (gameSettings.ofAnimatedWater == 1);
/*      */     }
/*  521 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isAnimatedPortal() {
/*  528 */     if (gameSettings != null) {
/*  529 */       return gameSettings.ofAnimatedPortal;
/*      */     }
/*  531 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isAnimatedLava() {
/*  538 */     if (gameSettings != null) {
/*  539 */       return (gameSettings.ofAnimatedLava != 2);
/*      */     }
/*  541 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isGeneratedLava() {
/*  548 */     if (gameSettings != null) {
/*  549 */       return (gameSettings.ofAnimatedLava == 1);
/*      */     }
/*  551 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isAnimatedFire() {
/*  558 */     if (gameSettings != null) {
/*  559 */       return gameSettings.ofAnimatedFire;
/*      */     }
/*  561 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isAnimatedRedstone() {
/*  568 */     if (gameSettings != null) {
/*  569 */       return gameSettings.ofAnimatedRedstone;
/*      */     }
/*  571 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isAnimatedExplosion() {
/*  578 */     if (gameSettings != null) {
/*  579 */       return gameSettings.ofAnimatedExplosion;
/*      */     }
/*  581 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isAnimatedFlame() {
/*  588 */     if (gameSettings != null) {
/*  589 */       return gameSettings.ofAnimatedFlame;
/*      */     }
/*  591 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isAnimatedSmoke() {
/*  598 */     if (gameSettings != null) {
/*  599 */       return gameSettings.ofAnimatedSmoke;
/*      */     }
/*  601 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isVoidParticles() {
/*  608 */     if (gameSettings != null) {
/*  609 */       return gameSettings.ofVoidParticles;
/*      */     }
/*  611 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isWaterParticles() {
/*  618 */     if (gameSettings != null) {
/*  619 */       return gameSettings.ofWaterParticles;
/*      */     }
/*  621 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isRainSplash() {
/*  628 */     if (gameSettings != null) {
/*  629 */       return gameSettings.ofRainSplash;
/*      */     }
/*  631 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isPortalParticles() {
/*  638 */     if (gameSettings != null) {
/*  639 */       return gameSettings.ofPortalParticles;
/*      */     }
/*  641 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isDepthFog() {
/*  648 */     if (gameSettings != null) {
/*  649 */       return gameSettings.ofDepthFog;
/*      */     }
/*  651 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static float getAmbientOcclusionLevel() {
/*  659 */     if (gameSettings != null) {
/*  660 */       return gameSettings.ofAoLevel;
/*      */     }
/*  662 */     return 0.0F;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static Method getMethod(Class cls, String methodName, Object[] params) {
/*  673 */     Method[] methods = cls.getMethods();
/*  674 */     for (int i = 0; i < methods.length; i++) {
/*      */       
/*  676 */       Method m = methods[i];
/*      */       
/*  678 */       if (m.getName().equals(methodName))
/*      */       {
/*      */         
/*  681 */         if ((m.getParameterTypes()).length == params.length)
/*      */         {
/*      */           
/*  684 */           return m; } 
/*      */       }
/*      */     } 
/*  687 */     dbg("No method found for: " + cls.getName() + "." + methodName + "(" + arrayToString(params) + ")");
/*  688 */     return null;
/*      */   }
/*      */ 
/*      */   
/*      */   public static String arrayToString(Object[] arr) {
/*  693 */     StringBuffer buf = new StringBuffer(arr.length * 5);
/*  694 */     for (int i = 0; i < arr.length; i++) {
/*      */       
/*  696 */       Object obj = arr[i];
/*  697 */       if (i > 0)
/*  698 */         buf.append(", "); 
/*  699 */       buf.append(String.valueOf(obj));
/*      */     } 
/*  701 */     return buf.toString();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static Minecraft getMinecraft() {
/*  709 */     return minecraft;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getIconWidthTerrain() {
/*  716 */     return iconWidthTerrain;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getIconWidthItems() {
/*  723 */     return iconWidthItems;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static void setIconWidthItems(int iconWidth) {
/*  730 */     iconWidthItems = iconWidth;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static void setIconWidthTerrain(int iconWidth) {
/*  737 */     iconWidthTerrain = iconWidth;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getMaxDynamicTileWidth() {
/*  744 */     return 64;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getSideGrassTexture(ali blockAccess, int x, int y, int z, int side, int tileNum) {
/*  753 */     if (!isBetterGrass())
/*      */     {
/*      */       
/*  756 */       return tileNum;
/*      */     }
/*      */     
/*  759 */     int fullTileNum = 0;
/*  760 */     int destBlockId = 2;
/*      */     
/*  762 */     if (tileNum == 77) {
/*      */       
/*  764 */       fullTileNum = 78;
/*  765 */       destBlockId = 110;
/*      */     } 
/*      */     
/*  768 */     if (isBetterGrassFancy()) {
/*      */ 
/*      */ 
/*      */       
/*  772 */       y--;
/*  773 */       switch (side) {
/*      */ 
/*      */         
/*      */         case 2:
/*  777 */           z--;
/*      */           break;
/*      */         
/*      */         case 3:
/*  781 */           z++;
/*      */           break;
/*      */         
/*      */         case 4:
/*  785 */           x--;
/*      */           break;
/*      */         
/*      */         case 5:
/*  789 */           x++;
/*      */           break;
/*      */       } 
/*      */       
/*  793 */       int blockId = blockAccess.a(x, y, z);
/*      */       
/*  795 */       if (blockId != destBlockId)
/*      */       {
/*      */         
/*  798 */         return tileNum;
/*      */       }
/*      */     } 
/*      */     
/*  802 */     return fullTileNum;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int getSideSnowGrassTexture(ali blockAccess, int x, int y, int z, int side) {
/*  810 */     if (!isBetterGrass())
/*      */     {
/*      */       
/*  813 */       return 68;
/*      */     }
/*  815 */     if (isBetterGrassFancy()) {
/*      */ 
/*      */ 
/*      */       
/*  819 */       switch (side) {
/*      */ 
/*      */         
/*      */         case 2:
/*  823 */           z--;
/*      */           break;
/*      */         
/*      */         case 3:
/*  827 */           z++;
/*      */           break;
/*      */         
/*      */         case 4:
/*  831 */           x--;
/*      */           break;
/*      */         
/*      */         case 5:
/*  835 */           x++;
/*      */           break;
/*      */       } 
/*      */       
/*  839 */       int blockId = blockAccess.a(x, y, z);
/*  840 */       if (blockId != 78 && blockId != 80)
/*      */       {
/*      */         
/*  843 */         return 68;
/*      */       }
/*      */     } 
/*      */     
/*  847 */     return 66;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isBetterGrass() {
/*  854 */     if (gameSettings == null) {
/*  855 */       return false;
/*      */     }
/*  857 */     return (gameSettings.ofBetterGrass != 3);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isBetterGrassFancy() {
/*  864 */     if (gameSettings == null) {
/*  865 */       return false;
/*      */     }
/*  867 */     return (gameSettings.ofBetterGrass == 2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static long getTextureUpdateTime() {
/*  874 */     return textureUpdateTime;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static void setTextureUpdateTime(long fontRendererUpdateTime) {
/*  881 */     textureUpdateTime = fontRendererUpdateTime;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isWeatherEnabled() {
/*  888 */     if (gameSettings == null) {
/*  889 */       return true;
/*      */     }
/*  891 */     return gameSettings.ofWeather;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isSkyEnabled() {
/*  898 */     if (gameSettings == null) {
/*  899 */       return true;
/*      */     }
/*  901 */     return gameSettings.ofSky;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isSunMoonEnabled() {
/*  908 */     if (gameSettings == null) {
/*  909 */       return true;
/*      */     }
/*  911 */     return gameSettings.ofSunMoon;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isStarsEnabled() {
/*  918 */     if (gameSettings == null) {
/*  919 */       return true;
/*      */     }
/*  921 */     return gameSettings.ofStars;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static void sleep(long ms) {
/*      */     try {
/*  930 */       Thread.currentThread(); Thread.sleep(ms);
/*      */     }
/*  932 */     catch (InterruptedException e) {
/*      */       
/*  934 */       e.printStackTrace();
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isTimeDayOnly() {
/*  942 */     if (gameSettings == null) {
/*  943 */       return false;
/*      */     }
/*  945 */     return (gameSettings.ofTime == 1);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isTimeNightOnly() {
/*  952 */     if (gameSettings == null) {
/*  953 */       return false;
/*      */     }
/*  955 */     return (gameSettings.ofTime == 3);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isClearWater() {
/*  962 */     if (gameSettings == null) {
/*  963 */       return false;
/*      */     }
/*  965 */     return gameSettings.ofClearWater;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isDrippingWaterLava() {
/*  972 */     if (gameSettings == null) {
/*  973 */       return false;
/*      */     }
/*  975 */     return gameSettings.ofDrippingWaterLava;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isBetterSnow() {
/*  982 */     if (gameSettings == null) {
/*  983 */       return false;
/*      */     }
/*  985 */     return gameSettings.ofBetterSnow;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static Dimension getFullscreenDimension() {
/*  992 */     if (gameSettings == null) {
/*  993 */       return new Dimension(desktopDisplayMode.getWidth(), desktopDisplayMode.getHeight());
/*      */     }
/*  995 */     String dimStr = gameSettings.ofFullscreenMode;
/*  996 */     if (dimStr.equals("Default")) {
/*  997 */       return new Dimension(desktopDisplayMode.getWidth(), desktopDisplayMode.getHeight());
/*      */     }
/*  999 */     String[] dimStrs = tokenize(dimStr, " x");
/* 1000 */     if (dimStrs.length < 2) {
/* 1001 */       return new Dimension(desktopDisplayMode.getWidth(), desktopDisplayMode.getHeight());
/*      */     }
/* 1003 */     return new Dimension(parseInt(dimStrs[0], -1), parseInt(dimStrs[1], -1));
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int parseInt(String str, int defVal) {
/*      */     try {
/* 1011 */       if (str == null) {
/* 1012 */         return defVal;
/*      */       }
/* 1014 */       return Integer.parseInt(str);
/*      */     }
/* 1016 */     catch (NumberFormatException e) {
/*      */       
/* 1018 */       return defVal;
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static float parseFloat(String str, float defVal) {
/*      */     try {
/* 1027 */       if (str == null) {
/* 1028 */         return defVal;
/*      */       }
/* 1030 */       return Float.parseFloat(str);
/*      */     }
/* 1032 */     catch (NumberFormatException e) {
/*      */       
/* 1034 */       return defVal;
/*      */     } 
/*      */   }
/*      */ 
/*      */   
/*      */   public static String[] tokenize(String str, String delim) {
/* 1040 */     StringTokenizer tok = new StringTokenizer(str, delim);
/* 1041 */     List<String> list = new ArrayList();
/* 1042 */     while (tok.hasMoreTokens()) {
/*      */       
/* 1044 */       String token = tok.nextToken();
/* 1045 */       list.add(token);
/*      */     } 
/* 1047 */     String[] strs = list.<String>toArray(new String[list.size()]);
/* 1048 */     return strs;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static DisplayMode getDesktopDisplayMode() {
/* 1056 */     return desktopDisplayMode;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static void setDesktopDisplayMode(DisplayMode desktopDisplayMode) {
/* 1063 */     Config.desktopDisplayMode = desktopDisplayMode;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static DisplayMode[] getFullscreenDisplayModes() {
/*      */     try {
/* 1072 */       DisplayMode[] modes = Display.getAvailableDisplayModes();
/* 1073 */       List<DisplayMode> list = new ArrayList();
/* 1074 */       for (int i = 0; i < modes.length; i++) {
/*      */         
/* 1076 */         DisplayMode dm = modes[i];
/* 1077 */         if (desktopDisplayMode != null) {
/*      */           
/* 1079 */           if (dm.getBitsPerPixel() != desktopDisplayMode.getBitsPerPixel())
/*      */             continue; 
/* 1081 */           if (dm.getFrequency() != desktopDisplayMode.getFrequency()) {
/*      */             continue;
/*      */           }
/*      */         } 
/* 1085 */         list.add(dm); continue;
/*      */       } 
/* 1087 */       DisplayMode[] fsModes = list.<DisplayMode>toArray(new DisplayMode[list.size()]);
/*      */       
/* 1089 */       Comparator<? super DisplayMode> comp = new Comparator()
/*      */         {
/*      */           public int compare(Object o1, Object o2)
/*      */           {
/* 1093 */             DisplayMode dm1 = (DisplayMode)o1;
/* 1094 */             DisplayMode dm2 = (DisplayMode)o2;
/*      */             
/* 1096 */             if (dm1.getWidth() != dm2.getWidth())
/* 1097 */               return dm2.getWidth() - dm1.getWidth(); 
/* 1098 */             if (dm1.getHeight() != dm2.getHeight()) {
/* 1099 */               return dm2.getHeight() - dm1.getHeight();
/*      */             }
/* 1101 */             return 0;
/*      */           }
/*      */         };
/*      */       
/* 1105 */       Arrays.sort(fsModes, comp);
/*      */       
/* 1107 */       return fsModes;
/*      */     }
/* 1109 */     catch (Exception e) {
/*      */       
/* 1111 */       e.printStackTrace();
/* 1112 */       return new DisplayMode[] { desktopDisplayMode };
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static String[] getFullscreenModes() {
/* 1120 */     DisplayMode[] modes = getFullscreenDisplayModes();
/* 1121 */     String[] names = new String[modes.length];
/* 1122 */     for (int i = 0; i < modes.length; i++) {
/*      */       
/* 1124 */       DisplayMode mode = modes[i];
/* 1125 */       String name = "" + mode.getWidth() + "x" + mode.getHeight();
/* 1126 */       names[i] = name;
/*      */     } 
/* 1128 */     return names;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static DisplayMode getDisplayMode(Dimension dim) throws LWJGLException {
/* 1137 */     DisplayMode[] modes = Display.getAvailableDisplayModes();
/* 1138 */     for (int i = 0; i < modes.length; i++) {
/*      */       
/* 1140 */       DisplayMode dm = modes[i];
/* 1141 */       if (dm.getWidth() != dim.width)
/*      */         continue; 
/* 1143 */       if (dm.getHeight() != dim.height)
/*      */         continue; 
/* 1145 */       if (desktopDisplayMode != null) {
/*      */         
/* 1147 */         if (dm.getBitsPerPixel() != desktopDisplayMode.getBitsPerPixel())
/*      */           continue; 
/* 1149 */         if (dm.getFrequency() != desktopDisplayMode.getFrequency()) {
/*      */           continue;
/*      */         }
/*      */       } 
/* 1153 */       return dm;
/*      */     } 
/*      */     
/* 1156 */     return desktopDisplayMode;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isAnimatedTerrain() {
/* 1163 */     if (gameSettings != null) {
/* 1164 */       return gameSettings.ofAnimatedTerrain;
/*      */     }
/* 1166 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isAnimatedItems() {
/* 1173 */     if (gameSettings != null) {
/* 1174 */       return gameSettings.ofAnimatedItems;
/*      */     }
/* 1176 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isSwampColors() {
/* 1183 */     if (gameSettings != null) {
/* 1184 */       return gameSettings.ofSwampColors;
/*      */     }
/* 1186 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isRandomMobs() {
/* 1193 */     if (gameSettings != null) {
/* 1194 */       return gameSettings.ofRandomMobs;
/*      */     }
/* 1196 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */   
/*      */   public static void checkGlError(String loc) {
/* 1202 */     int i = GL11.glGetError();
/* 1203 */     if (i != 0) {
/*      */       
/* 1205 */       String text = GLU.gluErrorString(i);
/* 1206 */       System.out.println("OpenGlError: " + i + " (" + text + "), at: " + loc);
/*      */     } 
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isSmoothBiomes() {
/* 1214 */     if (gameSettings != null) {
/* 1215 */       return gameSettings.ofSmoothBiomes;
/*      */     }
/* 1217 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isCustomColors() {
/* 1224 */     if (gameSettings != null) {
/* 1225 */       return gameSettings.ofCustomColors;
/*      */     }
/* 1227 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isShowCapes() {
/* 1234 */     if (gameSettings != null) {
/* 1235 */       return gameSettings.ofShowCapes;
/*      */     }
/* 1237 */     return true;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isConnectedTextures() {
/* 1244 */     if (gameSettings != null) {
/* 1245 */       return (gameSettings.ofConnectedTextures != 3);
/*      */     }
/* 1247 */     return false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isNaturalTextures() {
/* 1254 */     if (gameSettings != null) {
/* 1255 */       return gameSettings.ofNaturalTextures;
/*      */     }
/* 1257 */     return false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static boolean isConnectedTexturesFancy() {
/* 1264 */     if (gameSettings != null) {
/* 1265 */       return (gameSettings.ofConnectedTextures == 2);
/*      */     }
/* 1267 */     return false;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static String[] readLines(File file) throws IOException {
/* 1275 */     List<String> list = new ArrayList();
/*      */     
/* 1277 */     FileInputStream fis = new FileInputStream(file);
/* 1278 */     InputStreamReader isr = new InputStreamReader(fis, "ASCII");
/* 1279 */     BufferedReader br = new BufferedReader(isr);
/*      */     
/*      */     while (true) {
/* 1282 */       String line = br.readLine();
/* 1283 */       if (line == null)
/*      */         break; 
/* 1285 */       list.add(line);
/*      */     } 
/*      */ 
/*      */     
/* 1289 */     String[] lines = list.<String>toArray(new String[list.size()]);
/*      */     
/* 1291 */     return lines;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static String readFile(File file) throws IOException {
/* 1299 */     FileInputStream fin = new FileInputStream(file);
/* 1300 */     return readInputStream(fin, "ASCII");
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static String readInputStream(InputStream in) throws IOException {
/* 1308 */     return readInputStream(in, "ASCII");
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static String readInputStream(InputStream in, String encoding) throws IOException {
/* 1316 */     InputStreamReader inr = new InputStreamReader(in, encoding);
/* 1317 */     BufferedReader br = new BufferedReader(inr);
/* 1318 */     StringBuffer sb = new StringBuffer();
/*      */     
/*      */     while (true) {
/* 1321 */       String line = br.readLine();
/* 1322 */       if (line == null)
/*      */         break; 
/* 1324 */       sb.append(line);
/* 1325 */       sb.append("\n");
/*      */     } 
/*      */ 
/*      */     
/* 1329 */     return sb.toString();
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static hu getGameSettings() {
/* 1336 */     return gameSettings;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static String getNewRelease() {
/* 1343 */     return newRelease;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static void setNewRelease(String newRelease) {
/* 1350 */     Config.newRelease = newRelease;
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int compareRelease(String rel1, String rel2) {
/* 1361 */     String[] rels1 = splitRelease(rel1);
/* 1362 */     String[] rels2 = splitRelease(rel2);
/*      */     
/* 1364 */     String branch1 = rels1[0];
/* 1365 */     String branch2 = rels2[0];
/* 1366 */     if (!branch1.equals(branch2)) {
/* 1367 */       return branch1.compareTo(branch2);
/*      */     }
/* 1369 */     int rev1 = parseInt(rels1[1], -1);
/* 1370 */     int rev2 = parseInt(rels2[1], -1);
/* 1371 */     if (rev1 != rev2) {
/* 1372 */       return rev1 - rev2;
/*      */     }
/* 1374 */     String suf1 = rels1[2];
/* 1375 */     String suf2 = rels2[2];
/* 1376 */     return suf1.compareTo(suf2);
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   private static String[] splitRelease(String relStr) {
/* 1385 */     if (relStr == null || relStr.length() <= 0) {
/* 1386 */       return new String[] { "", "", "" };
/*      */     }
/* 1388 */     String branch = relStr.substring(0, 1);
/* 1389 */     if (relStr.length() <= 1) {
/* 1390 */       return new String[] { branch, "", "" };
/*      */     }
/* 1392 */     int pos = 1;
/* 1393 */     while (pos < relStr.length() && Character.isDigit(relStr.charAt(pos)))
/*      */     {
/* 1395 */       pos++;
/*      */     }
/* 1397 */     String revision = relStr.substring(1, pos);
/* 1398 */     if (pos >= relStr.length()) {
/* 1399 */       return new String[] { branch, revision, "" };
/*      */     }
/* 1401 */     String suffix = relStr.substring(pos);
/* 1402 */     return new String[] { branch, revision, suffix };
/*      */   }
/*      */ 
/*      */ 
/*      */ 
/*      */ 
/*      */   
/*      */   public static int intHash(int x) {
/* 1410 */     x = x ^ 0x3D ^ x >> 16;
/* 1411 */     x += x << 3;
/* 1412 */     x ^= x >> 4;
/* 1413 */     x *= 668265261;
/* 1414 */     x ^= x >> 15;
/* 1415 */     return x;
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
/*      */   public static int getRandom(int x, int y, int z, int face) {
/* 1427 */     int rand = intHash(face + 37);
/* 1428 */     rand = intHash(rand + x);
/* 1429 */     rand = intHash(rand + z);
/* 1430 */     rand = intHash(rand + y);
/*      */     
/* 1432 */     return rand;
/*      */   }
/*      */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\Config.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */