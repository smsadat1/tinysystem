LISTEN_HOST = "127.0.0.1"
LISTEN_PORT = 8443

BACKEND_HOST = "127.0.0.1"
BACKEND_PORT = 8001

MAX_HEADER_SIZE = 64 * 1024
MAX_BODY_SIZE = 10 * 1024 * 1024

HOP_BY_HOP = {
    b"connection",
    b"keep-alive",
    b"proxy-authenticate",
    b"proxy-authorization",
    b"te",
    b"trailer",
    b"transfer-encoding",
    b"upgrade",
}


