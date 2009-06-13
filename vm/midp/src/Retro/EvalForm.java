package Retro;

/**********************************************************************
 * RETRO 10 J2ME
 * Written by Martin Polak
 * Released into the public domain.
 *
 **********************************************************************/

import javax.microedition.lcdui.*;

public class EvalForm extends Form implements CommandListener, Runnable {
  Display mainDisplay;
  TextField tfIn;
  StringBuffer buffer = new StringBuffer(" ");
  StringItem siOut;
  Ngaro ngarovm;
  boolean saving = false;

  public EvalForm(Display mainDisplay) {
    super ("RETRO");
    this.mainDisplay = mainDisplay;
    this.append(tfIn = new TextField("", "", 1024, TextField.ANY));
    addCommand(new Command("Enter", Command.OK, 1));
    addCommand(new Command("Quit", Command.BACK, 3));
    this.append(siOut = new StringItem("", ""));
    ngarovm = new Ngaro(this);
    ngarovm.initVM();
    ngarovm.runImage();
    Thread T = new Thread(this);
    T.start();
    setCommandListener(this);
  }

  public void commandAction(Command command, Displayable displayable) {
    if(command.getCommandType() == Command.BACK) {
      Retroforth.quitApp();
    }
    if(command.getCommandType() == Command.OK) {
      buffer = new StringBuffer(tfIn.getString());
      buffer.append("   ");
      tfIn.setString("");
    }
  }

  public void run() {
    while(true) {
      if (saving == false) ngarovm.startVM();
    }
  }
}
