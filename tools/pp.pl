#!/usr/bin/perl

# Version 0.1
#   see perlmonks 951682
#
use strict;
use warnings;
use Getopt::Long;

my %DEF;
my $sourceName = "";

GetOptions("define=s" => \%DEF,
           "source=s" => \$sourceName);

open( my $source, '<', $sourceName ) or die "Can't open $sourceName: $!";

my $doPrint = 1;
my $inComment = 0;

while (<$source>) {
  if (/^\s*#\s*define\s+(\w+)\s+(.+)$/) { 
    $DEF{$1}=$2;
    next;
  }
  elsif (/^\s*#\s*undef\s+(\w+)/) { 
    delete $DEF{$1}; 
    next;
  }
  elsif (   (/^\s*#\s*ifdef\s+(\w+)/) 
         || (/^\s*#\s*if\s+defined\(\w+\)\s+(\w+)/))
  {
    if (exists($DEF{$1})) {
      $doPrint = 1;
    }
    else {
      $doPrint = 0;
    };
    next;
  }
  elsif (/^\s*#\s*ifndef\s+(\w+)/){
    if (!exists($DEF{$1})) {
      $doPrint = 1;
    }
    else {
      $doPrint = 0;
    };
    next;
  }
  elsif (/^\s*#\s*endif/){
    $doPrint = 1;
    next;
  }
  # Start of a C-style comment
  elsif (m|^\s*/\*| && !m|\*/|) {
    $inComment = 1;
  }
  # End of a C-style comment
  elsif ((m|^\s*\*/|) && (!m|/\*|)) {
    $inComment = 0;
    next;
  }
  # Remove inline comments
  s|/\*.+\*/||;
  if ($inComment) {
    next;
  }
  # Replace defined symbols
  my $symbol;
  my $value;
  while( ($symbol, $value) = each %DEF ) {
    if ((my $symbIndex = index($_, $symbol)) != -1) {
      substr($_, $symbIndex, length($symbol), $value);
    }
  }
  print if ($doPrint);
}

close($source);
