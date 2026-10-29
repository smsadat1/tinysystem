# TinyCache
**A lightweight key-value store with LRU & LFU eviction available.**

## Setup
```bash
$ git clone git@github.com:smsadat1/tinysystem.git
$ cd tinycache
$ sudo ./setup.sh
```

## Commands

| Command  | Purpose                    
| -------- | -------------------------- 
| `SET`    | Store/update a key         
| `DELETE` | Remove a key               
| `GET`    | Retrieve a value           
| `EXIST`  | Check whether a key exists 
| `EXPIRE` | Set/update TTL              
| `TTL`    | Get remaining TTL          
| `STAT`   | Status                     
| `PING`   | PING PONG             


## Operations

```text
SET <key> <value>
  ├── lock cache
  ├── update cache
  ├── update LRU 
  ├── unlock cache
  └── send "OK" to client
```
**Happens in a single mutex lock**

```text
DELETE <key> <value>
  ├── lock cache
  ├── update cache
  ├── update LRU
  ├── unlock storage  
  ├── unlock cache
  └── send "OK" to client
```
**Happens in a single mutex lock**

```text
GET <key>
  ├── lock cache
  ├── search value
  ├── update LRU 
  ├── unlock cache
  └── send <value> to client
```
**Happens in a single mutex lock**

```text
EXIST <key>
  ├── Cache hit → Update LRU → send "Y" to client
  └── Cache miss → Search pindex
                    ├── Found → Populate cache → Update LRU → send "Y" to client
                    └── Absent → send "N" to client
```
**Happens in a single mutex lock**

```text
EXPIRE <key> <seconds>
  ├── Cache hit → Add expiry → Update LRU → send "Y" to client
  └── Cache miss → send "FAILED" to client
```
**Happens in a single mutex lock**

```text
TTL <key> 
  ├── Cache hit → Update LRU →  send <remaining_seconds> to client
  └── Cache miss → send "FAILED" to client
```
**Happens in a single mutex lock**

```text
STAT KEY ── send client <number of keys in cache>
STAT MEM ── send client <number of used bytes in cache>
```
**Happens in a single mutex lock**

```text
PING ── send client "PONG"
```
**No mutex lock**

## Configurations
Configuration is stored in:

```text
/etc/tinysystem/db.conf
```

The configuration file determines the locations of the persistent database and replay log.


## Command-line Options

**Eviction policy**

Use LRU (set by default)
```bash 
$ tinycache --eviction lru 
```
Use LFU 
```bash
$ tinycache --eviction lfu
```

**Cache memory limit**

Set the cache capacity to 128 MB:
```bash
$ tinycache --mem_max_mb 128
```

Set the cache capacity to 256 MB:
```bash
$ tinycache --mem_max_mb 256
```

## Architecture

Startup sequence
```
TinyCache starts
  │
  ├── Initialize cache
  │
  ├── Launch eviction worker in separate thread
  │
  └── Start accepting TCP connections
```


## Design Notes
- LRU is the default eviction policy.
- LFU uses LRU as tie breaker.
- Eiction worker and cache operations use a mutex lock, except `PING`.