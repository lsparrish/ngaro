/**********************************************************************
 * Ngaro Virtual Machine
 * Copyright (C) 2008, 2009, Charles Childers
 **********************************************************************
 * This implementation provides an environment very similar to a text
 * console, including the ability to type directly in the browser.
 * It's not flawless, but it's really nice in my opinion.
 *
 * If your browser has problems handling this, or you need a form-based
 * approach, take a look in the 'form-io' subdirectory.
 **********************************************************************/


/**********************************************************************
 * Symbolic constants for each instruction.
 **********************************************************************/
  const VM_NOP = 0;       const VM_LIT = 1;         const VM_DUP = 2;
  const VM_DROP = 3;      const VM_SWAP = 4;        const VM_PUSH = 5;
  const VM_POP = 6;       const VM_CALL = 7;        const VM_JUMP = 8;
  const VM_RETURN = 9;    const VM_GT_JUMP = 10;    const VM_LT_JUMP = 11;
  const VM_NE_JUMP = 12;  const VM_EQ_JUMP = 13;    const VM_FETCH = 14;
  const VM_STORE = 15;    const VM_ADD = 16;        const VM_SUB = 17;
  const VM_MUL = 18;      const VM_DIVMOD = 19;     const VM_AND = 20;
  const VM_OR = 21;       const VM_XOR = 22;        const VM_SHL = 23;
  const VM_SHR = 24;      const VM_ZERO_EXIT = 25;  const VM_INC = 26;
  const VM_DEC = 27;      const VM_IN = 28;         const VM_OUT = 29;
  const VM_WAIT = 30;



/**********************************************************************
 * Some constants useful to us for dealing with the VM settings.
 *
 * If you have performance issues, try modifying CYLES_PER
 **********************************************************************/
  const IMAGE_SIZE  =   32768;         /* Amount of memory to provide */
  const STACK_DEPTH =     100;         /* Depth of the stacks         */
  const CYCLES_PER  =    2000;         /* Instructions to run per     */
                                       /* clock cycle                 */
  const TERM_WIDTH  =      80;         /* Width of emulated terminal  */
  const FB_WIDTH    =     800;         /* Canvas Width                */
  const FB_HEIGHT   =     400;         /* Canvas Height               */



/**********************************************************************
 * Internal registers, flags, and variables
 **********************************************************************/
  var sp = 0, rsp = 0, ip = 0;
  var run = 0;
  var data    = new Array(STACK_DEPTH);
  var address = new Array(STACK_DEPTH);
  var ports   = new Array(1024);
  var image   = new Array(IMAGE_SIZE);
  var interval;
  var devOutput = "";
  var output = document.getElementById("output");
  var lastKey = " ";
  var width = 0;
  var mx, my, mb;
  var fbraw, fb;

function init_fb()
{
 fbraw = document.getElementById("framebuffer");
 fb = fbraw.getContext("2d");
}

function video_color(c)
{
  if (c == 0)
    fb.fillStyle = "black";
  if (c == 1)
    fb.fillStyle = "darkblue";
  if (c == 2)
    fb.fillStyle = "darkgreen";
  if (c == 3)
    fb.fillStyle = "darkcyan";
  if (c == 4)
    fb.fillStyle = "darkred";
  if (c == 5)
    fb.fillStyle = "purple";
  if (c == 6)
    fb.fillStyle = "brown";
  if (c == 7)
    fb.fillStyle = "darkgray";
  if (c == 8)
    fb.fillStyle = "gray";
  if (c == 9)
    fb.fillStyle = "blue";
  if (c == 10)
    fb.fillStyle = "green";
  if (c == 11)
    fb.fillStyle = "cyan";
  if (c == 12)
    fb.fillStyle = "red";
  if (c == 13)
    fb.fillStyle = "magenta";
  if (c == 14)
    fb.fillStyle = "yellow";
  if (c == 15)
    fb.fillStyle = "white";
  if (c < 0 || c > 15)
    fb.fillStyle = "black";
}

function video_pixel(x, y)
{
  fb.fillRect(x, y, 2, 2);
}

function video_rect(x, y, w, h)
{
  fb.strokeRect(x, y, w, h);
}

function video_fillRect(x, y, w, h)
{
  fb.fillRect(x, y, w, h);
}

function video_hline(x, y, w)
{
  fb.fillRect(x, y, w, 2);
}

function video_vline(x, y, h)
{
  fb.fillRect(x, y, 2, h);
}

function video_circle(x, y, w)
{
  fb.beginPath();
  fb.arc(x, y, w, 0, Math.PI*2, true);
  fb.closePath();
  fb.stroke();
}

function video_fillCircle(x, y, w)
{
  fb.beginPath();
  fb.arc(x, y, w, 0, Math.PI*2, true);
  fb.closePath();
  fb.fill();
}



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
  width = 0;
  mx = 0; my = 0; mb = 0;
}


/**********************************************************************
 * This is a nice little hack to read key presses and store them
 * somewhere for later use. It's critical for the console emulation.
 **********************************************************************/
function readKeyboard(e)
{
  var uni = e.keyCode ? e.keyCode : e.charCode;
  lastKey = uni;
  if (uni == 8)
    return false;
}

function readMouse(e)
{
  mx = e.pageX;
  my = e.pageY;
  return true;
}

function setButton(e)
{
  mb = 1;
  return true;
}

function releaseButton(e)
{
  mb = 0;
  return true;
}



/**********************************************************************
 * handleDevices()
 * This handles the simulated hardware devices.
 **********************************************************************/
function handleDevices()
{
  if (ports[0] == 1)
    return;

  /* Input */
  ports[0] = 1;
  ports[1] = lastKey;
  lastKey = 0;


  /* Output */
  if (ports[2] == 1)
  {
    var ch = String.fromCharCode(data[sp]);

    /* Remap select characters to HTML */
    switch (data[sp])
    {
      case 10: ch = "<br>\n"; width = 0; break;
      case 32: ch = "&nbsp;"; break;
      case 38: ch = "&amp;";  break;
      case 60: ch = "&lt;";   break;
      case 62: ch = "&gt;";   break;
    }

    /* Display the character */
    if (data[sp] < 0)
    {
      clearDisplay();
    }
    else
    {
      devOutput += ch;
      width++;
      if (width > TERM_WIDTH)
      {
        width = 0;
        devOutput += "<br>\n";
      }
    }

    if (data[sp] == 8)
      devOutput = devOutput.substr(0, devOutput.length - 2);

    sp--;
    ports[2] = 0;
    ports[0] = 1;
  }

  /* Save */
  if (ports[4] == 1)
  {
    ports[4] = 0;
    ports[0] = 1;
    ajax.post('save.cgi', ajax.msg, 'csv="'+image+'"');
  }

  /* Capabilities */
  if (ports[5] == -1)
  {
    ports[5] = IMAGE_SIZE;
    ports[0] = 1;
  }
  if (ports[5] == -2)
  {
    ports[5] = -1;
    ports[0] = 1;
  }
  if (ports[5] == -3)
  {
    ports[5] = FB_WIDTH;
    ports[0] = 1;
  }
  if (ports[5] == -4)
  {
    ports[5] = FB_HEIGHT;
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
  if (ports[5] == -7)
  {
    ports[5] = -1;
    ports[0] = 1;
  }

  if (ports[6] == 1)
  {
    video_color(data[sp]); sp--;
    ports[6] = 0;
    ports[0] = 1;
  }
  if (ports[6] == 2)
  {
    var x, y;
    y = data[sp]; sp--;
    x = data[sp]; sp--;
    video_pixel(x, y);
    ports[6] = 0;
    ports[0] = 1;
  }
  if (ports[6] == 3)
  {
    var x, y, h, w;
    w = data[sp]; sp--;
    h = data[sp]; sp--;
    y = data[sp]; sp--;
    x = data[sp]; sp--;
    video_rect(x, y, h, w);
    ports[6] = 0;
    ports[0] = 1;
  }
  if (ports[6] == 4)
  {
    var x, y, h, w;
    w = data[sp]; sp--;
    h = data[sp]; sp--;
    y = data[sp]; sp--;
    x = data[sp]; sp--;
    video_fillRect(x, y, h, w);
    ports[6] = 0;
    ports[0] = 1;
  }
  if (ports[6] == 5)
  {
    var x, y, h;
    h = data[sp]; sp--;
    y = data[sp]; sp--;
    x = data[sp]; sp--;
    video_vline(x, y, h);
    ports[6] = 0;
    ports[0] = 1;
  }
  if (ports[6] == 6)
  {
    var x, y, w;
    w = data[sp]; sp--;
    y = data[sp]; sp--;
    x = data[sp]; sp--;
    video_hline(x, y, w);
    ports[6] = 0;
    ports[0] = 1;
  }
  if (ports[6] == 7)
  {
    var x, y, w;
    w = data[sp]; sp--;
    y = data[sp]; sp--;
    x = data[sp]; sp--;
    video_circle(x, y, w);
    ports[6] = 0;
    ports[0] = 1;
  }
  if (ports[6] == 8)
  {
    var x, y, w;
    w = data[sp]; sp--;
    y = data[sp]; sp--;
    x = data[sp]; sp--;
    video_fillCircle(x, y, w);
    ports[6] = 0;
    ports[0] = 1;
  }

  if (ports[7] == 1)
  {
    sp++; data[sp] = mx;
    sp++; data[sp] = my;
    ports[7] = 0;
    ports[0] = 1;
  }
  if (ports[7] == 2)
  {
    sp++; data[sp] = mb;
    ports[7] = 0;
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
  document.getElementById('output').innerHTML = devOutput;
  width = 0;
  fb.clearRect(0, 0, 800, 400);
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
      if (run == 1)
        document.getElementById('output').innerHTML = "Fatal Error.<br>Press COMMAND+R or CTRL+R to reload";
      ip = IMAGE_SIZE;
      run = 0;
  }
}




/**********************************************************************
 * startVM()
 **********************************************************************/
function startVM()
{
  interval = setInterval("processImage()", 10);
  run = 1;
}

function stopVM()
{
  clearInterval(interval);
  interval = null;
  run = 0;
}


/**********************************************************************
 * checkStack()
 **********************************************************************/
function checkStack()
{
  if (sp < 0 || rsp < 0)
  {
    document.getElementById('output').innerHTML = "Stack Underflow.<br>Press COMMAND+R or CTRL+R to reload";
    ip = IMAGE_SIZE;
    run = 0;
  }
  if (sp > STACK_DEPTH || rsp > STACK_DEPTH)
  {
    document.getElementById('output').innerHTML = "Stack Overflow.<br>Press COMMAND+R or CTRL+R to reload";
    ip = IMAGE_SIZE;
    run = 0;
  }
}


/**********************************************************************
 * processImage()
 * This runs through the image, calling processOpcode() for each
 * instruction.
 *
 * For performance reasons up to CYCLES_PER instructions will be
 * executed by this code per call.
 **********************************************************************/
function processImage()
{
  var a;

  for (a = CYCLES_PER; a > 0 && run == 1; a--)
  {
    processOpcode();
    checkStack();
    ip++;

    /* Update the display */
    if (ports[3] == 0)
    {
      ports[3] = 1;
      document.getElementById('output').innerHTML = devOutput;
    }
  }
}

/* Enable our keyboard handler */
document.onkeypress = readKeyboard;
document.onmousemove = readMouse;
document.onmousedown = setButton;
document.onmouseup = releaseButton;
