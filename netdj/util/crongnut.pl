#!/usr/bin/perl -w
use strict;
my $THRES = 20 * 60; # 20 minutes
my $DIR = "/home/beaufour/netdj/cache";
my $WD = "                          ";

print "Gnut watchdog v0.1\n";
print "------------------\n";

while (1) {
  # Count the cattle
  my $latest = check_gnut("$DIR/*");

  print "[".localtime()."] Latest filechange was $latest seconds ago\n";

  my $gnutproc = `ps --no-headers -C gnut`;

  if (!$gnutproc || $latest >= $THRES) {
    print "$WD   Hmmm, somebody is sleep on the job!\n";
    if ($gnutproc) {
      print "$WD   Killing gnut\n";
      `killall -9 gnut`;
      sleep 10;
    }

    print "$WD   Removing any leftover .gnut's\n";
    my $carcasses;
    if ($carcasses = unlink glob("$DIR/*.gnut")) {
      print "$WD     $carcasses deleted\n";
    }

    print "$WD   Spawn new gnut-daemon\n";
    `gnut -d > gnut.log 2> gnut.errlog &`;
  }

  # Sleeping my day away
  print "[".localtime()."] Waiting ".(($THRES - $latest)/60)." minutes ...\n";
  sleep $THRES - $latest;
}

sub check_gnut {
  my $newest = -1;
  my $dir = shift;
  while (glob($dir)) {
    my $diftime = time - (stat($_))[10];
    if ($diftime < $newest || $newest == -1) {
      $newest = $diftime;
    }
  }
  return $newest;
}
