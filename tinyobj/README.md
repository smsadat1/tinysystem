# TinyObj

## Commands

| Command   | Purpose                                   |
| --------- | ----------------------------------------- |
| `PUT`     |  Upload file                              |
| `GET`     |  Download file                            |
| `LIST`    |  List all files of specified key prefix   |
| `RENAME`  |  Rename key of an object (file)           |
| `DELETE`  |  Delete an object (file)                  |

## Request & Response


<b>Request</b>
```text
V1 PUT <keylen> <objlen>\n
<key bytes><exactly objlen object bytes>
```

<b>Response</b>

```text
V1 SUCCESS 2\n
OK 
```

```text
V1 FAIL <message-length>\n
<message bytes>
```


<b>Request</b>
```text
V1 GET <keylen>\n
<key bytes>
```

<b>**Response**</b>

```text
V1 READY <objlen>\n
<exactly objlen object bytes>
```

```text
V1 FAIL <message-length>\n
<message bytes>
```

<b>Request</b>
```text
V1 LIST <prefixlen>\n
<prefix bytes>
```

<b>Response</b>

```text
V1 READY <payloadlen>\n
<list payload bytes>
```

```text
V1 FAIL <message-length>\n
<message bytes>
```

<b>Request</b>
```text
V1 DELETE <keylen>\n
<key bytes>
```

<b>Response</b>
```text
V1 SUCCESS 2\n
OK
```

```text
V1 FAIL <message-length>\n
<message bytes>
```

<b>Request</b>
```text
V1 RENAME <old_keylen> <new_keylen>\n
<old key bytes><new key bytes>
```

<b>Response</b>

```text
V1 SUCCESS 2\n
OK
```

```text
V1 FAIL <message-length>\n
<message bytes>
```

**Protocol Notes**
- Sending unsupported version will send `V1 FAIL 29\nUnsupported protocol version\n`.
- `GET` `LIST` `RENAME` & `DELETE` are atomic so `FAIL` guarantees operation did not happen.
- `PUT` is not atomic so `FAIL` does not guarantee that operation failed, partial success is possible. Down stream service should retry or drop based on failure incident. 