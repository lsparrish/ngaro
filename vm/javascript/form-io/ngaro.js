/**********************************************************************
 * Ngaro Virtual Machine
 * Written by Charles Childers
 * This code is gifted to the public domain.
 **********************************************************************
 * This is mostly a straightforward port of the original C
 * implementation with a crude approximation of a text console
 * interface.
 **********************************************************************/


/**********************************************************************
 * Symbolic constants for each instruction. In C, I'd use an enum, but
 * I don't know how to do this properly in JavaScript.
 **********************************************************************/
  const VM_NOP = 0;   const VM_LIT = 1;   const VM_DUP = 2;
  const VM_DROP = 3;  const VM_SWAP = 4;  const VM_PUSH = 5;
  const VM_POP = 6;   const VM_CALL = 7;  const VM_JUMP = 8;
  const VM_RETURN = 9;const VM_GT_JUMP = 10;  const VM_LT_JUMP = 11;
  const VM_NE_JUMP = 12;  const VM_EQ_JUMP = 13;  const VM_FETCH = 14;
  const VM_STORE = 15;const VM_ADD = 16;  const VM_SUB = 17;
  const VM_MUL = 18;  const VM_DIVMOD = 19;   const VM_AND = 20;
  const VM_OR = 21;   const VM_XOR = 22;  const VM_SHL = 23;
  const VM_SHR = 24;  const VM_ZERO_EXIT = 25;const VM_INC = 26;
  const VM_DEC = 27;  const VM_IN = 28;   const VM_OUT = 29;
  const VM_WAIT = 30;



/**********************************************************************
 * Some constants useful to us for dealing with the VM settings.
 **********************************************************************/
  const IMAGE_SIZE  = 5000000
  const STACK_DEPTH =     100
  const CYCLES_PER  =     100



/**********************************************************************
 * Internal registers
 **********************************************************************/
  var sp = 0, rsp = 0, ip = 0;
  var run = 0;
  var data = new Array(STACK_DEPTH);
  var address = new Array(STACK_DEPTH);
  var ports = new Array(1024);
  var image = new Array(IMAGE_SIZE);
  var interval;
  var devOutput = "";


/**********************************************************************
 * initVM()
 * Initialize the Ngaro VM
 **********************************************************************/
function initVM()
{
  ip  = 0;
  sp  = 0;
  rsp = 0;
  ports[0] = 0;
  data[0] = 0;
}



/**********************************************************************
 * handleDevices()
 * This handles the simulated hardware devices. Specifically, it tries
 * to approximate a console interface using HTML forms. It's not great,
 * but does work.
 **********************************************************************/
function handleDevices()
{
  if (ports[0] == 1)
    return;

  /* Input */
  if (ports[0] == 0 && ports[1] == 1)
  {
    ports[0] = 1;
    ports[1] = document.forms[0].code.value.charCodeAt(0);
    document.forms[0].code.value = document.forms[0].code.value.substring(1, document.forms[0].code.value.length);
  }

  /* Output */
  if (ports[2] == 1)
  {
    var ch = String.fromCharCode(data[sp]);
    devOutput += ch;
    if (data[sp] == -1)
      clearDisplay();
    sp--;
    ports[2] = 0;
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



/**********************************************************************
 * clearDisplay()
 * This clears the display.
 **********************************************************************/
function clearDisplay()
{
  devOutput = "";
  document.forms[0].output.value = devOutput;
}



/**********************************************************************
 * processOpcode()
 * This is the main piece of code in Ngaro. It looks up each opcode and
 * carries out the proper action. This is mostly a straight port of the
 * C implementation, so some optimization opportunities will probably
 * exist.
 **********************************************************************/
function processOpcode()
{
  var x, y, z, op;
  op = image[ip];
  switch(op)
  {
    case VM_NOP:
      break;
    case VM_LIT:
      sp++; ip++; data[sp] = image[ip];
      break;
    case VM_DUP:
      sp++; data[sp] = data[sp-1];
      break;
    case VM_DROP:
      data[sp] = 0; sp--;
      break;
    case VM_SWAP:
      x = data[sp];
      y = data[sp-1];
      data[sp] = y;
      data[sp-1] = x;
      break;
    case VM_PUSH:
      rsp++;
      address[rsp] = data[sp];
      sp--;
      break;
    case VM_POP:
      sp++;
      data[sp] = address[rsp];
      rsp--;
      break;
    case VM_CALL:
      ip++; rsp++;
      address[rsp] = ip++;
      ip = image[ip-1] - 1;
      break;
    case VM_JUMP:
      ip++;
      ip = image[ip] - 1;
      break;
    case VM_RETURN:
      ip = address[rsp]; rsp--;
      break;
    case VM_GT_JUMP:
      ip++
      if (data[sp-1] > data[sp])
        ip = image[ip] - 1;
      sp = sp - 2;
      break;
    case VM_LT_JUMP:
      ip++
      if (data[sp-1] < data[sp])
        ip = image[ip] - 1;
      sp = sp - 2;
      break;
    case VM_NE_JUMP:
      ip++
      if (data[sp-1] != data[sp])
        ip = image[ip] - 1;
      sp = sp - 2;
      break;
    case VM_EQ_JUMP:
      ip++
      if (data[sp-1] == data[sp])
        ip = image[ip] - 1;
      sp = sp - 2;
      break;
    case VM_FETCH:
      x = data[sp];
      data[sp] = image[x];
      break;
    case VM_STORE:
      image[data[sp]] = data[sp-1];
      sp = sp - 2;
      break;
    case VM_ADD:
      data[sp-1] += data[sp]; data[sp] = 0; sp--;
      break;
    case VM_SUB:
      data[sp-1] -= data[sp]; data[sp] = 0; sp--;
      break;
    case VM_MUL:
      data[sp-1] *= data[sp]; data[sp] = 0; sp--;
      break;
    case VM_DIVMOD:
      x = data[sp];
      y = data[sp-1];
      data[sp] = Math.floor(y / x);
      data[sp-1] = y % x;
      break;
    case VM_AND:
      x = data[sp];
      y = data[sp-1];
      sp--;
      data[sp] = x & y;
      break;
    case VM_OR:
      x = data[sp];
      y = data[sp-1];
      sp--;
      data[sp] = x | y;
      break;
    case VM_XOR:
      x = data[sp];
      y = data[sp-1];
      sp--;
      data[sp] = x ^ y;
      break;
    case VM_SHL:
      x = data[sp];
      y = data[sp-1];
      sp--;
      data[sp] = y << x;
      break;
    case VM_SHR:
      x = data[sp];
      y = data[sp-1];
      sp--;
      data[sp] = y >>= x;
      break;
    case VM_ZERO_EXIT:
      if (data[sp] == 0)
      {
        sp--;
        ip = address[rsp]; rsp--;
      }
      break;
    case VM_INC:
      data[sp]++;
      break;
    case VM_DEC:
      data[sp]--;
      break;
    case VM_IN:
      x = data[sp];
      data[sp] = ports[x];
      ports[x] = 0;
      break;
    case VM_OUT:
      ports[0] = 0;
      ports[data[sp]] = data[sp-1];
      sp = sp - 2;
      break;
    case VM_WAIT:
      handleDevices();
      break;
    default:
      document.forms[0].trace.value += 'ERROR: IP: ' + ip + ' op: ' + image[ip] + '\n';
      ip = IMAGE_SIZE;
      run = 0;
  }
}



/**********************************************************************
 * runImage()
 **********************************************************************/
function runImage()
{
  run = 1;
  document.forms[0].code.value += '  ';
}



/**********************************************************************
 * startVM()
 **********************************************************************/
function startVM()
{
  run = 1;
  document.forms[0].code.value += '  ';
  interval = setInterval("processImage()", 10);
}


/**********************************************************************
 * processImage()
 * This runs through the image, calling processOpcode() for each
 * instruction. It will only run if:
 *
 *  a) There is input in the input buffer
 *  b) An internal 'run' flag is set
 *
 * For performance reasons up to CYCLES_PER instructions will be
 * executed by this code per call.
 **********************************************************************/
function processImage()
{
  var a;
  if (document.forms[0].code.value.length <= 0 || run == 0)
  {
    run = 0;
    return;
  }

  for (a = CYCLES_PER; a > 0; a--)
  {
    processOpcode();
    ip++;
    if (ports[3] == 0)
    {
      ports[3] = 1;
      document.forms[0].output.value = devOutput;
    }
  }
}
