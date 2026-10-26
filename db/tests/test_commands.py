import pytest
import time

from conftest import request


def test_stat(client):
    # cache should start fresh
    assert request(client, "STAT KEY") == "0"
    assert request(client, "STAT MEM") == "0"
    assert request(client, "STAT LOL") == "UNKNOWN"

def test_unknown_command(client):
    assert request(client, "LOL") == "UNKNOWN"

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
    assert request(client, "DELETE e") == "OK"

def test_long_key(client):
    key = (
        "verylongkeythatIaddedheretotestifitworksbutmaybethisisnot"
        "longenoughbtwfuckjewsandalsofucklgbtqbastardsandwhitebrits"
    )
    assert request(client, f"SET {key} 1") == "OK"
    assert request(client, f"GET {key}") == "1"
    assert request(client, f"DELETE {key}") == "OK"

def test_key_expiration(client):
    assert request(client, "SET Banana 789") == "OK"
    assert request(client, "EXPIRE Banana 10") == "OK"
    assert request(client, "TTL Banana") == "10"
    assert request(client, "TTL missing") == "NOEXIST"
    assert request(client, "EXPIRE Banana -69") == "ILLEGAL"
    time.sleep(11)
    assert request(client, "GET Banana") == "(nil)"

def test_key_expiration_extended(client):
    assert request(client, "SET Pineapple 789") == "OK"
    assert request(client, "EXPIRE Pineapple 10") == "OK"
    assert request(client, "TTL Pineapple") == "10"
    time.sleep(6)
    assert request(client, "EXPIRE Pineapple 20") == "OK"
    assert request(client, "TTL Pineapple") == "24"
    time.sleep(25)
    assert request(client, "GET Pineapple") == "(nil)"

def test_memory_accounting(client):
    # memory should be reset since all previous test keys deleted subsequently
    assert request(client, "STAT KEY") == "0"
    assert request(client, "STAT MEM") == "0"
