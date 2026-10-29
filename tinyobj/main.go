package main

import (
	"log"
	"net"
	"os"
	"os/signal"
	"sync"
	"syscall"
)

type server struct {
	wg         sync.WaitGroup
	listener   net.Listener
	shutdown   chan struct{}
	connection chan net.Conn
}

func main() {

	s, err := newServer("127.0.0.1:9998")
	if err != nil {
		log.Panicf("%s\n", err.Error())
	}

	log.Println("Starting server at port 9998...")
	s.Start()

	// Wait for a SIGINT or SIGTERM signal to gracefully shut down the server
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)
	<-sigChan

	log.Println("Shutting down server...")
	s.Stop()
}
