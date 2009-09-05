/******************************************************
 * Ngaro for Mono / .NET
 *
 * Copyright (c) 2009, Simon Waite and Charles Childers
 *
 * Please compile with `gmcs` as `mcs` seems to have a
 * simple Console.in implementation.
 ******************************************************/

using System;
using System.IO;

namespace Retro.Forth
{
  public class VM
  {
    int sp, rsp, ip;
    int[] data;
    int[] address;
    int[] ports;
    int[] memory;

    enum OpCodes
    {
      VM_NOP,       VM_LIT,       VM_DUP,
      VM_DROP,      VM_SWAP,      VM_PUSH,
      VM_POP,       VM_CALL,      VM_JUMP,
      VM_RETURN,    VM_GT_JUMP,   VM_LT_JUMP,
      VM_NE_JUMP,   VM_EQ_JUMP,   VM_FETCH,
      VM_STORE,     VM_ADD,       VM_SUB,
      VM_MUL,       VM_DIVMOD,    VM_AND,
      VM_OR,        VM_XOR,       VM_SHL,
      VM_SHR,       VM_ZERO_EXIT, VM_INC,
      VM_DEC,       VM_IN,        VM_OUT,
      VM_WAIT
    }


    public VM()
    {
      sp = 0;
      rsp = 0;
      ip = 0;
      data    = new int[100];
      address = new int[100];
      ports   = new int[1024];
      memory  = new int[5000000];
      Retro();
    }


    public int switchEndian(int value)
    {
      int b1 = (value >>  0) & 0xff;
      int b2 = (value >>  8) & 0xff;
      int b3 = (value >> 16) & 0xff;
      int b4 = (value >> 24) & 0xff;
      return b1 << 24 | b2 << 16 | b3 << 8 | b4 << 0;
    }


    public void loadImage()
    {
      int i;
      if (!File.Exists("retroImage"))
        return;

      BinaryReader binReader = new BinaryReader(File.Open("retroImage", FileMode.Open));
      try
      {
        // If the file is not empty,
        // read the application settings.
        // First read 4 bytes into a buffer to
        // determine if the file is empty.
        byte[] testArray = new byte[3];
        int count = binReader.Read(testArray, 0, 3);

        if (count != 0)
        {
          // Reset the position in the stream to zero.
          binReader.BaseStream.Seek(0, SeekOrigin.Begin);

          i = 0;
          while (i < 5000000)
          {
            memory[i] = binReader.ReadInt32(); i++;
          }
        }
      }

      // If the end of the stream is reached before reading
      // the four data values, ignore the error and use the
      // default settings for the remaining values.
      catch(EndOfStreamException e)
      {
        Console.WriteLine("{0} caught and ignored. " + "Using default values.", e.GetType().Name);
      }
      finally
      {
        binReader.Close();
      }
    }


    public void saveImage()
    {
      int i;
      BinaryWriter binWriter = new BinaryWriter(File.Open("retroImage", FileMode.Create));
      try
      {
        i = 0;
        while (i < 5000000)
        {
          binWriter.Write(memory[i]); i++;
        }
      }

      // If the end of the stream is reached before reading
      // the four data values, ignore the error and use the
      // default settings for the remaining values.
      catch(EndOfStreamException e)
      {
        Console.WriteLine("{0} caught and ignored. " + "Using default values.", e.GetType().Name);
      }
      finally
      {
        binWriter.Close();
      }
    }


    public void Retro()
    {
      loadImage();

      if (memory[0] == 0)
      {
// end top