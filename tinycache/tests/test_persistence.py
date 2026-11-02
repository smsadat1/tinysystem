import pytest
import time

from conftest import request

def test_key_value_persistence_after_eviction(client):
    assert request(client, "SET Eggplant 909") == "OK"
    assert request(client, "GET Eggplant") == "909"
    assert request(client, "DELETE Eggplant") == "OK"
    assert request(client, "GET Eggplant") == "(nil)"


def test_key_value_persistence_before_eviction(client):
    assert request(client, "SET Eggplant 909") == "OK"
    assert request(client, "GET Eggplant") == "909"
    assert request(client, "DELETE Eggplant") == "OK"
    assert request(client, "GET Eggplant") == "(nil)"