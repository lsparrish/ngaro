/* Touch Events */
/* Allows the canvas, etc to work on Safari Mobile */
function readTouch(e)
{
  mx = e.touches[0].pageX;
  my = e.touches[0].pageY;
  mb = 1;
  return true;
}

function releaseTouch(e)
{
  mb = 0;
  return true;
}


function OnTouchMove(event)
{
  mx = e.touches[0].pageX;
  my = e.touches[0].pageY;
  mb = 1;
  return true;
}
document.ontouchstart = readTouch;
document.ontouchend = releaseTouch;
document.ontouchmove = OnTouchMove;
