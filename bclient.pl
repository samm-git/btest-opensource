#!/usr/bin/env perl

# this is PoC for the Mikrotik bandwith server. On mikorotik use
# /tool bandwidth-test <ip> duration=10s protocol=tcp tcp-connection-count=1
# to connect to it

use IO::Socket::INET;
use Digest::MD5 qw(md5 md5_hex md5_base64);
use Data::Dumper;

# auto-flush on socket
$| = 1;
 
my $udp_port_offset=256;
my $require_auth=0;

# creating a listening socket
my $socket = new IO::Socket::INET (
    PeerHost => '192.168.10.1',
    PeerPort => '2000',
    Proto => 'tcp',
);
die "cannot create socket $!\n" unless $socket;
print "connecting to server port 2000\n";
 
$socket->recv($data, 1024);
print "client hello data is ".unpack('H*', "$data")."\n";
$data=pack('CCCCvvNN',
	1, # UDP
	2, # TX
	1, # Not random
	0, # TCP Count
	1500, # TX Size
	0, # Unknown
	0, # Unlimited
	0, # Unlimited
);
print "client command is ".unpack('H*', "$data")."\n";

$socket->send($data);

while(defined($socket->recv($data, 32768))) {
	print "client recv data is ".unpack('H*', "$data")."\n";
}
exit(0);

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
    my $action=unpackCmd($data);
    print(Dumper($action));
    # send auth requested command
    if ($require_auth) {
        $client_socket->send(pack 'H*', '02000000');
        $digest=pack 'H*', '00000000000000000000000000000000';
        # print "digest md5=".md5_hex($digest)." expected reply is: ".md5_hex(md5($digest))."\n";
        $client_socket->send($digest);
        $client_socket->recv($data, 1024);
        print "client auth data is ".unpack('H*', "$data")."\n";
    #    $client_socket->send(pack 'H*', '00000000'); # auth failed
        $client_socket->send(pack 'H*', '01000000'); # auth accepted
    }
    if ($action->{proto} eq 'TCP') {
    	# send data
    	while (1) {
	    $client_socket->send(pack 'H*', '00' x $action->{tx_size});
	    print ".";
        }
    	# notify client that response has been sent
        shutdown($client_socket, 1);
    } else {
	my $tx_socket = new IO::Socket::INET (
		PeerAddr => "$client_address:2000",
		Proto => 'udp',
	);
	die "cannot create socket $!\n" unless $tx_socket;
	sleep(10);
    	#while (1) {
	#    $tx_socket->send(pack 'H*', '00' x $action->{tx_size});
	#    print ".";
        #}
	$tx_socket->close();
    }
}
$socket->close();

sub unpackCmd {
	my $data=shift;
	my @cmdlist=unpack('CCCCvvNN', $data);
	my $cmd={
		proto => $cmdlist[0] ? 'TCP' : 'UDP',
		direction => ($cmdlist[1]==1) ? 'RX' : (($cmdlist[1]==2) ? 'TX' : 'TXRX'),
		random => ($cmdlist[2]==0),
		tcp_conn_count => ($cmdlist[3]==0) ? 1 : $cmdlist[3],
		tx_size => $cmdlist[4],
		unknown => $cmdlist[5],
		remote_tx_speed => $cmdlist[6],
		local_tx_speed => $cmdlist[7],
	};
	#print "data is ".unpack('H*', "$data")."\n";
	return($cmd);
}
