#!/usr/bin/perl
use strict;
use warnings;
use Socket;

&main();

sub main {
    if (@ARGV < 1) {
	print "Usage: $0 flowdata_file [list_ip]\n";
	exit;
    }

    my $listIP = $ARGV[1];
    my $buf;
    my $fh;

    open($fh, "gzcat $ARGV[0] |");
    read($fh, $buf, 16);
    my ($rcvNetListSize, $nSubnet, $hostflowSize, $sumIpCount) = unpack('i4', $buf);
    my @rcvNetList;

    print <<"EOB";
#
# Subnets List
# ----------------------------------------------
EOB

    foreach (1..($nSubnet)) {
	read($fh, $buf, $rcvNetListSize);
	my ($net, $mask, $maskBits, $ipCount) = unpack('a4a4i2', $buf);
	printf("%s/%d\n", inet_ntoa($net), $maskBits);
    }

    print <<"EOB";
#
# IP Flow List
# ----------------------------------------------
EOB


    foreach (1..($sumIpCount)) {
	read($fh, $buf, $hostflowSize);
	my ($ip,
	    $flowUp00, $flowDown00,
	    $flowUp01, $flowDown01,
	    $flowUp02, $flowDown02,
	    $flowUp03, $flowDown03,
	    $flowUp04, $flowDown04,
	    $flowUp05, $flowDown05,
	    $flowUp06, $flowDown06,
	    $flowUp07, $flowDown07,
	    $flowUp08, $flowDown08,
	    $flowUp09, $flowDown09,
	    $flowUp10, $flowDown10,
	    $flowUp11, $flowDown11,
	    $flowUp12, $flowDown12,
	    $flowUp13, $flowDown13,
	    $flowUp14, $flowDown14,
	    $flowUp15, $flowDown15,
	    $flowUp16, $flowDown16,
	    $flowUp17, $flowDown17,
	    $flowUp18, $flowDown18,
	    $flowUp19, $flowDown19,
	    $flowUp20, $flowDown20,
	    $flowUp21, $flowDown21,
	    $flowUp22, $flowDown22,
	    $flowUp23, $flowDown23,
	    $flowUpTotal, $flowDownTotal, $flowSum,
	) = unpack('a4Q48Q3', $buf);

	printf("%s\t%s\n", 
	    inet_ntoa($ip),
	    join("\t",
		$flowUp00, $flowDown00,
		$flowUp01, $flowDown01,
		$flowUp02, $flowDown02,
		$flowUp03, $flowDown03,
		$flowUp04, $flowDown04,
		$flowUp05, $flowDown05,
		$flowUp06, $flowDown06,
		$flowUp07, $flowDown07,
		$flowUp08, $flowDown08,
		$flowUp09, $flowDown09,
		$flowUp10, $flowDown10,
		$flowUp11, $flowDown11,
		$flowUp12, $flowDown12,
		$flowUp13, $flowDown13,
		$flowUp14, $flowDown14,
		$flowUp15, $flowDown15,
		$flowUp16, $flowDown16,
		$flowUp17, $flowDown17,
		$flowUp18, $flowDown18,
		$flowUp19, $flowDown19,
		$flowUp20, $flowDown20,
		$flowUp21, $flowDown21,
		$flowUp22, $flowDown22,
		$flowUp23, $flowDown23,
		$flowUpTotal, $flowDownTotal, $flowSum
	    ));
    }

    close($fh);
}
