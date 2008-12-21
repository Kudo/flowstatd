#!/usr/bin/perl -w
use strict;
use Socket;
use Getopt::Std;
our($opt_p);

my $FLOWPATH = "/Netflow/RAW";
my $ACL = "/Netflow/etc/flow.acl";
my $SUBNET = "140.123";
my $IPNUM = 65536;
my %hash;

sub usage {
  print "$0 [-p flowpath] Year Month Day\n";
  print "eg:\t$0 -p /Netflow/RAW 2008 01 01\n";
  exit;
}

sub initFlow {
  for (my $i = 0; $i < 256; $i++) {
    for (my $j = 0; $j < 256; $j++) {
      my @UDarr = ([0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0],[0,0]);
      $hash{"$SUBNET.$i.$j"} = \@UDarr;
    }
  }
}

sub addFlow {
  my ($ip, $hour, $UD, $octets) = @_;

  if (exists($hash{$ip})) {
    ${$hash{$ip}}[$hour][$UD] += $octets;
  } else {
    print "$ip hash bucket is not existed\n";
    exit;
  }
}

sub showFlow {
  my $count = 0;
  foreach my $ip (sort {$a cmp $b} keys %hash) {
    printf("%d\t%s\t", $count++, $ip);

    my $upFlow = 0;
    my $downFlow = 0;

    for (my $i = 0; $i < 24; $i++) {
      $upFlow += ${$hash{$ip}}[$i][0];
      $downFlow += ${$hash{$ip}}[$i][1];

      printf("%llu\t%llu\t", ${$hash{$ip}}[$i][0], ${$hash{$ip}}[$i][1]);
    }

    printf("%llu\t%llu\t%llu\n", $upFlow, $downFlow, $upFlow + $downFlow);
  }
}

sub dumpFlow {
  my ($year, $month, $day, $hour, $mode) = @_;

  if ($mode == 0) {			# Upload
    open PH, "flow-cat $FLOWPATH/$year-$month/$year-$month-$day/ft-v05.$year-$month-$day.$hour* | flow-nfilter -f $ACL -F Port0000-S | flow-stat -f8 |" or die "$!";

    for (my $i = 0; $i < 19; $i++) {
      <PH>;
      print;
    }
    exit;
    while (<PH>) {
      chomp;
      my @arr = split(/\s+/);
      addFlow($arr[0], $hour, 0, $arr[2]);
    }
    close PH;
  } else {				# Download
    open PH, "flow-cat $FLOWPATH/$year-$month/$year-$month-$day/ft-v05.$year-$month-$day.$hour* | flow-nfilter -f $ACL -F Port0000-D | flow-stat -f8 |" or die "$!";

    for (my $i = 0; $i < 19; $i++) {
      <PH>;
    }
    while (<PH>) {
      chomp;
      my @arr = split(/\s+/);
      addFlow($arr[0], $hour, 1, $arr[2]);
    }
    close PH;
  }
}

sub main {
  usage() unless (@ARGV >= 3) && getopts("p:");
  $FLOWPATH = $opt_p if defined($opt_p);

  my $year = $ARGV[0];
  my $month = $ARGV[1];
  my $day = $ARGV[2];

  if ($year !~ /^\d{4}$/ || $month !~ /^\d{2}$/ || $day !~ /^\d{2}$/) {
    usage();
  } elsif (! -d "$FLOWPATH/$year-$month/$year-$month-$day") {
    print "Error: $FLOWPATH/$year-$month/$year-$month-$day. No such directory\n";
    exit;
  }

  initFlow();

  for (my $i = 0; $i < 1; $i++) {
    my $hour = sprintf("%02d", $i);
    open PH, "ls $FLOWPATH/$year-$month/$year-$month-$day/ft-v05.$year-$month-$day.$hour* |" or warn "$!\n";
    close PH;

    dumpFlow($year, $month, $day, $hour, 0);
    #dumpFlow($year, $month, $day, $hour, 1);
  }

  showFlow();
}

main();
