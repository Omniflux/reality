#!/usr/bin/perl

print "Write version number to Installer\n";

if ($#ARGV < 1) {
  die "Gotta tell me what files you want to use";
}
$inputFileName = $ARGV[0];
$outputFileName = $ARGV[1];

print "Input: $inputFileName\n";
print "Output: $outputFileName\n";

open(VERFILE, $inputFileName) || die "Cannot open file $inputFileName: $!";

$VER_NO = "";
$MAIN_VN ="";
$SUB_VN ="";
$PATCH_VN ="";
$BUILD_VN ="";
# Read the version number
while(<VERFILE>) {
  if ( m|^const \S+ REALITY_MAIN_VERSION\s*=\s*(.+);| ) {
    $MAIN_VN = hex $1;
  }
  elsif ( m|^const \S+ REALITY_SUB_VERSION\s*=\s*(.+);| ) {
    $SUB_VN = hex $1;
  }
  elsif ( m|^const \S+ REALITY_PATCH_VERSION\s*=\s*(.+);| ) {
    $PATCH_VN = hex $1;
  }
  elsif ( m|^const \S+ \S+ REALITY_BUILD_NUMBER\s*=\s*(.+);| ) {
    $BUILD_VN = $1;
  }
}
close(VERFILE);

$VER_NO = "${MAIN_VN}.${SUB_VN}.${PATCH_VN}.${BUILD_VN}";

print "Build number: $VER_NO\n";

open( CFGFILE, $outputFileName) || die "Cannot open $outputFileName for reading\n";

$content = "";
while(<CFGFILE>) {
  if ( /^version:\s+\d/ ) {
    $content .= "version: ${VER_NO}\n";
  }
  else {
    $content .= $_;
  }
}
close(CFGFILE);

open( CFGFILE, ">$outputFileName") || die "Cannot open $outputFileName for writing\n";

print CFGFILE $content;
close(CFGFILE);
