<?
$data = file_get_contents("image.csv");

echo $data;
$mem = explode(',', $data);
$fp = fopen('retroImage.js', 'wb');
$i = 0;
fwrite($fp, "function loadImage() { var a; for (a = 0; a < 32768; a++) image[a]=0;\n");

foreach($mem as $opcode)
{
  if ((int)$opcode != 0)
    fwrite($fp, "image[$i]=$opcode;\n");
  $i++;
}
fwrite($fp, "\n}");
fclose($fp);
?>

