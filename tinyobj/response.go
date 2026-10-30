package main

import (
	"fmt"
	"io"
	"net"
)

func writeAll(w io.Writer, data []byte) error {
	for len(data) > 0 {
		n, err := w.Write(data)
		if err != nil {
			return err
		}
		if n == 0 {
			return io.ErrShortWrite
		}
		data = data[n:]
	}
	return nil
}

func WriteFailure(conn net.Conn, mesg string) {
	res := fmt.Appendf(nil, "V1 FAIL %d\n%s", len([]byte(mesg)), mesg)
	if err := writeAll(conn, res); err != nil {
		return
	}
}

func WriteSuccess(conn net.Conn, mesg string) {
	res := fmt.Appendf(nil, "V1 SUCCESS %d\n%s", len([]byte(mesg)), mesg)
	if err := writeAll(conn, res); err != nil {
		return
	}
}
