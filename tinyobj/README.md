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
PUT <keylen> <objlen>\n
<key bytes><exactly objlen object bytes>
```

<b>Response</b>

```text
SUCCESS 2\n
OK 
```

```text
FAIL <message-length>\n
<message bytes>
```


<b>Request</b>
```text
GET <keylen>\n
<key bytes>
```

<b>**Response**</b>

```text
READY <objlen>\n
<exactly objlen object bytes>
```

```text
FAIL <message-length>\n
<message bytes>
```

<b>Request</b>
```text
LIST <prefixlen>\n
<prefix bytes>
```

<b>Response</b>

```text
READY <payloadlen>\n
<list payload bytes>
```

```text
FAIL <message-length>\n
<message bytes>
```

<b>Request</b>
```text
DELETE <keylen>\n
<key bytes>
```

<b>Response</b>
```text
SUCCESS 2\n
OK
```

```text
FAIL <message-length>\n
<message bytes>
```

<b>Request</b>
```text
RENAME <old_keylen> <new_keylen>\n
<old key bytes><new key bytes>
```

<b>Response</b>

```text
SUCCESS 2\n
OK
```

```text
FAIL <message-length>\n
<message bytes>
```