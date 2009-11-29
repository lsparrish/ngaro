# Tuari -- A small http server
# Gifted to the public domain
################ Configuration  ##################################

use Cwd;

# Change the next line to let tuari know where it lives:
# This directory will also be the server root directory

our $version=0.04;
our $basdir = getcwd;

# The port on which tuari will listen:

our $port = 9812;

# Add your own MIME types here; text/plain is the default.
our %mime_types = (
      '\.html?'    => 'text/html',
      '\.gif'	   => 'image/gif',
      '\.jpe?g'    => 'image/jpeg'
		  );


###################  no real need to edit below ##################

require 5.6.0;
package tuari; # keep namespace separate from CGI scripts

use Socket;
use strict;

our($localname);

initialise();

main_loop();

################################## Subroutines ###################

sub logerr($$); sub logmsg($); sub cat($$;$); # forward declarations


sub initialise {
   $tuari::basdir= $basdir; # make this variable visible for CGI scripts
}


sub main_loop {
  my $tcp = getprotobyname('tcp');
  socket(Server, PF_INET, SOCK_STREAM, $tcp)      or die "socket: $!";
  setsockopt(Server, SOL_SOCKET, SO_REUSEADDR, pack("l", 1)) or warn "setsockopt: $!";
  bind(Server, sockaddr_in($port, INADDR_ANY))    or die "bind: $!";
  listen(Server,SOMAXCONN)                        or die "listen: $!";
  logmsg "server started on port $port";

 CONNECT:
  for ( ; accept(Client,Server); close Client) {

    *STDIN = *Client;
    *STDOUT = *Client;

    my $remote_sockaddr  = getpeername(STDIN);
    my (undef, $iaddr)   = sockaddr_in($remote_sockaddr);
    my $peername         = gethostbyaddr($iaddr, AF_INET) || "localhost";
    my $peeraddr         = inet_ntoa($iaddr) || "127.0.0.1";

    my $local_sockaddr   = getsockname(STDIN);
    my (undef, $iaddr)   = sockaddr_in($remote_sockaddr);
    $localname           = gethostbyaddr($iaddr, AF_INET) || "localhost";
    my $localaddr        = inet_ntoa($iaddr) || "127.0.0.1";


    chomp($_ = <STDIN>);
    my ($method, $url, $proto, undef) = split;

    $url =~ s#\\#/#g;
    logmsg "<- $peername: $_";
    my ($file, undef, $arglist) = ($url =~ /([^?]*)(\?(.*))?/); # split at ?
    my $file_escaped = $file;
    $file =~ s/%([\dA-Fa-f]{2})/chr(hex($1))/eg; # %20 -> space

    if ( $method !~ /^(GET|POST|HEAD)$/ ) {
      logerr 400, "I don't understand method $method";
      next CONNECT;
    }


    if (-d  "$basdir/$file" ) {
      unless ($file =~ m#/$#) {
	   redirect ("$file/");
	   next CONNECT;
	  }
	my $dir = "$basdir/$file";
    if (-f "$dir/motu.html") {
	  $file .= "/motu.html";
    } else {
	  directory_listing($file);
	  next CONNECT;
      }
    }

    if ( not -r "$basdir/$file" ) {
      logerr 404, "$file: $!";
      next CONNECT;
    }

    my @env_vars = qw(USER_AGENT CONTENT_LENGTH CONTENT_TYPE);
    foreach my $var (@env_vars) {
		$ENV{$var} = ""; # delete $ENV{$var} will crash perl on Netbook :-(
		}
    while(<STDIN>) {
	  s/[\r\l\n\s]+$//;
	  /^User-Agent: (.+)/i and $ENV{USER_AGENT} = $1;
	  /^Content-length: (\d+)/i and $ENV{CONTENT_LENGTH} = $1;
	  /^Content-type: (.+)/i    and $ENV{CONTENT_TYPE} = $1;
	  last if (/^$/);
	}


    print "HTTP/1.0 200 OK\n"; # probably OK by now



    if ( $file =~ m/\.cgi$/i) {
      $ENV{SERVER_PROTOCOL} = $proto;
      $ENV{SERVER_PORT}     = $port;
      $ENV{SERVER_NAME}     = $localname;
      $ENV{SERVER_URL}      = "http://$localname:$port/";
      $ENV{SCRIPT_NAME}	    = $file;
      $ENV{SCRIPT_FILENAME} = "$basdir/$file";
      $ENV{REQUEST_URI}     = $url;
      $ENV{REQUEST_METHOD}  = $method;
      $ENV{REMOTE_ADDR}     = $peeraddr;
      $ENV{REMOTE_HOST}     = $peername;
      $ENV{QUERY_STRING}    = $arglist;
      $ENV{SERVER_SOFTWARE} = "tuari/$version";

      if ($method =~ /POST/) {
	    logmsg "<- Content-length: $ENV{CONTENT_LENGTH}, type: $ENV{CONTENT_TYPE}";
      }
      cgi_run ($file,$arglist);
      next CONNECT;
    }

    my $mime_type =  "text/plain"; # default
    foreach my $suffix (keys %mime_types) {
    	if ($file =~ /$suffix$/i) {
	    $mime_type = $mime_types{$suffix};
	    last;
	}
    }
    cat $file, $mime_type, $method || logerr 500, "$file: $!";
      next CONNECT;
  }
    die "Fatal error: accept failed: $!\n"; # This should never happen
  }

#################### other subroutines ####################

sub logmsg ($) {
    my $fulltime = localtime();
    my ($hms) = ($fulltime =~ /(\d\d:\d\d:\d\d)/);
    print STDERR  "$$ $hms @_\n";
}

sub logerr ($$) {
  my ($code, $detail) = @_;
  my %codes =
      ( 200  => 'OK',
        400  => 'Bad Request',
        404  => 'Not Found',
        500  => 'Internal Server Error',
        501  => 'Not Implemented',
      );
  my $msg = "$code " . $codes{$code};
  logmsg "-> $msg $detail";
  print  <<EOF;
HTTP/1.0 $msg
Content-type: text/html

<html><body>
<h1>$msg</h1>
<p>$detail</p>
<hr>
<p><I>tuari/$version server at $localname port $port</I></p>
</body></html>
EOF
}



sub cat($$;$){   # cat ($file, $mimetype) writes Content-type header and $file to STDOUT
  my ($file, $mimetype, $method) = @_;
  $method = "GET" unless $method;
  my $fullpath = "$basdir/$file";

  my ($x,$x,$x,$x,$x,$x,$x,$length,$x,$mtime) = stat($fullpath);
  $mtime = gmtime $mtime;
  my ($day, $mon, $dm, $tm, $yr) =
	  ($mtime =~ m/(...) (...) (..) (..:..:..) (....)/);

  print "Content-length: $length\n";
  print "Last-Modified: $day, $dm $mon $yr $tm GMT\n";
  print "Content-type: $mimetype\n\n";
  my $sent=0;
  if ($method eq "GET") {
    local $/; undef $/; # gobble whole files
    open IN, "<$fullpath" || return 0;
    my $content = <IN>;
    close IN;
    $sent = length($content);
    print $content;
  }
  logmsg "-> 200 OK $file: $sent bytes sent as $mimetype";
  return 1;
}


sub cgi_run {
  my ($file,$arglist) = @_;
  my ($dir) = ($file =~ /^(.*\/)/);
  my $path = "$basdir/$file";
  chdir "$basdir/$dir" or return logerr 500, "Cannot chdir to $basdir/$dir: $!";
  $path=~s/[A-Z]://;
  logmsg "-> doing $path";

  {package main; do $path;} # or return logerr 500, "Cannot do $path: $!";
  $@ and logerr 500, "in $file:<br>  $@";
  chdir $basdir;
}


sub directory_listing {
  my ($dir) = @_;
  $dir =~ s#//#/#g;
  chdir "$basdir/$dir" or return logerr 500, "Cannot chdir to $basdir/$dir: $!";
  my @files = glob("*");
  print  <<EOF;
HTTP/1.0 200 OK
Content-type: text/html

<html>
<head><title>$dir</title></head>
<body>
<h1>$dir</h1>
EOF
  print "<p><a href=..>..</a></p>\n";
  foreach my $file (sort @files) {
    -d $file and $file .= "/";
    print "<p><a href=./$file>$file</a></p>\n";
  }
  print <<EOF;
<hr>
<p><I>tuari/$version server at $localname port $port</I>
</body></html>
EOF
logmsg "-> 200 OK listing $dir";
}


sub redirect {
	my ($redir) = @_;
	print "HTTP/1.0 301 Moved Permanently\nLocation: $redir\n\n";
    logmsg "-> 301 Moved Permanently to $redir"
}
