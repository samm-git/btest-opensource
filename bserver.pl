#!/usr/bin/env perl

# this is PoC for the Mikrotik bandwith server. On mikorotik use
# /tool bandwidth-test <ip> duration=10s protocol=tcp tcp-connection-count=1
# to connect to it

use IO::Socket::INET;
use Digest::MD5 qw(md5 md5_hex md5_base64);

# auto-flush on socket
$| = 1;
 
# creating a listening socket
my $socket = new IO::Socket::INET (
    LocalHost => '0.0.0.0',
    LocalPort => '2000',
    Proto => 'tcp',
    Listen => 5,
    Reuse => 1
);
die "cannot create socket $!\n" unless $socket;
print "server waiting for client connection on port 2000\n";
 
while(1)
{
    # waiting for a new client connection
    my $client_socket = $socket->accept();
 
    # get information about a newly connected client
    my $client_address = $client_socket->peerhost();
    my $client_port = $client_socket->peerport();
    print "connection from $client_address:$client_port\n";
    print "sending hello\n";
    # write response data to the connected client
    $data = pack 'H*', '01000000';
    $client_socket->send($data);
    print "reading reply\n";
    $client_socket->recv($data, 1024);
    print "data is ".unpack('H*', "$data")."\n";
    # send auth requested command
    $client_socket->send(pack 'H*', '02000000');
    $digest=pack 'H*', '00000000000000000000000000000000';
    # print "digest md5=".md5_hex($digest)." expected reply is: ".md5_hex(md5($digest))."\n";
    $client_socket->send($digest);
    $client_socket->recv($data, 1024);
    print "client auth data is ".unpack('H*', "$data")."\n";
#    $client_socket->send(pack 'H*', '00000000'); # auth failed
    $client_socket->send(pack 'H*', '01000000'); # auth accepted

    # send data
    while (1) {
	$client_socket->send(pack 'H*', '00' x 100000000);
	print ".";
   }
    # notify client that response has been sent
    shutdown($client_socket, 1);
}
$socket->close();
