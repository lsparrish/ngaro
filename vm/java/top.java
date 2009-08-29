/*
 * Copyright (c) 2009, Simon Waite and Charles Childers
 * Based on the C# implementation
*/

import java.*;
import java.io.*;


/**
 * @author Simon Waite
 * @author Charles Childers
 *
 * The retro class contains the Ngaro VM and an initial
 * image file for the Retro language.
 */
public class retro {
  int sp, rsp, ip;
  int data[], address[], ports[];
  int memory[];

  public static final int VM_NOP = 0;       public static final int VM_LIT = 1;       public static final int VM_DUP = 2;
  public static final int VM_DROP = 3;      public static final int VM_SWAP = 4;      public static final int VM_PUSH = 5;
  public static final int VM_POP = 6;       public static final int VM_CALL = 7;      public static final int VM_JUMP = 8;
  public static final int VM_RETURN = 9;    public static final int VM_GT_JUMP = 10;  public static final int VM_LT_JUMP = 11;
  public static final int VM_NE_JUMP = 12;  public static final int VM_EQ_JUMP = 13;  public static final int VM_FETCH = 14;
  public static final int VM_STORE = 15;    public static final int VM_ADD = 16;      public static final int VM_SUB = 17;
  public static final int VM_MUL = 18;      public static final int VM_DIVMOD = 19;   public static final int VM_AND = 20;
  public static final int VM_OR = 21;       public static final int VM_XOR = 22;      public static final int VM_SHL = 23;
  public static final int VM_SHR = 24;      public static final int VM_ZERO_EXIT = 25;public static final int VM_INC = 26;
  public static final int VM_DEC = 27;      public static final int VM_IN = 28;       public static final int VM_OUT = 29;
  public static final int VM_WAIT = 30;


 /**
  * Returns the value in the opposite endian
  *
  * @return int
  */
  public int switchEndian(int value)
  {
    int b1 = (value >>  0) & 0xff;
    int b2 = (value >>  8) & 0xff;
    int b3 = (value >> 16) & 0xff;
    int b4 = (value >> 24) & 0xff;
    return b1 << 24 | b2 << 16 | b3 << 8 | b4 << 0;
  }


 /**
  * Load an image file. Will do nothing if the file does
  * not exist.
  */
  public void loadImage(String name)
  {
    int i;
    RandomAccessFile in = null;

    boolean exists = (new File(name)).exists();
    if (exists)
    {
      try
      {
        in = new RandomAccessFile(name, "r");
        i = 0;

        while (i < 5000000)
        {
          memory[i] = in.readInt(); i++;
        }
        if (in != null)
          in.close();
      }
      catch(Exception e) { System.out.println(e); }
    }
    else
    {
      memory[0] = 0;
    }
  }


 /**
  * Save the current image to a file.
  */
  public void saveImage(String name)
  {
    int i;

    try
    {
      RandomAccessFile out = new RandomAccessFile(name, "rw");
      i = 0;

      while (i < 5000000)
      {
        out.writeInt(memory[i]); i++;
      }
      if (out != null)
        out.close();
    }
    catch(Exception e) { System.out.println(e); }
  }


 /**
  * Generic constructor
  */
  public void retro()
  {
  }


 /**
  * Setup and restore the terminal for key breaking and
  * no echoing of the characters being read. This *requires*
  * a Unix-like host. This is one place .NET is better than
  * Java: it at least tries to provide real console access.
  */
  public static void set_tty(boolean on)
  {
    try
    {
      String[] cmd = { "/bin/sh", "-c", "/bin/stty " + (on ? "-echo -icanon min 1" : "echo") + " < /dev/tty" };
      Process p = Runtime.getRuntime().exec(cmd);
      p.waitFor();
    }
    catch (IOException e) { }
    catch (InterruptedException e) { }
  }


 /**
  * Attempt to load the initial image. This tries to load a
  * file (retroImage). If that fails, it uses a built-in one.
  */
  public void initial_image()
  {
    int i;
    for(i = 0; i < 5000000; i++)
      memory[i] = 0;

    loadImage("retroImage");
    if (memory[0] == 0)
    {
