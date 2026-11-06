package main

/*
	The orchestrator dameon
	Keeps connection open with /run/tinysystem/netmon.sock & /run/tinysystem/powermon.sock

	Decision steps:
	If netmon sends "router_nonet":
		state = DEGRADED
	If netmon sends "router_offline":
		state = EMERGENCY
	If powermon sends "power_transition" (Battery -> AC) :
		state = RECOVERING


	During EMERGENCY state
		Initiate Wifi AP

	During RECOVERING state
		First wait till powermon commit outage report in hardKV
		Then if netmon already sent "router_normal"
		Turn off AP and transition to NORMAL
		Else wait till timeout and transition to DEGRADED

*/
