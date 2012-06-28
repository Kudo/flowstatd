Kudo's flowd
====================

1. Introduction
---------------------
flowd is a Netflow statistics service provides these features:
    - Very lightweight
    - Real time query, all the data queried is latest
    - Calculate each IP's flow day by day
    - Can list top flow's IP addresses
    - With given over quota flow bytes, flowd can list overflow IP addresses
    - Socket based query protocol, you can implement frontend your own.
    - Provide white list which can hide some IP addresses' record.

2. Support Platform
---------------------
FreeBSD (support kqueue)
Linux

3. How To
---------------------
cd src ; make
cd ../bin ; ./flowd [-i listen_ip_address] -p <netflow_export_to_which_port> [-P flowd_command_port]

4. License
---------------------
GPLv2

5. Authors
---------------------
Kudo Chien <ckchien At gmail.com>
