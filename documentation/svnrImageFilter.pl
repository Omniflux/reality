#!/usr/bin/perl
use strict;
use warnings;

my $num_args = $#ARGV + 1;
if ($num_args != 1) {
  print "\nYou must pass the file name to process!\n";
  exit;
}

my $document = $ARGV[0];
open(my $fh, $document) or die "Error: cannot open file $document for reading\n";

while(<$fh>) {
  # The string to search is in the format: ![label](fileName "description")
  if (/!\[([a-zA-Z\.0-9\-_]+)\]\(([a-zA-Z\.0-9\-_]+)\s"([^""]+)"/) {
#    print "ID: $1, file name: $2, Caption: $3\n";
    print "![$3][$1]\n\n";
    print "[$1]: $2 \"$3\" width=5in\n\n";
  }
  else {
    print;
  }
}
close($fh);
