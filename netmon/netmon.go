package main

/*
A daemon that monitors netowork state

Exposes an event interface to tell external system current router state
There are 3 possible router state:
	1. router_normal: router has power and network
	2. router_nonet: router has power but no internet
	2. router_offline: router has no power
Uses persistent unix domain socket (/run/tinysystem/netmon.sock) for push oriented event updates
JSON format:

	{
	  "sequence": 41,
	  "event": "network_transition",
	  "from": "router_normal",
	  "to": "router_nonet",
	  "timestamp": "2026-11-06T15:30:00Z",
	}

*/
