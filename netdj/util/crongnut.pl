#!/usr/bin/perl -w
use strict;
my $THRES = 20 * 60; # 20 minutes
my $DIR = "/home/beaufour/netdj/cache";
my $WD = "                          ";

print "Gnut watchdog v0.2\n";
print "------------------\n";

while (1) {
  # Count the cattle
  my $latest = check_gnut("$DIR/*");

  print "[".localtime()."] Latest filechange was ".($latest/60)." minutes ago\n";

  my $gnutproc = check_proc();

  if (!$gnutproc || $latest >= $THRES) {
    if ($gnutproc) {
      print "$WD   Hmmm, somebody is sleeping on the job!\n";
      print "$WD   Telling it to quit nicely\n";
      `killall gnut`;
      sleep 5;
      if (check_proc()) {
	print "$WD   Apparently not listening to nice talk, giving it the axe!\n";
	`killall -9 gnut`;
	sleep 5;
      }
    } else {
      print "$WD   Seems like gnut's not running\n"; 
    }

    print "$WD   Removing any leftover .gnut's\n";
    my $carcasses;
    if ($carcasses = unlink glob("$DIR/*.gnut")) {
      print "$WD     $carcasses deleted\n";
    }

    print "$WD   Spawn new gnut-daemon\n";
    `gnut -d > gnut.log 2> gnut.errlog &`;
      
    $latest = 0;  
  }

  # Sleeping my day away
  print "[".localtime()."] Waiting ".(($THRES - $latest)/60)." minutes ...\n";
  sleep ($THRES - $latest);
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

sub check_proc {
  return `ps --no-headers -C gnut`;
}
