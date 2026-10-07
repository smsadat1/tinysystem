#!/usr/bin/env bash

ROOTFS=$1
shift

LIMITS=()
MEMORY=""
PIDS=""
CPU=""
CPU_QUOTA=""
CPU_PERIOD=""   
CGROUP="/sys/fs/cgroup/tinyrunc"
NO_NEW_PRIVILEGES=false    
READONLY_ROOTFS=false


while [[ "$1" != "--" ]]; do 
    case "$1" in 
        "--memory")
            shift
            MEMORY="$1"
            if [[ "$MEMORY" =~ ^[0-9]+(K|M|G)$ ]]; then
                echo ""
            else 
                echo "Invalid memory: ${MEMORY}"
                exit 1
            fi
            shift
            ;;
        "--pids")
            shift
            PIDS="$1"
            shift
            ;;
        "--cpu")
            shift
            CPU="$1"
            IFS=: read -r CPU_QUOTA CPU_PERIOD <<< "$CPU"
            shift
            ;;
        "--no-new-privs")
            NO_NEW_PRIVILEGES=true
            shift
            ;;
        "--readonly-rootfs")
            READONLY_ROOTFS=true
            shift
            ;;
        *)
            echo "Unknown option: $i"
            exit 1
            ;;
    esac
done
shift

PROGRAM="$1"
shift
ARGS=("$@")

echo "Validating rootfs..."
if [[ -d "$ROOTFS" ]]; then 
    
    echo "Finding program in rootfs..."
    if [[ -x "$ROOTFS$PROGRAM" ]]; then
        
        printf '%s\n'\
    "Preparing cgroups...
    Memory: $MEMORY
    PID limits: $PIDS
    CPU quota: $CPU_QUOTA
    CPU period: $CPU_PERIOD
    No new privileges: $NO_NEW_PRIVILEGES
    Read only rootfs: $READONLY_ROOTFS"
        
        printf '%s\n' "$MEMORY" > "$CGROUP/memory.max"
        printf '%s\n' "$PIDS" > "$CGROUP/pids.max"
        printf '%s %s\n' "$CPU_QUOTA" "$CPU_PERIOD" > "$CGROUP/cpu.max"
        
        (   
            echo "Container process started"
            unshare --uts --pid --mount --fork bash -c  '
                hostname tinyrunc
                echo "Hostname: $(hostname)"
                echo "PID: $$"

                echo "Preparing Rootfs mounts"
                mount -t proc proc "$ROOTFS/proc"
                
                if [[ "$4" == "true" ]]; then
                    echo "Making rootfs readonly..."
                    mount --bind "$1" "$1"
                    mount -o remount,bind,ro "$1"
                else 
                    echo "Warning: Rootfs is not readonly"
                fi

                if [[ "$3" == "true" ]]; then
                    echo "Setting privilege restrictions..."
                    setpriv --no-new-privs chroot "$1" "$2" "${@:5}"
                else 
                    echo "Warning: Privilege restrictions not provided"
                    chroot "$1" "$2" "${@:5}"
                fi

            ' bash "$ROOTFS" "$PROGRAM" "$NO_NEW_PRIVILEGES" "$READONLY_ROOTFS" "${ARGS[@]}"
            
        ) & 
        CHILD_PID=$!
        printf '%s\n' "$CHILD_PID" > "$CGROUP/cgroup.procs"
        wait "$CHILD_PID"
    
    else 
        echo "Program ${PROGRAM} does not exist in Rootfs ${ROOTFS}"
        exit 1
    
    fi

else 
    echo "Rootfs does not exist: ${ROOTFS}"
    exit 1
fi