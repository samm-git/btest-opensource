# btest-opensource
## about
This is attempt to create opensource version of the btest tool. Currently it is possible to run bandwith-test only from another mikrotik devices or from the windows closed source client. 

Protocol itsef seems to be not complicated, so it should not be easy to create 3rd party client for it.

# Protocol description
There is no official protocl description, so everything was obtained using WireShark tool and RouterOS 6 running in the virtualbox, which was connecting to the real device. For now i am inspecting only TCP, later will try to do UDP if TCP works.
```
> bserver: hello
01:00:00:00 
> client: Start receive cmd 
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
01:03:01:00:00:80:00:00:00:00:00:00:00:00:00:00 (both)
01:03:00:00:00:80:00:00:00:00:00:00:00:00:00:00 (both, random data)
00:03:01:00:dc:05:00:00:00:00:00:00:00:00:00:00 (both, UDP)
> bserver: Start receive (auth is disabled on server):
01:00:00:00
> bserver: auth requested (3 random packets)
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
 - cmd[2]: random data, 00 - random, 01: use \00 character
 - cmd[4:5]: remote-udp-tx-size (dc:05) on UDP, 00:80 on TCP
 - cmd[6:7]: always 0?
 - cmd[8:11]: remote-tx-speed, 0: unlimimted, 1-4294967295: value
 - cmd[12:15]: local-tx-speed, 0: unlimimted, 1-4294967295: value
3. If server authentication is disabled it sends 01:00:00:00 and starts to transmit/recieve data. 
If auth is enabled it sends 20bytes command (probably) with 02:00:00:00 in the beginning and some random bytes in the [4:15] bytes.
Customer sends back 48bytes reply containing user name (unencrypted) and probably hash of the password with random salt. This is to be discovered. In the API manual (http://wiki.mikrotik.com/wiki/Manual:API) mikrotik using 16 bytes challenge and md5 to create response:
```
        md = md5.new()
        md.update('\x00')
        md.update(pwd)
        md.update(chal)
        self.talk(["/login", "=name=" + username,
                   "=response=00" + binascii.hexlify(md.digest())])
```
However it was found that this is not workong this way, e.g. this is challenge-response pair for the 'test' user/password:
```
ad32d6f94d28161625f2f390bb895637
3c968565bc0314f281a6da1571cf7255
```
The challenge protocol is not know yet. To guess challing alghoritm ive done a small btest server which always sends fixed hash (00000000000000000000000000000000). This way i been able to find that btest tool is sendn
