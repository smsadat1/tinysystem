package main

import (
	"bufio"
	"fmt"
	"io"
	"net"
	"strconv"
)

func handlePUT(args []string, conn net.Conn, reader *bufio.Reader) {

	if len(args) != 2 {
		WriteFailure(conn, "Malformed PUT header\n")
		return
	}

	keyLen, err := strconv.Atoi(args[0])
	if err != nil || keyLen < 1 || keyLen > 1024 {
		WriteFailure(conn, "Invalid key length")
		return
	}

	objLen, err := strconv.ParseInt(args[1], 10, 64)
	if err != nil || objLen < 1 {
		WriteFailure(conn, "Invalid object length")
		return
	}

	const maxObjSize = int64(10 << 30) // 10 GB size
	if objLen > maxObjSize {
		WriteFailure(conn, "Object too large")
		return
	}

	keyBytes := make([]byte, keyLen)
	if _, err := io.ReadFull(reader, keyBytes); err != nil {
		return
	}

	key := string(keyBytes)
	fmt.Printf("Uploading: %s (%d bytes)\n", key, objLen)

	SaveFileObject(conn, reader, objLen)
}

func handleGET(header []string, conn net.Conn) []byte {

	if len(header) < 1 {
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

	return []byte("V1 SUCCESS 3\nOK\n")
}

func handleDELETE(header []string) []byte {

	if len(header) < 1 {
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

	return []byte("V1 SUCCESS 3\nOK\n")
}

func handleLIST(header []string) []byte {

	if len(header) < 1 {
		return []byte("V1 FAIL 17\nMalformed header\n")
	}

	prefixlen, err := strconv.Atoi(header[0])
	if err != nil {
		mesglen := len(err.Error())
		return []byte("V1 FAIL " + string(mesglen) + err.Error())
	}
	if prefixlen < 1 {
		return []byte("V1 FAIL 14\nKey too small\n")
	}

	return []byte("V1 SUCCESS 3\nOK\n")
}

func handleRENAME(header []string) []byte {

	if len(header) < 2 {
		return []byte("V1 FAIL 17\nMalformed header\n")
	}

	newkeylen, err := strconv.Atoi(header[0])
	if err != nil {
		mesglen := len(err.Error())
		return []byte("V1 FAIL " + string(mesglen) + err.Error())
	}
	if newkeylen < 1 {
		return []byte("V1 FAIL 14\nKey too small\n")
	}

	oldkeylen, err := strconv.Atoi(header[1])
	if err != nil {
		mesglen := len(err.Error())
		return []byte("V1 FAIL " + string(mesglen) + err.Error())
	}
	if oldkeylen < 1 {
		return []byte("V1 FAIL 16\nValue too small\n")
	}

	return []byte("V1 SUCCESS 3\nOK\n")
}

func ParseCommands(args []string, conn net.Conn, reader *bufio.Reader) {

	// header's second part: Command
	switch args[0] {
	case "PUT":
		handlePUT(args[1:], conn, reader)

	case "GET":
		// handleGET(parts[2:], reader, conn)

	case "LIST":
		// handleLIST(parts[2:], reader, conn)

	case "DELETE":
		// handleDELETE(parts[2:], reader, conn)

	case "RENAME":
		// handleRENAME(parts[2:], reader, conn)

	default:
		WriteFailure(conn, "Unknown command")
	}
}
