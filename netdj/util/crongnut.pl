#!/usr/bin/perl -w
use strict;
my $THRES = 10 * 60; # seconds
my $DIR = "/var/cache/netdj";
my $WD = "                          ";
my $LASTCHECK = 0;
my $MINDOWNLOAD = 4*1024*1024;

print "Gnut watchdog v0.3\n";
print "------------------\n";

# Assume big download when started
my $addedbytes_last = $MINDOWNLOAD + 1;

while (1) {
  # Count the cattle
  my ($latest, $addedbytes, $otwbytes) = check_gnut("$DIR/*");

  printf("[".localtime()."] Latest filechange was %.1f minutes ago\n",
	 $latest/60);
  printf("[".localtime()."] Status: %.2f MB added, %.2f MB on the way\n",
	 $addedbytes/1024/1024, $otwbytes/1024/1024);

  my $sleeptime = $THRES;
  my $gnutproc = check_proc();

  # Check:
  # * Is gnut running?
  # * Is the oldest file newer than the specified threshold?
  # * Have we downloaded the minimum ammount?
  if (!$gnutproc || $latest >= $THRES
      || ($addedbytes_last + $addedbytes) < $MINDOWNLOAD) {
    if ($gnutproc) {
      print "$WD   Hmmm, somebody is sleeping on the job!\n";
#      print "$WD   Telling it to quit nicely\n";
#      `killall -HUP gnut`;
#      sleep 5;
#      if (check_proc()) {
#	print "$WD   Apparently not listening to nice talk, axing it!\n";
      print "$WD   Killing gnut!\n";
	`killall -9 gnut`;
	sleep 5;
#      }
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
    # Sleep a bit longer
    $sleeptime = 2 * $THRES;
    # Give it a chance to start up
    $addedbytes_last = $MINDOWNLOAD + 1;
  } else {
    $addedbytes_last = $addedbytes;
  }

  # Sleeping my day away
  $sleeptime -= $latest;
  printf("[".localtime()."] Waiting %.1f minutes ...\n",
	 ($sleeptime)/60);
  sleep ($sleeptime);
}


sub check_gnut {
  my $newest = -1;
  my $dir = shift;
  my $addedbytes = 0;
  my $otwbytes = 0;
  while (glob($dir)) {
    my $ctime = (stat($_))[10];
    my $diftime = time - $ctime;
    if ($diftime < $newest || $newest == -1) {
      $newest = $diftime;
    }
    if ($ctime > $LASTCHECK && m/.+\.mp3$/ && $LASTCHECK) {
      $addedbytes += (stat($_))[7];
    }
    if (m/.+\.gnut$/ && $LASTCHECK) {
      $otwbytes += (stat($_))[7];
    }
  }
  $LASTCHECK = time;
  return ($newest, $addedbytes, $otwbytes);
}

sub check_proc {
  return `ps --no-headers -C gnut`;
}
