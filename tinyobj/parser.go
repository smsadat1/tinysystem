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

func handleGET(args []string, conn net.Conn, reader *bufio.Reader) {

	if len(args) != 1 {
		WriteFailure(conn, "Malformed GET header")
		return
	}

	keylen, err := strconv.Atoi(args[0])
	if err != nil || keylen < 1 || keylen > 1024 {
		WriteFailure(conn, "Invalid key length")
		return
	}

	keyBytes := make([]byte, keylen)
	if _, err := io.ReadFull(reader, keyBytes); err != nil {
		return
	}
	key := string(keyBytes)

	GetFileObject(conn, key)
}

func handleDELETE(args []string, conn net.Conn, reader *bufio.Reader) {

	if len(args) != 1 {
		WriteFailure(conn, "Malformed DELETE header")
		return
	}

	keylen, err := strconv.Atoi(args[0])
	if err != nil || keylen < 1 || keylen > 1024 {
		WriteFailure(conn, "Invalid key length")
		return
	}

	keyBytes := make([]byte, keylen)
	if _, err := io.ReadFull(reader, keyBytes); err != nil {
		return
	}
	key := string(keyBytes)

	DeletetFileObject(conn, key)
}

func handleLIST(args []string, conn net.Conn, reader *bufio.Reader) {

	if len(args) != 1 {
		WriteFailure(conn, "Malformed LIST header")
		return
	}

	prefixlen, err := strconv.Atoi(args[0])
	if err != nil || prefixlen < 1 || prefixlen > 1024 {
		WriteFailure(conn, "Invalid key length")
		return
	}

	prefixBytes := make([]byte, prefixlen)
	if _, err := io.ReadFull(reader, prefixBytes); err != nil {
		return
	}
	prefix := string(prefixBytes)

	ListFileObjects(conn, prefix)
}

func handleRENAME(args []string, conn net.Conn, reader *bufio.Reader) {

	if len(args) != 2 {
		WriteFailure(conn, "Malformed RENAME header\n")
		return
	}

	oldkeyLen, err := strconv.Atoi(args[0])
	if err != nil || oldkeyLen < 1 || oldkeyLen > 1024 {
		WriteFailure(conn, "Invalid old key length")
		return
	}
	oldkeybytes := make([]byte, oldkeyLen)
	if _, err := io.ReadFull(reader, oldkeybytes); err != nil {
		return
	}
	oldkey := string(oldkeybytes)

	newkeylen, err := strconv.ParseInt(args[1], 10, 64)
	if err != nil || newkeylen < 1 {
		WriteFailure(conn, "Invalid new key length")
		return
	}
	newkeybytes := make([]byte, newkeylen)
	if _, err := io.ReadFull(reader, newkeybytes); err != nil {
		return
	}
	newkey := string(newkeybytes)

	RenameFileObject(conn, oldkey, newkey)
}

func ParseCommands(args []string, conn net.Conn, reader *bufio.Reader) {

	// header's second part: Command
	switch args[0] {
	case "PUT":
		handlePUT(args[1:], conn, reader)
	case "GET":
		handleGET(args[2:], conn, reader)
	case "LIST":
		handleLIST(args[2:], conn, reader)
	case "DELETE":
		handleDELETE(args[2:], conn, reader)
	case "RENAME":
		handleRENAME(args[2:], conn, reader)

	default:
		WriteFailure(conn, "Unknown command")
	}
}
