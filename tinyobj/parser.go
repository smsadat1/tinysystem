package main

import (
	"io"
	"log"
	"net"
	"strconv"
	"strings"
)

func handlePUT(header []string, conn net.Conn) []byte {
	if len(header) < 2 {
		return []byte("V1 FAIL 17\nMalformed header\n")
	}

	keylen, err := strconv.Atoi(header[0])
	if err != nil {
		mesglen := len(err.Error())
		return []byte("V1 FAIL " + string(mesglen) + err.Error())
	}
	if keylen < 1 {
		return []byte("V1 FAIL 14\nKey too small\n")
	}

	valuelenlen, err := strconv.Atoi(header[1])
	if err != nil {
		mesglen := len(err.Error())
		return []byte("V1 FAIL " + string(mesglen) + err.Error())
	}
	if valuelenlen < 1 {
		return []byte("V1 FAIL 16\nValue too small\n")
	}

	return []byte("V1 SUCCESS 3\nOK\n")
}

func handleGET(header []string, conn net.Conn) []byte {

}

func handleDELETE(header []string) []byte {

}

func handleLIST(header []string) []byte {

}

func handleRENAME(header []string) []byte {

}

func (s *server) ParseCommands(conn net.Conn) {

	buffer := make([]byte, 1024)

	for {
		n, err := conn.Read(buffer)
		if err != nil {
			if err != io.EOF {
				log.Printf("Read error: %s\n", err.Error())
			}
			return
		}

		header := string(buffer[:n])

		// parse header
		parts := strings.Split(header, " ")

		/*
		 * 	Header format
		 * 	[|VERSION_INFO| |COMMAND| |KEYLEN| |OPTIONAL|]
		 * 	OPTIONAL can be newkeylen for RENAME, valuelen for PUT
		 */

		if len(parts) > 0 {

			if strings.Compare("V1", parts[0]) != 0 {
				response := "V1 FAIL 29\nUnsupported protocol version\n"
				conn.Write([]byte(response))
			}

			if len(parts) > 1 {

				var response []byte
				command := parts[1]

				switch command {
				case "PUT":
					response = handlePUT(parts[2:], conn)
				case "GET":
					response = handleGET(parts[2:], conn)
				case "DELETE":
					response = handleDELETE(parts[2:])
				case "RENAME":
					response = handleRENAME(parts[2:])
				case "LIST":
					response = handleLIST(parts[2:])
				default:
					response = []byte("V1 FAIL 16\nUnknown command\n")
				}

				conn.Write([]byte(response))
			}
		}
	}
}
