#!/usr/bin/perl


use strict;

our %SIG;

$SIG{'RTMIN'} = sub { print "GOT SIGRTMIN\n"; };
$SIG{'INT'} = sub { print "GOT SIGINT\n"; };

sleep(60);
