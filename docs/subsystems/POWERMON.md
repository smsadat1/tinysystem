# netmon — Defined scope

**Responsibility:** Monitor the home router's power and network availability, then publish state transitions to external systems.

### 1. Router states

| State | Meaning |
|---|---|
| `router_normal` | Router has power and internet connectivity. |
| `router_nonet` | Router has power but no internet connectivity. |
| `router_offline` | Router has no power. |

### 2. Event interface

**Socket:**
```text
/run/tinysystem/netmon.sock
```

**Transport:** Persistent Unix Domain Socket.

**Protocol:** JSONL, server-to-client push events.

Example:

```json
{
  "sequence": 41,
  "event": "network_transition",
  "from": "router_normal",
  "to": "router_nonet",
  "timestamp": "2026-11-06T15:30:00Z"
}
```

### 3. State transitions

```text
router_normal <----> router_nonet
      |                    |
      |                    |
      v                    v
         router_offline
```

More explicitly:

```text
router_normal  -> router_nonet
router_nonet   -> router_normal

router_normal  -> router_offline
router_nonet   -> router_offline

router_offline -> router_normal
router_offline -> router_nonet
```
