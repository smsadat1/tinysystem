# Tinycache


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
│  set()                       │
│  get()                       │
│  delete()                    │
│  exists()                    │
│  expire()                    │
│  ttl()                       │
└──────────────┬───────────────┘
               │
               │ cache operations
               ▼
┌──────────────────────────────┐
│         Cache Layer          │
│                              │
│  hash table                  │
│  LRU policy                  │
│  TTL / expiration            │
│  memory accounting           │
│  eviction                    │
│  entry management            │
└──────────────────────────────┘
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
```