import socket
import pytest


HOST = "127.0.0.1"
PORT = 9736


@pytest.fixture(scope="session")
def client():
    with socket.create_connection((HOST, PORT)) as sock:
        yield sock


def request(client, message):
    client.sendall(f"{message}\n".encode("utf-8"))
    return client.recv(1024).decode("utf-8")


def test_set_get_and_overwrite(client):
    assert request(client, "SET Apple 123") == "OK"
    assert request(client, "GET Apple") == "123"

    assert request(client, "SET Apple 456") == "OK"
    assert request(client, "GET Apple") == "456"


def test_get_missing_key(client):
    assert request(client, "GET missing") == "(nil)"


def test_exist_and_delete(client):
    assert request(client, "SET Apple 123") == "OK"
    assert request(client, "EXIST Apple") == "Y"

    assert request(client, "DELETE Apple") == "OK"
    assert request(client, "EXIST Apple") == "N"


@pytest.mark.parametrize(
    "command",
    [
        "SET empty",
        "SET emptyagain",
        "SET whitespace    ",
    ],
)
def test_set_without_value_fails(client, command):
    assert request(client, command) == "FAILED"


def test_long_value(client):
    assert request(client, "SET e 1234567890") == "OK"
    assert request(client, "GET e") == "1234567890"


def test_long_key(client):
    key = (
        "verylongkeythatIaddedheretotestifitworksbutmaybethisisnot"
        "longenoughbtwfuckjews"
    )

    assert request(client, f"SET {key} 1") == "OK"
    assert request(client, f"GET {key}") == "1"