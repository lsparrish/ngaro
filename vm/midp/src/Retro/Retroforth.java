package Retro;

/**********************************************************************
 * RETRO 10 J2ME
 * Written by Martin Polak
 * Released into the public domain.
 *
 **********************************************************************/

import javax.microedition.midlet.*;
import javax.microedition.lcdui.*;
import java.io.*;
import javax.microedition.rms.*;

public class Retroforth extends MIDlet {
  public static Retroforth instance;
  public static Framebuffer fb;
  public static final int IMAGE_SIZE = 36000;
  public static boolean fbOn = false;
  public int [] image = new int [IMAGE_SIZE];
  Display mainDisplay;

  public Retroforth() {
    instance = this;
  }
  
  public void startApp() {
    RecordStore rstore = null;
    try {
      rstore = RecordStore.openRecordStore("image", true);
      for (int j = 0; j < Retroforth.IMAGE_SIZE / 1000; j++) {
        byte[] bytes = rstore.getRecord(j + 1);
        DataInputStream s = new DataInputStream(new ByteArrayInputStream(bytes));
        for (int i = j * 1000; i < (j * 1000 + 1000); i++) {
          this.image[i] = s.readInt();
        }
      }
    } catch (Exception e) {
      Img im = new Img(this);
    } finally {
      try {
        rstore.closeRecordStore();
      } catch (Exception e1) { }
    }
    mainDisplay = Display.getDisplay(this);
    fb = new Framebuffer(mainDisplay);
    mainDisplay.setCurrent(new EvalForm(mainDisplay, this));
  }

  public void pauseApp() {
  }

  public void destroyApp(boolean unconditional) {
  }

  public static void quitApp() {
    instance.destroyApp(true);
    instance.notifyDestroyed();
    instance = null;
  }
}
