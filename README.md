# btest-opensource
## about
This is attempt to create opensource version of the btest tool. Currently it is possible to run bandwith-test only from another mikrotik devices or from the windows closed source client. 

Protocol itsef seems to be not complicated, so it should not be easy to create 3rd party client for it.

## Protocol description
There is no official protocl description, so everything was obtained using WireShark tool and RouterOS 6 running in the virtualbox, which was connecting to the real device. For now i am inspecting only TCP, later will try to do UDP if TCP works.
```
> bserver: hello
01:00:00:00 
> client: Start test cmd, depending on the client settings:
01:01:01:00:00:80:00:00:00:00:00:00:00:00:00:00 (transmit)
01:01:00:00:00:80:00:00:00:00:00:00:00:00:00:00 (transmit, random data)
00:01:01:00:dc:05:00:00:00:00:00:00:00:00:00:00 (transmit, UDP)
01:02:01:00:00:80:00:00:00:00:00:00:00:00:00:00 (receive)
01:02:00:00:00:80:00:00:00:00:00:00:00:00:00:00 (receive, random data)
00:02:01:00:dc:05:00:00:00:00:00:00:00:00:00:00 (receive, UDP, remote-udp-tx-size: default (1500))
00:02:01:00:d0:07:00:00:00:00:00:00:00:00:00:00 (receive, UDP, remote-udp-tx-size: 2000)
00:02:01:00:00:fa:00:00:00:00:00:00:00:00:00:00 (receive, UDP, remote-udp-tx-size: 64000)
01:02:01:00:00:80:00:00:01:00:00:00:00:00:00:00 (receive, remote-tx-speed=1)
01:02:01:00:00:80:00:00:ff:ff:ff:ff:00:00:00:00 (receive, remote-tx-speed=4294967295)
01:02:01:00:00:80:00:00:00:00:00:00:01:00:00:00 (receive, local-tx-speed=1)
01:02:01:00:00:80:00:00:00:00:00:00:ff:ff:ff:ff (receive, local-tx-speed=4294967295)
01:02:01:0a:00:80:00:00:00:00:00:00:00:00:00:00 (receive, tcp-connection-count=10)
01:03:01:00:00:80:00:00:00:00:00:00:00:00:00:00 (both)
01:03:00:00:00:80:00:00:00:00:00:00:00:00:00:00 (both, random data)
00:03:01:00:dc:05:00:00:00:00:00:00:00:00:00:00 (both, UDP)
> bserver: Start test confirm (auth is disabled on server):
01:00:00:00 (tcp-connection-count=1 on a client)
01:bc:04:00 (tcp-connection-count>1 on a client)
> bserver: auth requested, with 16 challenge bytes (3 random packets provided)
02:00:00:00:90:67:3f:0f:5c:c7:4e:17:a0:e0:9e:1c:b9:ee:3b:0c
02:00:00:00:17:e7:ee:84:83:cc:15:53:e8:fa:9c:0d:ad:ac:b8:e1
02:00:00:00:ee:d1:19:b9:d3:f2:df:6d:04:46:da:25:55:44:49:81
> client: auth reply (3 random packets, userid test)
90:75:1f:b0:2a:f7:25:51:46:25:71:c3:16:ce:cc:2b:74:65:73:74:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
a0:1b:fa:27:78:55:08:71:93:09:70:86:15:30:84:ac:74:65:73:74:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
b4:f2:9e:06:5e:74:da:89:65:c9:be:94:4d:bf:8f:20:74:65:73:74:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
> bserver: packet data follows
00:00:00:00........
```
1. Server always starts with "hello" (01:00:00:00) command after establishing TCP connection to port 2000. If UDP protocl is specified in the client TCP connection is still established.
2. Client sends 16 bytes command started with 01 (TCP) or 00 (UDP) command to the server. Some of the bytes guess:
 - cmd[0]: protocol, 01: TCP, 00: UDP
 - cmd[1]: direction, 01 - transmit, 02 - receive, 03 - both
 - cmd[2]: use random data, 00 - use random, 01: use \00 character
 - cmd[3]: tcp-connection-count, 0 if tcp-connection-count=1, number if more
 - cmd[4:5]: remote-udp-tx-size (dc:05) on UDP, 00:80 on TCP, UINT16 - Little Endian
 - cmd[6:7]: always 0?
 - cmd[8:11]: remote-tx-speed, 0: unlimimted, 1-4294967295: value, UINT32 - Big Endian
 - cmd[12:15]: local-tx-speed, 0: unlimimted, 1-4294967295: value, UINT32 - Big Endian
3. If server authentication is disabled it sends 01:00:00:00 and starts to transmit/recieve data. 
If auth is enabled server sends 20bytes reply started with 02:00:00:00 in the beginning and random bytes (challenge) in the [4:15] range.
Customer sends back 48 bytes reply containing user name (unencrypted) and 16 bytes hash of the password with challenge. Hashing alghoritm is not known. See "authentication" section.
4. If auth failed server sends back `00000000` (client shows "authentication failed"), if succeed - `01000000` packet and test is starting.
5. If tcp-connection-count > 1 server should reply with `01:xx:xx:00` where xx seems to be some kind of authentification data to start other connections. This number is used in other threads. 

## Authentication
Sample of the challenge-response for the "test" password:
```
ad32d6f94d28161625f2f390bb895637
3c968565bc0314f281a6da1571cf7255
```
To guess hashing alghoritm i implemented btest server which always sends fixed hash data (`00000000000000000000000000000000`). This way i been able to find that btest tool is sending double md5 of the challenge if password is not set:
```
# echo  00000000000000000000000000000000| xxd -r -p | md5 |xxd -r -p |md5
398d01fdf7934d1292c263d374778e1a
```
But if password is set - hash is different, and i been not able to find the way how to reproduce it yet. E.g. with challenge `00000000000000000000000000000000` and password '1' final hash is `a56b579c4f5194426ae217b3ee4ec1ba`. Also it was found that only password and challenge are used in the hash, because username is not affecting resulting data. 
