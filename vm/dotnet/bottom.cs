// start of bottom
      }
    }


  public void HandleDevices()
  {
    if (ports[0] == 1)
      return;

    if (ports[0] == 0 && ports[1] == 1)
    {
      ConsoleKeyInfo cki = Console.ReadKey();
      int a = (int)cki.KeyChar;
      if (cki.Key == ConsoleKey.Backspace)
      {
        a = 8;
        Console.Write('a');
      }
      if (a >= 32)
        Console.Write((char)8);
      ports[1] = a;
      ports[0] = 1;
    }
    if (ports[2] == 1)
    {
      char c = (char)data[sp];
      if (data[sp] < 0)
        Console.Clear();
      else
        Console.Write(c);
      sp--;
      ports[2] = 0;
      ports[0] = 1;
    }
    if (ports[4] == 1)
    {
      saveImage();
      ports[4] = 0;
      ports[0] = 1;
    }
    /* Capabilities */
    if (ports[5] == -1)
    {
      ports[5] = 5000000;
      ports[0] = 1;
    }
    if (ports[5] == -2 || ports[5] == -3 || ports[5] == -4)
    {
      ports[5] = 0;
      ports[0] = 1;
    }
    if (ports[5] == -5)
    {
      ports[5] = sp;
      ports[0] = 1;
    }
    if (ports[5] == -6)
    {
      ports[5] = rsp;
      ports[0] = 1;
    }
  }


  public void Process()
  {
    int x, y;

    switch((OpCodes)memory[ip])
    {
    case OpCodes.VM_NOP:
      break;
    case OpCodes.VM_LIT:
      sp++; ip++; data[sp] = memory[ip];
      break;
    case OpCodes.VM_DUP:
      sp++; data[sp] = data[sp-1];
      break;
    case OpCodes.VM_DROP:
      data[sp] = 0; sp--;
      break;
    case OpCodes.VM_SWAP:
      x = data[sp];
      y = data[sp-1];
      data[sp] = y;
      data[sp-1] = x;
      break;
    case OpCodes.VM_PUSH:
      rsp++;
      address[rsp] = data[sp];
      sp--;
      break;
    case OpCodes.VM_POP:
      sp++;
      data[sp] = address[rsp];
      rsp--;
      break;
    case OpCodes.VM_CALL:
      ip++; rsp++;
      address[rsp] = ip++;
      ip = memory[ip-1] - 1;
      break;
    case OpCodes.VM_JUMP:
      ip++;
      ip = memory[ip] - 1;
      break;
    case OpCodes.VM_RETURN:
      ip = address[rsp]; rsp--;
      break;
    case OpCodes.VM_GT_JUMP:
      ip++;
      if (data[sp-1] > data[sp])
        ip = memory[ip] - 1;
      sp = sp - 2;
      break;
    case OpCodes.VM_LT_JUMP:
      ip++;
      if (data[sp-1] < data[sp])
        ip = memory[ip] - 1;
      sp = sp - 2;
      break;
    case OpCodes.VM_NE_JUMP:
      ip++;
      if (data[sp-1] != data[sp])
        ip = memory[ip] - 1;
      sp = sp - 2;
      break;
    case OpCodes.VM_EQ_JUMP:
      ip++;
      if (data[sp-1] == data[sp])
        ip = memory[ip] - 1;
      sp = sp - 2;
      break;
    case OpCodes.VM_FETCH:
      x = data[sp];
      data[sp] = memory[x];
      break;
    case OpCodes.VM_STORE:
      memory[data[sp]] = data[sp-1];
      sp = sp - 2;
      break;
    case OpCodes.VM_ADD:
      data[sp-1] += data[sp]; data[sp] = 0; sp--;
      break;
    case OpCodes.VM_SUB:
      data[sp-1] -= data[sp]; data[sp] = 0; sp--;
      break;
    case OpCodes.VM_MUL:
      data[sp-1] *= data[sp]; data[sp] = 0; sp--;
      break;
    case OpCodes.VM_DIVMOD:
      x = data[sp];
      y = data[sp-1];
      data[sp] = y / x;
      data[sp-1] = y % x;
      break;
    case OpCodes.VM_AND:
      x = data[sp];
      y = data[sp-1];
      sp--;
      data[sp] = x & y;
      break;
    case OpCodes.VM_OR:
      x = data[sp];
      y = data[sp-1];
      sp--;
      data[sp] = x | y;
      break;
    case OpCodes.VM_XOR:
      x = data[sp];
      y = data[sp-1];
      sp--;
      data[sp] = x ^ y;
      break;
    case OpCodes.VM_SHL:
      x = data[sp];
      y = data[sp-1];
      sp--;
      data[sp] = y << x;
      break;
    case OpCodes.VM_SHR:
      x = data[sp];
      y = data[sp-1];
      sp--;
      data[sp] = y >>= x;
      break;
    case OpCodes.VM_ZERO_EXIT:
      if (data[sp] == 0)
      {
        sp--;
        ip = address[rsp]; rsp--;
      }
      break;
    case OpCodes.VM_INC:
      data[sp]++;
      break;
    case OpCodes.VM_DEC:
      data[sp]--;
      break;
    case OpCodes.VM_IN:
      x = data[sp];
      data[sp] = ports[x];
      ports[x] = 0;
      break;
    case OpCodes.VM_OUT:
      ports[0] = 0;
      ports[data[sp]] = data[sp-1];
      sp = sp - 2;
      break;
    case OpCodes.VM_WAIT:
      HandleDevices();
      break;
    default:
      ip = 5000000;
      break;
    }
  }

  public void Execute()
  {
    for (; ip < 5000000; ip++)
       Process();
  }

  public static void Main(string [] args)
  {
    VM vm = new VM();

    foreach(string arg in args)
    {
      if (arg == "--endian")
      {
        for (int i = 0; i < 5000000; i++)
          vm.memory[i] = vm.switchEndian(vm.memory[i]);
      }
      if (arg == "--about")
      {
        Console.Write("Retro Language  [VM: C#, .NET]\n\n");
        Environment.Exit(0);
      }
    }

    vm.Execute();
  }
}
}// end namespace
