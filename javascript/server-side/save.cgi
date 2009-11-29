#! /usr/bin/env perl
use CGI qw/:standard/;
CGI::initialize_globals();

$| = 1;

print header;

$data = param('csv') || '(No input)';
$data =~ s/^"(.*)"$/$1/;
open ($motu, '>image.csv');
print $motu $data;
close ($motu);

system ("php build.php");
