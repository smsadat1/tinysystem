package main

import (
	"fmt"
	"io"
	"log"
	"net"
	"time"
)

func newServer(address string) (*server, error) {

	l, err := net.Listen("tcp", address)
	if err != nil {
		return nil, fmt.Errorf("Listen error: %s", err)
	}

	return &server{
		listener:   l,
		shutdown:   make(chan struct{}),
		connection: make(chan net.Conn),
	}, nil
}

func (s *server) acceptConnections() {
	defer s.wg.Done()

	for {
		select {
		case <-s.shutdown:
			return
		default:
			conn, err := s.listener.Accept()
			if err != nil {
				continue
			}
			s.connection <- conn
		}
	}
}

func (s *server) handleConnection(conn net.Conn) {

	buffer := make([]byte, 1024)

	for {
		n, err := conn.Read(buffer)
		if err != nil {
			if err != io.EOF {
				log.Printf("Read error: %s\n", err.Error())
			}
			return
		}

		fmt.Printf("Client: %s", string(buffer[:n]))
	}
}

func (s *server) handleConnections() {
	s.wg.Done()

	for {
		select {
		case <-s.shutdown:
			return
		case conn := <-s.connection:
			go s.handleConnection(conn)
		}
	}
}

func (s *server) Start() {
	s.wg.Add(2)
	go s.acceptConnections()
	go s.handleConnections()
}

func (s *server) Stop() {
	close(s.shutdown)
	s.listener.Close()

	done := make(chan struct{})
	go func() {
		s.wg.Wait()
		close(done)
	}()

	select {
	case <-done:
		return
	case <-time.After(time.Second):
		log.Println("Timed out waiting for connections to finish.")
		return
	}
}
