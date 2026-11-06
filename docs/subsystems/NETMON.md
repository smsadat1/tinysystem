## powermon — Architecture

```text
                  Linux Kernel
                       |
                 Power Supply
                       |
                  udev events
                       |
                       v
                +-------------+
                |  powermon   |
                +-------------+
                       |
          +------------+------------+
          |            |            |
          v            v            v
      Observer     Telemetry      IPC Server
          |        Generator         |
          |            |             |
          |            v             |
          |          hardKV          |
          |            |             |
          |            v             |
          |      Reporting Worker    |
          |            |             |
          |            v             |
          |         GitHub           |
          |                          |
          +--------------------------+
                                     |
                             Unix Domain Socket
                                     |
                              External Systems
```

## 1. Continuous responsibilities

**Daily battery telemetry**

- Collect battery energy information.
- Collect battery capacity.
- Generate a report every 86400 seconds.
- Persist the report in hardKV.

Example:

```json
{
  "id": "BAT0",
  "energy_now_wh": 30.8,
  "energy_full_wh": 42.0,
  "capacity_percent": 73.2
}
```

## 2. Situational responsibilities

**Power outage tracking**

When AC disconnects:

1. Capture outage start timestamp.
2. Capture starting energy.
3. Capture starting capacity.

When AC reconnects:

1. Capture ending energy.
2. Capture ending capacity.
3. Calculate outage duration.
4. Generate the outage report.
5. Persist it in hardKV.

## 3. Event interface

**Socket:**

```text
/run/tinysystem/powermon.sock
```

**Transport:** Persistent Unix Domain Socket.

**Protocol:** JSONL.

**Direction:** Server → Client.

Example:

```json
{
  "sequence": 42,
  "event": "power_transition",
  "from": "AC",
  "to": "BATTERY",
  "timestamp": "2026-11-06T15:30:00Z"
}
```

Expected behavior:

| Event             | Action                |
|-------------------|-----------------------|
| Client connects   | Register subscriber   |
| AC → Battery      | Broadcast             |
| Battery → AC      | Broadcast             |
| No transition     | No message            |
| Client disconnect | Remove subscriber     |
| Client reconnect  | Register again        |

Also:
- Monotonically increasing sequence numbers.
- Independent write queues per subscriber.
- Client reconnection support.
- Socket permissions restricted to authorized system users.

