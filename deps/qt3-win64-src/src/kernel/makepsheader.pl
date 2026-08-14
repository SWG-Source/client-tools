#!/usr/bin/perl

open(INPUT, 'qpsprinter.ps')
  or die "Can't open qpsprinter.ps";

$dontcompress = 1;
while(<INPUT>) {
  $line = $_;
  chomp $line;
  if ( /ENDUNCOMPRESS/ ) {
    $dontcompress = 0;
  }
  $line =~ s/%.*$//;
  $line = $line;
  if ( $dontcompress eq 1 ) {
    push(@uncompressed, $line);
  } else {
    push(@lines, $line);
  }
#  print "$line\n";
}

$uc = join(" ", @uncompressed);
$uc =~ s,\t+, ,g;
$uc=~ s, +, ,g;

$h = join(" ", @lines);
$h =~ s,\t+, ,g;
$h =~ s, +, ,g;
$h = $h.' ';

# now compress as much as possible
$h =~ s/ def / d /g;
$h =~ s/ bind def / D /g;
$h =~ s/ dup dup / d2 /g;
$h =~ s/ exch d / ED /g;
$h =~ s/ lineto / LT /g;
$h =~ s/ moveto / MT /g;
$h =~ s/ stroke / S /g;
$h =~ s/ setfont / F /g;
$h =~ s/ setlinewidth / SW /g;
$h =~ s/ closepath / CP /g;
$h =~ s/ rlineto / RL /g;
$h =~ s/ newpath / NP /g;
$h =~ s/ currentmatrix / CM /g;
$h =~ s/ setmatrix / SM /g;
$h =~ s/ translate / TR /g;
$h =~ s/ setdash / SD /g;
$h =~ s/ aload pop setrgbcolor / SC /g;
$h =~ s/ currentfile read pop / CR /g;
$h =~ s/ index / i /g;
$h =~ s/ bitshift / bs /g;
$h =~ s/ setcolorspace / scs /g;
$h =~ s/ dict dup begin / DB /g;
$h =~ s/ end d / DE /g;
$h =~ s/ ifelse / ie /g;
$h =~ s/ astore pop / sp /g;

# add the uncompressed part of the header before
$h = $uc.' '.$h;



#print $h;

# wordwrap at col 76
@head = split(' ', $h);
$line = shift @head;
while( @head ) {
  $token = shift @head;
  chomp $token;
#  print "\nl=$l, len=$len, token=$token.";
  $newline = $line.' '.$token;
  $newline =~ s, /,/,g;
  $newline =~ s, \{,\{,g;
  $newline =~ s, \},\},g;
  $newline =~ s, \[,\[,g;
  $newline =~ s, \],\],g;
  $newline =~ s,\{ ,\{,g;
  $newline =~ s,\} ,\},g;
  $newline =~ s,\[ ,\[,g;
  $newline =~ s,\] ,\],g;
  if ( length( $newline ) > 76 ) {
#    print "\nline=$line\n";
    $header = $header."\n\"".$line."\\n\"";
    $newline = $token;
  }
  $line = $newline;
}
$header = $header."\n\"".$line."\\n\"";


print $header;
