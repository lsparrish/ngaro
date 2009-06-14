package Retro;

/**********************************************************************
 * RETRO 10 J2ME
 * Written by Martin Polak
 * Released into the public domain.
 *
 **********************************************************************/

import javax.microedition.lcdui.*;

public class Framebuffer extends Canvas {
  Display mainDisplay;
  public int fw, fh;

  public Framebuffer(Display mainDisplay) {
    this.mainDisplay = mainDisplay;
    setFullScreenMode(true);
    fw = getWidth();
    fh = getHeight();
  }

  protected void paint(Graphics g) {
  }

  protected void keyPressed(int keyCode) {
  }
}
