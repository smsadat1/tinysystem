# db
**A lightweight persistent key-value store with B-tree indexing, log-based recovery, and a configurable in-memory cache.**

Purpose built database for TinySystem written in C. It combines persistent key-value storage with a bounded in-memory cache, providing a simple TCP interface for storing, retrieving, and managing keys.

## Commands

| Command  | Purpose                    | Effect
| -------- | -------------------------- |--------------------------
| `SET`    | Store/update a key         | Cache LRU Pindex Replay
| `DELETE` | Remove a key               | Cache LRU Pindex Replay
| `GET`    | Retrieve a value           | Cache LRU Pindex
| `EXIST`  | Check whether a key exists | Cache LRU Pindex
| `EXPIRE` | Set/update TTL             | Cache LRU 
| `TTL`    | Get remaining TTL          | Cache LRU
| `STAT`   | Status                     | Cache
| `PING`   | PING PONG                  | 


**Operations**

```text
SET <key> <value>
  ├── lock mutex_mutation
  ├── lock replay → update replay → unlock replay
  ├── lock storage  
  ├── update pindex
  ├── update cache → update LRU
  ├── unlock storage  
  ├── unlock mutex_mutation
  └── send "OK" to client
```

```text
DELETE <key> <value>
  ├── lock mutex_mutation
  ├── lock replay → update replay → unlock replay
  ├── lock storage  
  ├── update pindex
  ├── update cache → update LRU
  ├── unlock storage  
  ├── unlock mutex_mutation
  └── send "OK" to client
```

```text
GET <key>
  ├── Cache hit  → lock storage → get value → Update LRU  → Unlock storage → send <value> to client
  └── Cache miss → lock storage → Search pindex 
                                  ├── Found → Populate cache → Update LRU → unlock storage → send <value> to client
                                  └── Absent → unlock storage → send "(nil)" to client 
```


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
$ db --eviction lru 
```
Use LFU 
```bash
$ db --eviction lfu
```

**Cache memory limit**

Set the cache capacity to 128 MB:
```bash
$ db --mem_max_mb 128
```

Set the cache capacity to 256 MB:
```bash
$ db --mem_max_mb 256
```

**Cache statistics**

```bash
$ db --stat
status: running
cache_entries: 123
cache_hits: 90
cache_misses: 33
cache_memory: 78430/134217728
```
The numbers are illustrative

| Field           | Description                                                       |
|-----------------|-------------------------------------------------------------------|
| status          | Server status       
| cache_entries   | Number of entries currently in the cache
| cache_hits      | Number of successful cache lookups
| cache_misses    | Number of unsuccessful cache lookups
| cache_memory    | Cache memory usage relative to its configured limit, in bytes


## Architecture

Startup sequence
```
db starts
  │
  ├── Launch replay worker
  │     │
  │     ├── Read complete records from replay.log
  │     ├── Follow replay ruleset
  │     └── Build in-memory B-tree (pindex)
  │
  ├── Replay worker exits successfully
  │
  ├── Initialize cache
  │
  ├── Launch eviction worker in separate thread
  │
  └── Start accepting TCP connections
```

**Replay ruleset**
- Ignore all rows without newline delimiter, malformed writes are ignored.
- If key exists in both pindex and replay, find the last mutation and update pindex accordingly.
- If key exists only in replay, find the last mutation and update pindex accordingly.
- If key exists only in pindex, deleted it because both `SET` and `DELETE` first appends replay.
- After completion and guarantee of pindex reconstruction, empty replay to prevent indefinite growth.



## Design Notes
- `pindex` refers to the persistent index backed by  `db.log` file.
- `replay.log` stores mutations used for recovery.
- Disk lookup uses B-tree.
- LRU is the default eviction policy.
- LFU uses LRU as tie breaker.
- Cache expiration does not affect persistent records.
- The cache memory limit is independent of persistent storage capacity.
- Database, eviction worker and cache operations use a mutex lock, except `PING`.
- The recovery worker runs before the server begins accepting requests.