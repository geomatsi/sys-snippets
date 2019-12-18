package main

import (
	"bufio"
	"fmt"
	"io"
	"net"
	"os"
)

func main() {
	srv, err := net.Listen("tcp", "localhost:10000")
	if err != nil {
		fmt.Printf("err: listen: %s\n", err)
		os.Exit(-1)
	}

	for {
		conn, err := srv.Accept()
		if err != nil {
			fmt.Printf("err: accept: %s\n", err)
			continue
		}
		go handleEcho(conn)
	}
}

func handleEcho(conn net.Conn) {
	var r *bufio.Reader
	var b []byte
	var e error

	defer conn.Close()

	fmt.Printf("client %s: start\n", conn.RemoteAddr())
	r = bufio.NewReader(conn)

	for {
		b, e = r.ReadBytes('\n')
		if e != nil {
			if e == io.EOF {
				fmt.Printf("client %s: done\n", conn.RemoteAddr())
				return
			}

			fmt.Printf("err: read: %s\n", e)
			return
		}

		fmt.Printf("recv string: %v\n", b)

		_, e = conn.Write(b)
		if e != nil {
			fmt.Printf("err: write: %s\n", e)
			return
		}
	}
}
