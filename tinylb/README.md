# TinyLB

**_It's over for NGINX 💀_**
- Python-powered — readable, hackable, no C++ compilation ritual.
- Low setup overhead — clone, install dependencies, configure backends, run.
- Load balancing — Round Robin and Least Connections.
- TLS termination — HTTPS in, HTTP out.
- HTTP/1.1 reverse proxy — accept requests, forward them, relay responses.
- Minimal by design — no sprawling configuration system, no feature creep.


## Setup 

**1. Start TinyHTTP server**
```bash
go run tinyhttp
```
**2. Start TinyLB**
```bash
python main.py
```

**3. Done**
