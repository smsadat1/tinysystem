package main

import (
	"fmt"

	udev "github.com/jochenvg/go-udev"
)

/*
A daemon that monitors laptop power events

# Continuous

Reports battery status daily (86400 seconds) and stores in hardVK
JSONL format:

	{"id": "BAT0", "condition": "30.8/42.0Wh", "capacity_perc": "73.2"}

# Situational

Reports outage report after outage ends and stores in hardKV
JSONL format:

	{
	  "id": "BAT0",
	  "start_energy_wh": 30.8,
	  "start_capacity_percent": 100.0,
	  "end_energy_wh": 15.8,
	  "end_capacity_percent": 51.0,
	  "outage_start": "2026-11-05T17:13:58Z",
	  "duration_seconds": 12218
	}

Exposes an event interface to tell external system whether laptop is currently in AC or Battery
Uses persistent unix domain socket (/run/tinysystem/powermon.sock) for push oriented event updates
JSON format:

	{
	  "sequence": 42,
	  "event": "power_transition",
	  "from": "AC",
	  "to": "BATTERY",
	  "timestamp": "2026-11-06T15:30:00Z",
	}
*/
type PowerMonitor interface {
	ACConnected() (bool, error)
}

func main() {
	u := udev.Udev{}
	e := u.NewEnumerate()

	e.AddMatchSubsystem("net")
	e.AddMatchIsInitialized()

	devs, err := e.Devices()
	if err != nil {
		panic(err)
	}

	for i := range devs {
		device := devs[i]
		fmt.Println(device.Syspath())
	}
}
