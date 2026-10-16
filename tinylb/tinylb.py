import asyncio
import ssl

from .config import (
    BACKEND_HOST, BACKEND_PORT,
    LISTEN_HOST, LISTEN_PORT, 
    MAX_HEADER_SIZE,
)
from .proxy import handle_client

async def main():
    tls_context = ssl.create_default_context()
    tls_context.minimum_version = ssl.TLSVersion.TLSv1_2
    tls_context.load_cert_chain("cert.pem", "key.pem")

    server = await asyncio.start_server(
        handle_client,
        LISTEN_HOST, LISTEN_PORT,
        ssl=tls_context,
        limit=MAX_HEADER_SIZE+1,
    )

    print(f"HTTPS proxy listening on https://{LISTEN_HOST}:{LISTEN_PORT}")
    print(f"Forwarding to http://{BACKEND_HOST}:{BACKEND_PORT}")

    async with server:
        await server.serve_forever()


if __name__ == "__main__":
    asyncio.run(main())
