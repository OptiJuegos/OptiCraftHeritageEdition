/*    */ import java.io.InputStream;
/*    */ import java.net.HttpURLConnection;
/*    */ import java.net.URL;
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ 
/*    */ public class VersionCheckThread
/*    */   extends Thread
/*    */ {
/*    */   public void run() {
/* 20 */     HttpURLConnection conn = null;
/*    */     
/*    */     try {
/* 23 */       Config.dbg("Checking for new version");
/* 24 */       URL url = new URL("http://optifine.net/version/1.2.5/HD.txt");
/*    */       
/* 26 */       conn = (HttpURLConnection)url.openConnection();
/* 27 */       conn.setDoInput(true);
/* 28 */       conn.setDoOutput(false);
/* 29 */       conn.connect();
/*    */       
/*    */       try {
/* 32 */         InputStream in = conn.getInputStream();
/* 33 */         String verStr = Config.readInputStream(in);
/* 34 */         in.close();
/*    */         
/* 36 */         String[] verLines = Config.tokenize(verStr, "\n\r");
/* 37 */         if (verLines.length < 1) {
/*    */           return;
/*    */         }
/* 40 */         String newVer = verLines[0];
/* 41 */         Config.dbg("Version found: " + newVer);
/*    */         
/* 43 */         if (Config.compareRelease(newVer, "C6") <= 0) {
/*    */           return;
/*    */         }
/* 46 */         Config.setNewRelease(newVer);
/*    */       }
/*    */       finally {
/*    */         
/* 50 */         if (conn != null) {
/* 51 */           conn.disconnect();
/*    */         }
/*    */       } 
/* 54 */     } catch (Exception e) {
/*    */       
/* 56 */       e.printStackTrace();
/*    */     } 
/*    */   }
/*    */ }


/* Location:              D:\MC\OptiFine_1.2.5_HD_C6.zip!\VersionCheckThread.class
 * Java compiler version: 5 (49.0)
 * JD-Core Version:       1.1.3
 */