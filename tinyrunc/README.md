# tinyrunc

**A tiny educational container runtime written entirely in Bash.**


## Example usage
**Step 1**

Create necessary environments for **tinyrunc**
```
chmod +x setup.sh
./setup.sh
chmod +x tinyrunc.sh
```

**Step 2**

Then use it like this (script in src/tinyrunc.sh)
```bash
sudo ./tinyrunc.sh /var/tinyrunc/1/rootfs \
    --memory 128M \
    --pids 128 \
    --cpu 1000:2000 \
    --no-new-privs \
    --readonly-rootfs \
    -- \
    /bin/ls -l
```

## Features
- UTS namespace
- PID namespace
- mount namespace
- isolated /proc
- chroot rootfs
- cgroup v2 resource limits
  - memory
  - CPU
  - PIDs
- no_new_privs
- read-only rootfs
- arbitrary workload + arguments

## Architecture

```
                 tinyrunc parent
                       │
                       ↓
              create/configure cgroup
                       │
                       ↓
                 namespace child
                       │
             ┌─────────┼─────────┐
             ↓         ↓         ↓
          mounts     security   rootfs
             │         │         │
             │    no_new_privs   │
             ↓         ↓         ↓
                    chroot
                       ↓
                    execve()
                       ↓
                    PROGRAM
```


## Non Goals:
- A replacement for runc/crun
- OCI compliance
- Production grade security