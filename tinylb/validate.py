# parse and validate headers 

from config import MAX_HEADER_SIZE, HOP_BY_HOP


def get_headers(headers, name):
    values = [
        value 
        for key, value in headers 
        if key.lower() == name
    ]
    return values


def clean_header(headers: list):
    """Remove hop-by-hop headers and their nominated fields."""

    blocked = set(HOP_BY_HOP)

    # add headers listed in Connections too
    for name, value in headers:
        if name.lower() == "connection":
            blocked.update(
                token.strip().lower()
                for token in value.split(b",")
                if token.strip()
            )

    return [
        (name, value)
        for name, value in headers
        if name not in blocked
    ]

async def read_headers(reader):

    headers = []
    header_len = 0

    while True: 
        line = reader.readline()
        header_len += len(line)

        if header_len > MAX_HEADER_SIZE:
            ValueError("Header too long")

        if not line:
            ValueError("Unexpected EOF")

        if not line.endswith("\r\n"):
            ValueError("Malformed header")

        if line[:1] in (b" ", b"\t"):
            raise ValueError("Obsolete folded headers unsupported")

        if b":" not in line:
            raise ValueError("Malformed HTTP header")

        if line == b"\r\n":
            return headers

        name, value = line[:-2].split(b":", 1)

        if not name or any(
            c not in b"!#$%&'*+-.^_`|~"
            and not 48 <= c <= 57
            and not 65 <= c <= 90
            and not 97 <= c <= 122
            for c in name
        ):
            raise ValueError("Invalid header name")

        headers.append((name, value.strip()))

    return headers

async def write_headers(writer, start_tls, headers: dict[str, str]):

    writer.write(start_tls + b"\r\n")

    for name, value in headers:
        header_line = name + ": " + value + "\r\n"
        writer.write(header_line.encode("utf-8"))

    writer.write(b"\r\n")
    writer.drain()



async def handle_client():
    pass