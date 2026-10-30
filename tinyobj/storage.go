package main

import (
	"fmt"
	"io"
	"net"
	"os"

	nanoid "github.com/matoous/go-nanoid/v2"
)

/*
	1. Generate an object_id for the key
	2. Create and write object file
	3. Save [key, object_id] to tinycache
*/

func SaveFileObject(conn net.Conn, reader io.Reader, objlen int64) {

	object_id, err := nanoid.Generate("0123456789abcdefghijklmnopqrstuvwxyz", 8)
	if err != nil {
		WriteFailure(conn, "Failed to generated object id\n")
		return
	}

	f, err := os.Create(object_id)
	if err != nil {
		WriteFailure(conn, "Could not create object\n")
		return
	}

	written, err := io.CopyN(f, reader, objlen)
	if err != nil {
		fmt.Printf("Object %s partially written (%d/%d)\n", object_id, written, objlen)
		return
	}

	if err := f.Sync(); err != nil {
		WriteFailure(conn, "Storage write failed\n")
		return
	}

	if err := f.Close(); err != nil {
		WriteFailure(conn, "Storage close failed\n")
		return
	}

	WriteSuccess(conn, "Storage write successful\n")
}
