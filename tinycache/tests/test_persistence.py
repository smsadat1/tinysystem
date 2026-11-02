import pytest
import time

from conftest import request

def test_key_value_persistence_after_eviction(client):
    assert request(client, "SET Eggplant 909") == "OK"
    assert request(client, "EXPIRE Eggplant 10") == "OK"
    time.sleep(11+6)    # for 5s of interval of eviction worker
    assert request(client, "GET Eggplant") == "909"
    assert request(client, "DELETE Eggplant") == "OK"
    assert request(client, "GET Eggplant") == "(nil)"


def test_key_value_persistence_before_eviction(client):
    assert request(client, "SET Eggplant 909") == "OK"
    assert request(client, "EXPIRE Eggplant 10") == "OK"
    time.sleep(11)
    assert request(client, "GET Eggplant") == "909"
    assert request(client, "DELETE Eggplant") == "OK"
    assert request(client, "GET Eggplant") == "(nil)"