# Tinycache
**It's over for Redis, tinycache is here**


## Architecture

```bash
Client
  │
  │ TCP byte stream
  ▼
┌──────────────────────────────┐
│     Communication Layer      │
│                              │
│  TCP server                  │
│  connection handling         │
│  framing / parsing           │
│  protocol errors             │
└──────────────┬───────────────┘
               │
               │ parsed command
               ▼
┌──────────────────────────────┐
│          API Layer           │
│                              │
│  tc_set()                    │
│  tc_get()                    │
│  tc_delete()                 │
│  tc_exists()                 │
│  tc_expire()                 │
│  tc_ttl()                    │
|  tc_stat()                   |
└──────────────┬───────────────┘
               │
               │ cache operations
               ▼
┌─────────────────────────────────────────────────┐
│            Cache Layer                          │
│                                                 │
│  hash table                                     │
│  LRU policy                                     │
│  TTL / expiration                               │
│  memory accounting                              │
│  eviction                                       │
│  entry management                               │
|                                                 |
|  Internal manager functions:                    |
|  insert() -> manages lock internally            |
|  delete() -> expects caller to manage lock      |
|  search() -> expects caller to manage lock      |
└─────────────────────────────────────────────────┘
```


## Commands

| Command  | Purpose                    |
| -------- | -------------------------- |
| `SET`    | Store/update a key         |
| `GET`    | Retrieve a value           |
| `DELETE` | Remove a key               |
| `EXISTS` | Check whether a key exists |
| `EXPIRE` | Set/update TTL             |
| `TTL`    | Get remaining TTL          |
| `PING`   | PING PONG                  |
| `STAT`   | Status                     |

**Patterns**
```bash
SET key value
GET key
DELETE key
EXISTS key
EXPIRE key seconds
TTL key
PING
STAT
```


## Note
> TinyCache is a minimal, educational in-memory key-value cache written in C. It supports basic key-value operations, TTL-based expiration, and memory-bounded eviction. It is not intended to replace Redis in production workloads. Don't get offended if you are Redis fanboy.