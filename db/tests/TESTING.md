# 1. Core operations — must pass first

| Test                | Action                              | Expected result                               |
|---------------------|-------------------------------------|-----------------------------------------------|
| Set a key           | `SET apple 123`                     | `OK`                                          |
| Read a key          | `GET apple`                         | `123`                                         |
| Read missing key    | `GET missing`                       | Missing-key response                          |
| Update a key        | `SET apple 456`                     | `OK`; subsequent `GET` returns `456`          |
| Delete a key        | `DELETE apple`                      | `OK`                                          |
| Read deleted key    | `GET apple`                         | Missing-key response                          |
| Delete missing key  | `DELETE missing`                    | Consistent missing-key response               |
| Check existence     | `EXISTS apple`                      | Correct result before and after deletion      |
| Empty value         | `SET empty ""`                      | Correctly handles an empty value,if supported |
| Special characters  | Keys/values containing punctuation  | Correct round-trip                            |
| Long key            | Key with 1,000 characters           | Works or returns a documented error           |
| Long value          | Value with 1 MB of data             | Works within configured limits                |

### Important edge cases

Test keys that:
- Are one character long.
- Are very long.
- Share the same hash-table bucket.
- Differ only in the final character.
- Have never been inserted before.

**Collision test:** Deliberately generate multiple keys whose hashes map to the same bucket. Insert them all, retrieve them all, delete one from the middle of the collision chain, and verify the others still work.

---

# 2. TTL and expiration

| Test                  | Action                                            | Expected result                                 |
|-----------------------|---------------------------------------------------|-------------------------------------------------|
| Set TTL               | `EXPIRE apple 10`                                 | Expiration set                                  |
| Check TTL             | `TTL apple`                                       | Positive remaining time                         |
| Wait for expiration   | Wait 11 seconds, then `GET apple`                 | Key treated as missing                          |
| TTL on missing key    | `TTL missing`                                     | Documented missing-key response                 |
| TTL without expiry    | `SET apple 123`, then `TTL apple`                 | Documented no-expiry response                   |
| Reset TTL             | Set TTL to 30, then 60                            | Expiration updated                              |
| Replace value         | Update a key that has a TTL                       | TTL behavior matches your documented semantics  |
| Delete expiring key   | Delete before expiration                          | Key is removed; no later double-free            |
| Expire then recreate  | Let a key expire, then insert the same key again  | New entry works normally                        |
| Zero TTL              | `EXPIRE apple 0`                                  | Immediate expiration or a documented error      |
| Negative TTL          | `EXPIRE apple -5`                                 | Rejected or handled consistently                |

### Decide on these semantics

- `TTL key` → `-2` if the key is missing or expired.
- `TTL key` → `-1` if the key exists without expiration.
- `TTL key` → remaining seconds otherwise.

Also, if expiration is lazy, decide whether `STAT` counts expired entries that haven't yet been cleaned up.

**One particularly important test:** repeatedly expire and recreate the same key. Verify that the old entry isn't accidentally left in the hash table or LRU list.

---

# 3. Memory accounting and eviction

This is where TinyCache starts behaving like a real cache.

You've defined `mem_used` as the total size of **value payloads only**, excluding keys, metadata, and allocator overhead.

Test that accounting stays correct.

| Test | Action | Expected result |
|---|---|---|
| Insert | Insert a 10-byte value | Usage increases by 10 |
| Update | Replace it with a 20-byte value | Usage increases by a net 10 |
| Delete | Delete the key | Usage decreases by 20 |
| Expire | Let the key expire and trigger cleanup | Usage decreases exactly once |
| Evict | Force an eviction | Usage decreases by the victim's value size |
| Empty value | Insert an empty value, if supported | Usage remains consistent |
| Repeated update | Update the same key 1,000 times | No accounting drift |
| Oversized value | Insert a value larger than the allowed maximum | Rejected without corrupting the cache |

For example:

```text
SET a 1234567890
STAT
```

Expected payload memory: `10` bytes.

Then:

```text
SET a abc
STAT
```

Expected payload memory: `3` bytes.

Then:

```text
DELETE a
STAT
```

Expected payload memory: `0` bytes.

### Test the actual memory limit

Run TinyCache with a deliberately tiny memory limit.

For example, if the maximum is 100 bytes:

1. Insert values totaling 80 bytes.
2. Insert another value requiring 30 bytes.
3. Verify the configured policy evicts enough entries or rejects the write.
4. Verify `mem_used <= mem_max`.
5. Verify all surviving keys still return the correct values.

If your policy rejects values larger than `mem_max / 2`, test that boundary too:

- Value size exactly at the limit.
- Value size one byte above it.
- Value too large when the cache is empty.
- Value too large when the cache contains other entries.

**Important:** decide whether an oversized write is rejected before any eviction. I'd recommend rejecting it first so one impossible write can't empty the cache.

---

# 4. LRU eviction

For LRU, the key property is:

> Evict the entry that has gone the longest without being accessed.

Use a tiny memory limit so eviction happens quickly.

### Test A: Basic LRU

Assume each value uses 10 bytes and the cache can hold three entries.

```text
SET a 1111111111
SET b 2222222222
SET c 3333333333
```

Now access `a`:

```text
GET a
```

Then insert `d`:

```text
SET d 4444444444
```

Expected state:

| Key | Expected |
|---|---|
| `a` | Present |
| `b` | Evicted |
| `c` | Present |
| `d` | Present |

Why? Accessing `a` makes it recently used, so `b` becomes the least recently used entry.

### Test B: Updating an existing key

1. Insert `a`, `b`, and `c`.
2. Access `a`.
3. Update `b`.
4. Insert another key to force eviction.

Decide whether updating an existing key counts as an access. I'd recommend that it does, moving `b` to the MRU position.

### Test C: LRU linked-list edge cases

Test eviction when:
- The cache has one entry.
- The victim is the tail.
- The accessed entry is already the head.
- The list has exactly two entries.
- An entry is deleted from the middle.
- An entry expires while it's in the list.
- An entry is updated repeatedly.

These cases are important because pointer bugs often occur at the head and tail.

---

# 5. LFU eviction

For LFU, the rule changes:

> Evict the entry with the lowest access frequency.

Your current `uint32_t frequency` field is enough to implement a simple version.

### Test A: Different frequencies

Insert three keys:

```text
SET a alpha
SET b bravo
SET c charlie
```

Access them different numbers of times:

```text
GET a
GET a
GET a
GET b
GET b
GET c
```

Ignoring insertion frequency for this example, their access counts are:

| Key | Accesses |
|---|---:|
| `a` | 3 |
| `b` | 2 |
| `c` | 1 |

Force an eviction.

**Expected:** `c` is evicted.

Make sure your implementation consistently defines which operations count as accesses. I'd count successful `GET` operations and decide explicitly whether `EXISTS` and updating an existing key increment frequency.

### Test B: Frequency ties

Insert several keys and access each the same number of times.

Force an eviction.

If you're using LRU as the tie-breaker, the least recently used among the tied entries should be evicted.

### Test C: Frequency survives updates

Decide what happens when you update an existing key.

For a straightforward cache, I'd preserve the existing entry's frequency and treat the update as an access. Alternatively, you can reset frequency, but document the behavior and test it.

### Test D: Expiration under LFU

1. Insert two entries with different frequencies.
2. Give one a TTL.
3. Let it expire.
4. Trigger expiration cleanup.
5. Force eviction.

Verify that the expired entry is removed correctly and isn't selected as a live eviction victim.

### Test E: Counter overflow

Your `uint32_t` counter can overflow. Either saturate it at `UINT32_MAX` or define another aging strategy.

For now, saturation is the simpler option.

---

# 6. Switching between LRU and LFU

Since you're supporting both policies, test that the selected policy actually controls victim selection.

For each policy:

1. Start TinyCache with the policy configured.
2. Insert the same keys.
3. Perform the same sequence of accesses.
4. Force eviction.
5. Verify the expected victim.
6. Check `STAT` to ensure the selected policy is reported correctly.

Also verify the default policy if no option is provided.

If you're selecting the policy at startup, test valid and invalid configuration values:

```text
--eviction LRU
--eviction LFU
--eviction UNKNOWN
```

Invalid values should produce a useful error rather than silently selecting a different policy.

---

# 7. TCP protocol and client handling

Your cache is a TCP server, so test the network layer separately from the cache logic.

### Basic protocol tests

- Send one command and receive one response.
- Send multiple commands over the same connection.
- Send a malformed command.
- Send an unknown command.
- Send too few arguments.
- Send too many arguments.
- Send a command with a very long key or value.
- Disconnect immediately after sending a command.
- Connect and disconnect without sending anything.
- Send an empty line.
- Send commands with extra spaces.
- Send a command split across multiple TCP writes.

### Why fragmented writes matter

TCP is a **byte stream**, not a message protocol.

For example, the client might send:

```text
SET apple 123\n
```

But the server might receive it in chunks:

```text
SET ap
ple 123
\n
```

Your framing layer must handle that correctly.

Likewise, a single `recv()` call isn't guaranteed to receive an entire response. Define framing—newline-delimited messages are a reasonable starting point—and handle incomplete reads and writes.

### Multiple clients

Test:

1. Two clients reading different keys.
2. Two clients reading the same key.
3. Two clients writing different keys.
4. Two clients writing the same key.
5. One client deleting a key while another reads it.
6. One client expiring a key while another accesses it.
7. Many clients sending commands simultaneously.

For concurrent updates to the same key, you may not be able to predict which write wins. What you *can* require is that the result is valid, the server doesn't crash, and the internal state remains consistent.

---

# 8. C-specific memory safety tests

This deserves its own section because a cache can pass every functional test and still have use-after-free bugs.

### Compile with sanitizers

If you're using GCC and CMake, you can create a separate sanitizer build:

```bash
cmake -S . -B build-asan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -g" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"

cmake --build build-asan
```

Run your test suite against that build.

Look for:
- Heap buffer overflows.
- Use-after-free.
- Double-free.
- Invalid reads and writes.
- Undefined behavior.
- Memory leaks when the process exits.

AddressSanitizer is particularly useful for the kind of bucket-array bug you just found.

### Also test these sequences repeatedly

```text
SET a one
SET a two
SET a three
DELETE a
DELETE a
```

```text
SET a hello
EXPIRE a 1
```

Wait for expiration, then:

```text
GET a
SET a world
GET a
```

And:

```text
SET a one
SET b two
DELETE a
SET c three
```

Repeat each sequence hundreds or thousands of times. The goal is to expose stale pointers and broken list links.

**Important:** if you return pointers into cache entries from `GET`, don't let the caller use those pointers after releasing the cache mutex if another thread could delete or evict the entry. Copying the value into caller-owned memory is a safer initial design.

---

# 9. Randomized testing

Once the basic tests pass, generate random sequences of operations.

For example:

```text
SET k1 value1
GET k1
SET k2 value2
DELETE k1
EXISTS k2
SET k1 value3
GET k2
```

Generate thousands of commands with:
- Random keys.
- Random values.
- Repeated updates.
- Missing-key accesses.
- Deletes.
- TTL operations.
- Small memory limits to force eviction.

Maintain a simple reference model in Python using a dictionary. For operations where eviction is not involved, compare TinyCache's responses against the reference model.

For eviction tests, implement the same documented LRU/LFU semantics in the reference model and compare the surviving keys.

This can uncover bugs that hand-written examples miss.

---

# 10. Benchmarks — when correctness is solid

After functional and memory-safety testing, benchmark TinyCache against Redis under the same conditions.

Measure:

| Metric | Why it matters |
|---|---|
| Throughput | Operations per second |
| p50 latency | Typical request latency |
| p95 latency | Latency experienced by slower requests |
| p99 latency | Tail latency under load |
| Memory usage | Actual process memory, not just payload accounting |
| CPU usage | Cost of hashing, locking, parsing, and eviction |
| Behavior under pressure | Performance near the memory limit |

Use identical workloads, key/value sizes, concurrency, and hardware.

Don't compare TinyCache's `mem_used` against Redis's RSS: your counter excludes keys, metadata, allocator overhead, and internal structures, while RSS includes much more.

And remember that Redis has many capabilities beyond a simple in-memory key-value cache. You don't need to replicate all of them for TinyCache to be a strong systems project.

---

## My recommended testing order

If I were working on TinyCache, I'd tackle these in order:

1. **Core operations and hash collisions**
2. **Memory accounting**
3. **LRU linked-list edge cases**
4. **TTL and expiration**
5. **LFU victim selection and tie-breaking**
6. **ASan/UBSan runs**
7. **Multiple-client concurrency**
8. **Randomized tests**
9. **Benchmarks**

You don't need a giant test framework on day one. Start with small C unit tests for cache operations, then add integration tests that launch the server and send TCP commands.

**The most important principle:** test not just whether the correct key is returned, but whether every operation leaves the hash table, LRU list, counters, and allocated memory in a consistent state. That's where the really nasty cache bugs live. 💀