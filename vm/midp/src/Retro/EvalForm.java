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
  Retroforth root;
  boolean saving = false;

  public EvalForm(Display mainDisplay, Retroforth root) {
    super ("RETRO");
    this.mainDisplay = mainDisplay;
    this.root = root;
    this.append(siOut = new StringItem("", ""));
    this.append(tfIn = new TextField("", "", 1024, TextField.ANY));
    addCommand(new Command("Enter", Command.OK, 1));
    ngarovm = new Ngaro(this, root);
    ngarovm.initVM();
    ngarovm.runImage();
    Thread T = new Thread(this);
    T.start();
    setCommandListener(this);
  }

  public void commandAction(Command command, Displayable displayable) {
    if(command.getCommandType() == Command.OK) {
      buffer = new StringBuffer(tfIn.getString());
      buffer.append("    ");
      tfIn.setString("");
    }
}

  public void run() {
    while(true) {
      if (saving == false) ngarovm.startVM();
    }
  }
}
