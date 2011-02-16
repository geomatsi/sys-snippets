#!/usr/bin/perl -w

use strict;
use IO::Socket;

my $msg;

my $send = IO::Socket::INET->new(
        PeerAddr => '127.0.0.1',
        PeerPort => '20001',
        Proto => 'udp') || die "socket: $@";

while ($msg = <STDIN>){
    $send->send($msg);
    last if $msg =~ /quit/i;
}

undef $send;

