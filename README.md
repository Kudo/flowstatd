flowstatd
====================

1. Introduction
---------------------
flowstatd is a Netflow statistics service provides these features:
- Very lightweight
- Real time query, all the data queried is latest
- Calculate each IP's flow day by day
- Can list top flow's IP addresses
- With given over quota flow bytes, flowstatd can list overflow IP addresses
- Socket based query protocol, you can implement frontend your own.
- Provide white list which can hide some IP addresses' record.

2. Support Platform
---------------------
- FreeBSD (support kqueue)
- Linux

3. How To
---------------------
Requirements:
- cmake
- zlib

Build:
```
cd /path/to/flowstatd
mkdir build
cd build
cmake ..
make install
```

Usage:
```
cd /path/to/flowstatd/bin
./flowstatd [-f /path/to/config.json]
```

4. License
---------------------
GPLv2

5. Authors
---------------------
Kudo Chien &lt;ckchien At gmail.com&gt;
