import asyncio
import ssl

from config import (
    BACKEND_HOST, BACKEND_PORT,
    LISTEN_HOST, LISTEN_PORT,
    MAX_BODY_SIZE, MAX_HEADER_SIZE,
)
from proxy import proxy


async def main():
    # Create a TLS context for incoming client connections.
    tls_context = ssl.create_default_context()
    tls_context.minimum_version = ssl.TLSVersion.TLSv1_2
    tls_context.load_cert_chain(certfile="cert.pem", keyfile="key.pem")

    # asyncio handles the TLS handshake and encryption.
    server = await asyncio.start_server(
        proxy,
        LISTEN_HOST,
        LISTEN_PORT,
        ssl=tls_context,
        limit=MAX_HEADER_SIZE + 1,
    )

    print(f"HTTPS proxy listening on https://{LISTEN_HOST}:{LISTEN_PORT}")
    print(f"Forwarding to http://{BACKEND_HOST}:{BACKEND_PORT}")

    async with server:
        await server.serve_forever()


if __name__ == "__main__":
    asyncio.run(main())