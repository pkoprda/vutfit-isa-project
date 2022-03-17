# Network Applications and Network Administration - Project
## Wireshark dissector
---

**Author:** Peter Koprda <xkoprd00@stud.fit.vutbr.cz>


## Usage of client
```
$ bin/client [OPTION ...] COMMAND [ARGS]
```

### Options:
```
-a <addr>, --address <addr>         Server hostname or address to connect to
-p <port>, --port <port>            Server port to connect to
--help, -h                          Show this help
```

### Commands:
```
register <username> <password>
login <username> <password>
list
send <recipient> <subject> <body>
fetch <id>
logout
```
