import asyncio
from asyncio import StreamReader, StreamWriter
import ssl

from config import (
    BACKEND_HOST, BACKEND_PORT, MAX_BODY_SIZE
)
from validate import  (
    clean_header, read_headers, write_headers, get_headers
)



async def proxy(client_reader: StreamReader, client_writer: StreamWriter):

    backend_reader: StreamReader 
    backend_writer: StreamWriter

    try:
        # 1. Handle startline
        start_line = await asyncio.wait_for(
            client_reader.readline(),
            timeout=15,
        )
        if not start_line:
            return

        parts = start_line.rstrip(b"\r\n").split(b" ")
        if len(parts) != 3:
            # must be Method Path Version
            return

        method, path, version = parts
        if version not in (b"HTTP/1.0", b"HTTP/1.1"):
            # only HTTP/1.0 & HTTP/1.1 supported
            return 

        # 2. Handle headers
        headers = await asyncio.wait_for(read_headers(client_reader), timeout=15)

        # reject request bodies framed using chunked encoding.
        # TinyLB supports Content-Length only.
        if get_headers(headers=headers, name=b"transfer-encoding"):
            client_writer.write(
                b"HTTP/1.1 501 Not Implemented\r\n"
                b"Content-Length: 0\r\n"
                b"Connection: close\r\n\r\n"
            )
            await client_writer.drain()
            return
        
        # reject ambiguous Content-Length values.
        lengths = get_headers(headers, b"content-length")

        if len(lengths) > 1:
            raise ValueError("Multiple Content-Length headers")

        body_length = 0

        if lengths:
            try:
                raw_length = lengths[0]
                if not raw_length or not raw_length.isdigit():
                    raise ValueError

                body_length = int(raw_length)

                if body_length > MAX_BODY_SIZE:
                    raise ValueError
            except ValueError:
                client_writer.write(
                    b"HTTP/1.1 400 Bad Request\r\n"
                    b"Content-Length: 0\r\n"
                    b"Connection: close\r\n\r\n"
                )
                await client_writer.drain()
                return


        # 3. Handle body
        body = await asyncio.wait_for(
            client_reader.readexactly(body_length), timeout=30
        )

        backend_reader, backend_writer = await asyncio.wait_for(
            asyncio.open_connection(BACKEND_HOST, BACKEND_PORT),
            timeout=15,
        )

        # prepare upstream request preserving the request path and query string.
        # Rewrite Host because the upstream is a configured backend.
        upstream_headers = clean_header(headers=headers)
        upstream_headers = [
            (name, value) 
            for name, value in upstream_headers
            if name.lower() not in (b"host", b"content-length")
        ]
        upstream_headers.append((b"Host", f"{BACKEND_HOST}:{BACKEND_PORT}".encode()))
        upstream_headers.append((b"Content-Length", str(body_length).encode()))
        upstream_headers.append((b"Connection", b"close"))

        await write_headers(
            backend_writer,
            b" ".join((method, path, b"HTTP/1.1")),
            upstream_headers, # type: ignore
        )

        backend_writer.write(body)
        await backend_writer.drain()


        # 4. Handle backend response
        start_line = await asyncio.wait_for(backend_reader.readline(), timeout=15)
        if not start_line.startswith(b"HTTP/"):
            raise ValueError("Invalid upstream response")

        response_headers = await asyncio.wait_for(read_headers(backend_reader), timeout=30)
        
        # remove connection specific headers
        response_headers = clean_header(response_headers)
        
        # close connection after one response
        response_headers = [
            (name, value)
            for name, value in response_headers
            if name.lower() != b"connection"
        ]
        response_headers.append((b"Connection", b"close"))

        await write_headers(backend_writer, start_line.rstrip(b"\r\n"), response_headers)


        # 5. Relay backend response to the client
        # The backend connection uses Connection: close, so EOF
        # marks the end of the response stream.
        while True:
            chunk = await asyncio.wait_for(
                backend_reader.read(64 * 1024),
                timeout=60,
            )

            if not chunk:
                break

            client_writer.write(chunk)
            await client_writer.drain()


    except asyncio.IncompleteReadError:
        pass 

    except (TimeoutError, ValueError, OSError) as exc:
        print(f"Proxy error: {exc}")

        # No need to inject second response when headers were already sent
        if not client_writer.is_closing():
            try:
                client_writer.write(
                    b"HTTP/1.1 502 Bad Gateway\r\n"
                    b"Content-Length: 0\r\n"
                    b"Connection: close\r\n\r\n"
                )
                await client_writer.drain()
            except OSError:
                pass
    finally:
        if backend_writer is not None:
            backend_writer.close()
            try:
                await backend_writer.wait_closed()
            except OSError:
                pass 

        client_writer.close()
        try:
            await client_writer.wait_closed()
        except OSError:
            pass

    
